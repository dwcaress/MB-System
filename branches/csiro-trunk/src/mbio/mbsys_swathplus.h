/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_swathplus.h	2/22/2008
 *	$Id$
 *
 *    Copyright (c) 2008-2013 by
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
 * NOTE: In early 2013 SEA sold the SWATHplus system to BathySwath where
 * the system will be rebranded the BathySwath.
 *
 * mbsys_swathplus.h defines the MBIO data structures for handling data from
 * BathySwath (formerly SEA SWATHplus) interferometric formats:
 *      MBF_SWPLSSXI : MBIO ID 221 - SWATHplus intermediate format
 *      MBF_SWPLSSXP : MBIO ID 221 - SWATHplus processed format
 *
 * Author:	David Finlayson
 * Date:	August 27, 2013
 *
 * $Log: mbsys_swathplus.c,v $
 *
 */
/*
 * Notes on the MBSYS_SWATHPLUS data structure:
 *
 *   1. BathySwath defines three data formats associated with the SWATHplus
 *      interferometric sonar: raw, intermediate, and processed.
 *      MB-System supports the intermediate format as MBIO
 *      format 221 (MBF_SWPLSSXI) and the processed format as MBIO
 *      format 222 (MBF_SWPLSSXP).
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
 *      files for each input sxp file identifying the correct PCS.  (see
 *      mbr_swplssxp.c for more information).
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

/* sonar models */
#define MBSYS_SWPLS_UNKNOWN 0
#define MBSYS_SWPLS_XL      40
#define MBSYS_SWPLS_L       117
#define MBSYS_SWPLS_M       234
#define MBSYS_SWPLS_H       468

/* maximum number of beams and pixels */
#define MBSYS_SWPLS_MAX_BEAMS   8194
#define MBSYS_SWPLS_MAX_PIXELS  8194
#define MBSYS_SWPLS_MAX_TXERS   3
#define MBSYS_SWPLS_MAX_PACKETS 100
#define MBSYS_SWPLS_MAX_COMMENT 0
#define MBSYS_SWPLS_BUFFER_SIZE 2048

/* transducer azimuth beam widths */
#define SWPLS_TYPE_L_BEAM_WIDTH 0.85
#define SWPLS_TYPE_M_BEAM_WIDTH 0.55
#define SWPLS_TYPE_H_BEAM_WIDTH 0.55

/* Processed (SXP) datagrams */
#define SWPLS_ID_NONE               0x0		/* 0 means no record at all */
#define SWPLS_ID_UNKNOWN            0x1
#define SWPLS_ID_SXP_HEADER_DATA    0x01df01df	/* Processed file header */
#define SWPLS_ID_XYZA_PING          0x28	/* processed ping data (prior to January 2010) */
#define SWPLS_ID_XYZA_PING2         0x52	/* processed ping data (after January 2010) */
#define SWPLS_ID_SBP_PROJECTION     0x0		/* PLACEHOLDER */
#define SWPLS_ID_PROJECTION         0x0		/* PLACEHOLDER */

/* Parsed (SXI) datagrams */
#define SWPLS_ID_SXI_HEADER_DATA    0x521d52d1	/* parsed data file header */
#define SWPLS_ID_PARSED_PING_DATA   0x29	/* sonar data in parsed data */
#define SWPLS_ID_PARSED_ATTITUDE    0x2b	/* attitude data in parsed data */
#define SWPLS_ID_PARSED_POSITION_LL 0x2c	/* lat-long position idata in parsed data */
#define SWPLS_ID_PARSED_POSITION_EN 0x2d	/* easting-northing data in parsed data */
#define SWPLS_ID_PARSED_SVP         0x2e	/* speed of sound data in parsed data */
#define SWPLS_ID_PARSED_ECHOSOUNDER 0x2f	/* echosounder data in parsed data */
#define SWPLS_ID_PARSED_TIDE        0x30	/* tide data in parsed data */
#define SWPLS_ID_PARSED_AGDS        0x31	/* AGDS data in parsed data */

/* MB-System Custom datagrams (BathySwath approved) */
#define SWPLS_ID_COMMENT            0x100	/* MB System comment */

/* Record sizes definitions */
#define SWPLS_SIZE_NONE         0			/* 0 means no record at all  */
#define SWPLS_SIZE_BLOCKHEADER  8			/* Block Header (blockid, blocksize) */
#define SWPLS_SIZE_STARTER      8			/* File version block */
#define SWPLS_SIZE_PING         256			/* Old-style pings (prior to Jan 2010)  */
#define SWPLS_SIZE_PING2        264			/* New-style pings (after Jan 2010)     */
#define SWPLS_SIZE_POINT        40			/* Old-style points (prior to Jan 2010) */
#define SWPLS_SIZE_POINT2       48			/* New-style points (after Jan 2010)    */
#define SWPLS_MAX_RECORD_SIZE   ( SWPLS_SIZE_PING2 + ( MBSYS_SWPLS_MAX_BEAMS * SWPLS_SIZE_POINT2 ))

/* SWATHplus constants */
#define SWPLS_MAX_LINENAME_LEN  40
#define SWPLS_MAX_TX_INFO       4

/* Transducer channel names (2-channel system) */
#define SWPLS_TXNO_PORT 1
#define SWPLS_TXNO_STBD 2
#define SWPLS_TXNO_CNTR 3

/* Board type codes */
#define SWPLS_BRD_TYPE_117_Q0   1
#define SWPLS_BRD_TYPE_117      2
#define SWPLS_BRD_TYPE_IS           3
#define SWPLS_BRD_TYPE_23           4
#define SWPLS_BRD_TYPE_117_     5
#define SWPLS_BRD_TYPE_234_     6
#define SWPLS_BRD_TYPE_468_A        7
#define SWPLS_BRD_TYPE_USB_468  8

/* Transducer type identifier codes */
#define SWPLS_TXD_TYPE_117      10	/* 117187.5 Hz */
#define SWPLS_TXD_TYPE_234      5	/* 234375.0 Hz */
#define SWPLS_TXD_TYPE_468      13	/* 468750.0 Hz */
#define SWPLS_TXD_TYPE_NO_CONN  15	/* No transducer connected to TEM */

/* Transducer ping modes (bits 0-1 of status) */
#define SWPLS_SONAR_SEL_MASK    3
#define SWPLS_SONAR_SEL_OFF     0
#define SWPLS_SONAR_SEL_SINGLE  1
#define SWPLS_SONAR_SEL_ALT     2
#define SWPLS_SONAR_SEL_SIM     3

/* Transducer transmit modes (bit 2 of status) */
#define SWPLS_SONAR_PASSIVE 0
#define SWPLS_SONAR_ACTIVE  1

/* Point filter status */
#define SWPLS_POINT_REJECTED        0
#define SWPLS_POINT_ACCEPTED        1

/* SWATHplus file header structure (SXI, SXP)*/
typedef struct mbsys_swplssxp_file_header_struct
	{
	int swver;										/* 3065601 means: Major version 3, Minor version
													   06, Release 56, Build 01 */
	int fmtver;										/* Obsolete */
	} swplssxp_header;

/* SWATHplus processed point data. (SXP) */
typedef struct mbsys_swplssxp_point_struct
	{
	int sampnum;									/* sample number, rejected may not be present */
	double y;										/* north coordinate (m) */
	double x;										/* east coordinate (m) */
	float z;										/* depth positive down (m) */
	unsigned short int amp;							/* raw amplitude (16-bit) */
	unsigned short int procamp;	/* processed amplitude (16-bit) */
	unsigned char status;							/* 0 (bad) or 1 (good) */
	double tpu;										/* total propagated uncertainty (m) [Version 2
													   pings only] */
	} swplssxp_point;

/* SWATHplus processed ping data (SXP)*/
typedef struct mbsys_swplssxp_ping_struct
	{
	char linename[SWPLS_MAX_LINENAME_LEN];	/* apparently not used by SEA software */
	unsigned int pingnumber;						/* ping number */
	double time_d;									/* UNIX time of start of ping */
	int notxers;									/* number of transducers in this record, always
													   1 */
	double easting;									/* easting coordinate of transducer (m) */
	double northing;								/* northing coordinate of transducer (m) */
	double roll;									/* roll at start of ping (deg) */
	double pitch;									/* pitch at start of ping (deg) */
	double heading;									/* heading at start of ping (deg) */
	double height;									/* height of crp in survey datum (m) */
	double tide;									/* tide at start of ping (m) */
	double sos;										/* speed of sound (mean value) */
	unsigned char txno;								/* transducer identifier */
	unsigned char txstat;							/* transducer status */
	unsigned char txpower;							/* power setting */
	short int analoggain;							/* analog gain setting */
	unsigned char nostaves;							/* number of staves on transducer */
	unsigned char txinfo[SWPLS_MAX_TX_INFO];		/* board type/revision/serial num. */
	unsigned char freq;								/* frequency code */
	double frequency;								/* frequency in hertz */
	short int trnstime;								/* transmit time/number of cycles */
	short int recvtime;								/* receive time/number of samples */
	unsigned char samprate;							/* receive rate micro-sec/sample */
	int nosampsorig;								/* num. samp. read in real time */
	int nosampsfile;								/* num. samp. in the processed file */
	int nosampslots;								/* number of sample slots */
	double txer_e;									/* easting coordinate of transducer (m) */
	double txer_n;									/* northing coordinate of transducer (m) */
	double txer_height;								/* positive down (m) */
	double txer_forward;							/* positive forward (m) */
	double txer_starboard;							/* positive starboard (m) */
	double txer_azimuth;							/* positive clockwise looking down (deg) */
	double txer_elevation;							/* angle of txer plate, positive angles above
													   horizon (deg) */
	double txer_skew;								/* positive clockwise from rear (deg) */
	double txer_time;								/* time offset (sec) */
	double txer_waterdepth;							/* transducer draft (m) */
	double txer_pitch;								/* positive bow up (deg) [Version 2 pings only]
													 */
	swplssxp_point points[MBSYS_SWPLS_MAX_BEAMS];
	} swplssxp_ping;

/* SWATHplus parsed ping data (SXI) */
struct mbsys_swplsr_parsed_ping_data_struct
	{
	int time_d;										/* start of ping, seconds since 1970 */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the transducer */
	unsigned long pingnumber;						/* simultaneous pings are numbered seperately */
	float frequency;								/* frequency of the transducer in Hz */
	float samp_period;								/* time period between sonar data samples, in
													   seconds */
	unsigned short nosamps;							/* number of samples following */
	float sos;										/* speed of sound used to calcualte angles, m/s
													 */
	short int txpulse;								/* transmit pulse length, in sonar cycles */
	char data_options;								/* allows options in data encoding */
	unsigned char ping_state;						/* records the status of pinging
													   single/alternating/simultaneous */
	unsigned short max_count;						/* maximum data count before filtering */
	unsigned short reserve1;						/* reserved for other ping information */
	unsigned short sampnum[MBSYS_SWPLS_MAX_BEAMS];	/* sample number */

	/* Angle coded +15 bits = 180 deg up, -15 bits = 180 deg down, relative to the txer pointing
	   angle */
	short int angle[MBSYS_SWPLS_MAX_BEAMS];
	/* Amplitude scaled so that 16 bits is the full scale of the ADC */
	unsigned short amplitude[MBSYS_SWPLS_MAX_BEAMS];
	unsigned char quality[MBSYS_SWPLS_MAX_BEAMS];	/* as set by "data options */
	};

/*SWATHplus parsed attitude data (SXI) */
typedef struct mbsys_swplsr_parsed_attitude_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	float roll;										/* positive for starboard down */
	float pitch;									/* positive for nose up */
	float heading;									/* positive clockwise, looking down */
	float height;									/* positive for down */
	} swplssxi_pattitude;

/* Internal data structure for SWATHplus parsed position geographic coordinates */
typedef struct mbsys_swplsr_parsed_position_ll_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	double latitude;								/* degrees (of survey center?) */
	double longitude;								/* degrees (of survey center?) */
	} swplssxi_pposll;

/* Internal data structure for SWATHplus parsed position projected coordinats */
typedef struct mbsys_swplsr_parsed_position_en_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	double easting;									/* easting coordinate (m) */
	double northing;								/* northing coordinate (m) */
	} swplssxi_pposen;

/* Internal data structure for SWATHplus parsed sound speed data */
typedef struct mbsys_swplsr_parsed_svp_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	float sos;										/* speed of sound (m/s) */
	} swplssxi_psvp;

/* Internal data structure for SWATHplus parsed tide data */
struct mbsys_swplsr_parsed_tide_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	float tide;										/* speed of sound (m/s) */
	};

/* Internal data structure for SWATHplus parsed echosounder data */
struct mbsys_swplsr_parsed_echosounder_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	float altitude;	/* hieght above seabed (m) */
	};

/* Internal data structure for SWATHplus parsed Acoustic Ground
   Discrimination System (AGDS) data */
struct mbsys_swplsr_parsed_agds_struct
	{
	int time_d;										/* start of ping time code. */
	int microsec;									/* microseconds since time_d */
	unsigned char channel;							/* identifies the data source */
	float hardness;
	float roughness;
	};

/* MB System data structure */
struct mbsys_swathplus_struct
	{
	/* Type of data record */
	int kind;										/* MB-System record ID */
	int type;										/* SWATHplus datagram ID */

        /* Projection */
	int	projection_set;
        char	projection_id[MB_NAME_LENGTH];

	/* Data records stored? (MB_YES/MB_NO)*/
	int stored_header;
	int stored_ping;
	int stored_comment;

	/* Data records */
	swplssxp_header header;
	swplssxp_ping ping;
	char comment[MB_COMMENT_MAXLINE];

	/* Translated data for MB System */
	int time_i[7];
	double time_d;
	double navlon;
	double navlat;
	double speed;
	double heading;
	double draft;
	double roll;
	double pitch;
	double heave;
	double sos;
	double beamwidth_xtrack;
	double beamwidth_ltrack;
	int nbath;
	int namp;
	int nss;
	char beamflag[MBSYS_SWPLS_MAX_BEAMS];
	double bath[MBSYS_SWPLS_MAX_BEAMS];
	double amp[MBSYS_SWPLS_MAX_BEAMS];
	double bathacrosstrack[MBSYS_SWPLS_MAX_BEAMS];
	double bathalongtrack[MBSYS_SWPLS_MAX_BEAMS];
	double ss[MBSYS_SWPLS_MAX_BEAMS];
	double ssacrosstrack[MBSYS_SWPLS_MAX_BEAMS];
	double ssalongtrack[MBSYS_SWPLS_MAX_BEAMS];
	};


/* System specific function prototypes */
int mbsys_swathplus_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_swathplus_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_swathplus_dimensions(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbath,
	int *namp,
	int *nss,
	int *error);
int mbsys_swathplus_pingnumber(int verbose, void *mbio_ptr, int *pingnumber, int *error);
int mbsys_swathplus_extract(int verbose,
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
int mbsys_swathplus_insert(int verbose,
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
int mbsys_swathplus_ttimes(int verbose,
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
int mbsys_swathplus_detects(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbeams,
	int *detects,
	int *error);
int mbsys_swathplus_pulses(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nbeams,
	int *pulses,
	int *error);
int mbsys_swathplus_gains(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	double *transmit_gain,
	double *pulse_length,
	double *receive_gain,
	int *error);
int mbsys_swathplus_extract_altitude(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	double *transducer_depth,
	double *altitude,
	int *error);
int mbsys_swathplus_extract_nnav(int verbose,
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
int mbsys_swathplus_extract_nav(int verbose,
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
int mbsys_swathplus_insert_nav(int verbose,
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
int mbsys_swathplus_extract_svp(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int *kind,
	int *nsvp,
	double *depth,
	double *velocity,
	int *error);
int mbsys_swathplus_insert_svp(int verbose,
	void *mbio_ptr,
	void *store_ptr,
	int nsvp,
	double *depth,
	double *velocity,
	int *error);
int mbsys_swathplus_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_swathplus_ping_init(int verbose, swplssxp_ping *ping, int *error);
int mbsys_swathplus_print_store(int verbose, struct mbsys_swathplus_struct *store, int *error);
int mbsys_swathplus_print_ping(int verbose, swplssxp_ping *ping, int *error);
int mbsys_swathplus_print_header(int verbose, swplssxp_header *header, int *error);
