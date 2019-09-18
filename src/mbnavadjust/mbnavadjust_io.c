/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_io.c  3/23/00
 *
 *    Copyright (c) 2014-2019 by
 *    David W. Caress (caress@mbari.org)
 *      Monterey Bay Aquarium Research Institute
 *      Moss Landing, CA 95039
 *    and Dale N. Chayes (dale@ldeo.columbia.edu)
 *      Lamont-Doherty Earth Observatory
 *      Palisades, NY 10964
 *
 *    See README file for copying and redistribution conditions.
 *--------------------------------------------------------------------*/
/*
 * mbnavadjust is an interactive navigation adjustment package
 * for swath sonar data.
 * It can work with any data format supported by the MBIO library.
 * This file contains the code that does not directly depend on the
 * MOTIF interface.
 *
 * Author:  D. W. Caress
 * Date:  April 14, 2014
 */

/*--------------------------------------------------------------------*/

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/stat.h>

/* MBIO include files */
#include "mb_status.h"
#include "mb_define.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_aux.h"
#include "mbsys_ldeoih.h"

/* define mbnavadjust io structures */
#include "mbnavadjust_io.h"

/* get NaN detector */
#if defined(isnanf)
#define check_fnan(x) isnanf((x))
#elif defined(isnan)
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNANF == 1
#define check_fnan(x) isnanf(x)
extern int isnanf(float x);
#elif HAVE_ISNAN == 1
#define check_fnan(x) isnan((double)(x))
#elif HAVE_ISNAND == 1
#define check_fnan(x) isnand((double)(x))
#else
#define check_fnan(x) ((x) != (x))
#endif

static char program_name[] = "mbnavadjust i/o functions";

/*--------------------------------------------------------------------*/
int mbnavadjust_new_project(int verbose, char *projectpath, double section_length, int section_soundings, double cont_int,
                            double col_int, double tick_int, double label_int, int decimation, double smoothing,
                            double zoffsetwidth, struct mbna_project *project, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  char *slashptr, *nameptr;
  char *result;
  struct stat statbuf;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       projectpath:        %s\n", projectpath);
    fprintf(stderr, "dbg2       section_length:     %f\n", section_length);
    fprintf(stderr, "dbg2       section_soundings:  %d\n", section_soundings);
    fprintf(stderr, "dbg2       cont_int:           %f\n", cont_int);
    fprintf(stderr, "dbg2       col_int:            %f\n", col_int);
    fprintf(stderr, "dbg2       tick_int:           %f\n", tick_int);
    fprintf(stderr, "dbg2       label_int:          %f\n", label_int);
    fprintf(stderr, "dbg2       decimation:         %d\n", decimation);
    fprintf(stderr, "dbg2       smoothing:          %f\n", smoothing);
    fprintf(stderr, "dbg2       zoffsetwidth:       %f\n", zoffsetwidth);
    fprintf(stderr, "dbg2       project:            %p\n", project);
  }

  /* if project structure holds an open project close it first */
  if (project->open == MB_YES)
    status = mbnavadjust_close_project(verbose, project, error);

  /* check path to see if new project can be created */
  nameptr = (char *)NULL;
  slashptr = strrchr(projectpath, '/');
  if (slashptr != (char *)NULL)
    nameptr = slashptr + 1;
  else
    nameptr = projectpath;
  if (strlen(nameptr) > 4 && strcmp(&nameptr[strlen(nameptr) - 4], ".nvh") == 0)
    nameptr[strlen(nameptr) - 4] = '\0';
  if (strlen(nameptr) == 0) {
    fprintf(stderr, "Unable to create new project!\nInvalid project path: %s\n", projectpath);
    *error = MB_ERROR_INIT_FAIL;
    status = MB_FAILURE;
  }

  /* try to create new project */
  if (status == MB_SUCCESS) {
    strcpy(project->name, nameptr);
    if (strlen(projectpath) == strlen(nameptr)) {
      result = getcwd(project->path, MB_PATH_MAXLINE);
      strcat(project->path, "/");
    }
    else {
      strncpy(project->path, projectpath, strlen(projectpath) - strlen(nameptr));
    }
    strcpy(project->home, project->path);
    strcat(project->home, project->name);
    strcat(project->home, ".nvh");
    strcpy(project->datadir, project->path);
    strcat(project->datadir, project->name);
    strcat(project->datadir, ".dir");
    strcpy(project->logfile, project->datadir);
    strcat(project->logfile, "/log.txt");

    /* no new project if file or directory already exist */
    if (stat(project->home, &statbuf) == 0) {
      fprintf(stderr, "Unable to create new project!\nHome file %s already exists\n", project->home);
      *error = MB_ERROR_INIT_FAIL;
      status = MB_FAILURE;
    }
    if (stat(project->datadir, &statbuf) == 0) {
      fprintf(stderr, "Unable to create new project!\nData directory %s already exists\n", project->datadir);
      *error = MB_ERROR_INIT_FAIL;
      status = MB_FAILURE;
    }

    /* initialize new project */
    if (status == MB_SUCCESS) {
      /* set values */
      project->open = MB_YES;
      project->num_files = 0;
      project->num_files_alloc = 0;
      project->files = NULL;
      project->num_snavs = 0;
      project->num_pings = 0;
      project->num_beams = 0;
      project->num_crossings = 0;
      project->num_crossings_alloc = 0;
      project->num_crossings_analyzed = 0;
      project->num_goodcrossings = 0;
      project->num_truecrossings = 0;
      project->num_truecrossings_analyzed = 0;
      project->crossings = NULL;
      project->num_ties = 0;
      project->section_length = section_length;
      project->section_soundings = section_soundings;
      project->cont_int = cont_int;
      project->col_int = col_int;
      project->tick_int = tick_int;
      project->label_int = label_int;
      project->decimation = decimation;
      project->precision = SIGMA_MINIMUM;
      project->smoothing = smoothing;
      project->zoffsetwidth = zoffsetwidth;
      project->inversion_status = MBNA_INVERSION_NONE;
      project->grid_status = MBNA_GRID_NONE;
      project->modelplot = MB_NO;
      project->modelplot_style = MBNA_MODELPLOT_TIMESERIES;
            project->modelplot_uptodate = MB_NO;
      project->logfp = NULL;
      project->precision = SIGMA_MINIMUM;
      project->smoothing = MBNA_SMOOTHING_DEFAULT;

      /* create data directory */
#ifdef _WIN32
      if (mkdir(project->datadir) != 0) {
#else
      if (mkdir(project->datadir, 00775) != 0) {
#endif
        fprintf(stderr, "Error creating data directory %s\n", project->datadir);
        *error = MB_ERROR_INIT_FAIL;
        status = MB_FAILURE;
      }

      /* write home file and other files */
      else if ((status = mbnavadjust_write_project(verbose, project, error)) == MB_FAILURE) {
        fprintf(stderr, "Failure to write project file %s\n", project->home);
        *error = MB_ERROR_INIT_FAIL;
        status = MB_FAILURE;
      }

      /* initialize log file */
      else if ((project->logfp = fopen(project->logfile, "w")) == NULL) {
        fprintf(stderr, "Failure to create log file %s\n", project->logfile);
        *error = MB_ERROR_INIT_FAIL;
        status = MB_FAILURE;
      }

      /* first message in log file */
      else {
        fprintf(project->logfp, "New project initialized: %s\n > Project home: %s\n", project->name, project->home);
      }
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_read_project(int verbose, char *projectpath, struct mbna_project *project, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  char *slashptr, *nameptr;
  struct stat statbuf;
  FILE *hfp;
  struct mbna_file *file;
  struct mbna_section *section, *section1, *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  char label[STRING_MAX];
  char buffer[BUFFER_MAX];
  char obuffer[BUFFER_MAX];
  char command[MB_PATH_MAXLINE];
  char *result;
  int versionmajor, versionminor, version_id;
  double dummy;
  int nscan, idummy, jdummy;
  int s1id, s2id;
  int shellstatus;
  int first;
  int i, j, k, l;
  double mtodeglon;
  double mtodeglat;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       projectname:        %s\n", projectpath);
    fprintf(stderr, "dbg2       project:            %p\n", project);
  }

  /* if project structure holds an open project close it first */
  if (project->open == MB_YES)
    status = mbnavadjust_close_project(verbose, project, error);

  /* check path to see if project exists */
  nameptr = (char *)NULL;
  slashptr = strrchr(projectpath, '/');
  if (slashptr != (char *)NULL)
    nameptr = slashptr + 1;
  else
    nameptr = projectpath;
  if (strlen(nameptr) > 4 && strcmp(&nameptr[strlen(nameptr) - 4], ".nvh") == 0)
    nameptr[strlen(nameptr) - 4] = '\0';
  if (strlen(nameptr) == 0) {
    fprintf(stderr, "Unable to read project!\nInvalid project path: %s\n", projectpath);
    *error = MB_ERROR_INIT_FAIL;
    status = MB_FAILURE;
  }

  /* try to read project */
  if (status == MB_SUCCESS) {
    strcpy(project->name, nameptr);
    if (strlen(projectpath) == strlen(nameptr)) {
      result = getcwd(project->path, MB_PATH_MAXLINE);
      strcat(project->path, "/");
    }
    else {
      strcpy(project->path, projectpath);
      project->path[strlen(projectpath) - strlen(nameptr)] = '\0';
    }
    strcpy(project->home, project->path);
    strcat(project->home, project->name);
    strcat(project->home, ".nvh");
    strcpy(project->datadir, project->path);
    strcat(project->datadir, project->name);
    strcat(project->datadir, ".dir");
    strcpy(project->logfile, project->datadir);
    strcat(project->logfile, "/log.txt");

    /* check if project exists */
    if (stat(project->home, &statbuf) != 0) {
      fprintf(stderr, "Project home file %s does not exist\n", project->home);
      *error = MB_ERROR_INIT_FAIL;
      status = MB_FAILURE;
    }
    if (stat(project->datadir, &statbuf) != 0) {
      fprintf(stderr, "Data directory %s does not exist\n", project->datadir);
      *error = MB_ERROR_INIT_FAIL;
      status = MB_FAILURE;
    }

    /* read the project */
    if (status == MB_SUCCESS) {
      /* first save copy of the project file */
      sprintf(command, "cp %s %s.save", project->home, project->home);
      shellstatus = system(command);

      /* open and read home file */
      status = MB_SUCCESS;
      if ((hfp = fopen(project->home, "r")) != NULL) {
        /* check for proper header */
        if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer || strncmp(buffer, "##MBNAVADJUST PROJECT", 21) != 0)
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        /* read basic names and stats */
        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "MB-SYSTEM_VERSION") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "PROGRAM_VERSION") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS && ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                                     (nscan = sscanf(buffer, "%s %d.%d", label, &versionmajor, &versionminor)) != 3 ||
                                     strcmp(label, "FILE_VERSION") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }
        version_id = 100 * versionmajor + versionminor;

        if (version_id >= 302) {
          if (status == MB_SUCCESS &&
              ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
               (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "ORIGIN") != 0))
            status = MB_FAILURE;
        }
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "NAME") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "PATH") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "HOME") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %s", label, obuffer)) != 2 || strcmp(label, "DATADIR") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %d", label, &project->num_files)) != 2 || strcmp(label, "NUMFILES") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (version_id >= 306) {
          if (status == MB_SUCCESS &&
              ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
               (nscan = sscanf(buffer, "%s %d", label, &project->num_blocks)) != 2 || strcmp(label, "NUMBLOCKS") != 0))
            status = MB_FAILURE;
        }
        else {
          project->num_blocks = 0;
        }
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS && ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                                     (nscan = sscanf(buffer, "%s %d", label, &project->num_crossings)) != 2 ||
                                     strcmp(label, "NUMCROSSINGS") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS && ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                                     (nscan = sscanf(buffer, "%s %lf", label, &project->section_length)) != 2 ||
                                     strcmp(label, "SECTIONLENGTH") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS && version_id >= 101 &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %d", label, &project->section_soundings)) != 2 ||
             strcmp(label, "SECTIONSOUNDINGS") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %d", label, &project->decimation)) != 2 || strcmp(label, "DECIMATION") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %lf", label, &project->cont_int)) != 2 || strcmp(label, "CONTOURINTERVAL") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %lf", label, &project->col_int)) != 2 || strcmp(label, "COLORINTERVAL") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS &&
            ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
             (nscan = sscanf(buffer, "%s %lf", label, &project->tick_int)) != 2 || strcmp(label, "TICKINTERVAL") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS && ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                                     (nscan = sscanf(buffer, "%s %d", label, &project->inversion_status)) != 2 ||
                                     strcmp(label, "INVERSION") != 0))
          status = MB_FAILURE;
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s buffer:%s\n", __LINE__, __FILE__, buffer);
          exit(0);
        }

        if (status == MB_SUCCESS) {
          if (version_id >= 307) {
            if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                (nscan = sscanf(buffer, "%s %d", label, &project->grid_status)) != 2 ||
                strcmp(label, "GRIDSTATUS") != 0)
              status = MB_FAILURE;
          }
        }
        if (status == MB_SUCCESS) {
          if (version_id >= 301) {
            if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                (nscan = sscanf(buffer, "%s %lf", label, &project->smoothing)) != 2 ||
                strcmp(label, "SMOOTHING") != 0)
              status = MB_FAILURE;
            project->precision = SIGMA_MINIMUM;
          }
          else if (version_id >= 103) {
            if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                (nscan = sscanf(buffer, "%s %lf", label, &project->precision)) != 2 ||
                strcmp(label, "PRECISION") != 0)
              status = MB_FAILURE;
            project->smoothing = MBNA_SMOOTHING_DEFAULT;
          }
          else {
            project->precision = SIGMA_MINIMUM;
            project->smoothing = MBNA_SMOOTHING_DEFAULT;
          }
        }
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
          exit(0);
        }

        if (status == MB_SUCCESS) {
          if (version_id >= 105) {
            if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                (nscan = sscanf(buffer, "%s %lf", label, &project->zoffsetwidth)) != 2 ||
                strcmp(label, "ZOFFSETWIDTH") != 0)
              status = MB_FAILURE;
          }
          else
            project->zoffsetwidth = 5.0;
        }

        /* allocate memory for files array */
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
          exit(0);
        }

        if (project->num_files > 0) {
          project->files = (struct mbna_file *)malloc(sizeof(struct mbna_file) * (project->num_files));
          if (project->files != NULL) {
            project->num_files_alloc = project->num_files;
            memset(project->files, 0, project->num_files_alloc * sizeof(struct mbna_file));
          }
          else {
            project->num_files_alloc = 0;
            status = MB_FAILURE;
            *error = MB_ERROR_MEMORY_FAIL;
          }
        }
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
          exit(0);
        }

        if (project->num_crossings > 0) {
          project->crossings = (struct mbna_crossing *)malloc(sizeof(struct mbna_crossing) * (project->num_crossings));
          if (project->crossings != NULL) {
            project->num_crossings_alloc = project->num_crossings;
            memset(project->crossings, 0, sizeof(struct mbna_crossing) * project->num_crossings_alloc);
          }
          else {
            project->num_crossings_alloc = 0;
            status = MB_FAILURE;
            *error = MB_ERROR_MEMORY_FAIL;
          }
        }
        if (status == MB_FAILURE) {
          fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
          exit(0);
        }

        for (i = 0; i < project->num_files; i++) {
          file = &project->files[i];
          file->num_sections_alloc = 0;
          file->sections = NULL;
          file->num_snavs = 0;
          file->num_pings = 0;
          file->num_beams = 0;
          if (version_id >= 306) {
            if (status == MB_SUCCESS &&
                ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                 (nscan = sscanf(buffer, "FILE %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %d %d %s", &idummy,
                                 &(file->status), &(file->id), &(file->format), &(file->block),
                                 &(file->block_offset_x), &(file->block_offset_y), &(file->block_offset_z),
                                 &(file->heading_bias_import), &(file->roll_bias_import), &(file->heading_bias),
                                 &(file->roll_bias), &(file->num_sections), &(file->output_id), file->file)) != 15))
              status = MB_FAILURE;
          }
          else {
            if (status == MB_SUCCESS &&
                ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                 (nscan = sscanf(buffer, "FILE %d %d %d %d %lf %lf %lf %lf %d %d %s", &idummy, &(file->status),
                                 &(file->id), &(file->format), &(file->heading_bias_import),
                                 &(file->roll_bias_import), &(file->heading_bias), &(file->roll_bias),
                                 &(file->num_sections), &(file->output_id), file->file)) != 11))
              status = MB_FAILURE;
            file->block = 0;
            file->block_offset_x = 0.0;
            file->block_offset_y = 0.0;
            file->block_offset_z = 0.0;
          }

          /* set file->path as absolute path
              - file->file may be a relative path */
          if (status == MB_SUCCESS) {
            if (file->file[0] == '/')
              strcpy(file->path, file->file);
            else {
              strcpy(file->path, project->path);
              strcat(file->path, file->file);
            }
          }

          /* reset output_id to 0 - this should no longer have any value but 0 */
          file->output_id = 0;

          /* read section info */
          if (file->num_sections > 0) {
            file->sections = (struct mbna_section *)malloc(sizeof(struct mbna_section) * (file->num_sections));
            if (file->sections != NULL) {
              file->num_sections_alloc = file->num_sections;
              memset(file->sections, 0, sizeof(struct mbna_section) * file->num_sections_alloc);
            }
            else {
              file->num_sections_alloc = 0;
              status = MB_FAILURE;
              *error = MB_ERROR_MEMORY_FAIL;
            }
          }
          for (j = 0; j < file->num_sections; j++) {
            section = &file->sections[j];
            if (status == MB_SUCCESS)
              result = fgets(buffer, BUFFER_MAX, hfp);
            if (status == MB_SUCCESS && result == buffer)
              nscan = sscanf(buffer, "SECTION %d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %d", &idummy,
                             &section->num_pings, &section->num_beams, &section->num_snav, &section->continuity,
                             &section->distance, &section->btime_d, &section->etime_d, &section->lonmin,
                             &section->lonmax, &section->latmin, &section->latmax, &section->depthmin,
                             &section->depthmax, &section->contoursuptodate);
            if (result != buffer || nscan < 14) {
              status = MB_FAILURE;
              fprintf(stderr, "read failed on section: %s\n", buffer);
            }
            if (nscan < 15)
              section->contoursuptodate = MB_NO;
            for (k = MBNA_MASK_DIM - 1; k >= 0; k--) {
              if (status == MB_SUCCESS)
                result = fgets(buffer, BUFFER_MAX, hfp);
              for (l = 0; l < MBNA_MASK_DIM; l++) {
                sscanf(&buffer[l], "%1d", &section->coverage[l + k * MBNA_MASK_DIM]);
              }
            }
            if (status == MB_FAILURE) {
              fprintf(stderr, "Die at line:%d file:%s\n", __LINE__, __FILE__);
              exit(0);
            }
            /*fprintf(stderr,"%s/nvs_%4.4d_%4.4d.mb71\n",
            project->datadir,file->id,j);
            for (k=MBNA_MASK_DIM-1;k>=0;k--)
            {
            for (l=0;l<MBNA_MASK_DIM;l++)
            {
            fprintf(stderr, "%1d", section->coverage[l + k * MBNA_MASK_DIM]);
            }
            fprintf(stderr, "\n");
            }*/
            for (k = 0; k < section->num_snav; k++) {
              if (status == MB_SUCCESS)
                result = fgets(buffer, BUFFER_MAX, hfp);
                            if (version_id >= 308) {
                                if (status == MB_SUCCESS && result == buffer)
                                    nscan = sscanf(buffer, "SNAV %d %d %lf %lf %lf %lf %lf %lf %lf %lf",
                                                   &idummy, &section->snav_id[k], &section->snav_distance[k], &section->snav_time_d[k],
                                                   &section->snav_lon[k], &section->snav_lat[k], &section->snav_sensordepth[k],
                                                   &section->snav_lon_offset[k], &section->snav_lat_offset[k],
                                                   &section->snav_z_offset[k]);
                                section->snav_num_ties[k] = 0;
                                if (result != buffer || nscan != 10) {
                                    status = MB_FAILURE;
                                    fprintf(stderr, "read failed on snav: %s\n", buffer);
                                }
                            }
                            else {
                                if (status == MB_SUCCESS && result == buffer)
                                    nscan = sscanf(buffer, "SNAV %d %d %lf %lf %lf %lf %lf %lf %lf", &idummy, &section->snav_id[k],
                                                   &section->snav_distance[k], &section->snav_time_d[k], &section->snav_lon[k],
                                                   &section->snav_lat[k], &section->snav_lon_offset[k], &section->snav_lat_offset[k],
                                                   &section->snav_z_offset[k]);
                                section->snav_num_ties[k] = 0;
                                section->snav_sensordepth[k] = 0.0;
                                if (result == buffer && nscan == 6) {
                                    section->snav_lon_offset[k] = 0.0;
                                    section->snav_lat_offset[k] = 0.0;
                                    section->snav_z_offset[k] = 0.0;
                                }
                                else if (result == buffer && nscan == 8) {
                                    section->snav_z_offset[k] = 0.0;
                                }
                                else if (result != buffer || nscan != 9) {
                                    status = MB_FAILURE;
                                    fprintf(stderr, "read failed on snav: %s\n", buffer);
                                }

                                /* reverse offset values if older values */
                                if (version_id < 300) {
                                    section->snav_lon_offset[k] *= -1.0;
                                    section->snav_lat_offset[k] *= -1.0;
                                    section->snav_z_offset[k] *= -1.0;
                                }
                            }
                        }

            /* global fixed frame tie, whether defined or not */
                        section->global_tie_status = MBNA_TIE_NONE;
                        section->global_tie_snav = MBNA_SELECT_NONE;
                        section->global_tie_inversion_status = MBNA_INVERSION_NONE;
                        section->offset_x = 0.0;
                        section->offset_y = 0.0;
                        section->offset_x_m = 0.0;
                        section->offset_y_m = 0.0;
                        section->offset_z_m = 0.0;
                        section->xsigma = 0.0;
                        section->ysigma = 0.0;
                        section->zsigma = 0.0;
                        section->inversion_offset_x = 0.0;
                        section->inversion_offset_y = 0.0;
                        section->inversion_offset_x_m = 0.0;
                        section->inversion_offset_y_m = 0.0;
                        section->inversion_offset_z_m = 0.0;
                        section->dx_m = 0.0;
                        section->dy_m = 0.0;
                        section->dz_m = 0.0;
                        section->sigma_m = 0.0;
                        section->dr1_m = 0.0;
                        section->dr2_m = 0.0;
                        section->dr3_m = 0.0;
                        section->rsigma_m = 0.0;

            if (version_id >= 309) {
              if (status == MB_SUCCESS)
                result = fgets(buffer, BUFFER_MAX, hfp);
              if (status == MB_SUCCESS && result == buffer)
                nscan = sscanf(buffer, "GLOBALTIE %d %d %d %lf %lf %lf %lf %lf %lf",
                                                &section->global_tie_status, &section->global_tie_snav,
                                                &section->global_tie_inversion_status,
                                                &section->offset_x, &section->offset_y, &section->offset_z_m,
                                                &section->xsigma, &section->ysigma, &section->zsigma);
            }
            else if (version_id >= 305) {
              if (status == MB_SUCCESS)
                result = fgets(buffer, BUFFER_MAX, hfp);
              if (status == MB_SUCCESS && result == buffer)
                nscan = sscanf(buffer, "GLOBALTIE %d %d %lf %lf %lf %lf %lf %lf",
                                                &section->global_tie_status, &section->global_tie_snav,
                                                &section->offset_x, &section->offset_y, &section->offset_z_m,
                                                &section->xsigma, &section->ysigma, &section->zsigma);
              if (section->global_tie_status != MBNA_TIE_NONE) {
                                section->global_tie_inversion_status = project->inversion_status;
              }
            }
            else if (version_id == 304) {
              if (status == MB_SUCCESS)
                result = fgets(buffer, BUFFER_MAX, hfp);
              if (status == MB_SUCCESS && result == buffer)
                nscan = sscanf(buffer, "GLOBALTIE %d %lf %lf %lf %lf %lf %lf",
                                                &section->global_tie_snav,
                                                &section->offset_x, &section->offset_y, &section->offset_z_m,
                                                &section->xsigma, &section->ysigma, &section->zsigma);
              if (section->global_tie_snav != MBNA_SELECT_NONE) {
                section->global_tie_status = MBNA_TIE_XYZ;
                                section->global_tie_inversion_status = project->inversion_status;
               }
            }
          }
        }

        /* set project bounds and scaling */
        first = MB_YES;
        for (i = 0; i < project->num_files; i++) {
          file = &project->files[i];
          if (file->status != MBNA_FILE_FIXEDNAV) {
            for (j = 0; j < file->num_sections; j++) {
              section = &file->sections[j];
              if (!(check_fnan(section->lonmin) || check_fnan(section->lonmax) || check_fnan(section->latmin) ||
                    check_fnan(section->latmax))) {
                if (first == MB_YES) {
                  project->lon_min = section->lonmin;
                  project->lon_max = section->lonmax;
                  project->lat_min = section->latmin;
                  project->lat_max = section->latmax;
                  first = MB_NO;
                }
                else {
                  project->lon_min = MIN(project->lon_min, section->lonmin);
                  project->lon_max = MAX(project->lon_max, section->lonmax);
                  project->lat_min = MIN(project->lat_min, section->latmin);
                  project->lat_max = MAX(project->lat_max, section->latmax);
                }
              }
            }
          }
          // fprintf(stderr, "PROJECT BOUNDS: file %d %s: %.7f %.7f    %.7f %.7f\n",
          // i, file->path, project->lon_min, project->lon_max, project->lat_min, project->lat_max);
        }
        mb_coor_scale(verbose, 0.5 * (project->lat_min + project->lat_max), &project->mtodeglon, &project->mtodeglat);

        /* add sensordepth values to snav lists if needed */
                if (version_id < 308) {
fprintf(stderr,"Project version %d previous to 3.08: Adding sensordepth values to section snav arrays...\n", version_id);
                    status = mbnavadjust_fix_section_sensordepth(verbose, project, error);
                }

        /* count the number of blocks */
        if (version_id < 306) {
          project->num_blocks = 0;
          for (i = 0; i < project->num_files; i++) {
            file = &project->files[i];
            if (i == 0 || file->sections[0].continuity == MB_NO) {
              project->num_blocks++;
            }
            file->block = project->num_blocks - 1;
            file->block_offset_x = 0.0;
            file->block_offset_y = 0.0;
            file->block_offset_z = 0.0;
          }
        }

                /* now do scaling of global ties since mtodeglon and mtodeglat are defined */
        for (i = 0; i < project->num_files; i++) {
          file = &project->files[i];
          for (j = 0; j < file->num_sections; j++) {
            section = &file->sections[j];
                        if (section->global_tie_status != MBNA_TIE_NONE) {
                            section->offset_x_m = section->offset_x / project->mtodeglon;
                            section->offset_y_m = section->offset_y / project->mtodeglat;
                            if (section->global_tie_inversion_status != MBNA_INVERSION_NONE) {
                                section->inversion_offset_x = section->snav_lon_offset[section->global_tie_snav];
                                section->inversion_offset_y = section->snav_lat_offset[section->global_tie_snav];
                                section->inversion_offset_x_m = section->snav_lon_offset[section->global_tie_snav] / project->mtodeglon;
                                section->inversion_offset_y_m = section->snav_lat_offset[section->global_tie_snav] / project->mtodeglat;
                                section->inversion_offset_z_m = section->snav_z_offset[section->global_tie_snav];
                                section->dx_m = section->offset_x_m - section->inversion_offset_x_m;
                                section->dy_m = section->offset_y_m - section->inversion_offset_y_m;
                                section->dz_m = section->offset_z_m - section->inversion_offset_z_m;
                                section->sigma_m = sqrt(section->dx_m * section->dx_m + section->dy_m * section->dy_m + section->dz_m * section->dz_m);
                                section->dr1_m = section->inversion_offset_x_m / section->xsigma;
                                section->dr2_m = section->inversion_offset_y_m / section->ysigma;
                                section->dr3_m = section->inversion_offset_z_m / section->zsigma;
                                section->rsigma_m = sqrt(section->dr1_m * section->dr1_m + section->dr2_m * section->dr2_m + section->dr3_m * section->dr3_m);
                            }
                        }
                    }
                }

        /* read crossings */
        project->num_crossings_analyzed = 0;
        project->num_goodcrossings = 0;
        project->num_truecrossings = 0;
        project->num_truecrossings_analyzed = 0;
        project->num_ties = 0;
        for (i = 0; i < project->num_crossings; i++) {
          /* read each crossing */
          crossing = &project->crossings[i];
          if (status == MB_SUCCESS && version_id >= 106) {
            if (status == MB_SUCCESS &&
                ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                 (nscan =
                      sscanf(buffer, "CROSSING %d %d %d %d %d %d %d %d %d", &idummy, &crossing->status,
                             &crossing->truecrossing, &crossing->overlap, &crossing->file_id_1, &crossing->section_1,
                             &crossing->file_id_2, &crossing->section_2, &crossing->num_ties)) != 9)) {
              status = MB_FAILURE;
              fprintf(stderr, "read failed on crossing: %s\n", buffer);
            }
          }
          else if (status == MB_SUCCESS && version_id >= 102) {
            crossing->overlap = 0;
            if (status == MB_SUCCESS &&
                ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                 (nscan = sscanf(buffer, "CROSSING %d %d %d %d %d %d %d %d", &idummy, &crossing->status,
                                 &crossing->truecrossing, &crossing->file_id_1, &crossing->section_1,
                                 &crossing->file_id_2, &crossing->section_2, &crossing->num_ties)) != 8)) {
              status = MB_FAILURE;
              fprintf(stderr, "read failed on crossing: %s\n", buffer);
            }
          }
          else if (status == MB_SUCCESS) {
            crossing->truecrossing = MB_NO;
            crossing->overlap = 0;
            if (status == MB_SUCCESS &&
                ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                 (nscan = sscanf(buffer, "CROSSING %d %d %d %d %d %d %d", &idummy, &crossing->status,
                                 &crossing->file_id_1, &crossing->section_1, &crossing->file_id_2,
                                 &crossing->section_2, &crossing->num_ties)) != 7)) {
              status = MB_FAILURE;
              fprintf(stderr, "read failed on old format crossing: %s\n", buffer);
            }
          }
          if (status == MB_SUCCESS && crossing->status != MBNA_CROSSING_STATUS_NONE)
            project->num_crossings_analyzed++;
          if (status == MB_SUCCESS && crossing->truecrossing == MB_YES) {
            project->num_truecrossings++;
            if (crossing->status != MBNA_CROSSING_STATUS_NONE)
              project->num_truecrossings_analyzed++;
          }

          /* reorder crossing to be early file first older file second if
                  file version prior to 3.00 */
          if (version_id < 300) {
            idummy = crossing->file_id_1;
            jdummy = crossing->section_1;
            crossing->file_id_1 = crossing->file_id_2;
            crossing->section_1 = crossing->section_2;
            crossing->file_id_2 = idummy;
            crossing->section_2 = jdummy;
          }

          /* read ties */
          if (status == MB_SUCCESS) {
            for (j = 0; j < crossing->num_ties; j++) {
              /* read each tie */
              tie = &crossing->ties[j];
              if (status == MB_SUCCESS && version_id >= 302) {
                if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                    (nscan = sscanf(buffer, "TIE %d %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf", &idummy,
                                    &tie->status, &tie->snav_1, &tie->snav_1_time_d, &tie->snav_2,
                                    &tie->snav_2_time_d, &tie->offset_x, &tie->offset_y, &tie->offset_z_m,
                                    &tie->inversion_status, &tie->inversion_offset_x, &tie->inversion_offset_y,
                                    &tie->inversion_offset_z_m)) != 13) {
                  status = MB_FAILURE;
                  fprintf(stderr, "read failed on tie: %s\n", buffer);
                }
              }
              else if (status == MB_SUCCESS && version_id >= 104) {
                if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                    (nscan = sscanf(buffer, "TIE %d %d %lf %d %lf %lf %lf %lf %d %lf %lf %lf", &idummy,
                                    &tie->snav_1, &tie->snav_1_time_d, &tie->snav_2, &tie->snav_2_time_d,
                                    &tie->offset_x, &tie->offset_y, &tie->offset_z_m, &tie->inversion_status,
                                    &tie->inversion_offset_x, &tie->inversion_offset_y,
                                    &tie->inversion_offset_z_m)) != 12) {
                  status = MB_FAILURE;
                  fprintf(stderr, "read failed on tie: %s\n", buffer);
                }
                tie->status = MBNA_TIE_XYZ;
              }
              else if (status == MB_SUCCESS) {
                if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                    (nscan = sscanf(buffer, "TIE %d %d %lf %d %lf %lf %lf %d %lf %lf", &idummy, &tie->snav_1,
                                    &tie->snav_1_time_d, &tie->snav_2, &tie->snav_2_time_d, &tie->offset_x,
                                    &tie->offset_y, &tie->inversion_status, &tie->inversion_offset_x,
                                    &tie->inversion_offset_y)) != 10) {
                  status = MB_FAILURE;
                  fprintf(stderr, "read failed on tie: %s\n", buffer);
                }
                tie->status = MBNA_TIE_XYZ;
                tie->offset_z_m = 0.0;
                tie->inversion_offset_z_m = 0.0;
              }

              /* check for outrageous inversion_offset values */
              if (fabs(tie->inversion_offset_x) > 10000.0 || fabs(tie->inversion_offset_y) > 10000.0 ||
                  fabs(tie->inversion_offset_x_m) > 10000.0 || fabs(tie->inversion_offset_y_m) > 10000.0 ||
                  fabs(tie->inversion_offset_z_m) > 10000.0) {
                tie->inversion_status = MBNA_INVERSION_OLD;
                tie->inversion_offset_x = 0.0;
                tie->inversion_offset_y = 0.0;
                tie->inversion_offset_x_m = 0.0;
                tie->inversion_offset_y_m = 0.0;
                tie->inversion_offset_z_m = 0.0;
              }

              /* reorder crossing to be early file first older file second if
                      file version prior to 3.00 */
              if (version_id < 300) {
                idummy = tie->snav_1;
                dummy = tie->snav_1_time_d;
                tie->snav_1 = tie->snav_2;
                tie->snav_1_time_d = tie->snav_2_time_d;
                tie->snav_2 = idummy;
                tie->snav_2_time_d = dummy;
                /*                          tie->offset_x *= -1.0;
                                    tie->offset_y *= -1.0;
                                    tie->offset_z_m *= -1.0;
                                    tie->inversion_offset_x *= -1.0;
                                    tie->inversion_offset_y *= -1.0;
                                    tie->inversion_offset_z_m *= -1.0;*/
              }

              /* for version 2.0 or later read covariance */
              if (status == MB_SUCCESS && version_id >= 200) {
                if ((result = fgets(buffer, BUFFER_MAX, hfp)) != buffer ||
                    (nscan = sscanf(buffer, "COV %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &tie->sigmar1,
                                    &(tie->sigmax1[0]), &(tie->sigmax1[1]), &(tie->sigmax1[2]), &tie->sigmar2,
                                    &(tie->sigmax2[0]), &(tie->sigmax2[1]), &(tie->sigmax2[2]), &tie->sigmar3,
                                    &(tie->sigmax3[0]), &(tie->sigmax3[1]), &(tie->sigmax3[2]))) != 12) {
                  status = MB_FAILURE;
                  fprintf(stderr, "read failed on tie covariance: %s\n", buffer);
                }
                if (tie->sigmar1 <= MBNA_SMALL) {
                  tie->sigmar3 = MBNA_SMALL;
                }
                if (tie->sigmar2 <= MBNA_SMALL) {
                  tie->sigmar3 = MBNA_SMALL;
                }
                if (tie->sigmar3 <= MBNA_ZSMALL) {
                  tie->sigmar3 = MBNA_ZSMALL;
                }
              }
              else if (status == MB_SUCCESS) {
                tie->sigmar1 = 100.0;
                tie->sigmax1[0] = 1.0;
                tie->sigmax1[1] = 0.0;
                tie->sigmax1[2] = 0.0;
                tie->sigmar2 = 100.0;
                tie->sigmax2[0] = 0.0;
                tie->sigmax2[1] = 1.0;
                tie->sigmax2[2] = 0.0;
                tie->sigmar3 = 100.0;
                tie->sigmax3[0] = 0.0;
                tie->sigmax3[1] = 0.0;
                tie->sigmax3[2] = 1.0;
              }

              /* update number of ties */
              if (status == MB_SUCCESS) {
                project->num_ties++;
              }

              /* check for reasonable snav id's */
              if (status == MB_SUCCESS) {
                file = &project->files[crossing->file_id_1];
                section = &file->sections[crossing->section_1];
                if (tie->snav_1 >= section->num_snav) {
                  fprintf(stderr, "Crossing %4d:%4d %4d:%4d Reset tie snav_1 on read from %d to ",
                          crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2,
                          tie->snav_1);
                  tie->snav_1 = ((double)tie->snav_1 / (double)section->num_pings) * (MBNA_SNAV_NUM - 1);
                  tie->snav_1_time_d = section->snav_time_d[tie->snav_1];
                  fprintf(stderr, "%d because numsnav=%d\n", tie->snav_1, section->num_snav);
                }
                file = &project->files[crossing->file_id_2];
                section = &file->sections[crossing->section_2];
                if (tie->snav_2 >= section->num_snav) {
                  fprintf(stderr, "Crossing  %4d:%4d %4d:%4d  Reset tie snav_2 on read from %d to ",
                          crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2,
                          tie->snav_2);
                  tie->snav_2 = ((double)tie->snav_2 / (double)section->num_pings) * (MBNA_SNAV_NUM - 1);
                  tie->snav_2_time_d = section->snav_time_d[tie->snav_2];
                  fprintf(stderr, "%d because numsnav=%d\n", tie->snav_2, section->num_snav);
                }
              }

              /* update number of ties for snavs */
              if (status == MB_SUCCESS) {
                file = &project->files[crossing->file_id_1];
                section = &file->sections[crossing->section_1];
                section->snav_num_ties[tie->snav_1]++;
                file = &project->files[crossing->file_id_2];
                section = &file->sections[crossing->section_2];
                section->snav_num_ties[tie->snav_2]++;
              }

              /* calculate offsets in local meters */
              if (status == MB_SUCCESS) {
                tie->offset_x_m = tie->offset_x / project->mtodeglon;
                tie->offset_y_m = tie->offset_y / project->mtodeglat;
                                tie->inversion_offset_x_m = tie->inversion_offset_x / project->mtodeglon;
                                tie->inversion_offset_y_m = tie->inversion_offset_y / project->mtodeglat;
                                tie->dx_m = tie->offset_x_m - tie->inversion_offset_x_m;
                                tie->dy_m = tie->offset_y_m - tie->inversion_offset_y_m;
                                tie->dz_m = tie->offset_z_m - tie->inversion_offset_z_m;
                                tie->sigma_m = sqrt(tie->dx_m * tie->dx_m + tie->dy_m * tie->dy_m + tie->dz_m * tie->dz_m);
                                tie->dr1_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax1[0] +
                                               (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax1[1] +
                                               (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax1[2]) /
                                          tie->sigmar1;
                                tie->dr2_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax2[0] +
                                               (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax2[1] +
                                               (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax2[2]) /
                                          tie->sigmar2;
                                tie->dr3_m = fabs((tie->inversion_offset_x_m - tie->offset_x_m) * tie->sigmax3[0] +
                                               (tie->inversion_offset_y_m - tie->offset_y_m) * tie->sigmax3[1] +
                                               (tie->inversion_offset_z_m - tie->offset_z_m) * tie->sigmax3[2]) /
                                          tie->sigmar3;
                                tie->rsigma_m = sqrt(tie->dr1_m * tie->dr1_m + tie->dr2_m * tie->dr2_m + tie->dr3_m * tie->dr3_m);
              }
            }
          }

          /* finally make sure crossing has later section second, switch if needed */
          s1id = crossing->file_id_1 * 1000 + crossing->section_1;
          s2id = crossing->file_id_2 * 1000 + crossing->section_2;
          if (s2id < s1id) {
            idummy = crossing->file_id_1;
            jdummy = crossing->section_1;
            crossing->file_id_1 = crossing->file_id_2;
            crossing->section_1 = crossing->section_2;
            crossing->file_id_2 = idummy;
            crossing->section_2 = jdummy;
            for (j = 0; j < crossing->num_ties; j++) {
              tie = &crossing->ties[j];
              idummy = tie->snav_1;
              dummy = tie->snav_1_time_d;
              tie->snav_1 = tie->snav_2;
              tie->snav_1_time_d = tie->snav_2_time_d;
              tie->snav_2 = idummy;
              tie->snav_2_time_d = dummy;
              tie->offset_x *= -1.0;
              tie->offset_y *= -1.0;
              tie->offset_x_m *= -1.0;
              tie->offset_y_m *= -1.0;
              tie->offset_z_m *= -1.0;
              tie->inversion_offset_x *= -1.0;
              tie->inversion_offset_y *= -1.0;
              tie->inversion_offset_x_m *= -1.0;
              tie->inversion_offset_y_m *= -1.0;
              tie->inversion_offset_z_m *= -1.0;
                            tie->dx_m *= -1.0;
                            tie->dy_m *= -1.0;
                            tie->dz_m *= -1.0;
                            //tie->sigma_m;
                            tie->dr1_m *= -1.0;
                            tie->dr2_m *= -1.0;
                            tie->dr3_m *= -1.0;
                            //tie->rsigma_m;
            }
          }

          /* and even more finally, reset the snav times for the ties */
          for (j = 0; j < crossing->num_ties; j++) {
            tie = &crossing->ties[j];
            section1 = &(project->files[crossing->file_id_1].sections[crossing->section_1]);
            section2 = &(project->files[crossing->file_id_2].sections[crossing->section_2]);
            tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
            tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
          }
        }

        /* close home file */
        fclose(hfp);

        /* set project status flag */
        if (status == MB_SUCCESS)
          project->open = MB_YES;
        else {
          for (i = 0; i < project->num_files; i++) {
            file = &project->files[i];
            if (file->sections != NULL)
              free(file->sections);
          }
          if (project->files != NULL)
            free(project->files);
          if (project->crossings != NULL)
            free(project->crossings);
          project->open = MB_NO;
          memset(project->name, 0, STRING_MAX);
          strcpy(project->name, "None");
          memset(project->path, 0, STRING_MAX);
          memset(project->datadir, 0, STRING_MAX);
          project->num_files = 0;
          project->num_files_alloc = 0;
          project->num_snavs = 0;
          project->num_pings = 0;
          project->num_beams = 0;
          project->num_crossings = 0;
          project->num_crossings_alloc = 0;
          project->num_crossings_analyzed = 0;
          project->num_goodcrossings = 0;
          project->num_truecrossings = 0;
          project->num_truecrossings_analyzed = 0;
          project->num_ties = 0;
        }

        /* recalculate crossing overlap values if not already set */
        if (project->open == MB_YES) {
          for (i = 0; i < project->num_crossings; i++) {
            crossing = &(project->crossings[i]);
            if (crossing->overlap <= 0) {
              mbnavadjust_crossing_overlap(verbose, project, i, error);
            }
            if (crossing->overlap >= 25)
              project->num_goodcrossings++;
          }
        }
      }

      /* else set error */
      else {
        status = MB_FAILURE;
      }
    }

    /* open log file */
    if ((project->logfp = fopen(project->logfile, "a")) != NULL) {
      fprintf(project->logfp,
              "Project opened: %s\n > Project home: %s\n > Number of Files: %d\n > Number of Crossings Found: %d\n > "
              "Number of Crossings Analyzed: %d\n > Number of Navigation Ties: %d\n",
              project->name, project->home, project->num_files, project->num_crossings, project->num_crossings_analyzed,
              project->num_ties);
    }
    else {
      fprintf(stderr, "Failure to open log file %s\n", project->logfile);
      *error = MB_ERROR_INIT_FAIL;
      status = MB_FAILURE;
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_close_project(int verbose, struct mbna_project *project, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_file *file;
  int i;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       project:            %p\n", project);
  }

  /* add info text */
  fprintf(project->logfp, "Project closed: %s\n", project->name);
  fprintf(project->logfp, "Log file %s/log.txt closed\n", project->datadir);

  /* deallocate memory and reset values */
  for (i = 0; i < project->num_files; i++) {
    file = &project->files[i];
    if (file->sections != NULL)
      mb_freed(verbose, __FILE__, __LINE__, (void **)&file->sections, error);
  }
  if (project->files != NULL) {
    free(project->files);
    project->files = NULL;
    project->num_files_alloc = 0;
  }
  if (project->crossings != NULL) {
    free(project->crossings);
    project->crossings = NULL;
    project->num_crossings_alloc = 0;
  }
  if (project->logfp != NULL) {
    fclose(project->logfp);
    project->logfp = NULL;
  }

  /* reset values */
  project->open = MB_NO;
  memset(project->name, 0, STRING_MAX);
  strcpy(project->name, "None");
  memset(project->path, 0, STRING_MAX);
  memset(project->datadir, 0, STRING_MAX);
  memset(project->logfile, 0, STRING_MAX);
  project->num_files = 0;
  project->num_snavs = 0;
  project->num_pings = 0;
  project->num_beams = 0;
  project->num_crossings = 0;
  project->num_crossings_analyzed = 0;
  project->num_goodcrossings = 0;
  project->num_truecrossings = 0;
  project->num_truecrossings_analyzed = 0;
  project->num_ties = 0;
  project->inversion_status = MBNA_INVERSION_NONE;
  project->grid_status = MBNA_GRID_NONE;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_write_project(int verbose, struct mbna_project *project, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  FILE *hfp;
  struct mbna_file *file, *file_1, *file_2;
  struct mbna_section *section, *section_1, *section_2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  char datalist[STRING_MAX];
  char routefile[STRING_MAX];
  char routename[STRING_MAX];
  char offsetfile[STRING_MAX];
  double navlon1, navlon2, navlat1, navlat2;
    int time_i[7];
  int nroute;
  int snav_1, snav_2;
  int ncrossings_true = 0;
  int ncrossings_gt50 = 0;
  int ncrossings_gt25 = 0;
  int ncrossings_lt25 = 0;
  int ncrossings_fixed = 0;
  int nties_unfixed = 0;
  int nties_fixed = 0;
  char status_char, truecrossing_char;
  int routecolor = 1;
  char *unknown = "Unknown";
  double mtodeglon, mtodeglat;

  /* time, user, host variables */
  time_t right_now;
  char date[32], user[MB_PATH_MAXLINE], *user_ptr, host[MB_PATH_MAXLINE];

  int i, j, k, l;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       verbose:            %d\n", verbose);
    fprintf(stderr, "dbg2       project:            %p\n", project);
  }

  /* open and write home file */
  if ((hfp = fopen(project->home, "w")) != NULL) {
    fprintf(stderr, "Writing project %s (file version 3.09)\n", project->name);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      user_ptr = getenv("LOGNAME");
    if (user_ptr != NULL)
      strcpy(user, user_ptr);
    else
      strcpy(user, "unknown");
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "##MBNAVADJUST PROJECT\n");
    fprintf(hfp, "MB-SYSTEM_VERSION\t%s\n", MB_VERSION);
    fprintf(hfp, "PROGRAM_VERSION\t3.09\n");
    fprintf(hfp, "FILE_VERSION\t3.09\n");
    fprintf(hfp, "ORIGIN\tGenerated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
    fprintf(hfp, "NAME\t%s\n", project->name);
    fprintf(hfp, "PATH\t%s\n", project->path);
    fprintf(hfp, "HOME\t%s\n", project->home);
    fprintf(hfp, "DATADIR\t%s\n", project->datadir);
    fprintf(hfp, "NUMFILES\t%d\n", project->num_files);
    fprintf(hfp, "NUMBLOCKS\t%d\n", project->num_blocks);
    fprintf(hfp, "NUMCROSSINGS\t%d\n", project->num_crossings);
    fprintf(hfp, "SECTIONLENGTH\t%f\n", project->section_length);
    fprintf(hfp, "SECTIONSOUNDINGS\t%d\n", project->section_soundings);
    fprintf(hfp, "DECIMATION\t%d\n", project->decimation);
    fprintf(hfp, "CONTOURINTERVAL\t%f\n", project->cont_int);
    fprintf(hfp, "COLORINTERVAL\t%f\n", project->col_int);
    fprintf(hfp, "TICKINTERVAL\t%f\n", project->tick_int);
    fprintf(hfp, "INVERSION\t%d\n", project->inversion_status);
    fprintf(hfp, "GRIDSTATUS\t%d\n", project->grid_status);
    fprintf(hfp, "SMOOTHING\t%f\n", project->smoothing);
    fprintf(hfp, "ZOFFSETWIDTH\t%f\n", project->zoffsetwidth);
    for (i = 0; i < project->num_files; i++) {
      /* write out basic file info */
      file = &project->files[i];
      fprintf(hfp, "FILE %4d %4d %4d %4d %4d %13.8f %13.8f %13.8f %4.1f %4.1f %4.1f %4.1f %4d %4d %s\n", i, file->status,
              file->id, file->format, file->block, file->block_offset_x, file->block_offset_y, file->block_offset_z,
              file->heading_bias_import, file->roll_bias_import, file->heading_bias, file->roll_bias, file->num_sections,
              file->output_id, file->file);

      /* write out section info */
      for (j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        fprintf(hfp, "SECTION %4d %5d %5d %d %d %10.6f %16.6f %16.6f %13.8f %13.8f %13.8f %13.8f %9.3f %9.3f %d\n", j,
                section->num_pings, section->num_beams, section->num_snav, section->continuity, section->distance,
                section->btime_d, section->etime_d, section->lonmin, section->lonmax, section->latmin, section->latmax,
                section->depthmin, section->depthmax, section->contoursuptodate);
        for (k = MBNA_MASK_DIM - 1; k >= 0; k--) {
          for (l = 0; l < MBNA_MASK_DIM; l++) {
            fprintf(hfp, "%1d", section->coverage[l + k * MBNA_MASK_DIM]);
          }
          fprintf(hfp, "\n");
        }
        for (k = 0; k < section->num_snav; k++) {
          fprintf(hfp, "SNAV %4d %5d %10.6f %16.6f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
                            k, section->snav_id[k], section->snav_distance[k], section->snav_time_d[k],
                            section->snav_lon[k], section->snav_lat[k], section->snav_sensordepth[k],
                  section->snav_lon_offset[k], section->snav_lat_offset[k], section->snav_z_offset[k]);
        }
        fprintf(hfp, "GLOBALTIE %2d %4d %2d %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
                        section->global_tie_status, section->global_tie_snav, section->global_tie_inversion_status,
                        section->offset_x, section->offset_y, section->offset_z_m,
                        section->xsigma, section->ysigma, section->zsigma);
      }
    }

    /* write out crossing info */
    for (i = 0; i < project->num_crossings; i++) {
      /* write out basic crossing info */
      crossing = &project->crossings[i];
      fprintf(hfp, "CROSSING %5d %d %d %3d %5d %3d %5d %3d %2d\n", i, crossing->status, crossing->truecrossing,
              crossing->overlap, crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2,
              crossing->num_ties);

      /* write out tie info */
      for (j = 0; j < crossing->num_ties; j++) {
        /* write out basic tie info */
        tie = &crossing->ties[j];
        fprintf(hfp, "TIE %5d %1d %5d %16.6f %5d %16.6f %13.8f %13.8f %13.8f %1.1d %13.8f %13.8f %13.8f\n", j,
                tie->status, tie->snav_1, tie->snav_1_time_d, tie->snav_2, tie->snav_2_time_d, tie->offset_x,
                tie->offset_y, tie->offset_z_m, tie->inversion_status, tie->inversion_offset_x, tie->inversion_offset_y,
                tie->inversion_offset_z_m);
        fprintf(hfp, "COV %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f %13.8f\n",
                tie->sigmar1, tie->sigmax1[0], tie->sigmax1[1], tie->sigmax1[2], tie->sigmar2, tie->sigmax2[0],
                tie->sigmax2[1], tie->sigmax2[2], tie->sigmar3, tie->sigmax3[0], tie->sigmax3[1], tie->sigmax3[2]);
      }
    }

    /* close home file */
    fclose(hfp);
    status = MB_SUCCESS;
  }

  /* else set error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_WRITE_FAIL;
    fprintf(stderr, "Unable to update project %s\n > Home file: %s\n", project->name, project->home);
  }

  /* open and write datalist file */
  sprintf(datalist, "%s%s.mb-1", project->path, project->name);
  if ((hfp = fopen(datalist, "w")) != NULL) {
    for (i = 0; i < project->num_files; i++) {
      /* write file entry */
      file = &project->files[i];
      fprintf(hfp, "%s %d\n", file->file, file->format);
    }
    fclose(hfp);
  }

  /* else set error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, "Unable to update project %s\n > Datalist file: %s\n", project->name, datalist);
  }

  /* write mbgrdviz route files in which each tie point or crossing is a two point route
      consisting of the connected snav points
      - output several different route files
      - route files of ties (fixed and unfixed separate) represent each tie as a
          two point route consisting of the connected snav points
      - route files of crossings (<25%, >= 25% && < 50%, >= 50%, true crossings)
          represent each crossing as a two point route consisting of the central
          snav points for each of the two sections.
      - first count different types of crossings and ties to output as routes
      - then output each time of route file */
  ncrossings_true = 0;
  ncrossings_gt50 = 0;
  ncrossings_gt25 = 0;
  ncrossings_lt25 = 0;
  ncrossings_fixed = 0;
  nties_unfixed = 0;
  nties_fixed = 0;
  for (i = 0; i < project->num_crossings; i++) {
    crossing = &project->crossings[i];

    /* check all crossings */
    if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
        project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
      ncrossings_fixed++;
    else {
      if (crossing->truecrossing == MB_YES)
        ncrossings_true++;
      else if (crossing->overlap >= 50)
        ncrossings_gt50++;
      else if (crossing->overlap >= 25)
        ncrossings_gt25++;
      else
        ncrossings_lt25++;
    }

    /* check ties */
    if (crossing->status == MBNA_CROSSING_STATUS_SET) {
      if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
          project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)
        nties_fixed += crossing->num_ties;
      else
        nties_unfixed += crossing->num_ties;
    }
  }

  /* write mbgrdviz route file for all unfixed true crossings */
  sprintf(routefile, "%s%s_truecrossing.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", ncrossings_true);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only unfixed true crossings */
      if (crossing->truecrossing == MB_YES && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
                                                project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
        file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
        section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
        section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
        snav_1 = section_1->num_snav / 2;
        snav_2 = section_2->num_snav / 2;
        navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
        navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
        navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
        navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          status_char = 'U';
          routecolor = ROUTE_COLOR_YELLOW;
        }
        else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          status_char = '*';
          routecolor = ROUTE_COLOR_GREEN;
        }
        else {
          status_char = '-';
          routecolor = ROUTE_COLOR_RED;
        }
        if (crossing->truecrossing == MB_NO)
          truecrossing_char = ' ';
        else
          truecrossing_char = 'X';
        sprintf(routename, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing_char, i,
                file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
                crossing->section_2, crossing->overlap, crossing->num_ties);
        fprintf(hfp, "## ROUTENAME %s\n", routename);
        fprintf(hfp, "## ROUTESIZE %d\n", 1);
        fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
        fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
        fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
        fprintf(hfp, "> ## STARTROUTE\n");
        fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
        nroute++;
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) true crossing locations to %s\n", nroute, ncrossings_true, routefile);
  }

  /* write mbgrdviz route file for all unfixed >=50% crossings */
  sprintf(routefile, "%s%s_gt50crossing.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", ncrossings_gt50);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only unfixed >=50% crossings */
      if (crossing->overlap >= 50 && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
                                       project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
        file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
        section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
        section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
        snav_1 = section_1->num_snav / 2;
        snav_2 = section_2->num_snav / 2;
        navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
        navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
        navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
        navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          status_char = 'U';
          routecolor = ROUTE_COLOR_YELLOW;
        }
        else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          status_char = '*';
          routecolor = ROUTE_COLOR_GREEN;
        }
        else {
          status_char = '-';
          routecolor = ROUTE_COLOR_RED;
        }
        if (crossing->truecrossing == MB_NO)
          truecrossing_char = ' ';
        else
          truecrossing_char = 'X';
        sprintf(routename, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing_char, i,
                file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
                crossing->section_2, crossing->overlap, crossing->num_ties);
        fprintf(hfp, "## ROUTENAME %s\n", routename);
        fprintf(hfp, "## ROUTESIZE %d\n", 1);
        fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
        fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
        fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
        fprintf(hfp, "> ## STARTROUTE\n");
        fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
        nroute++;
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) >=50%% overlap crossing locations to %s\n", nroute, ncrossings_gt50,
    // routefile);
  }

  /* write mbgrdviz route file for all unfixed >=25% but less than 50% crossings */
  sprintf(routefile, "%s%s_gt25crossing.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", ncrossings_gt25);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only unfixed >=25% but less than 50% crossings crossings */
      if (crossing->overlap >= 25 && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
                                       project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
        file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
        section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
        section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
        snav_1 = section_1->num_snav / 2;
        snav_2 = section_2->num_snav / 2;
        navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
        navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
        navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
        navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          status_char = 'U';
          routecolor = ROUTE_COLOR_YELLOW;
        }
        else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          status_char = '*';
          routecolor = ROUTE_COLOR_GREEN;
        }
        else {
          status_char = '-';
          routecolor = ROUTE_COLOR_RED;
        }
        if (crossing->truecrossing == MB_NO)
          truecrossing_char = ' ';
        else
          truecrossing_char = 'X';
        sprintf(routename, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing_char, i,
                file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
                crossing->section_2, crossing->overlap, crossing->num_ties);
        fprintf(hfp, "## ROUTENAME %s\n", routename);
        fprintf(hfp, "## ROUTESIZE %d\n", 1);
        fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
        fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
        fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
        fprintf(hfp, "> ## STARTROUTE\n");
        fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
        nroute++;
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) >=25%% && < 50%% overlap crossing locations to %s\n", nroute, ncrossings_gt25,
    // routefile);
  }

  /* write mbgrdviz route file for all unfixed <25% crossings */
  sprintf(routefile, "%s%s_lt25crossing.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output tie route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", ncrossings_lt25);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only unfixed <25% crossings crossings */
      if (crossing->overlap < 25 && !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
                                      project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
        file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
        section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
        section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
        snav_1 = section_1->num_snav / 2;
        snav_2 = section_2->num_snav / 2;
        navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
        navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
        navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
        navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          status_char = 'U';
          routecolor = ROUTE_COLOR_YELLOW;
        }
        else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          status_char = '*';
          routecolor = ROUTE_COLOR_GREEN;
        }
        else {
          status_char = '-';
          routecolor = ROUTE_COLOR_RED;
        }
        if (crossing->truecrossing == MB_NO)
          truecrossing_char = ' ';
        else
          truecrossing_char = 'X';
        sprintf(routename, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing_char, i,
                file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
                crossing->section_2, crossing->overlap, crossing->num_ties);
        fprintf(hfp, "## ROUTENAME %s\n", routename);
        fprintf(hfp, "## ROUTESIZE %d\n", 1);
        fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
        fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
        fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
        fprintf(hfp, "> ## STARTROUTE\n");
        fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
        nroute++;
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) <25%% overlap crossing locations to %s\n", nroute, ncrossings_lt25, routefile);
  }

  /* write mbgrdviz route file for all fixed crossings */
  sprintf(routefile, "%s%s_fixedcrossing.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output fixed crossings route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", ncrossings_fixed);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only fixed crossings */
      if (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
          project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV) {
        file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
        file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
        section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
        section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
        snav_1 = section_1->num_snav / 2;
        snav_2 = section_2->num_snav / 2;
        navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
        navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
        navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
        navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          status_char = 'U';
          routecolor = ROUTE_COLOR_YELLOW;
        }
        else if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          status_char = '*';
          routecolor = ROUTE_COLOR_GREEN;
        }
        else {
          status_char = '-';
          routecolor = ROUTE_COLOR_RED;
        }
        if (crossing->truecrossing == MB_NO)
          truecrossing_char = ' ';
        else
          truecrossing_char = 'X';
        sprintf(routename, "%c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d", status_char, truecrossing_char, i,
                file_1->block, crossing->file_id_1, crossing->section_1, file_2->block, crossing->file_id_2,
                crossing->section_2, crossing->overlap, crossing->num_ties);
        fprintf(hfp, "## ROUTENAME %s\n", routename);
        fprintf(hfp, "## ROUTESIZE %d\n", 1);
        fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
        fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
        fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
        fprintf(hfp, "> ## STARTROUTE\n");
        fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
        nroute++;
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) fixed crossing locations to %s\n", nroute, ncrossings_fixed, routefile);
  }

  /* write mbgrdviz route file for all unfixed ties */
  sprintf(routefile, "%s%s_unfixedties.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output unfixed ties route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", nties_unfixed);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
    routecolor = ROUTE_COLOR_BLUEGREEN;

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only unfixed ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET &&
          !(project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
            project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        for (j = 0; j < crossing->num_ties; j++) {
          file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
          file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
          section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
          section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
          tie = (struct mbna_tie *)&crossing->ties[j];
          snav_1 = tie->snav_1;
          snav_2 = tie->snav_2;
          navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
          navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
          navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
          navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (crossing->truecrossing == MB_NO)
            truecrossing_char = ' ';
          else
            truecrossing_char = 'X';
          sprintf(routename, "Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d", status_char,
                  truecrossing_char, i, file_1->block, crossing->file_id_1, crossing->section_1, file_2->block,
                  crossing->file_id_2, crossing->section_2, crossing->overlap, j, crossing->num_ties);
          fprintf(hfp, "## ROUTENAME %s\n", routename);
          fprintf(hfp, "## ROUTESIZE %d\n", 1);
          fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
          fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
          fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
          fprintf(hfp, "> ## STARTROUTE\n");
          fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
          nroute++;
        }
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) unfixed tie locations to %s\n", nroute, nties_unfixed, routefile);
  }

  /* write mbgrdviz route file for all fixed ties */
  sprintf(routefile, "%s%s_fixedties.rte", project->path, project->name);
  if ((hfp = fopen(routefile, "w")) == NULL) {
    fclose(hfp);
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, " > Unable to open output fixed ties route file %s\n", routefile);
  }
  else {
    /* write the route file header */
    fprintf(hfp, "## Route File Version %s\n", ROUTE_VERSION);
    fprintf(hfp, "## Output by Program %s\n", program_name);
    fprintf(hfp, "## MB-System Version %s\n", MB_VERSION);
    strncpy(date, "\0", 25);
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      if ((user_ptr = getenv("LOGNAME")) == NULL)
        user_ptr = unknown;
    gethostname(host, MB_PATH_MAXLINE);
    fprintf(hfp, "## Run by user <%s> on cpu <%s> at <%s>\n", user_ptr, host, date);
    fprintf(hfp, "## Number of routes: %d\n", nties_fixed);
    fprintf(hfp, "## Route point format:\n");
    fprintf(hfp, "##   <longitude (deg)> <latitude (deg)> <topography (m)> <waypoint (boolean)>\n");
    routecolor = ROUTE_COLOR_RED;

    /* loop over all crossings */
    nroute = 0;
    for (i = 0; i < project->num_crossings; i++) {
      crossing = &project->crossings[i];

      /* output only fixed ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET &&
          (project->files[crossing->file_id_1].status == MBNA_FILE_FIXEDNAV ||
           project->files[crossing->file_id_2].status == MBNA_FILE_FIXEDNAV)) {
        for (j = 0; j < crossing->num_ties; j++) {
          file_1 = (struct mbna_file *)&project->files[crossing->file_id_1];
          file_2 = (struct mbna_file *)&project->files[crossing->file_id_2];
          section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
          section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
          tie = (struct mbna_tie *)&crossing->ties[j];
          snav_1 = tie->snav_1;
          snav_2 = tie->snav_2;
          navlon1 = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
          navlat1 = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
          navlon2 = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
          navlat2 = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
          if (crossing->status == MBNA_CROSSING_STATUS_NONE)
            status_char = 'U';
          else if (crossing->status == MBNA_CROSSING_STATUS_SET)
            status_char = '*';
          else
            status_char = '-';
          if (crossing->truecrossing == MB_NO)
            truecrossing_char = ' ';
          else
            truecrossing_char = 'X';
          sprintf(routename, "Tie: %c%c %4d %2.2d:%3.3d:%3.3d %2.2d:%3.3d:%3.3d %3d %2d of %2d", status_char,
                  truecrossing_char, i, file_1->block, crossing->file_id_1, crossing->section_1, file_2->block,
                  crossing->file_id_2, crossing->section_2, crossing->overlap, j, crossing->num_ties);
          fprintf(hfp, "## ROUTENAME %s\n", routename);
          fprintf(hfp, "## ROUTESIZE %d\n", 1);
          fprintf(hfp, "## ROUTECOLOR %d\n", routecolor);
          fprintf(hfp, "## ROUTEPOINTS %d\n", 2);
          fprintf(hfp, "## ROUTEEDITMODE %d\n", MB_NO);
          fprintf(hfp, "> ## STARTROUTE\n");
          fprintf(hfp, "%.10f %.10f 0.00 1\n%.10f %.10f 0.00 1\n>\n", navlon1, navlat1, navlon2, navlat2);
          nroute++;
        }
      }
    }
    fclose(hfp);
    // fprintf(stderr,"Output %d (expected %d) fixed tie locations to %s\n", nroute, nties_fixed, routefile);
  }

  /* output offset vectors */
  if (project->inversion_status == MBNA_INVERSION_CURRENT) {
    sprintf(offsetfile, "%s%s_offset.txt", project->path, project->name);
    if ((hfp = fopen(offsetfile, "w")) != NULL) {
      for (i = 0; i < project->num_files; i++) {
        file = &project->files[i];
        for (j = 0; j < file->num_sections; j++) {
          section = &file->sections[j];
          mb_coor_scale(verbose, 0.5 * (section->latmin + section->latmax), &mtodeglon, &mtodeglat);
          for (k = 0; k < section->num_snav; k++) {
                        mb_get_date(verbose, section->snav_time_d[k], time_i);
                        fprintf(hfp, "%4.4d:%4.4d:%2.2d  %4d/%2d/%2d %2d:%2d:%2d.%6.6d  %.6f %8.3f %8.3f %6.3f\n",
                                        i, j, k, time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
                                        section->snav_time_d[k],
                                        (section->snav_lon_offset[k] / mtodeglon),
                                        (section->snav_lat_offset[k] / mtodeglat),
                                        section->snav_z_offset[k]);
          }
        }
      }
      fclose(hfp);
    }

    /* else set error */
    else {
      status = MB_FAILURE;
      *error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "Unable to update project %s\n > Offset file: %s\n", project->name, offsetfile);
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_overlap(int verbose, struct mbna_project *project, int crossing_id, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_crossing *crossing;
  struct mbna_section *section1;
  struct mbna_section *section2;
  int overlap1[MBNA_MASK_DIM * MBNA_MASK_DIM];
  int overlap2[MBNA_MASK_DIM * MBNA_MASK_DIM];
  double lonoffset, latoffset;
  double lon1min, lon1max;
  double lat1min, lat1max;
  double lon2min, lon2max;
  double lat2min, lat2max;
  double dx1, dy1, dx2, dy2;
  double overlapfraction;
  int ncoverage1, ncoverage2;
  int noverlap1, noverlap2;
  int first;
  int i, ii1, jj1, kk1, ii2, jj2, kk2;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       project:              %p\n", project);
    fprintf(stderr, "dbg2       crossing_id:          %d\n", crossing_id);
  }

  /* get crossing */
  crossing = (struct mbna_crossing *)&project->crossings[crossing_id];

  /* get section endpoints */
  file = &project->files[crossing->file_id_1];
  section1 = &file->sections[crossing->section_1];
  file = &project->files[crossing->file_id_2];
  section2 = &file->sections[crossing->section_2];
  lonoffset = section2->snav_lon_offset[section2->num_snav / 2] - section1->snav_lon_offset[section1->num_snav / 2];
  latoffset = section2->snav_lat_offset[section2->num_snav / 2] - section1->snav_lat_offset[section1->num_snav / 2];

  /* initialize overlap arrays */
  for (i = 0; i < MBNA_MASK_DIM * MBNA_MASK_DIM; i++) {
    overlap1[i] = 0;
    overlap2[i] = 0;
  }

  /* check coverage masks for overlap */
  first = MB_YES;
  dx1 = (section1->lonmax - section1->lonmin) / MBNA_MASK_DIM;
  dy1 = (section1->latmax - section1->latmin) / MBNA_MASK_DIM;
  dx2 = (section2->lonmax - section2->lonmin) / MBNA_MASK_DIM;
  dy2 = (section2->latmax - section2->latmin) / MBNA_MASK_DIM;
  for (ii1 = 0; ii1 < MBNA_MASK_DIM; ii1++)
    for (jj1 = 0; jj1 < MBNA_MASK_DIM; jj1++) {
      kk1 = ii1 + jj1 * MBNA_MASK_DIM;
      if (section1->coverage[kk1] == 1) {
        lon1min = section1->lonmin + dx1 * ii1;
        lon1max = section1->lonmin + dx1 * (ii1 + 1);
        lat1min = section1->latmin + dy1 * jj1;
        lat1max = section1->latmin + dy1 * (jj1 + 1);
        for (ii2 = 0; ii2 < MBNA_MASK_DIM; ii2++)
          for (jj2 = 0; jj2 < MBNA_MASK_DIM; jj2++) {
            kk2 = ii2 + jj2 * MBNA_MASK_DIM;
            if (section2->coverage[kk2] == 1) {
              lon2min = section2->lonmin + dx2 * ii2 + lonoffset;
              lon2max = section2->lonmin + dx2 * (ii2 + 1) + lonoffset;
              lat2min = section2->latmin + dy2 * jj2 + latoffset;
              lat2max = section2->latmin + dy2 * (jj2 + 1) + latoffset;
              if ((lon1min < lon2max) && (lon1max > lon2min) && (lat1min < lat2max) && (lat1max > lat2min)) {
                overlap1[kk1] = 1;
                overlap2[kk2] = 1;
              }
            }
          }
      }
    }

  /* count fractions covered */
  ncoverage1 = 0;
  ncoverage2 = 0;
  noverlap1 = 0;
  noverlap2 = 0;
  for (i = 0; i < MBNA_MASK_DIM * MBNA_MASK_DIM; i++) {
    if (section1->coverage[i] == 1)
      ncoverage1++;
    if (section2->coverage[i] == 1)
      ncoverage2++;
    if (overlap1[i] == 1)
      noverlap1++;
    if (overlap2[i] == 1)
      noverlap2++;
  }
  overlapfraction = (dx1 * dy1) / (dx1 * dy1 + dx2 * dy2) * ((double)noverlap1) / ((double)ncoverage1) +
                    (dx2 * dy2) / (dx1 * dy1 + dx2 * dy2) * ((double)noverlap2) / ((double)ncoverage2);
  crossing->overlap = (int)(100.0 * overlapfraction);
  if (crossing->overlap < 1)
    crossing->overlap = 1;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       crossing->overlap: %d\n", crossing->overlap);
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_overlapbounds(int verbose, struct mbna_project *project, int crossing_id, double offset_x,
                                       double offset_y, double *lonmin, double *lonmax, double *latmin, double *latmax,
                                       int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_crossing *crossing;
  struct mbna_section *section1;
  struct mbna_section *section2;
  int overlap1[MBNA_MASK_DIM * MBNA_MASK_DIM];
  int overlap2[MBNA_MASK_DIM * MBNA_MASK_DIM];
  double lon1min, lon1max;
  double lat1min, lat1max;
  double lon2min, lon2max;
  double lat2min, lat2max;
  double dx1, dy1, dx2, dy2;
  int first;
  int i, ii1, jj1, kk1, ii2, jj2, kk2;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       project:              %p\n", project);
    fprintf(stderr, "dbg2       crossing_id:          %d\n", crossing_id);
    fprintf(stderr, "dbg2       offset_x:             %f\n", offset_x);
    fprintf(stderr, "dbg2       offset_y:             %f\n", offset_y);
  }

  /* get crossing */
  crossing = (struct mbna_crossing *)&project->crossings[crossing_id];

  /* get section endpoints */
  file = &project->files[crossing->file_id_1];
  section1 = &file->sections[crossing->section_1];
  file = &project->files[crossing->file_id_2];
  section2 = &file->sections[crossing->section_2];

  /* initialize overlap arrays */
  for (i = 0; i < MBNA_MASK_DIM * MBNA_MASK_DIM; i++) {
    overlap1[i] = 0;
    overlap2[i] = 0;
  }

  /* get overlap region bounds and focus point */
  first = MB_YES;
  *lonmin = 0.0;
  *lonmax = 0.0;
  *latmin = 0.0;
  *latmax = 0.0;
  dx1 = (section1->lonmax - section1->lonmin) / MBNA_MASK_DIM;
  dy1 = (section1->latmax - section1->latmin) / MBNA_MASK_DIM;
  dx2 = (section2->lonmax - section2->lonmin) / MBNA_MASK_DIM;
  dy2 = (section2->latmax - section2->latmin) / MBNA_MASK_DIM;
  for (ii1 = 0; ii1 < MBNA_MASK_DIM; ii1++) {
    for (jj1 = 0; jj1 < MBNA_MASK_DIM; jj1++) {
      kk1 = ii1 + jj1 * MBNA_MASK_DIM;
      if (section1->coverage[kk1] == 1) {
        lon1min = section1->lonmin + dx1 * ii1;
        lon1max = section1->lonmin + dx1 * (ii1 + 1);
        lat1min = section1->latmin + dy1 * jj1;
        lat1max = section1->latmin + dy1 * (jj1 + 1);
        for (ii2 = 0; ii2 < MBNA_MASK_DIM; ii2++) {
          for (jj2 = 0; jj2 < MBNA_MASK_DIM; jj2++) {
            kk2 = ii2 + jj2 * MBNA_MASK_DIM;
            if (section2->coverage[kk2] == 1) {
              lon2min = section2->lonmin + dx2 * ii2 + offset_x;
              lon2max = section2->lonmin + dx2 * (ii2 + 1) + offset_x;
              lat2min = section2->latmin + dy2 * jj2 + offset_y;
              lat2max = section2->latmin + dy2 * (jj2 + 1) + offset_y;
              if ((lon1min < lon2max) && (lon1max > lon2min) && (lat1min < lat2max) && (lat1max > lat2min)) {
                overlap1[kk1] = 1;
                overlap2[kk2] = 1;
                if (first == MB_NO) {
                  *lonmin = MIN(*lonmin, MAX(lon1min, lon2min));
                  *lonmax = MAX(*lonmax, MIN(lon1max, lon2max));
                  *latmin = MIN(*latmin, MAX(lat1min, lat2min));
                  *latmax = MAX(*latmax, MIN(lat1max, lat2max));
                }
                else {
                  first = MB_NO;
                  *lonmin = MAX(lon1min, lon2min);
                  *lonmax = MIN(lon1max, lon2max);
                  *latmin = MAX(lat1min, lat2min);
                  *latmax = MIN(lat1max, lat2max);
                }
              }
            }
          }
                }
      }
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       lonmin:      %.10f\n", *lonmin);
    fprintf(stderr, "dbg2       lonmax:      %.10f\n", *lonmax);
    fprintf(stderr, "dbg2       latmin:      %.10f\n", *latmin);
    fprintf(stderr, "dbg2       latmax:      %.10f\n", *latmax);
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_focuspoint(int verbose, struct mbna_project *project, int crossing_id,
                                    double offset_x, double offset_y, int *isnav1_focus, int *isnav2_focus,
                                    double *lon_focus, double *lat_focus, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_crossing *crossing;
  struct mbna_section *section1;
  struct mbna_section *section2;
    int snav_1_closest;
    int snav_2_closest;
    int isnav1, isnav2;
    double dx, dy;
    double distance, distance_closest;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       project:              %p\n", project);
    fprintf(stderr, "dbg2       crossing_id:          %d\n", crossing_id);
    fprintf(stderr, "dbg2       offset_x:             %f\n", offset_x);
    fprintf(stderr, "dbg2       offset_y:             %f\n", offset_y);
  }

  /* get crossing */
  crossing = (struct mbna_crossing *)&project->crossings[crossing_id];

  /* get section endpoints */
  file = &project->files[crossing->file_id_1];
  section1 = &file->sections[crossing->section_1];
  file = &project->files[crossing->file_id_2];
  section2 = &file->sections[crossing->section_2];

    /* find focus point - center of the line segment connecting the two closest
     * approach nav points */
    snav_1_closest = 0;
    snav_2_closest = 0;
    distance_closest = 999999999.999;
    for (isnav1=0; isnav1 < section1->num_snav; isnav1++) {
        for (isnav2=0; isnav2 < section2->num_snav; isnav2++) {
            dx = (section2->snav_lon[isnav2] + offset_x - section1->snav_lon[isnav1]) / project->mtodeglon;
            dy = (section2->snav_lat[isnav2] + offset_y - section1->snav_lat[isnav1]) / project->mtodeglat;
            distance = sqrt(dx * dx + dy * dy);
            if (distance < distance_closest) {
                distance_closest = distance;
                snav_1_closest = isnav1;
                snav_2_closest = isnav2;
            }
        }
    }
    *lon_focus = 0.5 * (section1->snav_lon[snav_1_closest] + section2->snav_lon[snav_2_closest]);
    *lat_focus = 0.5 * (section1->snav_lat[snav_1_closest] + section2->snav_lat[snav_2_closest]);

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       lon_focus:   %.10f\n", *lon_focus);
    fprintf(stderr, "dbg2       lat_focus:   %.10f\n", *lat_focus);
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_plot_functions(int verbose, struct mbna_project *project,
                             void *plot, void *newpen, void *setline,
                             void *justify_string, void *plot_string, int *error) {
  /* local variables */
  int status = MB_SUCCESS;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       plot:             %p\n", plot);
    fprintf(stderr, "dbg2       newpen:           %p\n", newpen);
    fprintf(stderr, "dbg2       setline:          %p\n", setline);
    fprintf(stderr, "dbg2       justify_string:   %p\n", justify_string);
    fprintf(stderr, "dbg2       plot_string:      %p\n", plot_string);
  }

    /* set the plotting function pointers */
    project->mbnavadjust_plot = plot;
    project->mbnavadjust_newpen = newpen;
    project->mbnavadjust_setline = setline;
    project->mbnavadjust_justify_string = justify_string;
    project->mbnavadjust_plot_string = plot_string;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_read_triangles(int verbose, struct mbna_project *project,
                             int file_id, int section_id,
                             struct swath *swath, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  mb_path tpath;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       file_id:          %d\n", file_id);
    fprintf(stderr, "dbg2       section_id:       %d\n", section_id);
    fprintf(stderr, "dbg2       swath:            %p\n", swath);
  }

  // if all good then try to read existing triangularization
  FILE *tfp = NULL;
  size_t read_size;
  int tfile_tag;
  unsigned short tfile_version_major, tfile_version_minor;
  sprintf(tpath, "%s/nvs_%4.4d_%4.4d.mb71.tri", project->datadir, file_id, section_id);
  if ((tfp = fopen(tpath, "r")) != NULL) {

    // read the triangularization file header
    if ((read_size = fread(&tfile_tag, 1, sizeof(int), tfp)) != sizeof(int))
      status = MB_FAILURE;
    if (status == MB_SUCCESS
        && (read_size = fread(&tfile_version_major, 1, sizeof(tfile_version_major), tfp))
        != sizeof(tfile_version_major))
      status = MB_FAILURE;
    if (status == MB_SUCCESS
        && (read_size = fread(&tfile_version_minor, 1, sizeof(tfile_version_minor), tfp))
        != sizeof(tfile_version_minor))
      status = MB_FAILURE;
    if (status == MB_SUCCESS
        && (read_size = fread(&(swath->triangle_scale), 1, sizeof(double), tfp)) != sizeof(double))
      status = MB_FAILURE;
    if (status == MB_SUCCESS
        && (read_size = fread(&(swath->npts), 1, sizeof(int), tfp)) != sizeof(int))
      status = MB_FAILURE;
    if (status == MB_SUCCESS
        && (read_size = fread(&(swath->ntri), 1, sizeof(int), tfp)) != sizeof(int))
      status = MB_FAILURE;
    if (status == MB_FAILURE) {
      *error = MB_ERROR_EOF;
    }

    // allocate memory if required
    if (status == MB_SUCCESS) {
      if (swath->npts > swath->npts_alloc) {
        swath->npts_alloc = swath->npts;
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(int), (void **)&(swath->edge), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(int), (void **)&(swath->pingid), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(int), (void **)&(swath->beamid), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(double), (void **)&(swath->x), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(double), (void **)&(swath->y), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->npts_alloc * sizeof(double), (void **)&(swath->z), error);
      }
      if (swath->ntri > swath->ntri_alloc) {
        swath->ntri_alloc = swath->ntri;
        for (int i = 0; i < 3; i++) {
          status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->iv[i]), error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->ct[i]), error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->cs[i]), error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->ed[i]), error);
          status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->flag[i]), error);
        }
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(double), (void **)&(swath->v1), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(double), (void **)&(swath->v2), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(double), (void **)&(swath->v3), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, swath->ntri * sizeof(int), (void **)&(swath->istack), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, 3 * swath->ntri * sizeof(int), (void **)&(swath->kv1), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, 3 * swath->ntri * sizeof(int), (void **)&(swath->kv2), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, (4 * swath->ntri + 1) * sizeof(double), (void **)&(swath->xsave), error);
        status &= mb_reallocd(verbose, __FILE__, __LINE__, (4 * swath->ntri + 1) * sizeof(double), (void **)&(swath->ysave), error);
      }
      if (status == MB_FAILURE) {
        *error = MB_ERROR_MEMORY_FAIL;
      }
    }

    // read the vertices and the triangles
    if (status == MB_SUCCESS) {
      for (int i=0; i<swath->npts; i++) {
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->edge[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->pingid[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->beamid[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
      }
      for (int i=0; i<swath->ntri; i++) {
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->iv[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->iv[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->iv[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ct[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ct[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ct[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->cs[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->cs[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->cs[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ed[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ed[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
        if (status == MB_SUCCESS
            && (read_size = fread(&(swath->ed[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status = MB_FAILURE;
      }
      fclose(tfp);

      if (status != MB_SUCCESS) {
        *error = MB_ERROR_EOF;
        swath->npts = 0;
        swath->ntri = 0;
      }
    }

    // get x y z values from swath->pings
    if (status == MB_SUCCESS && swath->npts > 0) {
      swath->bath_min = swath->z[0];
      swath->bath_max = swath->z[0];
      for (int ipt=0; ipt<swath->npts; ipt++) {
        int iping = swath->pingid[ipt];
        int ibeam = swath->beamid[ipt];
        swath->x[ipt] = swath->pings[iping].bathlon[ibeam];
        swath->y[ipt] = swath->pings[iping].bathlat[ibeam];
        swath->z[ipt] = swath->pings[iping].bath[ibeam];
        swath->bath_min = MIN(swath->bath_min, swath->z[ipt]);
        swath->bath_max = MAX(swath->bath_max, swath->z[ipt]);
      }
    }
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_write_triangles(int verbose, struct mbna_project *project,
                             int file_id, int section_id,
                             struct swath *swath, int *error) {

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       file_id:          %d\n", file_id);
    fprintf(stderr, "dbg2       section_id:       %d\n", section_id);
    fprintf(stderr, "dbg2       swath:            %p\n", swath);
  }

  // if all good then try to read existing triangularization
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  if (status == MB_SUCCESS && swath->ntri > 0) {
    FILE *tfp = NULL;
    size_t write_size;
    int tfile_tag = 74726961;
    unsigned short tfile_version_major = 1;
    unsigned short tfile_version_minor = 0;
    mb_path tpath;
    sprintf(tpath, "%s/nvs_%4.4d_%4.4d.mb71.tri", project->datadir, file_id, section_id);
    if ((tfp = fopen(tpath, "w")) != NULL) {

      // write the triangularization file header
      if ((write_size = fwrite(&tfile_tag, 1, sizeof(int), tfp)) != sizeof(int))
        status &= MB_FAILURE;
      if ((write_size = fwrite(&tfile_version_major, 1, sizeof(unsigned short), tfp)) != sizeof(unsigned short))
        status &= MB_FAILURE;
      if ((write_size = fwrite(&tfile_version_minor, 1, sizeof(unsigned short), tfp)) != sizeof(unsigned short))
        status &= MB_FAILURE;
      if ((write_size = fwrite(&(swath->triangle_scale), 1, sizeof(double), tfp)) != sizeof(double))
        status &= MB_FAILURE;
      if ((write_size = fwrite(&(swath->npts), 1, sizeof(int), tfp)) != sizeof(int))
        status &= MB_FAILURE;
      if ((write_size = fwrite(&(swath->ntri), 1, sizeof(int), tfp)) != sizeof(int))
        status &= MB_FAILURE;

      // write the vertices and the triangles
      for (int i=0; i<swath->npts; i++) {
        if ((write_size = fwrite(&(swath->edge[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->pingid[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->beamid[i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
      }
      for (int i=0; i<swath->ntri; i++) {
        if ((write_size = fwrite(&(swath->iv[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->iv[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->iv[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ct[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ct[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ct[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->cs[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->cs[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->cs[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ed[0][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ed[1][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        if ((write_size = fwrite(&(swath->ed[2][i]), 1, sizeof(int), tfp)) != sizeof(int))
          status &= MB_FAILURE;
        }

      fclose (tfp);
      if (status != MB_SUCCESS)
        *error = MB_ERROR_WRITE_FAIL;
    }
  }
  else {
    status &= MB_FAILURE;
    *error = MB_ERROR_OPEN_FAIL;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_section_load(int verbose, struct mbna_project *project,
                             int file_id, int section_id,
                             void **swathraw_ptr, void **swath_ptr, int num_pings, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *imb_io_ptr;
  struct mbna_swathraw *swathraw;
  struct mbna_pingraw *pingraw;
  struct swath *swath;
  struct ping *ping;
  struct mbna_file *file;
  struct mbna_section *section;

  /* mbio read and write values */
  void *imbio_ptr = NULL;
  void *istore_ptr = NULL;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sonardepth;
  double roll;
  double pitch;
  double heave;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  char *beamflag = NULL;
  double *bath = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *amp = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  char comment[MB_COMMENT_MAXLINE];

  /* MBIO control parameters */
  int pings = 1;
  int lonflip = 0;
  double bounds[4] = {-360, 360, -90, 90};
  int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
  int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
  double btime_d = -248016600.0;
  double etime_d = 2907743400.0;
  double speedmin = 0.0;
  double timegap = 1000000000.0;
  char *error_message;
  int contour_algorithm = MB_CONTOUR_TRIANGLES; /* not MB_CONTOUR_OLD;*/
  int contour_ncolor = 10;

  char path[STRING_MAX];
  char tpath[STRING_MAX];
  int iformat;
  double tick_len_map, label_hgt_map;
  int done;
  int i;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       file_id:          %d\n", file_id);
    fprintf(stderr, "dbg2       section_id:       %d\n", section_id);
    fprintf(stderr, "dbg2       swathraw_ptr:     %p  %p\n", swathraw_ptr, *swathraw_ptr);
    fprintf(stderr, "dbg2       swath_ptr:        %p  %p\n", swath_ptr, *swath_ptr);
    fprintf(stderr, "dbg2       num_pings:        %d\n", num_pings);
  }

  /* load specified section */
  if (project->open == MB_YES && project->num_crossings > 0) {
    /* set section format and path */
    sprintf(path, "%s/nvs_%4.4d_%4.4d.mb71", project->datadir, file_id, section_id);
    sprintf(tpath, "%s/nvs_%4.4d_%4.4d.mb71.tri", project->datadir, file_id, section_id);
    iformat = 71;
    file = &(project->files[file_id]);
    section = &(file->sections[section_id]);

    /* initialize section for reading */
    if ((status = mb_read_init(verbose, path, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                               &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, error)) != MB_SUCCESS) {
      mb_error(verbose, *error, &error_message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", error_message);
      fprintf(stderr, "\nSwath sonar File <%s> not initialized for reading\n", path);
      exit(0);
    }

    /* allocate memory for data arrays */
    if (status == MB_SUCCESS) {
      if (*error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
      if (*error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&bathacrosstrack, error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                   (void **)&bathalongtrack, error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack,
                                   error);
      if (*error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack,
                                   error);

      /* if error initializing memory then don't read the file */
      if (*error != MB_ERROR_NO_ERROR) {
        mb_error(verbose, *error, &error_message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
      }
    }

    /* allocate memory for data arrays */
    if (status == MB_SUCCESS) {
      /* get mb_io_ptr */
      imb_io_ptr = (struct mb_io_struct *)imbio_ptr;

      /* initialize data storage */
      status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbna_swathraw), (void **)swathraw_ptr, error);
      swathraw = (struct mbna_swathraw *)*swathraw_ptr;
      swathraw->beams_bath = beams_bath;
      swathraw->npings_max = num_pings;
      swathraw->npings = 0;
      status = mb_mallocd(verbose, __FILE__, __LINE__, num_pings * sizeof(struct mbna_pingraw),
                          (void **)&swathraw->pingraws, error);
      for (i = 0; i < swathraw->npings_max; i++) {
        pingraw = &swathraw->pingraws[i];
        pingraw->beams_bath = 0;
        pingraw->beamflag = NULL;
        pingraw->bath = NULL;
        pingraw->bathacrosstrack = NULL;
        pingraw->bathalongtrack = NULL;
      }

      /* initialize contour controls */
      tick_len_map = MAX(section->lonmax - section->lonmin, section->latmax - section->latmin) / 500;
      label_hgt_map = MAX(section->lonmax - section->lonmin, section->latmax - section->latmin) / 100;
      status = mb_contour_init(verbose, (struct swath **)swath_ptr, num_pings, beams_bath, contour_algorithm,
                               MB_YES, MB_NO, MB_NO, MB_NO, MB_NO, project->cont_int, project->col_int, project->tick_int,
                               project->label_int, tick_len_map, label_hgt_map, 0.0, contour_ncolor, 0, NULL, NULL, NULL, 0.0,
                               0.0, 0.0, 0.0, 0, 0, 0.0, 0.0,
                                     project->mbnavadjust_plot, project->mbnavadjust_newpen,
                                     project->mbnavadjust_setline, project->mbnavadjust_justify_string,
                                     project->mbnavadjust_plot_string,
                                     error);
      swath = (struct swath *)*swath_ptr;
      swath->beams_bath = beams_bath;
      swath->npings = 0;
      swath->triangle_scale = project->triangle_scale;

      /* if error initializing memory then quit */
      if (*error != MB_ERROR_NO_ERROR) {
        mb_error(verbose, *error, &error_message);
        fprintf(stderr, "\nMBIO Error allocating contour control structure:\n%s\n", error_message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(*error);
      }
    }

    /* now read the data */
    if (status == MB_SUCCESS) {
      done = MB_NO;
      while (done == MB_NO) {
        /* read the next ping */
        status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                            &heading, &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag,
                            bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);

        /* handle successful read */
        if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
          /* allocate memory for the raw arrays */
          pingraw = &swathraw->pingraws[swathraw->npings];
          status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char), (void **)&pingraw->beamflag,
                              error);
          status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double), (void **)&pingraw->bath,
                              error);
          status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
                              (void **)&pingraw->bathacrosstrack, error);
          status = mb_mallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
                              (void **)&pingraw->bathalongtrack, error);

          /* make sure enough memory is allocated for contouring arrays */
          ping = &swath->pings[swathraw->npings];
          if (ping->beams_bath_alloc < beams_bath) {
            status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
                                 (void **)&(ping->beamflag), error);
            status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
                                 (void **)&(ping->bath), error);
            status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
                                 (void **)&(ping->bathlon), error);
            status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
                                 (void **)&(ping->bathlat), error);
            if (contour_algorithm == MB_CONTOUR_OLD) {
              status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(int),
                                   (void **)&(ping->bflag[0]), error);
              status = mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(int),
                                   (void **)&(ping->bflag[1]), error);
            }
            ping->beams_bath_alloc = beams_bath;
          }

          /* copy arrays and update bookkeeping */
          if (*error == MB_ERROR_NO_ERROR) {
            swathraw->npings++;
            if (swathraw->npings >= swathraw->npings_max)
              done = MB_YES;

            for (i = 0; i < 7; i++)
              pingraw->time_i[i] = time_i[i];
            pingraw->time_d = time_d;
            pingraw->navlon = navlon;
            pingraw->navlat = navlat;
            pingraw->heading = heading;
            pingraw->draft = sonardepth;
            pingraw->beams_bath = beams_bath;
            /* fprintf(stderr,"\nPING %d : %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
            swathraw->npings,time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]); */
            for (i = 0; i < beams_bath; i++) {
              pingraw->beamflag[i] = beamflag[i];
              if (mb_beam_ok(beamflag[i])) {
                pingraw->beamflag[i] = beamflag[i];
                pingraw->bath[i] = bath[i];
                pingraw->bathacrosstrack[i] = bathacrosstrack[i];
                pingraw->bathalongtrack[i] = bathalongtrack[i];
              }
              else {
                pingraw->beamflag[i] = MB_FLAG_NULL;
                pingraw->bath[i] = 0.0;
                pingraw->bathacrosstrack[i] = 0.0;
                pingraw->bathalongtrack[i] = 0.0;
              }
              /* fprintf(stderr,"BEAM: %d:%d  Flag:%d    %f %f %f\n",
              swathraw->npings,i,pingraw->beamflag[i],pingraw->bath[i],pingraw->bathacrosstrack[i],pingraw->bathalongtrack[i]);
              */
            }
          }

          /* extract all nav values */
          status = mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, pingraw->time_i, &pingraw->time_d,
                                  &pingraw->navlon, &pingraw->navlat, &speed, &pingraw->heading, &pingraw->draft, &roll,
                                  &pitch, &heave, error);

          /*fprintf(stderr, "%d  %4d/%2d/%2d %2d:%2d:%2d.%6.6d  %15.10f %15.10f %d:%d\n",
          status,
          ping->time_i[0],ping->time_i[1],ping->time_i[2],
          ping->time_i[3],ping->time_i[4],ping->time_i[5],ping->time_i[6],
          ping->navlon, ping->navlat, beams_bath, swath->beams_bath);*/

          /* print debug statements */
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
            fprintf(stderr, "dbg2       kind:           %d\n", kind);
            fprintf(stderr, "dbg2       npings:         %d\n", swathraw->npings);
            fprintf(stderr, "dbg2       time:           %4d %2d %2d %2d %2d %2d %6.6d\n", pingraw->time_i[0],
                    pingraw->time_i[1], pingraw->time_i[2], pingraw->time_i[3], pingraw->time_i[4],
                    pingraw->time_i[5], pingraw->time_i[6]);
            fprintf(stderr, "dbg2       navigation:     %f  %f\n", pingraw->navlon, pingraw->navlat);
            fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
            fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
            fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
            fprintf(stderr, "dbg2       done:           %d\n", done);
            fprintf(stderr, "dbg2       error:          %d\n", *error);
            fprintf(stderr, "dbg2       status:         %d\n", status);
          }
        }
        else if (*error > MB_ERROR_NO_ERROR) {
          status = MB_SUCCESS;
          *error = MB_ERROR_NO_ERROR;
          done = MB_YES;
        }
      }

      /* close the input data file */
      status = mb_close(verbose, &imbio_ptr, error);
    }

    // translate the raw swath data to the contouring structure
    status = mbnavadjust_section_translate(verbose, project, file_id, *swathraw_ptr, *swath_ptr, 0.0, error);

    // if all good then try to read existing triangularization
    if (status == MB_SUCCESS && swath->npings > 0) {
      status = mbnavadjust_read_triangles(verbose, project, file_id, section_id, swath, error);

      // if triangles cannot be read then make triangles and write the triangles out
      if (status == MB_FAILURE) {
        status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
        status = mb_triangulate(verbose, swath, error);
        if (status == MB_SUCCESS) {
          status = mbnavadjust_write_triangles(verbose, project, file_id, section_id, swath, error);
        }
      }
    }
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_section_unload(int verbose, void **swathraw_ptr, void **swath_ptr, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_swathraw *swathraw;
  struct mbna_pingraw *pingraw;
  struct swath *swath;
  struct ping *ping;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       swathraw_ptr:     %p  %p\n", swathraw_ptr, *swathraw_ptr);
    fprintf(stderr, "dbg2       swath_ptr:        %p  %p\n", swath_ptr, *swath_ptr);
  }

  /* unload specified section */
  swathraw = (struct mbna_swathraw *)(*swathraw_ptr);
  swath = (struct swath *)(*swath_ptr);

  /* free raw swath data */
  if (swathraw != NULL && swathraw->pingraws != NULL) {
    for (int i = 0; i < swathraw->npings_max; i++) {
      pingraw = &swathraw->pingraws[i];
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&pingraw->beamflag, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&pingraw->bath, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&pingraw->bathacrosstrack, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&pingraw->bathalongtrack, error);
    }
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&swathraw->pingraws, error);
  }
  if (swathraw != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&swathraw, error);

  /* free contours */
  status = mb_contour_deall(verbose, swath, error);

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_fix_section_sensordepth(int verbose, struct mbna_project *project, int *error) {

  /* local variables */
  int status = MB_SUCCESS;
  struct mb_io_struct *imb_io_ptr;
  struct mbna_file *file;
  struct mbna_section *section;

  /* mbio read and write values */
  void *imbio_ptr = NULL;
  void *istore_ptr = NULL;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sonardepth;
  double roll;
  double pitch;
  double heave;
  int beams_bath;
  int beams_amp;
  int pixels_ss;
  char *beamflag = NULL;
  double *bath = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *amp = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  char comment[MB_COMMENT_MAXLINE];

    /* MBIO control parameters */
    int pings = 1;
    int lonflip = 0;
    double bounds[4] = {-360, 360, -90, 90};
    int btime_i[7] = {1962, 2, 21, 10, 30, 0, 0};
    int etime_i[7] = {2062, 2, 21, 10, 30, 0, 0};
    double btime_d = -248016600.0;
    double etime_d = 2907743400.0;
    double speedmin = 0.0;
    double timegap = 1000000000.0;
    char *error_message;

  char path[STRING_MAX];
  int iformat;
    int ifile, isection, isnav;
    int num_pings;
  int done;
  int i;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
  }

  /* read specified section, extracting sensordepth values for the snav points */
  if (project != NULL) {
        for (ifile=0;ifile<project->num_files;ifile++) {
            file = &(project->files[ifile]);
            for (isection=0;isection<file->num_sections;isection++) {

                /* set section format and path */
                sprintf(path, "%s/nvs_%4.4d_%4.4d.mb71", project->datadir, ifile, isection);
                iformat = 71;
                file = &(project->files[ifile]);
                section = &(file->sections[isection]);

                /* initialize section for reading */
                if ((status = mb_read_init(verbose, path, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                                           &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, error)) != MB_SUCCESS) {
                    mb_error(verbose, *error, &error_message);
                    fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", error_message);
                    fprintf(stderr, "\nSwath sonar File <%s> not initialized for reading\n", path);
                    exit(0);
                }

                /* allocate memory for data arrays */
                if (status == MB_SUCCESS) {
                    if (*error == MB_ERROR_NO_ERROR)
                        status =
                            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status =
                            mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                                   (void **)&bathacrosstrack, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                                   (void **)&bathalongtrack, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack,
                                                   error);
                    if (*error == MB_ERROR_NO_ERROR)
                        status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack,
                                                   error);

                    /* if error initializing memory then don't read the file */
                    if (*error != MB_ERROR_NO_ERROR) {
                        mb_error(verbose, *error, &error_message);
                        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
                    }
                }

                /* now read the data */
                if (status == MB_SUCCESS) {
                    imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
                    done = MB_NO;
                    isnav = 0;
                    num_pings = 0;
                    while (done == MB_NO && isnav < section->num_snav) {
                        /* read the next ping */
                        status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                                            &heading, &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag,
                                            bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);

                        /* handle successful read */
                        if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
                            if (num_pings == section->snav_id[isnav]) {
                                section->snav_sensordepth[isnav] = sonardepth;
fprintf(stderr, "Update sensordepth section %4.4d:%4.4d:%2.2d  %4d/%2d/%2d %2d:%2d:%2d.%6.6d  %.6f %.6f %.6f\n",
ifile, isection, isnav, time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
time_d, section->snav_time_d[isnav], (section->snav_time_d[isnav]-time_d));
                                isnav++;
                            }
                            num_pings++;
                        }
                        else if (*error > MB_ERROR_NO_ERROR) {
                            status = MB_SUCCESS;
                            *error = MB_ERROR_NO_ERROR;
                            done = MB_YES;
                        }
                    }

                    /* close the input data file */
                    status = mb_close(verbose, &imbio_ptr, error);
                }
            }
        }
    }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_translate(int verbose, struct mbna_project *project,
                                  int file_id, void *swathraw_ptr, void *swath_ptr,
                                  double zoffset, int *error) {
  /* local variables */
  int status = MB_SUCCESS;
  struct mbna_swathraw *swathraw;
  struct mbna_pingraw *pingraw;
  struct swath *swath;
  struct ping *ping;
  double mtodeglon, mtodeglat, headingx, headingy;
  double depth, depthacrosstrack, depthalongtrack;
  double alpha, beta, range;
  int i, iping;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       file_id:          %d\n", file_id);
    fprintf(stderr, "dbg2       swathraw_ptr:     %p\n", swathraw_ptr);
    fprintf(stderr, "dbg2       swath_ptr:        %p\n", swath_ptr);
    fprintf(stderr, "dbg2       zoffset:          %f\n", zoffset);
  }

  // translate soundings from the raw swath form to the structure used for
  // contouring, including applying a depth offset and any heading or roll bias
  // applied to the source data on a file by file basis
  if (project != NULL && swathraw_ptr != NULL && swath_ptr != NULL && project->open == MB_YES) {
    swathraw = (struct mbna_swathraw *)swathraw_ptr;
    swath = (struct swath *)swath_ptr;
    swath->npings = 0;

    for (iping = 0; iping < swathraw->npings; iping++) {
      swath->npings++;
      pingraw = &swathraw->pingraws[iping];
      ping = &swath->pings[swath->npings - 1];
      for (i = 0; i < 7; i++)
        ping->time_i[i] = pingraw->time_i[i];
      ping->time_d = pingraw->time_d;
      ping->navlon = pingraw->navlon;
      ping->navlat = pingraw->navlat;
      ping->heading = pingraw->heading + project->files[file_id].heading_bias;
      mb_coor_scale(verbose, pingraw->navlat, &mtodeglon, &mtodeglat);
      headingx = sin(ping->heading * DTR);
      headingy = cos(ping->heading * DTR);
      ping->beams_bath = pingraw->beams_bath;
      for (i = 0; i < ping->beams_bath; i++) {
        if (mb_beam_ok(pingraw->beamflag[i])) {
          /* strip off transducer depth */
          depth = pingraw->bath[i] - pingraw->draft;

          /* get range and angles in
          roll-pitch frame */
          range = sqrt(depth * depth + pingraw->bathacrosstrack[i] * pingraw->bathacrosstrack[i] +
                       pingraw->bathalongtrack[i] * pingraw->bathalongtrack[i]);
          alpha = asin(pingraw->bathalongtrack[i] / range);
          beta = acos(pingraw->bathacrosstrack[i] / range / cos(alpha));

          /* apply roll correction */
          beta += DTR * project->files[file_id].roll_bias;

          /* recalculate bathymetry */
          depth = range * cos(alpha) * sin(beta);
          depthalongtrack = range * sin(alpha);
          depthacrosstrack = range * cos(alpha) * cos(beta);

          /* add heave and draft back in */
          depth += pingraw->draft;

          /* add zoffset */
          depth += zoffset;

          /* get bathymetry in lon lat */
          ping->beamflag[i] = pingraw->beamflag[i];
          ping->bath[i] = depth;
          ping->bathlon[i] =
          pingraw->navlon + headingy * mtodeglon * depthacrosstrack + headingx * mtodeglon * depthalongtrack;
          ping->bathlat[i] =
          pingraw->navlat - headingx * mtodeglat * depthacrosstrack + headingy * mtodeglat * depthalongtrack;
        }
        else {
          ping->beamflag[i] = MB_FLAG_NULL;
          ping->bath[i] = 0.0;
          ping->bathlon[i] = pingraw->navlon;
          ping->bathlat[i] = pingraw->navlat;
        }
      }
    }

    // if soundings have been triangulated then reset the x y z values for the vertices
    if (swath->npts > 0) {
      for (int ipt=0; ipt<swath->npts; ipt++) {
        ping = &swath->pings[swath->pingid[ipt]];
        swath->x[ipt] = ping->bathlon[swath->beamid[ipt]];
        swath->y[ipt] = ping->bathlat[swath->beamid[ipt]];
        swath->z[ipt] = ping->bath[swath->beamid[ipt]];
      }
    }

  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_section_contour(int verbose, struct mbna_project *project,
                                int fileid, int sectionid, struct swath *swath,
                                struct mbna_contour_vector *contour, int *error) {
  /* local variables */
  int status = MB_SUCCESS;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:          %d\n", verbose);
    fprintf(stderr, "dbg2       project:          %p\n", project);
    fprintf(stderr, "dbg2       fileid:           %d\n", fileid);
    fprintf(stderr, "dbg2       sectionid:        %d\n", sectionid);
    fprintf(stderr, "dbg2       swath:            %p\n", swath);
    fprintf(stderr, "dbg2       contour:          %p\n", contour);
    fprintf(stderr, "dbg2       nvector:          %d\n", contour->nvector);
    fprintf(stderr, "dbg2       nvector_alloc:    %d\n", contour->nvector_alloc);
  }

  if (swath != NULL) {
    /* set vectors */
    contour->nvector = 0;

    /* reset contouring parameters */
    swath->contour_int = project->cont_int;
    swath->color_int = project->col_int;
    swath->tick_int = project->tick_int;

    /* generate contours */
    status = mb_contour(verbose, swath, error);

    /* set contours up to date flag */
    project->files[fileid].sections[sectionid].contoursuptodate = MB_YES;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
