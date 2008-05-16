/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_image83p.h	5/5/2008
 *	$Id: mbsys_image83p.h,v 5.0 2008-05-16 22:51:24 caress Exp $
 *
 *    Copyright (c) 2008 by
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
 * mbsys_image83p.h defines the data structures used by MBIO functions
 * to store data from Imagenex DeltaT multibeam sonar systems.
 * The data formats which are commonly used to store Imagenex DeltaT
 * data in files include
 *      MBF_IMAGE83P : MBIO ID 191
 *
 * Author:	Vivek Reddy, Santa Clara University
 * Date:	February 16, 1993
 * 
 * $Log: not supported by cvs2svn $
 *
 */
/*
 * Notes on the MBSYS_HSDS data structure:
 *   1. Imagex multibeam systems output raw data in an
 *      ascii format.  
 *   2. The system outputs 480 beams of bathymetry 
 *
 *   3. The data structure defined below includes all of the values
 *      which are passed in imagenex multibeam records
 */

/* number of beams for imagex multibeam */
#define MBSYS_IMAGE83P_BEAMS 480

struct mbsys_image83p_struct
	{
	/* type of data record */
	int	kind;

	/* time stamp (all records but comment ) */
	int	time_i[7];
	double	time_d;
	
	/* additional navigation and depths  */
	double	nav_lat;
	double	nav_long;
	int	nav_speed; /* 0.1 knots */
	int	nav_heading; /* 0.1 degrees */
	int	pitch; /* degrees / 10 - 900 */
	int	roll; /* degrees / 10 - 900 */
	int	heading /* degrees / 10 */;
	int	num_beams;
	int	samples_per_beam;
	int	sector_size; /* degrees */
	int	start_angle; /* 0.01 degrees + 180.0 */
	int	angle_increment; /* 0.01 degrees */
	int	acoustic_range; /* meters */
	int	acoustic_frequency; /* kHz */
	int	sound_velocity; /* 0.1 m/sec */
	int	range_resolution; /* 0.001 meters */
	int	pulse_length; /* usec */
	int	profile_tilt_angle; /* degrees + 180.0 */
	int	rep_rate; /* msec */
	int	ping_number;
	int	range[MBSYS_IMAGE83P_BEAMS];
	double bath[MBSYS_IMAGE83P_BEAMS];
	double bathacrosstrack[MBSYS_IMAGE83P_BEAMS];
	double bathalongtrack[MBSYS_IMAGE83P_BEAMS];
	char	beamflag[MBSYS_IMAGE83P_BEAMS];
	};
	
/* system specific function prototypes */
int mbsys_image83p_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
		int *error);
int mbsys_image83p_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_image83p_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_image83p_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_image83p_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_image83p_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_image83p_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitudev, 
			int *error);
int mbsys_image83p_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_image83p_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_image83p_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

