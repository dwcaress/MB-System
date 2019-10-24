/*--------------------------------------------------------------------
 *    The MB-system:  mb_proj.c  7/16/2002
  *
 *    Copyright (c) 2002-2019 by
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
 * mb_proj.c includes the "mb_" functions used to initialize
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


// By default use the PROJ6 API
#ifdef USE_PROJ6_API

#include <proj.h>

/*--------------------------------------------------------------------*/
int mb_proj_init(int verbose, char *source, char *target, void **pjptr, int *error) {
  if (verbose >= 0) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:     %d\n", verbose);
    fprintf(stderr, "dbg2       source:      %s\n", source);
    fprintf(stderr, "dbg2       destination: %s\n", destination);
  }

  *error = MB_ERROR_NO_ERROR;
  int status = MB_SUCCESS;

  /* initialize the projection */
  PJ *p = proj_create_crs_to_crs(PJ_DEFAULT_CTX, source, target, 0);
  *pjptr = (void *)p;

  /* check success */
  if (*pjptr == NULL) {
    *error = MB_ERROR_BAD_PROJECTION;
    status = MB_FAILURE;
  }

  if (verbose >= 0) {
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

  /* do forward projection */
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
    fprintf(stderr, "dbg2       easting:         %f\n", *easting);
    fprintf(stderr, "dbg2       northing:        %f\n", *northing);
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
    fprintf(stderr, "dbg2       easting:    %f\n", easting);
    fprintf(stderr, "dbg2       northing:   %f\n", northing);
  }

  /* do inverse projection */
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
    fprintf(stderr, "dbg2       lon:             %f\n", *lon);
    fprintf(stderr, "dbg2       lat:             %f\n", *lat);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

#else // use old PROJ4 API

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
  if (pjptr != NULL) {
    projPJ pj = (projPJ)*pjptr;
    pj_free(pj);
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
int mb_proj_forward(int verbose, void *pjptr, double lon, double lat, double *easting, double *northing, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjptr);
    fprintf(stderr, "dbg2       lon:        %f\n", lon);
    fprintf(stderr, "dbg2       lat:        %f\n", lat);
  }

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
int mb_proj_transform(int verbose, void *pjsrcptr, void *pjdstptr, int npoint, double *x, double *y, double *z, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjsrcptr);
    fprintf(stderr, "dbg2       pjptr:      %p\n", (void *)pjdstptr);
    fprintf(stderr, "dbg2       npoint:     %d\n", npoint);
    for (int i = 0; i < npoint; i++)
      fprintf(stderr, "dbg2       point[%d]:  x:%f y:%f z:%f\n", i, x[i], y[i], z[i]);
  }

  /* do transform */
  if (pjsrcptr != NULL && pjdstptr != NULL) {
    pj_transform((projPJ *)pjsrcptr, (projPJ *)pjdstptr, npoint, 1, x, y, z);
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       npoint:     %d\n", npoint);
    for (int i = 0; i < npoint; i++)
      fprintf(stderr, "dbg2       point[%d]:  x:%f y:%f z:%f\n", i, x[i], y[i], z[i]);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

#endif
