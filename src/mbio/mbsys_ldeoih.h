/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_ldeoih.h	3/2/93
 *	$Id$
 *
 *    Copyright (c) 1993-2012 by
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
 * mbsys_ldeoih.h defines the data structure used by MBIO functions
 * to store multibeam data in a general purpose archive format:
 *      MBF_HSLDEOIH : MBIO ID 71
 *
 * Author:	D. W. Caress
 * Date:	March 2, 1993
 *
 * $Log: mbsys_ldeoih.h,v $
 * Revision 5.6  2007/10/08 15:59:34  caress
 * MBIO changes as of 8 October 2007.
 *
 * Revision 5.5  2005/11/05 00:48:04  caress
 * Programs changed to register arrays through mb_register_array() rather than allocating the memory directly with mb_realloc() or mb_malloc().
 *
 * Revision 5.4  2003/04/17 21:05:23  caress
 * Release 5.0.beta30
 *
 * Revision 5.3  2002/04/06 02:43:39  caress
 * Release 5.0.beta16
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
 * Revision 4.5  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.4  1999/03/31  18:11:35  caress
 * MB-System 4.6beta7
 *
 * Revision 4.3  1998/10/05  17:46:15  caress
 * MB-System version 4.6beta
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
 * Revision 4.0  1994/02/17  20:42:20  caress
 * First cut at new version.  Recast format to include both
 * beam amplitude and sidescan data.  I hope noone has used
 * the old version of this format, as the files will be
 * orphaned!!!
 *
 * Revision 3.0  1993/05/14  23:04:50  sohara
 * initial version
 *
 */
/*
 * Notes on the MBSYS_LDEOIH data structure:
 *   1. This data structure is used to store multibeam bathymetry
 *      and/or backscatter data with arbitrary numbers of beams.
 *	This format was created by the Lamont-Doherty Earth
 *	Observatory to serve as a general purpose archive format for
 *	processed multibeam data.
 *   2. The data consist of variable length binary records encoded
 *	entirely in 2-byte integers.
 *   3. Both the depth and backscatter arrays are centered.
 *   4. The kind value in the mbf_sbsiocen_struct indicates whether the
 *      mbf_sbsiocen_data_struct structure holds data (kind = 1) or an
 *      ascii comment record (kind = 0).
 */

/* maximum line length in characters */
#define MBSYS_LDEOIH_MAXLINE 200

struct mbsys_ldeoih_struct
	{
	/* type of data record */
	int	kind;
	
	/* time stamp */
	double	time_d;		/* decimal seconds since start of 1970 */

	/* position */
	double	longitude;	/* longitude (degrees 0-360) */
	double	latitude;	/* latitude (degrees 0-360) */
	
	/* sonar depth and altitude */
	double	sonardepth;	/* meters (sonar depth for bathymetry calculation,
					already corrected for heave if needed,
						sonardepth = transducer_depth
						bath = altitude + sonardepth
						sonardepth = draft - heave
						draft = sonardepth + heave */
	double	altitude;	/* meters */

	/* heading and speed */
	float	heading;	/* heading (degrees 0-360) */
	float	speed;		/* km/hour */
	
	/* attitude */
	float	roll;		/* degrees */
	float	pitch;		/* degrees */
	float	heave;		/* meters */
	
	/* beam widths */
	float	beam_xwidth;	/* degrees */
	float	beam_lwidth;	/* degrees */

	/* numbers of beams */
	short	beams_bath;	/* number of depth values */
	short	beams_amp;	/* number of amplitude values */
	short	pixels_ss;	/* number of sidescan pixels */
	short	spare1;		
	short	beams_bath_alloc;	/* number of depth values allocated */
	short	beams_amp_alloc;	/* number of amplitude values allocated */
	short	pixels_ss_alloc;	/* number of sidescan pixels allocated */
	
	/* scaling */
	float	depth_scale;	/* depth[i] = (bath[i] * depth_scale) + transducer_depth */
	float	distance_scale;	/* acrosstrackdistance[i] = acrosstrack[i] * distance_scale 
					alongtrackdistance[i] = alongtrack[i] * distance_scale */

	/* sidescan type */
	short	ss_type;	/* indicates if sidescan values are logarithmic or linear
					ss_type = 0: logarithmic (dB)
					ss_type = 1: linear (voltage) */
	short	spare2;		

	/* pointers to arrays */
	unsigned char *beamflag;
	short	*bath;
	short	*amp;
	short	*bath_acrosstrack;
	short	*bath_alongtrack;
	short	*ss;
	short	*ss_acrosstrack;
	short	*ss_alongtrack;

	/* comment */
	char	comment[MBSYS_LDEOIH_MAXLINE];
	};

struct mbsys_ldeoih_old_struct
	{
	/* type of data record */
	int	kind;

	/* time stamp */
	short	year;		/* year (4 digits) */
	short	day;		/* julian day (1-366) */
	short	min;		/* minutes from beginning of day (0-1439) */
	short	sec;		/* seconds from beginning of minute (0-59) */
	short	msec;		/* milliseconds from beginning of minute (0-59) */

	/* position */
	unsigned short	lon2u;	/* minutes east of prime meridian */
	unsigned short	lon2b;	/* fraction of minute times 10000 */
	unsigned short	lat2u;	/* number of minutes north of 90S */
	unsigned short	lat2b;	/* fraction of minute times 10000 */

	/* heading and speed */
	unsigned short	heading;/* heading:
					0 = 0 degrees
					1 = 0.0055 degrees
					16384 = 90 degrees
					65535 = 359.9945 degrees
					0 = 360 degrees */
	unsigned short	speed;	/* km/s X100 */

	/* numbers of beams */
	short	beams_bath;	/* number of depth values */
	short	beams_amp;	/* number of amplitude values */
	short	pixels_ss;	/* number of sidescan pixels */
	short	beams_bath_alloc;	/* number of depth values allocated */
	short	beams_amp_alloc;	/* number of amplitude values allocated */
	short	pixels_ss_alloc;	/* number of sidescan pixels allocated */
	
	/* obsolete scaling */
/* 	short	depth_scale;*/	/* 1000 X scale where depth = bath X scale */
/* 	short	distance_scale;*//* 1000 X scale where distance = dist X scale */
/* 	short	transducer_depth;*/ /* scaled by depth_scale */
/* 	short	altitude;*/	/* scaled by depth_scale */
/* 	short	beam_xwidth;*/	/* 0.01 degrees */
/* 	short	beam_lwidth;*/	/* 0.01 degrees */
/* 	short	ss_type;*/	/* indicates if sidescan values are logarithmic or linear
					ss_type = 0: logarithmic (dB)
					ss_type = 1: linear (voltage) */
	
	/* scaling */
	short	depth_scale;	/* 1000*scale where depth = bath*scale + transducer_depth / 1000 */
	short	distance_scale;	/* 1000*scale where distance = dist*scale */
	int	transducer_depth; /* 0.001 m */
	int	altitude;	/* 0.001 m */
	short	beam_xwidth;	/* 0.01 degrees */
	short	beam_lwidth;	/* 0.01 degrees */
	short	ss_type;	/* indicates if sidescan values are logarithmic or linear
					ss_type = 0: logarithmic (dB)
					ss_type = 1: linear (voltage) */

	/* pointers to arrays */
	unsigned char *beamflag;
	short	*bath;
	short	*amp;
	short	*bath_acrosstrack;
	short	*bath_alongtrack;
	short	*ss;
	short	*ss_acrosstrack;
	short	*ss_alongtrack;

	/* comment */
	char	comment[MBSYS_LDEOIH_MAXLINE];
	};
	
/* system specific function prototypes */
int mbsys_ldeoih_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_ldeoih_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_ldeoih_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_ldeoih_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, 
		int *ss_type, int *error);
int mbsys_ldeoih_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_ldeoih_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_ldeoih_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_ldeoih_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_ldeoih_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_ldeoih_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			double transducer_depth, double altitude, 
			int *error);
int mbsys_ldeoih_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_ldeoih_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_ldeoih_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

