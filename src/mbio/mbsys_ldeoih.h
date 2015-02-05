/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_ldeoih.h	3/2/93
 *	$Id$
 *
 *    Copyright (c) 1993-2015 by
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
	int	beams_bath;	/* number of depth values */
	int	beams_amp;	/* number of amplitude values */
	int	pixels_ss;	/* number of sidescan pixels */
	int	spare1;
	int	beams_bath_alloc;	/* number of depth values allocated */
	int	beams_amp_alloc;	/* number of amplitude values allocated */
	int	pixels_ss_alloc;	/* number of sidescan pixels allocated */

	/* scaling */
	float	depth_scale;	/* depth[i] = (bath[i] * depth_scale) + transducer_depth */
	float	distance_scale;	/* acrosstrackdistance[i] = acrosstrack[i] * distance_scale
					alongtrackdistance[i] = alongtrack[i] * distance_scale */

	/* data type parameters */
        mb_s_char       ss_scalepower;  /* gives scaling factor for sidescan values in powers of 10:
                                                ss_scalepower = 0: ss = ss_stored * 1
                                                ss_scalepower = 1: ss = ss_stored * 10
                                                ss_scalepower = 2: ss = ss_stored * 100
                                                ss_scalepower = 3: ss = ss_stored * 1000 */
	mb_u_char	ss_type;	/* indicates if sidescan values are logarithmic or linear
                                                ss_type = 0: logarithmic (dB)
                                                ss_type = 1: linear (voltage) */
	mb_u_char	imagery_type;   /* MBIO imagery source types defined in mb_status.h
                                                MB_IMAGERY_TYPE_UNKNOWN		        0
                                                MB_IMAGERY_TYPE_ECHOSOUNDER	        1
                                                MB_IMAGERY_TYPE_MULTIBEAM		2
                                                MB_IMAGERY_TYPE_SIDESCAN		3
                                                MB_IMAGERY_TYPE_INTERFEROMETRIC	        4
                                                MB_IMAGERY_TYPE_LIDAR      	        5
                                                MB_IMAGERY_TYPE_CAMERA     	        6
                                                MB_IMAGERY_TYPE_GRID     	        7
                                                MB_IMAGERY_TYPE_POINT       	        8 */

	mb_u_char	topo_type;      /* MBIO topography source types
                                                MB_TOPOGRAPHY_TYPE_UNKNOWN		0
                                                MB_TOPOGRAPHY_TYPE_ECHOSOUNDER	        1
                                                MB_TOPOGRAPHY_TYPE_MULTIBEAM		2
                                                MB_TOPOGRAPHY_TYPE_SIDESCAN		3
                                                MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC	4
                                                MB_TOPOGRAPHY_TYPE_LIDAR      	        5
                                                MB_TOPOGRAPHY_TYPE_CAMERA     	        6
                                                MB_TOPOGRAPHY_TYPE_GRID     	        7
                                                MB_TOPOGRAPHY_TYPE_POINT       	        8 */

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
int mbsys_ldeoih_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
		int *sonartype, int *error);
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
