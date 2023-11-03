/*--------------------------------------------------------------------
 *    The MB-system:  mb_proj.c  7/16/2002
  *
 *    Copyright (c) 2002-2023 by
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
/** @file
 * @brief mb_system functions to initialize and transform between projections and
 * geographic coordinates systems.
 * 
 * @details  Declare mb_system functions used to initialize
 * projections, and then to do forward (mb_proj_forward())
 * and inverse (mb_proj_inverse()) projections
 * between geographic coordinates (longitude and latitude) and
 * projected coordinates (e.g. eastings and northings in meters).
 * One can also tranlate between coordinate systems using mb_proj_transform().
 * This code uses libproj. The code in libproj derives without modification
 * from the PROJ.4 distribution. PROJ was originally developed by
 * Gerard Evandim, and is now maintained and distributed by
 * Frank Warmerdam, <warmerdam@pobox.com>
 *
 * David W. Caress
 * July 16, 2002
 * RVIB Nathaniel B. Palmer
 * Somewhere west of Conception, Chile
 *
 * Author:  D. W. Caress
 * Date:  July 16, 2002
 *
 *
 */

#include <math.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
// If necessary use the obsolete PROJ 4 API
#ifdef USE_PROJ4_API
#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H

/*--------------------------------------------------------------------*/

#include "proj_api.h"

#ifndef _WIN32
#include "projections.h"
#else
char *GMT_runtime_bindir_win32(char *result);
#endif

/*--------------------------------------------------------------------*/
int mb_proj_init(int verbose, char *projection, void **pjptr, int *error) {

#ifdef _WIN32
  /* But on Windows get it from the bin dir */
#include <unistd.h>
  char projectionfile[MB_PATH_MAXLINE + 1];

  /* Find the path to the bin directory and from it, the location of the Projections.dat file */
  GMT_runtime_bindir_win32 (projectionfile);
  char *pch = strrchr(projectionfile, '\\');    /* Seek for the last '\' or '/'. One of them must exist. */
  if (pch == NULL)
    pch = strrchr(projectionfile, '/');
  pch[0] = '\0';
  strcat(projectionfile, "\\share\\mbsystem\\Projections.dat");
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       projection: %s\n", projection);
  }
//fprintf(stderr, "%s:%5.5d:%s(verbose=%d, projection=%s)\n",
//__FILE__, __LINE__, __FUNCTION__, verbose, projection);
/* Normally the header file projections.h sets the location of the
  projections.dat file in a string projectionfile, but on Windows instead
  use GMT constructs to find the path to the bin directory and from it,
  the location of the Projections.dat file.
  This construct has been defined by Joaquim Luis. */
#ifdef _WIN32
  GMT_runtime_bindir_win32(projectionfile);
  pch = strrchr(projectionfile, '\\'); /* Seek for the last '\' or '/'. One of them must exist. */
  if (pch == NULL)
    pch = strrchr(projectionfile, '/');
  pch[0] = '\0';
  strcat(projectionfile, "\\share\\mbsystem\\Projections.dat");
#endif

  int status = MB_SUCCESS;

  /* check the existence of the projection database */
  struct stat file_status;
  int fstat = stat(projectionfile, &file_status);
  if (fstat == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {

    mb_path projection_use;
    if (strncmp("EPSG:", projection, 5) == 0) {
      sprintf(projection_use, "epsg%s", (char *)&(projection[5]));
    } else {
      strcpy(projection_use, projection);
    }

    /* initialize the projection */
    char pj_init_args[MB_PATH_MAXLINE];
    sprintf(pj_init_args, "+init=%s:%s", projectionfile, projection_use);
    projPJ pj = pj_init_plus(pj_init_args);
    *pjptr = (void *)pj;

    /* check success */
    if (*pjptr != NULL) {
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }
    else {
      *error = MB_ERROR_BAD_PROJECTION;
      status = MB_FAILURE;
    }
  }
  else {
    /* cannot initialize the projection */
    fprintf(stderr, "\nUnable to open projection database at expected location:\n\t%s\n", projectionfile);
    fprintf(stderr, "Set projection database location using the $MBSYSTEM_HOME and $PROJECTIONS \ntags in the "
                    "install_makefiles script.\n\n");
    *error = MB_ERROR_MISSING_PROJECTIONS;
    status = MB_FAILURE;
  }
//fprintf(stderr, "%s:%5.5d:%s(pjptr=%p, error=%d)\n",
//__FILE__, __LINE__, __FUNCTION__, (void *)*pjptr, *error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pjptr:           %p\n", (void *)*pjptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_free(int verbose, void **pjptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)*pjptr);
  }
//fprintf(stderr, "%s:%5.5d:%s(pjptr=%p, error=%d)\n",
//__FILE__, __LINE__, __FUNCTION__, (void *)*pjptr, *error);

  /* free the projection */
  if (pjptr != NULL) {
    projPJ pj = (projPJ)*pjptr;
    pj_free(pj);
    *pjptr = NULL;
  }
//fprintf(stderr, "%s:%5.5d:%s(pjptr=%p, error=%d)\n",
//__FILE__, __LINE__, __FUNCTION__, (void *)*pjptr, *error);

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pjptr:           %p\n", (void *)*pjptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_forward(int verbose, void *pjptr, double lon, double lat, double *easting, double *northing, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjptr);
    fprintf(stderr, "dbg2       lon:        %f\n", lon);
    fprintf(stderr, "dbg2       lat:        %f\n", lat);
  }
//fprintf(stderr, "%s:%5.5d:%s(pjptr=%p, lon=%f, lat=%f)\n",
//__FILE__, __LINE__, __FUNCTION__, pjptr, lon, lat);

  /* do forward projection */
  if (pjptr != NULL) {
    projPJ pj = (projPJ)pjptr;
    projUV pjll;
    pjll.u = DTR * lon;
    pjll.v = DTR * lat;
    projUV pjxy = pj_fwd(pjll, pj);
    *easting = pjxy.u;
    *northing = pjxy.v;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;
//fprintf(stderr, "%s:%5.5d:%s(easting=%f, northing=%f, error=%d)\n",
//__FILE__, __LINE__, __FUNCTION__, *easting, *northing, *error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       easting:         %f\n", *easting);
    fprintf(stderr, "dbg2       northing:        %f\n", *northing);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_inverse(int verbose, void *pjptr, double easting, double northing, double *lon, double *lat, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjptr);
    fprintf(stderr, "dbg2       easting:    %f\n", easting);
    fprintf(stderr, "dbg2       northing:   %f\n", northing);
  }
//fprintf(stderr, "%s:%5.5d:%s(pjptr=%p, easting=%f, northing=%f)\n",
//__FILE__, __LINE__, __FUNCTION__, pjptr, easting, northing);

  /* do forward projection */
  if (pjptr != NULL) {
    projPJ pj = (projPJ)pjptr;
    projUV pjxy;
    pjxy.u = easting;
    pjxy.v = northing;
    projUV pjll = pj_inv(pjxy, pj);
    *lon = RTD * pjll.u;
    *lat = RTD * pjll.v;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;
//fprintf(stderr, "%s:%5.5d:%s(lon=%f, lat=%f, error=%d)\n",
//__FILE__, __LINE__, __FUNCTION__, *lon, *lat, *error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       lon:             %f\n", *lon);
    fprintf(stderr, "dbg2       lat:             %f\n", *lat);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
/*--------------------------------------------------------------------*/
// Otherwise use the PROJ 6+ API
#else

#include <proj.h>

/*--------------------------------------------------------------------*/
static int mb_proj6_init(int verbose, char *source_crs, char *target_crs, void **pjptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       source_crs: %s\n", source_crs);
    fprintf(stderr, "dbg2       target_crs: %s\n", target_crs);
  }
//fprintf(stderr, "%s:%5.5d:%s(verbose=%d, source_crs=%s, target_crs=%s)\n",
//__FILE__, __LINE__, __FUNCTION__, verbose, source_crs, target_crs);

  *error = MB_ERROR_NO_ERROR;
  int status = MB_SUCCESS;

  // Handle some special cases that may arise with MB-System programs
  mb_path source;
  if (source_crs == NULL) {
      strcpy(source, "EPSG:4326");
  }
  else if (strncmp(source_crs, "epsg:", 5) == 0) {
    strcpy(source, "EPSG:");
    strncpy(&source[5], &source_crs[5], (sizeof(mb_path) - 6));
    source[sizeof(mb_path)-1] = 0;
  }
  else if (strncmp(source_crs, "UTM", 3) == 0) {
    int utm_zone;
    char utm_ns;
    int n = sscanf(source_crs, "UTM%d%c", &utm_zone, &utm_ns);
    if (n == 2 && utm_zone > 0 && utm_zone < 61
      && (utm_ns == 'N' || utm_ns == 'n'
          || utm_ns == 'S' || utm_ns == 's')) {
      int epsg_id;
      if (utm_ns == 'N' || utm_ns == 'n') {
        epsg_id = 32600 + utm_zone;
      }
      else /* if (utm_ns == 'S' || utm_ns == 's') */ {
        epsg_id = 32700 + utm_zone;
      }
      sprintf(source, "EPSG:%4.4d", epsg_id);
    }
  }
  else {
    strncpy(source, source_crs, sizeof(mb_path)-1);
  }
  mb_path target;
  if (strncmp(target_crs, "epsg:", 5) == 0) {
    strcpy(target, "EPSG:");
    strncpy(&target[5], &target_crs[5], (sizeof(mb_path) - 6));
    target[sizeof(mb_path)-1] = 0;
  }
  else if (strncmp(target_crs, "UTM", 3) == 0) {
    int utm_zone;
    char utm_ns;
    int n = sscanf(target_crs, "UTM%d%c", &utm_zone, &utm_ns);
    if (n == 2 && utm_zone > 0 && utm_zone < 61
      && (utm_ns == 'N' || utm_ns == 'n'
          || utm_ns == 'S' || utm_ns == 's')) {
      int epsg_id;
      if (utm_ns == 'N' || utm_ns == 'n') {
        epsg_id = 32600 + utm_zone;
      }
      else /* if (utm_ns == 'S' || utm_ns == 's') */ {
        epsg_id = 32700 + utm_zone;
      }
      sprintf(target, "EPSG:%4.4d", epsg_id);
    }
  }
  else {
    strncpy(target, target_crs, sizeof(mb_path)-1);
  }

  /* initialize the geodetic operation */
  PJ *p = proj_create_crs_to_crs(PJ_DEFAULT_CTX, source, target, 0);
  *pjptr = (void *) proj_normalize_for_visualization(PJ_DEFAULT_CTX, p);
  proj_destroy(p);

  /* check success */
  if (*pjptr == NULL) {
    *error = MB_ERROR_BAD_PROJECTION;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pjptr:           %p\n", (void *)*pjptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_proj_init(int verbose, char *target_crs, void **pjptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       target_crs: %s\n", target_crs);
  }
//fprintf(stderr, "%s:%5.5d:%s(verbose=%d, target_crs=%s)\n",
//__FILE__, __LINE__, __FUNCTION__, verbose, target_crs);

  *error = MB_ERROR_NO_ERROR;
  int status = MB_SUCCESS;

  // The old init function only specified the target CRS as the source CRS is
  // assumed to be longtitude, latitude in the EPSG:4326 / WGS 84 datum.
  // The target CRS is specified by the string target_crs.
  // Here we add the source CRS and call the new init function, which allows
  // transformation between arbritrarily defined CRSs.
  mb_path source_crs = "EPSG:4326";
  status = mb_proj6_init(verbose, source_crs, target_crs, pjptr,  error);

  /* check success */
  if (*pjptr == NULL) {
    *error = MB_ERROR_BAD_PROJECTION;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pjptr:           %p\n", (void *)*pjptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_free(int verbose, void **pjptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)*pjptr);
  }

  /* free the projection */
  PJ *p = NULL;
  if (pjptr != NULL) {
    p = (PJ *)*pjptr;
    proj_destroy(p);
    *pjptr = NULL;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pjptr:           %p\n", (void *)*pjptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_forward(int verbose, void *pjptr, double u, double v, double *uu, double *vv, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjptr);
    fprintf(stderr, "dbg2       u:          %f\n", u);
    fprintf(stderr, "dbg2       v:          %f\n", v);
  }

  /* do forward projection - in MB-System this is usually from lon lat in WGS84
      to easting northing in a projected coordinate system like UTM */
  if (pjptr != NULL) {
    PJ *p = (PJ *) pjptr;
    PJ_COORD c;
    c.v[0] = u;
    c.v[1] = v;
    c = proj_trans(p, PJ_FWD, c);
    *uu = c.v[0];
    *vv = c.v[1];
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       uu:              %f\n", *uu);
    fprintf(stderr, "dbg2       vv:              %f\n", *vv);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_proj_inverse(int verbose, void *pjptr, double u, double v, double *uu, double *vv, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjptr);
    fprintf(stderr, "dbg2       u:          %f\n", u);
    fprintf(stderr, "dbg2       v:          %f\n", v);
  }

  /* do inverse projection - in MB-System this is usually from easting northing
      in a projected coordinate system like UTM to lon lat in WGS84 */
  if (pjptr != NULL) {
    PJ *p = (PJ *) pjptr;
    PJ_COORD c;
    c.v[0] = u;
    c.v[1] = v;
    c = proj_trans(p, PJ_INV, c);
    *uu = c.v[0];
    *vv = c.v[1];
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       uu:              %f\n", *uu);
    fprintf(stderr, "dbg2       vv:              %f\n", *vv);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

#endif
