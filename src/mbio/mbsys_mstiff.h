/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_mstiff.h	4/10/98
 *	$Id: mbsys_mstiff.h,v 5.1 2001-01-22 07:43:34 caress Exp $
 *
 *    Copyright (c) 1998, 2000 by
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
 * mbsys_mstiff.h  defines the data structure used by MBIO functions
 * to store sidescan data read from the MBF_MSTIFFSS format (MBIO id 131).  
 *
 * Author:	D. W. Caress
 * Date:	April 10, 1988
 * $Log: not supported by cvs2svn $
 * Revision 5.0  2000/12/01  22:48:41  caress
 * First cut at Version 5.0.
 *
 * Revision 4.1  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1998/10/05  18:30:03  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
*
 *
 */
/*
 * Notes on the MBSYS_MSTIFF data structure:
 *   1. The MSTIFF data format is used to store raw sidescan data from 
 *      Sea Scan sidescan sonars. This format is a variant of
 *      the TIFF image format with navigation and other information
 *      embedded within the file.
 *   2. The file structure consists of a bunch of pointers to
 *      data objects at various arbitrary locations within the
 *      file. The header contains a pointer to the location of
 *      the "image file directory", which in turn contains
 *      pointers to the locations of data arrays within the file.
 *   3. As far as MB-System is concerned,  this is a read-only
 *      data format.
 *   4. The raw sidescan data in the file consists of 1000 pings.
 *      Each ping produces two 512 sample arrays - one for
 *      each side (port and starboard).
 *   5. The sidescan data is not slant range corrected - the 
 *      bottom detect and slant range correction is done on
 *      input by MBIO. The data stored internally by MBIO
 *      is slant range corrected.
 *   6. The MSTIFF files contain lots of information not used by
 *      MBIO,  including images of the data derived from a
 *      realtime display.
 *   7. Comments are not supported in this format.
 */
 
/* number of sidescan pixels for Sea Scan sidescan sonars */
#define MBSYS_MSTIFF_PIXELS	1024

struct mbsys_mstiff_struct
	{
	/* time stamp */
	double	time_d;		/* unix time */

	/* position */
	double	lat;		/* latitude in degrees */
	double	lon;		/* longitude in degrees */

	/* other values */
	double	heading;	/* heading in degrees */
	double	speed;		/* fore-aft speed in km/hr */
	double	altitude;	/* altitude in m */
	double	slant_range_max;    /* seconds */
	double	range_delay;	    /* seconds */
	double	sample_interval;    /* seconds */

	/* sidescan data */
	int	pixels_ss;	/* number of pixels */
	unsigned char	ss[MBSYS_MSTIFF_PIXELS];
	double	ssacrosstrack[MBSYS_MSTIFF_PIXELS];
	};
	
/* system specific function prototypes */
int mbsys_mstiff_alloc(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_mstiff_deall(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_mstiff_extract(int verbose, char *mbio_ptr, char *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_mstiff_insert(int verbose, char *mbio_ptr, char *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_mstiff_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_mstiff_extract_altitude(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_mstiff_extract_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_mstiff_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_mstiff_copy(int verbose, char *mbio_ptr, 
			char *store_ptr, char *copy_ptr,
			int *error);

