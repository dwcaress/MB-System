/*--------------------------------------------------------------------
 *    The MB-system:  mbsvpbest.c  3 November 2026
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
 * mbsvpbest compares sound velocity profiles on swath data over locally
 * planar seafloor. Candidates are World Ocean Atlas climatological
 * profiles, taken from sound speed grids in a local directory, and the
 * profiles embedded in the swath files. Each candidate is applied through
 * mbprocess to trial output files. In each ping the processed depths are
 * fitted by a straight line against acrosstrack distance, and the candidates
 * are ranked by the root mean square of the pooled residuals.
 *
 * The winner is the best of the profiles tested, for the selected data and
 * this metric. The ranking does not show that it reproduces the true water
 * column. Original parameter files are restored after evaluation, and the
 * winner is only applied with --apply-best.
 *
 * This program derives from the Python program mbbestsvp, later renamed
 * mbsvpbest, by Dominik Patrick Nommensen:
 *   https://github.com/DominikPatrickNommensen/MB-System-Best-WOA18-SVP
 *
 * Author:  C. dos Santos Ferreira
 * Date:    November 3, 2026
 */

#include <getopt.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <utime.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

static const char program_name[] = "mbsvpbest";
static const char help_message[] =
    "mbsvpbest compares World Ocean Atlas and embedded sound velocity\n"
    "profiles on swath data over locally planar seafloor. Each candidate\n"
    "is applied through mbprocess to trial files and ranked by the RMSE of\n"
    "the depth residuals from a straight line fitted to each ping. The\n"
    "result is the best of the profiles tested, not the true water column.\n"
    "Parameter files are restored unless --apply-best is given.";
static const char usage_message[] =
    "mbsvpbest\n"
    "\t--svpfolder=directory {-Sdirectory}\n"
    "\t--apply-best\n"
    "\t--enddate {-E}\n"
    "\t--help {-H}\n"
    "\t--input=file {-Ifile} (also --swathfile)\n"
    "\t--minimum-beams=number\n"
    "\t--nearest {-N}\n"
    "\t--outputfolder=directory {-Odirectory}\n"
    "\t--period=period[,period...] {-Pperiod[,period...]}\n"
    "\t--plots\n"
    "\t--resolution=01|04[,...] {-R01|04[,...]}\n"
    "\t--selection=combined|per-file\n"
    "\t--verbose {-V}";

/* files next to an input that a trial changes; mbprocess leaves its lock
   behind if it is stopped, which would block later processing of the file */
#define SIDECARS 3
static const char *sidecar_suffix[SIDECARS] = {".par", ".resf", ".lck"};
/* files that mbprocess and mbinfo write next to a trial output */
#define TRIAL_SIDECARS 7
static const char *trial_suffix[TRIAL_SIDECARS] = {"", ".inf", ".fbt", ".fnv", ".ffa", ".ffs", ".resf"};

struct options {
  int verbose;
  mb_path input;
  mb_path svpfolder;
  mb_path outputfolder;
  struct mbsvp_strings periods;
  struct mbsvp_strings resolutions;
  bool nearest;
  bool enddate;
  bool per_file;
  bool plots;
  bool apply_best;
  int minimum_beams;
};

struct input_file {
  mb_path path;
  int format;
};

struct input_list {
  int n;
  int alloc;
  struct input_file *items;
};

struct record {
  mb_path profile;
  double rmse;
  double mae;
  double mse;
  double me;
  long soundings;
  long pings;
  long rejected_pings;
  int order;
};

struct sidecar_state {
  bool existed[SIDECARS];
  time_t access_time[SIDECARS];
  time_t modify_time[SIDECARS];
};

struct group_result {
  int exit_code;
  bool has_winner;
  struct record winner;
  mb_path ranking_file;
  bool applied;
  char error[MBSVP_MESSAGE_SIZE];
};

/*--------------------------------------------------------------------*/
static void handle_signal(int signal_number) {
  (void)signal_number;
  mbsvp_interrupted = 1;
}
/*--------------------------------------------------------------------*/
static int input_list_add(int verbose, struct input_list *list, const char *path, int format, int *error) {
  for (int i = 0; i < list->n; i++)
    if (strcmp(list->items[i].path, path) == 0)
      return MB_SUCCESS;
  if (list->n >= list->alloc) {
    const int alloc = list->alloc + 256;
    if (mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(struct input_file), (void **)&list->items, error) != MB_SUCCESS) {
      list->n = 0;
      list->alloc = 0;
      return MB_FAILURE;
    }
    list->alloc = alloc;
  }
  snprintf(list->items[list->n].path, sizeof(list->items[list->n].path), "%s", path);
  list->items[list->n].format = format;
  list->n++;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* mb_datalist_read() quietly skips missing files and stops at a recursion
   limit. Walk the lists first so that a missing file, a list that includes
   itself or an unsupported negative format is an error. */
static int check_datalist(int verbose, const char *path, const char **active, int nactive, char *message, int *error) {
  mb_path resolved;
  if (mbsvp_absolute_path(verbose, path, resolved, sizeof(resolved), error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Input does not exist: %s", path);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  for (int i = 0; i < nactive; i++) {
    if (strcmp(active[i], resolved) == 0) {
      mbsvp_set_message(message, "Recursive datalist: %s", resolved);
      *error = MB_ERROR_BAD_PARAMETER;
      return MB_FAILURE;
    }
  }
  if (nactive >= MB_DATALIST_RECURSION_MAX) {
    mbsvp_set_message(message, "Datalists nested too deeply at: %s", resolved);
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  active[nactive] = resolved;

  FILE *fp = fopen(resolved, "r");
  if (fp == NULL) {
    mbsvp_set_message(message, "Input does not exist: %s", resolved);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  int number = 0;
  char line[MB_PATH_MAXLINE];
  while (status == MB_SUCCESS && fgets(line, sizeof(line), fp) != NULL) {
    number++;
    if (line[0] == '#' || line[0] == '$')
      continue;
    const char *text = line;
    if (strncmp(text, "R:", 2) == 0 || strncmp(text, "P:", 2) == 0 || strncmp(text, "A:", 2) == 0)
      text += 2;
    mb_path child;
    int format = 0;
    const int nscan = sscanf(text, "%1023s %d", child, &format);
    if (nscan < 1)
      continue;
    mb_path full;
    bool fits = true;
    if (child[0] == '/' || strrchr(resolved, '/') == NULL) {
      fits = strlen(child) < sizeof(full);
      if (fits)
        strcpy(full, child);
    }
    else {
      mb_path dir;
      strcpy(dir, resolved);
      *strrchr(dir, '/') = '\0';
      fits = mbsvp_join(full, sizeof(full), dir, child);
    }
    if (!fits) {
      mbsvp_set_message(message, "Path is too long at %s:%d", resolved, number);
      *error = MB_ERROR_BAD_PARAMETER;
      status = MB_FAILURE;
      break;
    }
    if (nscan < 2) {
      int local_error = MB_ERROR_NO_ERROR;
      mb_get_format(verbose, full, NULL, &format, &local_error);
    }
    if (format < 0 && format != MBF_DATALIST) {
      mbsvp_set_message(message, "Only ordinary/nested -1 datalists are supported, at %s:%d", resolved, number);
      *error = MB_ERROR_BAD_PARAMETER;
      status = MB_FAILURE;
    }
    else if (format == MBF_DATALIST) {
      status = check_datalist(verbose, full, active, nactive + 1, message, error);
    }
    else if (!mbsvp_is_file(full)) {
      mbsvp_set_message(message, "Input does not exist: %s", full);
      *error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
    }
  }
  fclose(fp);
  return status;
}
/*--------------------------------------------------------------------*/
static int expand_inputs(int verbose, const char *input, struct input_list *list, char *message, int *error) {
  if (!mbsvp_is_file(input)) {
    mbsvp_set_message(message, "Input does not exist: %s", input);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  mb_path read_file;
  snprintf(read_file, sizeof(read_file), "%s", input);
  int format = 0;
  int local_error = MB_ERROR_NO_ERROR;
  mb_get_format(verbose, read_file, NULL, &format, &local_error);

  mb_path resolved;
  if (format >= 0) {
    if (format == 0) {
      mbsvp_set_message(message, "Cannot tell the swath format of %s", input);
      *error = MB_ERROR_BAD_FORMAT;
      return MB_FAILURE;
    }
    if (mbsvp_absolute_path(verbose, input, resolved, sizeof(resolved), error) != MB_SUCCESS) {
      mbsvp_set_message(message, "Input does not exist: %s", input);
      return MB_FAILURE;
    }
    return input_list_add(verbose, list, resolved, format, error);
  }

  const char *active[MB_DATALIST_RECURSION_MAX + 1];
  if (check_datalist(verbose, input, active, 0, message, error) != MB_SUCCESS)
    return MB_FAILURE;

  void *datalist = NULL;
  if (mb_datalist_open(verbose, &datalist, read_file, MB_DATALIST_LOOK_NO, error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Unable to open data list file: %s", input);
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  mb_path path;
  mb_path dpath;
  double weight;
  while (status == MB_SUCCESS && mb_datalist_read(verbose, datalist, path, dpath, &format, &weight, &local_error) == MB_SUCCESS) {
    if (mbsvp_absolute_path(verbose, path, resolved, sizeof(resolved), error) != MB_SUCCESS) {
      mbsvp_set_message(message, "Input does not exist: %s", path);
      status = MB_FAILURE;
    }
    else {
      status = input_list_add(verbose, list, resolved, format, error);
    }
  }
  local_error = MB_ERROR_NO_ERROR;
  mb_datalist_close(verbose, &datalist, &local_error);
  if (status == MB_SUCCESS && list->n == 0) {
    mbsvp_set_message(message, "Empty datalist: %s", input);
    *error = MB_ERROR_BAD_PARAMETER;
    status = MB_FAILURE;
  }
  return status;
}
/*--------------------------------------------------------------------*/
/* Bounds, maximum depth and date of a file, from mbinfo. Its output goes in
   the run directory, so nothing is added next to the swath file. */
static int read_metadata(int verbose, const struct input_file *file, const char *folder, int index, const char *log_file,
                         bool end, struct mbsvp_survey *survey, char *message, int *error) {
  mb_path root;
  mb_path inf;
  char format_option[32];
  snprintf(format_option, sizeof(format_option), "-F%d", file->format);
  const int len = snprintf(root, sizeof(root), "%s/mbinfo_%04d", folder, index);
  if (len < 0 || (size_t)len >= sizeof(root) - 4) {
    mbsvp_set_message(message, "Output path is too long");
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  snprintf(inf, sizeof(inf), "%s.inf", root);
  const char *argv[] = {"mbinfo", format_option, "-I", file->path, NULL};
  int exit_code;
  int local_error = MB_ERROR_NO_ERROR;
  if (mbsvp_run_command(verbose, argv, inf, log_file, &exit_code, &local_error) != MB_SUCCESS) {
    mbsvp_set_message(message, "mbinfo failed (%d) on %s; see %s", exit_code, file->path, log_file);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  struct mb_info_struct info;
  if (mb_get_info(verbose, root, &info, 0, error) != MB_SUCCESS || info.nrecords <= 0 || info.time_start <= 0.0) {
    mbsvp_set_message(message, "Cannot use mbinfo results for %s", file->path);
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  survey->west = info.lon_min;
  survey->east = info.lon_max;
  survey->south = info.lat_min;
  survey->north = info.lat_max;
  survey->depth = info.depth_max;
  int time_i[7];
  mb_get_date(verbose, end ? info.time_end : info.time_start, time_i);
  snprintf(survey->month, sizeof(survey->month), "%02d", time_i[1]);
  snprintf(survey->season, sizeof(survey->season), "%02d", 13 + (time_i[1] - 1) / 3);
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Combine the files: each keeps its own circular longitude interval. */
static int combine_surveys(const struct mbsvp_survey *surveys, int n, struct mbsvp_survey *combined, char *message, int *error) {
  *combined = surveys[0];
  double west = surveys[0].west;
  double east = surveys[0].east;
  mbsvp_circular_bounds(&west, &east);
  const double reference = 0.5 * (west + east);
  for (int i = 0; i < n; i++) {
    double w = surveys[i].west;
    double e = surveys[i].east;
    mbsvp_circular_bounds(&w, &e);
    const double shift = 360.0 * rint((reference - 0.5 * (w + e)) / 360.0);
    if (i == 0 || w + shift < combined->west)
      combined->west = w + shift;
    if (i == 0 || e + shift > combined->east)
      combined->east = e + shift;
    if (surveys[i].south < combined->south)
      combined->south = surveys[i].south;
    if (surveys[i].north > combined->north)
      combined->north = surveys[i].north;
    if (surveys[i].depth > combined->depth)
      combined->depth = surveys[i].depth;
    if (strcmp(surveys[i].month, surveys[0].month) != 0) {
      mbsvp_set_message(message, "Datalist spans different selected months: evaluate month groups separately");
      *error = MB_ERROR_BAD_PARAMETER;
      return MB_FAILURE;
    }
  }
  if (combined->east - combined->west >= 180.0) {
    mbsvp_set_message(message, "Survey extent >=180 degrees is unsupported");
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Extract embedded profiles. mbsvplist writes next to its input, so it reads
   a link in a work directory, not the file in the user's directory. A table
   identical to one already kept is the same candidate and is evaluated once. */
static int internal_profiles(int verbose, const struct input_list *files, const char *folder, const char *log_file,
                             double maximum, struct mbsvp_strings *candidates, struct mbsvp_strings *notes, int *error) {
  struct mbsvp_profile *kept = NULL;
  int nkept = 0;
  int kept_alloc = 0;
  int status = MB_SUCCESS;
  char note[2 * MB_PATH_MAXLINE + 128];

  for (int index = 0; index < files->n && status == MB_SUCCESS && !mbsvp_interrupted; index++) {
    const char *source = files->items[index].path;
    mb_path work;
    mb_path link_path;
    char work_name[32];
    snprintf(work_name, sizeof(work_name), "extract_%04d", index);
    int local_error = MB_ERROR_NO_ERROR;
    if (!mbsvp_join(work, sizeof(work), folder, work_name) ||
        !mbsvp_join(link_path, sizeof(link_path), work, mbsvp_basename(source)) ||
        mbsvp_make_dirs(verbose, work, &local_error) != MB_SUCCESS ||
        mbsvp_link_or_copy(verbose, source, link_path, &local_error) != MB_SUCCESS) {
      snprintf(note, sizeof(note), "Internal extraction %s: cannot prepare work directory", source);
      status = mbsvp_strings_add(verbose, notes, note, error);
      continue;
    }
    char format_option[32];
    snprintf(format_option, sizeof(format_option), "-F%d", files->items[index].format);
    const char *argv[] = {"mbsvplist", format_option, "-I", link_path, "-O", NULL};
    int exit_code;
    if (mbsvp_run_command(verbose, argv, NULL, log_file, &exit_code, &local_error) != MB_SUCCESS) {
      snprintf(note, sizeof(note), "Internal extraction %s: mbsvplist failed (%d); see %s", source, exit_code, log_file);
      status = mbsvp_strings_add(verbose, notes, note, error);
    }
    else {
      struct mbsvp_strings names;
      mbsvp_strings_init(&names);
      status = mbsvp_list_dir(verbose, work, &names, error);
      const size_t base_len = strlen(mbsvp_basename(source));
      int number = 0;
      for (int i = 0; i < names.n && status == MB_SUCCESS; i++) {
        const char *name = names.items[i];
        if (strncmp(name, mbsvp_basename(source), base_len) != 0 || name[base_len] != '_' ||
            strcmp(mbsvp_suffix(name), ".svp") != 0)
          continue;
        mb_path path;
        struct mbsvp_profile profile;
        mbsvp_profile_init(&profile);
        bool valid = false;
        if (mbsvp_join(path, sizeof(path), work, name))
          status = mbsvp_profile_read_internal(verbose, path, maximum, &profile, &valid, error);
        if (status == MB_SUCCESS && !valid) {
          snprintf(note, sizeof(note), "Rejected malformed/short internal profile: %s", name);
          status = mbsvp_strings_add(verbose, notes, note, error);
        }
        else if (status == MB_SUCCESS) {
          int same = -1;
          for (int k = 0; k < nkept && same < 0; k++)
            if (mbsvp_profile_equal(&profile, &kept[k]))
              same = k;
          mb_path target;
          char target_name[64];
          snprintf(target_name, sizeof(target_name), "internal_%04d_%04d.svp", index, number);
          if (same >= 0) {
            snprintf(note, sizeof(note), "Skipped internal profile %s: same table as an earlier internal profile", name);
            status = mbsvp_strings_add(verbose, notes, note, error);
          }
          else if (!mbsvp_join(target, sizeof(target), folder, target_name) ||
                   mbsvp_copy_file(verbose, path, target, &local_error) != MB_SUCCESS ||
                   mbsvp_location_annotate_internal(verbose, target, NULL, &local_error) != MB_SUCCESS) {
            snprintf(note, sizeof(note), "Internal extraction %s: cannot store %s", source, name);
            status = mbsvp_strings_add(verbose, notes, note, error);
          }
          else {
            status = mbsvp_strings_add(verbose, candidates, target, error);
            if (status == MB_SUCCESS && nkept >= kept_alloc) {
              kept_alloc += 64;
              status = mb_reallocd(verbose, __FILE__, __LINE__, kept_alloc * sizeof(struct mbsvp_profile), (void **)&kept, error);
            }
            if (status == MB_SUCCESS) {
              kept[nkept++] = profile; /* the arrays now belong to the kept list */
              mbsvp_profile_init(&profile);
            }
          }
        }
        int error_free = MB_ERROR_NO_ERROR;
        mbsvp_profile_free(verbose, &profile, &error_free);
        number++;
      }
      int error_free = MB_ERROR_NO_ERROR;
      mbsvp_strings_free(verbose, &names, &error_free);
    }
    local_error = MB_ERROR_NO_ERROR;
    mbsvp_remove_dir(verbose, work, &local_error);
  }

  int error_free = MB_ERROR_NO_ERROR;
  for (int k = 0; k < nkept; k++)
    mbsvp_profile_free(verbose, &kept[k], &error_free);
  if (kept != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&kept, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
/* The same edit mbset makes for -PSVPFILE and -POUTFILE. Then read the file
   back: the parameter file cannot hold a path with spaces. */
static int set_parameters(int verbose, const struct input_file *file, const char *svpfile, const char *ofile, char *message,
                          int *error) {
  static struct mb_process_struct process;
  mb_path ifile;
  strcpy(ifile, file->path);
  if (strlen(svpfile) >= sizeof(process.mbp_svpfile) || (ofile != NULL && strlen(ofile) >= sizeof(process.mbp_ofile))) {
    mbsvp_set_message(message, "Path is too long for a parameter file");
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  if (mb_pr_readpar(verbose, ifile, false, &process, error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot read parameters of %s", ifile);
    return MB_FAILURE;
  }
  process.mbp_ifile_specified = true;
  if (!process.mbp_format_specified) {
    process.mbp_format = file->format;
    process.mbp_format_specified = true;
  }
  strcpy(process.mbp_svpfile, svpfile);
  process.mbp_svp_mode = MBP_SVP_ON;
  if (ofile != NULL) {
    strcpy(process.mbp_ofile, ofile);
    process.mbp_ofile_specified = true;
  }
  if (mb_pr_writepar(verbose, ifile, &process, error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot write parameters of %s", ifile);
    return MB_FAILURE;
  }
  if (mb_pr_readpar(verbose, ifile, false, &process, error) != MB_SUCCESS ||
      strcmp(mbsvp_basename(process.mbp_svpfile), mbsvp_basename(svpfile)) != 0 ||
      (ofile != NULL && strcmp(mbsvp_basename(process.mbp_ofile), mbsvp_basename(ofile)) != 0)) {
    mbsvp_set_message(message, "Parameter file of %s cannot hold these paths; avoid spaces in directory names", ifile);
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static bool sidecar_paths(const char *folder, const char *source, int index, int s, char *live, char *saved, size_t size) {
  const int a = snprintf(live, size, "%s%s", source, sidecar_suffix[s]);
  const int b = snprintf(saved, size, "%s/%04d%s", folder, index, sidecar_suffix[s]);
  return a > 0 && (size_t)a < size && b > 0 && (size_t)b < size;
}
/*--------------------------------------------------------------------*/
/* The copies stay on disk until the run ends, so the files can still be
   recovered by hand if this program is killed. */
static int sidecars_save(int verbose, const struct input_list *files, const char *folder, struct sidecar_state *states,
                         char *message, int *error) {
  if (mbsvp_make_dirs(verbose, folder, error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot make %s", folder);
    return MB_FAILURE;
  }
  for (int i = 0; i < files->n; i++) {
    for (int s = 0; s < SIDECARS; s++) {
      mb_path live;
      mb_path saved;
      if (!sidecar_paths(folder, files->items[i].path, i, s, live, saved, sizeof(live))) {
        mbsvp_set_message(message, "Path is too long: %s", files->items[i].path);
        *error = MB_ERROR_BAD_PARAMETER;
        return MB_FAILURE;
      }
      struct stat status;
      states[i].existed[s] = stat(live, &status) == 0;
      if (states[i].existed[s]) {
        states[i].access_time[s] = status.st_atime;
        states[i].modify_time[s] = status.st_mtime;
        if (mbsvp_copy_file(verbose, live, saved, error) != MB_SUCCESS) {
          mbsvp_set_message(message, "Cannot save %s", live);
          return MB_FAILURE;
        }
      }
    }
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
static int sidecars_restore(int verbose, const struct input_list *files, const char *folder,
                            const struct sidecar_state *states, char *message, int *error) {
  int status = MB_SUCCESS;
  for (int i = 0; i < files->n; i++) {
    for (int s = 0; s < SIDECARS; s++) {
      mb_path live;
      mb_path saved;
      sidecar_paths(folder, files->items[i].path, i, s, live, saved, sizeof(live));
      if (states[i].existed[s]) {
        if (mbsvp_copy_file(verbose, saved, live, error) != MB_SUCCESS) {
          mbsvp_set_message(message, "Cannot restore %s; a copy is kept at %s", live, saved);
          status = MB_FAILURE;
          continue;
        }
        struct utimbuf times;
        times.actime = states[i].access_time[s];
        times.modtime = states[i].modify_time[s];
        utime(live, &times);
      }
      else if (mbsvp_is_file(live)) {
        remove(live);
      }
    }
  }
  return status;
}
/*--------------------------------------------------------------------*/
static int plot_residuals(int verbose, const struct mbsvp_score *score, const char *candidate, const char *folder,
                          double rmse, const char *log_file, int *error) {
  /* table of beam statistics */
  mb_path stem;
  snprintf(stem, sizeof(stem), "%s", mbsvp_basename(candidate));
  char *dot = strrchr(stem, '.');
  if (dot != NULL)
    *dot = '\0';
  mb_path prefix;
  char table[MB_PATH_MAXLINE + 32];
  char zero[MB_PATH_MAXLINE + 32];
  if (!mbsvp_join(prefix, sizeof(prefix), folder, stem)) {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  snprintf(table, sizeof(table), "%s_residuals.txt", prefix);
  snprintf(zero, sizeof(zero), "%s_zero.txt", prefix);
  FILE *fp = fopen(table, "w");
  if (fp == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  fprintf(fp, "# beam mean_residual_m std_residual_m soundings\n");
  double xmin = 0.0;
  double xmax = 1.0;
  double ymin = 0.0;
  double ymax = 0.0;
  bool first = true;
  for (int b = 0; b < score->nbeams; b++) {
    const long n = score->beam_count[b];
    if (n <= 0)
      continue;
    const double mean = score->beam_sum[b] / n;
    const double variance = score->beam_sum2[b] / n - mean * mean;
    const double std = variance > 0.0 ? sqrt(variance) : 0.0;
    fprintf(fp, "%d %.6f %.6f %ld\n", b, mean, std, n);
    if (first)
      xmin = b;
    xmax = b;
    ymin = MIN(ymin, mean - std);
    ymax = MAX(ymax, mean + std);
    first = false;
  }
  fclose(fp);
  if (first) {
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  if (xmax <= xmin)
    xmax = xmin + 1.0;
  const double pad = ymax > ymin ? 0.05 * (ymax - ymin) : 1.0;
  ymin -= pad;
  ymax += pad;
  fp = fopen(zero, "w");
  if (fp == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  fprintf(fp, "%f 0\n%f 0\n", xmin, xmax);
  fclose(fp);

  /* one shell runs the whole GMT session */
  char q_prefix[2 * MB_PATH_MAXLINE];
  char q_table[2 * MB_PATH_MAXLINE];
  char q_zero[2 * MB_PATH_MAXLINE];
  char q_log[2 * MB_PATH_MAXLINE];
  char title[2 * MB_PATH_MAXLINE];
  char q_title[4 * MB_PATH_MAXLINE];
  snprintf(title, sizeof(title), "-B+t%s; RMSE=%.4f m", mbsvp_basename(candidate), rmse);
  if (mbsvp_quote(prefix, q_prefix, sizeof(q_prefix)) == NULL || mbsvp_quote(table, q_table, sizeof(q_table)) == NULL ||
      mbsvp_quote(zero, q_zero, sizeof(q_zero)) == NULL || mbsvp_quote(log_file, q_log, sizeof(q_log)) == NULL ||
      mbsvp_quote(title, q_title, sizeof(q_title)) == NULL) {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  char command[20 * MB_PATH_MAXLINE];
  const int len = snprintf(command, sizeof(command),
                           "( gmt begin %s png E150 && "
                           "gmt basemap -R%g/%g/%g/%g -JX24c/-13c -Bxaf+l\"Beam number\" -Byaf+l\"Depth residual (m)\" %s && "
                           "gmt plot %s -W0.5p,blue && "
                           "gmt plot %s -i0,1,2 -Ey+p0.5p,grey && "
                           "gmt plot %s -i0,1 -W0.75p,black && "
                           "gmt end ) >> %s 2>&1",
                           q_prefix, xmin, xmax, ymin, ymax, q_title, q_zero, q_table, q_table, q_log);
  int status = MB_FAILURE;
  if (len > 0 && (size_t)len < sizeof(command)) {
    int exit_code;
    status = mbsvp_run_shell(verbose, command, &exit_code, error);
  }
  remove(zero);
  return status;
}
/*--------------------------------------------------------------------*/
static void write_record(FILE *fp, const struct record *record, const char *indent) {
  char escaped[2 * MB_PATH_MAXLINE];
  char text[64];
  mbsvp_json_escape(record->profile, escaped, sizeof(escaped));
  fprintf(fp, "%s{\n%s  \"profile\": \"%s\",\n", indent, indent, escaped);
  mbsvp_format_double(record->rmse, text, sizeof(text));
  fprintf(fp, "%s  \"rmse\": %s,\n", indent, text);
  mbsvp_format_double(record->mae, text, sizeof(text));
  fprintf(fp, "%s  \"mae\": %s,\n", indent, text);
  mbsvp_format_double(record->mse, text, sizeof(text));
  fprintf(fp, "%s  \"mse\": %s,\n", indent, text);
  mbsvp_format_double(record->me, text, sizeof(text));
  fprintf(fp, "%s  \"me\": %s,\n", indent, text);
  fprintf(fp, "%s  \"soundings\": %ld,\n", indent, record->soundings);
  fprintf(fp, "%s  \"pings\": %ld,\n", indent, record->pings);
  fprintf(fp, "%s  \"rejected_pings\": %ld\n", indent, record->rejected_pings);
  fprintf(fp, "%s}", indent);
}
/*--------------------------------------------------------------------*/
static void write_string_array(FILE *fp, const char *name, const struct mbsvp_strings *list) {
  char escaped[2 * MBSVP_MESSAGE_SIZE];
  fprintf(fp, "  \"%s\": [", name);
  for (int i = 0; i < list->n; i++) {
    mbsvp_json_escape(list->items[i], escaped, sizeof(escaped));
    fprintf(fp, "%s\n    \"%s\"", i > 0 ? "," : "", escaped);
  }
  fprintf(fp, "%s],\n", list->n > 0 ? "\n  " : "");
}
/*--------------------------------------------------------------------*/
static int write_ranking(const char *folder, const struct input_list *files, const struct record *records, int nrecords,
                         const struct mbsvp_strings *failed, const struct mbsvp_strings *failed_why,
                         const struct mbsvp_strings *notes, bool applied, char *ranking_file, size_t ranking_size,
                         int *error) {
  mb_path path;
  if (!mbsvp_join(path, sizeof(path), folder, "ranking.json")) {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  FILE *fp = fopen(path, "w");
  if (fp == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  char escaped[2 * MBSVP_MESSAGE_SIZE];
  fprintf(fp, "{\n  \"inputs\": [");
  for (int i = 0; i < files->n; i++) {
    mbsvp_json_escape(files->items[i].path, escaped, sizeof(escaped));
    fprintf(fp, "%s\n    \"%s\"", i > 0 ? "," : "", escaped);
  }
  fprintf(fp, "\n  ],\n  \"ranking\": [");
  for (int i = 0; i < nrecords; i++) {
    fprintf(fp, "%s\n", i > 0 ? "," : "");
    write_record(fp, &records[i], "    ");
  }
  fprintf(fp, "%s],\n  \"candidate_failures\": [", nrecords > 0 ? "\n  " : "");
  for (int i = 0; i < failed->n; i++) {
    mbsvp_json_escape(failed->items[i], escaped, sizeof(escaped));
    fprintf(fp, "%s\n    {\n      \"profile\": \"%s\",\n", i > 0 ? "," : "", escaped);
    mbsvp_json_escape(failed_why->items[i], escaped, sizeof(escaped));
    fprintf(fp, "      \"error\": \"%s\"\n    }", escaped);
  }
  fprintf(fp, "%s],\n", failed->n > 0 ? "\n  " : "");
  write_string_array(fp, "profile_notes", notes);
  fprintf(fp, "  \"applied\": %s\n}\n", applied ? "true" : "false");
  const int closed = fclose(fp);
  snprintf(ranking_file, ranking_size, "%s", path);
  if (closed != 0) {
    *error = MB_ERROR_WRITE_FAIL;
    return MB_FAILURE;
  }

  if (!mbsvp_join(path, sizeof(path), folder, "ranking.csv")) {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  fp = fopen(path, "w");
  if (fp == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  if (nrecords > 0)
    fprintf(fp, "profile,rmse,mae,mse,me,soundings,pings,rejected_pings\n");
  for (int i = 0; i < nrecords; i++) {
    char rmse[64];
    char mae[64];
    char mse[64];
    char me[64];
    mbsvp_format_double(records[i].rmse, rmse, sizeof(rmse));
    mbsvp_format_double(records[i].mae, mae, sizeof(mae));
    mbsvp_format_double(records[i].mse, mse, sizeof(mse));
    mbsvp_format_double(records[i].me, me, sizeof(me));
    fprintf(fp, "%s,%s,%s,%s,%s,%ld,%ld,%ld\n", records[i].profile, rmse, mae, mse, me, records[i].soundings, records[i].pings,
            records[i].rejected_pings);
  }
  if (fclose(fp) != 0) {
    *error = MB_ERROR_WRITE_FAIL;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Lowest RMSE first; equal scores keep the candidate order. */
static int compare_records(const void *a, const void *b) {
  const struct record *ra = (const struct record *)a;
  const struct record *rb = (const struct record *)b;
  if (ra->rmse < rb->rmse)
    return -1;
  if (ra->rmse > rb->rmse)
    return 1;
  return ra->order - rb->order;
}
/*--------------------------------------------------------------------*/
/* Process every file with one candidate and pool the residuals. */
static int evaluate_candidate(const struct options *options, const struct input_list *files, const char *candidate,
                              const char *folder, const char *log_file, struct record *record, struct mbsvp_score *score,
                              char *message, int *error) {
  const int verbose = options->verbose;
  for (int i = 0; i < files->n; i++) {
    const struct input_file *file = &files->items[i];
    mb_path output;
    const int len = snprintf(output, sizeof(output), "%s/processed_%04d%s", folder, i, mbsvp_suffix(file->path));
    if (len < 0 || (size_t)len >= sizeof(output)) {
      mbsvp_set_message(message, "Output path is too long");
      *error = MB_ERROR_BAD_PARAMETER;
      return MB_FAILURE;
    }
    if (set_parameters(verbose, file, candidate, output, message, error) != MB_SUCCESS)
      return MB_FAILURE;
    remove(output);
    const char *argv[] = {"mbprocess", "-I", file->path, "-P", NULL};
    int exit_code;
    int local_error = MB_ERROR_NO_ERROR;
    if (mbsvp_run_command(verbose, argv, NULL, log_file, &exit_code, &local_error) != MB_SUCCESS) {
      mbsvp_set_message(message, "mbprocess failed (%d) on %s; see %s", exit_code, file->path, log_file);
      *error = MB_ERROR_BAD_DATA;
      return MB_FAILURE;
    }
    if (mbsvp_interrupted)
      return MB_FAILURE;
    if (!mbsvp_is_file(output)) {
      mbsvp_set_message(message, "mbprocess did not create expected output: %s", output);
      *error = MB_ERROR_OPEN_FAIL;
      return MB_FAILURE;
    }
    int format = 0;
    local_error = MB_ERROR_NO_ERROR;
    mb_get_format(verbose, output, NULL, &format, &local_error);
    if (format <= 0)
      format = file->format;
    if (mbsvp_score_file(verbose, output, format, options->minimum_beams, score, message, error) != MB_SUCCESS)
      return MB_FAILURE;
    if (mbsvp_interrupted)
      return MB_FAILURE;
  }

  const double n = (double)score->soundings;
  record->mse = score->sum2 / n;
  record->rmse = sqrt(record->mse);
  record->mae = score->sum_abs / n;
  record->me = score->sum / n;
  record->soundings = score->soundings;
  record->pings = score->pings;
  record->rejected_pings = score->rejected_pings;
  snprintf(record->profile, sizeof(record->profile), "%s", candidate);
  if (!isfinite(record->rmse) || !isfinite(record->mae) || !isfinite(record->mse) || !isfinite(record->me)) {
    mbsvp_set_message(message, "Nonfinite score");
    *error = MB_ERROR_BAD_DATA;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Trial outputs hold the last candidate, not the winner. */
static void remove_trial_outputs(const struct input_list *files, const char *folder) {
  for (int i = 0; i < files->n; i++) {
    for (int s = 0; s < TRIAL_SIDECARS; s++) {
      mb_path trial;
      const int len = snprintf(trial, sizeof(trial), "%s/processed_%04d%s%s", folder, i, mbsvp_suffix(files->items[i].path),
                               trial_suffix[s]);
      if (len > 0 && (size_t)len < sizeof(trial))
        remove(trial);
    }
  }
}
/*--------------------------------------------------------------------*/
/* Evaluate a group of files with one shared winner. */
static void evaluate_group(const struct options *options, const struct input_list *files, const char *outputfolder,
                           struct group_result *result) {
  const int verbose = options->verbose;
  int error = MB_ERROR_NO_ERROR;
  char message[MBSVP_MESSAGE_SIZE] = "";
  int exit_code = 1;

  struct mbsvp_strings candidates;
  struct mbsvp_strings notes;
  struct mbsvp_strings failed;
  struct mbsvp_strings failed_why;
  mbsvp_strings_init(&candidates);
  mbsvp_strings_init(&notes);
  mbsvp_strings_init(&failed);
  mbsvp_strings_init(&failed_why);
  struct record *records = NULL;
  struct mbsvp_survey *surveys = NULL;
  struct sidecar_state *states = NULL;
  int nrecords = 0;
  bool saved = false;
  bool have_folder = false;

  mb_path folder;
  mb_path log_file;
  mb_path backup;
  struct mbsvp_survey survey;

  if (mbsvp_make_dirs(verbose, outputfolder, &error) != MB_SUCCESS ||
      mbsvp_make_unique_dir(verbose, outputfolder, "run", folder, sizeof(folder), &error) != MB_SUCCESS ||
      !mbsvp_join(log_file, sizeof(log_file), folder, "commands.log") ||
      !mbsvp_join(backup, sizeof(backup), folder, "backup")) {
    mbsvp_set_message(message, "Cannot make a run directory in %s", outputfolder);
    goto done;
  }
  have_folder = true;
  fprintf(stdout, "Run directory: %s\n", folder);
  fflush(stdout);

  if (mb_mallocd(verbose, __FILE__, __LINE__, files->n * sizeof(struct mbsvp_survey), (void **)&surveys, &error) != MB_SUCCESS ||
      mb_mallocd(verbose, __FILE__, __LINE__, files->n * sizeof(struct sidecar_state), (void **)&states, &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Out of memory");
    goto done;
  }
  for (int i = 0; i < files->n; i++) {
    if (read_metadata(verbose, &files->items[i], folder, i, log_file, options->enddate, &surveys[i], message, &error) != MB_SUCCESS)
      goto done;
    if (mbsvp_interrupted)
      goto done;
  }
  if (combine_surveys(surveys, files->n, &survey, message, &error) != MB_SUCCESS)
    goto done;

  if (mbsvp_woa_generate(verbose, options->svpfolder, folder, &survey, &options->periods, &options->resolutions,
                         options->nearest, &candidates, &notes, message, &error) != MB_SUCCESS)
    goto done;
  if (internal_profiles(verbose, files, folder, log_file, survey.depth, &candidates, &notes, &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot extract internal profiles");
    goto done;
  }
  if (mbsvp_interrupted)
    goto done;
  for (int i = 0; i < notes.n; i++)
    fprintf(stderr, "Profile note: %s\n", notes.items[i]);
  if (candidates.n == 0) {
    mbsvp_set_message(message, "No valid candidate profiles. Check NetCDF availability and depth coverage.");
    goto done;
  }
  if (mb_mallocd(verbose, __FILE__, __LINE__, candidates.n * sizeof(struct record), (void **)&records, &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Out of memory");
    goto done;
  }

  /* evaluate, then put the parameter files back whatever happened */
  if (sidecars_save(verbose, files, backup, states, message, &error) != MB_SUCCESS)
    goto done;
  saved = true;
  for (int c = 0; c < candidates.n && !mbsvp_interrupted; c++) {
    struct mbsvp_score score;
    mbsvp_score_init(&score, options->plots);
    char reason[MBSVP_MESSAGE_SIZE] = "";
    int local_error = MB_ERROR_NO_ERROR;
    struct record *record = &records[nrecords];
    const int status = evaluate_candidate(options, files, candidates.items[c], folder, log_file, record, &score, reason, &local_error);
    if (mbsvp_interrupted) {
      /* nothing to report for this candidate */
    }
    else if (status == MB_SUCCESS) {
      record->order = c;
      nrecords++;
      fprintf(stdout, "%s: RMSE=%.6f m; soundings=%ld\n", mbsvp_basename(candidates.items[c]), record->rmse, record->soundings);
      fflush(stdout);
      if (options->plots && plot_residuals(verbose, &score, candidates.items[c], folder, record->rmse, log_file, &local_error) != MB_SUCCESS)
        fprintf(stderr, "Warning: optional plot failed for %s; see %s\n", mbsvp_basename(candidates.items[c]), log_file);
    }
    else {
      fprintf(stderr, "Candidate failed: %s: %s\n", mbsvp_basename(candidates.items[c]), reason);
      if (mbsvp_strings_add(verbose, &failed, candidates.items[c], &error) != MB_SUCCESS ||
          mbsvp_strings_add(verbose, &failed_why, reason, &error) != MB_SUCCESS) {
        mbsvp_set_message(message, "Out of memory");
        mbsvp_score_free(verbose, &score, &local_error);
        goto done;
      }
    }
    mbsvp_score_free(verbose, &score, &local_error);
  }
  if (sidecars_restore(verbose, files, backup, states, message, &error) != MB_SUCCESS)
    goto done;
  if (mbsvp_interrupted)
    goto done;

  remove_trial_outputs(files, folder);

  qsort(records, nrecords, sizeof(struct record), compare_records);
  if (write_ranking(folder, files, records, nrecords, &failed, &failed_why, &notes, false, result->ranking_file,
                    sizeof(result->ranking_file), &error) != MB_SUCCESS) {
    mbsvp_set_message(message, "Cannot write the ranking in %s", folder);
    goto done;
  }
  if (nrecords == 0) {
    mbsvp_set_message(message, "Every candidate failed; see %s", result->ranking_file);
    goto done;
  }
  result->has_winner = true;
  result->winner = records[0];
  fprintf(stdout, "Best profile: %s\n", records[0].profile);
  fflush(stdout);

  if (options->apply_best) {
    /* parameter files change only if every file succeeds */
    for (int i = 0; i < files->n && !mbsvp_interrupted; i++) {
      if (set_parameters(verbose, &files->items[i], records[0].profile, NULL, message, &error) != MB_SUCCESS)
        goto done;
      const char *argv[] = {"mbprocess", "-I", files->items[i].path, "-P", NULL};
      int command_exit;
      int local_error = MB_ERROR_NO_ERROR;
      if (mbsvp_run_command(verbose, argv, NULL, log_file, &command_exit, &local_error) != MB_SUCCESS) {
        mbsvp_set_message(message, "mbprocess failed (%d) on %s; see %s", command_exit, files->items[i].path, log_file);
        goto done;
      }
    }
    if (mbsvp_interrupted)
      goto done;
    saved = false;
    result->applied = true;
    if (write_ranking(folder, files, records, nrecords, &failed, &failed_why, &notes, true, result->ranking_file,
                      sizeof(result->ranking_file), &error) != MB_SUCCESS) {
      mbsvp_set_message(message, "Cannot write the ranking in %s", folder);
      goto done;
    }
    fprintf(stdout, "Winner applied to original inputs.\n");
  }
  else {
    saved = false;
    fprintf(stdout, "Original parameter files restored.\n");
  }
  exit_code = 0;

done:
  if (saved) {
    /* a failed or interrupted run leaves the parameter files as they were */
    int local_error = MB_ERROR_NO_ERROR;
    char restore_message[MBSVP_MESSAGE_SIZE] = "";
    if (sidecars_restore(verbose, files, backup, states, restore_message, &local_error) != MB_SUCCESS)
      fprintf(stderr, "%s\n", restore_message);
    else
      saved = false;
  }
  if (!saved && have_folder && mbsvp_is_dir(backup)) {
    int local_error = MB_ERROR_NO_ERROR;
    mbsvp_remove_dir(verbose, backup, &local_error);
  }
  if (have_folder)
    remove_trial_outputs(files, folder);
  if (mbsvp_interrupted) {
    exit_code = 130;
    fprintf(stderr, "Interrupted; temporary evaluation parameters restored.\n");
  }
  else if (exit_code != 0) {
    fprintf(stderr, "%s\n", message);
    snprintf(result->error, sizeof(result->error), "%s", message);
  }
  result->exit_code = exit_code;

  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_strings_free(verbose, &candidates, &error_free);
  mbsvp_strings_free(verbose, &notes, &error_free);
  mbsvp_strings_free(verbose, &failed, &error_free);
  mbsvp_strings_free(verbose, &failed_why, &error_free);
  if (records != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&records, &error_free);
  if (surveys != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&surveys, &error_free);
  if (states != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&states, &error_free);
}
/*--------------------------------------------------------------------*/
struct summary_row {
  mb_path input;
  mb_path output_directory;
  char status[16];
  bool has_exit_code;
  struct group_result result;
};

/* Written after every step, by replacing the file, so that a stopped batch
   leaves a readable summary. */
static void write_summary(const char *batch, const char *created, bool apply_best, const struct summary_row *rows, int nrows,
                          bool interrupted) {
  mb_path path;
  mb_path temporary;
  char text[2 * MBSVP_MESSAGE_SIZE];
  if (!mbsvp_join(path, sizeof(path), batch, "summary.json") ||
      !mbsvp_join(temporary, sizeof(temporary), batch, "summary.json.tmp"))
    return;
  FILE *fp = fopen(temporary, "w");
  if (fp == NULL)
    return;
  mbsvp_json_escape(batch, text, sizeof(text));
  fprintf(fp, "{\n  \"selection\": \"per-file\",\n  \"created_utc\": \"%s\",\n  \"batch_directory\": \"%s\",\n", created, text);
  fprintf(fp, "  \"apply_best_requested\": %s,\n  \"files\": [", apply_best ? "true" : "false");
  for (int i = 0; i < nrows; i++) {
    const struct summary_row *row = &rows[i];
    mbsvp_json_escape(row->input, text, sizeof(text));
    fprintf(fp, "%s\n    {\n      \"input\": \"%s\",\n", i > 0 ? "," : "", text);
    mbsvp_json_escape(row->output_directory, text, sizeof(text));
    fprintf(fp, "      \"output_directory\": \"%s\",\n      \"status\": \"%s\",\n", text, row->status);
    if (row->has_exit_code)
      fprintf(fp, "      \"exit_code\": %d,\n", row->result.exit_code);
    else
      fprintf(fp, "      \"exit_code\": null,\n");
    if (row->result.has_winner) {
      fprintf(fp, "      \"winner\":\n");
      write_record(fp, &row->result.winner, "      ");
      fprintf(fp, ",\n");
    }
    else {
      fprintf(fp, "      \"winner\": null,\n");
    }
    if (row->result.ranking_file[0] != '\0') {
      mbsvp_json_escape(row->result.ranking_file, text, sizeof(text));
      fprintf(fp, "      \"ranking_file\": \"%s\",\n", text);
    }
    else {
      fprintf(fp, "      \"ranking_file\": null,\n");
    }
    if (row->result.error[0] != '\0') {
      mbsvp_json_escape(row->result.error, text, sizeof(text));
      fprintf(fp, "      \"error\": \"%s\",\n", text);
    }
    fprintf(fp, "      \"applied\": %s\n    }", row->result.applied ? "true" : "false");
  }
  fprintf(fp, "%s],\n  \"interrupted\": %s\n}\n", nrows > 0 ? "\n  " : "", interrupted ? "true" : "false");
  if (fclose(fp) != 0)
    return;
#ifdef _WIN32
  remove(path);
#endif
  rename(temporary, path);
}
/*--------------------------------------------------------------------*/
/* One independent winner for each file. A failed file is recorded and the
   next one goes on; an interrupt stops the batch. */
static int evaluate_per_file(const struct options *options, const struct input_list *files) {
  const int verbose = options->verbose;
  int error = MB_ERROR_NO_ERROR;
  mb_path parent;
  mb_path batch;
  if (mbsvp_make_dirs(verbose, options->outputfolder, &error) != MB_SUCCESS ||
      mbsvp_absolute_path(verbose, options->outputfolder, parent, sizeof(parent), &error) != MB_SUCCESS ||
      mbsvp_make_unique_dir(verbose, parent, "per-file", batch, sizeof(batch), &error) != MB_SUCCESS) {
    fprintf(stderr, "Per-file initialization failed: cannot make a directory in %s\n", options->outputfolder);
    return 1;
  }
  char created[32] = "";
  const time_t now = time(NULL);
  const struct tm *utc = gmtime(&now);
  if (utc != NULL)
    strftime(created, sizeof(created), "%Y-%m-%dT%H:%M:%S+00:00", utc);

  struct summary_row *rows = NULL;
  if (mb_mallocd(verbose, __FILE__, __LINE__, files->n * sizeof(struct summary_row), (void **)&rows, &error) != MB_SUCCESS) {
    fprintf(stderr, "Per-file initialization failed: out of memory\n");
    return 1;
  }
  write_summary(batch, created, options->apply_best, rows, 0, false);
  fprintf(stdout, "Per-file batch directory: %s\n", batch);
  fflush(stdout);

  int nrows = 0;
  bool any_failed = false;
  for (int i = 0; i < files->n; i++) {
    struct summary_row *row = &rows[i];
    memset(row, 0, sizeof(struct summary_row));
    snprintf(row->input, sizeof(row->input), "%s", files->items[i].path);
    mb_path stem;
    snprintf(stem, sizeof(stem), "%s", mbsvp_basename(files->items[i].path));
    char *suffix = (char *)mbsvp_suffix(stem);
    *suffix = '\0';
    const int len = snprintf(row->output_directory, sizeof(row->output_directory), "%s/%04d_%s", batch, i + 1, stem);
    if (len < 0 || (size_t)len >= sizeof(row->output_directory))
      snprintf(row->output_directory, sizeof(row->output_directory), "%s/%04d", batch, i + 1);
    strcpy(row->status, "running");
    nrows = i + 1;
    write_summary(batch, created, options->apply_best, rows, nrows, false);
    fprintf(stdout, "[%d/%d] Evaluating %s\n", i + 1, files->n, row->input);
    fflush(stdout);

    struct input_list single;
    single.n = 1;
    single.alloc = 1;
    single.items = &files->items[i];
    evaluate_group(options, &single, row->output_directory, &row->result);
    row->has_exit_code = true;
    if (row->result.exit_code == 0) {
      strcpy(row->status, "success");
    }
    else if (row->result.exit_code == 130) {
      strcpy(row->status, "interrupted");
    }
    else {
      strcpy(row->status, "failed");
      any_failed = true;
    }
    write_summary(batch, created, options->apply_best, rows, nrows, mbsvp_interrupted != 0);
    if (mbsvp_interrupted)
      break;
  }
  fprintf(stdout, "Per-file summary: %s/summary.json\n", batch);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&rows, &error);
  if (mbsvp_interrupted)
    return 130;
  return any_failed ? 1 : 0;
}
/*--------------------------------------------------------------------*/
/* Add values given as "a,b" and in the following arguments, as in "-P a b",
   up to the next option. */
static int collect_values(int verbose, struct mbsvp_strings *list, const char *value, int argc, char **argv, int *error) {
  for (;;) {
    mb_path text;
    snprintf(text, sizeof(text), "%s", value);
    char *saveptr = text;
    while (saveptr != NULL) {
      char *token = saveptr;
      char *comma = strchr(saveptr, ',');
      if (comma != NULL) {
        *comma = '\0';
        saveptr = comma + 1;
      }
      else {
        saveptr = NULL;
      }
      if (token[0] != '\0' && !mbsvp_strings_contains(list, token) &&
          mbsvp_strings_add(verbose, list, token, error) != MB_SUCCESS)
        return MB_FAILURE;
    }
    if (optind >= argc || argv[optind][0] == '-')
      return MB_SUCCESS;
    value = argv[optind++];
  }
}
/*--------------------------------------------------------------------*/
int main(int argc, char **argv) {
  struct options options;
  memset(&options, 0, sizeof(options));
  strcpy(options.input, "datalist.mb-1");
  strcpy(options.outputfolder, "svpprofiles");
  options.minimum_beams = 5;
  mbsvp_strings_init(&options.periods);
  mbsvp_strings_init(&options.resolutions);
  int error = MB_ERROR_NO_ERROR;

  static const struct option long_options[] = {{"apply-best", no_argument, NULL, 0},
                                               {"enddate", no_argument, NULL, 'E'},
                                               {"help", no_argument, NULL, 'H'},
                                               {"input", required_argument, NULL, 'I'},
                                               {"minimum-beams", required_argument, NULL, 0},
                                               {"nearest", no_argument, NULL, 'N'},
                                               {"outputfolder", required_argument, NULL, 'O'},
                                               {"period", required_argument, NULL, 'P'},
                                               {"plots", no_argument, NULL, 0},
                                               {"resolution", required_argument, NULL, 'R'},
                                               {"selection", required_argument, NULL, 0},
                                               {"svpfolder", required_argument, NULL, 'S'},
                                               {"swathfile", required_argument, NULL, 'I'},
                                               {"verbose", no_argument, NULL, 'V'},
                                               {NULL, 0, NULL, 0}};
  bool help = false;
  bool errflg = false;
  int c;
  int option_index;
  while ((c = getopt_long(argc, argv, "EeHhI:i:NnO:o:P:p:R:r:S:s:Vv", long_options, &option_index)) != -1) {
    switch (c) {
    case 0:
      if (strcmp("apply-best", long_options[option_index].name) == 0) {
        options.apply_best = true;
      }
      else if (strcmp("minimum-beams", long_options[option_index].name) == 0) {
        if (sscanf(optarg, "%d", &options.minimum_beams) != 1)
          errflg = true;
      }
      else if (strcmp("plots", long_options[option_index].name) == 0) {
        options.plots = true;
      }
      else if (strcmp("selection", long_options[option_index].name) == 0) {
        if (strcmp(optarg, "combined") == 0) {
          options.per_file = false;
        }
        else if (strcmp(optarg, "per-file") == 0) {
          options.per_file = true;
        }
        else {
          fprintf(stderr, "--selection must be combined or per-file\n");
          errflg = true;
        }
      }
      break;
    case 'E':
    case 'e':
      options.enddate = true;
      break;
    case 'H':
    case 'h':
      help = true;
      break;
    case 'I':
    case 'i':
      snprintf(options.input, sizeof(options.input), "%s", optarg);
      break;
    case 'N':
    case 'n':
      options.nearest = true;
      break;
    case 'O':
    case 'o':
      snprintf(options.outputfolder, sizeof(options.outputfolder), "%s", optarg);
      break;
    case 'P':
    case 'p':
      if (collect_values(options.verbose, &options.periods, optarg, argc, argv, &error) != MB_SUCCESS)
        exit(error);
      break;
    case 'R':
    case 'r':
      if (collect_values(options.verbose, &options.resolutions, optarg, argc, argv, &error) != MB_SUCCESS)
        exit(error);
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
  if (options.minimum_beams < 3) {
    fprintf(stderr, "--minimum-beams must be >=3\n");
    errflg = true;
  }
  if (options.svpfolder[0] == '\0') {
    fprintf(stderr, "--svpfolder {-S} is required\n");
    errflg = true;
  }
  for (int i = 0; i < options.resolutions.n; i++) {
    if (strcmp(options.resolutions.items[i], "01") != 0 && strcmp(options.resolutions.items[i], "04") != 0) {
      fprintf(stderr, "--resolution must be 01 or 04, not %s\n", options.resolutions.items[i]);
      errflg = true;
    }
  }
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  /* the period lists of the old wrappers, together */
  if (options.periods.n == 0) {
    static const char *defaults[3] = {"decav", "A5B7", "B5C2"};
    for (int i = 0; i < 3; i++)
      if (mbsvp_strings_add(verbose, &options.periods, defaults[i], &error) != MB_SUCCESS)
        exit(error);
  }
  if (options.resolutions.n == 0) {
    if (mbsvp_strings_add(verbose, &options.resolutions, "01", &error) != MB_SUCCESS ||
        mbsvp_strings_add(verbose, &options.resolutions, "04", &error) != MB_SUCCESS)
      exit(error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       input:          %s\n", options.input);
    fprintf(stderr, "dbg2       svpfolder:      %s\n", options.svpfolder);
    fprintf(stderr, "dbg2       outputfolder:   %s\n", options.outputfolder);
    fprintf(stderr, "dbg2       nearest:        %d\n", options.nearest);
    fprintf(stderr, "dbg2       enddate:        %d\n", options.enddate);
    fprintf(stderr, "dbg2       per_file:       %d\n", options.per_file);
    fprintf(stderr, "dbg2       plots:          %d\n", options.plots);
    fprintf(stderr, "dbg2       apply_best:     %d\n", options.apply_best);
    fprintf(stderr, "dbg2       minimum_beams:  %d\n", options.minimum_beams);
  }

  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  struct input_list files = {0, 0, NULL};
  char message[MBSVP_MESSAGE_SIZE] = "";
  int exit_code = 1;
  mb_path absolute;
  if (expand_inputs(verbose, options.input, &files, message, &error) != MB_SUCCESS) {
    fprintf(stderr, "%s\n", message);
  }
  else if (mbsvp_absolute_path(verbose, options.svpfolder, absolute, sizeof(absolute), &error) != MB_SUCCESS) {
    fprintf(stderr, "SVP directory does not exist: %s\n", options.svpfolder);
  }
  else {
    strcpy(options.svpfolder, absolute);
    if (options.per_file) {
      exit_code = evaluate_per_file(&options, &files);
    }
    else {
      static struct group_result result;
      if (mbsvp_make_dirs(verbose, options.outputfolder, &error) != MB_SUCCESS ||
          mbsvp_absolute_path(verbose, options.outputfolder, absolute, sizeof(absolute), &error) != MB_SUCCESS) {
        fprintf(stderr, "Cannot make output directory: %s\n", options.outputfolder);
      }
      else {
        evaluate_group(&options, &files, absolute, &result);
        exit_code = result.exit_code;
      }
    }
  }

  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_strings_free(verbose, &options.periods, &error_free);
  mbsvp_strings_free(verbose, &options.resolutions, &error_free);
  if (files.items != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&files.items, &error_free);
  if (verbose >= 4)
    mb_memory_list(verbose, &error_free);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2       exit code:      %d\n", exit_code);
  }
  exit(exit_code);
}
/*--------------------------------------------------------------------*/
