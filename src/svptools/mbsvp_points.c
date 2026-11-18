/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_points.c  4 November 2026
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
 * Sites and route waypoints read from the text files that MB-System writes:
 * site files (.ste) and route files (.rte). See the sections on these files
 * in the mbgrdviz man page.
 *
 * In a route file a waypoint is a numeric line with a comment starting
 * with "## WAYPOINT", such as "## WAYPOINT" or "## WAYPOINT STARTLINE". The
 * other numeric lines only give the topography between waypoints, and only
 * the position is read from a waypoint line.
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

#define MBSVP_LINE_SIZE (2 * MB_PATH_MAXLINE)

/*--------------------------------------------------------------------*/
void mbsvp_points_init(struct mbsvp_points *points) {
  points->n = 0;
  points->alloc = 0;
  points->items = NULL;
}
/*--------------------------------------------------------------------*/
int mbsvp_points_free(int verbose, struct mbsvp_points *points, int *error) {
  if (points->items != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&points->items, error);
  points->n = 0;
  points->alloc = 0;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static struct mbsvp_point *mbsvp_points_add(int verbose, struct mbsvp_points *points, int *error) {
  if (points->n >= points->alloc) {
    const int alloc = points->alloc + 256;
    if (mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(struct mbsvp_point), (void **)&points->items, error) !=
        MB_SUCCESS) {
      points->n = 0;
      points->alloc = 0;
      return NULL;
    }
    points->alloc = alloc;
  }
  struct mbsvp_point *point = &points->items[points->n++];
  memset(point, 0, sizeof(struct mbsvp_point));
  return point;
}
/*--------------------------------------------------------------------*/
/* Read one line. A longer line keeps its start and drops the rest. Trailing
   white space and line ends are removed. */
static bool mbsvp_read_line(FILE *fp, char *line, size_t size) {
  if (fgets(line, (int)size, fp) == NULL)
    return false;
  size_t len = strlen(line);
  if (len > 0 && line[len - 1] != '\n' && !feof(fp)) {
    int c;
    while ((c = fgetc(fp)) != EOF && c != '\n') {
    }
  }
  while (len > 0 && isspace((unsigned char)line[len - 1]))
    line[--len] = '\0';
  return true;
}
/*--------------------------------------------------------------------*/
/* Skip a UTF-8 byte order mark at the start of a file. */
static void mbsvp_skip_bom(FILE *fp) {
  unsigned char mark[3];
  const size_t nread = fread(mark, 1, 3, fp);
  if (nread != 3 || mark[0] != 0xEF || mark[1] != 0xBB || mark[2] != 0xBF)
    rewind(fp);
}
/*--------------------------------------------------------------------*/
/* Decimal degrees or signed degrees:minutes. Longitudes from -180 to 360 are
   accepted and returned from -180 to 180. */
int mbsvp_coordinate(const char *text, bool latitude, double *value, char *message) {
  while (isspace((unsigned char)*text))
    text++;
  double result;
  char *end;
  const char *colon = strchr(text, ':');
  if (colon != NULL) {
    const double degrees = strtod(text, &end);
    if (end != colon) {
      mbsvp_set_message(message, "Expected decimal degrees or degrees:minutes");
      return MB_FAILURE;
    }
    const double minutes = strtod(colon + 1, &end);
    if (end == colon + 1) {
      mbsvp_set_message(message, "Expected decimal degrees or degrees:minutes");
      return MB_FAILURE;
    }
    if (!(minutes >= 0.0 && minutes < 60.0)) {
      mbsvp_set_message(message, "Minutes must be in [0,60)");
      return MB_FAILURE;
    }
    result = copysign(fabs(degrees) + minutes / 60.0, degrees);
  }
  else {
    result = strtod(text, &end);
    if (end == text) {
      mbsvp_set_message(message, "Expected decimal degrees or degrees:minutes");
      return MB_FAILURE;
    }
  }
  while (isspace((unsigned char)*end))
    end++;
  if (*end != '\0') {
    mbsvp_set_message(message, "Expected decimal degrees or degrees:minutes");
    return MB_FAILURE;
  }
  if (!isfinite(result)) {
    mbsvp_set_message(message, "Coordinate must be finite");
    return MB_FAILURE;
  }
  if (latitude) {
    if (result < -90.0 || result > 90.0) {
      mbsvp_set_message(message, "Latitude outside [-90,90]");
      return MB_FAILURE;
    }
  }
  else {
    if (result < -180.0 || result > 360.0) {
      mbsvp_set_message(message, "Longitude outside [-180,360]");
      return MB_FAILURE;
    }
    result = mbsvp_normalize_longitude(result);
  }
  *value = result;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* The header line decides, then the suffix. */
int mbsvp_points_detect(int verbose, const char *path, enum mbsvp_input_type *type, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
  }

  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    mbsvp_set_message(message, "Input does not exist: %s", path);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  mbsvp_skip_bom(fp);
  *type = MBSVP_INPUT_AUTO;
  char line[MBSVP_LINE_SIZE];
  while (*type == MBSVP_INPUT_AUTO && mbsvp_read_line(fp, line, sizeof(line))) {
    if (strncmp(line, "## Route File Version", 21) == 0)
      *type = MBSVP_INPUT_ROUTE;
    else if (strncmp(line, "## Site File Version", 20) == 0)
      *type = MBSVP_INPUT_SITES;
    else if (line[0] != '#' && line[0] != '\0')
      break;
  }
  fclose(fp);
  if (*type == MBSVP_INPUT_AUTO) {
    const char *suffix = mbsvp_suffix(path);
    if (strcmp(suffix, ".rte") == 0)
      *type = MBSVP_INPUT_ROUTE;
    else if (strcmp(suffix, ".ste") == 0)
      *type = MBSVP_INPUT_SITES;
  }
  if (*type == MBSVP_INPUT_AUTO) {
    mbsvp_set_message(message, "Cannot tell whether %s is a site or route file; use --sites or --route", path);
    *error = MB_ERROR_BAD_FORMAT;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Split one site line. With commas the name is all after the fifth comma, so
   that it can hold commas; without them it is all after the fifth field. */
static void mbsvp_site_fields(char *line, bool commas, char **lon, char **lat, char **name) {
  *lon = NULL;
  *lat = NULL;
  *name = NULL;
  char *c = line;
  for (int field = 0; field < 5; field++) {
    while (isspace((unsigned char)*c))
      c++;
    char *start = c;
    if (commas) {
      while (*c != '\0' && *c != ',')
        c++;
    }
    else {
      while (*c != '\0' && !isspace((unsigned char)*c))
        c++;
    }
    const bool more = *c != '\0';
    if (more)
      *c++ = '\0';
    if (field == 0)
      *lon = start;
    else if (field == 1)
      *lat = start;
    if (!more)
      return;
  }
  while (isspace((unsigned char)*c))
    c++;
  *name = c;
}
/*--------------------------------------------------------------------*/
static int mbsvp_read_sites(int verbose, FILE *fp, const char *path, struct mbsvp_points *points, char *message, int *error) {
  char line[MBSVP_LINE_SIZE];
  int number = 0;
  int version = 0;
  while (mbsvp_read_line(fp, line, sizeof(line))) {
    number++;
    const char *text = line;
    while (isspace((unsigned char)*text))
      text++;
    if (strncmp(text, "## Site File Version", 20) == 0) {
      sscanf(text, "## Site File Version %d", &version);
      continue;
    }
    if (*text == '\0' || *text == '#')
      continue;

    const bool commas = version > 1 || (version == 0 && strchr(line, ',') != NULL);
    char *lon;
    char *lat;
    char *name;
    mbsvp_site_fields(line, commas, &lon, &lat, &name);
    double longitude;
    double latitude;
    char reason[MBSVP_MESSAGE_SIZE] = "Expected longitude and latitude";
    if (lon == NULL || lat == NULL || mbsvp_coordinate(lon, false, &longitude, reason) != MB_SUCCESS ||
        mbsvp_coordinate(lat, true, &latitude, reason) != MB_SUCCESS) {
      mbsvp_set_message(message, "%s:%d: %s", path, number, reason);
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    const int index = points->n;
    struct mbsvp_point *point = mbsvp_points_add(verbose, points, error);
    if (point == NULL) {
      mbsvp_set_message(message, "Out of memory");
      return MB_FAILURE;
    }
    point->longitude = longitude;
    point->latitude = latitude;
    point->line = number;
    if (name != NULL && name[0] != '\0')
      snprintf(point->name, sizeof(point->name), "%s", name);
    else
      snprintf(point->name, sizeof(point->name), "Site %d", index);
  }
  if (points->n == 0) {
    mbsvp_set_message(message, "No sites in %s", path);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* The comment of a waypoint line, or NULL. "## WAYPOINTS" is not a marker. */
static const char *mbsvp_waypoint_marker(const char *line) {
  const char *comment = strchr(line, '#');
  if (comment == NULL || strncmp(comment, "## WAYPOINT", 11) != 0)
    return NULL;
  if (comment[11] != '\0' && !isspace((unsigned char)comment[11]))
    return NULL;
  return comment;
}
/*--------------------------------------------------------------------*/
static int mbsvp_read_route(int verbose, FILE *fp, const char *path, struct mbsvp_points *points, struct mbsvp_strings *notes,
                            char *message, int *error) {
  char line[MBSVP_LINE_SIZE];
  char note[MBSVP_LINE_SIZE + 256];
  char route[256] = "";
  char pending_name[256] = "";
  int number = 0;
  int version = 0;
  int route_number = 0;
  int waypoint = 0;
  int expected = -1;
  int listed = 0;
  bool in_route = false;
  bool started = false;

  while (mbsvp_read_line(fp, line, sizeof(line))) {
    number++;
    const char *text = line;
    while (isspace((unsigned char)*text))
      text++;
    if (*text == '\0')
      continue;
    if (*text == '#') {
      if (strncmp(text, "## Route File Version", 21) == 0)
        sscanf(text, "## Route File Version %d", &version);
      else if (strncmp(text, "## ROUTENAME", 12) == 0)
        snprintf(pending_name, sizeof(pending_name), "%s", strlen(text) > 13 ? &text[13] : "");
      continue;
    }
    if (*text == '>') {
      if (in_route && expected >= 0 && listed != expected) {
        snprintf(note, sizeof(note), "Route %s at %s:%d lists %d points but holds %d", route, path, number, expected, listed);
        if (mbsvp_strings_add(verbose, notes, note, error) != MB_SUCCESS)
          return MB_FAILURE;
      }
      in_route = false;
      expected = -1;
      listed = 0;
      if (strncmp(text, "> ## STARTROUTE", 15) == 0) {
        int np = -1;
        int color;
        int size;
        int editmode;
        char name[256] = "";
        const int nget = sscanf(text, "> ## STARTROUTE %d,%d,%d,%d,%255[^\n]", &np, &color, &size, &editmode, name);
        route_number++;
        started = true;
        in_route = true;
        expected = nget >= 1 ? np : -1;
        waypoint = 0;
        if (nget >= 5 && name[0] != '\0')
          snprintf(route, sizeof(route), "%s", name);
        else if (pending_name[0] != '\0')
          snprintf(route, sizeof(route), "%s", pending_name);
        else
          snprintf(route, sizeof(route), "Route %d", route_number);
        pending_name[0] = '\0';
      }
      continue;
    }

    /* a numeric line; older files separate the values with white space */
    listed++;
    double lon_value = 0.0;
    double lat_value = 0.0;
    double topo = 0.0;
    int type = 0;
    int nget;
    if (version == 1) {
      nget = sscanf(text, "%lf %lf %lf %d", &lon_value, &lat_value, &topo, &type);
    }
    else {
      nget = sscanf(text, "%lf,%lf,%lf,%d", &lon_value, &lat_value, &topo, &type);
      if (nget < 2 && version == 0)
        nget = sscanf(text, "%lf %lf %lf %d", &lon_value, &lat_value, &topo, &type);
    }
    const bool marked = mbsvp_waypoint_marker(text) != NULL;
    if (!marked) {
      if (nget >= 4 && type > 0) {
        snprintf(note, sizeof(note), "%s:%d has waypoint type %d but no ## WAYPOINT comment; not used", path, number, type);
        if (mbsvp_strings_add(verbose, notes, note, error) != MB_SUCCESS)
          return MB_FAILURE;
      }
      continue;
    }
    if (nget < 2) {
      mbsvp_set_message(message, "%s:%d: Expected longitude and latitude", path, number);
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    if (nget >= 4 && type == 0) {
      snprintf(note, sizeof(note), "%s:%d has a ## WAYPOINT comment but waypoint type 0; used", path, number);
      if (mbsvp_strings_add(verbose, notes, note, error) != MB_SUCCESS)
        return MB_FAILURE;
    }
    char reason[MBSVP_MESSAGE_SIZE] = "";
    char value[64];
    snprintf(value, sizeof(value), "%.17g", lon_value);
    double longitude;
    double latitude;
    int status = mbsvp_coordinate(value, false, &longitude, reason);
    if (status == MB_SUCCESS) {
      snprintf(value, sizeof(value), "%.17g", lat_value);
      status = mbsvp_coordinate(value, true, &latitude, reason);
    }
    if (status != MB_SUCCESS) {
      mbsvp_set_message(message, "%s:%d: %s", path, number, reason);
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    if (!started) {
      /* waypoints before any route start line */
      route_number++;
      started = true;
      waypoint = 0;
      if (pending_name[0] != '\0')
        snprintf(route, sizeof(route), "%s", pending_name);
      else
        snprintf(route, sizeof(route), "Route %d", route_number);
    }
    struct mbsvp_point *point = mbsvp_points_add(verbose, points, error);
    if (point == NULL) {
      mbsvp_set_message(message, "Out of memory");
      return MB_FAILURE;
    }
    point->longitude = longitude;
    point->latitude = latitude;
    point->line = number;
    point->is_waypoint = true;
    snprintf(point->route, sizeof(point->route), "%s", route);
    point->route_number = route_number;
    point->waypoint = waypoint;
    point->waypoint_type = nget >= 4 ? type : 0;
    snprintf(point->name, sizeof(point->name), "%s WP%03d", route, waypoint);
    waypoint++;
  }
  if (points->n == 0) {
    mbsvp_set_message(message, "No lines marked ## WAYPOINT in %s", path);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mbsvp_points_read(int verbose, const char *path, enum mbsvp_input_type type, struct mbsvp_points *points,
                      struct mbsvp_strings *notes, char *message, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       type:       %d\n", type);
  }

  FILE *fp = fopen(path, "r");
  if (fp == NULL) {
    mbsvp_set_message(message, "Input does not exist: %s", path);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  mbsvp_skip_bom(fp);
  const int status = type == MBSVP_INPUT_ROUTE ? mbsvp_read_route(verbose, fp, path, points, notes, message, error)
                                               : mbsvp_read_sites(verbose, fp, path, points, message, error);
  fclose(fp);

  if (verbose >= 2) {
    fprintf(stderr, "dbg2  Function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2       points:     %d\n", points->n);
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }
  return status;
}
/*--------------------------------------------------------------------*/
/* File names from the point names: safe characters only, at most 100 of them,
   and unique without regard to case. */
void mbsvp_points_make_bases(struct mbsvp_points *points) {
  for (int i = 0; i < points->n; i++) {
    struct mbsvp_point *point = &points->items[i];
    char clean[sizeof(point->name)];
    size_t k = 0;
    for (const char *c = point->name; *c != '\0'; c++) {
      if (isalnum((unsigned char)*c) && (unsigned char)*c < 128)
        clean[k++] = *c;
      else if (*c == '_' || *c == '.' || *c == '-')
        clean[k++] = *c;
      else if (k == 0 || clean[k - 1] != '\1')
        clean[k++] = '\1';
    }
    clean[k] = '\0';
    for (size_t j = 0; j < k; j++)
      if (clean[j] == '\1')
        clean[j] = '_';
    const char *start = clean;
    while (*start == '.' || *start == '_' || *start == '-')
      start++;
    size_t len = strlen(start);
    while (len > 0 && (start[len - 1] == '.' || start[len - 1] == '_' || start[len - 1] == '-'))
      len--;
    if (len > 100)
      len = 100;

    char base[sizeof(point->base)];
    if (len == 0) {
      snprintf(base, sizeof(base), "Site_%d", i);
    }
    else {
      memcpy(base, start, len);
      base[len] = '\0';
    }
    char candidate[sizeof(point->base)];
    snprintf(candidate, sizeof(candidate), "%s", base);
    int n = 2;
    bool used = true;
    while (used) {
      used = false;
      for (int j = 0; j < i && !used; j++) {
        const char *a = points->items[j].base;
        const char *b = candidate;
        while (*a != '\0' && tolower((unsigned char)*a) == tolower((unsigned char)*b)) {
          a++;
          b++;
        }
        used = *a == '\0' && *b == '\0';
      }
      if (used) {
        snprintf(candidate, sizeof(candidate), "%.110s_%d", base, n);
        n++;
      }
    }
    snprintf(point->base, sizeof(point->base), "%s", candidate);
  }
}
/*--------------------------------------------------------------------*/
