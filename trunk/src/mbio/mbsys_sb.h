/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb.h	3.00	2/17/93
 *	$Id: mbsys_sb.h,v 3.0 1993-05-14 23:05:32 sohara Exp $
 *
 *    Copyright (c) 1993 by 
 *    D. W. Caress (caress@lamont.ldgo.columbia.edu)
 *    and D. N. Chayes (dale@lamont.ldgo.columbia.edu)
 *    Lamont-Doherty Earth Observatory
 *    Palisades, NY  10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_sb.h defines the data structures used by MBIO functions
 * to store data from the 16-beam Sea Beam multibeam sonar systems.
 * The data formats which are commonly used to store Sea Beam
 * data in files include
 *      MBF_SBSIOMRG : MBIO ID 1
 *      MBF_SBSIOCEN : MBIO ID 2
 *      MBF_SBSIOLSI : MBIO ID 3
 *      MBF_SBURICEN : MBIO ID 4
 *
 * Author:	D. W. Caress
 * Date:	February 17, 1993
 * $Log: not supported by cvs2svn $
 */
/*
 * Notes on the MBSYS_SB data structure:
 *   1. Sea Beam multibeam systems output raw data in 16 uncentered
 *      beams.  MBIO and most data formats store the data as 19
 *      centered beams.
 *   5. The kind value in the mbsys_sb_struct indicates whether the
 *      mbsys_sb_data_struct structure holds data from a ping or
 *      data from a comment:
 *        kind = 1 : data from a ping 
 *        kind = 2 : comment 
 *   6. The data structure defined below includes all of the values
 *      which are passed in Sea Beam records.
 */

/* maximum line length in characters */
#define MBSYS_SB_MAXLINE 200

/* number of beams for hydrosweep */
#define MBSYS_SB_BEAMS 19

struct mbsys_sb_struct
	{
	/* type of data record */
	int	kind;

	/* position */
	short	lon2u;		/* minutes east of prime meridian */
	short	lon2b;		/* fraction of minute times 10000 */
	short	lat2u;		/* number of minutes north of 90S */
	short	lat2b;		/* fraction of minute times 10000 */

	/* time stamp */
	int	year;		/* year (4 digits) */
	int	day;		/* julian day (1-366) */
	int	min;		/* minutes from beginning of day (0-1439) */
	int	sec;		/* seconds from beginning of minute (0-59) */

	/* depths and distances */
	int	dist[MBSYS_SB_BEAMS]; /* 19 depths from Sea Beam in meters
					assuming 1500 m/s water velocity */
	int	deph[MBSYS_SB_BEAMS]; /* 19 cross track distances in 
					meters from port (negative) 
					to starboard (positive) */

	/* additional values */
	unsigned short	sbtim;	/* Sea Beam computer clock time in 10ths of
					seconds from start of hour (0-3600) */
	unsigned short	sbhdg;	/* Sea Beam gyro heading 
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	short	axis;		/* navigation error ellipse major axis angle */
	short	major;		/* navigation error ellipse major axis */
	short	minor;		/* navigation error ellipse minor axis */

	/* comment */
	char	comment[MBSYS_SB_MAXLINE];
	};	
