/*--------------------------------------------------------------------
 *    The MB-system:  mb_readwritegrd.c  12/10/2007
 *
 *    Copyright (c) 2007-2023 by
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
 * Standalone functions to read or write a GMT grid for programs not interfacing
 * with other GMT functionality.
 *
 * Author:  D. W. Caress
 * Date:  September 3, 2007
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "gmt_dev.h"

#ifndef CMAKE_BUILD_SYSTEM
  #ifdef HAVE_SINCOS
    #undef HAVE_SINCOS  // avoid clash between gmt_config.h and the Autotools build system mb_config.h
  #endif
#endif

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_status.h"

/* Projection defines */
enum ModelType {
  ModelTypeProjected = 1,
  ModelTypeGeographic = 2
};
static const int GCS_WGS_84 = 4326;

/*--------------------------------------------------------------------------*/
int mb_check_gmt_grd(int verbose, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue, int *nxy,
                    int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin, double *ymax,
                    double *dx, double *dy, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBBA function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       grdfile:         %s\n", grdfile);
  }

  int status = MB_SUCCESS;

  /* check if the file exists and is readable */
  struct stat file_status;
  if (stat(grdfile, &file_status) == 0
    && (file_status.st_mode & S_IFMT) != S_IFDIR
    && file_status.st_size > 0) {
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }
  else {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  }

  struct GMT_GRID_HEADER *header; /* GMT grid header structure pointer */
  mb_path projectionname = "";
  int epsgid;
  enum ModelType modeltype;

  /* if file exists proceed */
  if (status == MB_SUCCESS) {

    /* Initialize new GMT session */
    /* GMT API control structure pointer */
    unsigned int gmt_mode = 1;  // bitmask bit 1 set so that in an error condition
                                // GMT calls return rather than exit immediately
    void *API  = GMT_Create_Session(__func__, 2U, gmt_mode, NULL);
    if (API == NULL) {
      fprintf(stderr, "\nUnable to initialize a GMT session with GMT_Create_Session() in function %s\n", __func__);
      fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }

    /* read in the grid */
    const int MAX_GRID_READ_ATTEMPTS = 1000;
    int num_tries = 0;
    struct GMT_GRID *G = NULL;      /* GMT grid structure pointer */
    while (G == NULL && num_tries < MAX_GRID_READ_ATTEMPTS) {
      if ((G = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, grdfile, NULL)) == NULL) {
        num_tries++;
#ifdef _WIN32
        Sleep(1);    /* 25 milisec */
#else
        usleep(25000);
#endif
        if (num_tries > 0)
          fprintf(stderr,"!!-- Failed to read grid <%s> - Number of attempts: %d out of %d possible\n",
                  grdfile, num_tries, MAX_GRID_READ_ATTEMPTS);
      }
      else if (num_tries > 0) {
        fprintf(stderr, "!!-- Succeeded reading grid <%s> on attempt %d\n", grdfile, num_tries+1);
      }
    }
    if (G == NULL) {
      fprintf(stderr, "\nUnable to read GMT grid file %s with GMT_Read_Data() after %d tries in function %s\n",
          grdfile, num_tries, __func__);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }

    /* proceed if ok */
    if (status == MB_SUCCESS) {
      /* try to get projection from the grd file remark */
      header = G->header;
      if (strncmp(&(header->remark[2]), "Projection: ", 12) == 0) {
        int nscan;
        int utmzone;
        char NorS;
        if ((nscan = sscanf(&(header->remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2) {
          if (NorS == 'N') {
            epsgid = 32600 + utmzone;
          }
          else if (NorS == 'S') {
            epsgid = 32700 + utmzone;
          }
          else {
            epsgid = 32600 + utmzone;
          }
          modeltype = ModelTypeProjected;
          sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
          *grid_projection_mode = MB_PROJECTION_PROJECTED;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
        else if ((nscan = sscanf(&(header->remark[2]), "Projection: EPSG:%d", &epsgid)) == 1) {
          sprintf(projectionname, "EPSG:%d", epsgid);
          modeltype = ModelTypeProjected;
          *grid_projection_mode = MB_PROJECTION_PROJECTED;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
        else {
          strcpy(projectionname, "Geographic WGS84");
          modeltype = ModelTypeGeographic;
          epsgid = GCS_WGS_84;
          *grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
      }
      else {
        strcpy(projectionname, "Geographic WGS84");
        modeltype = ModelTypeGeographic;
        epsgid = GCS_WGS_84;
        *grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
        sprintf(grid_projection_id, "EPSG:%d", epsgid);
      }

      /* set up internal arrays */
      *nodatavalue = MIN(MB_DEFAULT_GRID_NODATA, header->z_min - 10 * (header->z_max - header->z_min));
      *nxy = header->n_columns * header->n_rows;
      *n_columns = header->n_columns;
      *n_rows = header->n_rows;
      *xmin = header->wesn[0];
      *xmax = header->wesn[1];
      *ymin = header->wesn[2];
      *ymax = header->wesn[3];
      *dx = header->inc[0];
      *dy = header->inc[1];
      *min = header->z_min;
      *max = header->z_max;
    }

    /* Destroy GMT session */
    if (GMT_Destroy_Session(API) != 0) {
      fprintf(stderr, "\nUnable to destroy a GMT session with GMT_Destroy_Session() in function %s\n", __func__);
      fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }
  }

  if (status == MB_SUCCESS && verbose > 0) {
    fprintf(stderr, "\nGrid read:\n");
    fprintf(stderr, "  Dimensions:     %u %u\n", header->n_columns, header->n_rows);
    fprintf(stderr, "  Registration:   %d\n", header->registration);
    if (modeltype == ModelTypeProjected) {
      fprintf(stderr, "  Projected Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Projected Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Easting:    %f %f  %g\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Northing:   %f %f  %g\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    else {
      fprintf(stderr, "  Geographic Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Geographic Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Longitude:  %.9f %.9f  %.9f\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Latitude:   %.9f %.9f  %.9f\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    fprintf(stderr, "  Grid Projection Mode:     %d\n", *grid_projection_mode);
    fprintf(stderr, "  Grid Projection ID:       %s\n", grid_projection_id);
    fprintf(stderr, "  Data Extrema:             %f %f\n", header->z_min, header->z_max);
    fprintf(stderr, "  Other Grid Parameters:\n");
    fprintf(stderr, "    z_scale_factor:         %f\n", header->z_scale_factor);
    fprintf(stderr, "    z_add_offset:           %f\n", header->z_add_offset);
    fprintf(stderr, "    type:                   %d\n", header->type);
    fprintf(stderr, "    bits:                   %d\n", header->bits);
    fprintf(stderr, "    complex_mode:           %d\n", header->complex_mode);
    fprintf(stderr, "    mx:                     %d\n", header->mx);
    fprintf(stderr, "    my:                     %d\n", header->my);
    fprintf(stderr, "    nm:                     %zu\n", header->nm);
    fprintf(stderr, "    size:                   %zu\n", header->size);
    fprintf(stderr, "    pad:                    %d %d %d %d\n", header->pad[0], header->pad[1], header->pad[2],
            header->pad[3]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBBA function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    if (status == MB_SUCCESS) {
            fprintf(stderr, "dbg2       Dimensions: %d %d\n", header->n_columns, header->n_rows);
            if (modeltype == ModelTypeProjected) {
              fprintf(stderr, "dbg2       Projected Coordinate System Name: %s\n", projectionname);
              fprintf(stderr, "dbg2       Projected Coordinate System ID:   %d\n", epsgid);
              fprintf(stderr, "dbg2       Easting:                  %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
              fprintf(stderr, "dbg2       Northing:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
            }
            else {
              fprintf(stderr, "dbg2       Geographic Coordinate System Name: %s\n", projectionname);
              fprintf(stderr, "dbg2       Geographic Coordinate System ID:   %d\n", epsgid);
              fprintf(stderr, "dbg2       Longitude:                %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
              fprintf(stderr, "dbg2       Latitude:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
            }
            fprintf(stderr, "dbg2       Internal Grid Projection Mode: %d\n", *grid_projection_mode);
            fprintf(stderr, "dbg2       Internal Grid Projection ID:   %s\n", grid_projection_id);
            fprintf(stderr, "Data Read:\n");
            fprintf(stderr, "dbg2       grid_projection_mode:     %d\n", *grid_projection_mode);
            fprintf(stderr, "dbg2       grid_projection_id:       %s\n", grid_projection_id);
            fprintf(stderr, "dbg2       nodatavalue:              %f\n", *nodatavalue);
            fprintf(stderr, "dbg2       n_columns:                %d\n", *n_columns);
            fprintf(stderr, "dbg2       n_rows:                   %d\n", *n_rows);
            fprintf(stderr, "dbg2       min:                      %f\n", *min);
            fprintf(stderr, "dbg2       max:                      %f\n", *max);
            fprintf(stderr, "dbg2       xmin:                     %f\n", *xmin);
            fprintf(stderr, "dbg2       xmax:                     %f\n", *xmax);
            fprintf(stderr, "dbg2       ymin:                     %f\n", *ymin);
            fprintf(stderr, "dbg2       ymax:                     %f\n", *ymax);
            fprintf(stderr, "dbg2       dx:                       %f\n", *dx);
            fprintf(stderr, "dbg2       dy:                       %f\n", *dy);
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------------*/
int mb_read_gmt_grd(int verbose, char *grdfile, int *grid_projection_mode, char *grid_projection_id, float *nodatavalue, int *nxy,
                    int *n_columns, int *n_rows, double *min, double *max, double *xmin, double *xmax, double *ymin, double *ymax,
                    double *dx, double *dy, float **data, float **data_dzdx, float **data_dzdy, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBBA function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       grdfile:         %s\n", grdfile);
  }

  int status = MB_SUCCESS;

  /* check if the file exists and is readable */
  struct stat file_status;
  if (stat(grdfile, &file_status) == 0
    && (file_status.st_mode & S_IFMT) != S_IFDIR
    && file_status.st_size > 0) {
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
  }
  else {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  }

  struct GMT_GRID_HEADER *header; /* GMT grid header structure pointer */
  mb_path projectionname = "";
  int epsgid;
  enum ModelType modeltype;

  /* if file exists proceed */
  if (status == MB_SUCCESS) {

    /* Initialize new GMT session */
    /* GMT API control structure pointer */
    unsigned int gmt_mode = 1;  // bitmask bit 1 set so that in an error condition
                                // GMT calls return rather than exit immediately
    void *API  = GMT_Create_Session(__func__, 2U, gmt_mode, NULL);
    if (API == NULL) {
      fprintf(stderr, "\nUnable to initialize a GMT session with GMT_Create_Session() in function %s\n", __func__);
      fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }

    /* read in the grid */
    const int MAX_GRID_READ_ATTEMPTS = 1000;
    int num_tries = 0;
    struct GMT_GRID *G = NULL;      /* GMT grid structure pointer */
    while (G == NULL && num_tries < MAX_GRID_READ_ATTEMPTS) {
      if ((G = GMT_Read_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, grdfile, NULL)) == NULL) {
        num_tries++;
#ifdef _WIN32
        Sleep(1);    /* 25 milisec */
#else
        usleep(25000);
#endif
        if (num_tries > 0)
          fprintf(stderr,"!!-- Failed to read grid <%s> - Number of attempts: %d out of %d possible\n",
                  grdfile, num_tries, MAX_GRID_READ_ATTEMPTS);
      }
      else if (num_tries > 0) {
        fprintf(stderr, "!!-- Succeeded reading grid <%s> on attempt %d\n", grdfile, num_tries+1);
      }
    }
    if (G == NULL) {
      fprintf(stderr, "\nUnable to read GMT grid file %s with GMT_Read_Data() after %d tries in function %s\n",
          grdfile, num_tries, __func__);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }

    /* proceed if ok */
    if (status == MB_SUCCESS) {
      /* try to get projection from the grd file remark */
      header = G->header;
      if (strncmp(&(header->remark[2]), "Projection: ", 12) == 0) {
        int nscan;
        int utmzone;
        char NorS;
        if ((nscan = sscanf(&(header->remark[2]), "Projection: UTM%d%c", &utmzone, &NorS)) == 2) {
          if (NorS == 'N') {
            epsgid = 32600 + utmzone;
          }
          else if (NorS == 'S') {
            epsgid = 32700 + utmzone;
          }
          else {
            epsgid = 32600 + utmzone;
          }
          modeltype = ModelTypeProjected;
          sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
          *grid_projection_mode = MB_PROJECTION_PROJECTED;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
        else if ((nscan = sscanf(&(header->remark[2]), "Projection: EPSG:%d", &epsgid)) == 1) {
          sprintf(projectionname, "EPSG:%d", epsgid);
          modeltype = ModelTypeProjected;
          *grid_projection_mode = MB_PROJECTION_PROJECTED;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
        else {
          strcpy(projectionname, "Geographic WGS84");
          modeltype = ModelTypeGeographic;
          epsgid = GCS_WGS_84;
          *grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
          sprintf(grid_projection_id, "EPSG:%d", epsgid);
        }
      }
      else {
        strcpy(projectionname, "Geographic WGS84");
        modeltype = ModelTypeGeographic;
        epsgid = GCS_WGS_84;
        *grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
        sprintf(grid_projection_id, "EPSG:%d", epsgid);
      }

      /* set up internal arrays */
      *nodatavalue = MIN(MB_DEFAULT_GRID_NODATA, header->z_min - 10 * (header->z_max - header->z_min));
      *nxy = header->n_columns * header->n_rows;
      *n_columns = header->n_columns;
      *n_rows = header->n_rows;
      *xmin = header->wesn[0];
      *xmax = header->wesn[1];
      *ymin = header->wesn[2];
      *ymax = header->wesn[3];
      *dx = header->inc[0];
      *dy = header->inc[1];
      *min = header->z_min;
      *max = header->z_max;

      float *usedata;
      status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
      if (status == MB_SUCCESS) {
        *data = usedata;
      }
      if (status == MB_SUCCESS && data_dzdx != NULL) {
        status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
        *data_dzdx = usedata;
      }
      if (status == MB_SUCCESS && data_dzdy != NULL) {
        status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(float) * (*nxy), (void **)&usedata, error);
        *data_dzdy = usedata;
      }
    }

    /* copy grid data, reordering to internal convention */
    if (status == MB_SUCCESS) {
      for (int i = 0; i < *n_columns; i++)
        for (int j = 0; j < *n_rows; j++) {
          const int k = i * *n_rows + j;
          const int kk = (*n_rows + header->pad[2] + header->pad[3] - 1 - j) * (*n_columns + header->pad[0] + header->pad[1]) +
             (i + header->pad[0]);
          if (MB_IS_FNAN(G->data[kk]))
            (*data)[k] = *nodatavalue;
          else
            (*data)[k] = G->data[kk];
        }
    }

    /* calculate derivatives */
    if (status == MB_SUCCESS && data_dzdx != NULL && data_dzdy != NULL) {
      double ddx = *dx;
      double ddy = *dy;
      if (*grid_projection_mode == MB_PROJECTION_GEOGRAPHIC) {
        double mtodeglon;
        double mtodeglat;
        mb_coor_scale(verbose, 0.5 * (*ymin + *ymax), &mtodeglon, &mtodeglat);
        ddx /= mtodeglon;
        ddy /= mtodeglon;
      }
      for (int i = 0; i < *n_columns; i++)
        for (int j = 0; j < *n_rows; j++) {
          const int k = i * (*n_rows) + j;
          int ii = 0;

          int kx0;
          if (i > 0) {
            kx0 = (i - 1) * (*n_rows) + j;
            ii++;
          } else {
            kx0 = k;
          }

          int kx2 = 0;
          if (i < *n_columns - 1) {
            kx2 = (i + 1) * (*n_rows) + j;
            ii++;
          } else {
            kx2 = k;
          }

          int jj = 0;

          int ky0;
          if (j > 0) {
            ky0 = i * (*n_rows) + j + 1;
            jj++;
          } else {
            ky0 = k;
          }

          int ky2;
          if (j < *n_rows - 1) {
            ky2 = i * (*n_rows) + j - 1;
            jj++;
          } else {
            ky2 = k;
          }

          if (ii > 0)
            (*data_dzdx)[k] = ((*data)[kx2] - (*data)[kx0]) / (((double)ii) * ddx);
          if (jj > 0)
            (*data_dzdy)[k] = ((*data)[ky2] - (*data)[ky0]) / (((double)jj) * ddy);
        }
    }

    /* Destroy GMT session */
    if (GMT_Destroy_Session(API) != 0) {
      fprintf(stderr, "\nUnable to destroy a GMT session with GMT_Destroy_Session() in function %s\n", __func__);
      fprintf(stderr, "Unable to read GMT grid file %s\n",grdfile);
      fprintf(stderr, "Program terminated\n");
      exit(EXIT_FAILURE);
    }
  }

  if (status == MB_SUCCESS && verbose > 0) {
    fprintf(stderr, "\nGrid read:\n");
    fprintf(stderr, "  Dimensions:     %u %u\n", header->n_columns, header->n_rows);
    fprintf(stderr, "  Registration:   %d\n", header->registration);
    if (modeltype == ModelTypeProjected) {
      fprintf(stderr, "  Projected Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Projected Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Easting:    %f %f  %g\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Northing:   %f %f  %g\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    else {
      fprintf(stderr, "  Geographic Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Geographic Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Longitude:  %.9f %.9f  %.9f\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Latitude:   %.9f %.9f  %.9f\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    fprintf(stderr, "  Grid Projection Mode:     %d\n", *grid_projection_mode);
    fprintf(stderr, "  Grid Projection ID:       %s\n", grid_projection_id);
    fprintf(stderr, "  Data Extrema:             %f %f\n", header->z_min, header->z_max);
    fprintf(stderr, "  Other Grid Parameters:\n");
    fprintf(stderr, "    z_scale_factor:         %f\n", header->z_scale_factor);
    fprintf(stderr, "    z_add_offset:           %f\n", header->z_add_offset);
    fprintf(stderr, "    type:                   %d\n", header->type);
    fprintf(stderr, "    bits:                   %d\n", header->bits);
    fprintf(stderr, "    complex_mode:           %d\n", header->complex_mode);
    fprintf(stderr, "    mx:                     %d\n", header->mx);
    fprintf(stderr, "    my:                     %d\n", header->my);
    fprintf(stderr, "    nm:                     %zu\n", header->nm);
    fprintf(stderr, "    size:                   %zu\n", header->size);
    fprintf(stderr, "    pad:                    %d %d %d %d\n", header->pad[0], header->pad[1], header->pad[2],
            header->pad[3]);
    fprintf(stderr, "    data ptr:               %p\n", data);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBBA function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    if (status == MB_SUCCESS) {
            fprintf(stderr, "dbg2       Dimensions: %d %d\n", header->n_columns, header->n_rows);
            if (modeltype == ModelTypeProjected) {
              fprintf(stderr, "dbg2       Projected Coordinate System Name: %s\n", projectionname);
              fprintf(stderr, "dbg2       Projected Coordinate System ID:   %d\n", epsgid);
              fprintf(stderr, "dbg2       Easting:                  %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
              fprintf(stderr, "dbg2       Northing:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
            }
            else {
              fprintf(stderr, "dbg2       Geographic Coordinate System Name: %s\n", projectionname);
              fprintf(stderr, "dbg2       Geographic Coordinate System ID:   %d\n", epsgid);
              fprintf(stderr, "dbg2       Longitude:                %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
              fprintf(stderr, "dbg2       Latitude:                 %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
            }
            fprintf(stderr, "dbg2       Internal Grid Projection Mode: %d\n", *grid_projection_mode);
            fprintf(stderr, "dbg2       Internal Grid Projection ID:   %s\n", grid_projection_id);
            fprintf(stderr, "Data Read:\n");
            fprintf(stderr, "dbg2       grid_projection_mode:     %d\n", *grid_projection_mode);
            fprintf(stderr, "dbg2       grid_projection_id:       %s\n", grid_projection_id);
            fprintf(stderr, "dbg2       nodatavalue:              %f\n", *nodatavalue);
            fprintf(stderr, "dbg2       n_columns:                %d\n", *n_columns);
            fprintf(stderr, "dbg2       n_rows:                   %d\n", *n_rows);
            fprintf(stderr, "dbg2       min:                      %f\n", *min);
            fprintf(stderr, "dbg2       max:                      %f\n", *max);
            fprintf(stderr, "dbg2       xmin:                     %f\n", *xmin);
            fprintf(stderr, "dbg2       xmax:                     %f\n", *xmax);
            fprintf(stderr, "dbg2       ymin:                     %f\n", *ymin);
            fprintf(stderr, "dbg2       ymax:                     %f\n", *ymax);
            fprintf(stderr, "dbg2       dx:                       %f\n", *dx);
            fprintf(stderr, "dbg2       dy:                       %f\n", *dy);
            fprintf(stderr, "dbg2       data:                     %p\n", *data);
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
 * function write_cdfgrd writes output grid to a
 * GMT version 2 netCDF grd file
 */
int mb_write_gmt_grd(int verbose, const char *grdfile, float *grid,
                     float nodatavalue, int n_columns, int n_rows,
                     double xmin, double xmax, double ymin, double ymax,
                     double zmin, double zmax, double dx, double dy,
                     const char *xlab, const char *ylab, const char *zlab,
                     const char *titl, const char *projection,
                     int argc, char **argv, int *error) {
  (void)zmin;  // Unused parameter.
  (void)zmax;  // Unused parameter.
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       grdfile:    %s\n", grdfile);
    fprintf(stderr, "dbg2       grid:       %p\n", (void *)grid);
    fprintf(stderr, "dbg2       nodatavalue:%f\n", nodatavalue);
    fprintf(stderr, "dbg2       n_columns:  %d\n", n_columns);
    fprintf(stderr, "dbg2       n_rows:     %d\n", n_rows);
    fprintf(stderr, "dbg2       xmin:       %f\n", xmin);
    fprintf(stderr, "dbg2       xmax:       %f\n", xmax);
    fprintf(stderr, "dbg2       ymin:       %f\n", ymin);
    fprintf(stderr, "dbg2       ymax:       %f\n", ymax);
    fprintf(stderr, "dbg2       dx:         %g\n", dx);
    fprintf(stderr, "dbg2       dy:         %g\n", dy);
    fprintf(stderr, "dbg2       xlab:       %s\n", xlab);
    fprintf(stderr, "dbg2       ylab:       %s\n", ylab);
    fprintf(stderr, "dbg2       zlab:       %s\n", zlab);
    fprintf(stderr, "dbg2       projection: %s\n", projection);
    fprintf(stderr, "dbg2       titl:       %s\n", titl);
    fprintf(stderr, "dbg2       argc:       %d\n", argc);
    fprintf(stderr, "dbg2       *argv:      %p\n", (void *)*argv);
  }

  /* Initializing new GMT session */
  void *API = GMT_Create_Session(__func__, 2U, 0U, NULL);
  if (API == NULL) {
    fprintf(stderr, "\nUnable to initialize a GMT session with GMT_Create_Session() in function %s\n", __func__);
    fprintf(stderr, "Unable to write GMT grid file %s\n",grdfile);
    fprintf(stderr, "Program terminated\n");
    exit(EXIT_FAILURE);
  }

  unsigned int mode = GMT_GRID_ALL;

  /* set grid creation control values */
  /* GMT_GRID_NODE_REG (0) for node grids, GMT_GRID_PIXEL_REG (1) for pixel grids */
  const int nx_node_registration = lround((xmax - xmin) / dx + 1);
  unsigned int registration;
  if (n_columns == nx_node_registration) {
    registration = GMT_GRID_NODE_REG;
  }
  else if (n_columns == nx_node_registration - 1) {
    registration = GMT_GRID_PIXEL_REG;
  }
  else {
    registration = GMT_GRID_DEFAULT_REG;
  }

  /* Min/max x and y coordinates */
  double wesn[4] = {xmin, xmax, ymin, ymax};
  double inc[2] = {dx, dy};  /* x and y increment */
  int pad = 0;

  int status = MB_SUCCESS;

  /* create structure for the grid */
  struct GMT_GRID *G = GMT_Create_Data(API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, registration, pad, NULL);
  if (G == NULL) {
    status = MB_FAILURE;
    *error = MB_ERROR_MEMORY_FAIL;
    return (status);
  }

  /* Get some projection and user info needed for the grid remark field */
  enum ModelType modeltype;
  int epsgid;
  int grid_projection_mode;
  mb_path projectionname = "";
  mb_path grid_projection_id = "";
  int utmzone;
  int nscan;
  char NorS;
  if ((nscan = sscanf(projection, "UTM%d%c", &utmzone, &NorS)) == 2) {
    if (NorS == 'N') {
      epsgid = 32600 + utmzone;
    }
    else if (NorS == 'S') {
      epsgid = 32700 + utmzone;
    }
    else {
      epsgid = 32600 + utmzone;
    }
    modeltype = ModelTypeProjected;
    sprintf(projectionname, "UTM%2.2d%c", utmzone, NorS);
    grid_projection_mode = MB_PROJECTION_PROJECTED;
    sprintf(grid_projection_id, "epsg%d", epsgid);
  }
  else if ((nscan = sscanf(projection, "EPSG:%d", &epsgid)) == 1) {
    sprintf(projectionname, "EPSG:%d", epsgid);
    modeltype = ModelTypeProjected;
    grid_projection_mode = MB_PROJECTION_PROJECTED;
    sprintf(grid_projection_id, "epsg%d", epsgid);
  }
  else {
    strcpy(projectionname, "Geographic WGS84");
    modeltype = ModelTypeGeographic;
    epsgid = GCS_WGS_84;
    grid_projection_mode = MB_PROJECTION_GEOGRAPHIC;
    sprintf(grid_projection_id, "epsg%d", epsgid);
  }

  struct GMT_GRID_HEADER *header = G->header;
#ifdef HAVE_GDAL
  /* If GDAL available use it to get the Proj string of this EPSG code */
  OGRErr eErr = OGRERR_NONE;
  OGRSpatialReferenceH hSRS = OSRNewSpatialReference(NULL);
  char *pszResult = NULL;
  if ((eErr = OSRImportFromEPSG(hSRS, epsgid)) != OGRERR_NONE) {
    fprintf(stderr, "Did not get the SRS from input EPSG  %d\n", epsgid);
  }
  if ((eErr = OSRExportToProj4(hSRS, &pszResult)) != OGRERR_NONE) {
    fprintf(stderr, "Failed to convert the SRS to Proj syntax\n");
  }
#if (GMT_MAJOR_VERSION == 6 && GMT_MINOR_VERSION >= 1) || GMT_MAJOR_VERSION > 6
  header->ProjRefPROJ4 = gmt_strdup_noquote(pszResult); // allocated within GMT because it will be freed within GMT
#else
  header->ProjRefPROJ4 = strdup(pszResult);
#endif
#if GMT_MAJOR_VERSION >= 6
  header->ProjRefEPSG = epsgid;
#endif
  CPLFree(pszResult); // make sure this is freed within GDAL because it was allocated within GDAL
  OSRDestroySpatialReference(hSRS);
#endif

  mb_path program_name = "";
  if (argc > 0)
    strncpy(program_name, argv[0], MB_PATH_MAXLINE);
  else
    strcpy(program_name, "");
  char user[256], host[256], date[32];
  status = mb_user_host_date(verbose, user, host, date, error);
  char remark[2048];
  sprintf(remark, "\n\tProjection: %s\n\tGrid created by %s\n\tMB-system Version %s\n\tRun by <%s> on <%s> at <%s>", projection,
          program_name, MB_VERSION, user, host, date);

  /* set grid labels and remark */
  strcpy(header->command, program_name); /* name of generating command */
  strcpy(header->x_units, xlab);         /* units in x-direction */
  strcpy(header->y_units, ylab);         /* units in y-direction */
  strcpy(header->z_units, zlab);         /* grid value units */
  strcpy(header->title, titl);           /* name of data set */
  strncpy(header->remark, remark, GMT_GRID_REMARK_LEN160);

  /* recopy grid data, reordering from internal convention to grd file convention */
  if (status == MB_SUCCESS) {
    double NaN;
    MB_MAKE_FNAN(NaN);
    /* bool first = false; */
    /* double min = 0.0; */
    /* double max = 0.0; */
    for (int i = 0; i < n_columns; i++)
      for (int j = 0; j < n_rows; j++) {
        const int k = i * n_rows + j;
        const int kk = (n_rows - 1 - j) * n_columns + i;
        if (grid[k] == nodatavalue)
          G->data[kk] = NaN;
        else {
          G->data[kk] = grid[k];
          /* if (first) { */
            /* min = grid[k]; */
            /* max = grid[k]; */
            /* first = false; */
          /* } */
          /* else { */
            /* min = MIN(min, grid[k]); */
            /* max = MAX(max, grid[k]); */
          /* } */
        }
      }
  }

/* create null grid mode flags if valid flags don't exist - Paul Wessel
 * indicated in December 2016 that these will appear with GMT 5.3.2
 * and fix the problem of generating geographic grids that can be
 * directly imported to ESRI ArcGIS */
#ifndef GMT_GRID_IS_GEO
#define GMT_GRID_IS_GEO 0
#endif
#ifndef GMT_GRID_IS_CARTESIAN
#define GMT_GRID_IS_CARTESIAN 0
#endif

  /* set GMT grid mode flag */
  if (modeltype == ModelTypeGeographic)
    mode = GMT_GRID_ALL | GMT_GRID_IS_GEO;
  else
    mode = GMT_GRID_ALL | GMT_GRID_IS_CARTESIAN;

  if (verbose > 0) {
    fprintf(stderr, "\nGrid to be written:\n");
    fprintf(stderr, "  Dimensions:     %d %d\n", header->n_columns, header->n_rows);
    fprintf(stderr, "  Registration:   %d\n", header->registration);
    if (modeltype == ModelTypeProjected) {
      fprintf(stderr, "  Projected Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Projected Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Easting:    %f %f  %f\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Northing:   %f %f  %f\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    else {
      fprintf(stderr, "  Geographic Coordinate System Name: %s\n", projectionname);
      fprintf(stderr, "  Geographic Coordinate System ID:   %d\n", epsgid);
      fprintf(stderr, "  Longitude:  %f %f  %g\n", header->wesn[0], header->wesn[1], header->inc[0]);
      fprintf(stderr, "  Latitude:   %f %f  %g\n", header->wesn[2], header->wesn[3], header->inc[1]);
    }
    fprintf(stderr, "  Grid Projection Mode:     %d\n", grid_projection_mode);
    fprintf(stderr, "  Grid Projection ID:       %s\n", grid_projection_id);
    fprintf(stderr, "  Data Extrema:             %f %f\n", header->z_min, header->z_max);
    fprintf(stderr, "  Other Grid Parameters:\n");
    fprintf(stderr, "    z_scale_factor:         %f\n", header->z_scale_factor);
    fprintf(stderr, "    z_add_offset:           %f\n", header->z_add_offset);
    fprintf(stderr, "    type:                   %d\n", header->type);
    fprintf(stderr, "    bits:                   %d\n", header->bits);
    fprintf(stderr, "    complex_mode:           %d\n", header->complex_mode);
    fprintf(stderr, "    mx:                     %d\n", header->mx);
    fprintf(stderr, "    my:                     %d\n", header->my);
    fprintf(stderr, "    nm:                     %zu\n", header->nm);
    fprintf(stderr, "    size:                   %zu\n", header->size);
    fprintf(stderr, "    pad:                    %d %d %d %d\n", header->pad[0], header->pad[1], header->pad[2],
            header->pad[3]);
    fprintf(stderr, "    data ptr:               %p\n", G->data);
  }

  if (GMT_Write_Data(API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, mode, NULL, grdfile, G) != 0) {
    status = MB_FAILURE;
    *error = MB_ERROR_WRITE_FAIL;
    fprintf(stderr, "Unable to write GMT grid file %s with GMT_Write_Data() in function %s\n", grdfile, __func__);
  }

  if (GMT_Destroy_Session(API) != 0) {
    status = MB_FAILURE;
    *error = MB_ERROR_WRITE_FAIL;
    fprintf(stderr, "\nUnable to destroy a GMT session with GMT_Write_Data() in function %s\n", __func__);
    fprintf(stderr, "Unable to write GMT grid file %s\n",grdfile);
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
