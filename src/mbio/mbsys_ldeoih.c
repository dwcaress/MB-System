/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_ldeoih.c  2/26/93
 *
 *    Copyright (c) 1993-2025 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, California, USA
 *    Dale N. Chayes 
 *      Center for Coastal and Ocean Mapping
 *      University of New Hampshire
 *      Durham, New Hampshire, USA
 *    Christian dos Santos Ferreira
 *      MARUM
 *      University of Bremen
 *      Bremen Germany
 *     
 *    MB-System was created by Caress and Chayes in 1992 at the
 *      Lamont-Doherty Earth Observatory
 *      Columbia University
 *      Palisades, NY 10964
 *
 *    See README.md file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbsys_ldeoih.c contains the functions for handling the data structure
 * used by MBIO functions to store data from a generic multibeam
 * format which handles data with arbitrary numbers of bathymetry,
 * amplitude, and sidescan data.  This generic format is
 *      MBF_MBLDEOIH : MBIO ID 61
 *
 * Author:  D. W. Caress
 * Date:  February 26, 1993
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_ldeoih.h"

/*--------------------------------------------------------------------*/
int mbsys_ldeoih_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_ldeoih_struct), (void **)store_ptr, error);

  /* get pointer to data structure */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)*store_ptr;

  /* initialize values in structure */
  store->kind = MB_DATA_NONE;
  store->time_d = 0.0;
  store->longitude = 0.0;
  store->latitude = 0.0;
  store->sensordepth = 0.0;
  store->altitude = 0.0;
  store->heading = 0.0;
  store->speed = 0.0;
  store->roll = 0.0;
  store->pitch = 0.0;
  store->heave = 0.0;
  store->beam_xwidth = 0.0;
  store->beam_lwidth = 0.0;
  store->beams_bath = 0;
  store->beams_amp = 0;
  store->pixels_ss = 0;
  store->sensorhead = 0;
  store->beams_bath_alloc = 0;
  store->beams_amp_alloc = 0;
  store->pixels_ss_alloc = 0;
  store->depth_scale = 0.0;
  store->distance_scale = 0.0;
  store->ss_scalepower = 0;
  store->ss_type = 0;
  store->imagery_type = MB_IMAGERY_TYPE_UNKNOWN;
  store->topo_type = MB_TOPOGRAPHY_TYPE_UNKNOWN;
  store->beamflag = NULL;
  store->bath = NULL;
  store->amp = NULL;
  store->bath_acrosstrack = NULL;
  store->bath_alongtrack = NULL;
  store->ss = NULL;
  store->ss_acrosstrack = NULL;
  store->ss_alongtrack = NULL;
  memset(store->comment, 0, MBSYS_LDEOIH_MAXLINE);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  /* get pointer to data structure */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)*store_ptr;

  /* deallocate memory for data structures */
  int status = mb_freed(verbose, __FILE__, __LINE__, (void **)&store->beamflag, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_alongtrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->amp, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss_acrosstrack, error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss_alongtrack, error);

  /* deallocate memory for data structure */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                            int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get beam and pixel numbers */
    *nbath = store->beams_bath;
    *namp = store->beams_amp;
    *nss = store->pixels_ss;
  }
  else {
    /* get beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
    fprintf(stderr, "dbg2       namp:       %d\n", *namp);
    fprintf(stderr, "dbg2       nss:        %d\n", *nss);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get sidescan type */
  *sonartype = store->topo_type;
    if (*sonartype == MB_TOPOGRAPHY_TYPE_UNKNOWN
        && store->beam_xwidth > 0.0 && store->beam_lwidth > 0.0) {
        *sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
    }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get sidescan type */
  *ss_type = store->ss_type;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_sensorhead(int verbose, void *mbio_ptr, void *store_ptr,
                int *sensorhead, int *error) {
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* if survey data extract which sensor head used for this scan */
  if (store->kind == MB_DATA_DATA) {
    *sensorhead = store->sensorhead;
  } else {
    *sensorhead = 0;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sensorhead: %d\n", *sensorhead);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                         double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                         double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                         double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    *time_d = store->time_d;
    mb_get_date(verbose, *time_d, time_i);

    /* get navigation */
    *navlon = store->longitude;
    *navlat = store->latitude;

    /* get heading */
    *heading = store->heading;

    /* get speed */
    *speed = store->speed;

    /* set beamwidths in mb_io structure */
    if (store->beam_lwidth > 0)
      mb_io_ptr->beamwidth_ltrack = store->beam_lwidth;
    else
      mb_io_ptr->beamwidth_ltrack = 2.0;
    if (store->beam_xwidth > 0)
      mb_io_ptr->beamwidth_xtrack = store->beam_xwidth;
    else
      mb_io_ptr->beamwidth_xtrack = 2.0;

    /* read distance, depth, and backscatter
        values into storage arrays */
    *nbath = store->beams_bath;
    *namp = store->beams_amp;
    *nss = store->pixels_ss;
    for (int i = 0; i < *nbath; i++) {
      beamflag[i] = store->beamflag[i];
      if (beamflag[i] != MB_FLAG_NULL) {
        bath[i] = store->depth_scale * store->bath[i] + store->sensordepth;
        bathacrosstrack[i] = store->distance_scale * store->bath_acrosstrack[i];
        bathalongtrack[i] = store->distance_scale * store->bath_alongtrack[i];
      }
      else {
        bath[i] = 0.0;
        bathacrosstrack[i] = 0.0;
        bathalongtrack[i] = 0.0;
      }
    }
    for (int i = 0; i < *namp; i++) {
      amp[i] = store->amp[i];
    }
    const double ss_scale = pow(2.0, (double)(store->ss_scalepower));
    for (int i = 0; i < *nss; i++) {
      if (store->ss[i] != 0) {
        ss[i] = ss_scale * store->ss[i];
      } else {
        ss[i] = MB_SIDESCAN_NULL;
      }
      ssacrosstrack[i] = store->distance_scale * store->ss_acrosstrack[i];
      ssalongtrack[i] = store->distance_scale * store->ss_alongtrack[i];
    }

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
      fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
      for (int i = 0; i < *nbath; i++)
        fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
                bathacrosstrack[i], bathalongtrack[i]);
      fprintf(stderr, "dbg4        namp:     %d\n", *namp);
      for (int i = 0; i < *namp; i++)
        fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
                bathalongtrack[i]);
      fprintf(stderr, "dbg4        nss:      %d\n", *nss);
      for (int i = 0; i < *nss; i++)
        fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
                ssalongtrack[i]);
    }

    /* done translating values */
  }

  /* extract comment from structure */
  else if (*kind == MB_DATA_COMMENT) {
    /* copy comment */
    memset((void *)comment, 0, sizeof(comment));
    strncpy(comment, store->comment, MIN(sizeof(comment), sizeof(store->comment)) - 1);

    if (verbose >= 4) {
      fprintf(stderr, "\ndbg4  New ping read by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  New ping values:\n");
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       comment:    %s\n", comment);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }
  else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
    fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", *speed);
    fprintf(stderr, "dbg2       heading:       %f\n", *heading);
  }
  if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
    for (int i = 0; i < *nbath; i++)
      fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
              bathacrosstrack[i], bathalongtrack[i]);
    fprintf(stderr, "dbg2        namp:     %d\n", *namp);
    for (int i = 0; i < *namp; i++)
      fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
              bathalongtrack[i]);
    fprintf(stderr, "dbg2        nss:      %d\n", *nss);
    for (int i = 0; i < *nss; i++)
      fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
              ssalongtrack[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                        double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                        double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                        double *ssalongtrack, char *comment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:       %d\n", kind);
  }
  if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
    fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
    fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
    fprintf(stderr, "dbg2       speed:      %f\n", speed);
    fprintf(stderr, "dbg2       heading:    %f\n", heading);
  }
  if (verbose >= 2 && kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
    if (verbose >= 3)
      for (int i = 0; i < nbath; i++)
        fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
                bathacrosstrack[i], bathalongtrack[i]);
    fprintf(stderr, "dbg2       namp:       %d\n", namp);
    if (verbose >= 3)
      for (int i = 0; i < namp; i++)
        fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
                bathalongtrack[i]);
    fprintf(stderr, "dbg2        nss:       %d\n", nss);
    if (verbose >= 3)
      for (int i = 0; i < nss; i++)
        fprintf(stderr, "dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
                ssalongtrack[i]);
  }
  if (verbose >= 2 && kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* set data kind */
  store->kind = kind;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    store->time_d = time_d;

    /* get navigation */
    store->longitude = navlon;
    store->latitude = navlat;

    /* get heading */
    store->heading = heading;
    store->speed = speed;

    /* if needed reset numbers of beams and allocate
       memory for store arrays */
    if (nbath > store->beams_bath_alloc) {
      store->beams_bath_alloc = nbath;
      status &= mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(char),
                          (void **)&store->beamflag, error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                          (void **)&store->bath, error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                          (void **)&store->bath_acrosstrack, error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                          (void **)&store->bath_alongtrack, error);

      /* deal with a memory allocation failure */
      if (status == MB_FAILURE) {
        /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->beamflag, error);
        /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath, error);
        /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_acrosstrack, error);
        /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_alongtrack, error);
        status = MB_FAILURE;
        *error = MB_ERROR_MEMORY_FAIL;
        if (verbose >= 2) {
          fprintf(stderr, "\ndbg2  MBcopy function <%s> terminated with error\n", __func__);
          fprintf(stderr, "dbg2  Return values:\n");
          fprintf(stderr, "dbg2       error:      %d\n", *error);
          fprintf(stderr, "dbg2  Return status:\n");
          fprintf(stderr, "dbg2       status:  %d\n", status);
        }
        return (status);
      }
    }
    if (namp > store->beams_amp_alloc) {
      store->beams_amp_alloc = namp;
      /* if (store != NULL) */ {
        status = mb_reallocd(verbose, __FILE__, __LINE__, store->beams_amp_alloc * sizeof(short),
                            (void **)&store->amp, error);

        /* deal with a memory allocation failure */
        if (status == MB_FAILURE) {
          /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->amp, error);
          status = MB_FAILURE;
          *error = MB_ERROR_MEMORY_FAIL;
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  MBIO function <%s> terminated with error\n", __func__);
            fprintf(stderr, "dbg2  Return values:\n");
            fprintf(stderr, "dbg2       error:      %d\n", *error);
            fprintf(stderr, "dbg2  Return status:\n");
            fprintf(stderr, "dbg2       status:  %d\n", status);
          }
          return (status);
        }
      }
    }
    if (nss > store->pixels_ss_alloc) {
      store->pixels_ss_alloc = nss;
      /* if (store != NULL) */ {
        status &= mb_reallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(short),
                            (void **)&store->ss, error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(short),
                            (void **)&store->ss_acrosstrack, error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, store->pixels_ss_alloc * sizeof(short),
                            (void **)&store->ss_alongtrack, error);

        /* deal with a memory allocation failure */
        if (status == MB_FAILURE) {
          /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss, error);
          /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss_acrosstrack, error);
          /* status &= */ mb_freed(verbose, __FILE__, __LINE__, (void **)&store->ss_alongtrack, error);
          status = MB_FAILURE;
          *error = MB_ERROR_MEMORY_FAIL;
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  MBIO function <%s> terminated with error\n", __func__);
            fprintf(stderr, "dbg2  Return values:\n");
            fprintf(stderr, "dbg2       error:      %d\n", *error);
            fprintf(stderr, "dbg2  Return status:\n");
            fprintf(stderr, "dbg2       status:  %d\n", status);
          }
          return (status);
        }
      }
    }

    /* get scaling */
    bool notdepthfirst = false;
    bool notdistfirst = false;
    bool notssfirst = false;
    double depthmax = 0.0;
    double distmax = 0.0;
    double ssmax = 0.0;
    for (int i = 0; i < nbath; i++) {
      if (beamflag[i] != MB_FLAG_NULL) {
        if (notdepthfirst) {
          depthmax = MAX(depthmax, fabs(bath[i] - store->sensordepth));
          distmax = MAX(distmax, fabs(bathacrosstrack[i]));
          distmax = MAX(distmax, fabs(bathalongtrack[i]));
        } else {
          notdepthfirst = true;
          notdistfirst = true;
          depthmax = fabs(bath[i] - store->sensordepth);
          distmax = fabs(bathacrosstrack[i]);
          distmax = MAX(distmax, fabs(bathalongtrack[i]));
        }
      }
    }
    for (int i = 0; i < nss; i++) {
      if (ss[i] > MB_SIDESCAN_NULL) {
        if (notdistfirst) {
          distmax = MAX(distmax, fabs(ssacrosstrack[i]));
          distmax = MAX(distmax, fabs(ssalongtrack[i]));
        } else {
          notdistfirst = true;
          distmax = fabs(ssacrosstrack[i]);
          distmax = fabs(ssalongtrack[i]);
        }
        if (notssfirst) {
          ssmax = MAX(ssmax, fabs(ss[i]));
        } else {
          notssfirst = true;
          ssmax = fabs(ss[i]);
        }
      }
    }
    if (notdepthfirst) {
      store->depth_scale = 0.001 * (float)(MAX((depthmax / 30.0), 1.0));
    } else {
      store->depth_scale = 0.001;
    }
    if (notdistfirst) {
      store->distance_scale = 0.001 * (float)(MAX((distmax / 30.0), 1.0));
    } else {
      store->distance_scale = 0.001;
    }
    if (notssfirst) {
      store->ss_scalepower = (mb_s_char)(log2(ssmax / 32767.0)) + 1;
    } else {
      store->ss_scalepower = 0;
    }
    double const ss_scale = pow(2.0, (double)(store->ss_scalepower));

    /* set beam widths */
    if (store->beam_xwidth == 0.0)
      store->beam_xwidth = mb_io_ptr->beamwidth_xtrack;
    if (store->beam_lwidth == 0.0)
      store->beam_lwidth = mb_io_ptr->beamwidth_ltrack;

    /* put distance, depth, and backscatter values
        into data structure */
    store->beams_bath = nbath;
    for (int i = 0; i < nbath; i++) {
      if (beamflag[i] != MB_FLAG_NULL) {
        store->beamflag[i] = beamflag[i];
        store->bath[i] = (bath[i] - store->sensordepth) / store->depth_scale;
        store->bath_acrosstrack[i] = bathacrosstrack[i] / store->distance_scale;
        store->bath_alongtrack[i] = bathalongtrack[i] / store->distance_scale;
      }
      else {
        store->beamflag[i] = beamflag[i];
        store->bath[i] = 0;
        store->bath_acrosstrack[i] = 0;
        store->bath_alongtrack[i] = 0;
      }
    }
    store->beams_amp = namp;
    for (int i = 0; i < namp; i++) {
      if (beamflag[i] != MB_FLAG_NULL) {
        store->amp[i] = amp[i];
      }
    }
    store->pixels_ss = nss;
    for (int i = 0; i < nss; i++) {
      if (ss[i] > MB_SIDESCAN_NULL) {
        store->ss[i] = (short)(ss[i] / ss_scale);
        if (store->ss[i] == 0)
          store->ss[i] = 1; //making sure only flagged ss values are exactly zero.
      } else{
        store->ss[i] = 0;
      }
      store->ss_acrosstrack[i] = ssacrosstrack[i] / store->distance_scale;
      store->ss_alongtrack[i] = ssalongtrack[i] / store->distance_scale;
    }
  }

  /* insert comment in structure */
  else if (store->kind == MB_DATA_COMMENT) {
    strncpy(store->comment, comment, MBSYS_LDEOIH_MAXLINE - 1);
    if (strlen(comment) > MBSYS_LDEOIH_MAXLINE - 2)
      store->comment[MBSYS_LDEOIH_MAXLINE - 1] = '\0';
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                        double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                        double *ssv, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
    fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
    fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
    fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
    fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
    fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get nbeams */
    *nbeams = store->beams_bath;

    /* get travel times, angles */
    for (int i = 0; i < store->beams_bath; i++) {
      ttimes[i] = 0.0;
      angles[i] = 0.0;
      angles_forward[i] = 0.0;
      angles_null[i] = 0.0;
      heave[i] = 0.0;
      alongtrack_offset[i] = 0.0;
    }

    /* get ssv */
    *ssv = 0.0;
    *draft = 0.0;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       draft:      %f\n", *draft);
    fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
              i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get nbeams */
    *nbeams = store->beams_bath;

    /* get detects */
    for (int i = 0; i < *nbeams; i++) {
      detects[i] = MB_DETECT_UNKNOWN;
    }

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                  double *altitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    *transducer_depth = store->sensordepth;
    if (store->altitude <= 0.0 && store->beams_bath > 0) {
      double bath_best = 0.0;
      if (store->bath[store->beams_bath / 2] > 0.0)
        bath_best = store->depth_scale * store->bath[store->beams_bath / 2] + (*transducer_depth);
      else {
        double xtrack_min = 99999999.9;
        for (int i = 0; i < store->beams_bath; i++) {
          if (store->bath[i] > 0 && fabs(store->distance_scale * store->bath_acrosstrack[i]) < xtrack_min) {
            xtrack_min = fabs(store->distance_scale * store->bath_acrosstrack[i]);
            bath_best = store->depth_scale * store->bath[i] + (*transducer_depth);
          }
        }
      }
      if (bath_best <= 0.0) {
        double xtrack_min = 99999999.9;
        for (int i = 0; i < store->beams_bath; i++) {
          if (store->bath[i] < 0.0 && fabs(store->distance_scale * store->bath_acrosstrack[i]) < xtrack_min) {
            xtrack_min = fabs(store->distance_scale * store->bath_acrosstrack[i]);
            bath_best = -store->depth_scale * store->bath[i] + (*transducer_depth);
          }
        }
      }
      *altitude = bath_best - *transducer_depth;
    }
    else
      *altitude = store->altitude;
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
    fprintf(stderr, "dbg2       altitude:          %f\n", *altitude);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude,
                                 int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       transducer_depth:  %f\n", transducer_depth);
    fprintf(stderr, "dbg2       altitude:          %f\n", altitude);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* insert data into structure */
  if (store->kind == MB_DATA_DATA) {
    store->sensordepth = transducer_depth;
    store->altitude = altitude;
  }

  /* deal with comment */
  else if (store->kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                             double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                             double *pitch, double *heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */;
    *time_d = store->time_d;
    mb_get_date(verbose, *time_d, time_i);

    /* get navigation */
    *navlon = store->longitude;
    *navlat = store->latitude;

    /* get heading */
    *heading = store->heading;

    /* set speed */
    *speed = store->speed;

    /* set draft */
    *draft = store->sensordepth + store->heave;

    /* get roll pitch and heave */
    *roll = store->roll;
    *pitch = store->pitch;
    *heave = store->heave;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
      fprintf(stderr, "dbg4       draft:      %f\n", *draft);
      fprintf(stderr, "dbg4       roll:       %f\n", *roll);
      fprintf(stderr, "dbg4       pitch:      %f\n", *pitch);
      fprintf(stderr, "dbg4       heave:      %f\n", *heave);
    }

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
    fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", *speed);
    fprintf(stderr, "dbg2       heading:       %f\n", *heading);
    fprintf(stderr, "dbg2       draft:         %f\n", *draft);
    fprintf(stderr, "dbg2       roll:          %f\n", *roll);
    fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
    fprintf(stderr, "dbg2       heave:         %f\n", *heave);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                            double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                            int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
    fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
    fprintf(stderr, "dbg2       speed:      %f\n", speed);
    fprintf(stderr, "dbg2       heading:    %f\n", heading);
    fprintf(stderr, "dbg2       draft:      %f\n", draft);
    fprintf(stderr, "dbg2       roll:       %f\n", roll);
    fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
    fprintf(stderr, "dbg2       heave:      %f\n", heave);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    store->time_d = time_d;

    /* get navigation */
    store->longitude = navlon;
    store->latitude = navlat;

    /* get heading */
    store->heading = heading;
    store->speed = speed;

    /* get draft */
    store->sensordepth = draft - heave;

    /* get roll pitch and heave */
    store->roll = roll;
    store->pitch = pitch;
    store->heave = heave;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_ldeoih_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointers */
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;
  struct mbsys_ldeoih_struct *copy = (struct mbsys_ldeoih_struct *)copy_ptr;

  int status = MB_SUCCESS;

  if (copy->beamflag != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->beamflag, error);
  if (copy->bath != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath, error);
  if (copy->bath_acrosstrack != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath_acrosstrack, error);
  if (copy->bath_alongtrack != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath_alongtrack, error);
  if (copy->amp != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->amp, error);
  if (copy->ss != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss, error);
  if (copy->ss_acrosstrack != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss_acrosstrack, error);
  if (copy->ss_alongtrack != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss_alongtrack, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath * sizeof(char), (void **)&copy->beamflag, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath * sizeof(short), (void **)&copy->bath, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath * sizeof(short), (void **)&copy->bath_acrosstrack, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_bath * sizeof(short), (void **)&copy->bath_alongtrack, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->beams_amp * sizeof(short), (void **)&copy->amp, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss * sizeof(short), (void **)&copy->ss, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss * sizeof(short), (void **)&copy->ss_acrosstrack, error);
  status &= mb_mallocd(verbose, __FILE__, __LINE__, store->pixels_ss * sizeof(short), (void **)&copy->ss_alongtrack, error);

  /* deal with a memory allocation failure */
  if (status == MB_FAILURE) {
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->beamflag, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath_acrosstrack, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->bath_alongtrack, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->amp, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss_acrosstrack, error);
    /* status = */ mb_freed(verbose, __FILE__, __LINE__, (void **)&copy->ss_alongtrack, error);
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
  }

  /* copy the data */
  if (status == MB_SUCCESS) {
    copy->kind = store->kind;
    copy->time_d = store->time_d;
    copy->longitude = store->longitude;
    copy->latitude = store->latitude;
    copy->sensordepth = store->sensordepth;
    copy->altitude = store->altitude;
    copy->heading = store->heading;
    copy->speed = store->speed;
    copy->roll = store->roll;
    copy->pitch = store->pitch;
    copy->heave = store->heave;
    copy->beam_xwidth = store->beam_xwidth;
    copy->beam_lwidth = store->beam_lwidth;
    copy->beams_bath = store->beams_bath;
    copy->beams_amp = store->beams_amp;
    copy->pixels_ss = store->pixels_ss;
    copy->sensorhead = store->sensorhead;
    copy->depth_scale = store->depth_scale;
    copy->distance_scale = store->distance_scale;
    copy->ss_type = store->ss_type;
    copy->ss_scalepower = store->ss_scalepower;
    for (int i = 0; i < copy->beams_bath; i++) {
      copy->beamflag[i] = store->beamflag[i];
      copy->bath[i] = store->bath[i];
      copy->bath_acrosstrack[i] = store->bath_acrosstrack[i];
      copy->bath_alongtrack[i] = store->bath_alongtrack[i];
    }
    for (int i = 0; i < copy->beams_amp; i++) {
      copy->amp[i] = store->amp[i];
    }
    for (int i = 0; i < copy->pixels_ss; i++) {
      copy->ss[i] = store->ss[i];
      copy->ss_acrosstrack[i] = store->ss_acrosstrack[i];
      copy->ss_alongtrack[i] = store->ss_alongtrack[i];
    }
    strcpy(copy->comment, store->comment);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
