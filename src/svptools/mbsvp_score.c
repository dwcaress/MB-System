/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_score.c  3 November 2026
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
 * Residual score of swath bathymetry. In each ping the depth is fitted by
 * a straight line against acrosstrack distance; the residuals of all pings
 * are pooled, so that signed errors cannot cancel before the squares are
 * taken. The score describes the geometry of the swath only: it does not
 * show that a profile is the true water column.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

/*--------------------------------------------------------------------*/
void mbsvp_score_init(struct mbsvp_score *score, bool keep_beams) {
  memset(score, 0, sizeof(struct mbsvp_score));
  score->keep_beams = keep_beams;
}
/*--------------------------------------------------------------------*/
int mbsvp_score_free(int verbose, struct mbsvp_score *score, int *error) {
  if (score->beam_count != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&score->beam_count, error);
  if (score->beam_sum != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&score->beam_sum, error);
  if (score->beam_sum2 != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&score->beam_sum2, error);
  score->nbeams = 0;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static int mbsvp_score_beams(int verbose, int nbeams, struct mbsvp_score *score, int *error) {
  if (nbeams <= score->nbeams)
    return MB_SUCCESS;
  if (mb_reallocd(verbose, __FILE__, __LINE__, nbeams * sizeof(long), (void **)&score->beam_count, error) != MB_SUCCESS ||
      mb_reallocd(verbose, __FILE__, __LINE__, nbeams * sizeof(double), (void **)&score->beam_sum, error) != MB_SUCCESS ||
      mb_reallocd(verbose, __FILE__, __LINE__, nbeams * sizeof(double), (void **)&score->beam_sum2, error) != MB_SUCCESS)
    return MB_FAILURE;
  for (int i = score->nbeams; i < nbeams; i++) {
    score->beam_count[i] = 0;
    score->beam_sum[i] = 0.0;
    score->beam_sum2[i] = 0.0;
  }
  score->nbeams = nbeams;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Score one ping from its listed beams. Soundings with a nonfinite value
   are left out. A ping with too few soundings, or without distinct
   acrosstrack positions, counts as rejected. */
int mbsvp_score_ping(int verbose, int n, const int *beam, const double *across, const double *depth, int minimum,
                     struct mbsvp_score *score, int *error) {
  if (n <= 0)
    return MB_SUCCESS;

  int count = 0;
  double xmin = 0.0;
  double xmax = 0.0;
  double sumx = 0.0;
  double sumz = 0.0;
  for (int i = 0; i < n; i++) {
    if (isfinite(across[i]) && isfinite(depth[i])) {
      if (count == 0 || across[i] < xmin)
        xmin = across[i];
      if (count == 0 || across[i] > xmax)
        xmax = across[i];
      sumx += across[i];
      sumz += depth[i];
      count++;
    }
  }
  if (count < minimum || xmax - xmin <= 1.0e-6) {
    score->rejected_pings++;
    return MB_SUCCESS;
  }

  const double meanx = sumx / count;
  const double meanz = sumz / count;
  double sxx = 0.0;
  double sxz = 0.0;
  for (int i = 0; i < n; i++) {
    if (isfinite(across[i]) && isfinite(depth[i])) {
      const double dx = across[i] - meanx;
      sxx += dx * dx;
      sxz += dx * (depth[i] - meanz);
    }
  }
  const double slope = sxz / sxx;

  for (int i = 0; i < n; i++) {
    if (isfinite(across[i]) && isfinite(depth[i])) {
      const double residual = depth[i] - (meanz + slope * (across[i] - meanx));
      score->sum += residual;
      score->sum_abs += fabs(residual);
      score->sum2 += residual * residual;
      score->soundings++;
      if (score->keep_beams && beam[i] >= 0) {
        if (mbsvp_score_beams(verbose, beam[i] + 1, score, error) != MB_SUCCESS)
          return MB_FAILURE;
        score->beam_count[beam[i]]++;
        score->beam_sum[beam[i]] += residual;
        score->beam_sum2[beam[i]] += residual * residual;
      }
    }
  }
  score->pings++;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Add one swath file to the score. Flagged and null beams are left out.
   Each survey record is one ping. */
int mbsvp_score_file(int verbose, char *file, int format, int minimum, struct mbsvp_score *score, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       file:       %s\n", file);
    fprintf(stderr, "dbg2       format:     %d\n", format);
    fprintf(stderr, "dbg2       minimum:    %d\n", minimum);
  }

  int format_default;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  mb_defaults(verbose, &format_default, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
  pings = 1;

  void *mbio_ptr = NULL;
  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr, &btime_d,
                   &etime_d, &beams_bath, &beams_amp, &pixels_ss, error) != MB_SUCCESS) {
    char *mbio_message;
    mb_error(verbose, *error, &mbio_message);
    mbsvp_set_message(message, "Cannot read %s: %s", file, mbio_message);
    return MB_FAILURE;
  }

  char *beamflag = NULL;
  double *bath = NULL;
  double *amp = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  int status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, error);

  /* per-ping arrays of the listed beams */
  int *list_beam = NULL;
  double *list_across = NULL;
  double *list_depth = NULL;
  int list_alloc = 0;

  const long soundings_start = score->soundings;
  long listed_pings = 0;

  while (status == MB_SUCCESS && !mbsvp_interrupted) {
    void *store_ptr = NULL;
    int kind = MB_DATA_NONE;
    int time_i[7];
    double time_d;
    double navlon;
    double navlat;
    double speed;
    double heading;
    double distance;
    double altitude;
    double sensordepth;
    char comment[MB_COMMENT_MAXLINE];
    int local_error = MB_ERROR_NO_ERROR;
    mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude,
               &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss,
               ssacrosstrack, ssalongtrack, comment, &local_error);

    /* time gaps are not a problem here */
    if (local_error == MB_ERROR_TIME_GAP)
      local_error = MB_ERROR_NO_ERROR;
    if (local_error > MB_ERROR_NO_ERROR)
      break;
    if (local_error != MB_ERROR_NO_ERROR || kind != MB_DATA_DATA)
      continue;

    if (beams_bath > list_alloc) {
      list_alloc = beams_bath;
      if (mb_reallocd(verbose, __FILE__, __LINE__, list_alloc * sizeof(int), (void **)&list_beam, error) != MB_SUCCESS ||
          mb_reallocd(verbose, __FILE__, __LINE__, list_alloc * sizeof(double), (void **)&list_across, error) != MB_SUCCESS ||
          mb_reallocd(verbose, __FILE__, __LINE__, list_alloc * sizeof(double), (void **)&list_depth, error) != MB_SUCCESS) {
        status = MB_FAILURE;
        break;
      }
    }
    int n = 0;
    for (int j = 0; j < beams_bath; j++) {
      if (mb_beam_ok(beamflag[j])) {
        list_beam[n] = j;
        list_across[n] = bathacrosstrack[j];
        list_depth[n] = bath[j];
        n++;
      }
    }
    if (n > 0)
      listed_pings++;
    status = mbsvp_score_ping(verbose, n, list_beam, list_across, list_depth, minimum, score, error);
  }

  int error_close = MB_ERROR_NO_ERROR;
  mb_close(verbose, &mbio_ptr, &error_close);
  int error_free = MB_ERROR_NO_ERROR;
  if (list_beam != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&list_beam, &error_free);
  if (list_across != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&list_across, &error_free);
  if (list_depth != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&list_depth, &error_free);

  if (status == MB_SUCCESS && !mbsvp_interrupted) {
    if (listed_pings == 0) {
      mbsvp_set_message(message, "No usable bathymetry in %s", file);
      *error = MB_ERROR_BAD_DATA;
      status = MB_FAILURE;
    }
    else if (score->soundings == soundings_start) {
      mbsvp_set_message(message, "No pings with enough valid, distinct across-track positions");
      *error = MB_ERROR_BAD_DATA;
      status = MB_FAILURE;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "dbg2  Function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2       soundings:  %ld\n", score->soundings);
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }
  return status;
}
/*--------------------------------------------------------------------*/
static int mbsvp_compare(const void *a, const void *b) {
  const double x = *(const double *)a;
  const double y = *(const double *)b;
  return x < y ? -1 : (x > y ? 1 : 0);
}
/*--------------------------------------------------------------------*/
/* The box around ping positions, without positions far from all the others,
   such as pings before the first navigation record of a file that has not
   been through mbpreprocess. Distance from the median position is compared
   with the 90th percentile of all distances, so that a long line is safe.
   margin is added to each side, in metres. Returns the number of positions
   kept. The arrays are not changed. */
int mbsvp_position_box(const double *lon, const double *lat, int n, double margin, double box[4], int *noutliers) {
  *noutliers = 0;
  if (n <= 0)
    return 0;
  double *dx = (double *)malloc(n * sizeof(double));
  double *dy = (double *)malloc(n * sizeof(double));
  double *dd = (double *)malloc(n * sizeof(double));
  if (dx == NULL || dy == NULL || dd == NULL) {
    free(dx);
    free(dy);
    free(dd);
    return 0;
  }
  /* longitudes on one circular branch, around a circular mean */
  double sumx = 0.0;
  double sumy = 0.0;
  for (int i = 0; i < n; i++) {
    sumx += cos(DTR * lon[i]);
    sumy += sin(DTR * lon[i]);
  }
  const double reference = RTD * atan2(sumy, sumx);
  for (int i = 0; i < n; i++) {
    double d = fmod(lon[i] - reference + 180.0, 360.0);
    if (d < 0.0)
      d += 360.0;
    dx[i] = reference + d - 180.0;
    dy[i] = lat[i];
    dd[i] = dx[i];
  }
  qsort(dd, n, sizeof(double), mbsvp_compare);
  const double medlon = dd[n / 2];
  memcpy(dd, dy, n * sizeof(double));
  qsort(dd, n, sizeof(double), mbsvp_compare);
  const double medlat = dd[n / 2];
  const double scale = MAX(cos(DTR * medlat), 0.05);
  for (int i = 0; i < n; i++) {
    const double a = (dx[i] - medlon) * scale;
    const double b = dy[i] - medlat;
    dd[i] = sqrt(a * a + b * b);
  }
  double *sorted = (double *)malloc(n * sizeof(double));
  int nkept = 0;
  if (sorted != NULL) {
    memcpy(sorted, dd, n * sizeof(double));
    qsort(sorted, n, sizeof(double), mbsvp_compare);
    const double limit = MAX(1.0, 5.0 * sorted[(int)(0.9 * (n - 1))]);
    for (int i = 0; i < n; i++) {
      if (dd[i] > limit) {
        (*noutliers)++;
        continue;
      }
      if (nkept == 0 || dx[i] < box[0])
        box[0] = dx[i];
      if (nkept == 0 || dx[i] > box[1])
        box[1] = dx[i];
      if (nkept == 0 || dy[i] < box[2])
        box[2] = dy[i];
      if (nkept == 0 || dy[i] > box[3])
        box[3] = dy[i];
      nkept++;
    }
    const double mperdeg = 111120.0;
    box[0] -= margin / (mperdeg * scale);
    box[1] += margin / (mperdeg * scale);
    box[2] -= margin / mperdeg;
    box[3] += margin / mperdeg;
  }
  free(sorted);
  free(dx);
  free(dy);
  free(dd);
  return nkept;
}
/*--------------------------------------------------------------------*/
/* Read the ping positions of a file and find its box. */
int mbsvp_navigation_box(int verbose, char *file, int format, double box[4], int *npings, int *noutliers, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       file:       %s\n", file);
    fprintf(stderr, "dbg2       format:     %d\n", format);
  }

  *npings = 0;
  *noutliers = 0;
  int format_default;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  mb_defaults(verbose, &format_default, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
  pings = 1;

  void *mbio_ptr = NULL;
  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr, &btime_d, &etime_d,
                   &beams_bath, &beams_amp, &pixels_ss, error) != MB_SUCCESS) {
    char *mbio_message;
    mb_error(verbose, *error, &mbio_message);
    mbsvp_set_message(message, "Cannot read %s: %s", file, mbio_message);
    return MB_FAILURE;
  }
  char *beamflag = NULL;
  double *bath = NULL;
  double *amp = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  int status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, error);
  if (status == MB_SUCCESS)
    status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, error);

  double *lon = NULL;
  double *lat = NULL;
  int alloc = 0;
  double reach = 0.0;
  while (status == MB_SUCCESS && !mbsvp_interrupted) {
    void *store_ptr = NULL;
    int kind = MB_DATA_NONE;
    int time_i[7];
    double time_d;
    double navlon;
    double navlat;
    double speed;
    double heading;
    double distance;
    double altitude;
    double sensordepth;
    char comment[MB_COMMENT_MAXLINE];
    int local_error = MB_ERROR_NO_ERROR;
    mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance, &altitude,
               &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack,
               ssalongtrack, comment, &local_error);
    if (local_error == MB_ERROR_TIME_GAP)
      local_error = MB_ERROR_NO_ERROR;
    if (local_error > MB_ERROR_NO_ERROR)
      break;
    if (local_error != MB_ERROR_NO_ERROR || kind != MB_DATA_DATA)
      continue;
    if (*npings >= alloc) {
      alloc += 4096;
      if (mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(double), (void **)&lon, error) != MB_SUCCESS ||
          mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(double), (void **)&lat, error) != MB_SUCCESS) {
        status = MB_FAILURE;
        break;
      }
    }
    lon[*npings] = navlon;
    lat[*npings] = navlat;
    (*npings)++;
    for (int j = 0; j < beams_bath; j++)
      if (mb_beam_ok(beamflag[j]) && isfinite(bathacrosstrack[j]) && fabs(bathacrosstrack[j]) > reach)
        reach = fabs(bathacrosstrack[j]);
  }
  int error_close = MB_ERROR_NO_ERROR;
  mb_close(verbose, &mbio_ptr, &error_close);

  if (status == MB_SUCCESS && !mbsvp_interrupted && mbsvp_position_box(lon, lat, *npings, reach, box, noutliers) == 0) {
    mbsvp_set_message(message, "No ping positions in %s", file);
    *error = MB_ERROR_BAD_DATA;
    status = MB_FAILURE;
  }
  int error_free = MB_ERROR_NO_ERROR;
  if (lon != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lon, &error_free);
  if (lat != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&lat, &error_free);

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, pings %d outliers %d status %d\n", __func__, *npings, *noutliers, status);
  return status;
}
/*--------------------------------------------------------------------*/
