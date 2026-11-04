/*--------------------------------------------------------------------
 *    The MB-system:  mbsvpextract.c  4 November 2026
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
 * mbsvpextract extracts annual sound velocity profiles from a World Ocean
 * Atlas sound speed grid at the sites of a site file or at the waypoints of
 * a route file. The user names the grid by edition, period and resolution, so
 * that nothing depends on which datasets happen to be installed. Each position
 * takes the profile of the nearest grid cell that can be completed to 11000 m,
 * and each gets its own profile file, for each formula.
 *
 * A profile is a climatological candidate, not a forecast or a measurement
 * of the water column at that position.
 *
 * Author:  C. dos Santos Ferreira
 * Date:    November 4, 2026
 */

#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

static const char program_name[] = "mbsvpextract";
static const char help_message[] =
    "mbsvpextract extracts annual sound velocity profiles from a World Ocean\n"
    "Atlas sound speed grid at the sites of a site file (.ste) or at the\n"
    "waypoints of a route file (.rte). The grid is named by edition, period\n"
    "and resolution. Each position takes the nearest grid cell whose profile\n"
    "can be completed to 11000 m. It requires one of the datasets posted at\n"
    MBSVP_DATASETS_URL;
static const char usage_message[] =
    "mbsvpextract\n"
    "\t--input=file {-Ifile} (or --sites=file or --route=file)\n"
    "\t--svpfolder=directory {-Sdirectory}\n"
    "\t--outputfolder=directory {-Odirectory}\n"
    "\t--woa-version=18|23\n"
    "\t--period=period {-Pperiod} (such as decav, A5B7 or B5C2)\n"
    "\t--resolution=01|04 {-R01|04}\n"
    "\t--formula=chen|leroy|both\n"
    "\t--strict-nearest\n"
    "\t--overwrite\n"
    "\t--help {-H}\n"
    "\t--verbose {-V}";

struct row {
  char status[16];
  char outputs[2][MB_PATH_MAXLINE];
  int noutputs;
  int formula[2];
  char metadata[MBSVP_COMMENT_LIMIT + 64];
  bool has_result;
  bool substituted;
  int rank;
  char error[MBSVP_MESSAGE_SIZE];
};

struct options {
  int verbose;
  mb_path input;
  enum mbsvp_input_type input_type;
  mb_path svpfolder;
  mb_path outputfolder;
  char edition[8];
  char period[64];
  char resolution[8];
  char formula[8];
  bool strict_nearest;
  bool overwrite;
};

static const char *formula_names[2] = {"chen", "leroy"};

/*--------------------------------------------------------------------*/
static void handle_signal(int signal_number) {
  (void)signal_number;
  mbsvp_interrupted = 1;
}
/*--------------------------------------------------------------------*/
/* The profile of the nearest completable cell. All wanted formulas must
   complete in the same cell. */
static int extract_point(int verbose, const struct mbsvp_atlas *atlas, const struct mbsvp_point *point, const bool want[2],
                         bool strict, double *speed, struct mbsvp_profile profiles[2], int *rank, double *cell_lon,
                         double *cell_lat, char *message, int *error) {
  int cell_lat_index[MBSVP_NEAREST_CELLS];
  int cell_lon_index[MBSVP_NEAREST_CELLS];
  int ncells = 0;
  if (mbsvp_woa_order_cells(verbose, atlas->lat, atlas->nlat, atlas->lon, atlas->nlon, point->latitude, point->longitude,
                            strict ? 1 : MBSVP_NEAREST_CELLS, cell_lat_index, cell_lon_index, &ncells, message,
                            error) != MB_SUCCESS)
    return MB_FAILURE;

  struct mbsvp_strings reasons;
  mbsvp_strings_init(&reasons);
  int status = MB_FAILURE;
  for (int c = 0; c < ncells && status != MB_SUCCESS; c++) {
    bool complete = true;
    for (int v = 0; v < 2 && complete; v++) {
      if (!want[v])
        continue;
      char reason[MBSVP_MESSAGE_SIZE] = "";
      int local_error = MB_ERROR_NO_ERROR;
      if (mbsvp_atlas_read_cell(verbose, atlas, v, cell_lat_index[c], cell_lon_index[c], speed, reason, &local_error) !=
          MB_SUCCESS) {
        mbsvp_set_message(message, "%s", reason);
        *error = local_error;
        int error_free = MB_ERROR_NO_ERROR;
        mbsvp_strings_free(verbose, &reasons, &error_free);
        return MB_FAILURE;
      }
      const struct mbsvp_profile raw = {atlas->ndepth, atlas->depth, speed};
      if (mbsvp_profile_complete(verbose, &raw, MBSVP_TARGET_DEPTH, &profiles[v], reason, &local_error) != MB_SUCCESS) {
        complete = false;
        if (!mbsvp_strings_contains(&reasons, reason))
          mbsvp_strings_add(verbose, &reasons, reason, error);
      }
    }
    if (complete) {
      *rank = c;
      *cell_lon = mbsvp_normalize_longitude(atlas->lon[cell_lon_index[c]]);
      *cell_lat = atlas->lat[cell_lat_index[c]];
      status = MB_SUCCESS;
    }
  }
  if (status != MB_SUCCESS) {
    char joined[MBSVP_MESSAGE_SIZE] = "";
    size_t len = 0;
    for (int i = 0; i < reasons.n && len < sizeof(joined); i++)
      len += snprintf(&joined[len], sizeof(joined) - len, "%s%s", i > 0 ? "; " : "", reasons.items[i]);
    mbsvp_set_message(message, "No completable near-surface profile among %d nearest cells: %s", ncells, joined);
    *error = MB_ERROR_BAD_DATA;
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_strings_free(verbose, &reasons, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
/* The JSON metadata of a profile, in one line. */
static bool make_metadata(const struct options *options, const struct mbsvp_point *point, double cell_lon, double cell_lat,
                          char *metadata, size_t size) {
  char lon[64];
  char lat[64];
  char woa_lon[64];
  char woa_lat[64];
  mbsvp_format_double(point->longitude, lon, sizeof(lon));
  mbsvp_format_double(point->latitude, lat, sizeof(lat));
  mbsvp_format_double(cell_lon, woa_lon, sizeof(woa_lon));
  mbsvp_format_double(cell_lat, woa_lat, sizeof(woa_lat));
  int len = snprintf(metadata, size,
                     "{\"version\":1,\"Longitude_site\":%s,\"Latitude_site\":%s,\"WOAdataset\":\"WOA%s\","
                     "\"WOA_resolution\":\"%s\",\"WOA_period\":\"%s\",\"Longitude_WOA\":%s,\"Latitude_WOA\":%s",
                     lon, lat, options->edition, options->resolution, options->period, woa_lon, woa_lat);
  if (len < 0 || (size_t)len >= size)
    return false;
  if (point->is_waypoint) {
    char route[sizeof(point->route) * 2];
    mbsvp_json_escape(point->route, route, sizeof(route));
    const int added = snprintf(&metadata[len], size - len, ",\"Route\":\"%.160s\",\"Waypoint\":%d", route, point->waypoint);
    if (added < 0 || (size_t)added >= size - len)
      return false;
    len += added;
  }
  const int added = snprintf(&metadata[len], size - len, "}");
  return added > 0 && (size_t)added < size - len;
}
/*--------------------------------------------------------------------*/
static int write_profile(int verbose, const struct options *options, const struct mbsvp_point *point, int formula,
                         const char *metadata, const struct mbsvp_profile *profile, const char *path, char *message, int *error) {
  /* the first comment holds no control characters and at most 160 of the name */
  char label[161];
  size_t k = 0;
  for (const unsigned char *c = (const unsigned char *)point->name; *c != '\0' && k < sizeof(label) - 1; c++)
    label[k++] = (*c < 0x20 || *c == 0x7f) ? ' ' : (char)*c;
  label[k] = '\0';

  char temporary[MB_PATH_MAXLINE + 64];
  snprintf(temporary, sizeof(temporary), "%s.%d.tmp", path, mbsvp_process_id());
  FILE *fp = fopen(temporary, "w");
  if (fp == NULL) {
    mbsvp_set_message(message, "Cannot write %s", temporary);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  fprintf(fp, "# mbsvpextract; formula=%s; period=%s; annual average; endpoint=11000m; site=%s\n", formula_names[formula],
          options->period, label);
  fprintf(fp, "%s%s\n", MBSVP_MARKER_EXTRACT, metadata);
  for (int i = 0; i < profile->n; i++)
    fprintf(fp, "%.6f %.9f\n", profile->depth[i], profile->speed[i]);
  const bool bad = ferror(fp) != 0;
  if (fclose(fp) != 0 || bad) {
    remove(temporary);
    mbsvp_set_message(message, "Cannot write %s", temporary);
    *error = MB_ERROR_WRITE_FAIL;
    return MB_FAILURE;
  }
  if (mbsvp_commit_file(verbose, temporary, path, options->overwrite, error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot create %s", path);
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Written after every position, by replacing the file, so that a stopped run
   leaves a readable summary. */
static void write_summary(const char *summary, const char *created, const struct options *options, const char *dataset,
                          const struct mbsvp_points *points, const struct row *rows, int nrows, bool interrupted,
                          const char *fatal) {
  char temporary[MB_PATH_MAXLINE + 64];
  char text[2 * MBSVP_MESSAGE_SIZE];
  char number[64];
  snprintf(temporary, sizeof(temporary), "%s.%d.tmp", summary, mbsvp_process_id());
  FILE *fp = fopen(temporary, "w");
  if (fp == NULL)
    return;
  fprintf(fp, "{\n  \"tool\": \"%s\",\n  \"created_utc\": \"%s\",\n", program_name, created);
  mbsvp_json_escape(options->input, text, sizeof(text));
  fprintf(fp, "  \"input_file\": \"%s\",\n  \"input_type\": \"%s\",\n", text,
          options->input_type == MBSVP_INPUT_ROUTE ? "route" : "sites");
  mbsvp_json_escape(dataset, text, sizeof(text));
  fprintf(fp, "  \"dataset\": \"%s\",\n  \"edition\": \"WOA%s\",\n  \"period\": \"%s\",\n", text, options->edition, options->period);
  fprintf(fp, "  \"resolution\": \"%s\",\n  \"formula\": \"%s\",\n  \"strict_nearest\": %s,\n", options->resolution, options->formula,
          options->strict_nearest ? "true" : "false");
  fprintf(fp, "  \"endpoint_metres\": 11000,\n  \"sites\": [");
  for (int i = 0; i < nrows; i++) {
    const struct mbsvp_point *point = &points->items[i];
    const struct row *row = &rows[i];
    mbsvp_json_escape(point->name, text, sizeof(text));
    fprintf(fp, "%s\n    {\n      \"name\": \"%s\",\n", i > 0 ? "," : "", text);
    mbsvp_format_double(point->longitude, number, sizeof(number));
    fprintf(fp, "      \"longitude\": %s,\n", number);
    mbsvp_format_double(point->latitude, number, sizeof(number));
    fprintf(fp, "      \"latitude\": %s,\n      \"line\": %d,\n", number, point->line);
    if (point->is_waypoint) {
      mbsvp_json_escape(point->route, text, sizeof(text));
      fprintf(fp, "      \"route\": \"%s\",\n      \"waypoint\": %d,\n      \"waypoint_type\": %d,\n", text, point->waypoint,
              point->waypoint_type);
    }
    fprintf(fp, "      \"status\": \"%s\",\n      \"outputs\": [", row->status);
    for (int k = 0; k < row->noutputs; k++) {
      mbsvp_json_escape(row->outputs[k], text, sizeof(text));
      fprintf(fp, "%s{\"file\": \"%s\", \"formula\": \"%s\"}", k > 0 ? ", " : "", text, formula_names[row->formula[k]]);
    }
    fprintf(fp, "]");
    if (row->has_result)
      fprintf(fp, ",\n      \"metadata\": %s,\n      \"substituted\": %s,\n      \"candidate_rank\": %d", row->metadata,
              row->substituted ? "true" : "false", row->rank + 1);
    if (row->error[0] != '\0') {
      mbsvp_json_escape(row->error, text, sizeof(text));
      fprintf(fp, ",\n      \"error\": \"%s\"", text);
    }
    fprintf(fp, "\n    }");
  }
  fprintf(fp, "%s],\n", nrows > 0 ? "\n  " : "");
  if (fatal != NULL && fatal[0] != '\0') {
    mbsvp_json_escape(fatal, text, sizeof(text));
    fprintf(fp, "  \"error\": \"%s\",\n", text);
  }
  fprintf(fp, "  \"interrupted\": %s\n}\n", interrupted ? "true" : "false");
  if (fclose(fp) != 0) {
    remove(temporary);
    return;
  }
#ifdef _WIN32
  remove(summary);
#endif
  rename(temporary, summary);
}
/*--------------------------------------------------------------------*/
/* Process one point. A failure is recorded and the run goes on. */
static void process_point(const struct options *options, const struct mbsvp_atlas *atlas, const struct mbsvp_point *point,
                          const char *folder, const bool want[2], double *speed, struct row *row) {
  const int verbose = options->verbose;
  int error = MB_ERROR_NO_ERROR;
  const int nwanted = (want[0] ? 1 : 0) + (want[1] ? 1 : 0);
  mb_path paths[2];
  size_t len = 0;
  char existing[MBSVP_MESSAGE_SIZE] = "";
  for (int v = 0; v < 2; v++) {
    if (!want[v])
      continue;
    const int nchar = snprintf(paths[v], sizeof(paths[v]), "%s/%s%s%s.svp", folder, point->base, nwanted > 1 ? "_" : "",
                               nwanted > 1 ? formula_names[v] : "");
    if (nchar < 0 || (size_t)nchar >= sizeof(paths[v])) {
      strcpy(row->status, "failed");
      mbsvp_set_message(row->error, "Output path is too long");
      return;
    }
    if (!options->overwrite && mbsvp_is_file(paths[v]) && len < sizeof(existing))
      len += snprintf(&existing[len], sizeof(existing) - len, "%s%s", len > 0 ? ", " : "", paths[v]);
  }
  if (len > 0) {
    strcpy(row->status, "failed");
    mbsvp_set_message(row->error, "Output exists; use --overwrite to replace: %s", existing);
    fprintf(stderr, "%s: %s\n", point->name, row->error);
    return;
  }

  struct mbsvp_profile profiles[2];
  mbsvp_profile_init(&profiles[0]);
  mbsvp_profile_init(&profiles[1]);
  double cell_lon = 0.0;
  double cell_lat = 0.0;
  int rank = 0;
  int status = extract_point(verbose, atlas, point, want, options->strict_nearest, speed, profiles, &rank, &cell_lon, &cell_lat,
                             row->error, &error);
  if (status == MB_SUCCESS && !make_metadata(options, point, cell_lon, cell_lat, row->metadata, sizeof(row->metadata))) {
    mbsvp_set_message(row->error, "Profile comment is too long");
    status = MB_FAILURE;
  }
  for (int v = 0; v < 2 && status == MB_SUCCESS; v++) {
    if (!want[v])
      continue;
    status = write_profile(verbose, options, point, v, row->metadata, &profiles[v], paths[v], row->error, &error);
    if (status == MB_SUCCESS) {
      snprintf(row->outputs[row->noutputs], sizeof(row->outputs[row->noutputs]), "%s", paths[v]);
      row->formula[row->noutputs] = v;
      row->noutputs++;
    }
  }
  if (status == MB_SUCCESS) {
    if (rank > 0)
      fprintf(stderr, "Warning: %s: nearest cell invalid; using valid candidate #%d\n", point->name, rank + 1);
    strcpy(row->status, "success");
    row->has_result = true;
    row->substituted = rank > 0;
    row->rank = rank;
    fprintf(stdout, "%s:", point->name);
    for (int k = 0; k < row->noutputs; k++)
      fprintf(stdout, "%s %s", k > 0 ? "," : "", row->outputs[k]);
    fprintf(stdout, "\n");
    fflush(stdout);
  }
  else {
    strcpy(row->status, "failed");
    fprintf(stderr, "%s: %s\n", point->name, row->error);
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_profile_free(verbose, &profiles[0], &error_free);
  mbsvp_profile_free(verbose, &profiles[1], &error_free);
}
/*--------------------------------------------------------------------*/
static int run(const struct options *options_in) {
  struct options options = *options_in;
  const int verbose = options.verbose;
  int error = MB_ERROR_NO_ERROR;
  char message[MBSVP_MESSAGE_SIZE] = "";

  if (!mbsvp_is_file(options.input)) {
    fprintf(stderr, "Input does not exist: %s\n", options.input);
    return 1;
  }
  if (options.input_type == MBSVP_INPUT_AUTO && mbsvp_points_detect(verbose, options.input, &options.input_type, message, &error) != MB_SUCCESS) {
    fprintf(stderr, "%s\n", message);
    return 1;
  }

  struct mbsvp_points points;
  struct mbsvp_strings notes;
  mbsvp_points_init(&points);
  mbsvp_strings_init(&notes);
  struct row *rows = NULL;
  double *speed = NULL;
  struct mbsvp_atlas atlas;
  mbsvp_atlas_init(&atlas);
  int exit_code = 1;
  bool have_summary = false;
  mb_path summary = "";
  char created[32] = "";
  mb_path dataset = "";
  int nrows = 0;

  if (mbsvp_points_read(verbose, options.input, options.input_type, &points, &notes, message, &error) != MB_SUCCESS)
    goto done;
  mbsvp_points_make_bases(&points);
  for (int i = 0; i < notes.n; i++)
    fprintf(stderr, "Note: %s\n", notes.items[i]);

  char name[256];
  const int name_len = snprintf(name, sizeof(name), "woa%s_%s_svp00_%s.nc", options.edition, options.period, options.resolution);
  if (name_len < 0 || (size_t)name_len >= sizeof(name) || !mbsvp_join(dataset, sizeof(dataset), options.svpfolder, name)) {
    mbsvp_set_message(message, "Dataset path is too long");
    goto done;
  }
  if (!mbsvp_is_file(dataset)) {
    mbsvp_set_message(message, "Required dataset not found: %s\nDatasets are posted at %s", dataset, MBSVP_DATASETS_URL);
    goto done;
  }

  mb_path folder;
  if (mbsvp_make_dirs(verbose, options.outputfolder, &error) != MB_SUCCESS ||
      mbsvp_absolute_path(verbose, options.outputfolder, folder, sizeof(folder), &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot make output directory: %s", options.outputfolder);
    goto done;
  }
  mb_path absolute;
  if (mbsvp_absolute_path(verbose, options.input, absolute, sizeof(absolute), &error) == MB_SUCCESS)
    strcpy(options.input, absolute);

  /* a new summary for each run: an earlier run keeps its record */
  const time_t now = time(NULL);
  const struct tm *utc = gmtime(&now);
  char stamp[32] = "00000000-000000";
  if (utc != NULL) {
    strftime(created, sizeof(created), "%Y-%m-%dT%H:%M:%S+00:00", utc);
    strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", utc);
  }
  for (int attempt = 0; attempt < 10000; attempt++) {
    int len;
    if (attempt == 0)
      len = snprintf(summary, sizeof(summary), "%s/mbsvpextract-%s.json", folder, stamp);
    else
      len = snprintf(summary, sizeof(summary), "%s/mbsvpextract-%s-%d.json", folder, stamp, attempt);
    if (len < 0 || (size_t)len >= sizeof(summary)) {
      mbsvp_set_message(message, "Output path is too long");
      goto done;
    }
    if (!mbsvp_is_file(summary))
      break;
  }

  if (mb_mallocd(verbose, __FILE__, __LINE__, points.n * sizeof(struct row), (void **)&rows, &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Out of memory");
    goto done;
  }
  have_summary = true;
  write_summary(summary, created, &options, dataset, &points, rows, 0, false, NULL);

  const bool want[2] = {strcmp(options.formula, "leroy") != 0, strcmp(options.formula, "chen") != 0};
  if (mbsvp_atlas_open(verbose, dataset, want, &atlas, message, &error) != MB_SUCCESS)
    goto done;
  if (mb_mallocd(verbose, __FILE__, __LINE__, atlas.ndepth * sizeof(double), (void **)&speed, &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Out of memory");
    goto done;
  }

  bool any_failed = false;
  for (int i = 0; i < points.n && !mbsvp_interrupted; i++) {
    struct row *row = &rows[i];
    memset(row, 0, sizeof(struct row));
    strcpy(row->status, "running");
    nrows = i + 1;
    write_summary(summary, created, &options, dataset, &points, rows, nrows, false, NULL);
    process_point(&options, &atlas, &points.items[i], folder, want, speed, row);
    if (strcmp(row->status, "success") != 0)
      any_failed = true;
    write_summary(summary, created, &options, dataset, &points, rows, nrows, false, NULL);
  }
  if (mbsvp_interrupted) {
    write_summary(summary, created, &options, dataset, &points, rows, nrows, true, NULL);
    exit_code = 130;
  }
  else {
    exit_code = any_failed ? 1 : 0;
  }
  fprintf(stdout, "Summary: %s\n", summary);

done:
  if (exit_code == 1 && message[0] != '\0') {
    fprintf(stderr, "%s\n", message);
    if (have_summary)
      write_summary(summary, created, &options, dataset, &points, rows, nrows, false, message);
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_atlas_close(verbose, &atlas, &error_free);
  mbsvp_points_free(verbose, &points, &error_free);
  mbsvp_strings_free(verbose, &notes, &error_free);
  if (rows != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&rows, &error_free);
  if (speed != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&speed, &error_free);
  return exit_code;
}
/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
  struct options options;
  memset(&options, 0, sizeof(options));
  strcpy(options.formula, "chen");

  static const struct option long_options[] = {{"formula", required_argument, NULL, 0},
                                               {"help", no_argument, NULL, 'H'},
                                               {"input", required_argument, NULL, 'I'},
                                               {"outputfolder", required_argument, NULL, 'O'},
                                               {"overwrite", no_argument, NULL, 0},
                                               {"period", required_argument, NULL, 'P'},
                                               {"resolution", required_argument, NULL, 'R'},
                                               {"route", required_argument, NULL, 0},
                                               {"sites", required_argument, NULL, 0},
                                               {"strict-nearest", no_argument, NULL, 0},
                                               {"svpfolder", required_argument, NULL, 'S'},
                                               {"verbose", no_argument, NULL, 'V'},
                                               {"woa-version", required_argument, NULL, 0},
                                               {NULL, 0, NULL, 0}};
  bool help = false;
  bool errflg = false;
  int c;
  int option_index;
  while ((c = getopt_long(argc, argv, "HhI:i:O:o:P:p:R:r:S:s:Vv", long_options, &option_index)) != -1) {
    switch (c) {
    case 0: {
      const char *option = long_options[option_index].name;
      if (strcmp("formula", option) == 0) {
        snprintf(options.formula, sizeof(options.formula), "%s", optarg);
      }
      else if (strcmp("overwrite", option) == 0) {
        options.overwrite = true;
      }
      else if (strcmp("route", option) == 0 || strcmp("sites", option) == 0) {
        snprintf(options.input, sizeof(options.input), "%s", optarg);
        options.input_type = strcmp("route", option) == 0 ? MBSVP_INPUT_ROUTE : MBSVP_INPUT_SITES;
      }
      else if (strcmp("strict-nearest", option) == 0) {
        options.strict_nearest = true;
      }
      else if (strcmp("woa-version", option) == 0) {
        snprintf(options.edition, sizeof(options.edition), "%s", optarg);
      }
      break;
    }
    case 'H':
    case 'h':
      help = true;
      break;
    case 'I':
    case 'i':
      snprintf(options.input, sizeof(options.input), "%s", optarg);
      options.input_type = MBSVP_INPUT_AUTO;
      break;
    case 'O':
    case 'o':
      snprintf(options.outputfolder, sizeof(options.outputfolder), "%s", optarg);
      break;
    case 'P':
    case 'p':
      snprintf(options.period, sizeof(options.period), "%s", optarg);
      break;
    case 'R':
    case 'r':
      snprintf(options.resolution, sizeof(options.resolution), "%s", optarg);
      break;
    case 'S':
    case 's':
      snprintf(options.svpfolder, sizeof(options.svpfolder), "%s", optarg);
      break;
    case 'V':
    case 'v':
      options.verbose++;
      break;
    default:
      errflg = true;
      break;
    }
  }
  const int verbose = options.verbose;

  if (help || verbose >= 1) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  if (optind < argc) {
    fprintf(stderr, "Unexpected argument: %s\n", argv[optind]);
    errflg = true;
  }
  const struct {
    const char *value;
    const char *text;
  } required[] = {{options.input, "--input {-I}, --sites or --route"},
                  {options.svpfolder, "--svpfolder {-S}"},
                  {options.outputfolder, "--outputfolder {-O}"},
                  {options.edition, "--woa-version (18 or 23)"},
                  {options.period, "--period {-P} (such as decav, A5B7 or B5C2)"},
                  {options.resolution, "--resolution {-R} (01 or 04)"}};
  for (size_t i = 0; i < sizeof(required) / sizeof(required[0]); i++) {
    if (required[i].value[0] == '\0') {
      fprintf(stderr, "%s is required\n", required[i].text);
      errflg = true;
    }
  }
  if (options.edition[0] != '\0' && strcmp(options.edition, "18") != 0 && strcmp(options.edition, "23") != 0) {
    fprintf(stderr, "--woa-version must be 18 or 23\n");
    errflg = true;
  }
  if (options.resolution[0] != '\0' && strcmp(options.resolution, "01") != 0 && strcmp(options.resolution, "04") != 0) {
    fprintf(stderr, "--resolution must be 01 or 04\n");
    errflg = true;
  }
  if (strcmp(options.formula, "chen") != 0 && strcmp(options.formula, "leroy") != 0 && strcmp(options.formula, "both") != 0) {
    fprintf(stderr, "--formula must be chen, leroy or both\n");
    errflg = true;
  }
  for (const char *p = options.period; *p != '\0'; p++) {
    if (!((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9'))) {
      fprintf(stderr, "--period holds letters and digits only\n");
      errflg = true;
      break;
    }
  }
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       input:          %s\n", options.input);
    fprintf(stderr, "dbg2       input_type:     %d\n", options.input_type);
    fprintf(stderr, "dbg2       svpfolder:      %s\n", options.svpfolder);
    fprintf(stderr, "dbg2       outputfolder:   %s\n", options.outputfolder);
    fprintf(stderr, "dbg2       edition:        %s\n", options.edition);
    fprintf(stderr, "dbg2       period:         %s\n", options.period);
    fprintf(stderr, "dbg2       resolution:     %s\n", options.resolution);
    fprintf(stderr, "dbg2       formula:        %s\n", options.formula);
    fprintf(stderr, "dbg2       strict_nearest: %d\n", options.strict_nearest);
    fprintf(stderr, "dbg2       overwrite:      %d\n", options.overwrite);
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  const int exit_code = run(&options);

  if (verbose >= 4) {
    int error_free = MB_ERROR_NO_ERROR;
    mb_memory_list(verbose, &error_free);
  }
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2       exit code:      %d\n", exit_code);
  }
  exit(exit_code);
}
/*--------------------------------------------------------------------*/
