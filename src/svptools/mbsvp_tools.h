/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_tools.h  3 November 2026
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
 * Shared code of the sound velocity profile tools in src/svptools.
 *
 * mbsvp_profile.c   cleaning and completion of climatological profiles
 * mbsvp_location.c  location and provenance comments in profile files
 * mbsvp_woa.c       World Ocean Atlas file discovery and grid sampling
 * mbsvp_score.c     residual score of swath bathymetry
 * mbsvp_util.c      strings, files, external commands and JSON numbers
 *
 * Functions follow the MBIO pattern: they take verbose first and error
 * last, return MB_SUCCESS or MB_FAILURE and set *error. Where a reason
 * is useful to the user, it is written to message, a buffer of
 * MBSVP_MESSAGE_SIZE characters.
 *
 * This work derives from the Python program mbbestsvp, later renamed
 * mbsvpbest, by Dominik Patrick Nommensen:
 *   https://github.com/DominikPatrickNommensen/MB-System-Best-WOA18-SVP
 */

#ifndef MBSVP_TOOLS_H_
#define MBSVP_TOOLS_H_

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MBSVP_MESSAGE_SIZE 4096
#define MBSVP_TARGET_DEPTH 11000.0
#define MBSVP_TREND_WINDOW 2000.0
#define MBSVP_NEAREST_CELLS 16
#define MBSVP_COMMENT_LIMIT 900
#define MBSVP_MARKER_LOCATION "# MBSVPBEST_LOCATION "
#define MBSVP_MARKER_DONOR "# MBSVPBEST_DONOR "

/*--------------------------------------------------------------------*/
/* mbsvp_util.c */

struct mbsvp_strings {
  int n;
  int alloc;
  char **items;
};

extern volatile sig_atomic_t mbsvp_interrupted;

void mbsvp_set_message(char *message, const char *format, ...)
#ifdef __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
    ;
void mbsvp_strings_init(struct mbsvp_strings *list);
int mbsvp_strings_add(int verbose, struct mbsvp_strings *list, const char *text, int *error);
bool mbsvp_strings_contains(const struct mbsvp_strings *list, const char *text);
int mbsvp_strings_free(int verbose, struct mbsvp_strings *list, int *error);

bool mbsvp_is_file(const char *path);
bool mbsvp_is_dir(const char *path);
const char *mbsvp_basename(const char *path);
const char *mbsvp_suffix(const char *path);
bool mbsvp_join(char *out, size_t size, const char *dir, const char *name);
int mbsvp_absolute_path(int verbose, const char *path, char *out, size_t size, int *error);
int mbsvp_make_dirs(int verbose, const char *path, int *error);
int mbsvp_make_unique_dir(int verbose, const char *parent, const char *prefix, char *out, size_t size, int *error);
int mbsvp_copy_file(int verbose, const char *source, const char *target, int *error);
int mbsvp_link_or_copy(int verbose, const char *source, const char *target, int *error);
int mbsvp_remove_dir(int verbose, const char *path, int *error);
int mbsvp_list_dir(int verbose, const char *path, struct mbsvp_strings *names, int *error);

int mbsvp_run_command(int verbose, const char *const *argv, const char *stdout_file, const char *log_file,
                      int *exit_code, int *error);
int mbsvp_run_shell(int verbose, const char *command, int *exit_code, int *error);
char *mbsvp_quote(const char *text, char *out, size_t size);

void mbsvp_format_double(double value, char *out, size_t size);
void mbsvp_json_escape(const char *text, char *out, size_t size);

/*--------------------------------------------------------------------*/
/* mbsvp_profile.c */

struct mbsvp_profile {
  int n;
  double *depth;
  double *speed;
};

enum mbsvp_kind { MBSVP_ANNUAL = 0, MBSVP_SEASONAL = 1, MBSVP_MONTHLY = 2, MBSVP_KINDS = 3 };

void mbsvp_profile_init(struct mbsvp_profile *profile);
int mbsvp_profile_free(int verbose, struct mbsvp_profile *profile, int *error);
int mbsvp_profile_clean(int verbose, int n, const double *depth, const double *speed, struct mbsvp_profile *clean,
                        char *message, int *error);
int mbsvp_profile_tail(int verbose, const struct mbsvp_profile *primary, const struct mbsvp_profile *donor,
                       struct mbsvp_profile *result, char *message, int *error);
int mbsvp_profile_complete(int verbose, const struct mbsvp_profile *observed, double target,
                           struct mbsvp_profile *complete, char *message, int *error);
int mbsvp_profile_chain(int verbose, const struct mbsvp_profile *raw[MBSVP_KINDS],
                        struct mbsvp_profile observed[MBSVP_KINDS], char *message, int *error);
int mbsvp_profile_read_internal(int verbose, const char *path, double maximum, struct mbsvp_profile *profile, bool *valid,
                                int *error);
bool mbsvp_profile_equal(const struct mbsvp_profile *a, const struct mbsvp_profile *b);

/*--------------------------------------------------------------------*/
/* mbsvp_location.c */

struct mbsvp_location {
  bool has_position;
  double longitude;
  double latitude;
  char method[32];
  char source[32];
  bool has_area;
  double west;
  double east;
  double south;
  double north;
  int grid_lat;
  int grid_lon;
  char representative[64];
  char dataset[256];
  char formula[16];
  char kind[16];
  bool has_appended;
  double appended_min_depth;
  double appended_max_depth;
};

void mbsvp_location_init(struct mbsvp_location *location);
double mbsvp_location_longitude(double longitude);
int mbsvp_location_comment(int verbose, const struct mbsvp_location *location, const char *marker, char *line, size_t size,
                           char *message, int *error);
int mbsvp_location_annotate_internal(int verbose, const char *path, char *message, int *error);

/*--------------------------------------------------------------------*/
/* mbsvp_woa.c */

struct mbsvp_survey {
  double west;
  double east;
  double south;
  double north;
  double depth;
  char month[3];
  char season[3];
};

struct mbsvp_grid {
  int ndepth;
  double *depth;
  double *chen;
  double *leroy;
  struct mbsvp_location location;
};

void mbsvp_circular_bounds(double *west, double *east);
bool mbsvp_woa_parse_name(const char *name, int *year, char *period, size_t period_size, char *time_code,
                          char *resolution);
void mbsvp_grid_init(struct mbsvp_grid *grid);
int mbsvp_grid_free(int verbose, struct mbsvp_grid *grid, int *error);
int mbsvp_woa_read_grid(int verbose, const char *path, const struct mbsvp_survey *survey, bool nearest,
                        struct mbsvp_grid *grid, char *message, int *error);
int mbsvp_woa_generate(int verbose, const char *nc_folder, const char *folder, const struct mbsvp_survey *survey,
                       const struct mbsvp_strings *periods, const struct mbsvp_strings *resolutions, bool nearest,
                       struct mbsvp_strings *candidates, struct mbsvp_strings *notes, char *message, int *error);
int mbsvp_write_profile(int verbose, const char *path, const struct mbsvp_profile *profile, const char *description,
                        const struct mbsvp_location *location, const struct mbsvp_location *donors, int ndonors,
                        char *message, int *error);

/*--------------------------------------------------------------------*/
/* mbsvp_score.c */

struct mbsvp_score {
  long soundings;
  long pings;
  long rejected_pings;
  double sum;
  double sum_abs;
  double sum2;
  bool keep_beams;
  int nbeams;
  long *beam_count;
  double *beam_sum;
  double *beam_sum2;
};

void mbsvp_score_init(struct mbsvp_score *score, bool keep_beams);
int mbsvp_score_free(int verbose, struct mbsvp_score *score, int *error);
int mbsvp_score_ping(int verbose, int n, const int *beam, const double *across, const double *depth, int minimum,
                     struct mbsvp_score *score, int *error);
int mbsvp_score_file(int verbose, char *file, int format, int minimum, struct mbsvp_score *score, char *message, int *error);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* MBSVP_TOOLS_H_ */
