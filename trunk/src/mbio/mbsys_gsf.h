/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_gsf.h	6/10/97
 *	$Id: mbsys_gsf.h,v 5.0 2000-12-01 22:48:41 caress Exp $
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
 * mbsys_gsf.h defines the data structures used by MBIO functions
 * to store data from the Generic Sensor Format (GSF).
 * The MBIO representation of GSF is:
 *      MBF_GSFGENMB : MBIO ID 121
 *
 *
 * Author:	D. W. Caress
 * Date:	March 5, 1998
 *
 * $Log: not supported by cvs2svn $
 * Revision 4.1  2000/09/30  06:31:19  caress
 * Snapshot for Dale.
 *
 * Revision 4.0  1998/10/05  19:16:02  caress
 * MB-System version 4.6beta
 *
 * Revision 1.1  1998/10/05  18:32:27  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  18:22:40  caress
 * Initial revision
 *
 * Revision 1.1  1998/10/05  17:46:15  caress
 * Initial revision
 *
 *
 */
/*
 * Notes on the MBF_GSFGENMB data format:
 *   1. The underlying data format is the Generic Sensor Format (GSF)
 *      developed by Shannon Byrne of SAIC. The GSF format stores swath 
 *      bathymetry, single beam bathymetry, and other data.
 *   2. This MBIO i/o module accesses swath bathymtry data stored in
 *      the GSF format using the gsflib library also written at SAIC.
 *      The gsflib calls translate the data from scaled short integers
 *      (big endian) stored in the file to double values. The sensor
 *      specific values held in the GSF data stream are not
 *      accessed by this module. However, all of the GSF records
 *      and the included information are passed when mb_get_all, 
 *      mb_put_all, and the mb_buffer routines are used for
 *      reading and writing.
 *
 */
 
#ifndef __GSF_H__
#include "gsf.h"
#endif

/* internal data structure */
struct mbsys_gsf_struct
	{
	int	    kind;
	gsfDataID   dataID;
	gsfRecords  records;
	};

	
/* system specific function prototypes */
int mbsys_gsf_alloc(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_gsf_deall(int verbose, char *mbio_ptr, char **store_ptr, 
			int *error);
int mbsys_gsf_extract(int verbose, char *mbio_ptr, char *store_ptr, 
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading,
			int *nbath, int *namp, int *nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_gsf_insert(int verbose, char *mbio_ptr, char *store_ptr, 
			int kind, int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading,
			int nbath, int namp, int nss,
			char *beamflag, double *bath, double *amp, 
			double *bathacrosstrack, double *bathalongtrack,
			double *ss, double *ssacrosstrack, double *ssalongtrack,
			char *comment, int *error);
int mbsys_gsf_ttimes(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int *nbeams,
			double *ttimes, double *angles, 
			double *angles_forward, double *angles_null,
			double *heave, double *alongtrack_offset, 
			double *draft, double *ssv, int *error);
int mbsys_gsf_altitude(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, double *transducer_depth, double *altitude, 
			int *error);
int mbsys_gsf_insert_altitude(int verbose, char *mbio_ptr, char *store_ptr,
			double transducer_depth, double altitude, 
			int *error);
int mbsys_gsf_extract_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int *kind, int time_i[7], double *time_d,
			double *navlon, double *navlat,
			double *speed, double *heading, double *draft, 
			double *roll, double *pitch, double *heave, 
			int *error);
int mbsys_gsf_insert_nav(int verbose, char *mbio_ptr, char *store_ptr,
			int time_i[7], double time_d,
			double navlon, double navlat,
			double speed, double heading, double draft, 
			double roll, double pitch, double heave,
			int *error);
int mbsys_gsf_copy(int verbose, char *mbio_ptr, 
			char *store_ptr, char *copy_ptr,
			int *error);

