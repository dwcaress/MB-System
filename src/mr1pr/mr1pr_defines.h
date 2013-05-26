/*--------------------------------------------------------------------
 *    The MB-system:	mr1pr_defines.h	3/7/2003
 *	$Id$
 *
 *    Copyright (c) 2003 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/* This source code is part of the MR1PR library used to read and write
 * swath sonar data in the MR1PR format devised and used by the 
 * Hawaii Mapping Research Group of the University of Hawaii.
 * This source code was made available by Roger Davis of the
 * University of Hawaii under the GPL. Minor modifications have
 * been made to the version distributed here as part of MB-System.
 *
 * Author:	Roger Davis (primary author)
 * Author:	D. W. Caress (MB-System revisions)
 * Date:	March 7, 2003 (MB-System revisions)
 * $Log: mr1pr_defines.h,v $
 * Revision 5.2  2007/10/08 16:37:00  caress
 * Added cygwin define.
 *
 * Revision 5.1  2006/01/11 07:46:15  caress
 * Working towards 5.0.8
 *
 * Revision 5.0  2003/03/11 19:09:14  caress
 * Initial version.
 *
 *
 *
 *--------------------------------------------------------------------*/
/*
 *	Copyright (c) 1991 by University of Hawaii.
 */

/*
 *	mr1pr_defines.h --
 *	Hawaii MR1 post-processing software definitions.
 */
#ifdef HAVE_CONFIG_H
#  include <mbsystem_config.h>
#endif

#ifndef __MR1PR_DEFINES__
#define __MR1PR_DEFINES__

/* XDR i/o include file */
#ifdef HAVE_RPC_RPC_H
# include <rpc/rpc.h>
#endif
#ifdef HAVE_RPC_TYPES_H
# include <rpc/types.h>
# include <rpc/xdr.h>
#endif

/* Various system dependent defines */
#ifdef SUN
#define Free			(void) free
#define MemType			char
#define MemSizeType		unsigned int
#define MemCopy(m0, m1, n)	bcopy((char *) (m0), (char *) (m1), (int) (n))
#define MemZero(m, n)		bzero((char *) (m), (int) (n))
#else
#define Free			free
#define MemType			void
#define MemSizeType		size_t
#define MemCopy(m0, m1, n)	(void) memmove((void *) (m1), (void *) (m0), (size_t) (n))
#define MemZero(m, n)		(void) memset((void *) (m), (int) 0, (size_t) (n))
#endif

/* MR1 channel defines */
#define ACP_NSIDES		(2)
#define ACP_PORT		(0)
#define ACP_STBD		(1)
#define ACP_UNKNOWN		(2)
#define ACP_BOTH		(3)

/* version number */
#define MR1_VERSION_1_0		(6666)		/* obsolete version */
#define MR1_VERSION_2_0		(6667)		/* current version */

/* MR1File --
   This structure appears at the beginning of all MR1
   post-processing files. It describes the class and
   number of data objects contained within the file. */

typedef struct mf_struct {
	int mf_version;		/* file format version number */
	int mf_count;		/* number of objects */
	char *mf_log;		/* processing log */
} MR1File;

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
	float ps_ssoffset;	/* across-track distance to first sidescan sample */
	int ps_sscount;		/* number of valid sidescan samples */
	int ps_sspad;		/* number of invalid trailing pad samples */
} PingSide;

/* MR1Timeval --
   This structure is defined for the MB-System version
   of this code because MB-System explicitely avoids 
   using standard time structures and functions, thereby
   avoiding the wide variability in time handling amongst
   Unix-like operating systems. */

typedef struct mf_timeval {
	int	tv_sec;		/* seconds */
	int	tv_usec;	/* and microseconds */
} MR1Timeval;

/* Ping --
   This structure describes the single ping of bathymetry
   and sidescan data which follows it in an MR1 sidescan file.
   A file may have any number of such pings. */

typedef struct png_struct {
	MR1Timeval png_tm;	/* timestamp */
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
	Sensor png_pitch;	/* towfish pitch (deg) */
	Sensor png_roll;	/* towfish roll (deg) */
	int png_snspad;		/* number of invalid trailing pad sensor samples */
	float png_temp;		/* water temperature (deg) */
	float png_atssincr;	/* across-track sidescan increment (m) */
	float png_alt;		/* towfish altitude (m) */
	float png_magcorr;	/* magnetic correction (deg) */
	float png_sndvel;	/* sound velocity (m/sec) */
	PingSide png_sides[ACP_NSIDES];
} Ping;

/* SMControl --
   This structure describes a shared memory control
   block used to pass information back and forth between
   cooperating MR1 processing programs. */

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

#define MR1_UNDEFINED		(-1)

/* error codes */
#define MR1_SUCCESS		(0)
#define MR1_FAILURE		(1)
#define MR1_FILTERWAIT		(2)
#define MR1_MISC		(3)
#define MR1_BADARG		(4)
#define MR1_MEMALLOC		(5)
#define MR1_OPEN		(6)
#define MR1_READ		(7)
#define MR1_WRITE		(8)
#define MR1_SYSVIPC		(9)
#define MR1_X11			(10)
#define MR1_SIGNAL		(11)
#define MR1_PIPE		(12)
#define MR1_FCNTL		(13)
#define MR1_FORK		(14)
#define MR1_DUP2		(15)
#define MR1_CHDIR		(16)
#define MR1_EXEC		(17)
#define MR1_PDB			(18)
#define MR1_EOF			(19)
#define MR1_BADDATA		(20)

/* data access mode */
#define MR1_FILEIO		(0)
#define MR1_SHAREDMEM		(1)

/* ping flags */
#define MR1_NULLMARK		(0x0)
#define MR1_LOWMARK		(0x1)
#define MR1_HIGHMARK		(0x2)

/*
   The material within the enclosing '#ifdef __MR1PR_OBSOLETE__' which follows
   is documentation of now-obsolete versions of the MR1 post-processing file format.
*/
#ifdef __MR1PR_OBSOLETE__

#define MR1_VERSION_1_0		(6666)

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
	MR1Timeval png_tm;	/* timestamp */
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

#endif /* __MR1PR_OBSOLETE__ */

#endif /* __MR1PR_DEFINES__ */
