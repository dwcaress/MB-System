/*--------------------------------------------------------------------
 *    The MB-system:	mbsys_gsf.h	6/10/97
 *
 *    Copyright (c) 1998-2019 by
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

#ifndef MBSYS_GSF_H_
#define MBSYS_GSF_H_

#include "gsf.h"
#include "gsf_ft.h"
#include "gsf_enc.h"

/* internal data structure */
struct mbsys_gsf_struct {
	int kind;
	gsfDataID dataID;
	gsfRecords records;
};

#ifdef __cplusplus
extern "C" {
#endif

/* system specific function prototypes */
int mbsys_gsf_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_gsf_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error);
int mbsys_gsf_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error);
int mbsys_gsf_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error);
int mbsys_gsf_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error);
int mbsys_gsf_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                      double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                      double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                      double *ssacrosstrack, double *ssalongtrack, char *comment, int *error);
int mbsys_gsf_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                     double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                     double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                     double *ssalongtrack, char *comment, int *error);
int mbsys_gsf_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                     double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                     double *ssv, int *error);
int mbsys_gsf_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error);
int mbsys_gsf_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                               double *altitude, int *error);
int mbsys_gsf_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude, int *error);
int mbsys_gsf_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                          double *heave, int *error);
int mbsys_gsf_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                         double speed, double heading, double draft, double roll, double pitch, double heave, int *error);
int mbsys_gsf_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                          int *error);
int mbsys_gsf_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error);
int mbsys_gsf_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error);
int mbsys_gsf_setscalefactors(int verbose, int reset_all, gsfSwathBathyPing *mb_ping, int *error);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* MBSYS_GSF_H_ */
