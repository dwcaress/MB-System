/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_location.c  3 November 2026
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
 * Versioned location and provenance comments in profile files. A comment
 * holds one line of JSON in decimal degrees. It stays below 900 bytes so
 * that readers with fixed line buffers never meet the rest of a split
 * line as numeric data.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

/*--------------------------------------------------------------------*/
void mbsvp_location_init(struct mbsvp_location *location) {
  memset(location, 0, sizeof(struct mbsvp_location));
  strcpy(location->method, "unknown");
  strcpy(location->source, "unknown");
}
/*--------------------------------------------------------------------*/
static double mbsvp_round8(double value) {
  char text[64];
  snprintf(text, sizeof(text), "%.8f", value);
  return strtod(text, NULL);
}
/*--------------------------------------------------------------------*/
/* -180 to 180, rounded to 8 decimals */
double mbsvp_location_longitude(double longitude) {
  double wrapped = fmod(longitude + 180.0, 360.0);
  if (wrapped < 0.0)
    wrapped += 360.0;
  return mbsvp_round8(wrapped - 180.0);
}
/*--------------------------------------------------------------------*/
static bool mbsvp_append(char *line, size_t size, size_t *len, const char *format, const char *a, const char *b) {
  const int added = snprintf(&line[*len], size - *len, format, a, b);
  if (added < 0 || (size_t)added >= size - *len)
    return false;
  *len += added;
  return true;
}
/*--------------------------------------------------------------------*/
int mbsvp_location_comment(int verbose, const struct mbsvp_location *location, const char *marker, char *line, size_t size,
                           char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       marker:     %s\n", marker);
  }

  size_t len = 0;
  bool fits = true;
  char v1[64];
  char v2[64];
  char v3[64];
  char v4[64];
  char escaped[512];

  if (location->has_position) {
    if (!isfinite(location->longitude)) {
      mbsvp_set_message(message, "Nonfinite longitude");
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    if (!isfinite(location->latitude) || fabs(location->latitude) > 90.0) {
      mbsvp_set_message(message, "Invalid latitude");
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    mbsvp_format_double(mbsvp_location_longitude(location->longitude), v1, sizeof(v1));
    mbsvp_format_double(mbsvp_round8(location->latitude), v2, sizeof(v2));
  }
  else {
    strcpy(v1, "null");
    strcpy(v2, "null");
  }
  fits = mbsvp_append(line, size, &len, "%s{\"version\":1,", marker, "");
  if (fits)
    fits = mbsvp_append(line, size, &len, "\"longitude\":%s,\"latitude\":%s,\"units\":\"degrees\",", v1, v2);
  if (fits)
    fits = mbsvp_append(line, size, &len, "\"method\":\"%s\",\"source\":\"%s\"", location->method, location->source);

  if (fits && location->has_area) {
    mbsvp_format_double(location->west, v1, sizeof(v1));
    mbsvp_format_double(location->east, v2, sizeof(v2));
    mbsvp_format_double(location->south, v3, sizeof(v3));
    mbsvp_format_double(location->north, v4, sizeof(v4));
    fits = mbsvp_append(line, size, &len, ",\"bounds\":{\"west\":%s,\"east\":%s,", v1, v2);
    if (fits)
      fits = mbsvp_append(line, size, &len, "\"south\":%s,\"north\":%s},", v3, v4);
    snprintf(v1, sizeof(v1), "%d", location->grid_lat);
    snprintf(v2, sizeof(v2), "%d", location->grid_lon);
    if (fits)
      fits = mbsvp_append(line, size, &len, "\"grid_shape\":[%s,%s],", v1, v2);
    if (fits)
      fits = mbsvp_append(line, size, &len, "\"representative\":\"%s\"%s", location->representative, "");
  }
  if (fits && location->dataset[0] != '\0') {
    mbsvp_json_escape(location->dataset, escaped, sizeof(escaped));
    fits = mbsvp_append(line, size, &len, ",\"dataset\":\"%s\"%s", escaped, "");
  }
  if (fits && location->formula[0] != '\0')
    fits = mbsvp_append(line, size, &len, ",\"formula\":\"%s\"%s", location->formula, "");
  if (fits && location->kind[0] != '\0')
    fits = mbsvp_append(line, size, &len, ",\"kind\":\"%s\"%s", location->kind, "");
  if (fits && location->has_appended) {
    mbsvp_format_double(location->appended_min_depth, v1, sizeof(v1));
    mbsvp_format_double(location->appended_max_depth, v2, sizeof(v2));
    fits = mbsvp_append(line, size, &len, ",\"appended_min_depth\":%s,\"appended_max_depth\":%s", v1, v2);
  }
  if (fits)
    fits = mbsvp_append(line, size, &len, "}%s%s", "", "");

  if (!fits || len >= MBSVP_COMMENT_LIMIT) {
    mbsvp_set_message(message, "SVP location comment exceeds conservative %d-byte limit", MBSVP_COMMENT_LIMIT);
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  if (!mbsvp_append(line, size, &len, "\n%s%s", "", "")) {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Keep the comments and data of a measured profile and add a comment that
   its location is not known: no WOA location is inferred for it. */
int mbsvp_location_annotate_internal(int verbose, const char *path, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
  }

  struct mbsvp_location location;
  mbsvp_location_init(&location);
  strcpy(location.source, "internal");
  char comment[MBSVP_COMMENT_LIMIT + 2];
  if (mbsvp_location_comment(verbose, &location, MBSVP_MARKER_LOCATION, comment, sizeof(comment), message, error) !=
      MB_SUCCESS)
    return MB_FAILURE;

  /* read the whole file */
  FILE *fp = fopen(path, "rb");
  if (fp == NULL) {
    mbsvp_set_message(message, "Cannot read %s", path);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  fseek(fp, 0, SEEK_END);
  const long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char *text = NULL;
  int status = MB_SUCCESS;
  if (size < 0 || mb_mallocd(verbose, __FILE__, __LINE__, size + 1, (void **)&text, error) != MB_SUCCESS) {
    status = MB_FAILURE;
  }
  else if (fread(text, 1, size, fp) != (size_t)size) {
    mbsvp_set_message(message, "Cannot read %s", path);
    *error = MB_ERROR_EOF;
    status = MB_FAILURE;
  }
  fclose(fp);

  if (status == MB_SUCCESS) {
    text[size] = '\0';
    fp = fopen(path, "wb");
    if (fp == NULL) {
      mbsvp_set_message(message, "Cannot write %s", path);
      *error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
    }
  }
  if (status == MB_SUCCESS) {
    const char *start = text;
    const char *first_end = start;
    if (size == 0 || text[0] != '#') {
      fputs("# Internal SVP extracted by mbsvpbest\n", fp);
    }
    else {
      first_end = strchr(start, '\n');
      first_end = first_end != NULL ? first_end + 1 : start + size;
      fwrite(start, 1, first_end - start, fp);
    }
    fputs(comment, fp);
    const char *c = first_end;
    const size_t markerlen = strlen(MBSVP_MARKER_LOCATION);
    while (*c != '\0') {
      const char *end = strchr(c, '\n');
      end = end != NULL ? end + 1 : c + strlen(c);
      if (strncmp(c, MBSVP_MARKER_LOCATION, markerlen) != 0)
        fwrite(c, 1, end - c, fp);
      c = end;
    }
    if (fclose(fp) != 0) {
      *error = MB_ERROR_WRITE_FAIL;
      status = MB_FAILURE;
    }
  }
  if (text != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&text, error);
  return status;
}
/*--------------------------------------------------------------------*/
