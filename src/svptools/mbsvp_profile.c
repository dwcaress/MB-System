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
 * gets a tail in which only pressure changes the speed (mbsvp_soundspeed.c),
 * and a deeper one is cut at the target by interpolation. A computed tail is
 * never passed on as a donor sample.
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
int mbsvp_profile_complete(int verbose, const struct mbsvp_profile *observed, double target, enum mbsvp_formula formula,
                           double latitude, struct mbsvp_profile *complete, double *physics_from, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       observed n: %d\n", observed->n);
    fprintf(stderr, "dbg2       target:     %f\n", target);
    fprintf(stderr, "dbg2       formula:    %d\n", formula);
    fprintf(stderr, "dbg2       latitude:   %f\n", latitude);
  }

  struct mbsvp_profile z;
  mbsvp_profile_init(&z);
  int status = mbsvp_profile_clean(verbose, observed->n, observed->depth, observed->speed, &z, message, error);
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
  if (physics_from != NULL)
    *physics_from = target;

  if (status == MB_SUCCESS && z.depth[z.n - 1] >= target) {
    /* interpolate at the target and cut the profile there */
    int nkeep = 0;
    while (nkeep < z.n && z.depth[nkeep] < target)
      nkeep++;
    double endpoint;
    if (nkeep == 0 || z.depth[nkeep] == target) {
      endpoint = z.speed[nkeep];
    }
    else {
      const double fraction = (target - z.depth[nkeep - 1]) / (z.depth[nkeep] - z.depth[nkeep - 1]);
      endpoint = z.speed[nkeep - 1] + fraction * (z.speed[nkeep] - z.speed[nkeep - 1]);
    }
    status = mbsvp_profile_alloc(verbose, nkeep + 1, complete, error);
    if (status == MB_SUCCESS) {
      memcpy(complete->depth, z.depth, nkeep * sizeof(double));
      memcpy(complete->speed, z.speed, nkeep * sizeof(double));
      complete->depth[nkeep] = target;
      complete->speed[nkeep] = endpoint;
      complete->n = nkeep + 1;
    }
  }
  else if (status == MB_SUCCESS) {
    /* samples of the tail at whole multiples of the step, and at the target */
    const double deepest = z.depth[z.n - 1];
    if (physics_from != NULL)
      *physics_from = deepest;
    int ntail = 0;
    for (double zt = (floor(deepest / MBSVP_PHYSICS_STEP) + 1.0) * MBSVP_PHYSICS_STEP; zt < target; zt += MBSVP_PHYSICS_STEP)
      ntail++;
    ntail++;
    status = mbsvp_profile_alloc(verbose, z.n + ntail, complete, error);
    if (status == MB_SUCCESS) {
      memcpy(complete->depth, z.depth, z.n * sizeof(double));
      memcpy(complete->speed, z.speed, z.n * sizeof(double));
      for (int i = 0; i < ntail - 1; i++)
        complete->depth[z.n + i] = (floor(deepest / MBSVP_PHYSICS_STEP) + 1.0 + i) * MBSVP_PHYSICS_STEP;
      complete->depth[z.n + ntail - 1] = target;
      status = mbsvp_physics_tail(verbose, formula, deepest, z.speed[z.n - 1], latitude, ntail, &complete->depth[z.n],
                                  &complete->speed[z.n], message, error);
      complete->n = z.n + ntail;
    }
    if (status == MB_SUCCESS) {
      for (int i = z.n; i < complete->n; i++) {
        if (!(complete->speed[i] > MBSVP_MIN_SPEED && complete->speed[i] < MBSVP_MAX_SPEED)) {
          mbsvp_set_message(message, "Sound speed %.1f m/s at %.0f m is outside %.0f-%.0f m/s", complete->speed[i], complete->depth[i],
                            MBSVP_MIN_SPEED, MBSVP_MAX_SPEED);
          *error = MB_ERROR_BAD_DATA;
          status = MB_FAILURE;
          break;
        }
      }
    }
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
