/*--------------------------------------------------------------------
 *    The MB-system:	mbf_mr1prhig.h	3/3/94
 *	$Id: mbf_mr1prhig.h,v 4.0 1994-10-21 12:35:05 caress Exp $
 *
 *    Copyright (c) 1994 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbf_mr1prhig.h defines the data structures used by MBIO functions
 * to store multibeam data read from the MBF_MR1PRHIG format (MBIO id 61).  
 *
 * Author:	D. W. Caress
 * Date:	July 15, 1994
 * $Log: not supported by cvs2svn $
 * Revision 1.2  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 1.1  1994/07/29  18:46:51  caress
 * Initial revision
 *
 *
 *
 */
/*
 * Notes on the MBF_MR1PRHIG data format:
 *   1. The MR1 post processing format uses the xdr external
 *      data representation for portability.
 *   2. The data stream consists of a file header followed
 *      by individual pings.
 *   3. The file header contains a comment string and the
 *      number of pings. The comment string is broken up
 *      into multiple comments by MBIO on reading; the comments
 *      are concatenated into a single string on writing.
 *   4. The pings each contain a header plus the bathymetry and/or
 *      sidescan data.
 *   6. The data structure defined below includes all of the values
 *      which are passed in the MR1 post processing format.
 */

/* maximum number of bathymetry beams per side for MR1 */
#define MBF_MR1PRHIG_BEAMS_SIDE 100

/* maximum number of sidescan pixels per side for MR1 */
#define MBF_MR1PRHIG_PIXELS_SIDE 2000

/* maximum number of bathymetry beams for MR1 */
#define MBF_MR1PRHIG_BEAMS (2*MBF_MR1PRHIG_BEAMS_SIDE + 3)

/* maximum number of sidescan pixels output for MR1 */
#define MBF_MR1PRHIG_PIXELS (2*MBF_MR1PRHIG_PIXELS_SIDE + 3)

/* maximum length of comment */
#define	MBF_MR1PRHIG_MAXLINE 200

struct mbf_mr1prhig_struct
	{
	/* type of data record */
	int	kind;

	/* file header info */
	int mf_magic;		/* magic cookie */
	int mf_count;		/* number of objects */
	char *mf_log;		/* processing log */

	/* ping header */
	int sec;		/* timestamp */
	int usec;		/* timestamp */
	double png_lon;		/* longitude (deg) */
	double png_lat;		/* latitude (deg) */
	float png_course;	/* course determined from nav (deg) */
	float png_compass;	/* compass heading of vehicle 
					0=N,90=E, etc. (deg) */
	float png_prdepth;	/* pressure depth (m) */
	float png_alt;		/* altitude of vehicle (m) */
	float png_pitch;	/* vehicle pitch (deg) */
	float png_roll;		/* vehicle roll (deg) */
	float png_temp;		/* water temperature (deg) */
	float png_atssincr;	/* across-track sidescan increment (m) */

	/* port settings */
	float port_trans[2];	/* transmitter settings (units?) */
	float port_gain;	/* gain setting (units?) */
	float port_pulse;	/* pulse length (units?) */
	int port_btycount;	/* number of valid bathymetry samples */
	int port_btypad;	/* number of invalid trailing pad samples */
	float port_ssoffset;	/* across-track distance to 
					first sidescan sample */
	int port_sscount;	/* number of valid sidescan samples */
	int port_sspad;		/* number of invalid trailing pad samples */

	/* starboard settings */
	float stbd_trans[2];	/* transmitter settings (units?) */
	float stbd_gain;	/* gain setting (units?) */
	float stbd_pulse;	/* pulse length (units?) */
	int stbd_btycount;	/* number of valid bathymetry samples */
	int stbd_btypad;	/* number of invalid trailing pad samples */
	float stbd_ssoffset;	/* across-track distance to 
					first sidescan sample */
	int stbd_sscount;	/* number of valid sidescan samples */
	int stbd_sspad;		/* number of invalid trailing pad samples */

	/* bathymetry */
	float	bath_acrosstrack_port[MBF_MR1PRHIG_BEAMS_SIDE];
	float	bath_port[MBF_MR1PRHIG_BEAMS_SIDE];
	float	bath_acrosstrack_stbd[MBF_MR1PRHIG_BEAMS_SIDE];
	float	bath_stbd[MBF_MR1PRHIG_BEAMS_SIDE];

	/* sidescan */
	float	ss_port[MBF_MR1PRHIG_PIXELS_SIDE];
	float	ss_stbd[MBF_MR1PRHIG_PIXELS_SIDE];

	/* comment */
	char	comment[MBF_MR1PRHIG_MAXLINE];
};

