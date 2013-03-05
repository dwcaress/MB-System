/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_wassp.h	1/25/2011
 *	$Id: mbsys_wassp.h 1770 2009-10-19 17:16:39Z caress $
 *
 *    Copyright (c) 2011-2012 by
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
 * mbsys_wassp.h defines the data structures used by MBIO functions
 * to store data from the 16-beam SeaBeam multibeam sonar systems.
 * The data formats which are commonly used to store SeaBeam
 * data in files include
 *      MBF_SBSIOMRG : MBIO ID 11
 *      MBF_SBSIOCEN : MBIO ID 12
 *      MBF_SBSIOLSI : MBIO ID 13
 *      MBF_SBURICEN : MBIO ID 14
 *
 * Author:	D. W. Caress
 * Date:	January 25, 2011
 * Location:    Lugos Towing and Auto Repair.....
 *
 * $Log: mbsys_wassp.h,v $
 *
 */
/*
 * Notes on the MBSYS_WASSP data structure:
 *   1. SeaBeam multibeam systems output raw data in 16 uncentered
 *      beams.  MBIO and most data formats store the data as 19
 *      centered beams.
 *   5. The kind value in the mbsys_wassp_struct indicates whether the
 *      mbsys_wassp_data_struct structure holds data from a ping or
 *      data from a comment:
 *        kind = 1 : data from a ping 
 *        kind = 2 : comment 
 *   6. The data structure defined below includes all of the values
 *      which are passed in SeaBeam records.
 */

/* maximum line length in characters */
#define MBSYS_WASSP_MAXLINE 200

/* number of beams for hydrosweep */
#define MBSYS_WASSP_BEAMS 121

struct mbsys_wassp_struct
	{
	/* type of data record */
	int	kind;
	
	/* message header */
	unsigned int	sync;	/* Hex value should be 0xFF7F7FFF */
	unsigned int	msg_size;	/* Total size for message including sync */
	unsigned int	version;	/* 4 */
	unsigned int	num_points;	/* Number of valid detections for the ping (variable) */
	float		lat_deg;	/* Latitude at transducer (degrees) */
	float		lat_min;	/* Longitude at transducer (minutes) */
	float		lon_deg;	/* Latitude at transducer (degrees) */
	float		lon_min;	/* Latitude at transducer (minutes) */
	float		bearing;	/* Bearing/heading of vessel at transmit (degrees) */
	float		pitch;		/* Pitch of vessel on transmit (radians) */
	float		roll;		/* Roll of vessel on transmit (radians) */
	float		heave;		/* Heave of vessel on transmit (meters) */
	float		tide;		/* Tide value at transmit (meters) */
	unsigned int	hour;		/* 0-23 */
	unsigned int	minute;		/* 0-59 */
	unsigned int	second;		/* 0-59 */
	unsigned int	day;		/* 1-31 */
	unsigned int	month;		/* 1-12 */
	unsigned int	year;		/* 1-12 */

	/* beams */
	float		x[MBSYS_WASSP_BEAMS];		/* Acrosstrack distance (meters) */
	float		z[MBSYS_WASSP_BEAMS];		/* Depth (meters) */
	mb_u_char	texture[MBSYS_WASSP_BEAMS];	/* Seafloor intensity value */
	mb_u_char	reserved1[MBSYS_WASSP_BEAMS];	/* Reserved */
	mb_u_char	fish[MBSYS_WASSP_BEAMS];	/* Fish intensity value for all fish targest 
								vertically above detection point */
	mb_u_char	reserved1[MBSYS_WASSP_BEAMS];	/* Reserved */

	/* comment */
	char		comment[MBSYS_WASSP_MAXLINE];
	};	
	
/* system specific function prototypes */
int mbsys_wassp_alloc(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_wassp_deall(int verbose, void *mbio_ptr, void **store_ptr, 
			int *error);
int mbsys_wassp_dimensions(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_wassp_extract(int verbose, void *mbio_ptr, void *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_wassp_insert(int verbose, void *mbio_ptr, void *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_wassp_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_wassp_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_wassp_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_wassp_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_wassp_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_wassp_copy(int verbose, void *mbio_ptr, 
			void *store_ptr, void *copy_ptr,
			int *error);

