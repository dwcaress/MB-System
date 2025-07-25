/*--------------------------------------------------------------------
 *    The MB-system:  mb_access.c  11/1/00
  *
 *    Copyright (c) 2000-2025 by
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
 * This source file includes the functions used to extract data from
 * and insert data into sonar specific structures.
 *
 * Author:  D. W. Caress
 * Date:  October 1, 2000
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_segy.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
int mb_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call appropriate memory allocation routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_store_alloc != NULL) {
    status = (*mb_io_ptr->mb_io_store_alloc)(verbose, mbio_ptr, store_ptr, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

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
int mb_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call appropriate memory deallocation routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_store_free != NULL) {
    status = (*mb_io_ptr->mb_io_store_free)(verbose, mbio_ptr, store_ptr, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_get_store(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  *store_ptr = (void *)mb_io_ptr->store_data;

  const int status = MB_SUCCESS;

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
int mb_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract != NULL) {
    status = (*mb_io_ptr->mb_io_dimensions)(verbose, mbio_ptr, store_ptr, kind, nbath, namp, nss, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

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
int mb_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_pingnumber != NULL) {
    status = (*mb_io_ptr->mb_io_pingnumber)(verbose, mbio_ptr, pingnumber, error);
  }
  else {
    *pingnumber = mb_io_ptr->ping_count;
    status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_segynumber(int verbose, void *mbio_ptr, unsigned int *line, unsigned int *shot, unsigned int *cdp, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_segynumber != NULL) {
    status = (*mb_io_ptr->mb_io_segynumber)(verbose, mbio_ptr, line, shot, cdp, error);
  }
  else {
    *line = 0;
    *shot = mb_io_ptr->ping_count;
    *cdp = 0;
    status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       line:       %d\n", *line);
    fprintf(stderr, "dbg2       shot:       %d\n", *shot);
    fprintf(stderr, "dbg2       cdp:        %d\n", *cdp);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_beamwidths(int verbose, void *mbio_ptr, double *beamwidth_xtrack, double *beamwidth_ltrack, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:           %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* extract the values from the mb_io structure */
  *beamwidth_xtrack = mb_io_ptr->beamwidth_xtrack;
  *beamwidth_ltrack = mb_io_ptr->beamwidth_ltrack;
  const int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       beamwidth_xtrack: %f\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack: %f\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       error:            %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* reset error value */
  *error = MB_ERROR_NO_ERROR;

  /* start off with sonartype unknown */
  *sonartype = MB_TOPOGRAPHY_TYPE_UNKNOWN;

  /* call the appropriate mbsys_ sonartype routine
          mb_io_ptr->system == MB_SYS_LDEOIH
          mb_io_ptr->system == MB_SYS_GSF
          mb_io_ptr->system == MB_SYS_HDCS
          mb_io_ptr->system == MB_SYS_HYSWEEP */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_sonartype != NULL) {
    if (store_ptr == NULL)
      store_ptr = (void *)mb_io_ptr->store_data;
    status = (*mb_io_ptr->mb_io_sonartype)(verbose, mbio_ptr, store_ptr, sonartype, error);
  }

  /* Some systems are definitively echosounders */
  else if (mb_io_ptr->system == MB_SYS_SINGLEBEAM) {
    if (mb_io_ptr->format == MBF_ASCIIXYZ || mb_io_ptr->format == MBF_ASCIIYXZ || mb_io_ptr->format == MBF_ASCIIYXT ||
        mb_io_ptr->format == MBF_ASCIIYXT) {
      *sonartype = MB_TOPOGRAPHY_TYPE_POINT;
    }
    else {
      *sonartype = MB_TOPOGRAPHY_TYPE_ECHOSOUNDER;
    }
  }

  /* Some systems are definitively multibeams */
  else if (mb_io_ptr->system == MB_SYS_SB || mb_io_ptr->system == MB_SYS_HSDS || mb_io_ptr->system == MB_SYS_SB2000 ||
           mb_io_ptr->system == MB_SYS_SB2100 || mb_io_ptr->system == MB_SYS_SIMRAD || mb_io_ptr->system == MB_SYS_SIMRAD2 ||
           mb_io_ptr->system == MB_SYS_SIMRAD3 || mb_io_ptr->system == MB_SYS_RESON || mb_io_ptr->system == MB_SYS_RESON8K ||
           mb_io_ptr->system == MB_SYS_ELAC || mb_io_ptr->system == MB_SYS_ELACMK2 || mb_io_ptr->system == MB_SYS_HSMD ||
           mb_io_ptr->system == MB_SYS_XSE || mb_io_ptr->system == MB_SYS_NETCDF || mb_io_ptr->system == MB_SYS_HS10 ||
           mb_io_ptr->system == MB_SYS_ATLAS || mb_io_ptr->system == MB_SYS_SURF || mb_io_ptr->system == MB_SYS_RESON7K ||
           mb_io_ptr->system == MB_SYS_WASSP) {
    *sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;
  }

  /* Some systems are definitively sidescans */
  else if (mb_io_ptr->system == MB_SYS_MSTIFF || mb_io_ptr->system == MB_SYS_JSTAR || mb_io_ptr->system == MB_SYS_BENTHOS ||
           mb_io_ptr->system == MB_SYS_IMAGE83P) {
    *sonartype = MB_TOPOGRAPHY_TYPE_SIDESCAN;
  }

  /* Some systems are definitively interferometric sonars */
  else if (mb_io_ptr->system == MB_SYS_MR1 || mb_io_ptr->system == MB_SYS_MR1B || mb_io_ptr->system == MB_SYS_MR1V2001 ||
           mb_io_ptr->system == MB_SYS_DSL || mb_io_ptr->system == MB_SYS_OIC || mb_io_ptr->system == MB_SYS_SWATHPLUS) {
    *sonartype = MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC;
  }

  /* Some systems are definitively lidars */
  else if (mb_io_ptr->system == MB_SYS_3DATDEPTHLIDAR) {
    *sonartype = MB_TOPOGRAPHY_TYPE_LIDAR;
  }

  /* Some systems are definitively stereo cameras */
  else if (mb_io_ptr->system == MB_SYS_STEREOPAIR) {
    *sonartype = MB_TOPOGRAPHY_TYPE_CAMERA;
  }

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
int mb_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* reset error value */
  *error = MB_ERROR_NO_ERROR;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_sidescantype != NULL) {
    if (store_ptr == NULL)
      store_ptr = (void *)mb_io_ptr->store_data;
    status = (*mb_io_ptr->mb_io_sidescantype)(verbose, mbio_ptr, store_ptr, ss_type, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

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
int mb_preprocess(int verbose, void *mbio_ptr, void *store_ptr, void *platform_ptr, void *preprocess_pars_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_preprocess != NULL) {
    status = (*mb_io_ptr->mb_io_preprocess)(verbose, mbio_ptr, store_ptr, platform_ptr, preprocess_pars_ptr, error);
  }

  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:         %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  struct mb_platform_struct *platform = NULL;
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_platform != NULL) {
    status = (*mb_io_ptr->mb_io_extract_platform)(verbose, mbio_ptr, store_ptr, kind, platform_ptr, error);

    if (status == MB_SUCCESS && *platform_ptr == NULL) {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_SYSTEM;
    }
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    platform = (struct mb_platform_struct *)*platform_ptr;
    fprintf(stderr, "dbg2       platform:                 %p\n", platform);
    fprintf(stderr, "dbg2       platform->type:             %d\n", platform->type);
    fprintf(stderr, "dbg2       platform->name:             %s\n", platform->name);
    fprintf(stderr, "dbg2       platform->organization:         %s\n", platform->organization);
    fprintf(stderr, "dbg2       platform->source_bathymetry1:    %d\n", platform->source_bathymetry1);
    fprintf(stderr, "dbg2       platform->source_bathymetry2:    %d\n", platform->source_bathymetry2);
    fprintf(stderr, "dbg2       platform->source_bathymetry3:    %d\n", platform->source_bathymetry3);
    fprintf(stderr, "dbg2       platform->source_backscatter1:    %d\n", platform->source_backscatter1);
    fprintf(stderr, "dbg2       platform->source_backscatter2:    %d\n", platform->source_backscatter2);
    fprintf(stderr, "dbg2       platform->source_backscatter3:    %d\n", platform->source_backscatter3);
    fprintf(stderr, "dbg2       platform->source_position1:    %d\n", platform->source_position1);
    fprintf(stderr, "dbg2       platform->source_position2:    %d\n", platform->source_position2);
    fprintf(stderr, "dbg2       platform->source_position3:    %d\n", platform->source_position3);
    fprintf(stderr, "dbg2       platform->source_depth1:      %d\n", platform->source_depth1);
    fprintf(stderr, "dbg2       platform->source_depth2:      %d\n", platform->source_depth2);
    fprintf(stderr, "dbg2       platform->source_depth3:      %d\n", platform->source_depth3);
    fprintf(stderr, "dbg2       platform->source_heading1:      %d\n", platform->source_heading1);
    fprintf(stderr, "dbg2       platform->source_heading2:      %d\n", platform->source_heading2);
    fprintf(stderr, "dbg2       platform->source_heading3:      %d\n", platform->source_heading3);
    fprintf(stderr, "dbg2       platform->source_rollpitch1:    %d\n", platform->source_rollpitch1);
    fprintf(stderr, "dbg2       platform->source_rollpitch2:    %d\n", platform->source_rollpitch2);
    fprintf(stderr, "dbg2       platform->source_rollpitch3:    %d\n", platform->source_rollpitch3);
    fprintf(stderr, "dbg2       platform->source_heave1:      %d\n", platform->source_heave1);
    fprintf(stderr, "dbg2       platform->source_heave2:      %d\n", platform->source_heave2);
    fprintf(stderr, "dbg2       platform->source_heave3:      %d\n", platform->source_heave3);
    fprintf(stderr, "dbg2       platform->num_sensors:         %d\n", platform->num_sensors);
    for (int i = 0; i < platform->num_sensors; i++) {
      fprintf(stderr, "dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
      fprintf(stderr, "dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
      fprintf(stderr, "dbg2       platform->sensors[%2d].manufacturer:         %s\n", i, platform->sensors[i].manufacturer);
      fprintf(stderr, "dbg2       platform->sensors[%2d].serialnumber:         %s\n", i, platform->sensors[i].serialnumber);
      fprintf(stderr, "dbg2       platform->sensors[%2d].capability1:          %d\n", i, platform->sensors[i].capability1);
      fprintf(stderr, "dbg2       platform->sensors[%2d].capability2:          %d\n", i, platform->sensors[i].capability2);
      fprintf(stderr, "dbg2       platform->sensors[%2d].num_offsets:          %d\n", i, platform->sensors[i].num_offsets);
      for (int j = 0; j < platform->sensors[i].num_offsets; j++) {
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:          %d\n", i, j,
                platform->sensors[i].offsets[j].position_offset_mode);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:          %f\n", i, j,
                platform->sensors[i].offsets[j].position_offset_x);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:          %f\n", i, j,
                platform->sensors[i].offsets[j].position_offset_y);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:          %f\n", i, j,
                platform->sensors[i].offsets[j].position_offset_z);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:          %d\n", i, j,
                platform->sensors[i].offsets[j].attitude_offset_mode);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_heading:      %f\n", i, j,
                platform->sensors[i].offsets[j].attitude_offset_heading);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:          %f\n", i, j,
                platform->sensors[i].offsets[j].attitude_offset_roll);
        fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:      %f\n", i, j,
                platform->sensors[i].offsets[j].attitude_offset_pitch);
      }
      fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency_mode:  %d\n", i,
              platform->sensors[i].time_latency_mode);
      fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency_static:  %f\n", i,
              platform->sensors[i].time_latency_static);
      fprintf(stderr, "dbg2       platform->sensors[%2d].num_time_latency:    %d\n", i,
              platform->sensors[i].num_time_latency);
      for (int j = 0; j < platform->sensors[i].num_time_latency; j++) {
        fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency[%2d]:    %16.6f %8.6f\n", i, j,
                platform->sensors[i].time_latency_time_d[j], platform->sensors[i].time_latency_value[j]);
      }
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_sensorhead(int verbose, void *mbio_ptr, void *store_ptr, int *sensorhead, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* reset error value */
  *error = MB_ERROR_NO_ERROR;

  /* call the appropriate mbsys_ sensorhead routine
          defined for:
            mb_io_ptr->system == MB_SYS_WISSL */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_sensorhead != NULL) {
    if (store_ptr == NULL)
      store_ptr = (void *)mb_io_ptr->store_data;
    if (store_ptr != NULL)
      status = (*mb_io_ptr->mb_io_sensorhead)(verbose, mbio_ptr, store_ptr, sensorhead, error);
  }

  /* else set error so calling function knows to use timestamp comparison
     - calling function should immediately reset the error */
  else {
    *sensorhead = 0;
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_FORMAT;
  }

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
int mb_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
               double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag, double *bath,
               double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
               double *ssalongtrack, char *comment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract != NULL) {
    status = (*mb_io_ptr->mb_io_extract)(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
                                         nbath, namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                                         ssacrosstrack, ssalongtrack, comment, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  /* apply projection and lonflip if necessary */
  if (status == MB_SUCCESS) {
    /* apply inverse projection if required */
    if (mb_io_ptr->projection_initialized) {
      const double easting = *navlon;
      const double northing = *navlat;
      mb_proj_inverse(verbose, mb_io_ptr->pjptr, easting, northing, navlon, navlat, error);
    }

    /* apply lonflip */
    if (mb_io_ptr->lonflip < 0) {
      if (*navlon > 0.)
        *navlon = *navlon - 360.;
      else if (*navlon < -360.)
        *navlon = *navlon + 360.;
    }
    else if (mb_io_ptr->lonflip == 0) {
      if (*navlon > 180.)
        *navlon = *navlon - 360.;
      else if (*navlon < -180.)
        *navlon = *navlon + 360.;
    }
    else {
      if (*navlon > 360.)
        *navlon = *navlon - 360.;
      else if (*navlon < 0.)
        *navlon = *navlon + 360.;
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
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_lonlat(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
               double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag, double *bath,
               double *amp, double *bathlon, double *bathlat, double *ss, double *sslon,
               double *sslat, char *comment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  double *bathacrosstrack = bathlon;
  double *bathalongtrack = bathlat;
  double *ssacrosstrack = sslon;
  double *ssalongtrack = sslat;
  if (mb_io_ptr->mb_io_extract != NULL) {
    status = (*mb_io_ptr->mb_io_extract)(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
                                         nbath, namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                                         ssacrosstrack, ssalongtrack, comment, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  /* apply projection and lonflip if necessary */
  if (status == MB_SUCCESS) {
    /* apply inverse projection if required */
    if (mb_io_ptr->projection_initialized) {
      const double easting = *navlon;
      const double northing = *navlat;
      mb_proj_inverse(verbose, mb_io_ptr->pjptr, easting, northing, navlon, navlat, error);
    }

    /* apply lonflip */
    if (mb_io_ptr->lonflip < 0) {
      if (*navlon > 0.)
        *navlon = *navlon - 360.;
      else if (*navlon < -360.)
        *navlon = *navlon + 360.;
    }
    else if (mb_io_ptr->lonflip == 0) {
      if (*navlon > 180.)
        *navlon = *navlon - 360.;
      else if (*navlon < -180.)
        *navlon = *navlon + 360.;
    }
    else {
      if (*navlon > 360.)
        *navlon = *navlon - 360.;
      else if (*navlon < 0.)
        *navlon = *navlon + 360.;
    }

    /* translate beam and pixel locations to lon lat */
    double headingx = sin(DTR * (*heading));
    double headingy = cos(DTR * (*heading));

    /* get coordinate scaling */
    double mtodeglon, mtodeglat;
    mb_coor_scale(verbose, *navlat, &mtodeglon, &mtodeglat);

    /* get lon lat beams and pixels */
    for (int i = 0; i < *nbath; i++) {
      if (beamflag[i] != MB_FLAG_NULL) {
        double bathxt = bathacrosstrack[i];
        double bathlt = bathalongtrack[i];
        bathlon[i] = *navlon + headingy * mtodeglon * bathxt +
                     headingx * mtodeglon * bathlt;
        bathlat[i] = *navlat - headingx * mtodeglat * bathxt +
                     headingy * mtodeglat * bathlt;
      }
    }
    for (int i = 0; i < *nss; i++) {
      if (ss[i] > MB_SIDESCAN_NULL) {
        double ssxt = ssacrosstrack[i];
        double sslt = ssalongtrack[i];
        sslon[i] = *navlon + headingy * mtodeglon * ssxt +
                    + headingx * mtodeglon * sslt;
        sslat[i] = *navlat - headingx * mtodeglat * ssxt +
                    + headingy * mtodeglat * sslt;
      }
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
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon, double navlat,
              double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath, double *amp,
              double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack, double *ssalongtrack,
              char *comment, int *error) {
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

  /* check that io arrays are large enough, allocate larger arrays if necessary */
  int status = MB_SUCCESS;
  if (nbath > mb_io_ptr->beams_bath_alloc || namp > mb_io_ptr->beams_amp_alloc || nss > mb_io_ptr->pixels_ss_alloc) {
    status &= mb_update_arrays(verbose, mbio_ptr, nbath, namp, nss, error);
  }
  mb_io_ptr->beams_bath_max = MAX(mb_io_ptr->beams_bath_max, nbath);
  mb_io_ptr->beams_amp_max = MAX(mb_io_ptr->beams_amp_max, namp);
  mb_io_ptr->pixels_ss_max = MAX(mb_io_ptr->pixels_ss_max, nss);

  /* apply inverse projection if required */
  if (mb_io_ptr->projection_initialized) {
    double easting;
    double northing;
    mb_proj_forward(verbose, mb_io_ptr->pjptr, navlon, navlat, &easting, &northing, error);
    navlon = easting;
    navlat = northing;
  }

  /* call the appropriate mbsys_ insertion routine */
  if (mb_io_ptr->mb_io_insert != NULL) {
    status &= (*mb_io_ptr->mb_io_insert)(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
                                        nbath, namp, nss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
                                        ssacrosstrack, ssalongtrack, comment, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                   double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch, double *heave,
                   int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_nav != NULL) {
    status = (*mb_io_ptr->mb_io_extract_nav)(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed,
                                             heading, draft, roll, pitch, heave, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  /* apply projection and lonflip if necessary */
  if (status == MB_SUCCESS) {
    /* apply inverse projection if required */
    if (mb_io_ptr->projection_initialized) {
      const double easting = *navlon;
      const double northing = *navlat;
      mb_proj_inverse(verbose, mb_io_ptr->pjptr, easting, northing, navlon, navlat, error);
    }

    /* apply lonflip */
    if (mb_io_ptr->lonflip < 0) {
      if (*navlon > 0.)
        *navlon = *navlon - 360.;
      else if (*navlon < -360.)
        *navlon = *navlon + 360.;
    }
    else if (mb_io_ptr->lonflip == 0) {
      if (*navlon > 180.)
        *navlon = *navlon - 360.;
      else if (*navlon < -180.)
        *navlon = *navlon + 360.;
    }
    else {
      if (*navlon > 360.)
        *navlon = *navlon - 360.;
      else if (*navlon < 0.)
        *navlon = *navlon + 360.;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
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
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i, double *time_d,
                    double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll, double *pitch,
                    double *heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_nnav != NULL) {
    status = (*mb_io_ptr->mb_io_extract_nnav)(verbose, mbio_ptr, store_ptr, nmax, kind, n, time_i, time_d, navlon, navlat,
                                              speed, heading, draft, roll, pitch, heave, error);
  }
  else if (mb_io_ptr->mb_io_extract_nav != NULL) {
    status = (*mb_io_ptr->mb_io_extract_nav)(verbose, mbio_ptr, store_ptr, kind, time_i, time_d, navlon, navlat, speed,
                                             heading, draft, roll, pitch, heave, error);
    if (status == MB_SUCCESS)
      *n = 1;
    else
      *n = 0;
  }
  else {
    *n = 0;
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  /* if call was made for an unsupported record type ("kind") then try to get the
      values out of the asynchronous data interpolation buffers
      The time stamp must be sensible for this to work. */
  if (status == MB_FAILURE && *error == MB_ERROR_OTHER && *time_d > 0.0) {
    /* get number of available navigation values */
    *n = 1;

    /* get heading */
    status = mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);

    /* get longitude, latitude, and speed */
    *speed = 0.0;
    if (status == MB_SUCCESS)
      status = mb_navint_interp(verbose, mbio_ptr, *time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get roll pitch and heave */
    if (status == MB_SUCCESS)
      status = mb_attint_interp(verbose, mbio_ptr, *time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);

    /* get draft  */
    if (status == MB_SUCCESS)
      status = mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);

    /* get roll pitch and heave */
    if (status == MB_SUCCESS)
      status = mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
  }

  /* apply projection and lonflip if necessary */
  if (status == MB_SUCCESS) {
    for (int inav = 0; inav < *n; inav++) {
      /* apply inverse projection if required */
      if (mb_io_ptr->projection_initialized) {
        double easting;
        double northing;
        easting = navlon[inav];
        northing = navlat[inav];
        mb_proj_inverse(verbose, mb_io_ptr->pjptr, easting, northing, &(navlon[inav]), &(navlat[inav]), error);
      }

      /* apply lonflip */
      if (mb_io_ptr->lonflip < 0) {
        if (navlon[inav] > 0.)
          navlon[inav] = navlon[inav] - 360.;
        else if (navlon[inav] < -360.)
          navlon[inav] = navlon[inav] + 360.;
      }
      else if (mb_io_ptr->lonflip == 0) {
        if (navlon[inav] > 180.)
          navlon[inav] = navlon[inav] - 360.;
        else if (navlon[inav] < -180.)
          navlon[inav] = navlon[inav] + 360.;
      }
      else {
        if (navlon[inav] > 360.)
          navlon[inav] = navlon[inav] - 360.;
        else if (navlon[inav] < 0.)
          navlon[inav] = navlon[inav] + 360.;
      }
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       n:          %d\n", *n);
    for (int inav = 0; inav < *n; inav++) {
      for (int i = 0; i < 7; i++)
        fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i, time_i[inav * 7 + i]);
      fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
      fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
      fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
      fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
      fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
      fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
      fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
      fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
      fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
    }
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon, double navlat,
                  double speed, double heading, double draft, double roll, double pitch, double heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:        %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:     %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", time_d);
    fprintf(stderr, "dbg2       longitude:     %f\n", navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", speed);
    fprintf(stderr, "dbg2       heading:       %f\n", heading);
    fprintf(stderr, "dbg2       draft:         %f\n", draft);
    fprintf(stderr, "dbg2       roll:          %f\n", roll);
    fprintf(stderr, "dbg2       pitch:         %f\n", pitch);
    fprintf(stderr, "dbg2       heave:         %f\n", heave);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* apply inverse projection if required */
  if (mb_io_ptr->projection_initialized) {
    double easting;
    double northing;
    mb_proj_forward(verbose, mb_io_ptr->pjptr, navlon, navlat, &easting, &northing, error);
    navlon = easting;
    navlat = northing;
  }

  /* call the appropriate mbsys_ insertion routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_insert_nav != NULL) {
    status = (*mb_io_ptr->mb_io_insert_nav)(verbose, mbio_ptr, store_ptr, time_i, time_d, navlon, navlat, speed, heading,
                                            draft, roll, pitch, heave, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth, double *altitude,
                        int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_altitude != NULL) {
    status = (*mb_io_ptr->mb_io_extract_altitude)(verbose, mbio_ptr, store_ptr, kind, transducer_depth, altitude, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_insert_altitude(int verbose, void *mbio_ptr, void *store_ptr, double transducer_depth, double altitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       transducer_depth:  %f\n", transducer_depth);
    fprintf(stderr, "dbg2       altitude:          %f\n", altitude);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ insertion routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_insert_altitude != NULL) {
    status = (*mb_io_ptr->mb_io_insert_altitude)(verbose, mbio_ptr, store_ptr, transducer_depth, altitude, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                   int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_svp != NULL) {
    status = (*mb_io_ptr->mb_io_extract_svp)(verbose, mbio_ptr, store_ptr, kind, nsvp, depth, velocity, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
    for (int i = 0; i < *nsvp; i++)
      fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       nsvp:              %d\n", nsvp);
    for (int i = 0; i < nsvp; i++)
      fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ insertion routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_insert_svp != NULL) {
    status = (*mb_io_ptr->mb_io_insert_svp)(verbose, mbio_ptr, store_ptr, nsvp, depth, velocity, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
/*
 * mb_ttimes() calls the appropriate mbsys_ routine for
 * extracting travel times and  beam angles from
 * a stored survey data ping.
 *
 * The coordinates of the beam angles can be a bit confusing.
 * The angles are returned in "takeoff angle coordinates"
 * appropriate for raytracing. The array angles contains the
 * angle from vertical (theta below) and the array angles_forward
 * contains the angle from acrosstrack (phi below). This
 * coordinate system is distinct from the roll-pitch coordinates
 * appropriate for correcting roll and pitch values. The following
 * is a description of these relevant coordinate systems:
 *
 * Notes on Coordinate Systems used in MB-System
 *
 * David W. Caress
 * April 22, 1996
 * R/V Maurice Ewing, EW9602
 *
 * I. Introduction
 * ---------------
 * The coordinate systems described below are used
 * within MB-System for calculations involving
 * the location in space of depth, amplitude, or
 * sidescan data. In all cases the origin of the
 * coordinate system is at the center of the sonar
 * transducers.
 *
 * II. Cartesian Coordinates
 * -------------------------
 * The cartesian coordinate system used in MB-System
 * is a bit odd because it is left-handed, as opposed
 * to the right-handed x-y-z space conventionally
 * used in most circumstances. With respect to the
 * sonar (or the ship on which the sonar is mounted),
 * the x-axis is athwartships with positive to starboard
 * (to the right if facing forward), the y-axis is
 * fore-aft with positive forward, and the z-axis is
 * positive down.
 *
 * III. Spherical Coordinates
 * --------------------------
 * There are two non-traditional spherical coordinate
 * systems used in MB-System. The first, referred to here
 * as takeoff angle coordinates, is useful for raytracing.
 * The second, referred to here as roll-pitch
 * coordinates, is useful for taking account of
 * corrections to roll and pitch angles.
 *
 * 1. Takeoff Angle Coordinates
 * ----------------------------
 * The three parameters are r, theta, and phi, where
 * r is the distance from the origin, theta is the
 * angle from vertical down (that is, from the
 * positive z-axis), and phi is the angle from
 * acrosstrack (the positive x-axis) in the x-y plane.
 * Note that theta is always positive; the direction
 * in the x-y plane is given by phi.
 * Raytracing is simple in these coordinates because
 * the ray takeoff angle is just theta. However,
 * applying roll or pitch corrections is complicated because
 * roll and pitch have components in both theta and phi.
 *
 *   0 <= theta <= PI/2
 *   -PI/2 <= phi <= 3*PI/2
 *
 *   x = r * SIN(theta) * COS(phi)
 *   y = r * SIN(theta) * SIN(phi)
 *   z = r * COS(theta)
 *
 *   theta = 0    ---> vertical, along positive z-axis
 *   theta = PI/2 ---> horizontal, in x-y plane
 *   phi = -PI/2  ---> aft, in y-z plane with y negative
 *   phi = 0      ---> port, in x-z plane with x positive
 *   phi = PI/2   ---> forward, in y-z plane with y positive
 *   phi = PI     ---> starboard, in x-z plane with x negative
 *   phi = 3*PI/2 ---> aft, in y-z plane with y negative
 *
 * 2. Roll-Pitch Coordinates
 * -------------------------
 * The three parameters are r, alpha, and beta, where
 * r is the distance from the origin, alpha is the angle
 * forward (effectively pitch angle), and beta is the
 * angle from horizontal in the x-z plane (effectively
 * roll angle). Applying a roll or pitch correction is
 * simple in these coordinates because pitch is just alpha
 * and roll is just beta. However, raytracing is complicated
 * because deflection from vertical has components in both
 * alpha and beta.
 *
 *   -PI/2 <= alpha <= PI/2
 *   0 <= beta <= PI
 *
 *   x = r * COS(alpha) * COS(beta)
 *   y = r * SIN(alpha)
 *   z = r * COS(alpha) * SIN(beta)
 *
 *   alpha = -PI/2 ---> horizontal, in x-y plane with y negative
 *   alpha = 0     ---> ship level, zero pitch, in x-z plane
 *   alpha = PI/2  ---> horizontal, in x-y plane with y positive
 *   beta = 0      ---> starboard, along positive x-axis
 *   beta = PI/2   ---> in y-z plane rotated by alpha
 *   beta = PI     ---> port, along negative x-axis
 *
 * IV. SeaBeam Coordinates
 * ----------------------
 * The per-beam parameters in the SB2100 data format include
 * angle-from-vertical and angle-forward. Angle-from-vertical
 * is the same as theta except that it is signed based on
 * the acrosstrack direction (positive to starboard, negative
 * to port). The angle-forward values are also defined
 * slightly differently from phi, in that angle-forward is
 * signed differently on the port and starboard sides. The
 * SeaBeam 2100 External Interface Specifications document
 * includes both discussion and figures illustrating the
 * angle-forward value. To summarize:
 *
 *     Port:
 *
 *   theta = absolute value of angle-from-vertical
 *
 *   -PI/2 <= phi <= PI/2
 *   is equivalent to
 *   -PI/2 <= angle-forward <= PI/2
 *
 *   phi = -PI/2 ---> angle-forward = -PI/2 (aft)
 *   phi = 0     ---> angle-forward = 0     (starboard)
 *   phi = PI/2  ---> angle-forward = PI/2  (forward)
 *
 *     Starboard:
 *
 *   theta = angle-from-vertical
 *
 *   PI/2 <= phi <= 3*PI/2
 *   is equivalent to
 *   -PI/2 <= angle-forward <= PI/2
 *
 *   phi = PI/2   ---> angle-forward = -PI/2 (forward)
 *   phi = PI     ---> angle-forward = 0     (port)
 *   phi = 3*PI/2 ---> angle-forward = PI/2  (aft)
 *
 * V. Usage of Coordinate Systems in MB-System
 * ------------------------------------------
 * Some sonar data formats provide angle values along with
 * travel times. The angles are converted to takoff-angle
 * coordinates regardless of the  storage form of the
 * particular data format. Currently, most data formats
 * do not contain an alongtrack component to the position
 * values; in these cases the conversion is trivial since
 * phi = beta = 0 and theta = alpha. The angle and travel time
 * values can be accessed using the MBIO function mb_ttimes.
 * All angle values passed by MB-System functions are in
 * degrees rather than radians.
 *
 * The programs mbbath and mbvelocitytool use angles in
 * take-off angle coordinates to do the raytracing. If roll
 * and/or pitch corrections are to be made, the angles are
 * converted to roll-pitch coordinates, corrected, and then
 * converted back prior to raytracing.
 *
 * The SeaBeam patch test tool SeaPatch calculates angles
 * in roll-pitch coordinates from the initial bathymetry
 * and then applies whatever roll and pitch corrections are
 * set interactively by the user.
 *
 *
 */
/*--------------------------------------------------------------------*/
int mb_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
              double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft, double *ssv,
              int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_ttimes != NULL) {
    status = (*mb_io_ptr->mb_io_ttimes)(verbose, mbio_ptr, store_ptr, kind, nbeams, ttimes, angles, angles_forward,
                                        angles_null, heave, alongtrack_offset, draft, ssv, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
      fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  heave:%f  ltrk_off:%f\n", i,
              ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_detects != NULL) {
    status = (*mb_io_ptr->mb_io_detects)(verbose, mbio_ptr, store_ptr, kind, nbeams, detects, error);
  }
  else {
    for (int i = 0; i < *nbeams; i++)
      detects[i] = MB_DETECT_UNKNOWN;
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
int mb_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_pulses != NULL) {
    status = (*mb_io_ptr->mb_io_pulses)(verbose, mbio_ptr, store_ptr, kind, nbeams, pulses, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: pulses:%d\n", i, pulses[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
             double *receive_gain, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_gains != NULL) {
    status = (*mb_io_ptr->mb_io_gains)(verbose, mbio_ptr, store_ptr, kind, transmit_gain, pulse_length, receive_gain, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
    fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
    fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
  int mb_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_makess != NULL) {
    status = (*mb_io_ptr->mb_io_makess)(verbose, mbio_ptr, store_ptr, pixel_size_set, pixel_size,
                           swath_width_set, swath_width, pixel_int, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_rawssdimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *sample_interval,
                               int *num_samples_port, int *num_samples_stbd, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_rawssdimensions != NULL) {
    status = (*mb_io_ptr->mb_io_extract_rawssdimensions)(verbose, mbio_ptr, store_ptr, kind, sample_interval,
                                                         num_samples_port, num_samples_stbd, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       sample_interval:   %lf\n", *sample_interval);
    fprintf(stderr, "dbg2       num_samples_port:  %d\n", *num_samples_port);
    fprintf(stderr, "dbg2       num_samples_stbd:  %d\n", *num_samples_stbd);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_rawss(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *sidescan_type, double *sample_interval,
                     double *beamwidth_xtrack, double *beamwidth_ltrack, int *num_samples_port, double *rawss_port,
                     int *num_samples_stbd, double *rawss_stbd, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_rawss != NULL) {
    status = (*mb_io_ptr->mb_io_extract_rawss)(verbose, mbio_ptr, store_ptr, kind, sidescan_type, sample_interval,
                                               beamwidth_xtrack, beamwidth_ltrack, num_samples_port, rawss_port,
                                               num_samples_stbd, rawss_stbd, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       sidescan_type:     %d\n", *sidescan_type);
    fprintf(stderr, "dbg2       sample_interval:   %lf\n", *sample_interval);
    fprintf(stderr, "dbg2       beamwidth_xtrack:  %lf\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:  %lf\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       num_samples_port:  %d\n", *num_samples_port);
    for (int i = 0; i < *num_samples_port; i++)
      fprintf(stderr, "dbg2       sample: %d  rawss_port:%f\n", i, rawss_port[i]);
    fprintf(stderr, "dbg2       num_samples_stbd:  %d\n", *num_samples_stbd);
    for (int i = 0; i < *num_samples_stbd; i++)
      fprintf(stderr, "dbg2       sample: %d  rawss_stbd:%f\n", i, rawss_stbd[i]);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_insert_rawss(int verbose, void *mbio_ptr, void *store_ptr, int kind, int sidescan_type, double sample_interval,
                    double beamwidth_xtrack, double beamwidth_ltrack, int num_samples_port, double *rawss_port,
                    int num_samples_stbd, double *rawss_stbd, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:              %d\n", kind);
    fprintf(stderr, "dbg2       sidescan_type:     %d\n", sidescan_type);
    fprintf(stderr, "dbg2       sample_interval:   %lf\n", sample_interval);
    fprintf(stderr, "dbg2       beamwidth_xtrack:  %lf\n", beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:  %lf\n", beamwidth_ltrack);
    fprintf(stderr, "dbg2       num_samples_port:  %d\n", num_samples_port);
    for (int i = 0; i < num_samples_port; i++)
      fprintf(stderr, "dbg2       sample: %d  rawss_port:%f\n", i, rawss_port[i]);
    fprintf(stderr, "dbg2       num_samples_stbd:  %d\n", num_samples_stbd);
    for (int i = 0; i < num_samples_stbd; i++)
      fprintf(stderr, "dbg2       sample: %d  rawss_stbd:%f\n", i, rawss_stbd[i]);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ insertion routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_insert_rawss != NULL) {
    status =
        (*mb_io_ptr->mb_io_insert_rawss)(verbose, mbio_ptr, store_ptr, kind, sidescan_type, sample_interval, beamwidth_xtrack,
                                         beamwidth_ltrack, num_samples_port, rawss_port, num_samples_stbd, rawss_stbd, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segytraceheader_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:           %d\n", *kind);
    fprintf(stderr, "dbg2       segytraceheader_ptr: %p\n", (void *)segytraceheader_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mb_segytraceheader_struct *mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segytraceheader_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_segy != NULL) {
    status = (*mb_io_ptr->mb_io_extract_segytraceheader)(verbose, mbio_ptr, store_ptr, kind, segytraceheader_ptr, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       seq_num:           %d\n", mb_segytraceheader_ptr->seq_num);
    fprintf(stderr, "dbg2       seq_reel:          %d\n", mb_segytraceheader_ptr->seq_reel);
    fprintf(stderr, "dbg2       shot_num:          %d\n", mb_segytraceheader_ptr->shot_num);
    fprintf(stderr, "dbg2       shot_tr:           %d\n", mb_segytraceheader_ptr->shot_tr);
    fprintf(stderr, "dbg2       espn:              %d\n", mb_segytraceheader_ptr->espn);
    fprintf(stderr, "dbg2       rp_num:            %d\n", mb_segytraceheader_ptr->rp_num);
    fprintf(stderr, "dbg2       rp_tr:             %d\n", mb_segytraceheader_ptr->rp_tr);
    fprintf(stderr, "dbg2       trc_id:            %d\n", mb_segytraceheader_ptr->trc_id);
    fprintf(stderr, "dbg2       num_vstk:          %d\n", mb_segytraceheader_ptr->num_vstk);
    fprintf(stderr, "dbg2       cdp_fold:          %d\n", mb_segytraceheader_ptr->cdp_fold);
    fprintf(stderr, "dbg2       use:               %d\n", mb_segytraceheader_ptr->use);
    fprintf(stderr, "dbg2       range:             %d\n", mb_segytraceheader_ptr->range);
    fprintf(stderr, "dbg2       grp_elev:          %d\n", mb_segytraceheader_ptr->grp_elev);
    fprintf(stderr, "dbg2       src_elev:          %d\n", mb_segytraceheader_ptr->src_elev);
    fprintf(stderr, "dbg2       src_depth:         %d\n", mb_segytraceheader_ptr->src_depth);
    fprintf(stderr, "dbg2       grp_datum:         %d\n", mb_segytraceheader_ptr->grp_datum);
    fprintf(stderr, "dbg2       src_datum:         %d\n", mb_segytraceheader_ptr->src_datum);
    fprintf(stderr, "dbg2       src_wbd:           %d\n", mb_segytraceheader_ptr->src_wbd);
    fprintf(stderr, "dbg2       grp_wbd:           %d\n", mb_segytraceheader_ptr->grp_wbd);
    fprintf(stderr, "dbg2       elev_scalar:       %d\n", mb_segytraceheader_ptr->elev_scalar);
    fprintf(stderr, "dbg2       coord_scalar:      %d\n", mb_segytraceheader_ptr->coord_scalar);
    fprintf(stderr, "dbg2       src_long:          %d\n", mb_segytraceheader_ptr->src_long);
    fprintf(stderr, "dbg2       src_lat:           %d\n", mb_segytraceheader_ptr->src_lat);
    fprintf(stderr, "dbg2       grp_long:          %d\n", mb_segytraceheader_ptr->grp_long);
    fprintf(stderr, "dbg2       grp_lat:           %d\n", mb_segytraceheader_ptr->grp_lat);
    fprintf(stderr, "dbg2       coord_units:       %d\n", mb_segytraceheader_ptr->coord_units);
    fprintf(stderr, "dbg2       wvel:              %d\n", mb_segytraceheader_ptr->wvel);
    fprintf(stderr, "dbg2       sbvel:             %d\n", mb_segytraceheader_ptr->sbvel);
    fprintf(stderr, "dbg2       src_up_vel:        %d\n", mb_segytraceheader_ptr->src_up_vel);
    fprintf(stderr, "dbg2       grp_up_vel:        %d\n", mb_segytraceheader_ptr->grp_up_vel);
    fprintf(stderr, "dbg2       src_static:        %d\n", mb_segytraceheader_ptr->src_static);
    fprintf(stderr, "dbg2       grp_static:        %d\n", mb_segytraceheader_ptr->grp_static);
    fprintf(stderr, "dbg2       tot_static:        %d\n", mb_segytraceheader_ptr->tot_static);
    fprintf(stderr, "dbg2       laga:              %d\n", mb_segytraceheader_ptr->laga);
    fprintf(stderr, "dbg2       delay_mils:        %d\n", mb_segytraceheader_ptr->delay_mils);
    fprintf(stderr, "dbg2       smute_mils:        %d\n", mb_segytraceheader_ptr->smute_mils);
    fprintf(stderr, "dbg2       emute_mils:        %d\n", mb_segytraceheader_ptr->emute_mils);
    fprintf(stderr, "dbg2       nsamps:            %d\n", mb_segytraceheader_ptr->nsamps);
    fprintf(stderr, "dbg2       si_micros:         %d\n", mb_segytraceheader_ptr->si_micros);
    for (int i = 0; i < 19; i++)
      fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
    fprintf(stderr, "dbg2       year:              %d\n", mb_segytraceheader_ptr->year);
    fprintf(stderr, "dbg2       day_of_yr:         %d\n", mb_segytraceheader_ptr->day_of_yr);
    fprintf(stderr, "dbg2       hour:              %d\n", mb_segytraceheader_ptr->hour);
    fprintf(stderr, "dbg2       min:               %d\n", mb_segytraceheader_ptr->min);
    fprintf(stderr, "dbg2       sec:               %d\n", mb_segytraceheader_ptr->sec);
    fprintf(stderr, "dbg2       mils:              %d\n", mb_segytraceheader_ptr->mils);
    fprintf(stderr, "dbg2       tr_weight:         %d\n", mb_segytraceheader_ptr->tr_weight);
    for (int i = 0; i < 5; i++)
      fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
    fprintf(stderr, "dbg2       delay:             %f\n", mb_segytraceheader_ptr->delay);
    fprintf(stderr, "dbg2       smute_sec:         %f\n", mb_segytraceheader_ptr->smute_sec);
    fprintf(stderr, "dbg2       emute_sec:         %f\n", mb_segytraceheader_ptr->emute_sec);
    fprintf(stderr, "dbg2       si_secs:           %f\n", mb_segytraceheader_ptr->si_secs);
    fprintf(stderr, "dbg2       wbt_secs:          %f\n", mb_segytraceheader_ptr->wbt_secs);
    fprintf(stderr, "dbg2       end_of_rp:         %d\n", mb_segytraceheader_ptr->end_of_rp);
    fprintf(stderr, "dbg2       dummy1:            %f\n", mb_segytraceheader_ptr->dummy1);
    fprintf(stderr, "dbg2       dummy2:            %f\n", mb_segytraceheader_ptr->dummy2);
    fprintf(stderr, "dbg2       dummy3:            %f\n", mb_segytraceheader_ptr->dummy3);
    fprintf(stderr, "dbg2       sensordepthtime:   %f\n", mb_segytraceheader_ptr->sensordepthtime);
    fprintf(stderr, "dbg2       soundspeed:        %f\n", mb_segytraceheader_ptr->soundspeed);
    fprintf(stderr, "dbg2       distance:          %f\n", mb_segytraceheader_ptr->distance);
    fprintf(stderr, "dbg2       dummy7:            %f\n", mb_segytraceheader_ptr->roll);
    fprintf(stderr, "dbg2       dummy8:            %f\n", mb_segytraceheader_ptr->pitch);
    fprintf(stderr, "dbg2       heading:           %f\n", mb_segytraceheader_ptr->heading);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_extract_segy(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind, void *segytraceheader_ptr,
                    float *segydata, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       sampleformat:   %d\n", *sampleformat);
    fprintf(stderr, "dbg2       segytraceheader_ptr: %p\n", (void *)segytraceheader_ptr);
    fprintf(stderr, "dbg2       segydata:       %p\n", (void *)segydata);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mb_segytraceheader_struct *mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segytraceheader_ptr;

  /* call the appropriate mbsys_ extraction routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_extract_segy != NULL) {
    status = (*mb_io_ptr->mb_io_extract_segy)(verbose, mbio_ptr, store_ptr, sampleformat, kind, segytraceheader_ptr, segydata,
                                              error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sampleformat:   %d\n", *sampleformat);
    fprintf(stderr, "dbg2       kind:           %d\n", *kind);
    fprintf(stderr, "dbg2       seq_num:        %d\n", mb_segytraceheader_ptr->seq_num);
    fprintf(stderr, "dbg2       seq_reel:       %d\n", mb_segytraceheader_ptr->seq_reel);
    fprintf(stderr, "dbg2       shot_num:       %d\n", mb_segytraceheader_ptr->shot_num);
    fprintf(stderr, "dbg2       shot_tr:        %d\n", mb_segytraceheader_ptr->shot_tr);
    fprintf(stderr, "dbg2       espn:           %d\n", mb_segytraceheader_ptr->espn);
    fprintf(stderr, "dbg2       rp_num:         %d\n", mb_segytraceheader_ptr->rp_num);
    fprintf(stderr, "dbg2       rp_tr:          %d\n", mb_segytraceheader_ptr->rp_tr);
    fprintf(stderr, "dbg2       trc_id:         %d\n", mb_segytraceheader_ptr->trc_id);
    fprintf(stderr, "dbg2       num_vstk:       %d\n", mb_segytraceheader_ptr->num_vstk);
    fprintf(stderr, "dbg2       cdp_fold:       %d\n", mb_segytraceheader_ptr->cdp_fold);
    fprintf(stderr, "dbg2       use:            %d\n", mb_segytraceheader_ptr->use);
    fprintf(stderr, "dbg2       range:          %d\n", mb_segytraceheader_ptr->range);
    fprintf(stderr, "dbg2       grp_elev:       %d\n", mb_segytraceheader_ptr->grp_elev);
    fprintf(stderr, "dbg2       src_elev:       %d\n", mb_segytraceheader_ptr->src_elev);
    fprintf(stderr, "dbg2       src_depth:      %d\n", mb_segytraceheader_ptr->src_depth);
    fprintf(stderr, "dbg2       grp_datum:      %d\n", mb_segytraceheader_ptr->grp_datum);
    fprintf(stderr, "dbg2       src_datum:      %d\n", mb_segytraceheader_ptr->src_datum);
    fprintf(stderr, "dbg2       src_wbd:        %d\n", mb_segytraceheader_ptr->src_wbd);
    fprintf(stderr, "dbg2       grp_wbd:        %d\n", mb_segytraceheader_ptr->grp_wbd);
    fprintf(stderr, "dbg2       elev_scalar:    %d\n", mb_segytraceheader_ptr->elev_scalar);
    fprintf(stderr, "dbg2       coord_scalar:   %d\n", mb_segytraceheader_ptr->coord_scalar);
    fprintf(stderr, "dbg2       src_long:       %d\n", mb_segytraceheader_ptr->src_long);
    fprintf(stderr, "dbg2       src_lat:        %d\n", mb_segytraceheader_ptr->src_lat);
    fprintf(stderr, "dbg2       grp_long:       %d\n", mb_segytraceheader_ptr->grp_long);
    fprintf(stderr, "dbg2       grp_lat:        %d\n", mb_segytraceheader_ptr->grp_lat);
    fprintf(stderr, "dbg2       coord_units:    %d\n", mb_segytraceheader_ptr->coord_units);
    fprintf(stderr, "dbg2       wvel:           %d\n", mb_segytraceheader_ptr->wvel);
    fprintf(stderr, "dbg2       sbvel:          %d\n", mb_segytraceheader_ptr->sbvel);
    fprintf(stderr, "dbg2       src_up_vel:     %d\n", mb_segytraceheader_ptr->src_up_vel);
    fprintf(stderr, "dbg2       grp_up_vel:     %d\n", mb_segytraceheader_ptr->grp_up_vel);
    fprintf(stderr, "dbg2       src_static:     %d\n", mb_segytraceheader_ptr->src_static);
    fprintf(stderr, "dbg2       grp_static:     %d\n", mb_segytraceheader_ptr->grp_static);
    fprintf(stderr, "dbg2       tot_static:     %d\n", mb_segytraceheader_ptr->tot_static);
    fprintf(stderr, "dbg2       laga:           %d\n", mb_segytraceheader_ptr->laga);
    fprintf(stderr, "dbg2       delay_mils:     %d\n", mb_segytraceheader_ptr->delay_mils);
    fprintf(stderr, "dbg2       smute_mils:     %d\n", mb_segytraceheader_ptr->smute_mils);
    fprintf(stderr, "dbg2       emute_mils:     %d\n", mb_segytraceheader_ptr->emute_mils);
    fprintf(stderr, "dbg2       nsamps:         %d\n", mb_segytraceheader_ptr->nsamps);
    fprintf(stderr, "dbg2       si_micros:      %d\n", mb_segytraceheader_ptr->si_micros);
    for (int i = 0; i < 19; i++)
      fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
    fprintf(stderr, "dbg2       year:           %d\n", mb_segytraceheader_ptr->year);
    fprintf(stderr, "dbg2       day_of_yr:      %d\n", mb_segytraceheader_ptr->day_of_yr);
    fprintf(stderr, "dbg2       hour:           %d\n", mb_segytraceheader_ptr->hour);
    fprintf(stderr, "dbg2       min:            %d\n", mb_segytraceheader_ptr->min);
    fprintf(stderr, "dbg2       sec:            %d\n", mb_segytraceheader_ptr->sec);
    fprintf(stderr, "dbg2       mils:           %d\n", mb_segytraceheader_ptr->mils);
    fprintf(stderr, "dbg2       tr_weight:      %d\n", mb_segytraceheader_ptr->tr_weight);
    for (int i = 0; i < 5; i++)
      fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
    fprintf(stderr, "dbg2       delay:          %f\n", mb_segytraceheader_ptr->delay);
    fprintf(stderr, "dbg2       smute_sec:      %f\n", mb_segytraceheader_ptr->smute_sec);
    fprintf(stderr, "dbg2       emute_sec:      %f\n", mb_segytraceheader_ptr->emute_sec);
    fprintf(stderr, "dbg2       si_secs:        %f\n", mb_segytraceheader_ptr->si_secs);
    fprintf(stderr, "dbg2       wbt_secs:       %f\n", mb_segytraceheader_ptr->wbt_secs);
    fprintf(stderr, "dbg2       end_of_rp:      %d\n", mb_segytraceheader_ptr->end_of_rp);
    fprintf(stderr, "dbg2       dummy1:         %f\n", mb_segytraceheader_ptr->dummy1);
    fprintf(stderr, "dbg2       dummy2:         %f\n", mb_segytraceheader_ptr->dummy2);
    fprintf(stderr, "dbg2       dummy3:         %f\n", mb_segytraceheader_ptr->dummy3);
    fprintf(stderr, "dbg2       sensordepthtime:%f\n", mb_segytraceheader_ptr->sensordepthtime);
    fprintf(stderr, "dbg2       soundspeed:     %f\n", mb_segytraceheader_ptr->soundspeed);
    fprintf(stderr, "dbg2       distance:       %f\n", mb_segytraceheader_ptr->distance);
    fprintf(stderr, "dbg2       dummy7:         %f\n", mb_segytraceheader_ptr->roll);
    fprintf(stderr, "dbg2       dummy8:         %f\n", mb_segytraceheader_ptr->pitch);
    fprintf(stderr, "dbg2       heading:        %f\n", mb_segytraceheader_ptr->heading);
    for (int i = 0; i < mb_segytraceheader_ptr->nsamps; i++)
      fprintf(stderr, "dbg2       sample:%d  data:%f\n", i, segydata[i]);
    fprintf(stderr, "dbg2       error:          %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segytraceheader_ptr, float *segydata,
                   int *error) {
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mb_segytraceheader_struct *mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segytraceheader_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       segytraceheader_ptr: %p\n", (void *)segytraceheader_ptr);
    fprintf(stderr, "dbg2       segydata:       %p\n", (void *)segydata);
    fprintf(stderr, "dbg2       kind:           %d\n", kind);
    fprintf(stderr, "dbg2       seq_num:        %d\n", mb_segytraceheader_ptr->seq_num);
    fprintf(stderr, "dbg2       seq_reel:       %d\n", mb_segytraceheader_ptr->seq_reel);
    fprintf(stderr, "dbg2       shot_num:       %d\n", mb_segytraceheader_ptr->shot_num);
    fprintf(stderr, "dbg2       shot_tr:        %d\n", mb_segytraceheader_ptr->shot_tr);
    fprintf(stderr, "dbg2       espn:           %d\n", mb_segytraceheader_ptr->espn);
    fprintf(stderr, "dbg2       rp_num:         %d\n", mb_segytraceheader_ptr->rp_num);
    fprintf(stderr, "dbg2       rp_tr:          %d\n", mb_segytraceheader_ptr->rp_tr);
    fprintf(stderr, "dbg2       trc_id:         %d\n", mb_segytraceheader_ptr->trc_id);
    fprintf(stderr, "dbg2       num_vstk:       %d\n", mb_segytraceheader_ptr->num_vstk);
    fprintf(stderr, "dbg2       cdp_fold:       %d\n", mb_segytraceheader_ptr->cdp_fold);
    fprintf(stderr, "dbg2       use:            %d\n", mb_segytraceheader_ptr->use);
    fprintf(stderr, "dbg2       range:          %d\n", mb_segytraceheader_ptr->range);
    fprintf(stderr, "dbg2       grp_elev:       %d\n", mb_segytraceheader_ptr->grp_elev);
    fprintf(stderr, "dbg2       src_elev:       %d\n", mb_segytraceheader_ptr->src_elev);
    fprintf(stderr, "dbg2       src_depth:      %d\n", mb_segytraceheader_ptr->src_depth);
    fprintf(stderr, "dbg2       grp_datum:      %d\n", mb_segytraceheader_ptr->grp_datum);
    fprintf(stderr, "dbg2       src_datum:      %d\n", mb_segytraceheader_ptr->src_datum);
    fprintf(stderr, "dbg2       src_wbd:        %d\n", mb_segytraceheader_ptr->src_wbd);
    fprintf(stderr, "dbg2       grp_wbd:        %d\n", mb_segytraceheader_ptr->grp_wbd);
    fprintf(stderr, "dbg2       elev_scalar:    %d\n", mb_segytraceheader_ptr->elev_scalar);
    fprintf(stderr, "dbg2       coord_scalar:   %d\n", mb_segytraceheader_ptr->coord_scalar);
    fprintf(stderr, "dbg2       src_long:       %d\n", mb_segytraceheader_ptr->src_long);
    fprintf(stderr, "dbg2       src_lat:        %d\n", mb_segytraceheader_ptr->src_lat);
    fprintf(stderr, "dbg2       grp_long:       %d\n", mb_segytraceheader_ptr->grp_long);
    fprintf(stderr, "dbg2       grp_lat:        %d\n", mb_segytraceheader_ptr->grp_lat);
    fprintf(stderr, "dbg2       coord_units:    %d\n", mb_segytraceheader_ptr->coord_units);
    fprintf(stderr, "dbg2       wvel:           %d\n", mb_segytraceheader_ptr->wvel);
    fprintf(stderr, "dbg2       sbvel:          %d\n", mb_segytraceheader_ptr->sbvel);
    fprintf(stderr, "dbg2       src_up_vel:     %d\n", mb_segytraceheader_ptr->src_up_vel);
    fprintf(stderr, "dbg2       grp_up_vel:     %d\n", mb_segytraceheader_ptr->grp_up_vel);
    fprintf(stderr, "dbg2       src_static:     %d\n", mb_segytraceheader_ptr->src_static);
    fprintf(stderr, "dbg2       grp_static:     %d\n", mb_segytraceheader_ptr->grp_static);
    fprintf(stderr, "dbg2       tot_static:     %d\n", mb_segytraceheader_ptr->tot_static);
    fprintf(stderr, "dbg2       laga:           %d\n", mb_segytraceheader_ptr->laga);
    fprintf(stderr, "dbg2       delay_mils:     %d\n", mb_segytraceheader_ptr->delay_mils);
    fprintf(stderr, "dbg2       smute_mils:     %d\n", mb_segytraceheader_ptr->smute_mils);
    fprintf(stderr, "dbg2       emute_mils:     %d\n", mb_segytraceheader_ptr->emute_mils);
    fprintf(stderr, "dbg2       nsamps:         %d\n", mb_segytraceheader_ptr->nsamps);
    fprintf(stderr, "dbg2       si_micros:      %d\n", mb_segytraceheader_ptr->si_micros);
    for (int i = 0; i < 19; i++)
      fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
    fprintf(stderr, "dbg2       year:           %d\n", mb_segytraceheader_ptr->year);
    fprintf(stderr, "dbg2       day_of_yr:      %d\n", mb_segytraceheader_ptr->day_of_yr);
    fprintf(stderr, "dbg2       hour:           %d\n", mb_segytraceheader_ptr->hour);
    fprintf(stderr, "dbg2       min:            %d\n", mb_segytraceheader_ptr->min);
    fprintf(stderr, "dbg2       sec:            %d\n", mb_segytraceheader_ptr->sec);
    fprintf(stderr, "dbg2       mils:           %d\n", mb_segytraceheader_ptr->mils);
    fprintf(stderr, "dbg2       tr_weight:      %d\n", mb_segytraceheader_ptr->tr_weight);
    for (int i = 0; i < 5; i++)
      fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
    fprintf(stderr, "dbg2       delay:          %f\n", mb_segytraceheader_ptr->delay);
    fprintf(stderr, "dbg2       smute_sec:      %f\n", mb_segytraceheader_ptr->smute_sec);
    fprintf(stderr, "dbg2       emute_sec:      %f\n", mb_segytraceheader_ptr->emute_sec);
    fprintf(stderr, "dbg2       si_secs:        %f\n", mb_segytraceheader_ptr->si_secs);
    fprintf(stderr, "dbg2       wbt_secs:       %f\n", mb_segytraceheader_ptr->wbt_secs);
    fprintf(stderr, "dbg2       end_of_rp:      %d\n", mb_segytraceheader_ptr->end_of_rp);
    fprintf(stderr, "dbg2       dummy1:         %f\n", mb_segytraceheader_ptr->dummy1);
    fprintf(stderr, "dbg2       dummy2:         %f\n", mb_segytraceheader_ptr->dummy2);
    fprintf(stderr, "dbg2       dummy3:         %f\n", mb_segytraceheader_ptr->dummy3);
    fprintf(stderr, "dbg2       sensordepthtime:%f\n", mb_segytraceheader_ptr->sensordepthtime);
    fprintf(stderr, "dbg2       soundspeed:     %f\n", mb_segytraceheader_ptr->soundspeed);
    fprintf(stderr, "dbg2       distance:       %f\n", mb_segytraceheader_ptr->distance);
    fprintf(stderr, "dbg2       roll:           %f\n", mb_segytraceheader_ptr->roll);
    fprintf(stderr, "dbg2       pitch:          %f\n", mb_segytraceheader_ptr->pitch);
    fprintf(stderr, "dbg2       heading:        %f\n", mb_segytraceheader_ptr->heading);
    for (int i = 0; i < mb_segytraceheader_ptr->nsamps; i++)
      fprintf(stderr, "dbg2       sample:%d  data:%f\n", i, segydata[i]);
  }

  /* call the appropriate mbsys_ insertion routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_insert_segy != NULL) {
    status = (*mb_io_ptr->mb_io_insert_segy)(verbose, mbio_ptr, store_ptr, kind, segytraceheader_ptr, segydata, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
           double *temperature, double *depth, double *salinity, double *soundspeed, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  /* note: the arrays should be allocated to MB_CTD_MAX length */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_ctd != NULL) {
    status = (*mb_io_ptr->mb_io_ctd)(verbose, mbio_ptr, store_ptr, kind, nctd, time_d, conductivity, temperature, depth,
                                     salinity, soundspeed, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nctd:          %d\n", *nctd);
    for (int i = 0; i < *nctd; i++) {
      fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
      fprintf(stderr, "dbg2       conductivity:  %f\n", conductivity[i]);
      fprintf(stderr, "dbg2       temperature:   %f\n", temperature[i]);
      fprintf(stderr, "dbg2       depth:         %f\n", depth[i]);
      fprintf(stderr, "dbg2       salinity:      %f\n", salinity[i]);
      fprintf(stderr, "dbg2       soundspeed:    %f\n", soundspeed[i]);
    }
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsensor, double *time_d, double *sensor1,
                        double *sensor2, double *sensor3, double *sensor4, double *sensor5, double *sensor6, double *sensor7,
                        double *sensor8, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call the appropriate mbsys_ extraction routine */
  /* note: the arrays should be allocated to MB_CTD_MAX length */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_ancilliarysensor != NULL) {
    status = (*mb_io_ptr->mb_io_ancilliarysensor)(verbose, mbio_ptr, store_ptr, kind, nsensor, time_d, sensor1, sensor2,
                                                  sensor3, sensor4, sensor5, sensor6, sensor7, sensor8, error);
  }
  else {
    *nsensor = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nsensor:       %d\n", *nsensor);
    for (int i = 0; i < *nsensor; i++) {
      fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
      fprintf(stderr, "dbg2       sensor1:       %f\n", sensor1[i]);
      fprintf(stderr, "dbg2       sensor2:       %f\n", sensor2[i]);
      fprintf(stderr, "dbg2       sensor3:       %f\n", sensor3[i]);
      fprintf(stderr, "dbg2       sensor4:       %f\n", sensor4[i]);
      fprintf(stderr, "dbg2       sensor5:       %f\n", sensor5[i]);
      fprintf(stderr, "dbg2       sensor6:       %f\n", sensor6[i]);
      fprintf(stderr, "dbg2       sensor7:       %f\n", sensor7[i]);
      fprintf(stderr, "dbg2       sensor8:       %f\n", sensor8[i]);
      fprintf(stderr, "dbg2       sensor1:       %f\n", sensor1[i]);
    }
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_copyrecord(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call appropriate memory copy routine */
  int status = MB_SUCCESS;
  if (mb_io_ptr->mb_io_copyrecord != NULL) {
    status = (*mb_io_ptr->mb_io_copyrecord)(verbose, mbio_ptr, store_ptr, copy_ptr, error);
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_SYSTEM;
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
int mb_indextable(int verbose, void *mbio_ptr, int *num_indextable, void **indextable_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:          %p\n", (void *)mbio_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* return index table */
  *num_indextable = mb_io_ptr->num_indextable;
  *indextable_ptr = (void *) mb_io_ptr->indextable;

  /* return no error */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       num_indextable:    %d\n", *num_indextable);
    fprintf(stderr, "dbg2       indextable_ptr:    %p\n", *indextable_ptr);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_indextablefix(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:          %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       num_indextable:    %d\n", num_indextable);
    fprintf(stderr, "dbg2       indextable_ptr:    %p\n", indextable_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call appropriate index table fix routine */
  /* TODO(schwehr): Do we care about any error cases? */
  if (mb_io_ptr->mb_io_indextablefix != NULL) {
    /* status = */ (*mb_io_ptr->mb_io_indextablefix)(verbose, mbio_ptr, num_indextable, indextable_ptr, error);
  }

  /* return no error */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

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
int mb_indextableapply(int verbose, void *mbio_ptr, int num_indextable, void *indextable_ptr, int n_file, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:          %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       num_indextable:    %d\n", num_indextable);
    fprintf(stderr, "dbg2       indextable_ptr:    %p\n", indextable_ptr);
    fprintf(stderr, "dbg2       n_file:            %d\n", n_file);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* call appropriate index table fix routine */
  /* TODO(schwehr): Do we care about any error cases? */
  if (mb_io_ptr->mb_io_indextableapply != NULL) {
    /* status = */ (*mb_io_ptr->mb_io_indextableapply)(verbose, mbio_ptr, num_indextable, indextable_ptr, n_file, error);
  }

  /* return no error */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

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
