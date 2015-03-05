/*--------------------------------------------------------------------
 *    The MB-system:	mbbs_defines.h	3/3/2014
 *	$Id$
 *
 *    Copyright (c) 2014-2015 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the mbbsio library used to read and write
 * swath sonar data in the bsio format devised and used by the
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 3, 2014 (MB-System revisions)
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 2004 by University of Hawaii.
 */

/*
 *	bs_defines.h --
 *	Hawaii Mapping Research Group BS (bathymetry/sidescan)
 *	file format definitions.
 */

#ifndef __MBBS_DEFINES__
#define __MBBS_DEFINES__

#include <time.h>
#include <sys/time.h>

/* The preprocessor code inserted here to insure access to
 * XDR definitions is changed from the HMRG codebase */

#ifdef HAVE_CONFIG_H
#include <mb_config.h>

/* XDR i/o include file */
#ifdef HAVE_RPC_RPC_H
# include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_TYPES_H
# include <rpc/types.h>
# include <rpc/xdr.h>
#endif

#else /* no HAVE_CONFIG_H */

/* XDR i/o include file */
#ifdef IRIX
#include <rpc/rpc.h>
#endif
#ifdef IRIX64
#include <rpc/rpc.h>
#endif
#ifdef SOLARIS
#include <rpc/rpc.h>
#endif
#ifdef LINUX
#include <rpc/rpc.h>
#endif
#ifdef LYNX
#include <rpc/rpc.h>
#endif
#ifdef SUN
#include <rpc/xdr.h>
#endif
#ifdef HPUX
#include <rpc/rpc.h>
#endif
#ifdef DARWIN
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif
#ifdef CYGWIN
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif
#ifdef OTHER
#include <rpc/types.h>
#include <rpc/xdr.h>
#endif

#endif /* HAVE_CONFIG_H */

/* Some type definitions given here are in a separate
 * header file unixversion.h in the original HMRG codebase */
#define CFree			free
#define MemType			void
#define MemSizeType		size_t
#define StrSizeType		size_t
#define MemCopy(m0, m1, n)	(void) memmove((void *) (m1), (void *) (m0), (size_t) (n))
#define MemZero(m, n)		(void) memset((void *) (m), (int) 0, (size_t) (n))


/* The ACPSIDES definitions are in a separate header file
 * acpsides.h in the original HMRG codebase */
#ifndef __ACPSIDES__
#define __ACPSIDES__

/* do not change these **EVER**!! */
#define ACP_PORT		(0)
#define ACP_STBD		(1)
#define ACP_NSIDES		(2)

#define ACP_UNKNOWN		(ACP_STBD+1)
#define ACP_BOTH		(ACP_STBD+2)
#define ACP_NONE		(ACP_STBD+3)

#endif /* __ACPSIDES__ */

/* version number, guaranteed to be strictly
   increasing and in chronological order */
#define MR1_VERSION_1_0		(6666)		/* obsolete */
#define MR1_VERSION_2_0		(6667)		/* obsolete */
#define BS_VERSION_1_0		(6668)		/* obsolete as of 2007/06/28 */
#define BS_VERSION_1_1		(6669)		/* obsolete as of 2007/11/27 */
#define BS_VERSION_1_2		(6670)		/* obsolete as of 2008/04/14 */
#define BS_VERSION_1_3		(6671)		/* obsolete as of 2010/03/10 */
#define BS_VERSION_1_4		(6672)		/* current version */
/* always update the following definition
   when a new version is added! */
#define BS_VERSION_CURR		(BS_VERSION_1_4)

/* file flag bits */
#define BS_CLEAR		(0x0)
#define BS_SSSLANTRNG		(0x1)		/* sidescan are slant range */
#define BS_MSCPINGDELRST	(0x2)		/* ping delete/restore via							   mosaic GUI */
#define BS_MSCNAVEDIT		(0x4)		/* navigation edits via
						   mosaic GUI */
#define BS_MSCBRKFILE		(0x8)		/* file break via mosaic GUI */
#define BS_MSCEDGETRIM		(0x10)		/* edge trims via mosaic GUI */

/* acquisition instruments */
#define BS_INST_UNDEFINED	(-1)
#define BS_INST_MR1		(0)
#define BS_INST_SEAMAPB		(1)
#define BS_INST_IMI30		(2)
#define BS_INST_IMI12		(3)
#define BS_INST_DSL120A		(4)
#define BS_INST_SEAMAPC		(100)
#define BS_INST_SCAMP		(150)
#define BS_INST_EM120		(2000)
#define BS_INST_EM1002		(2001)
#define BS_INST_EM300		(2002)
#define BS_INST_EM3000		(2003)
#define BS_INST_EM3002		(2004)
#define BS_INST_EM3000D		(2005)
#define BS_INST_EM3002D		(2006)
#define BS_INST_EM2000		(2007)
#define BS_INST_EM122		(2008)
#define BS_INST_EM302		(2009)
#define BS_INST_EM710		(2010)
#define BS_INST_SM2000		(2050)
#define BS_INST_RESON8101	(3000)
#define BS_INST_RESON8111	(3001)
#define BS_INST_RESON8124	(3002)
#define BS_INST_RESON8125	(3003)
#define BS_INST_RESON8150	(3004)
#define BS_INST_RESON8160	(3005)
#define BS_INST_AMS120		(4000)
#define BS_INST_REMUS		(4100)
#define BS_INST_KLEIN5000	(5000)
#define BS_INST_SEABEAM2000	(6000)
#define BS_INST_SEABEAM2100	(6010)
#define BS_INST_SEABEAM3012	(6050)
#define BS_INST_SSI		(7000)
#define BS_INST_SAICLLS		(8000)
#define BS_INST_EDGETECHSS	(9000)
#define BS_INST_EDGETECHSSM	(9001)
#define BS_INST_EDGETECHSSH	(9002)
#define BS_INST_EDGETECHSB	(9003)

/* source file formats */
#define BS_SFMT_UNDEFINED	(-1)
#define BS_SFMT_MR1		(0)
#define BS_SFMT_TTS		(1)
#define BS_SFMT_GSF		(1000)
#define BS_SFMT_GSFDUAL		(1001)
#define BS_SFMT_XTF		(1100)
#define BS_SFMT_SIMRADEM	(2000)
#define BS_SFMT_SIMRADMPB	(2001)
#define BS_SFMT_OIC		(4000)
#define BS_SFMT_OICLLS		(4001)
#define BS_SFMT_MSTIFF		(4100)
#define BS_SFMT_SIOSB2000	(6000)
#define BS_SFMT_SSIV21		(7000)
#define BS_SFMT_XSE		(8000)
#define BS_SFMT_JSF		(9000)

/* data type mask bits */
#define BS_DTM_NONE		(0)
#define BS_DTM_COMPASS		(0x1)
#define BS_DTM_DEPTH		(0x2)
#define BS_DTM_PITCH		(0x4)
#define BS_DTM_ROLL		(0x8)
#define BS_DTM_BATHYMETRY	(0x10)
#define BS_DTM_SIDESCAN		(0x20)

/* BSFile --
   This structure appears at the beginning of all BS
   files. It describes the format version and
   number of data objects contained within the file. */

typedef struct bsf_struct {
	int bsf_version;	/* file format version number */
	int bsf_count;		/* number of objects */
	unsigned int bsf_flags;	/* BS_SSSLANTRNG, etc. */
	int bsf_inst;		/* acquisition instrument */
	int bsf_srcformat;	/* source file format */
	char *bsf_srcfilenm;	/* source file name */
	char *bsf_log;		/* processing log */
} BSFile;

/* Sensor --
   This structure describes the sample interval, number
   of samples and the samples themselves for a single
   sensor (e.g., roll). Unknown sample values are indicated
   by NaN. */

typedef struct sns_struct {
	float sns_int;		/* sample interval (secs) */
	int sns_nsamps;		/* number of samples */
	float sns_repval;	/* single representative value of the sensor
				   for an entire ping, usually derived from
				   the full set of samples for that ping */
} Sensor;

/* PingSide --
   This structure describes either the
   port or starboard side of a single ping. */

typedef struct ps_struct {
	float ps_xmitpwr;	/* transmitter power (1=full) */
	float ps_gain;		/* gain setting (units?) */
	float ps_pulse;		/* pulse length (millisecs) */
	float ps_bdrange;	/* bottom detect range (m) */
	int ps_btycount;	/* number of valid bathymetry samples */
	int ps_btypad;		/* number of invalid trailing pad samples */
	float ps_ssxoffset;	/* across-track distance (m) or, for
				   BS_SSSLANTRNG files, time (s) to first
				   sidescan sample */
	int ps_sscount;		/* number of valid sidescan samples */
	int ps_sspad;		/* number of invalid trailing pad samples */
	float ps_ssndrmask;	/* across-track distance to outer edge
				   of nadir region data to be masked */
	float ps_ssyoffset;	/* sidescan along-track offset (m) */
} PingSide;

/* Ping --
   This structure describes the single ping of bathymetry
   and sidescan data which follows it in a BS sidescan file.
   A file may have any number of such pings. */

#define PNG_CLEAR		(0x0)
#define PNG_XYZ			(0x1)	/* bathymetry is x/y/z instead
					   of x/z only */
#define PNG_ABI			(0x2)	/* auxiliary beam info present */
#define PNG_BTYSSFLAGSABSENT	(0x4)	/* indicates that input file does
					   not contain bathymetry or
					   sidescan flags, i.e., the file
					   is in an older flagless format
					   version; all output files are
					   written with flags and this bit
					   is always unset when written
					   to output */
#define PNG_HIDE		(0x8)	/* ping should not be displayed */
#define PNG_LOWQUALITY		(0x10)	/* ping is of unacceptably low quality */
#define PNG_MSCHIDE		(0x20)	/* ping should not be displayed
					   in a mosaic */

/* sidescan along-track offset mode */
#define PNG_SSYOM_UNKNOWN	(0)	/* unknown (all pre-BS-1.4 files) */
#define PNG_SSYOM_CONSTANT	(1)	/* constant offset for entire ping */
#define PNG_SSYOM_USEBTYY	(2)	/* use bathymetry y-offsets */

#define PNG_BYTEALIGNSZ		(8)	/* byte alignment constraint for
					   beginning of auxiliary beam
					   info section of data buffer */

typedef struct png_struct {
	unsigned int png_flags;	/* PNG_XYZ, etc. */
	struct timeval png_tm;	/* timestamp */
	float png_period;	/* ping period (secs) */
	double png_slon;	/* ship longitude (deg) */
	double png_slat;	/* ship latitude (deg) */
	float png_scourse;	/* ship course (deg) */
	float png_laybackrng;	/* towfish layback range (m) */
	float png_laybackbrg;	/* towfish layback bearing (deg, where 0=shipaxis,
				   pos=port, neg=starboard) */
	double png_tlon;	/* towfish longitude (deg) */
	double png_tlat;	/* towfish latitude (deg) */
	float png_tcourse;	/* towfish course (deg) */
	Sensor png_compass;	/* towfish compass heading (deg, where 0=N, 90=E),
				   with no magnetic correction applied to either
				   the representative value or the sample array */
	Sensor png_depth;	/* towfish depth (m) */
	Sensor png_pitch;	/* towfish pitch (deg, where + is nose up) */
	Sensor png_roll;	/* towfish roll (deg, where + is port down) */
	int png_snspad;		/* number of invalid trailing pad sensor samples */
	float png_temp;		/* water temperature (deg C) */
	float png_ssincr;	/* sidescan increment in across-track distance (m)
				   or, for BS_SSSLANTRNG files, time (s) */
	int png_ssyoffsetmode;	/* sidescan along-track offset mode */
	float png_alt;		/* towfish altitude (m) */
	float png_magcorr;	/* magnetic correction (deg) */
	float png_sndvel;	/* sound velocity (m/sec) */
	float png_cond;		/* conductivity (siemens/m) */
	float png_magx;		/* magnetic field x (microteslas) */
	float png_magy;		/* magnetic field y (microteslas) */
	float png_magz;		/* magnetic field z (microteslas) */
	PingSide png_sides[ACP_NSIDES];
} Ping;

/* bathymetry per-sample flag bits
   (must fit in a 4-byte integer) */
#define BTYD_CLEAR		(0x0)	/* no flag -- sample is valid */
#define BTYD_MISC		(0x1)	/* invalidated for non-specific reason */
#define BTYD_EXTERNAL		(0x2)	/* invalidated by external (non-HMRG) software */
#define BTYD_MINMAXCLIP		(0x4)	/* deleted by min/max depth clip */
#define BTYD_MAXANGLE		(0x8)	/* exceeds maximum angle */
#define BTYD_MINANGLE		(0x10)	/* less than minimum angle */
#define BTYD_SWEDGE		(0x20)	/* deleted from edge of swath */
#define BTYD_SWRECT		(0x40)	/* deleted swath rectangle interior */
#define BTYD_MFSWAPERR		(0x80)	/* mapping function swap error */
#define BTYD_SRFABOVECLIP	(0x100)	/* clipped from above a surface */
#define BTYD_SRFBELOWCLIP	(0x200)	/* clipped from below a surface */
#define BTYD_XZPRECT		(0x400)	/* deleted xz-profile rectangle interior */

/* sidescan per-sample flag bits (must
   fit in a 1-byte unsigned char) */
#define SSD_CLEAR		(0x0)	/* no flag -- sample is valid */
#define SSD_MISC		(0x1)	/* invalidated for non-specific reason */
#define SSD_EXTERNAL		(0x2)	/* invalidated by external (non-HMRG) software */
#define SSD_MAXANGLE		(0x4)	/* exceeds maximum angle */
#define SSD_MINANGLE		(0x8)	/* less than minimum angle */
#define SSD_SWEDGE		(0x10)	/* deleted from edge of swath */
#define SSD_SWRECT		(0x20)	/* deleted swath rectangle interior */

/* AuxBeamInfo --
   This structure contains various bits of per-beam information
   necessary to reconvert back to a source multibeam format. */

/* auxiliary beam information flag bits */
#define ABI_CLEAR		(0x0)
#define ABI_SSVALID		(0x1)	/* abi_ssat{0,1} distances valid */

typedef struct abi_struct {
	unsigned int abi_flags;	/* ABI_SSVALID, etc. */
	int abi_id;		/* beam number */
	float abi_ssat0;	/* across-track distance of first sidescan sample */
	float abi_ssat1;	/* across-track distance of last sidescan sample */
} AuxBeamInfo;

/* PingData --
   This structure contains pointers to sections of a data buffer holding
   all of the ping's samples, i.e., sensors, bathymetry, bathymetry flags,
   sidescan, sidescan flags and auxiliary beam information.
   Samples are stored in the buffer in the following order:

	compass
	depth
	pitch
	roll
	port bathymetry
	port bathymetry flags
	port sidescan
	port sidescan flags
	starboard bathymetry
	starboard bathymetry flags
	starboard sidescan
	starboard sidescan flags
	port auxiliary beam information
	starboard auxiliary beam information

   The sections containing the port bathymetry, starboard bathymetry,
   and auxiliary beam information must start on a PNG_BYTEALIGNSZ byte
   boundary. Note that when bathymetry sample padding is in effect for
   a particular side, that padding must be present after each of the
   bathymetry, bathymetry flags and auxiliary beam information sections
   of that side. When sidescan sample padding is in effect for a particular
   side, that padding must be present after each of the sidescan and
   sidescan flags sections of that side. */

typedef struct pd_struct {
	float *pd_compass;
	float *pd_depth;
	float *pd_pitch;
	float *pd_roll;
	float *pd_bty[ACP_NSIDES];
	unsigned int *pd_btyflags[ACP_NSIDES];
	float *pd_ss[ACP_NSIDES];
	unsigned char *pd_ssflags[ACP_NSIDES];
	AuxBeamInfo *pd_abi[ACP_NSIDES];
} PingData;

/* SMControl --
   This structure describes a shared memory control
   block used to pass information back and forth between
   cooperating BS processing programs. */

#define SMC_RDRNONE		(0x0)
#define SMC_RDRDATA		(0x1)
#define SMC_RDRMARKS		(0x2)
#define SMC_MSGSTDPCT		(0)
#define SMC_MSGALTPCT		(1)
#define SMC_MSGOTHER		(2)
#define SMC_MAXMSG		(40)

typedef struct sm_struct {
	int sm_shmiid;		/* shared memory ID of ping offsets */
	int sm_shmmid;		/* shared memory ID of ping marks */
	int sm_shmdid;		/* shared memory ID of actual data block */
	int sm_count;		/* total number of objects in data block */
	int sm_slantrng;	/* non-zero if data are slant range */
	int sm_ping;		/* number of last processed ping */
	int sm_status;		/* IPC status flag */
	int sm_redraw;		/* data and ping mark redraw flag */
	int sm_msgtype;		/* message format identifier */
	char sm_msg[SMC_MAXMSG+1];
} SMControl;

/* time string parser definitions */
#define TM_JULIAN		(0)
#define TM_CALENDAR		(1)
#define TM_MAXSTRLEN		(120)

#define TM_TZ_UNKNOWN		(0)
#define TM_TZ_GMT		(1)

#define BS_UNDEFINED		(-1)

/* error codes */
#define BS_SUCCESS		(0)
#define BS_FAILURE		(1)
#define BS_FILTERWAIT		(2)
#define BS_MISC			(3)
#define BS_BADARG		(4)
#define BS_MEMALLOC		(5)
#define BS_OPEN			(6)
#define BS_READ			(7)
#define BS_WRITE		(8)
#define BS_SYSVIPC		(9)
#define BS_X11			(10)
#define BS_SIGNAL		(11)
#define BS_PIPE			(12)
#define BS_FCNTL		(13)
#define BS_FORK			(14)
#define BS_DUP2			(15)
#define BS_CHDIR		(16)
#define BS_EXEC			(17)
#define BS_PDB			(18)
#define BS_EOF			(19)
#define BS_BADDATA		(20)
#define BS_FSEEK		(21)
#define BS_ACCESS		(22)
#define BS_RENAME		(23)
#define BS_BADARCH		(24)
#define BS_HUGEPING		(25)
#define BS_GTK			(26)
#define BS_CAIRO		(27)

/* data access mode */
#define BS_FILEIO		(0)
#define BS_SHAREDMEM		(1)

/* ping flags */
#define BS_NULLMARK		(0x0)
#define BS_LOWMARK		(0x1)
#define BS_HIGHMARK		(0x2)

#define BS_MAXSIGNEDINT32	(2147483647)

/* the following restrictions on per-ping data sizes are
   enforced only when sizeof(unsigned long long) is less
   than 8 bytes and we cannot easily determine an actual
   total ping size without risking integer overflow; change
   these values only with great caution, making sure that
   the largest aggregate ping buffer size does not
   exceed BS_MAXSIGNEDINT32 */
#define BS_MAXATTSAMPS		(10000000)
#define BS_MAXBTYSAMPS		(10000000)
#define BS_MAXSSSAMPS		(100000000)

/*
   The material within the enclosing '#ifdef __BS_OBSOLETE__'
   which follows is documentation of now-obsolete versions of the
   post-processing file format.
*/
#ifdef __BS_V_1_0_OBSOLETE__

#if defined(__MR1_V_2_0__)

typedef struct mf_struct {
	int mf_version;		/* MR1_VERSION_2_0 */
	int mf_count;		/* number of objects */
	char *mf_log;		/* processing log */
} MR1File;

#else
#if defined(__MR1_V_1_0__)

typedef struct mf_struct {
	int mf_version;		/* MR1_VERSION_1_0 */
	int mf_count;		/* number of objects */
	char *mf_log;		/* processing log */
} MR1File;

typedef struct ps_struct {
	float ps_trans[2];	/* transmitter settings (units?) */
	float ps_gain;		/* gain setting (units?) */
	float ps_pulse;		/* pulse length (units?) */
	int ps_btycount;	/* number of valid bathymetry samples */
	int ps_btypad;		/* number of invalid trailing pad samples */
	float ps_ssoffset;	/* across-track distance to first sidescan sample */
	int ps_sscount;		/* number of valid sidescan samples */
	int ps_sspad;		/* number of invalid trailing pad samples */
} PingSide;

typedef struct png_struct {
	struct timeval png_tm;	/* timestamp */
	double png_lon;		/* longitude (deg) */
	double png_lat;		/* latitude (deg) */
	float png_course;	/* course determined from lats and longs (deg) */
	float png_compass;	/* compass heading of vehicle 0=N,90=E, etc. (deg) */
	float png_prdepth;	/* pressure depth (m) */
	float png_alt;		/* altitude of vehicle (m) */
	float png_pitch;	/* vehicle pitch (deg) */
	float png_roll;		/* vehicle roll (deg) */
	float png_temp;		/* water temperature (deg) */
	float png_atssincr;	/* across-track sidescan increment (m) */
	PingSide png_port;
	PingSide png_stbd;
} Ping;

#endif /* defined(__MR1_V_1_0__) */

#endif /* defined(__MR1_V_2_0__) */

#endif /* __BS_V_1_0_OBSOLETE__ */

#endif /* __MBBS_DEFINES__ */
