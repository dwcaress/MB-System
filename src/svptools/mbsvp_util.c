/*--------------------------------------------------------------------
 *    The MB-system:  mbsvp_util.c  3 November 2026
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
 * Strings, files, external commands and number formatting for the
 * sound velocity profile tools.
 */

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <process.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "mb_define.h"
#include "mb_status.h"
#include "mbsvp_tools.h"

volatile sig_atomic_t mbsvp_interrupted = 0;

/* Exit codes of a shell whose command was stopped by SIGINT or SIGTERM. */
#define MBSVP_SHELL_SIGINT 130
#define MBSVP_SHELL_SIGTERM 143

/*--------------------------------------------------------------------*/
void mbsvp_set_message(char *message, const char *format, ...) {
  if (message == NULL)
    return;
  va_list args;
  va_start(args, format);
  vsnprintf(message, MBSVP_MESSAGE_SIZE, format, args);
  va_end(args);
}
/*--------------------------------------------------------------------*/
void mbsvp_strings_init(struct mbsvp_strings *list) {
  list->n = 0;
  list->alloc = 0;
  list->items = NULL;
}
/*--------------------------------------------------------------------*/
int mbsvp_strings_add(int verbose, struct mbsvp_strings *list, const char *text, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       text:       %s\n", text);
  }

  if (list->n >= list->alloc) {
    const int alloc = list->alloc + 64;
    if (mb_reallocd(verbose, __FILE__, __LINE__, alloc * sizeof(char *), (void **)&list->items, error) != MB_SUCCESS) {
      list->n = 0;
      list->alloc = 0;
      return MB_FAILURE;
    }
    list->alloc = alloc;
  }
  char *copy = NULL;
  if (mb_mallocd(verbose, __FILE__, __LINE__, strlen(text) + 1, (void **)&copy, error) != MB_SUCCESS)
    return MB_FAILURE;
  strcpy(copy, text);
  list->items[list->n++] = copy;

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, n: %d\n", __func__, list->n);
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
bool mbsvp_strings_contains(const struct mbsvp_strings *list, const char *text) {
  for (int i = 0; i < list->n; i++)
    if (strcmp(list->items[i], text) == 0)
      return true;
  return false;
}
/*--------------------------------------------------------------------*/
int mbsvp_strings_free(int verbose, struct mbsvp_strings *list, int *error) {
  for (int i = 0; i < list->n; i++)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&list->items[i], error);
  if (list->items != NULL)
    mb_freed(verbose, __FILE__, __LINE__, (void **)&list->items, error);
  list->n = 0;
  list->alloc = 0;
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
bool mbsvp_is_file(const char *path) {
  struct stat status;
  return stat(path, &status) == 0 && (status.st_mode & S_IFMT) == S_IFREG;
}
/*--------------------------------------------------------------------*/
bool mbsvp_is_dir(const char *path) {
  struct stat status;
  return stat(path, &status) == 0 && (status.st_mode & S_IFMT) == S_IFDIR;
}
/*--------------------------------------------------------------------*/
const char *mbsvp_basename(const char *path) {
  const char *base = path;
  for (const char *c = path; *c != '\0'; c++) {
    if (*c == '/'
#ifdef _WIN32
        || *c == '\\'
#endif
    )
      base = c + 1;
  }
  return base;
}
/*--------------------------------------------------------------------*/
/* The last ".xxx" of the file name, or an empty string. A leading dot
   does not start a suffix. */
const char *mbsvp_suffix(const char *path) {
  const char *base = mbsvp_basename(path);
  const char *dot = strrchr(base, '.');
  if (dot == NULL || dot == base)
    return base + strlen(base);
  return dot;
}
/*--------------------------------------------------------------------*/
bool mbsvp_join(char *out, size_t size, const char *dir, const char *name) {
  const int len = snprintf(out, size, "%s/%s", dir, name);
  return len > 0 && (size_t)len < size;
}
/*--------------------------------------------------------------------*/
int mbsvp_absolute_path(int verbose, const char *path, char *out, size_t size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
  }

  int status = MB_SUCCESS;
#ifdef _WIN32
  if (_fullpath(out, path, size) == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  }
#else
  char *resolved = realpath(path, NULL);
  if (resolved == NULL || strlen(resolved) >= size) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  }
  else {
    strcpy(out, resolved);
  }
  free(resolved);
#endif

  if (verbose >= 2)
    fprintf(stderr, "dbg2  Function <%s> completed, status: %d\n", __func__, status);
  return status;
}
/*--------------------------------------------------------------------*/
static int mbsvp_mkdir(const char *path) {
#ifdef _WIN32
  return mkdir(path);
#else
  return mkdir(path, 0775);
#endif
}
/*--------------------------------------------------------------------*/
int mbsvp_make_dirs(int verbose, const char *path, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
  }

  mb_path copy;
  if (strlen(path) >= sizeof(copy) || path[0] == '\0') {
    *error = MB_ERROR_BAD_PARAMETER;
    return MB_FAILURE;
  }
  strcpy(copy, path);
  for (char *c = copy + 1; *c != '\0'; c++) {
    if (*c == '/'
#ifdef _WIN32
        || *c == '\\'
#endif
    ) {
      const char keep = *c;
      *c = '\0';
      if (mbsvp_mkdir(copy) != 0 && errno != EEXIST && !mbsvp_is_dir(copy)) {
        *error = MB_ERROR_OPEN_FAIL;
        return MB_FAILURE;
      }
      *c = keep;
    }
  }
  if (mbsvp_mkdir(copy) != 0 && !mbsvp_is_dir(copy)) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
int mbsvp_make_unique_dir(int verbose, const char *parent, const char *prefix, char *out, size_t size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       parent:     %s\n", parent);
    fprintf(stderr, "dbg2       prefix:     %s\n", prefix);
  }

  const time_t now = time(NULL);
  const struct tm *local = localtime(&now);
  char stamp[32] = "00000000-000000";
  if (local != NULL)
    strftime(stamp, sizeof(stamp), "%Y%m%d-%H%M%S", local);
  for (int attempt = 0; attempt < 10000; attempt++) {
    int len;
    if (attempt == 0)
      len = snprintf(out, size, "%s/%s-%s", parent, prefix, stamp);
    else
      len = snprintf(out, size, "%s/%s-%s-%d", parent, prefix, stamp, attempt);
    if (len < 0 || (size_t)len >= size) {
      *error = MB_ERROR_BAD_PARAMETER;
      return MB_FAILURE;
    }
    if (mbsvp_mkdir(out) == 0)
      return MB_SUCCESS;
    if (errno != EEXIST)
      break;
  }
  *error = MB_ERROR_OPEN_FAIL;
  return MB_FAILURE;
}
/*--------------------------------------------------------------------*/
int mbsvp_copy_file(int verbose, const char *source, const char *target, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       source:     %s\n", source);
    fprintf(stderr, "dbg2       target:     %s\n", target);
  }

  FILE *in = fopen(source, "rb");
  if (in == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  FILE *out = fopen(target, "wb");
  if (out == NULL) {
    fclose(in);
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  char buffer[65536];
  size_t nread;
  while ((nread = fread(buffer, 1, sizeof(buffer), in)) > 0) {
    if (fwrite(buffer, 1, nread, out) != nread) {
      *error = MB_ERROR_WRITE_FAIL;
      status = MB_FAILURE;
      break;
    }
  }
  if (ferror(in)) {
    *error = MB_ERROR_EOF;
    status = MB_FAILURE;
  }
  fclose(in);
  if (fclose(out) != 0) {
    *error = MB_ERROR_WRITE_FAIL;
    status = MB_FAILURE;
  }
  return status;
}
/*--------------------------------------------------------------------*/
int mbsvp_link_or_copy(int verbose, const char *source, const char *target, int *error) {
#ifndef _WIN32
  if (symlink(source, target) == 0)
    return MB_SUCCESS;
#endif
  return mbsvp_copy_file(verbose, source, target, error);
}
/*--------------------------------------------------------------------*/
int mbsvp_list_dir(int verbose, const char *path, struct mbsvp_strings *names, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       path:       %s\n", path);
  }

  DIR *dir = opendir(path);
  if (dir == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    return MB_FAILURE;
  }
  int status = MB_SUCCESS;
  const struct dirent *entry;
  while (status == MB_SUCCESS && (entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
      status = mbsvp_strings_add(verbose, names, entry->d_name, error);
  }
  closedir(dir);

  /* readdir order depends on the file system: sort by name */
  for (int i = 1; i < names->n; i++) {
    char *item = names->items[i];
    int j = i - 1;
    while (j >= 0 && strcmp(names->items[j], item) > 0) {
      names->items[j + 1] = names->items[j];
      j--;
    }
    names->items[j + 1] = item;
  }
  return status;
}
/*--------------------------------------------------------------------*/
/* Remove a directory made by this program and the plain files in it.
   Links are removed, not followed. */
int mbsvp_remove_dir(int verbose, const char *path, int *error) {
  struct mbsvp_strings names;
  mbsvp_strings_init(&names);
  int status = mbsvp_list_dir(verbose, path, &names, error);
  for (int i = 0; status == MB_SUCCESS && i < names.n; i++) {
    mb_path item;
    if (!mbsvp_join(item, sizeof(item), path, names.items[i]) || remove(item) != 0) {
      *error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
    }
  }
  if (status == MB_SUCCESS && rmdir(path) != 0) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
  }
  int error_free = MB_ERROR_NO_ERROR;
  mbsvp_strings_free(verbose, &names, &error_free);
  return status;
}
/*--------------------------------------------------------------------*/
int mbsvp_process_id(void) {
#ifdef _WIN32
  return (int)_getpid();
#else
  return (int)getpid();
#endif
}
/*--------------------------------------------------------------------*/
/* Give a finished temporary file its final name. Without overwrite an
   existing file is left alone and the temporary file is removed. */
int mbsvp_commit_file(int verbose, const char *temporary, const char *path, bool overwrite, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       temporary:  %s\n", temporary);
    fprintf(stderr, "dbg2       path:       %s\n", path);
    fprintf(stderr, "dbg2       overwrite:  %d\n", overwrite);
  }

  if (!overwrite) {
    const int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
    if (fd < 0) {
      remove(temporary);
      *error = MB_ERROR_OPEN_FAIL;
      return MB_FAILURE;
    }
    close(fd);
  }
#ifdef _WIN32
  remove(path);
#endif
  if (rename(temporary, path) != 0) {
    remove(temporary);
    *error = MB_ERROR_WRITE_FAIL;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Quote one argument for the shell that system() runs. */
char *mbsvp_quote(const char *text, char *out, size_t size) {
  size_t k = 0;
  bool fits = true;
#ifdef _WIN32
  const char *quote = "\"";
#else
  const char *quote = "'";
#endif
  if (k + 1 < size)
    out[k++] = quote[0];
  else
    fits = false;
  for (const char *c = text; fits && *c != '\0'; c++) {
#ifndef _WIN32
    if (*c == '\'') {
      if (k + 4 < size) {
        memcpy(&out[k], "'\\''", 4);
        k += 4;
      }
      else {
        fits = false;
      }
      continue;
    }
#endif
    if (k + 1 < size)
      out[k++] = *c;
    else
      fits = false;
  }
  if (fits && k + 2 <= size) {
    out[k++] = quote[0];
    out[k] = '\0';
    return out;
  }
  return NULL;
}
/*--------------------------------------------------------------------*/
int mbsvp_run_shell(int verbose, const char *command, int *exit_code, int *error) {
  if (verbose >= 1)
    fprintf(stderr, "Running: %s\n", command);
  fflush(stdout);
  fflush(stderr);

  const int result = system(command);
  if (result == -1) {
    *exit_code = -1;
    *error = MB_ERROR_OTHER;
    return MB_FAILURE;
  }
#ifdef _WIN32
  *exit_code = result;
#else
  if (WIFSIGNALED(result)) {
    const int signal_number = WTERMSIG(result);
    *exit_code = 128 + signal_number;
  }
  else {
    *exit_code = WEXITSTATUS(result);
  }
#endif
  if (*exit_code == MBSVP_SHELL_SIGINT || *exit_code == MBSVP_SHELL_SIGTERM)
    mbsvp_interrupted = 1;
  if (*exit_code != 0) {
    *error = MB_ERROR_OTHER;
    return MB_FAILURE;
  }
  return MB_SUCCESS;
}
/*--------------------------------------------------------------------*/
/* Run a program with its arguments. Standard output goes to stdout_file
   when one is given, and everything else is added to log_file. */
int mbsvp_run_command(int verbose, const char *const *argv, const char *stdout_file, const char *log_file,
                      int *exit_code, int *error) {
  char command[4 * MB_PATH_MAXLINE + 512];
  char quoted[2 * MB_PATH_MAXLINE];
  size_t len = 0;
  for (int i = 0; argv[i] != NULL; i++) {
    /* the program name is left unquoted, cmd.exe treats a leading quote specially */
    const char *text = argv[i];
    if (i > 0 && (text = mbsvp_quote(argv[i], quoted, sizeof(quoted))) == NULL) {
      *error = MB_ERROR_BAD_PARAMETER;
      *exit_code = -1;
      return MB_FAILURE;
    }
    const int added = snprintf(&command[len], sizeof(command) - len, "%s%s", i > 0 ? " " : "", text);
    if (added < 0 || (size_t)added >= sizeof(command) - len) {
      *error = MB_ERROR_BAD_PARAMETER;
      *exit_code = -1;
      return MB_FAILURE;
    }
    len += added;
  }
  if (stdout_file != NULL) {
    if (mbsvp_quote(stdout_file, quoted, sizeof(quoted)) == NULL)
      len = sizeof(command);
    else if (len < sizeof(command))
      len += snprintf(&command[len], sizeof(command) - len, " > %s", quoted);
  }
  if (log_file != NULL && len < sizeof(command)) {
    if (mbsvp_quote(log_file, quoted, sizeof(quoted)) == NULL)
      len = sizeof(command);
    else if (stdout_file != NULL)
      len += snprintf(&command[len], sizeof(command) - len, " 2>> %s", quoted);
    else
      len += snprintf(&command[len], sizeof(command) - len, " >> %s 2>&1", quoted);
  }
  if (len >= sizeof(command)) {
    *error = MB_ERROR_BAD_PARAMETER;
    *exit_code = -1;
    return MB_FAILURE;
  }
  return mbsvp_run_shell(verbose, command, exit_code, error);
}
/*--------------------------------------------------------------------*/
/* The shortest decimal text that reads back as the same double, written
   the way Python and JSON tools write it (93.5, 1550.0, 1.3e-13). */
void mbsvp_format_double(double value, char *out, size_t size) {
  if (!isfinite(value)) {
    snprintf(out, size, "null");
    return;
  }
  char text[64];
  int precision = 0;
  for (precision = 0; precision < 17; precision++) {
    snprintf(text, sizeof(text), "%.*e", precision, value);
    if (strtod(text, NULL) == value)
      break;
  }
  const char *exponent_text = strchr(text, 'e');
  const int exponent = exponent_text != NULL ? atoi(exponent_text + 1) : 0;
  if (exponent >= -4 && exponent < 16) {
    int decimals = precision - exponent;
    if (decimals < 0)
      decimals = 0;
    snprintf(out, size, "%.*f%s", decimals, value, decimals == 0 ? ".0" : "");
  }
  else {
    snprintf(out, size, "%s", text);
  }
}
/*--------------------------------------------------------------------*/
void mbsvp_json_escape(const char *text, char *out, size_t size) {
  size_t k = 0;
  for (const unsigned char *c = (const unsigned char *)text; *c != '\0' && k + 7 < size; c++) {
    if (*c == '"' || *c == '\\') {
      out[k++] = '\\';
      out[k++] = (char)*c;
    }
    else if (*c < 0x20) {
      k += snprintf(&out[k], size - k, "\\u%04x", *c);
    }
    else {
      out[k++] = (char)*c;
    }
  }
  out[k] = '\0';
}
/*--------------------------------------------------------------------*/
