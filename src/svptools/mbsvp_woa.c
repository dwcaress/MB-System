/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_woa.c  3 November 2026
 *
 *    Copyright (c) 2026-2026 by
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
 * World Ocean Atlas sound speed grids: discovery of the files named
 * woaYY_PERIOD_svpTT_RR.nc in one directory, sampling of the grid at a
 * survey (area mean or nearest valid cell), and generation of candidate
 * profile files. The grids hold sound speed already computed from the
 * atlas temperature and salinity, in the variables v_an_chen and
 * v_an_leroy. WOA18 and WOA23 profiles are never blended.
 */

#include <math.h>
#include <netcdf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

struct mbsvp_axis {
  int n;
  double *value; /* increasing */
  int *index;    /* position of each value in the file */
};

static const char *mbsvp_kind_name[MBSVP_KINDS] = {"annual", "seasonal", "monthly"};
static const char *mbsvp_formula_name[2] = {"chen", "leroy"};

/*--------------------------------------------------------------------*/
/* Make an east bound that is less than the west bound lie east of it, and
   take the shorter way around when the interval is longer than 180 deg. */
void mbsvp_circular_bounds(double *west, double *east) {
  if (*east < *west)
    *east += 360.0;
  if (*east - *west > 180.0) {
    const double old_west = *west;
    *west = *east;
    *east = old_west + 360.0;
  }
}
/*--------------------------------------------------------------------*/
/* woa(18|23)_PERIOD_svp(00..16)_(01|04).nc */
bool mbsvp_woa_parse_name(const char *name, int *year, char *period, size_t period_size, char *time_code,
                          char *resolution) {
  const size_t len = strlen(name);
  const size_t tail = strlen("_svpTT_RR.nc");
  const size_t head = strlen("woaYY_");
  if (len <= head + tail || strncmp(name, "woa", 3) != 0 || name[5] != '_')
    return false;
  if (strncmp(&name[3], "18", 2) == 0)
    *year = 18;
  else if (strncmp(&name[3], "23", 2) == 0)
    *year = 23;
  else
    return false;
  const char *end = &name[len - tail];
  if (strncmp(end, "_svp", 4) != 0 || end[6] != '_' || strcmp(&end[9], ".nc") != 0)
    return false;
  if (end[4] < '0' || end[4] > '9' || end[5] < '0' || end[5] > '9')
    return false;
  const int code = 10 * (end[4] - '0') + (end[5] - '0');
  if (code > 16)
    return false;
  if (strncmp(&end[7], "01", 2) != 0 && strncmp(&end[7], "04", 2) != 0)
    return false;
  const size_t period_len = len - head - tail;
  if (period_len >= period_size)
    return false;
  memcpy(period, &name[head], period_len);
  period[period_len] = '\0';
  memcpy(time_code, &end[4], 2);
  time_code[2] = '\0';
  memcpy(resolution, &end[7], 2);
  resolution[2] = '\0';
  return true;
}
/*--------------------------------------------------------------------*/
void mbsvp_grid_init(struct mbsvp_grid *grid) {
  grid->ndepth = 0;
  grid->depth = NULL;
  grid->chen = NULL;
  grid->leroy = NULL;
  mbsvp_location_init(&grid->location);
}
/*--------------------------------------------------------------------*/
int mbsvp_grid_free(int verbose, struct mbsvp_grid *grid, int *error) {
  if (grid->depth != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&grid->depth, error);
  if (grid->chen != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&grid->chen, error);
  if (grid->leroy != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&grid->leroy, error);
  grid->ndepth = 0;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static int mbsvp_axis_read(int verbose, int ncid, const char *name, double **values, int *n, char *message, int *error) {
  int varid;
  int ndims;
  int dimid;
  size_t len;
  if (nc_inq_varid(ncid, name, &varid) != NC_NOERR || nc_inq_varndims(ncid, varid, &ndims) != NC_NOERR || ndims != 1 ||
      nc_inq_vardimid(ncid, varid, &dimid) != NC_NOERR || nc_inq_dimlen(ncid, dimid, &len) != NC_NOERR || len < 1) {
    mbsvp_set_message(message, "Invalid %s coordinate", name);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  if (mb_mallocd(verbose, __FILE__, __LINE__, len * sizeof(double), (void **)values, error) != MB_SUCCESS)
    return MB_FAILURE;
  if (nc_get_var_double(ncid, varid, *values) != NC_NOERR) {
    mbsvp_set_message(message, "Cannot read %s coordinate", name);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  *n = (int)len;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Sort an axis, which must then be finite and strictly increasing, and
   check that it covers low to high within half a cell. */
static int mbsvp_axis_sort(int verbose, double *values, int n, double low, double high, const char *label,
                           struct mbsvp_axis *axis, char *message, int *error) {
  if (mb_mallocd(verbose, __FILE__, __LINE__, n * sizeof(int), (void **)&axis->index, error) != MB_SUCCESS)
    return MB_FAILURE;
  axis->n = n;
  axis->value = values;
  for (int i = 0; i < n; i++) {
    const double value = values[i];
    int j = i - 1;
    while (j >= 0 && values[j] > value) {
      values[j + 1] = values[j];
      axis->index[j + 1] = axis->index[j];
      j--;
    }
    values[j + 1] = value;
    axis->index[j + 1] = i;
  }
  double tolerance = 1.0e-6;
  for (int i = 0; i < n; i++) {
    if (!isfinite(values[i]) || (i > 0 && values[i] <= values[i - 1])) {
      mbsvp_set_message(message, "Invalid %s grid", label);
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    if (i > 0 && (i == 1 || 0.5 * (values[i] - values[i - 1]) < tolerance))
      tolerance = 0.5 * (values[i] - values[i - 1]);
  }
  if (low < values[0] - tolerance - 1.0e-8 || high > values[n - 1] + tolerance + 1.0e-8) {
    mbsvp_set_message(message, "Survey outside %s coverage", label);
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static int mbsvp_variable_open(int ncid, const char *name, struct mbsvp_variable *variable) {
  int dimids[NC_MAX_VAR_DIMS];
  if (nc_inq_varid(ncid, name, &variable->varid) != NC_NOERR ||
      nc_inq_varndims(ncid, variable->varid, &variable->ndims) != NC_NOERR || variable->ndims > NC_MAX_VAR_DIMS ||
      nc_inq_vardimid(ncid, variable->varid, dimids) != NC_NOERR)
    return MB_FAILURE;
  variable->depth_dim = -1;
  variable->lat_dim = -1;
  variable->lon_dim = -1;
  for (int i = 0; i < variable->ndims; i++) {
    char dimname[NC_MAX_NAME + 1];
    if (nc_inq_dimname(ncid, dimids[i], dimname) != NC_NOERR)
      return MB_FAILURE;
    if (strcmp(dimname, "depth") == 0)
      variable->depth_dim = i;
    else if (strcmp(dimname, "lat") == 0)
      variable->lat_dim = i;
    else if (strcmp(dimname, "lon") == 0)
      variable->lon_dim = i;
    /* any other dimension, such as time, is read at its first position */
  }
  if (variable->depth_dim < 0 || variable->lat_dim < 0 || variable->lon_dim < 0)
    return MB_FAILURE;
  variable->has_fill[0] = nc_get_att_float(ncid, variable->varid, "_FillValue", &variable->fill[0]) == NC_NOERR;
  variable->has_fill[1] = nc_get_att_float(ncid, variable->varid, "missing_value", &variable->fill[1]) == NC_NOERR;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Read a block as doubles, with missing values as NaN. The block is either
   one depth level (lat by lon, in the order of the file) or one column. */
static int mbsvp_variable_read(int ncid, const struct mbsvp_variable *variable, int depth_start, int depth_count,
                               int lat_start, int lat_count, int lon_start, int lon_count, float *buffer) {
  size_t start[NC_MAX_VAR_DIMS];
  size_t count[NC_MAX_VAR_DIMS];
  for (int i = 0; i < variable->ndims; i++) {
    start[i] = 0;
    count[i] = 1;
  }
  start[variable->depth_dim] = depth_start;
  count[variable->depth_dim] = depth_count;
  start[variable->lat_dim] = lat_start;
  count[variable->lat_dim] = lat_count;
  start[variable->lon_dim] = lon_start;
  count[variable->lon_dim] = lon_count;
  if (nc_get_vara_float(ncid, variable->varid, start, count, buffer) != NC_NOERR)
    return MB_FAILURE;
  const size_t total = (size_t)depth_count * lat_count * lon_count;
  for (int f = 0; f < 2; f++) {
    if (variable->has_fill[f] && !isnan(variable->fill[f])) {
      for (size_t i = 0; i < total; i++)
        if (buffer[i] == variable->fill[f])
          buffer[i] = (float)NAN;
    }
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Mean over the selected cells at each depth, skipping missing values. */
static int mbsvp_area_mean(int verbose, int ncid, const struct mbsvp_variable *variable, int ndepth,
                           const struct mbsvp_axis *lat, int lat_a, int lat_b, const struct mbsvp_axis *lon, int lon_a,
                           int lon_b, double *mean, char *message, int *error) {
  /* selected latitudes: a block in the file, with a mask for unsorted axes */
  int lat_first = lat->index[lat_a];
  int lat_last = lat->index[lat_a];
  for (int i = lat_a; i < lat_b; i++) {
    if (lat->index[i] < lat_first)
      lat_first = lat->index[i];
    if (lat->index[i] > lat_last)
      lat_last = lat->index[i];
  }
  const int lat_count = lat_last - lat_first + 1;

  /* selected longitudes in file order: blocks split at the seam of the grid */
  const int nlon = lon_b - lon_a;
  int *lon_file = NULL;
  char *lat_mask = NULL;
  float *buffer = NULL;
  int status = mb_mallocd(verbose, __FILE__, __LINE__, nlon * sizeof(int), (void **)&lon_file, error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, lat_count, (void **)&lat_mask, error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, (size_t)lat_count * nlon * sizeof(float), (void **)&buffer, error);
  if (status == MB_SUCCESS) {
    for (int i = 0; i < nlon; i++) {
      const int value = lon->index[lon_a + i];
      int j = i - 1;
      while (j >= 0 && lon_file[j] > value) {
        lon_file[j + 1] = lon_file[j];
        j--;
      }
      lon_file[j + 1] = value;
    }
    memset(lat_mask, 0, lat_count);
    for (int i = lat_a; i < lat_b; i++)
      lat_mask[lat->index[i] - lat_first] = 1;

    for (int k = 0; k < ndepth && status == MB_SUCCESS; k++) {
      double sum = 0.0;
      long count = 0;
      int first = 0;
      while (first < nlon && status == MB_SUCCESS) {
        int run = 1;
        while (first + run < nlon && lon_file[first + run] == lon_file[first] + run)
          run++;
        if (mbsvp_variable_read(ncid, variable, k, 1, lat_first, lat_count, lon_file[first], run, buffer) != MB_SUCCESS) {
          mbsvp_set_message(message, "Cannot read sound speed");
          *error = MB_ERROR_BAD_DATA;
          status = MB_FAILURE;
          break;
        }
        for (int i = 0; i < lat_count; i++) {
          if (lat_mask[i] == 0)
            continue;
          for (int j = 0; j < run; j++) {
            const float value =
                variable->lat_dim < variable->lon_dim ? buffer[(size_t)i * run + j] : buffer[(size_t)j * lat_count + i];
            if (isfinite(value)) {
              sum += value;
              count++;
            }
          }
        }
        first += run;
      }
      mean[k] = count > 0 ? sum / count : NAN;
    }
  }

  int error_free = MB_ERROR_NO_ERROR;
  if (lon_file != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lon_file, &error_free);
  if (lat_mask != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lat_mask, &error_free);
  if (buffer != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&buffer, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
/* -180 to 180. A value already in range keeps its exact value. */
double mbsvp_normalize_longitude(double longitude) {
  if (longitude >= -180.0 && longitude < 180.0)
    return longitude;
  double wrapped = fmod(longitude + 180.0, 360.0);
  if (wrapped < 0.0)
    wrapped += 360.0;
  return wrapped - 180.0;
}
/*--------------------------------------------------------------------*/
/* The cells closest to a position in a local distance with circular
   longitude, closest first. Equal distances keep the order of the axes. */
static void mbsvp_rank_cells(const double *lat, int nlat, const double *lon, int nlon, double target_lat, double target_lon,
                             int limit, int *cell_lat, int *cell_lon, int *ncells) {
  const double scale = MAX(cos(DTR * target_lat), 0.05);
  double best_distance[MBSVP_NEAREST_CELLS];
  int nbest = 0;
  for (int i = 0; i < nlat; i++) {
    const double dy = lat[i] - target_lat;
    for (int j = 0; j < nlon; j++) {
      double dx = fmod(lon[j] - target_lon + 180.0, 360.0);
      if (dx < 0.0)
        dx += 360.0;
      dx = (dx - 180.0) * scale;
      const double distance = dy * dy + dx * dx;
      if (nbest == limit && distance >= best_distance[nbest - 1])
        continue;
      int k = nbest < limit ? nbest : limit - 1;
      while (k > 0 && best_distance[k - 1] > distance) {
        best_distance[k] = best_distance[k - 1];
        cell_lat[k] = cell_lat[k - 1];
        cell_lon[k] = cell_lon[k - 1];
        k--;
      }
      best_distance[k] = distance;
      cell_lat[k] = i;
      cell_lon[k] = j;
      if (nbest < limit)
        nbest++;
    }
  }
  *ncells = nbest;
}
/*--------------------------------------------------------------------*/
static int mbsvp_compare_doubles(const void *a, const void *b) {
  const double x = *(const double *)a;
  const double y = *(const double *)b;
  return x < y ? -1 : (x > y ? 1 : 0);
}
/*--------------------------------------------------------------------*/
/* Check that a position lies inside a possibly cropped grid, then rank the
   cells around it. The axes are in file order and are not changed. A remote
   edge of a cropped grid is never used silently. */
int mbsvp_woa_order_cells(int verbose, const double *lat, int nlat, const double *lon, int nlon, double target_lat,
                          double target_lon, int limit, int *cell_lat, int *cell_lon, int *ncells, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       target:     %f %f\n", target_lon, target_lat);
    fprintf(stderr, "dbg2       limit:      %d\n", limit);
  }

  *ncells = 0;
  if (nlat < 1 || nlon < 1 || limit < 1 || limit > MBSVP_NEAREST_CELLS) {
    mbsvp_set_message(message, "Invalid grid or cell limit");
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  const size_t size = (size_t)MAX(nlat, nlon + 1) * sizeof(double);
  double *work = NULL;
  double *gaps = NULL;
  if (mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&work, error) != MB_SUCCESS ||
      mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&gaps, error) != MB_SUCCESS) {
    int error_free = MB_ERROR_NO_ERROR;
    if (work != NULL)
      mb_freed(verbose, __FILE__, __LINE__, (void **)&work, &error_free);
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;

  /* longitude axis */
  for (int i = 0; i < nlon && status == MB_SUCCESS; i++) {
    if (!isfinite(lon[i])) {
      mbsvp_set_message(message, "Invalid lon coordinate axis");
      status = MB_FAILURE;
    }
    work[i] = lon[i];
  }
  if (status == MB_SUCCESS) {
    qsort(work, nlon, sizeof(double), mbsvp_compare_doubles);
    for (int i = 1; i < nlon && status == MB_SUCCESS; i++) {
      if (work[i] == work[i - 1]) {
        mbsvp_set_message(message, "Duplicate lon coordinates");
        status = MB_FAILURE;
      }
    }
  }
  if (status == MB_SUCCESS && nlon > 1) {
    for (int i = 0; i < nlon; i++) {
      double wrapped = fmod(lon[i] + 180.0, 360.0);
      if (wrapped < 0.0)
        wrapped += 360.0;
      work[i] = wrapped - 180.0;
    }
    qsort(work, nlon, sizeof(double), mbsvp_compare_doubles);
    double largest = -1.0;
    int k = 0;
    for (int i = 0; i < nlon; i++) {
      gaps[i] = i < nlon - 1 ? work[i + 1] - work[i] : work[0] + 360.0 - work[nlon - 1];
      if (gaps[i] > largest) {
        largest = gaps[i];
        k = i;
      }
    }
    const double start = work[(k + 1) % nlon];
    qsort(gaps, nlon, sizeof(double), mbsvp_compare_doubles);
    const double typical = nlon % 2 == 1 ? gaps[nlon / 2] : 0.5 * (gaps[nlon / 2 - 1] + gaps[nlon / 2]);
    if (largest > MAX(typical * 3.0, 1.0e-6)) {
      const double span = 360.0 - largest;
      double offset = fmod(target_lon - start, 360.0);
      if (offset < 0.0)
        offset += 360.0;
      if (offset > span + typical / 2.0 + 1.0e-8 && offset < 360.0 - typical / 2.0 - 1.0e-8) {
        mbsvp_set_message(message, "Site outside cropped longitude coverage");
        status = MB_FAILURE;
      }
    }
  }
  else if (status == MB_SUCCESS) {
    double dx = fmod(lon[0] - target_lon + 180.0, 360.0);
    if (dx < 0.0)
      dx += 360.0;
    if (fabs(dx - 180.0) > 1.0e-6) {
      mbsvp_set_message(message, "Site outside single-cell longitude coverage");
      status = MB_FAILURE;
    }
  }

  /* latitude axis */
  for (int i = 0; i < nlat && status == MB_SUCCESS; i++) {
    if (!isfinite(lat[i])) {
      mbsvp_set_message(message, "Invalid lat coordinate axis");
      status = MB_FAILURE;
    }
    else if (fabs(lat[i]) > 90.0) {
      mbsvp_set_message(message, "Latitude grid outside [-90,90]");
      status = MB_FAILURE;
    }
    work[i] = lat[i];
  }
  if (status == MB_SUCCESS) {
    qsort(work, nlat, sizeof(double), mbsvp_compare_doubles);
    double tolerance = 1.0e-6;
    for (int i = 1; i < nlat && status == MB_SUCCESS; i++) {
      if (work[i] == work[i - 1]) {
        mbsvp_set_message(message, "Duplicate lat coordinates");
        status = MB_FAILURE;
      }
      if (i == 1 || 0.5 * (work[i] - work[i - 1]) < tolerance)
        tolerance = 0.5 * (work[i] - work[i - 1]);
    }
    if (status == MB_SUCCESS &&
        (target_lat < work[0] - tolerance - 1.0e-8 || target_lat > work[nlat - 1] + tolerance + 1.0e-8)) {
      mbsvp_set_message(message, "Site outside latitude coverage");
      status = MB_FAILURE;
    }
  }

  if (status == MB_SUCCESS)
    mbsvp_rank_cells(lat, nlat, lon, nlon, target_lat, target_lon, limit, cell_lat, cell_lon, ncells);
  else
    *error = MB_ERROR_BAD_PARAMETER;

  int error_free = MB_ERROR_NO_ERROR;
  mb_freed(verbose, __FILE__, __LINE__, (void **)&work, &error_free);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&gaps, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
/* Examine only nearby cells: a remote ocean is never used silently. */
static int mbsvp_nearest_cell(int verbose, int ncid, const struct mbsvp_variable variable[2], double target_lat,
                              double center, const struct mbsvp_axis *lat, const struct mbsvp_axis *lon,
                              struct mbsvp_grid *grid, float *buffer, char *message, int *error) {
  int best_lat[MBSVP_NEAREST_CELLS];
  int best_lon[MBSVP_NEAREST_CELLS];
  int nbest = 0;
  mbsvp_rank_cells(lat->value, lat->n, lon->value, lon->n, target_lat, center, MBSVP_NEAREST_CELLS, best_lat, best_lon, &nbest);

  for (int c = 0; c < nbest; c++) {
    double *speed[2] = {grid->chen, grid->leroy};
    for (int v = 0; v < 2; v++) {
      if (mbsvp_variable_read(ncid, &variable[v], 0, grid->ndepth, lat->index[best_lat[c]], 1, lon->index[best_lon[c]], 1,
                              buffer) != MB_SUCCESS) {
        mbsvp_set_message(message, "Cannot read sound speed");
        *error = MB_ERROR_BAD_DATA;
        return MB_FAILURE;
      }
      for (int k = 0; k < grid->ndepth; k++)
        speed[v][k] = buffer[k];
    }
    int nvalid = 0;
    bool surface = false;
    for (int k = 0; k < grid->ndepth; k++) {
      if (isfinite(grid->chen[k]) && isfinite(grid->leroy[k])) {
        nvalid++;
        if (grid->depth[k] <= 1.0)
          surface = true;
      }
    }
    if (nvalid >= 2 && surface) {
      if (c > 0)
        fprintf(stderr, "Warning: nearest cell invalid; using a neighbouring valid ocean cell\n");
      grid->location.has_position = true;
      grid->location.longitude = lon->value[best_lon[c]];
      grid->location.latitude = lat->value[best_lat[c]];
      strcpy(grid->location.method, "nearest");
      (void)verbose;
      return MB_SUCCESS;
    }
  }
  mbsvp_set_message(message, "No valid near-surface ocean profile among %d nearest cells", MBSVP_NEAREST_CELLS);
  *error = MB_ERROR_BAD_DATA;
  return MB_FAILURE;
}
/*--------------------------------------------------------------------*/
int mbsvp_woa_read_grid(int verbose, const char *path, const struct mbsvp_survey *survey, bool nearest,
                        struct mbsvp_grid *grid, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       bounds:     %f %f %f %f\n", survey->west, survey->east, survey->south, survey->north);
    fprintf(stderr, "dbg2       nearest:    %d\n", nearest);
  }

  int ncid;
  const int nc_status = nc_open(path, NC_NOWRITE, &ncid);
  if (nc_status != NC_NOERR) {
    mbsvp_set_message(message, "%s", nc_strerror(nc_status));
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }

  struct mbsvp_variable variable[2];
  int status = MB_SUCCESS;
  if (mbsvp_variable_open(ncid, "v_an_chen", &variable[0]) != MB_SUCCESS ||
      mbsvp_variable_open(ncid, "v_an_leroy", &variable[1]) != MB_SUCCESS) {
    mbsvp_set_message(message, "Missing sound-speed variables in %s", path);
    *error = MB_ERROR_BAD_DATA;
    status = MB_FAILURE;
  }

  double west = survey->west;
  double east = survey->east;
  mbsvp_circular_bounds(&west, &east);
  const double center = 0.5 * (west + east);

  struct mbsvp_axis lat = {0, NULL, NULL};
  struct mbsvp_axis lon = {0, NULL, NULL};
  double *lat_values = NULL;
  double *lon_values = NULL;
  float *buffer = NULL;
  int nlat = 0;
  int nlon = 0;
  if (status == MB_SUCCESS)
    status = mbsvp_axis_read(verbose, ncid, "lat", &lat_values, &nlat, message, error);
  if (status == MB_SUCCESS)
    status = mbsvp_axis_read(verbose, ncid, "lon", &lon_values, &nlon, message, error);
  if (status == MB_SUCCESS)
    status = mbsvp_axis_read(verbose, ncid, "depth", &grid->depth, &grid->ndepth, message, error);
  if (status == MB_SUCCESS) {
    /* whole turns only, so that the coordinates keep their exact values */
    for (int i = 0; i < nlon; i++)
      if (isfinite(lon_values[i]))
        lon_values[i] -= 360.0 * floor((lon_values[i] - center + 180.0) / 360.0);
    status = mbsvp_axis_sort(verbose, lat_values, nlat, survey->south, survey->north, "latitude", &lat, message, error);
  }
  if (status == MB_SUCCESS)
    status = mbsvp_axis_sort(verbose, lon_values, nlon, west, east, "longitude", &lon, message, error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, grid->ndepth * sizeof(double), (void **)&grid->chen, error);
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, grid->ndepth * sizeof(double), (void **)&grid->leroy, error);

  if (status == MB_SUCCESS && nearest) {
    status = mb_mallocd(verbose, __FILE__, __LINE__, grid->ndepth * sizeof(float), (void **)&buffer, error);
    if (status == MB_SUCCESS)
      status = mbsvp_nearest_cell(verbose, ncid, variable, 0.5 * (survey->south + survey->north), center, &lat, &lon, grid,
                                  buffer, message, error);
  }
  else if (status == MB_SUCCESS) {
    /* the cells that bracket the survey */
    int lat_a = 0;
    while (lat_a < nlat && lat.value[lat_a] <= survey->south)
      lat_a++;
    lat_a = MAX(0, lat_a - 1);
    int lat_b = 0;
    while (lat_b < nlat && lat.value[lat_b] < survey->north)
      lat_b++;
    lat_b = MIN(nlat, lat_b + 1);
    int lon_a = 0;
    while (lon_a < nlon && lon.value[lon_a] <= west)
      lon_a++;
    lon_a = MAX(0, lon_a - 1);
    int lon_b = 0;
    while (lon_b < nlon && lon.value[lon_b] < east)
      lon_b++;
    lon_b = MIN(nlon, lon_b + 1);

    status = mbsvp_area_mean(verbose, ncid, &variable[0], grid->ndepth, &lat, lat_a, lat_b, &lon, lon_a, lon_b, grid->chen,
                             message, error);
    if (status == MB_SUCCESS)
      status = mbsvp_area_mean(verbose, ncid, &variable[1], grid->ndepth, &lat, lat_a, lat_b, &lon, lon_a, lon_b, grid->leroy,
                               message, error);
    if (status == MB_SUCCESS) {
      /* the centre of the selected axes, not a single source observation */
      double sumlon = 0.0;
      double sumlat = 0.0;
      for (int i = lon_a; i < lon_b; i++)
        sumlon += lon.value[i];
      for (int i = lat_a; i < lat_b; i++)
        sumlat += lat.value[i];
      struct mbsvp_location *location = &grid->location;
      location->has_position = true;
      location->longitude = sumlon / (lon_b - lon_a);
      location->latitude = sumlat / (lat_b - lat_a);
      strcpy(location->method, "area_mean");
      location->has_area = true;
      location->west = lon.value[lon_a] - 360.0 * floor((lon.value[lon_a] + 180.0) / 360.0);
      location->east = lon.value[lon_b - 1] - 360.0 * floor((lon.value[lon_b - 1] + 180.0) / 360.0);
      location->south = lat.value[lat_a];
      location->north = lat.value[lat_b - 1];
      location->grid_lat = lat_b - lat_a;
      location->grid_lon = lon_b - lon_a;
      strcpy(location->representative, "selected_grid_axis_mean");
    }
  }

  nc_close(ncid);
  int error_free = MB_ERROR_NO_ERROR;
  if (lat_values != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lat_values, &error_free);
  if (lon_values != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lon_values, &error_free);
  if (lat.index != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lat.index, &error_free);
  if (lon.index != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lon.index, &error_free);
  if (buffer != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&buffer, &error_free);

  if (verbose >= 2) {
    fprintf(stderr, "dbg2  Function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2       ndepth:     %d\n", grid->ndepth);
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }
  return status;
}
/*--------------------------------------------------------------------*/
void mbsvp_atlas_init(struct mbsvp_atlas *atlas) {
  memset(atlas, 0, sizeof(struct mbsvp_atlas));
}
/*--------------------------------------------------------------------*/
int mbsvp_atlas_close(int verbose, struct mbsvp_atlas *atlas, int *error) {
  if (atlas->open)
    nc_close(atlas->ncid);
  atlas->open = false;
  if (atlas->lat != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&atlas->lat, error);
  if (atlas->lon != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&atlas->lon, error);
  if (atlas->depth != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&atlas->depth, error);
  if (atlas->buffer != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&atlas->buffer, error);
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Open a grid that must hold the wanted variables, each with one entry in
   any dimension other than depth, lat and lon. */
int mbsvp_atlas_open(int verbose, const char *path, const bool want[2], struct mbsvp_atlas *atlas, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       want:       %d %d\n", want[0], want[1]);
  }

  const char *names[2] = {"v_an_chen", "v_an_leroy"};
  const int nc_status = nc_open(path, NC_NOWRITE, &atlas->ncid);
  if (nc_status != NC_NOERR) {
    mbsvp_set_message(message, "%s: %s", path, nc_strerror(nc_status));
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  atlas->open = true;

  int status = mbsvp_axis_read(verbose, atlas->ncid, "lat", &atlas->lat, &atlas->nlat, message, error);
  if (status == MB_SUCCESS)
    status = mbsvp_axis_read(verbose, atlas->ncid, "lon", &atlas->lon, &atlas->nlon, message, error);
  if (status == MB_SUCCESS)
    status = mbsvp_axis_read(verbose, atlas->ncid, "depth", &atlas->depth, &atlas->ndepth, message, error);
  for (int v = 0; v < 2 && status == MB_SUCCESS; v++) {
    atlas->have[v] = false;
    if (!want[v])
      continue;
    if (mbsvp_variable_open(atlas->ncid, names[v], &atlas->variable[v]) != MB_SUCCESS) {
      mbsvp_set_message(message, "Missing %s in %s", names[v], path);
      *error = MB_ERROR_BAD_DATA;
      status = MB_FAILURE;
      break;
    }
    atlas->have[v] = true;
    int dimids[NC_MAX_VAR_DIMS];
    nc_inq_vardimid(atlas->ncid, atlas->variable[v].varid, dimids);
    for (int d = 0; d < atlas->variable[v].ndims; d++) {
      size_t len = 1;
      nc_inq_dimlen(atlas->ncid, dimids[d], &len);
      if (d != atlas->variable[v].depth_dim && d != atlas->variable[v].lat_dim && d != atlas->variable[v].lon_dim && len != 1) {
        mbsvp_set_message(message, "Annual dataset must have exactly one time entry: %s", path);
        *error = MB_ERROR_BAD_DATA;
        status = MB_FAILURE;
      }
    }
  }
  if (status == MB_SUCCESS)
    status = mb_mallocd(verbose, __FILE__, __LINE__, atlas->ndepth * sizeof(float), (void **)&atlas->buffer, error);
  return status;
}
/*--------------------------------------------------------------------*/
/* The sound speed column of one cell, with missing values as NaN. */
int mbsvp_atlas_read_cell(int verbose, const struct mbsvp_atlas *atlas, int variable, int lat_index, int lon_index, double *speed,
                          char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       cell:       %d %d\n", lat_index, lon_index);
  }

  if (!atlas->have[variable] ||
      mbsvp_variable_read(atlas->ncid, &atlas->variable[variable], 0, atlas->ndepth, lat_index, 1, lon_index, 1, atlas->buffer) !=
          MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot read sound speed");
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  for (int k = 0; k < atlas->ndepth; k++)
    speed[k] = atlas->buffer[k];
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mbsvp_write_profile(int verbose, const char *path, const struct mbsvp_profile *profile, const char *description,
                        const struct mbsvp_location *location, const struct mbsvp_location *donors, int ndonors,
                        char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       n:          %d\n", profile->n);
  }

  char comment[MBSVP_COMMENT_LIMIT + 2];
  FILE *fp = fopen(path, "w");
  if (fp == NULL) {
    mbsvp_set_message(message, "Cannot write %s", path);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  fprintf(fp, "# %s\n", description);
  if (mbsvp_location_comment(verbose, location, MBSVP_MARKER_LOCATION, comment, sizeof(comment), message, error) == MB_SUCCESS)
    fputs(comment, fp);
  else
    status = MB_FAILURE;
  for (int i = 0; i < ndonors && status == MB_SUCCESS; i++) {
    if (mbsvp_location_comment(verbose, &donors[i], MBSVP_MARKER_DONOR, comment, sizeof(comment), message, error) == MB_SUCCESS)
      fputs(comment, fp);
    else
      status = MB_FAILURE;
  }
  for (int i = 0; i < profile->n && status == MB_SUCCESS; i++)
    fprintf(fp, "%.6f %.9f\n", profile->depth[i], profile->speed[i]);
  if (ferror(fp) != 0) {
    mbsvp_set_message(message, "Cannot write %s", path);
    *error = MB_ERROR_WRITE_FAIL;
    status = MB_FAILURE;
  }
  if (fclose(fp) != 0) {
    mbsvp_set_message(message, "Cannot write %s", path);
    *error = MB_ERROR_WRITE_FAIL;
    status = MB_FAILURE;
  }
  if (status != MB_SUCCESS)
    remove(path);
  return status;
}
/*--------------------------------------------------------------------*/
/* Write one candidate of one kind and formula. Only samples really taken
   from a donor are recorded as donors. */
static int mbsvp_woa_write_candidate(int verbose, const char *folder, int year, const char *period, const char *resolution,
                                     const char *codes[MBSVP_KINDS], int kind, int formula,
                                     const struct mbsvp_grid grids[MBSVP_KINDS], const bool present[MBSVP_KINDS],
                                     const struct mbsvp_profile raw[MBSVP_KINDS],
                                     const struct mbsvp_profile *observed, struct mbsvp_strings *candidates, char *message,
                                     int *error) {
  struct mbsvp_profile complete;
  struct mbsvp_profile primary;
  struct mbsvp_profile donor_clean;
  mbsvp_profile_init(&complete);
  mbsvp_profile_init(&primary);
  mbsvp_profile_init(&donor_clean);
  struct mbsvp_location location = grids[kind].location;
  struct mbsvp_location donors[2];
  int ndonors = 0;

  int status = mbsvp_profile_complete(verbose, observed, MBSVP_TARGET_DEPTH, &complete, message, error);

  char name[512];
  char source[16];
  snprintf(source, sizeof(source), "WOA%d", year);
  snprintf(name, sizeof(name), "woa%d_%s_svp%s_%s", year, period, codes[kind], resolution);
  snprintf(location.source, sizeof(location.source), "%s", source);
  snprintf(location.dataset, sizeof(location.dataset), "%s.nc", name);
  snprintf(location.formula, sizeof(location.formula), "%s", mbsvp_formula_name[formula]);

  if (status == MB_SUCCESS)
    status = mbsvp_profile_clean(verbose, raw[kind].n, raw[kind].depth, raw[kind].speed, &primary, message, error);
  if (status == MB_SUCCESS) {
    double deepest = primary.n > 0 ? primary.depth[primary.n - 1] : -HUGE_VAL;
    int donor_kinds[2] = {-1, -1};
    if (kind == MBSVP_MONTHLY) {
      donor_kinds[0] = MBSVP_SEASONAL;
      donor_kinds[1] = MBSVP_ANNUAL;
    }
    else if (kind == MBSVP_SEASONAL) {
      donor_kinds[0] = MBSVP_ANNUAL;
    }
    for (int d = 0; d < 2 && status == MB_SUCCESS; d++) {
      const int donor_kind = donor_kinds[d];
      if (donor_kind < 0 || !present[donor_kind])
        continue;
      status = mbsvp_profile_clean(verbose, raw[donor_kind].n, raw[donor_kind].depth, raw[donor_kind].speed, &donor_clean,
                                   message, error);
      if (status != MB_SUCCESS)
        break;
      int first = -1;
      int last = -1;
      for (int i = 0; i < donor_clean.n; i++) {
        if (donor_clean.depth[i] > deepest) {
          if (first < 0)
            first = i;
          last = i;
        }
      }
      if (first < 0)
        continue;
      struct mbsvp_location *donor = &donors[ndonors++];
      *donor = grids[donor_kind].location;
      snprintf(donor->source, sizeof(donor->source), "%s", source);
      snprintf(donor->dataset, sizeof(donor->dataset), "woa%d_%s_svp%s_%s.nc", year, period, codes[donor_kind], resolution);
      snprintf(donor->kind, sizeof(donor->kind), "%s", mbsvp_kind_name[donor_kind]);
      donor->has_appended = true;
      donor->appended_min_depth = donor_clean.depth[first];
      donor->appended_max_depth = donor_clean.depth[last];
      deepest = donor_clean.depth[last];
    }
  }

  if (status == MB_SUCCESS) {
    mb_path file;
    mb_path path;
    char description[1024];
    snprintf(description, sizeof(description),
             "WOA%d; period=%s; time=%s; resolution=%s; formula=%s; "
             "NaNs removed before annual->seasonal->monthly tail extension; "
             "endpoint=11000m; remaining shortfall uses least-squares "
             "trend over deepest 2000m of valid climatological values",
             year, period, codes[kind], resolution, mbsvp_formula_name[formula]);
    const int len = snprintf(file, sizeof(file), "%s_%s.svp", name, mbsvp_formula_name[formula]);
    if (len < 0 || (size_t)len >= sizeof(file) || !mbsvp_join(path, sizeof(path), folder, file)) {
      mbsvp_set_message(message, "Output path is too long");
      *error = MB_ERROR_BAD_PARAMETER;
      status = MB_FAILURE;
    }
    if (status == MB_SUCCESS)
      status = mbsvp_write_profile(verbose, path, &complete, description, &location, donors, ndonors, message, error);
    if (status == MB_SUCCESS)
      status = mbsvp_strings_add(verbose, candidates, path, error);
  }

  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_profile_free(verbose, &complete, &error_free);
  mbsvp_profile_free(verbose, &primary, &error_free);
  mbsvp_profile_free(verbose, &donor_clean, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
static int mbsvp_note(int verbose, struct mbsvp_strings *notes, int year, const char *what, const char *why, int *error) {
  char text[2 * MBSVP_MESSAGE_SIZE];
  snprintf(text, sizeof(text), "WOA%d: %s: %s", year, what, why);
  return mbsvp_strings_add(verbose, notes, text, error);
}
/*--------------------------------------------------------------------*/
/* All candidates of one edition, period and resolution. A kind or formula
   that fails is noted and the others go on. */
static int mbsvp_woa_generate_atlas(int verbose, const char *nc_folder, const char *folder,
                                    const struct mbsvp_survey *survey, int year, const char *period, const char *resolution,
                                    bool nearest, struct mbsvp_strings *candidates, struct mbsvp_strings *notes, int *error) {
  const char *codes[MBSVP_KINDS] = {"00", survey->season, survey->month};
  /* candidates are presented in this order; the annual grid is read first */
  const int order[MBSVP_KINDS] = {MBSVP_SEASONAL, MBSVP_MONTHLY, MBSVP_ANNUAL};
  struct mbsvp_grid grids[MBSVP_KINDS];
  bool present[MBSVP_KINDS];
  char message[MBSVP_MESSAGE_SIZE];
  int status = MB_SUCCESS;

  for (int kind = 0; kind < MBSVP_KINDS; kind++) {
    char name[512];
    mb_path file;
    mb_path path;
    mbsvp_grid_init(&grids[kind]);
    present[kind] = false;
    snprintf(name, sizeof(name), "woa%d_%s_svp%s_%s", year, period, codes[kind], resolution);
    const int len = snprintf(file, sizeof(file), "%s.nc", name);
    message[0] = '\0';
    int local_error = MB_ERROR_NO_ERROR;
    if (len < 0 || (size_t)len >= sizeof(file) || !mbsvp_join(path, sizeof(path), nc_folder, file)) {
      mbsvp_set_message(message, "Path is too long");
    }
    else if (mbsvp_woa_read_grid(verbose, path, survey, nearest, &grids[kind], message, &local_error) == MB_SUCCESS) {
      present[kind] = true;
    }
    if (!present[kind]) {
      if (message[0] == '\0') {
        char *mbio_message;
        mb_error(verbose, local_error, &mbio_message);
        mbsvp_set_message(message, "%s", mbio_message);
      }
      status = mbsvp_note(verbose, notes, year, name, message, error);
    }
  }

  for (int formula = 0; formula < 2 && status == MB_SUCCESS; formula++) {
    struct mbsvp_profile raw[MBSVP_KINDS];
    const struct mbsvp_profile *raw_pointers[MBSVP_KINDS];
    struct mbsvp_profile observed[MBSVP_KINDS];
    for (int kind = 0; kind < MBSVP_KINDS; kind++) {
      raw[kind].n = grids[kind].ndepth;
      raw[kind].depth = grids[kind].depth;
      raw[kind].speed = formula == 0 ? grids[kind].chen : grids[kind].leroy;
      raw_pointers[kind] = present[kind] ? &raw[kind] : NULL;
      mbsvp_profile_init(&observed[kind]);
    }
    message[0] = '\0';
    int local_error = MB_ERROR_NO_ERROR;
    if (mbsvp_profile_chain(verbose, raw_pointers, observed, message, &local_error) != MB_SUCCESS) {
      char what[512];
      snprintf(what, sizeof(what), "%s/%s/%s", period, resolution, mbsvp_formula_name[formula]);
      status = mbsvp_note(verbose, notes, year, what, message, error);
    }
    else {
      for (int k = 0; k < MBSVP_KINDS && status == MB_SUCCESS; k++) {
        const int kind = order[k];
        if (!present[kind])
          continue;
        message[0] = '\0';
        local_error = MB_ERROR_NO_ERROR;
        if (mbsvp_woa_write_candidate(verbose, folder, year, period, resolution, codes, kind, formula, grids, present, raw,
                                      &observed[kind], candidates, message, &local_error) != MB_SUCCESS) {
          char what[512];
          snprintf(what, sizeof(what), "woa%d_%s_svp%s_%s/%s", year, period, codes[kind], resolution,
                   mbsvp_formula_name[formula]);
          status = mbsvp_note(verbose, notes, year, what, message, error);
        }
      }
    }
    int error_free = MB_ERROR_NO_ERROR;
    for (int kind = 0; kind < MBSVP_KINDS; kind++)
      mbsvp_profile_free(verbose, &observed[kind], &error_free);
  }

  int error_free = MB_ERROR_NO_ERROR;
  for (int kind = 0; kind < MBSVP_KINDS; kind++)
    mbsvp_grid_free(verbose, &grids[kind], &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
/* Scan the direct children of nc_folder and generate the candidates of each
   edition, period and resolution with annual, seasonal or monthly files
   for this survey. */
int mbsvp_woa_generate(int verbose, const char *nc_folder, const char *folder, const struct mbsvp_survey *survey,
                       const struct mbsvp_strings *periods, const struct mbsvp_strings *resolutions, bool nearest,
                       struct mbsvp_strings *candidates, struct mbsvp_strings *notes, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       nc_folder:  %s\n", nc_folder);
    fprintf(stderr, "dbg2       folder:     %s\n", folder);
  }

  if (!mbsvp_is_dir(nc_folder)) {
    mbsvp_set_message(message, "SVP directory does not exist: %s", nc_folder);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  if (survey->depth > MBSVP_TARGET_DEPTH) {
    mbsvp_set_message(message, "Survey maximum depth exceeds the required 11000 m profile endpoint");
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }

  struct mbsvp_strings names;
  struct mbsvp_strings available;
  mbsvp_strings_init(&names);
  mbsvp_strings_init(&available);
  int status = mbsvp_list_dir(verbose, nc_folder, &names, error);
  if (status != MB_SUCCESS)
    mbsvp_set_message(message, "Cannot read SVP directory: %s", nc_folder);
  for (int i = 0; i < names.n && status == MB_SUCCESS; i++) {
    int year;
    char period[256];
    char time_code[3];
    char resolution[3];
    if (!mbsvp_woa_parse_name(names.items[i], &year, period, sizeof(period), time_code, resolution))
      continue;
    if (strcmp(time_code, "00") != 0 && strcmp(time_code, survey->season) != 0 && strcmp(time_code, survey->month) != 0)
      continue;
    mb_path path;
    if (!mbsvp_join(path, sizeof(path), nc_folder, names.items[i]) || !mbsvp_is_file(path))
      continue;
    char key[512];
    snprintf(key, sizeof(key), "%d/%s/%s", year, period, resolution);
    if (!mbsvp_strings_contains(&available, key))
      status = mbsvp_strings_add(verbose, &available, key, error);
  }

  const int years[2] = {18, 23};
  bool any = false;
  for (int y = 0; y < 2 && status == MB_SUCCESS; y++) {
    for (int p = 0; p < periods->n && status == MB_SUCCESS; p++) {
      for (int r = 0; r < resolutions->n && status == MB_SUCCESS; r++) {
        char key[512];
        snprintf(key, sizeof(key), "%d/%s/%s", years[y], periods->items[p], resolutions->items[r]);
        if (!mbsvp_strings_contains(&available, key))
          continue;
        any = true;
        status = mbsvp_woa_generate_atlas(verbose, nc_folder, folder, survey, years[y], periods->items[p],
                                          resolutions->items[r], nearest, candidates, notes, error);
      }
    }
  }
  if (status == MB_SUCCESS && !any)
    status = mbsvp_strings_add(verbose, notes,
                               "No WOA18/WOA23 sound-speed NetCDF files match the requested periods, resolutions and "
                               "survey month/season (or annual time 00).",
                               error);

  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_strings_free(verbose, &names, &error_free);
  mbsvp_strings_free(verbose, &available, &error_free);

  if (verbose >= 2) {
    fprintf(stderr, "dbg2  Function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2       candidates: %d\n", candidates->n);
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }
  return status;
}
/*--------------------------------------------------------------------*/
