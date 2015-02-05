/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_templatesystem.h	1/28/2014
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
/*
 * mbsys_templatesystem.h defines the MBIO data structures for handling data from
 * the following data formats:
 *    MBSYS_TEMPLATESYSTEM formats (code in mbsys_templatesystem.c and mbsys_templatesystem.h):
 *      MBF_TEMPFORM : MBIO ID ??? (code in mbr_tempform.c)
 *
 * Author:	D. W. Caress
 * Date:	January 28, 2014
 *
 *
 */
/*
 * Notes on the mbsys_templatesystem data structure and associated format:
 *   1. This is example source to demonstrate how to code an
 *      MB-System i/o module.
 *   2. The structure in mbsys_templatesystem.h defines the internal
 *      representation of a class of data. This may be data
 *      associated with a single format, or data associated
 *      with multiple similar or related formats.
 *   3. The functions in mbsys_templatesystem.c allow for extracting
 *      data from or inserting data into this internal
 *      representation. These functions are called by the
 *      MBIO API functions found in mbio/mb_access.c.
 *   4. The functions in mbr_tempform.c actually read and
 *      write the mbf_template format.
 *   5. Prototypes for all of the functions in mbsys_templatesystem.c
 *      are provided in mbsys_templatesystem.h.
 *   6. This list of functions corresponds to the function pointers
 *      that are included in the structure mb_io_struct that is defined
 *      in the file mbsystem/src/mbio/mb_io.h
 *      Not all of these functions are required - some only make sense to
 *      define if the relevant data type is part of the format. For instance,
 *      do not define mbsys_templatesystem_extract_segy() if there are no subbottom
 *      profiler data supported by this data system */
*/

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/*---------------------------------------------------------------*/
/* Record ID definitions (if needed for use in data reading and writing) */

/*---------------------------------------------------------------*/
/* Record size definitions (if needed for use in data reading and writing) */

/*---------------------------------------------------------------*/
/* Array size definitions (if needed for use in data reading and writing) */
#define MBSYS_TEMPLATESYSTEM_MAX_BEAMS 400
#define MBSYS_TEMPLATESYSTEM_MAX_PIXELS 400

/*---------------------------------------------------------------*/

/* Structure size definitions (if needed because there are dynamically allocated substructures) */

/* Internal data structure */
struct mbsys_templatesystem_struct
	{
	/* Type of most recently read data record */
	int		kind;			/* MB-System record ID */

	/* MB-System time stamp of most recently read record */
	double		time_d;
	int		time_i[7];
        
        /* Survey data
        
        /* Navigation and attitude associated with survey data */
        
        /* Bathymetry and amplitude data */
        
        /* Raw backscatter data */
        
        /* Sidescan derived from raw backscatter */
        
        /* Navigation data */
        
        /* Sensordepth */
        
        /* Attitude data */
        
        /* Comment */

	};
        
/*---------------------------------------------------------------*/

/* System specific function prototypes */
/* Note: this list of functions corresponds to the function pointers
 * that are included in the structure mb_io_struct that is defined
 * in the file mbsystem/src/mbio/mb_io.h
 * Not all of these functions are required - some only make sense to
 * define if the relevant data type is part of the format. For instance,
 * do not define mbsys_templatesystem_extract_segy() if there are no subbottom
 * profiler data supported by this data system.
 * The function prototypes that are not required for all data systems
 * are commented out below. When using this example as the basis for
 * for coding a new MB-System I/O module, uncomment any non-required
 * functions that will be useful. */
int mbsys_templatesystem_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_templatesystem_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_templatesystem_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
//int mbsys_templatesystem_pingnumber(int verbose, void *mbio_ptr,
//			int *pingnumber, int *error);
//int mbsys_templatesystem_sonartype(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *sonartype, int *error);
//int mbsys_templatesystem_sidescantype(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *ss_type, int *error);
//int mbsys_templatesystem_preprocess(int verbose, void *mbio_ptr, void *store_ptr,
//                        double time_d, double navlon, double navlat,
//                        double speed, double heading, double sonardepth,
//                        double roll, double pitch, double heave,
//                        int *error);
int mbsys_templatesystem_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_templatesystem_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_templatesystem_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
//int mbsys_templatesystem_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
//			int nmax, int *kind, int *n,
//			int *time_i, double *time_d,
//			double *navlon, double *navlat,
//			double *speed, double *heading, double *draft,
//			double *roll, double *pitch, double *heave,
//			int *error);
int mbsys_templatesystem_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_templatesystem_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
//int mbsys_templatesystem_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr,
//                        double transducer_depth, double altitude,
//                        int *error);
int mbsys_templatesystem_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_templatesystem_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_templatesystem_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
//int mbsys_templatesystem_detects(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nbeams, int *detects, int *error);
//int mbsys_templatesystem_pulses(int verbose, void *mbio_ptr, void *store_ptr,
//                        int *kind, int *nbeams, int *pulses, int *error);
//int mbsys_templatesystem_gains(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, double *transmit_gain, double *pulse_length,
//			double *receive_gain, int *error);
//int mbsys_templatesystem_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_templatesystem_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr,
//			int nrawss,
//			double *rawss,
//			double *rawssacrosstrack,
//			double *rawssalongtrack,
//			int *error);
//int mbsys_templatesystem_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind,
//			void *segytraceheader_ptr,
//			int *error);
//int mbsys_templatesystem_extract_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int *sampleformat,
//			int *kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_templatesystem_insert_segy(int verbose, void *mbio_ptr, void *store_ptr,
//			int kind,
//			void *segytraceheader_ptr,
//			float *segydata,
//			int *error);
//int mbsys_templatesystem_ctd(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nctd, double *time_d,
//			double *conductivity, double *temperature,
//			double *depth, double *salinity, double *soundspeed, int *error);
//int mbsys_templatesystem_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr,
//			int *kind, int *nsensor, double *time_d,
//			double *sensor1, double *sensor2, double *sensor3,
//			double *sensor4, double *sensor5, double *sensor6,
//			double *sensor7, double *sensor8, int *error);
int mbsys_templatesystem_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
/*---------------------------------------------------------------*/
