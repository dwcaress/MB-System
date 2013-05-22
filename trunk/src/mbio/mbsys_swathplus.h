/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_swathplus.h	2/22/2008
 *	$Id$
 *
 *    Copyright (c) 2008-2013 by
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
 * mbsys_swathplus.h defines the MBIO data structures for handling data from
 * SEA SWATHplus interferometric formats:
 *      MBF_SWPLSSXI : MBIO ID 221 - SWATHplus intermediate format
 *      MBF_SWPLSSXP : MBIO ID 221 - SWATHplus processed format
 *
 * Author:	D. W. Caress
 * Date:	May 7, 2013
 *
 * $Log: mbsys_swathplus.c,v $
 *
 */
/*
 * Notes on the MBSYS_SWATHPLUS data structure:
 *   1. SEA defines three data formats associated with the SWATHplus
 *      interferometric sonar: raw, intermediate, and processed.
 *      MB-System supports the intermediate format as MBIO
 *      format 221 (MBF_SWPLSSXI) and the processed format as MBIO
 *      format 222 (MBF_SWPLSSXP).
 *   2.
 *   3.
 *   4.
 *
 */

/* include mb_define.h */
#ifndef MB_DEFINE_DEF
#include "mb_define.h"
#endif

/* internal data structure */
struct mbsys_swathplus_struct
	{
	/* type of data record */
	int	kind;		/* MB-System record ID */
	};


/* system specific function prototypes */
int mbsys_swathplus_alloc(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_swathplus_deall(int verbose, void *mbio_ptr, void **store_ptr,
			int *error);
int mbsys_swathplus_dimensions(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_swathplus_pingnumber(int verbose, void *mbio_ptr,
			int *pingnumber, int *error);
int mbsys_swathplus_extract(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_swathplus_insert(int verbose, void *mbio_ptr, void *store_ptr,
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp,
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_swathplus_ttimes(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles,
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset,
			double *draft, double *ssv, int *error);
int mbsys_swathplus_detects(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *detects, int *error);
int mbsys_swathplus_pulses(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int *nbeams, int *pulses, int *error);
int mbsys_swathplus_gains(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transmit_gain, double *pulse_length,
			double *receive_gain, int *error);
int mbsys_swathplus_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, double *transducer_depth, double *altitude,
			int *error);
int mbsys_swathplus_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
			int nmax, int *kind, int *n,
			int *time_i, double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_swathplus_extract_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft,
			double *roll, double *pitch, double *heave,
			int *error);
int mbsys_swathplus_insert_nav(int verbose, void *mbio_ptr, void *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft,
			double roll, double pitch, double heave,
			int *error);
int mbsys_swathplus_extract_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int *kind,
			int *nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_swathplus_insert_svp(int verbose, void *mbio_ptr, void *store_ptr,
			int nsvp,
			double *depth, double *velocity,
			int *error);
int mbsys_swathplus_copy(int verbose, void *mbio_ptr,
			void *store_ptr, void *copy_ptr,
			int *error);
