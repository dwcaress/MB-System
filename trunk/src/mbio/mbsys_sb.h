/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_sb.h	2/17/93
 *	$Id$
 *
 *    Copyright (c) 1993-2009 by
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
 * mbsys_sb.h defines the data structures used by MBIO functions
 * to store data from the 16-beam SeaBeam multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam
 * data in files include
 *      MBF_SBSIOMRG : MBIO ID 11
 *      MBF_SBSIOCEN : MBIO ID 12
 *      MBF_SBSIOLSI : MBIO ID 13
 *      MBF_SBURICEN : MBIO ID 14
 *
 * Author:	D. W. Caress
 * Date:	February 17, 1993
 * $Log: mbsys_sb.h,v $
 * Revision 5.6  2009/03/08 09:21:00  caress
 * Fixed problem reading and writing format 16 (MBF_SBSIOSWB) data on little endian systems.
 *
 * Revision 5.5  2005/11/05 00:48:05  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/09/18 23:32:59  caress
 * Release 5.0.beta23
 *
 * Revision 5.2  2001/07/20 00:32:54  caress
 * Release 5.0.beta03
 *
 * Revision 5.1  2001/01/22  07:43:34  caress
 * Version 5.0.beta01
 *
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.4  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.3  1998/10/05  18:32:27  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.2  1997/04/21  17:02:07  caress
 * MB-System 4.5 Beta Release.
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.1  1994/10/21  12:20:01  caress
 * Release V4.0
 *
 * Revision 4.0  1994/03/06  00:01:56  caress
 * First cut at version 4.0
 *
 * Revision 4.1  1994/03/03  03:39:43  caress
 * Fixed copyright message.
 *
 * Revision 4.0  1994/02/17  20:46:50  caress
 * First cut at new version.  Comment changes only.
 *
 * Revision 3.0  1993/05/14  23:05:32  sohara
 * initial version
 *
 */
/*
 * Notes on the MBSYS_SB data structure:
 *   1. SeaBeam multibeam systems output raw data in 16 uncentered
 *      beams.  MBIO and most data formats store the data as 19
 *      centered beams.
 *   5. The kind value in the mbsys_sb_struct indicates whether the
 *      mbsys_sb_data_struct structure holds data from a ping or
 *      data from a comment:
 *        kind = 1 : data from a ping 
 *        kind = 2 : comment 
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam records.
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
	int	dist[MBSYS_SB_BEAMS]; /* 19 depths from SeaBeam in meters
					assuming 1500 m/s water velocity */
	int	deph[MBSYS_SB_BEAMS]; /* 19 cross track distances in 
					meters from port (negative) 
					to starboard (positive) */

	/* additional values */
	unsigned short	sbtim;	/* SeaBeam computer clock time in 10ths of
					seconds from start of hour (0-3600) */
	unsigned short	sbhdg;	/* SeaBeam gyro heading 
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
	
/* system specific function prototypes */
int mbsys_sb_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_sb_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_sb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_sb_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_sb_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_sb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_sb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_sb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_sb_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

