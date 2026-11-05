/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_profile.c  3 November 2026
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
 * Completion of climatological sound velocity profiles to 11000 m.
 *
 * Invalid depth and speed pairs are removed first. A seasonal profile
 * then takes deeper valid annual samples and a monthly profile takes the
 * deeper seasonal and annual samples; no value is changed at a join.
 * Only after that is each profile completed: a profile that stops short
 * gets an endpoint from a least-squares line through the samples in its
 * deepest 2000 m, and a deeper one is cut at the target by interpolation.
 * A fitted endpoint is never passed on as a donor sample.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

/*--------------------------------------------------------------------*/
void mbsvp_profile_init(struct mbsvp_profile *profile) {
  profile->n = 0;
  profile->depth = NULL;
  profile->speed = NULL;
}
/*--------------------------------------------------------------------*/
int mbsvp_profile_free(int verbose, struct mbsvp_profile *profile, int *error) {
  if (profile->depth != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&profile->depth, error);
  if (profile->speed != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&profile->speed, error);
  profile->n = 0;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static int mbsvp_profile_alloc(int verbose, int n, struct mbsvp_profile *profile, int *error) {
  mbsvp_profile_free(verbose, profile, error);
  const size_t size = (n > 0 ? n : 1) * sizeof(double);
  if (mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&profile->depth, error) != MB_SUCCESS ||
      mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&profile->speed, error) != MB_SUCCESS) {
    int error_free = MB_ERROR_NO_ERROR;
    mbsvp_profile_free(verbose, profile, &error_free);
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mbsvp_profile_clean(int verbose, int n, const double *depth, const double *speed, struct mbsvp_profile *clean,
                        char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       n:          %d\n", n);
  }

  if (mbsvp_profile_alloc(verbose, n, clean, error) != MB_SUCCESS)
    return MB_FAILURE;

  /* keep valid pairs, in stable depth order */
  int count = 0;
  for (int i = 0; i < n; i++) {
    if (isfinite(depth[i]) && isfinite(speed[i]) && depth[i] >= 0.0) {
      int j = count - 1;
      while (j >= 0 && clean->depth[j] > depth[i]) {
        clean->depth[j + 1] = clean->depth[j];
        clean->speed[j + 1] = clean->speed[j];
        j--;
      }
      clean->depth[j + 1] = depth[i];
      clean->speed[j + 1] = speed[i];
      count++;
    }
  }
  clean->n = count;

  for (int i = 1; i < count; i++) {
    if (clean->depth[i] <= clean->depth[i - 1]) {
      mbsvp_set_message(message, "Duplicate depths in profile");
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
  }

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, valid pairs: %d\n", __func__, count);
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Append donor samples deeper than the last valid primary sample. */
int mbsvp_profile_tail(int verbose, const struct mbsvp_profile *primary, const struct mbsvp_profile *donor,
                       struct mbsvp_profile *result, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       primary n:  %d\n", primary->n);
    fprintf(stderr, "dbg2       donor n:    %d\n", donor != NULL ? donor->n : 0);
  }

  struct mbsvp_profile z;
  struct mbsvp_profile d;
  mbsvp_profile_init(&z);
  mbsvp_profile_init(&d);
  int status = mbsvp_profile_clean(verbose, primary->n, primary->depth, primary->speed, &z, message, error);
  if (status == MB_SUCCESS && donor != NULL)
    status = mbsvp_profile_clean(verbose, donor->n, donor->depth, donor->speed, &d, message, error);
  if (status == MB_SUCCESS)
    status = mbsvp_profile_alloc(verbose, z.n + d.n, result, error);
  if (status == MB_SUCCESS) {
    int count = 0;
    for (int i = 0; i < z.n; i++) {
      result->depth[count] = z.depth[i];
      result->speed[count] = z.speed[i];
      count++;
    }
    for (int i = 0; i < d.n; i++) {
      if (z.n == 0 || d.depth[i] > z.depth[z.n - 1]) {
        result->depth[count] = d.depth[i];
        result->speed[count] = d.speed[i];
        count++;
      }
    }
    result->n = count;
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_profile_free(verbose, &z, &error_free);
  mbsvp_profile_free(verbose, &d, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
int mbsvp_profile_complete(int verbose, const struct mbsvp_profile *observed, double target,
                           struct mbsvp_profile *complete, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       observed n: %d\n", observed->n);
    fprintf(stderr, "dbg2       target:     %f\n", target);
  }

  struct mbsvp_profile z;
  mbsvp_profile_init(&z);
  int status = mbsvp_profile_clean(verbose, observed->n, observed->depth, observed->speed, &z, message, error);
  double endpoint = 0.0;
  int nkeep = z.n;
  if (status == MB_SUCCESS && z.n < 2) {
    mbsvp_set_message(message, "Need at least two valid depth/speed pairs");
    *error = MB_ERROR_BAD_DATA;
    status = MB_FAILURE;
  }
  if (status == MB_SUCCESS && z.depth[0] > 1.0) {
    mbsvp_set_message(message, "Profile is missing near-surface coverage");
    *error = MB_ERROR_BAD_DATA;
    status = MB_FAILURE;
  }
  if (status == MB_SUCCESS) {
    const double deepest = z.depth[z.n - 1];
    if (deepest >= target) {
      /* interpolate at the target and cut the profile there */
      nkeep = 0;
      while (nkeep < z.n && z.depth[nkeep] < target)
        nkeep++;
      if (nkeep == 0 || z.depth[nkeep] == target) {
        endpoint = z.speed[nkeep];
      }
      else {
        const double fraction = (target - z.depth[nkeep - 1]) / (z.depth[nkeep] - z.depth[nkeep - 1]);
        endpoint = z.speed[nkeep - 1] + fraction * (z.speed[nkeep] - z.speed[nkeep - 1]);
      }
    }
    else {
      /* straight line through the samples in the deepest 2000 m */
      int nfit = 0;
      double sumx = 0.0;
      double sumy = 0.0;
      for (int i = 0; i < z.n; i++) {
        if (z.depth[i] >= deepest - MBSVP_TREND_WINDOW) {
          sumx += z.depth[i] - deepest;
          sumy += z.speed[i];
          nfit++;
        }
      }
      if (nfit < 2) {
        mbsvp_set_message(message, "Need at least two valid values in deepest 2000 m for trend");
        *error = MB_ERROR_BAD_DATA;
        status = MB_FAILURE;
      }
      else {
        const double meanx = sumx / nfit;
        const double meany = sumy / nfit;
        double sxx = 0.0;
        double sxy = 0.0;
        for (int i = 0; i < z.n; i++) {
          if (z.depth[i] >= deepest - MBSVP_TREND_WINDOW) {
            const double dx = z.depth[i] - deepest - meanx;
            sxx += dx * dx;
            sxy += dx * (z.speed[i] - meany);
          }
        }
        const double slope = sxy / sxx;
        endpoint = meany + slope * (target - deepest - meanx);
        if (!isfinite(endpoint)) {
          mbsvp_set_message(message, "Nonfinite extrapolated sound speed");
          *error = MB_ERROR_BAD_DATA;
          status = MB_FAILURE;
        }
      }
    }
  }
  if (status == MB_SUCCESS)
    status = mbsvp_profile_alloc(verbose, nkeep + 1, complete, error);
  if (status == MB_SUCCESS) {
    for (int i = 0; i < nkeep; i++) {
      complete->depth[i] = z.depth[i];
      complete->speed[i] = z.speed[i];
    }
    complete->depth[nkeep] = target;
    complete->speed[nkeep] = endpoint;
    complete->n = nkeep + 1;
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_profile_free(verbose, &z, &error_free);

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, status: %d\n", __func__, status);
  return status;
}
/*--------------------------------------------------------------------*/
/* Annual, then seasonal with annual tail, then monthly with the combined
   tail. A missing kind has raw[kind] == NULL and an unset result. */
int mbsvp_profile_chain(int verbose, const struct mbsvp_profile *raw[MBSVP_KINDS],
                        struct mbsvp_profile observed[MBSVP_KINDS], char *message, int *error) {
  int status = MB_SUCCESS;
  if (raw[MBSVP_ANNUAL] != NULL)
    status = mbsvp_profile_tail(verbose, raw[MBSVP_ANNUAL], NULL, &observed[MBSVP_ANNUAL], message, error);
  if (status == MB_SUCCESS && raw[MBSVP_SEASONAL] != NULL) {
    const struct mbsvp_profile *donor = raw[MBSVP_ANNUAL] != NULL ? &observed[MBSVP_ANNUAL] : NULL;
    status = mbsvp_profile_tail(verbose, raw[MBSVP_SEASONAL], donor, &observed[MBSVP_SEASONAL], message, error);
  }
  if (status == MB_SUCCESS && raw[MBSVP_MONTHLY] != NULL) {
    const struct mbsvp_profile *donor = NULL;
    if (raw[MBSVP_SEASONAL] != NULL)
      donor = &observed[MBSVP_SEASONAL];
    else if (raw[MBSVP_ANNUAL] != NULL)
      donor = &observed[MBSVP_ANNUAL];
    status = mbsvp_profile_tail(verbose, raw[MBSVP_MONTHLY], donor, &observed[MBSVP_MONTHLY], message, error);
  }
  return status;
}
/*--------------------------------------------------------------------*/
/* Read a profile embedded in swath data, as written by mbsvplist -O. It
   is usable if it has at least two samples, starts within 1 m of the
   surface, reaches the maximum depth, has strictly increasing depths and
   plausible speeds. */
int mbsvp_profile_read_internal(int verbose, const char *path, double maximum, struct mbsvp_profile *profile, bool *valid,
                                int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       maximum:    %f\n", maximum);
  }

  *valid = false;
  FILE *fp = fopen(path, "r");
  if (fp == NULL)
    return MB_SUCCESS;

  int alloc = 0;
  int count = 0;
  bool ok = true;
  int status = MB_SUCCESS;
  char line[8192];
  while (ok && status == MB_SUCCESS && fgets(line, sizeof(line), fp) != NULL) {
    /* a line longer than the buffer can only be a comment: drop its tail */
    const size_t len = strlen(line);
    const bool whole = len > 0 && line[len - 1] == '\n';
    const char *text = line;
    while (isspace((unsigned char)*text))
      text++;
    const bool skip = *text == '\0' || *text == '#' || *text == '>';
    if (!whole) {
      int c;
      while ((c = fgetc(fp)) != EOF && c != '\n') {
      }
    }
    if (skip)
      continue;

    char *end;
    const double depth = strtod(text, &end);
    if (end == text) {
      ok = false;
      break;
    }
    const char *next = end;
    const double speed = strtod(next, &end);
    if (end == next) {
      ok = false;
      break;
    }
    while (isspace((unsigned char)*end))
      end++;
    if (*end != '\0') {
      ok = false;
      break;
    }
    if (count >= alloc) {
      alloc += 1024;
      if (mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(double), (void **)&profile->depth, error) != MB_SUCCESS ||
          mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(double), (void **)&profile->speed, error) != MB_SUCCESS) {
        status = MB_FAILURE;
        break;
      }
    }
    profile->depth[count] = depth;
    profile->speed[count] = speed;
    count++;
  }
  fclose(fp);
  profile->n = count;

  if (status == MB_SUCCESS && ok && count >= 2 && profile->depth[0] <= 1.0 && profile->depth[count - 1] >= maximum) {
    bool good = true;
    for (int i = 0; i < count && good; i++) {
      if (!isfinite(profile->depth[i]) || !isfinite(profile->speed[i]) || profile->speed[i] <= 1200.0 ||
          profile->speed[i] >= 2000.0)
        good = false;
      if (i > 0 && profile->depth[i] <= profile->depth[i - 1])
        good = false;
    }
    *valid = good;
  }

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, samples: %d valid: %d\n", __func__, count, *valid);
  return status;
}
/*--------------------------------------------------------------------*/
bool mbsvp_profile_equal(const struct mbsvp_profile *a, const struct mbsvp_profile *b) {
  if (a->n != b->n)
    return false;
  for (int i = 0; i < a->n; i++)
    if (a->depth[i] != b->depth[i] || a->speed[i] != b->speed[i])
      return false;
  return true;
}
/*--------------------------------------------------------------------*/
