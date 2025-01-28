/*--------------------------------------------------------------------
 *    The MB-system:  mbeditviz_prog.c    5/1/2007
 *
 *    Copyright (c) 2007-2024 by
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
 * qt-mbeditviz is derived from mbeditviz an interactive swath bathymetry 
 * editor and patch test tool for  MB-System.
 * It can work with any data format supported by the MBIO library.
 * This include file contains global control parameters shared with
 * the Motif interface code.
 *
 * Author:  D. W. Caress, T. O'Reilly
 * Date:  December 31, 2024
 */

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_singlebeam.h"

#include "Backend.h"

/* No isinf() os VS, so make one */
#ifdef _MSC_VER
#  define isinf(x) (!_finite(x))
#endif

/// Show message
int (*showMessage)(char *message);

/// Hide message
int (*hideMessage)(void);

/// Update GUI
void (*updateGui)(void);

/// Show error dialog
int (*showErrorDialog)(char *s1, char *s2, char *s3);

/* id variables - set in mbeditviz_init() */
char *program_name = NULL;
char *help_message = NULL;
char *usage_message = NULL;

/* status variables */
char *error_message;
mb_path message = "";

/* data file parameters */
void *datalist;



/*--------------------------------------------------------------------*/
int Backend::init(int argc, char **argv,
                   char *programName,
                   char *helpMsg,
                   char *usageMsg,
                   int (*showMessageArg)(char *),
                   int (*hideMessageArg)(void),
                   void (*updateGuiArg)(void),
                   int (*showErrorDialogArg)(char *, char *, char *)) {

  program_name = strdup(programName);
  help_message = strdup(helpMsg);
  usage_message = strdup(usageMsg);

  showMessage = showMessageArg;
  hideMessage = hideMessageArg;
  updateGui = updateGuiArg;
  showErrorDialog = showErrorDialogArg;

  status_ = MB_SUCCESS;
  error_ = MB_ERROR_NO_ERROR;
  verbose_ = 0;

  mode_output_ = MBEV_OUTPUT_MODE_EDIT;
  grid_algorithm_ = MBEV_GRID_ALGORITH_FOOTPRINT;
  num_files_ = 0;
  num_files_alloc_ = 0;
  num_files__loaded = 0;

  num_esf_open_ = 0;

  files_ = NULL;
  grid_.status = grid__NONE;
  grid_.projection_id[0] = 0;
  for (int i = 0; i < 4; i++) {
    grid_.bounds[i] = 0.0;
    grid_.boundsutm[i] = 0.0;
  }
  grid_.dx = 0.0;
  grid_.dy = 0.0;
  grid_.n_columns = 0;
  grid_.n_rows = 0;
  grid_.min = 0.0;
  grid_.max = 0.0;
  grid_.smin = 0.0;
  grid_.smax = 0.0;
  grid_.nodatavalue = 0.0;
  grid_.sum = NULL;
  grid_.wgt = NULL;
  grid_.val = NULL;
  grid_.sgm = NULL;
  for (int i = 0; i < 4; i++) {
    grid__bounds[i] = 0.0;
    grid__boundsutm[i] = 0.0;
  }
  grid_cellsize_ = 0.0;
  grid_n_columns_ = 0;
  grid_n_rows_ = 0;
  selected_.displayed = false;
  selected_.xorigin = 0.0;
  selected_.yorigin = 0.0;
  selected_.zorigin = 0.0;
  selected_.bearing = 0.0;
  selected_.xmin = 0.0;
  selected_.ymin = 0.0;
  selected_.zmin = 0.0;
  selected_.xmax = 0.0;
  selected_.ymax = 0.0;
  selected_.zmax = 0.0;
  selected_.sinbearing = 0.0;
  selected_.cosbearing = 0.0;
  selected_.scale = 0.0;
  selected_.zscale = 0.0;
  selected_.num_soundings = 0;
  selected_.num_soundings_unflagged = 0;
  selected_.num_soundings_flagged = 0;
  selected_.num_soundings_alloc = 0;
  selected_.soundings = NULL;
  rollBias_ = 0.0;
  pitchBias_ = 0.0;
  headingBias_ = 0.0;
  timeLag_ = 0.0;
  snell_ = 1.0;
  sizeMultiplier_ = 2;
  nSoundingThreshold_ = 5;

  /* set mbio default values */
  mb_lonflip(verbose_, &lonFlip_);
  mb_uselockfiles(verbose_, &useLockFiles_);
  format_ = 0;

  bool input_file_set = false;
  bool delete_input_file = false;
  mb_path ifile;

  {
  int errflg = 0;
  int c;
  int help = 0;

  while ((c = getopt(argc, argv, "VvHhF:f:GgI:i:Rr")) != -1)
    switch (c) {
    case 'H':
    case 'h':
      help++;
      break;
    case 'V':
    case 'v':
      verbose_++;
      break;
    case 'F':
    case 'f':
      sscanf(optarg, "%d", &format_);
      break;
    case 'G':
    case 'g':
      grid_algorithm_ = grid_algorithm__SIMPLEMEAN;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%s", ifile);
      input_file_set = true;
      break;
    case 'R':
    case 'r':
      delete_input_file = true;
      break;
    case '?':
      errflg++;
    }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error_ = MB_ERROR_BAD_USAGE;
    exit(error_);
  }

  /* print starting message */
  if (verbose_ == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting debug statements */
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose_:        %d\n", verbose_);
    fprintf(stderr, "dbg2       help:                %d\n", help);
    fprintf(stderr, "dbg2       input_file_set:      %d\n", input_file_set);
    fprintf(stderr, "dbg2       delete_input_file:   %d\n", delete_input_file);
    fprintf(stderr, "dbg2       input file:          %s\n", ifile);
  }

  /* if help desired then print it and exit */
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error_);
  }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       argc:      %d\n", argc);
    for (int i = 0; i < argc; i++)
      fprintf(stderr, "dbg2       argv[%d]:    %s\n", i, argv[i]);
  }

  /* If specified read input data */
  if (input_file_set) {
    status_ = Backend::open_data(ifile, format_);
    if (delete_input_file) {
      mb_pathplus shell_command;
      snprintf(shell_command, sizeof(shell_command), "rm %s &", ifile);
      system(shell_command);
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBeditviz function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:        %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:  %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::get_format(char *file, int *form) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       file:        %s\n", file);
    fprintf(stderr, "dbg2       format:      %d\n", *form);
  }

  /* get filenames */
  /* look for MB suffix convention */
  char tmp[MB_PATH_MAXLINE];
  int tform;
  if ((status_ = mb_get_format(verbose_, file, tmp, &tform, &error_)) == MB_SUCCESS) {
    *form = tform;
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       format:      %d\n", *form);
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::open_data(char *path, int format) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       file:        %s\n", path);
    fprintf(stderr, "dbg2       format:      %d\n", format);
  }

  /* get format if required */
  if (format == 0)
    mb_get_format(verbose_, path, NULL, &format, &error_);

  double weight;
  int filestatus;
  char fileraw[MB_PATH_MAXLINE];
  char fileprocessed[MB_PATH_MAXLINE];
  char dfile[MB_PATH_MAXLINE];

  /* loop until all inf files are read */
  bool done = false;
  while (!done) {
    if (format > 0) {
      status_ = Backend::import_file(path, format);
      done = true;
    }
    else if (format == -1) {
      if ((status_ = mb_datalist_open(verbose_, &datalist, path, MB_DATALIST_LOOK_NO, &error_)) == MB_SUCCESS) {
        while (!done) {
          if ((status_ = mb_datalist_read2(verbose_, datalist, &filestatus, fileraw, fileprocessed, dfile,
                                               &format, &weight, &error_)) == MB_SUCCESS) {
            status_ = Backend::import_file(fileraw, format);
          }
          else {
            status_ = mb_datalist_close(verbose_, &datalist, &error_);
            done = true;
          }
        }
      }
    }
  }
  (*hideMessage)();
  (*updateGui)();

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::import_file(char *path, int format) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       path:        %s\n", path);
    fprintf(stderr, "dbg2       format:      %d\n", format);
  }

  /* turn on message */
  char *root = (char *)strrchr(path, '/');
  if (root == NULL)
    root = path;
  else
    root++;
  if (num_files_ % 100 == 0) {
    snprintf(message, sizeof(message), "Importing format %d data from %s", format, root);
    (*showMessage)(message);
  }

  /* allocate mbpr_file_struct array if needed */
  status_ = MB_SUCCESS;
  if (num_files_alloc_ <= num_files_) {
    void *nptr = realloc(files_, sizeof(File) * (num_files__alloc + MBEV_ALLOC_NUM));
    if (nptr != NULL) {
      files_ = (File *)nptr;
      num_files__alloc += MBEV_ALLOC_NUM;
    } else {
      free(files_);
      files_ = NULL;
      status_ = MB_FAILURE;
      error_ = MB_ERROR_MEMORY_FAIL;
    }
  }

  /* set new file structure */
  if (status_ == MB_SUCCESS) {
    File *file = &(files_[num_files_]);
    file->load_status = false;
    file->load_status_shown = false;
    file->locked = false;
    file->esf_exists = false;
    strcpy(file->path, path);
    strcpy(file->name, root);
    file->format = format;
    file->raw_info_loaded = false;
    file->esf_open = false;
    file->esf_changed = false;
    file->n_async_heading = 0;
    file->n_async_heading_alloc = 0;
    file->async_heading_time_d = NULL;
    file->async_heading_heading = NULL;
    file->n_async_attitude = 0;
    file->n_async_attitude_alloc = 0;
    file->async_attitude_time_d = NULL;
    file->async_attitude_roll = NULL;
    file->async_attitude_pitch = NULL;
    file->n_sync_attitude = 0;
    file->n_sync_attitude_alloc = 0;
    file->sync_attitude_time_d = NULL;
    file->sync_attitude_roll = NULL;
    file->sync_attitude_pitch = NULL;

    /* load info */
    status_ = mb_get_info(verbose_, file->path, &(file->raw_info), lonFlip_,
			  &error_);
    if (status_ == MB_SUCCESS) {
      file->raw_info_loaded = true;
      num_files_++;
    }
    else {
      fprintf(stderr, "Unable to load file %s because of missing *.inf file\n", file->path);
    }

    /* load processing parameters */
    if (status_ == MB_SUCCESS) {
      status_ = mb_pr_readpar(verbose_, file->path, false, &(file->process), &error_);
      if (!file->process.mbp_format_specified) {
        file->process.mbp_format_specified = true;
        file->process.mbp_format = file->format;
      }
    }

    /* load processed file info */
    if (status_ == MB_SUCCESS) {
      struct stat file_status;
      const int fstatus = stat(file->process.mbp_ofile, &file_status);
      if (fstatus == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
        status_ =
            mb_get_info(verbose_, file->process.mbp_ofile,
			&(file->processed_info), lonFlip_, &error_);
        if (status_ == MB_SUCCESS)
          file->processed_info_loaded = true;
      }
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::load_file(int ifile, bool assertLock) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ifile:       %d\n", ifile);
  }

  File *file;

  /* lock the file if it needs loading */
  status_ = MB_SUCCESS;
  error_ = MB_ERROR_NO_ERROR;
  if (ifile >= 0 && ifile < num_files_ && !files_[ifile].load_status &&
      files_[ifile].raw_info.nrecords > 0) {
    file = &(files_[ifile]);

    /* try to lock file */
    if (assertLock && useLockFiles_) {
      status_ = mb_pr_lockswathfile(verbose_, file->path, MBP_LOCK_EDITBATHY, program_name, &error_);
    } else {
      bool locked;
      int lock_purpose;
      mb_path lock_program = "";
      mb_path lock_user = "";
      mb_path lock_cpu = "";
      char lock_date[25] = "";
      status_ = mb_pr_lockinfo(verbose_, file->path, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                                   lock_date, &error_);

      /* if locked get lock info */
      if (error_ == MB_ERROR_FILE_LOCKED) {
        fprintf(stderr, "\nFile %s locked but lock ignored\n", file->path);
        fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
        fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
        error_ = MB_ERROR_NO_ERROR;
        status_ = MB_SUCCESS;
      }
    }

    /* if locked let the user know file can't be opened */
    if (status_ == MB_FAILURE) {
      /* turn off message */
      (*hideMessage)();

      mb_pathplusplus error1 = "";
      mb_pathplusplus error2 = "";
      mb_pathplusplus error3 = "";

      /* if locked get lock info */
      if (error_ == MB_ERROR_FILE_LOCKED) {
        // const int lock_status =
        bool locked;
        int lock_purpose;
        mb_path lock_program = "";
        mb_path lock_user = "";
        mb_path lock_cpu = "";
        char lock_date[25] = "";
         mb_pr_lockinfo(verbose_, file->path, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                                     lock_date, &error_);

        snprintf(error1, sizeof(error1), "Unable to open input file:");
        snprintf(error2, sizeof(error1), "File locked by <%s> running <%s>", lock_user, lock_program);
        snprintf(error3, sizeof(error1), "on cpu <%s> at <%s>", lock_cpu, lock_date);
        fprintf(stderr, "\nUnable to open input file:\n");
        fprintf(stderr, "  %s\n", file->path);
        fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
        fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
      }

      /* else if unable to create lock file there is a permissions problem */
      else if (error_ == MB_ERROR_OPEN_FAIL) {
        snprintf(error1, sizeof(error1), "Unable to create lock file");
        snprintf(error2, sizeof(error1), "for intended input file:");
        snprintf(error3, sizeof(error1), "-Likely permissions issue");
        fprintf(stderr, "Unable to create lock file\n");
        fprintf(stderr, "for intended input file:\n");
        fprintf(stderr, "  %s\n", file->path);
        fprintf(stderr, "-Likely permissions issue\n");
      }

      /* put up error dialog */
      (*showErrorDialog)(error1, error2, error3);
    }
  }

  /* load the file if it needs loading and has been locked */
  if (status_ == MB_SUCCESS && ifile >= 0 && ifile < num_files_ && !files_[ifile].load_status &&
      files_[ifile].raw_info.nrecords > 0) {
    file = &(files_[ifile]);

    /* allocate memory for pings */
    if (file->raw_info.nrecords > 0) {
      file->pings = (Ping *)malloc(sizeof(Ping) * (file->raw_info.nrecords + 1));
      if (file->pings != NULL) {
        file->num_pings_alloc = file->raw_info.nrecords + 1;
        memset(file->pings, 0, sizeof(Ping) * (file->num_pings_alloc));
        file->num_pings = 0;
      }
      else {
        file->num_pings_alloc = 0;
        file->num_pings = 0;
        status_ = MB_FAILURE;
        error_ = MB_ERROR_MEMORY_FAIL;
      }
    }

    mb_path swathfile = "";
    int format;
    void *imbio_ptr = NULL;
    int beams_bath;
    int beams_amp;
    int pixels_ss;

    /* open the file for reading */
    if (status_ == MB_SUCCESS) {
      /* read processed file if available, raw otherwise (fbt if possible) */
      if (file->processed_info_loaded)
        strcpy(swathfile, file->process.mbp_ofile);
      else
        strcpy(swathfile, file->path);
      format = file->format;
      file->esf_open = false;
      file->esf_changed = false;
      mb_get_shortest_path(verbose_, swathfile, &error_);

      /* use fbt file if possible */
      mb_get_fbt(verbose_, swathfile, &format, &error_);

      /* initialize reading the swath file */

      // Dave's birthday!
      int iBeginTime[] = {1962, 2, 21, 10, 30, 0, 0};
      int iEndTime[] = {2062, 2, 21, 10, 30, 0, 0};
      double beginTime, endTime;
      double lonLatBounds[] = -360., 360., -90., 90);

    
    if ((status_ = mb_read_init(verbose_, swathfile, format, 1, 
				  lonFlip_, lonLatBounds,
				  iBeginTime, iEndTime,
				  0., 1000000000., &imbio_ptr,
				  &beginTime, &endTime,
				  &beams_bath, &beams_amp, &pixels_ss,
				  &error_)) != MB_SUCCESS) {
        mb_error(verbose_, error_, &error_message);
        fprintf(stderr,
		"\nMBIO Error returned from function <mb_read_init>:\n%s\n",
		error_message);
        fprintf(stderr,
		"\nSwath sonar File <%s> not initialized for reading\n",
		file->path);
      }
    }

    char *beamflag = NULL;
    double *bath = NULL;
    double *amp = NULL;
    double *bathacrosstrack = NULL;
    double *bathalongtrack = NULL;
    double *ss = NULL;
    double *ssacrosstrack = NULL;
    double *ssalongtrack = NULL;

    /* allocate memory for data arrays */
    if (status_ == MB_SUCCESS) {
      beamflag = NULL;
      bath = NULL;
      amp = NULL;
      bathacrosstrack = NULL;
      bathalongtrack = NULL;
      ss = NULL;
      ssacrosstrack = NULL;
      ssalongtrack = NULL;
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag,
                                        &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath,
                                        &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ =
            mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                        (void **)&bathacrosstrack, &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                        (void **)&bathalongtrack, &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ =
            mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                        (void **)&ssacrosstrack, &error_);
      if (error_ == MB_ERROR_NO_ERROR)
        status_ = mb_register_array(verbose_, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                        (void **)&ssalongtrack, &error_);

      /* if error initializing memory then don't read the file */
      if (error_ != MB_ERROR_NO_ERROR) {
        mb_error(verbose_, error_, &error_message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
      }
    }

    /* set the topo_type and beamwidths */
    struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
    file->beamwidth_xtrack = imb_io_ptr->beamwidth_xtrack;
    file->beamwidth_ltrack = imb_io_ptr->beamwidth_ltrack;
    status_ = mb_sonartype(verbose_, imbio_ptr, imb_io_ptr->store_data, &file->topo_type, &error_);

    Ping *ping;
    struct stat file_status;
    FILE *afp;
    mb_path asyncfile = "";
    mb_pathplus resffile = "";
    mb_path buffer = "";
    char *result = NULL;
    mb_pathplus command = "";
    int nread;
    int n_unused;

    /* mbio read and write values */
    void *istore_ptr = NULL;
    int kind;
    double draft;
    int nbeams;
    char comment[MB_COMMENT_MAXLINE];

    int rawmodtime = 0;
    int resfmodtime = 0;

    int sensorhead = 0;
    int sensorhead_status = MB_SUCCESS;
    int sensorhead_error = MB_ERROR_NO_ERROR;
    int time_i[7];
    double mtodeglon, mtodeglat;
    double heading, sensordepth;
    double rolldelta, pitchdelta;
    int icenter, iport, istbd;
    double centerdistance, portdistance, stbddistance;
    int iping, ibeam, iedit;
    float value_float = 0.0;
    int read_size = 0;
    int index = 0;

    /* read the data */
    if (status_ == MB_SUCCESS) {
      file->num_pings = 0;
      while (error_ <= MB_ERROR_NO_ERROR) {
        /* get pointer to next ping */
        ping = &(file->pings[file->num_pings]);

        /* read a ping of data */
        status_ = mb_get_all(verbose_, imbio_ptr, &istore_ptr, &kind, ping->time_i, &ping->time_d, &ping->navlon,
                                 &ping->navlat, &ping->speed, &ping->heading, &ping->distance, &ping->altitude,
                                 &ping->sensordepth, &ping->beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                                 bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error_);

        /* ignore minor errors */
        if (kind == MB_DATA_DATA && (error_ == MB_ERROR_TIME_GAP || error_ == MB_ERROR_OUT_BOUNDS ||
                                     error_ == MB_ERROR_OUT_TIME || error_ == MB_ERROR_SPEED_TOO_SMALL)) {
          status_ = MB_SUCCESS;
          error_ = MB_ERROR_NO_ERROR;
        }

        /* check for multiplicity of pings */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          sensorhead_status = mb_sensorhead(verbose_, imbio_ptr, istore_ptr, &sensorhead, &sensorhead_error);
          if (sensorhead_status == MB_SUCCESS) {
            ping->multiplicity = sensorhead;
          }
          else if (file->num_pings > 0 && fabs(ping->time_d - file->pings[file->num_pings - 1].time_d) < MB_ESF_MAXTIMEDIFF) {
            ping->multiplicity = file->pings[file->num_pings - 1].multiplicity + 1;
          }
          else {
            ping->multiplicity = 0;
          }
        }

        /* allocate memory for pings */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          if ((ping->beamflag = (char *)malloc(ping->beams_bath)) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->beamflagorg = (char *)malloc(ping->beams_bath)) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->beamcolor = (int *)malloc(sizeof(int) * ping->beams_bath)) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bath = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->amp = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathacrosstrack = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathalongtrack = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathcorr = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathlon = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathlat = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathx = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bathy = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->angles = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->angles_forward = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->angles_null = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->ttimes = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->bheave = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if ((ping->alongtrack_offset = (double *)malloc(sizeof(double) * (ping->beams_bath))) == NULL)
            error_ = MB_ERROR_MEMORY_FAIL;
          if (error_ == MB_ERROR_MEMORY_FAIL) {
            fprintf(stderr, "MEMORY FAILURE in Backend::load_file\n");
            status_ = MB_FAILURE;
            if (ping->beamflag != NULL) {
              free(ping->beamflag);
              ping->beamflag = NULL;
            }
            if (ping->beamflagorg != NULL) {
              free(ping->beamflagorg);
              ping->beamflagorg = NULL;
            }
            if (ping->beamcolor != NULL) {
              free(ping->beamcolor);
              ping->beamcolor = NULL;
            }
            if (ping->bath != NULL) {
              free(ping->bath);
              ping->bath = NULL;
            }
            if (ping->amp != NULL) {
              free(ping->amp);
              ping->amp = NULL;
            }
            if (ping->bathacrosstrack != NULL) {
              free(ping->bathacrosstrack);
              ping->bathacrosstrack = NULL;
            }
            if (ping->bathalongtrack != NULL) {
              free(ping->bathalongtrack);
              ping->bathalongtrack = NULL;
            }
            if (ping->bathcorr != NULL) {
              free(ping->bathcorr);
              ping->bathcorr = NULL;
            }
            if (ping->bathlon != NULL) {
              free(ping->bathlon);
              ping->bathlon = NULL;
            }
            if (ping->bathlat != NULL) {
              free(ping->bathlat);
              ping->bathlat = NULL;
            }
            if (ping->bathx != NULL) {
              free(ping->bathx);
              ping->bathx = NULL;
            }
            if (ping->bathy != NULL) {
              free(ping->bathy);
              ping->bathy = NULL;
            }
            if (ping->angles != NULL) {
              free(ping->angles);
              ping->angles = NULL;
            }
            if (ping->angles_forward != NULL) {
              free(ping->angles_forward);
              ping->angles_forward = NULL;
            }
            if (ping->angles_null != NULL) {
              free(ping->angles_null);
              ping->angles_null = NULL;
            }
            if (ping->ttimes != NULL) {
              free(ping->ttimes);
              ping->ttimes = NULL;
            }
            if (ping->bheave != NULL) {
              free(ping->bheave);
              ping->bheave = NULL;
            }
            if (ping->alongtrack_offset != NULL) {
              free(ping->alongtrack_offset);
              ping->alongtrack_offset = NULL;
            }
          }
        }
        /* fprintf(stderr,"num_pings:%d ping:%p beamflags: %p
         * %p\n",file->num_pings,ping,ping->beamflag,ping->beamflagorg); */

        /* copy data into ping arrays */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_, timeLag_, &heading,
                                  &sensordepth, &rolldelta, &pitchdelta);
          mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);

          for (ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            ping->beamflag[ibeam] = beamflag[ibeam];
            ping->beamflagorg[ibeam] = beamflag[ibeam];
            ping->beamcolor[ibeam] = MBV_COLOR_BLACK;
            if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam]) &&
                (isnan(bath[ibeam]) || isnan(bathacrosstrack[ibeam]) || isnan(bathalongtrack[ibeam]))) {
              ping->beamflag[ibeam] = MB_FLAG_NULL;
              fprintf(stderr, "\nEncountered NaN value in swath data from file: %s\n", swathfile);
              fprintf(stderr, "     Ping time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ping->time_i[0],
                      ping->time_i[1], ping->time_i[2], ping->time_i[3], ping->time_i[4], ping->time_i[5],
                      ping->time_i[6]);
              fprintf(stderr, "     Beam bathymetry: %d %f %f %f\n", ibeam, ping->bath[ibeam],
                      ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam]);
            }
            if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam])) {
              /* copy bath */
              ping->bath[ibeam] = bath[ibeam];
              if (beams_amp == ping->beams_bath)
                ping->amp[ibeam] = amp[ibeam];
              else
                ping->amp[ibeam] = 0.0;
              ping->bathacrosstrack[ibeam] = bathacrosstrack[ibeam];
              ping->bathalongtrack[ibeam] = bathalongtrack[ibeam];

              /* apply rotations and calculate position */
              Backend::beam_position(ping->navlon, ping->navlat, mtodeglon, mtodeglat,
                                      ping->bath[ibeam] - ping->sensordepth, ping->bathacrosstrack[ibeam],
                                      ping->bathalongtrack[ibeam], sensordepth, rolldelta, pitchdelta, heading,
                                      &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
            }
          }
        }

        /* extract some more values */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          status_ = mb_extract_nav(verbose_, imbio_ptr, istore_ptr, &kind, ping->time_i, &ping->time_d,
                                       &ping->navlon, &ping->navlat, &ping->speed, &ping->heading, &draft, &ping->roll,
                                       &ping->pitch, &ping->heave, &error_);
        }

        /* extract some more values */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          status_ = mb_ttimes(verbose_, imbio_ptr, istore_ptr, &kind, &nbeams, ping->ttimes, ping->angles,
                                  ping->angles_forward, ping->angles_null, ping->bheave, ping->alongtrack_offset,
                                  &ping->draft, &ping->ssv, &error_);
        }

        /* get swathbounds */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
          if (format == MBF_MBPRONAV) {
            status_ = mbsys_singlebeam_swathbounds(verbose_, imbio_ptr, istore_ptr, &kind, &ping->portlon,
                                                       &ping->portlat, &ping->stbdlon, &ping->stbdlat, &error_);
            // if (ping->portlon != ping->stbdlon || ping->portlat != ping->stbdlat)
            //  swathbounds = true;
          }

          else {
            /* find centermost beam */
            icenter = -1;
            iport = -1;
            istbd = -1;
            centerdistance = 0.0;
            portdistance = 0.0;
            stbddistance = 0.0;
            for (ibeam = 0; ibeam < beams_bath; ibeam++) {
              if (!mb_beam_check_flag_unusable(beamflag[ibeam])) {
                if (icenter == -1 || fabs(bathacrosstrack[ibeam]) < centerdistance) {
                  icenter = ibeam;
                  centerdistance = bathacrosstrack[ibeam];
                }
                if (iport == -1 || bathacrosstrack[ibeam] < portdistance) {
                  iport = ibeam;
                  portdistance = bathacrosstrack[ibeam];
                }
                if (istbd == -1 || bathacrosstrack[ibeam] > stbddistance) {
                  istbd = ibeam;
                  stbddistance = bathacrosstrack[ibeam];
                }
              }
            }

            mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
            if (icenter >= 0) {
              ping->portlon = ping->bathlon[iport];
              ping->portlat = ping->bathlat[iport];
              ping->stbdlon = ping->bathlon[istbd];
              ping->stbdlat = ping->bathlat[istbd];
            }
            else {
              ping->portlon = ping->navlon;
              ping->portlat = ping->navlat;
              ping->stbdlon = ping->navlon;
              ping->stbdlat = ping->navlat;
            }
          }
        }

        /* increment counters */
        if (error_ == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
          file->num_pings++;

        /* print debug statements */
        if (verbose_ >= 2) {
          fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
          fprintf(stderr, "dbg2       kind:           %d\n", kind);
          fprintf(stderr, "dbg2       error:          %d\n", error_);
          fprintf(stderr, "dbg2       status:         %d\n", status_);
        }
        if (verbose_ >= 2 && kind == MB_DATA_COMMENT) {
          fprintf(stderr, "dbg2       comment:        %s\n", comment);
        }
        if (verbose_ >= 2 && error_ <= 0 && kind == MB_DATA_DATA) {
          fprintf(stderr, "dbg2       time_i:         %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n", ping->time_i[0],
                  ping->time_i[1], ping->time_i[2], ping->time_i[3], ping->time_i[4], ping->time_i[5], ping->time_i[6]);
          fprintf(stderr, "dbg2       time_d:         %f\n", ping->time_d);
          fprintf(stderr, "dbg2       navlon:         %f\n", ping->navlon);
          fprintf(stderr, "dbg2       navlat:         %f\n", ping->navlat);
          fprintf(stderr, "dbg2       speed:          %f\n", ping->speed);
          fprintf(stderr, "dbg2       heading:        %f\n", ping->heading);
          fprintf(stderr, "dbg2       distance:       %f\n", ping->distance);
          fprintf(stderr, "dbg2       beams_bath:     %d\n", ping->beams_bath);
          fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
          fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
        }
      }

      /* close the file */
      status_ = mb_close(verbose_, &imbio_ptr, &error_);

      /* if processed file read, then reset the beam edits to the original raw state
       * by reading in an *.resf (reverse edit save file) generated by mbprocess */
      if (file->processed_info_loaded) {
        /* check if reverse edit save file (*.resf) exists and is up to date */
        if (stat(file->path, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
          rawmodtime = file_status.st_mtime;
        else
          rawmodtime = 0;
        snprintf(resffile, sizeof(resffile), "%s.resf", file->path);
        if (stat(resffile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR)
          resfmodtime = file_status.st_mtime;
        else
          resfmodtime = 0;
        if (rawmodtime >= resfmodtime) {
          snprintf(command, sizeof(command), "mbprocess -I %s -P", file->path);
          fprintf(stderr, "Generating *.resf file by rerunning mbprocess:\n\t%s\n", command);
          /* const int shellstatus = */ system(command);
        }

        /* now read and apply the reverse edits */
        status_ = mb_esf_open(verbose_, program_name, resffile,
                      true, MBP_ESF_NOWRITE, &(file->esf),
                      &error_);
        if (status_ == MB_SUCCESS) {
          file->esf_open = true;
          num_esf_open_++;
          if (verbose_ > 0)
            fprintf(stderr, "%d reverse edits read from %s...\n", file->esf.nedit, resffile);
        }
        else {
          file->esf_open = false;
          status_ = MB_SUCCESS;
          error_ = MB_ERROR_NO_ERROR;
        }
        if (file->esf_open) {
          /* loop over pings applying edits */
          (*showMessage)("MBeditviz is recreating original beam states...");
          if (verbose_ > 0)
            fprintf(stderr, "MBeditviz is applying %d reverse edits\n", file->esf.nedit);
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = &(file->pings[iping]);

            /* apply edits for this ping */
            mb_esf_apply(verbose_, &(file->esf), ping->time_d, ping->multiplicity, ping->beams_bath,
                         ping->beamflag, &error_);
            for (ibeam = 0; ibeam < ping->beams_bath; ibeam++)
              ping->beamflagorg[ibeam] = ping->beamflag[ibeam];

            /* update message every 250 records */
            if (iping % 250 == 0) {
              snprintf(message, sizeof(message), "MBeditviz: reverse edits applied to %d of %d records so far...", iping,
                      file->num_pings);
              (*showMessage)(message);
            }
          }

          /* close the esf */
          if (file->esf_open) {
            mb_esf_close(verbose_, &file->esf, &error_);
            file->esf_open = false;
            num_esf_open_--;
          }
        }
      }

      /* attempt to load bathymetry edits */
      status_ = mb_esf_load(verbose_, program_name, file->path, true, MBP_ESF_NOWRITE, file->esffile,
                                &(file->esf), &error_);
      if (status_ == MB_SUCCESS) {
        file->esf_open = true;
        num_esf_open_++;
      }
      else {
        file->esf_open = false;
        status_ = MB_SUCCESS;
        error_ = MB_ERROR_NO_ERROR;
      }
      if (file->esf_open) {
        /* loop over pings applying edits */
        if (verbose_ > 0)
          fprintf(stderr, "MBeditviz is applying %d saved edits from version %d esf file %s\n", file->esf.nedit,
                  file->esf.version, file->path);
        (*showMessage)("MBeditviz is applying saved edits...");
        for (iping = 0; iping < file->num_pings; iping++) {
          ping = &(file->pings[iping]);

          /* apply edits for this ping */
          mb_esf_apply(verbose_, &(file->esf), ping->time_d, ping->multiplicity, ping->beams_bath, ping->beamflag,
                       &error_);
          for (ibeam = 0; ibeam < ping->beams_bath; ibeam++)
            ping->beamflagorg[ibeam] = ping->beamflag[ibeam];

          /* update message every 250 records */
          if (iping % 250 == 0) {
            snprintf(message, sizeof(message), "MBeditviz: saved edits applied to %d of %d records so far...", iping, file->num_pings);
            (*showMessage)(message);
          }
        }

        /* check for unused edits */
        n_unused = 0;
        for (iedit = 0; iedit < file->esf.nedit; iedit++) {
          if (file->esf.edit[iedit].use == 0) {
            n_unused++;
            mb_get_date(verbose_, file->esf.edit[iedit].time_d, time_i);
            fprintf(stderr, "Unused beam edit: %f %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d beam:%d action:%d\n",
                    file->esf.edit[iedit].time_d, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
                    time_i[6], file->esf.edit[iedit].beam, file->esf.edit[iedit].action);
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Total unused beam edits for file %s: %d\n", swathfile, n_unused);

        /* close the esf */
        if (file->esf_open) {
          mb_esf_close(verbose_, &file->esf, &error_);
          file->esf_open = false;
          num_esf_open_--;
        }
      }
    }

    /* load asynchronous data if available */
    if (status_ == MB_SUCCESS) {
      /* try to load asynchronous heading data from .bah file */
      strcpy(asyncfile, file->path);
      strcat(asyncfile, ".bah");
      if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR &&
          file_status.st_size > 0) {
        /* allocate space for asynchronous heading */
        file->n_async_heading = file_status.st_size / (sizeof(double) + sizeof(float));
        file->n_async_heading_alloc = 0;
        if (file->n_async_heading > 0) {
          if ((file->async_heading_time_d = (double *)malloc(sizeof(double) * (file->n_async_heading))) != NULL &&
              (file->async_heading_heading = (double *)malloc(sizeof(double) * (file->n_async_heading))) != NULL) {
            file->n_async_heading_alloc = file->n_async_heading;
          }
          else if (file->async_heading_time_d != NULL) {
            free(file->async_heading_time_d);
            file->async_heading_time_d = NULL;
          }
        }
        file->n_async_heading = file->n_async_heading_alloc;

        /* read the asynchronous heading data */
        if ((afp = fopen(asyncfile, "r")) != NULL) {
          read_size = sizeof(double) + sizeof(float);
          for (int i = 0; i < file->n_async_heading; i++) {
            fread(buffer, read_size, 1, afp);
            index = 0;
            mb_get_binary_double(true, &buffer[index], &file->async_heading_time_d[i]);
            index += 8;
            mb_get_binary_float(true, &buffer[index], &value_float);
            // index += 4;
            file->async_heading_heading[i] = value_float;
          }
          fclose(afp);
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d heading data from file %s\n", file->n_async_heading, asyncfile);
      }

      /* if necessary try to load heading data from ath file */
      if (file->n_async_heading <= 0) {
        strcpy(asyncfile, file->path);
        strcat(asyncfile, ".ath");
        if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          /* count the asynchronous heading data */
          file->n_async_heading = 0;
          file->n_async_heading_alloc = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer)
              if (buffer[0] != '#')
                file->n_async_heading++;
            fclose(afp);
          }

          /* allocate space for asynchronous heading */
          if (file->n_async_heading > 0) {
            if ((file->async_heading_time_d = (double *)malloc(sizeof(double) * (file->n_async_heading))) != NULL) {
              if ((file->async_heading_heading = (double *)malloc(sizeof(double) * (file->n_async_heading))) !=
                  NULL) {
                file->n_async_heading_alloc = file->n_async_heading;
              }
              else if (file->async_heading_time_d != NULL) {
                free(file->async_heading_time_d);
                file->async_heading_time_d = NULL;
              }
            }
          }

          /* read the asynchronous heading data */
          file->n_async_heading = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer) {
              if (buffer[0] != '#') {
                nread = sscanf(buffer, "%lf %lf", &(file->async_heading_time_d[file->n_async_heading]),
                               &(file->async_heading_heading[file->n_async_heading]));
                if (nread == 2)
                  file->n_async_heading++;
              }
            }
            fclose(afp);
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d heading data from file %s\n", file->n_async_heading, asyncfile);
      }

      /* if heading data not loaded from file extract from ping data */
      if (file->n_async_heading <= 0) {
        if (file->num_pings > 0) {
          if ((file->async_heading_time_d = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
            if ((file->async_heading_heading = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
              file->n_async_heading = file->num_pings;
              file->n_async_heading_alloc = file->n_async_heading;
            }
            else if (file->async_heading_time_d != NULL) {
              free(file->async_heading_time_d);
              file->async_heading_time_d = NULL;
            }
          }
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = &(file->pings[iping]);
            file->async_heading_time_d[iping] = ping->time_d;
            file->async_heading_heading[iping] = ping->heading;
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d heading data from ping data of file %s\n", file->n_async_heading, file->path);
      }

      /* try to load asynchronous sensordepth data from .bas file */
      strcpy(asyncfile, file->path);
      strcat(asyncfile, ".bas");
      if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR &&
          file_status.st_size > 0) {
        /* allocate space for asynchronous sensordepth */
        file->n_async_sensordepth = file_status.st_size / (sizeof(double) + sizeof(float));
        file->n_async_sensordepth_alloc = 0;
        if (file->n_async_sensordepth > 0) {
          if ((file->async_sensordepth_time_d = (double *)malloc(sizeof(double) * (file->n_async_sensordepth))) != NULL &&
              (file->async_sensordepth_sensordepth = (double *)malloc(sizeof(double) * (file->n_async_sensordepth))) !=
                  NULL) {
            file->n_async_sensordepth_alloc = file->n_async_sensordepth;
          }
          else if (file->async_sensordepth_time_d != NULL) {
            free(file->async_sensordepth_time_d);
            file->async_sensordepth_time_d = NULL;
          }
        }
        file->n_async_sensordepth = file->n_async_sensordepth_alloc;

        /* read the asynchronous sensordepth data */
        if ((afp = fopen(asyncfile, "rb")) != NULL) {
          read_size = sizeof(double) + sizeof(float);
          for (int i = 0; i < file->n_async_sensordepth; i++) {
            fread(buffer, read_size, 1, afp);
            index = 0;
            mb_get_binary_double(true, &buffer[index], &file->async_sensordepth_time_d[i]);
            index += 8;
            mb_get_binary_float(true, &buffer[index], &value_float);
            // index += 4;
            file->async_sensordepth_sensordepth[i] = value_float;
          }
          fclose(afp);
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d sensordepth data from file %s\n", file->n_async_sensordepth, asyncfile);
      }

      /* if necessary try to load sensordepth data from ats file */
      if (file->n_async_heading <= 0) {
        strcpy(asyncfile, file->path);
        strcat(asyncfile, ".ats");
        if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          /* count the asynchronous sensordepth data */
          file->n_async_sensordepth = 0;
          file->n_async_sensordepth_alloc = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer)
              if (buffer[0] != '#')
                file->n_async_sensordepth++;
            fclose(afp);
          }

          /* allocate space for asynchronous sensordepth */
          if (file->n_async_sensordepth > 0) {
            if ((file->async_sensordepth_time_d = (double *)malloc(sizeof(double) * (file->n_async_sensordepth))) !=
                NULL) {
              if ((file->async_sensordepth_sensordepth =
                       (double *)malloc(sizeof(double) * (file->n_async_sensordepth))) != NULL) {
                file->n_async_sensordepth_alloc = file->n_async_sensordepth;
              }
              else if (file->async_sensordepth_time_d != NULL) {
                free(file->async_sensordepth_time_d);
                file->async_sensordepth_time_d = NULL;
              }
            }
          }

          /* read the asynchronous sensordepth data */
          file->n_async_sensordepth = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer) {
              if (buffer[0] != '#') {
                nread = sscanf(buffer, "%lf %lf", &(file->async_sensordepth_time_d[file->n_async_sensordepth]),
                               &(file->async_sensordepth_sensordepth[file->n_async_sensordepth]));
                if (nread == 2)
                  file->n_async_sensordepth++;
              }
            }
            fclose(afp);
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d sensordepth data from file %s\n", file->n_async_sensordepth, asyncfile);
      }

      /* if sensordepth data not loaded from file extract from ping data */
      if (file->n_async_sensordepth <= 0) {
        if (file->num_pings > 0) {
          if ((file->async_sensordepth_time_d = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
            if ((file->async_sensordepth_sensordepth = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
              file->n_async_sensordepth = file->num_pings;
              file->n_async_sensordepth_alloc = file->n_async_sensordepth;
            }
            else if (file->async_sensordepth_time_d != NULL) {
              free(file->async_sensordepth_time_d);
              file->async_sensordepth_time_d = NULL;
            }
          }
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = &(file->pings[iping]);
            file->async_sensordepth_time_d[iping] = ping->time_d;
            file->async_sensordepth_sensordepth[iping] = ping->sensordepth;
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d sensordepth data from ping data of file %s\n", file->n_async_sensordepth,
                  file->path);
      }

      /* try to load asynchronous attitude data from .baa file */
      strcpy(asyncfile, file->path);
      strcat(asyncfile, ".baa");
      if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR &&
          file_status.st_size > 0) {
        /* allocate space for asynchronous attitude */
        file->n_async_attitude = file_status.st_size / (sizeof(double) + 2 * sizeof(float));
        file->n_async_attitude_alloc = 0;
        if (file->n_async_attitude > 0) {
          if ((file->async_attitude_time_d = (double *)malloc(sizeof(double) * (file->n_async_attitude))) != NULL &&
              (file->async_attitude_roll = (double *)malloc(sizeof(double) * (file->n_async_attitude))) != NULL &&
              (file->async_attitude_pitch = (double *)malloc(sizeof(double) * (file->n_async_attitude))) != NULL) {
            file->n_async_attitude_alloc = file->n_async_attitude;
          }
          else {
            if (file->async_attitude_time_d != NULL) {
              free(file->async_attitude_time_d);
              file->async_attitude_time_d = NULL;
            }
            if (file->async_attitude_roll != NULL) {
              free(file->async_attitude_roll);
              file->async_attitude_roll = NULL;
            }
          }
        }
        file->n_async_attitude = file->n_async_attitude_alloc;

        /* read the asynchronous attitude data */
        if ((afp = fopen(asyncfile, "rb")) != NULL) {
          read_size = sizeof(double) + 2 * sizeof(float);
          for (int i = 0; i < file->n_async_attitude; i++) {
            if (fread(buffer, read_size, 1, afp) == 1) {
              index = 0;
              mb_get_binary_double(true, &buffer[index], &file->async_attitude_time_d[i]);
              index += 8;
              mb_get_binary_float(true, &buffer[index], &value_float);
              index += 4;
              file->async_attitude_roll[i] = value_float;
              mb_get_binary_float(true, &buffer[index], &value_float);
              // index += 4;
              file->async_attitude_pitch[i] = value_float;
            }
            // fprintf(stderr,"Attitude: %d  %.6f %.3f
            // %.3f\n",i,file->async_attitude_time_d[i],file->async_attitude_roll[i],file->async_attitude_pitch[i]);
          }
          fclose(afp);
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from file %s\n", file->n_async_attitude, asyncfile);
      }

      /* if necessary try to load asynchronous attitude data from ata file */
      if (file->n_async_attitude <= 0) {
        strcpy(asyncfile, file->path);
        strcat(asyncfile, ".ata");
        if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          /* count the asynchronous attitude data */
          file->n_async_attitude = 0;
          file->n_async_attitude_alloc = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer)
              if (buffer[0] != '#')
                file->n_async_attitude++;
            fclose(afp);
          }

          /* allocate space for asynchronous attitude */
          if (file->n_async_attitude > 0) {
            if ((file->async_attitude_time_d = (double *)malloc(sizeof(double) * (file->n_async_attitude))) != NULL) {
              if ((file->async_attitude_roll = (double *)malloc(sizeof(double) * (file->n_async_attitude))) !=
                  NULL) {
                if ((file->async_attitude_pitch = (double *)malloc(sizeof(double) * (file->n_async_attitude))) !=
                    NULL) {
                  file->n_async_attitude_alloc = file->n_async_attitude;
                }
                else {
                  if (file->async_attitude_time_d != NULL) {
                    free(file->async_attitude_time_d);
                    file->async_attitude_time_d = NULL;
                  }
                  if (file->async_attitude_roll != NULL) {
                    free(file->async_attitude_roll);
                    file->async_attitude_roll = NULL;
                  }
                }
              }
              else if (file->async_attitude_time_d != NULL) {
                free(file->async_attitude_time_d);
                file->async_attitude_time_d = NULL;
              }
            }
          }

          /* read the asynchronous attitude data */
          file->n_async_attitude = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer) {
              if (buffer[0] != '#') {
                nread = sscanf(buffer, "%lf %lf %lf", &(file->async_attitude_time_d[file->n_async_attitude]),
                               &(file->async_attitude_roll[file->n_async_attitude]),
                               &(file->async_attitude_pitch[file->n_async_attitude]));
                if (nread == 3)
                  file->n_async_attitude++;
              }
            }
            fclose(afp);
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from file %s\n", file->n_async_attitude, asyncfile);
      }

      /* if attitude data not loaded from file extract from ping data */
      if (file->n_async_attitude <= 0) {
        if (file->num_pings > 0) {
          if ((file->async_attitude_time_d = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
            if ((file->async_attitude_roll = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
              if ((file->async_attitude_pitch = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
                file->n_async_attitude = file->num_pings;
                file->n_async_attitude_alloc = file->n_async_attitude;
              }
              else {
                if (file->async_attitude_time_d != NULL) {
                  free(file->async_attitude_time_d);
                  file->async_attitude_time_d = NULL;
                }
                if (file->async_attitude_roll != NULL) {
                  free(file->async_attitude_roll);
                  file->async_attitude_roll = NULL;
                }
              }
            }
            else if (file->async_attitude_time_d != NULL) {
              free(file->async_attitude_time_d);
              file->async_attitude_time_d = NULL;
            }
          }
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = &(file->pings[iping]);
            file->async_attitude_time_d[iping] = ping->time_d;
            file->async_attitude_roll[iping] = ping->roll;
            file->async_attitude_pitch[iping] = ping->pitch;
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from ping data of file %s\n", file->n_async_attitude, file->path);
      }

      /* try to load synchronous attitude data from .bsa file */
      strcpy(asyncfile, file->path);
      strcat(asyncfile, ".bsa");
      if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR &&
          file_status.st_size > 0) {
        /* allocate space for synchronous attitude */
        file->n_sync_attitude = file_status.st_size / (sizeof(double) + 2 * sizeof(float));
        file->n_sync_attitude_alloc = 0;
        if (file->n_sync_attitude > 0) {
          if ((file->sync_attitude_time_d = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) != NULL &&
              (file->sync_attitude_roll = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) != NULL &&
              (file->sync_attitude_pitch = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) != NULL) {
            file->n_sync_attitude_alloc = file->n_sync_attitude;
          }
          else {
            if (file->sync_attitude_time_d != NULL) {
              free(file->sync_attitude_time_d);
              file->sync_attitude_time_d = NULL;
            }
            if (file->sync_attitude_roll != NULL) {
              free(file->sync_attitude_roll);
              file->sync_attitude_roll = NULL;
            }
          }
        }
        file->n_sync_attitude = file->n_sync_attitude_alloc;

        /* read the synchronous attitude data */
        if ((afp = fopen(asyncfile, "rb")) != NULL) {
          read_size = sizeof(double) + 2 * sizeof(float);
          for (int i = 0; i < file->n_sync_attitude; i++) {
            if (fread(buffer, read_size, 1, afp) == 1) {
              index = 0;
              mb_get_binary_double(true, &buffer[index], &file->sync_attitude_time_d[i]);
              index += 8;
              mb_get_binary_float(true, &buffer[index], &value_float);
              index += 4;
              file->sync_attitude_roll[i] = value_float;
              mb_get_binary_float(true, &buffer[index], &value_float);
              // index += 4;
              file->sync_attitude_pitch[i] = value_float;
            }
          }
          fclose(afp);
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from file %s\n", file->n_sync_attitude, asyncfile);
      }

      /* if necessary try to load synchronous attitude data from sta file */
      if (file->n_sync_attitude <= 0) {
        strcpy(asyncfile, file->path);
        strcat(asyncfile, ".sta");
        if (stat(asyncfile, &file_status) == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          /* count the synchronous attitude data */
          file->n_sync_attitude = 0;
          file->n_sync_attitude_alloc = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer)
              if (buffer[0] != '#')
                file->n_sync_attitude++;
            fclose(afp);
          }

          /* allocate space for synchronous attitude */
          if (file->n_sync_attitude > 0) {
            if ((file->sync_attitude_time_d = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) != NULL) {
              if ((file->sync_attitude_roll = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) != NULL) {
                if ((file->sync_attitude_pitch = (double *)malloc(sizeof(double) * (file->n_sync_attitude))) !=
                    NULL) {
                  file->n_sync_attitude_alloc = file->n_sync_attitude;
                }
                else {
                  if (file->sync_attitude_time_d != NULL) {
                    free(file->sync_attitude_time_d);
                    file->sync_attitude_time_d = NULL;
                  }
                  if (file->sync_attitude_roll != NULL) {
                    free(file->sync_attitude_roll);
                    file->sync_attitude_roll = NULL;
                  }
                }
              }
              else if (file->sync_attitude_time_d != NULL) {
                free(file->sync_attitude_time_d);
                file->sync_attitude_time_d = NULL;
              }
            }
          }

          /* read the synchronous attitude data */
          file->n_sync_attitude = 0;
          if ((afp = fopen(asyncfile, "r")) != NULL) {
            while ((result = fgets(buffer, MBP_FILENAMESIZE, afp)) == buffer) {
              if (buffer[0] != '#') {
                nread = sscanf(buffer, "%lf %lf %lf", &(file->sync_attitude_time_d[file->n_sync_attitude]),
                               &(file->sync_attitude_roll[file->n_sync_attitude]),
                               &(file->sync_attitude_pitch[file->n_sync_attitude]));
                if (nread == 3)
                  file->n_sync_attitude++;
              }
            }
            fclose(afp);
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from file %s\n", file->n_sync_attitude, asyncfile);
      }

      /* if attitude data not loaded from file extract from ping data */
      if (file->n_sync_attitude <= 0) {
        if (file->num_pings > 0) {
          if ((file->sync_attitude_time_d = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
            if ((file->sync_attitude_roll = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
              if ((file->sync_attitude_pitch = (double *)malloc(sizeof(double) * (file->num_pings))) != NULL) {
                file->n_sync_attitude = file->num_pings;
                file->n_sync_attitude_alloc = file->n_sync_attitude;
              }
              else {
                if (file->sync_attitude_time_d != NULL) {
                  free(file->sync_attitude_time_d);
                  file->sync_attitude_time_d = NULL;
                }
                if (file->sync_attitude_roll != NULL) {
                  free(file->sync_attitude_roll);
                  file->sync_attitude_roll = NULL;
                }
              }
            }
            else if (file->sync_attitude_time_d != NULL) {
              free(file->sync_attitude_time_d);
              file->sync_attitude_time_d = NULL;
            }
          }
          for (iping = 0; iping < file->num_pings; iping++) {
            ping = &(file->pings[iping]);
            file->sync_attitude_time_d[iping] = ping->time_d;
            file->sync_attitude_roll[iping] = ping->roll;
            file->sync_attitude_pitch[iping] = ping->pitch;
          }
        }
        if (verbose_ > 0)
          fprintf(stderr, "Loaded %d attitude data from ping data of file %s\n", file->n_sync_attitude, file->path);
      }
    }

    if (verbose_ > 0)
      fprintf(stderr, "loaded swathfile:%s file->processed_info_loaded:%d file->process.mbp_edit_mode:%d\n\n", swathfile,
              file->processed_info_loaded, file->process.mbp_edit_mode);
    else
      fprintf(stderr, "loaded swathfile:%s\n", swathfile);

    /* set the load status */
    if (status_ == MB_SUCCESS) {
      file->load_status = true;
      num_files__loaded++;
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::apply_biasesandtimelag(File *file, Ping *ping,
				    double rollbias, double pitchbias,
				    double headingbias, double timelag,
				    double *heading, double *sensordepth,
				    double *rolldelta,
				    double *pitchdelta) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       file:        %p\n", file);
    fprintf(stderr, "dbg2       ping:        %p\n", ping);
    fprintf(stderr, "dbg2       rollbias:    %f\n", rollbias);
    fprintf(stderr, "dbg2       pitchbias:   %f\n", pitchbias);
    fprintf(stderr, "dbg2       headingbias: %f\n", headingbias);
    fprintf(stderr, "dbg2       timelag:     %f\n", timelag);
  }

  double time_d;
  int iheading = 0;
  int isensordepth = 0;
  int iattitude = 0;
  double rollasync, pitchasync, headingasync;

  /* apply timelag to get new values of lon, lat, heading, rollbias, pitchbias */
  if (file != NULL && ping != NULL) {
    /* get adjusted time for interpolation in asyncronous time series */
    time_d = ping->time_d + timelag;

    /* if asyncronous sensordepth available, interpolate new value */
    // int intstat;
    if (timelag != 0.0 && file->n_async_sensordepth > 0) {
      /* intstat = */ mb_linear_interp(verbose_, file->async_sensordepth_time_d - 1, file->async_sensordepth_sensordepth - 1,
                                 file->n_async_sensordepth, time_d, sensordepth, &isensordepth, &error_);
    }
    else {
      *sensordepth = ping->sensordepth;
    }

    /* if asyncronous heading available, interpolate new value */
    if (timelag != 0.0 && file->n_async_heading > 0) {
      /* intstat = */ mb_linear_interp_heading(verbose_, file->async_heading_time_d - 1, file->async_heading_heading - 1,
                                         file->n_async_heading, time_d, &headingasync, &iheading, &error_);
    }
    else {
      headingasync = ping->heading;
    }

    /* if asynchronous roll and pitch available, interpolate new values */
    if (timelag != 0.0 && file->n_async_attitude > 0) {
      /* intstat = */ mb_linear_interp(verbose_, file->async_attitude_time_d - 1, file->async_attitude_roll - 1,
                                 file->n_async_attitude, time_d, &rollasync, &iattitude, &error_);
      /* intstat = */ mb_linear_interp(verbose_, file->async_attitude_time_d - 1, file->async_attitude_pitch - 1,
                                 file->n_async_attitude, time_d, &pitchasync, &iattitude, &error_);
    }
    else {
      rollasync = ping->roll;
      pitchasync = ping->pitch;
    }

    /* Calculate attitude delta altogether */
    mb_platform_math_attitude_offset_corrected_by_nav(
        verbose_, ping->roll, ping->pitch, 0.0, // In: Old Pitch and Roll applied
        rollbias, pitchbias, headingbias,           // In: New Bias to apply
        rollasync, pitchasync, headingasync,        // In: New nav attitude to apply
        rolldelta, pitchdelta, heading,             // Out: Calculated rolldelta, pitchdelta and heading
        &error_);

    /*
    fprintf(stderr,"sensordepth: %f %f   %f %d\n", *sensordepth, ping->sensordepth,*sensordepth-ping->sensordepth, isensordepth);
    fprintf(stderr,"heading:    %f %f   %f  %f %d\n", *heading, headingasync, ping->heading, *heading-ping->heading,
    iheading); fprintf(stderr,"rolldelta:  %f %f   roll:%f %f   %d\n", *rolldelta, rollbias, rollasync, ping->roll,
    iattitude); fprintf(stderr,"pitchdelta: %f %f   pitch:%f %f   %d\n", *pitchdelta, pitchbias, pitchasync, ping->pitch,
    iattitude);*/
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2       heading:    %f\n", *heading);
    fprintf(stderr, "dbg2       sensordepth: %f\n", *sensordepth);
    fprintf(stderr, "dbg2       rolldelta:  %f\n", *rolldelta);
    fprintf(stderr, "dbg2       pitchdelta: %f\n", *pitchdelta);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/

int Backend::snell_correction(double snell, double roll, double *beam_xtrack,
                 double *beam_ltrack, double *beam_z) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       snell:       %f\n", snell);
    fprintf(stderr, "dbg2       roll:        %f\n", roll);
    fprintf(stderr, "dbg2       beam_xtrack: %f\n", *beam_xtrack);
    fprintf(stderr, "dbg2       beam_ltrack: %f\n", *beam_ltrack);
    fprintf(stderr, "dbg2       beam_z:      %f\n", *beam_z);
  }

  double range, alphar, betar;

  /* if beamforming sound speed correction to be applied */
  if (snell != 1.0) {
    /* check for bad input values */
    if (isnan(snell) || isinf(snell)
      || isnan(roll) || isinf(roll)
      || isnan(*beam_xtrack) || isinf(*beam_xtrack)
      || isnan(*beam_ltrack) || isinf(*beam_ltrack)
      || isnan(*beam_z) || isinf(*beam_z)) {
    fprintf(stderr,"\nNaN or Inf input in Backend::snell_correction: snell:%f roll:%f BEAM: %f %f %f\n",
        snell, roll, *beam_xtrack, *beam_ltrack, *beam_z);
    }
    /* get range and angles in
        roll-pitch frame */
    range = sqrt((*beam_xtrack) * (*beam_xtrack) + (*beam_ltrack) * (*beam_ltrack) + (*beam_z) * (*beam_z));
    if (isnan(range) || isinf(range)
      || isnan(*beam_xtrack) || isinf(*beam_xtrack)
      || isnan(*beam_ltrack) || isinf(*beam_ltrack)
      || isnan(*beam_z) || isinf(*beam_z)) {
      fprintf(stderr,"NaN range in Backend::snell_correction: range:%f BEAM: %f %f %f\n",
          range, *beam_xtrack, *beam_ltrack, *beam_z);
    }
    if (fabs(range) < 0.001) {
      alphar = 0.0;
      betar = 0.5 * M_PI;
    }
    else {
      alphar = asin(MAX(-1.0, MIN(1.0, ((*beam_ltrack) / range))));
      betar = acos(MAX(-1.0, MIN(1.0, ((*beam_xtrack) / range / cos(alphar)))));
    }
    if ((*beam_z) < 0.0)
      betar = 2.0 * M_PI - betar;
//fprintf(stderr,"Backend::mb3dsoundings_bias: ifile:%d iping:%d ibeam:%d  XYZR: %f %f %f %f   alphar:%f betar:%f ",
//ifile,iping,ibeam,(*beam_xtrack),beam_ltrack,beam_z,range,alphar,betar);

    /* subtract off the roll + roll correction */
    betar -= DTR * roll;

    /* apply the beamforming sound speed correction using Snell's law */
//fprintf(stderr,"| ping->roll:%f rolldelta:%f betar:%f |",
//ping->roll,rolldelta,betar);
    betar = asin(MAX(-1.0, MIN(1.0, snell * sin(betar - 0.5 * M_PI)))) + 0.5 * M_PI;

    /* add back in the roll + roll correction */
    betar += DTR * roll;

    /* recalculate bathymetry  using new angles */
    *beam_ltrack = range * sin(alphar);
    *beam_xtrack = range * cos(alphar) * cos(betar);
    *beam_z = range * cos(alphar) * sin(betar);
//range = sqrt(beam_xtrack * beam_xtrack + beam_ltrack * beam_ltrack + beam_z * beam_z);
//fprintf(stderr," snell_:%f  betar:%f XYZR: %f %f %f %f \n",snell_,betar,beam_xtrack,beam_ltrack,beam_z,range);
    if (isnan(*beam_xtrack) || isinf(*beam_xtrack)
      || isnan(*beam_ltrack) || isinf(*beam_ltrack)
      || isnan(*beam_z) || isinf(*beam_z)) {
    fprintf(stderr,"NaN result in Backend::snell_correction: range:%f alphar:%f %f betar:%f %f   BEAM: %f %f %f\n",
        range, alphar, RTD*alphar, betar, RTD*betar, *beam_xtrack, *beam_ltrack, *beam_z);
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:          %d\n", error_);
    fprintf(stderr, "dbg2       beam_xtrack:    %f\n", *beam_xtrack);
    fprintf(stderr, "dbg2       beam_ltrack:    %f\n", *beam_ltrack);
    fprintf(stderr, "dbg2       labeam_zt:      %f\n", *beam_z);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:    %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/

int Backend::beam_position(double navlon, double navlat, double mtodeglon, double mtodeglat, double rawbath, double acrosstrack,
                            double alongtrack, double sensordepth, double rolldelta, double pitchdelta, double heading,
                            double *bathcorr, double *lon, double *lat) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       navlon:      %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:      %f\n", navlat);
    fprintf(stderr, "dbg2       mtodeglon:   %f\n", mtodeglon);
    fprintf(stderr, "dbg2       mtodeglat:   %f\n", mtodeglat);
    fprintf(stderr, "dbg2       rawbath:     %f\n", rawbath);
    fprintf(stderr, "dbg2       acrosstrack: %f\n", acrosstrack);
    fprintf(stderr, "dbg2       alongtrack:  %f\n", alongtrack);
    fprintf(stderr, "dbg2       sensordepth:  %f\n", sensordepth);
    fprintf(stderr, "dbg2       rolldelta:   %f\n", rolldelta);
    fprintf(stderr, "dbg2       pitchdelta:  %f\n", pitchdelta);
    fprintf(stderr, "dbg2       heading:     %f\n", heading);
  }

  /* initial sounding rawbath is relative to sensor (sensor depth subtracted)
   * rotate sounding by
       rolldelta:  Roll relative to previous correction and bias included
       pitchdelta: Pitch relative to previous correction and bias included
       heading:    Heading absolute (bias included) */
  double newbath;
  double neweasting;
  double newnorthing;
  mb_platform_math_attitude_rotate_beam(verbose_, acrosstrack, alongtrack, rawbath, rolldelta, pitchdelta, heading,
                                        &neweasting, &newnorthing, &newbath, &error_);

  /* add sensordepth to get full corrected bathymetry */
  *bathcorr = newbath + sensordepth;

  /* locate lon lat position */
  *lon = navlon + mtodeglon * neweasting;
  *lat = navlat + mtodeglat * newnorthing;

  if (isnan(*bathcorr) || isinf(*bathcorr)) {
    fprintf(stderr, "\nFunction Backend::beam_position(): Calculated NaN bathcorr\n");
    fprintf(stderr, "     navlon:      %f\n", navlon);
    fprintf(stderr, "     navlat:      %f\n", navlat);
    fprintf(stderr, "     mtodeglon:   %f\n", mtodeglon);
    fprintf(stderr, "     mtodeglat:   %f\n", mtodeglat);
    fprintf(stderr, "     bath:        %f\n", rawbath);
    fprintf(stderr, "     acrosstrack: %f\n", acrosstrack);
    fprintf(stderr, "     alongtrack:  %f\n", alongtrack);
    fprintf(stderr, "     sensordepth:  %f\n", sensordepth);
    fprintf(stderr, "     rolldelta:   %f\n", rolldelta);
    fprintf(stderr, "     pitchdelta:  %f\n", pitchdelta);
    fprintf(stderr, "     heading:     %f\n", heading);
    fprintf(stderr, "     newbath:     %f\n", newbath);
    fprintf(stderr, "     bathcorr:    %f\n", *bathcorr);
    fprintf(stderr, "     lon:         %f\n", *lon);
    fprintf(stderr, "     lat:         %f\n", *lat);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2       bathcorr:    %f\n", *bathcorr);
    fprintf(stderr, "dbg2       lon:         %f\n", *lon);
    fprintf(stderr, "dbg2       lat:         %f\n", *lat);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::unload_file(int ifile, bool assertUnlock) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ifile:       %d\n", ifile);
  }

  /* unload the file */
  if (ifile >= 0 && ifile < num_files_ && files_[ifile].load_status) {

    Ping *ping;
    int lock_error = MB_ERROR_NO_ERROR;

    /* release memory */
    File *file = &(files_[ifile]);
    if (file->pings != NULL) {
      for (int iping = 0; iping < file->num_pings; iping++) {
        ping = &(file->pings[iping]);
        if (ping->beamflag != NULL) {
          free(ping->beamflag);
          ping->beamflag = NULL;
        }
        if (ping->beamflagorg != NULL) {
          free(ping->beamflagorg);
          ping->beamflagorg = NULL;
        }
        if (ping->beamcolor != NULL) {
          free(ping->beamcolor);
          ping->beamcolor = NULL;
        }
        if (ping->bath != NULL) {
          free(ping->bath);
          ping->bath = NULL;
        }
        if (ping->amp != NULL) {
          free(ping->amp);
          ping->amp = NULL;
        }
        if (ping->bathacrosstrack != NULL) {
          free(ping->bathacrosstrack);
          ping->bathacrosstrack = NULL;
        }
        if (ping->bathalongtrack != NULL) {
          free(ping->bathalongtrack);
          ping->bathalongtrack = NULL;
        }
        if (ping->bathcorr != NULL) {
          free(ping->bathcorr);
          ping->bathcorr = NULL;
        }
        if (ping->bathlon != NULL) {
          free(ping->bathlon);
          ping->bathlon = NULL;
        }
        if (ping->bathlat != NULL) {
          free(ping->bathlat);
          ping->bathlat = NULL;
        }
        if (ping->bathx != NULL) {
          free(ping->bathx);
          ping->bathx = NULL;
        }
        if (ping->bathy != NULL) {
          free(ping->bathy);
          ping->bathy = NULL;
        }
        if (ping->angles != NULL) {
          free(ping->angles);
          ping->angles = NULL;
        }
        if (ping->angles_forward != NULL) {
          free(ping->angles_forward);
          ping->angles_forward = NULL;
        }
        if (ping->angles_null != NULL) {
          free(ping->angles_null);
          ping->angles_null = NULL;
        }
        if (ping->ttimes != NULL) {
          free(ping->ttimes);
          ping->ttimes = NULL;
        }
        if (ping->bheave != NULL) {
          free(ping->bheave);
          ping->bheave = NULL;
        }
        if (ping->alongtrack_offset != NULL) {
          free(ping->alongtrack_offset);
          ping->alongtrack_offset = NULL;
        }
      }
      free(file->pings);
      file->pings = NULL;

      file->n_async_heading = 0;
      file->n_async_heading_alloc = 0;
      if (file->async_heading_time_d != NULL) {
        free(file->async_heading_time_d);
        file->async_heading_time_d = NULL;
      }
      if (file->async_heading_heading != NULL) {
        free(file->async_heading_heading);
        file->async_heading_heading = NULL;
      }
      file->n_async_sensordepth = 0;
      file->n_async_sensordepth_alloc = 0;
      if (file->async_sensordepth_time_d != NULL) {
        free(file->async_sensordepth_time_d);
        file->async_sensordepth_time_d = NULL;
      }
      if (file->async_sensordepth_sensordepth != NULL) {
        free(file->async_sensordepth_sensordepth);
        file->async_sensordepth_sensordepth = NULL;
      }
      file->n_async_attitude = 0;
      file->n_async_attitude_alloc = 0;
      if (file->async_attitude_time_d != NULL) {
        free(file->async_attitude_time_d);
        file->async_attitude_time_d = NULL;
      }
      if (file->async_attitude_roll != NULL) {
        free(file->async_attitude_roll);
        file->async_attitude_roll = NULL;
      }
      if (file->async_attitude_pitch != NULL) {
        free(file->async_attitude_pitch);
        file->async_attitude_pitch = NULL;
      }
      file->n_sync_attitude = 0;
      file->n_sync_attitude_alloc = 0;
      if (file->sync_attitude_time_d != NULL) {
        free(file->sync_attitude_time_d);
        file->sync_attitude_time_d = NULL;
      }
      if (file->sync_attitude_roll != NULL) {
        free(file->sync_attitude_roll);
        file->sync_attitude_roll = NULL;
      }
      if (file->sync_attitude_pitch != NULL) {
        free(file->sync_attitude_pitch);
        file->sync_attitude_pitch = NULL;
      }
    }

    /* reset load status */
    file->load_status = false;
    num_files__loaded--;

    /* unlock the file */
    if (assertUnlock && useLockFiles_) {
      // const int lock_status =
      mb_pr_unlockswathfile(verbose_, file->path, MBP_LOCK_EDITBATHY, program_name, &lock_error);
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::delete_file(int ifile) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ifile:       %d\n", ifile);
  }

  /* unload the file if needed */
  if (ifile >= 0 && ifile < num_files_ && files_[ifile].load_status) {
    bool assertUnlock = true;
    Backend::unload_file(ifile, assertUnlock);
  }

  /* delete the file */
  for (int i = ifile; i < num_files_ - 1; i++) {
    files_[i] = files_[i + 1];
  }
  num_files_--;

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
/* approximate error function altered from numerical recipies */
double Backend::erf(double x) {
  const double z = fabs(x);
  const double t = 1.0 / (1.0 + 0.5 * z);
  double erfc_d =
      t *
      exp(-z * z - 1.26551223 +
          t * (1.00002368 +
               t * (0.37409196 +
                    t * (0.09678418 +
                         t * (-0.18628806 +
                              t * (0.27886807 + t * (-1.13520398 + t * (1.48851587 + t * (-0.82215223 + t * 0.17087277)))))))));
  erfc_d = x >= 0.0 ? erfc_d : 2.0 - erfc_d;
  const double erf_d = 1.0 - erfc_d;
  return erf_d;
}
/*--------------------------------------------------------------------*/
/*
 * function Backend::bin_weight calculates the integrated weight over a bin
 * given the footprint of a sounding
 */
int Backend::bin_weight(double foot_a, double foot_b, double scale, double pcx, double pcy, double dx, double dy, double *px,
                         double *py, double *weight, int *use) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       foot_a:     %f\n", foot_a);
    fprintf(stderr, "dbg2       foot_b:     %f\n", foot_b);
    fprintf(stderr, "dbg2       scale:      %f\n", scale);
    fprintf(stderr, "dbg2       pcx:        %f\n", pcx);
    fprintf(stderr, "dbg2       pcy:        %f\n", pcy);
    fprintf(stderr, "dbg2       dx:         %f\n", dx);
    fprintf(stderr, "dbg2       dy:         %f\n", dy);
    fprintf(stderr, "dbg2       p1 x:       %f\n", px[0]);
    fprintf(stderr, "dbg2       p1 y:       %f\n", py[0]);
    fprintf(stderr, "dbg2       p2 x:       %f\n", px[1]);
    fprintf(stderr, "dbg2       p2 y:       %f\n", py[1]);
    fprintf(stderr, "dbg2       p3 x:       %f\n", px[2]);
    fprintf(stderr, "dbg2       p3 y:       %f\n", py[2]);
    fprintf(stderr, "dbg2       p4 x:       %f\n", px[3]);
    fprintf(stderr, "dbg2       p4 y:       %f\n", py[3]);
  }

  /* The weighting function is
      w(x, y) = (1 / (PI * a * b)) * exp(-(x**2/a**2 + y**2/b**2))
      in the footprint coordinate system, where the x axis
      is along the horizontal projection of the beam and the
      y axix is perpendicular to that. The integral of the
      weighting function over an simple rectangle defined
      by corners (x1, y1), (x2, y1), (x1, y2), (x2, y2) is
          x2 y2
      W = I  I { w(x, y) } dx dy
          x1 y1

        = 1 / 4 * ( erfc(x1/a) - erfc(x2/a)) * ( erfc(y1/a) - erfc(y2/a))
      where erfc(u) is the complementary error function.
      Each bin is represented as a simple integral in geographic
      coordinates, but is rotated in the footprint coordinate system.
      I can't figure out how to evaluate this integral over a
      rotated rectangle,  and so I am crudely and incorrectly
      approximating the integrated weight value by evaluating it over
      the same sized rectangle centered at the same location.
      Maybe someday I'll figure out how to do it correctly.
      DWC 11/18/99 */

  /* get integrated weight */
  const double fa = scale * foot_a;
  const double fb = scale * foot_b;
  *weight = 0.25 * (Backend::erf((pcx + dx) / fa) - Backend::erf((pcx - dx) / fa)) *
            (Backend::erf((pcy + dy) / fb) - Backend::erf((pcy - dy) / fb));

  /* use if weight large or any ratio <= 1 */
  if (*weight > 0.05) {
    *use = MBEV_USE_YES;
  }
  /* check ratio of each corner footprint 1/e distance */
  else {
    *use = MBEV_USE_NO;
    // TODO(schwehr): This appears to overwrite *use 4 times?
    for (int i = 0; i < 4; i++) {
      const double ang = RTD * atan2(py[i], px[i]);
      const double xe = foot_a * cos(DTR * ang);
      const double ye = foot_b * sin(DTR * ang);
      const double ratio = sqrt((px[i] * px[i] + py[i] * py[i]) / (xe * xe + ye * ye));
      if (ratio <= 1.0)
        *use = MBEV_USE_YES;
      else if (ratio <= 2.0)
        *use = MBEV_USE_CONDITIONAL;
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2       weight:     %f\n", *weight);
    fprintf(stderr, "dbg2       use:        %d\n", *use);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::get_grid_bounds() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  double depth_max;
  double altitude_min;
  double altitude_max;
  double reference_lon;
  double reference_lat;

  /* find lon lat bounds of loaded files */
  if (num_files__loaded > 0) {
    bool first = true;
    // double depth_min;
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        struct mb_info_struct *info;
        if (file->processed_info_loaded)
          info = &(file->processed_info);
        else
          info = &(file->raw_info);
        if (first) {
          grid_bounds_[0] = info->lon_min;
          grid_bounds_[1] = info->lon_max;
          grid_bounds_[2] = info->lat_min;
          grid_bounds_[3] = info->lat_max;
          // depth_min = info->depth_min;
          depth_max = info->depth_max;
          altitude_min = info->altitude_min;
          altitude_max = info->altitude_max;
          first = false;
        }
        else {
          grid_bounds_[0] = MIN(grid_bounds_[0], info->lon_min);
          grid_bounds_[1] = MAX(grid_bounds_[1], info->lon_max);
          grid_bounds_[2] = MIN(grid_bounds_[2], info->lat_min);
          grid_bounds_[3] = MAX(grid_bounds_[3], info->lat_max);
          // depth_min = MIN(depth_min, info->depth_min);
          depth_max = MIN(depth_max, info->depth_max);
          altitude_min = MIN(altitude_min, info->altitude_min);
          altitude_max = MIN(altitude_max, info->altitude_max);
        }
      }
    }
  }
  if (num_files__loaded <= 0 || grid_bounds_[1] <= grid_bounds_[0] || grid_bounds_[3] <= grid_bounds_[2]) {
    status_ = MB_FAILURE;
    error_ = MB_ERROR_BAD_PARAMETER;
  }
  else {
    status_ = MB_SUCCESS;
    error_ = MB_ERROR_NO_ERROR;
  }


  void *pjptr;

  /* get projection */
  if (status_ == MB_SUCCESS) {
    /* get projection */
    reference_lon = 0.5 * (grid_bounds_[0] + grid_bounds_[1]);
    reference_lat = 0.5 * (grid_bounds_[2] + grid_bounds_[3]);
    if (reference_lon < 180.0)
      reference_lon += 360.0;
    if (reference_lon >= 180.0)
      reference_lon -= 360.0;
    const int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
    mb_path projection_id;
    if (reference_lat >= 0.0)
      snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
    else
      snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
    const int proj_status = mb_proj_init(verbose_, projection_id, &(pjptr), &error_);
    if (proj_status != MB_SUCCESS) {
      status_ = MB_FAILURE;
      error_ = MB_ERROR_BAD_PARAMETER;
    }
  }

  /* get grid cell size and dimensions */
  if (status_ == MB_SUCCESS) {
    /* get projected bounds */

    /* first point */
    double xx;
    double yy;
    mb_proj_forward(verbose_, pjptr, grid_bounds_[0], grid_bounds_[2], &xx, &yy, &error_);
    grid_bounds_utm[0] = xx;
    grid_bounds_utm[1] = xx;
    grid_bounds_utm[2] = yy;
    grid_bounds_utm[3] = yy;
    // fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
    // grid_bounds_[0],grid_bounds_[1],grid_bounds_[2],grid_bounds_[3],
    // grid_bounds_utm[0],grid_bounds_utm[1],grid_bounds_utm[2],grid_bounds_utm[3]);

    /* second point */
    mb_proj_forward(verbose_, pjptr, grid_bounds_[1], grid_bounds_[2], &xx, &yy, &error_);
    grid_bounds_utm[0] = MIN(grid_bounds_utm[0], xx);
    grid_bounds_utm[1] = MAX(grid_bounds_utm[1], xx);
    grid_bounds_utm[2] = MIN(grid_bounds_utm[2], yy);
    grid_bounds_utm[3] = MAX(mbev_grid.boundsutm[3], yy);
    // fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
    // grid_bounds_[0],grid_bounds_[1],grid_bounds_[2],grid_bounds_[3],
    // grid_bounds_utm[0],grid_bounds_utm[1],grid_bounds_utm[2],grid_bounds_utm[3]);

    /* third point */
    mb_proj_forward(verbose_, pjptr, grid_bounds_[0], grid_bounds_[3], &xx, &yy, &error_);
    grid_bounds_utm[0] = MIN(grid_bounds_utm[0], xx);
    grid_bounds_utm[1] = MAX(grid_bounds_utm[1], xx);
    grid_bounds_utm[2] = MIN(grid_bounds_utm[2], yy);
    grid_bounds_utm[3] = MAX(grid_bounds_utm[3], yy);
    // fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
    // grid_bounds_[0],grid_bounds_[1],grid_bounds_[2],grid_bounds_[3],
    // grid_bounds_utm[0],grid_bounds_utm[1],grid_bounds_utm[2],grid_bounds_utm[3]);

    /* fourth point */
    mb_proj_forward(verbose_, pjptr, grid_bounds_[1], grid_bounds_[3], &xx, &yy, &error_);
    grid_bounds_utm[0] = MIN(grid_bounds_utm[0], xx);
    grid_bounds_utm[1] = MAX(grid_bounds_utm[1], xx);
    grid_bounds_utm[2] = MIN(grid_bounds_utm[2], yy);
    grid_bounds_utm[3] = MAX(grid_bounds_utm[3], yy);
    // fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
    // grid_bounds_[0],grid_bounds_[1],grid_bounds_[2],grid_bounds_[3],
    // grid_bounds_utm[0],grid_bounds_utm[1],grid_bounds_utm[2],grid_bounds_utm[3]);

    /* get grid spacing */
    // fprintf(stderr,"altitude: %f %f\n", altitude_min, altitude_max);
    if (altitude_max > 0.0)
      grid_cellsize_ = 0.02 * altitude_max;
    else if (depth_max > 0.0)
      grid_cellsize_ = 0.02 * depth_max;
    else
      grid_cellsize_ = (grid_bounds_utm[1] - grid_bounds_utm[0]) / 250;

    /* get grid dimensions */
    grid_n_columns_ = (grid_bounds_utm[1] - grid_bounds_utm[0]) / grid_cellsize_ + 1;
    grid_n_rows_ = (grid_bounds_utm[3] - grid_bounds_utm[2]) / grid_cellsize_ + 1;
    grid_bounds_utm[1] = grid_bounds_utm[0] + (grid_n_columns_ - 1) * grid_cellsize_;
    grid_bounds_utm[3] = grid_bounds_utm[2] + (grid_n_rows_ - 1) * grid_cellsize_;
    fprintf(stderr, "\nGrid bounds (longitude latitude): %.7f %.7f %.7f %.7f\n", grid_bounds_[0], grid_bounds_[1],
            grid_bounds_[2], grid_bounds_[3]);
    fprintf(stderr, "Grid bounds (eastings northings): %.3f %.3f %.3f %.3f\n", grid_bounds_utm[0], grid_bounds_utm[1],
            grid_bounds_utm[2], grid_bounds_utm[3]);
    fprintf(stderr, "Altitude range: %.3f %.3f\n", altitude_min, altitude_max);
    fprintf(stderr, "Cell size:%.3f\nGrid Dimensions: %d %d\n\n", grid_cellsize_, grid_n_columns_, grid_n_rows_);

    /* release projection */
    mb_proj_free(verbose_, &(pjptr), &error_);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}

/*--------------------------------------------------------------------*/
int Backend::setup_grid() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  /* find lon lat bounds of loaded files */
  if (num_files__loaded > 0) {
    /* get grid bounds */
    mbev_grid.bounds[0] = grid_bounds_[0];
    mbev_grid.bounds[1] = grid_bounds_[1];
    mbev_grid.bounds[2] = grid_bounds_[2];
    mbev_grid.bounds[3] = grid_bounds_[3];

    /* get grid spacing */
    mbev_grid.dx = grid_cellsize_;
    mbev_grid.dy = grid_cellsize_;
  }
  if (num_files__loaded <= 0 || mbev_grid.bounds[1] <= mbev_grid.bounds[0] || mbev_grid.bounds[3] <= mbev_grid.bounds[2]) {
    status_ = MB_FAILURE;
    error_ = MB_ERROR_BAD_PARAMETER;
  }
  else {
    status_ = MB_SUCCESS;
    error_ = MB_ERROR_NO_ERROR;
  }

  /* get projection */
  if (status_ == MB_SUCCESS) {
    /* get projection */
    double reference_lon = 0.5 * (mbev_grid.bounds[0] + mbev_grid.bounds[1]);
    const double reference_lat = 0.5 * (mbev_grid.bounds[2] + mbev_grid.bounds[3]);
    if (reference_lon < 180.0)
      reference_lon += 360.0;
    if (reference_lon >= 180.0)
      reference_lon -= 360.0;
    int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
    if (reference_lat >= 0.0)
      snprintf(mbev_grid.projection_id, sizeof(mb_path), "UTM%2.2dN", utm_zone);
    else
      snprintf(mbev_grid.projection_id, sizeof(mb_path), "UTM%2.2dS", utm_zone);
    const int proj_status = mb_proj_init(verbose_, mbev_grid.projection_id, &(mbev_grid.pjptr), &error_);
    if (proj_status != MB_SUCCESS) {
      status_ = MB_FAILURE;
      error_ = MB_ERROR_BAD_PARAMETER;
    }
  }

  /* get grid cell size and dimensions */
  if (status_ == MB_SUCCESS) {
    /* get projected bounds */

    /* first point */
    double xx;
    double yy;
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[2], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = xx;
    mbev_grid.boundsutm[1] = xx;
    mbev_grid.boundsutm[2] = yy;
    mbev_grid.boundsutm[3] = yy;

    /* second point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[2], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* third point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[3], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* fourth point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[3], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* get grid dimensions */
    mbev_grid.n_columns = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / mbev_grid.dx + 1;
    mbev_grid.n_rows = (mbev_grid.boundsutm[3] - mbev_grid.boundsutm[2]) / mbev_grid.dy + 1;
    mbev_grid.boundsutm[1] = mbev_grid.boundsutm[0] + (mbev_grid.n_columns - 1) * mbev_grid.dx;
    mbev_grid.boundsutm[3] = mbev_grid.boundsutm[2] + (mbev_grid.n_rows - 1) * mbev_grid.dy;
    /*fprintf(stderr,"Grid bounds: %f %f %f %f    %f %f %f %f\n",
    mbev_grid.bounds[0],mbev_grid.bounds[1],mbev_grid.bounds[2],mbev_grid.bounds[3],
    mbev_grid.boundsutm[0],mbev_grid.boundsutm[1],mbev_grid.boundsutm[2],mbev_grid.boundsutm[3]);
    fprintf(stderr,"cell size:%f %f dimensions: %d %d\n",
    mbev_grid.dx,mbev_grid.dy,mbev_grid.n_columns,mbev_grid.n_rows);*/
  }

  /* allocate memory for grid */
  if (status_ == MB_SUCCESS) {
    if ((mbev_grid.sum = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.wgt = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.val = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.sgm = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if (error_ == MB_ERROR_NO_ERROR) {
      memset(mbev_grid.sum, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.wgt, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.val, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.sgm, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
    }
    else
      status_ = MB_FAILURE;
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}

/*--------------------------------------------------------------------*/
int Backend::project_soundings() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  /* project all soundings into the grid coordinates */
  if (status_ == MB_SUCCESS) {
    /* loop over loaded files */
    int filecount = 0;
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        filecount++;
        snprintf(message, sizeof(message), "Projecting file %d of %d...", filecount, num_files__loaded);
        (*showMessage)(message);
        for (int iping = 0; iping < file->num_pings; iping++) {
          Ping *ping = &(file->pings[iping]);
          mb_proj_forward(verbose_, mbev_grid.pjptr, ping->navlon, ping->navlat, &ping->navlonx, &ping->navlaty,
                          &error_);
          for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam])) {
              mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                              &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);
            }
          }
        }
      }
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}

/*--------------------------------------------------------------------*/
int Backend::make_grid() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  fprintf(stderr, "\nGenerating Grid:\n----------------\n");
  fprintf(stderr, "Grid bounds (longitude latitude): %.7f %.7f %.7f %.7f\n",
          grid_bounds_[0], grid_bounds_[1],
          grid_bounds_[2], grid_bounds_[3]);
  fprintf(stderr, "Grid bounds (eastings northings): %.3f %.3f %.3f %.3f\n",
          grid_bounds_utm[0], grid_bounds_utm[1],
          grid_bounds_utm[2], grid_bounds_utm[3]);
  fprintf(stderr, "Cell size:%.3f\nGrid Dimensions: %d %d\n",
          grid_cellsize_, grid_n_columns_, grid_n_rows_);
  if (grid_algorithm_ == grid_algorithm__SIMPLEMEAN)
    fprintf(stderr, "Algorithm: Simple Mean\n");
  else if (grid_algorithm_ == grid_algorithm__FOOTPRINT)
    fprintf(stderr, "Algorithm: Footprint\n");
  else //if (grid_algorithm_ == grid_algorithm__SHOALBIAS)
    fprintf(stderr, "Algorithm: Shoal Bias\n");
  fprintf(stderr, "Interpolation: %d\n\n", grid_interpolation_);

  /* zero the grid arrays */
  memset(mbev_grid.sum, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
  memset(mbev_grid.wgt, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
  /* memset(mbev_grid.val, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));*/
  memset(mbev_grid.sgm, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));

  /* loop over loaded files */
  int filecount = 0;
  for (int ifile = 0; ifile < num_files_; ifile++) {
    File *file = &files_[ifile];
    if (file->load_status) {
      filecount++;
      snprintf(message, sizeof(message), "Gridding file %d of %d...", filecount, num_files__loaded);
      (*showMessage)(message);
      for (int iping = 0; iping < file->num_pings; iping++) {
        Ping *ping = &(file->pings[iping]);
        for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
          if (mb_beam_ok(ping->beamflag[ibeam])) {
            Backend::grid_beam(file, ping, ibeam, true, false);
          }
        }
      }
    }
  }
  mbev_grid.nodatavalue = MBEV_NODATA;
  bool first = true;
  for (int i = 0; i < mbev_grid.n_columns; i++)
    for (int j = 0; j < mbev_grid.n_rows; j++) {
      const int k = i * mbev_grid.n_rows + j;
      if (mbev_grid.wgt[k] > 0.0) {
        mbev_grid.val[k] = mbev_grid.sum[k] / mbev_grid.wgt[k];
        mbev_grid.sgm[k] = sqrt(fabs(mbev_grid.sgm[k] / mbev_grid.wgt[k] - mbev_grid.val[k] * mbev_grid.val[k]));
        if (first) {
          mbev_grid.min = mbev_grid.val[k];
          mbev_grid.max = mbev_grid.val[k];
          mbev_grid.smin = mbev_grid.sgm[k];
          mbev_grid.smax = mbev_grid.sgm[k];
          first = false;
        }
        else {
          mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[k]);
          mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[k]);
          mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[k]);
          mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[k]);
        }
      }
      else {
        mbev_grid.val[k] = mbev_grid.nodatavalue;
        mbev_grid.sgm[k] = mbev_grid.nodatavalue;
      }
    }
  if (mbev_grid.status == MBEV_GRID_NONE)
    mbev_grid.status = MBEV_GRID_NOTVIEWED;

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}

/*--------------------------------------------------------------------*/
int Backend::grid_beam(File *file, Ping *ping, int ibeam,
                        bool beam_ok,
                        bool apply_now
                        ) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       file:       %p\n", file);
    fprintf(stderr, "dbg2       ping:       %p\n", ping);
    fprintf(stderr, "dbg2       ibeam:      %d\n", ibeam);
    fprintf(stderr, "dbg2       beam_ok:    %d\n", beam_ok);
    fprintf(stderr, "dbg2       apply_now:  %d\n", apply_now);
  }

  /* find location of beam center */
  const int i = (ping->bathx[ibeam] - mbev_grid.boundsutm[0] + 0.5 * mbev_grid.dx) / mbev_grid.dx;
  const int j = (ping->bathy[ibeam] - mbev_grid.boundsutm[2] + 0.5 * mbev_grid.dy) / mbev_grid.dy;

  /* proceed if beam in grid */
  if (i >= 0 && i < mbev_grid.n_columns && j >= 0 && j < mbev_grid.n_rows) {
    double foot_dx, foot_dy, foot_dxn, foot_dyn;
    double foot_lateral, foot_range, foot_theta;
    double foot_dtheta, foot_dphi;
    double foot_hwidth, foot_hlength;
    int foot_wix, foot_wiy, foot_lix, foot_liy, foot_dix, foot_diy;
    int ix1, ix2, iy1, iy2;

    /* shoal bias gridding mode */
    if (grid_algorithm_ == grid_algorithm__SHOALBIAS) {
      /* get location in grid arrays */
      const int kk = i * mbev_grid.n_rows + j;

      if (isnan(ping->bathcorr[ibeam])) {
        fprintf(stderr, "\nFunction Backend::grid_beam(): Encountered NaN value in swath data from file: %s\n",
                file->path);
        fprintf(stderr, "     Ping time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ping->time_i[0], ping->time_i[1],
                ping->time_i[2], ping->time_i[3], ping->time_i[4], ping->time_i[5], ping->time_i[6]);
        fprintf(stderr, "     Beam bathymetry: beam:%d flag:%d bath:<%f %f> acrosstrack:%f alongtrack:%f\n", ibeam,
                ping->beamflag[ibeam], ping->bath[ibeam], ping->bathcorr[ibeam], ping->bathacrosstrack[ibeam],
                ping->bathalongtrack[ibeam]);
      }

      /* add to weights and sums */
      if (beam_ok && (-ping->bathcorr[ibeam]) >  mbev_grid.sum[kk]) {
        mbev_grid.wgt[kk] = 1.0;
        mbev_grid.sum[kk] = (-ping->bathcorr[ibeam]);
        mbev_grid.sgm[kk] = ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
      }

      /* recalculate grid cell if desired */
      if (apply_now) {
        /* recalculate grid cell */
        if (mbev_grid.wgt[kk] > 0.0) {
          mbev_grid.val[kk] = mbev_grid.sum[kk] / mbev_grid.wgt[kk];
          mbev_grid.sgm[kk] = sqrt(fabs(mbev_grid.sgm[kk] / mbev_grid.wgt[kk] - mbev_grid.val[kk] * mbev_grid.val[kk]));
          mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[kk]);
          mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[kk]);
          mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[kk]);
          mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[kk]);
        }
        else {
          mbev_grid.val[kk] = mbev_grid.nodatavalue;
          mbev_grid.sgm[kk] = mbev_grid.nodatavalue;
        }

        /* update grid in mbview display */
        mbview_updateprimarygridcell(verbose_, 0, i, j, mbev_grid.val[kk], &error_);
      }
    }

    /* simple gridding mode */
    else if (file->topo_type != MB_TOPOGRAPHY_TYPE_MULTIBEAM || grid_algorithm_ == grid_algorithm__SIMPLEMEAN) {
      /* get location in grid arrays */
      const int kk = i * mbev_grid.n_rows + j;

      if (isnan(ping->bathcorr[ibeam])) {
        fprintf(stderr, "\nFunction Backend::grid_beam(): Encountered NaN value in swath data from file: %s\n",
                file->path);
        fprintf(stderr, "     Ping time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", ping->time_i[0], ping->time_i[1],
                ping->time_i[2], ping->time_i[3], ping->time_i[4], ping->time_i[5], ping->time_i[6]);
        fprintf(stderr, "     Beam bathymetry: beam:%d flag:%d bath:<%f %f> acrosstrack:%f alongtrack:%f\n", ibeam,
                ping->beamflag[ibeam], ping->bath[ibeam], ping->bathcorr[ibeam], ping->bathacrosstrack[ibeam],
                ping->bathalongtrack[ibeam]);
      }

      /* add to weights and sums */
      if (beam_ok) {
        mbev_grid.wgt[kk] += 1.0;
        mbev_grid.sum[kk] += (-ping->bathcorr[ibeam]);
        mbev_grid.sgm[kk] += ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
      }
      else {
        mbev_grid.wgt[kk] -= 1.0;
        mbev_grid.sum[kk] -= (-ping->bathcorr[ibeam]);
        mbev_grid.sgm[kk] -= ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
        if (mbev_grid.wgt[kk] < MBEV_GRID_WEIGHT_TINY)
          mbev_grid.wgt[kk] = 0.0;
      }

      /* recalculate grid cell if desired */
      if (apply_now) {
        /* recalculate grid cell */
        if (mbev_grid.wgt[kk] > 0.0) {
          mbev_grid.val[kk] = mbev_grid.sum[kk] / mbev_grid.wgt[kk];
          mbev_grid.sgm[kk] = sqrt(fabs(mbev_grid.sgm[kk] / mbev_grid.wgt[kk] - mbev_grid.val[kk] * mbev_grid.val[kk]));
          mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[kk]);
          mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[kk]);
          mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[kk]);
          mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[kk]);
        }
        else {
          mbev_grid.val[kk] = mbev_grid.nodatavalue;
          mbev_grid.sgm[kk] = mbev_grid.nodatavalue;
        }

        /* update grid in mbview display */
        mbview_updateprimarygridcell(verbose_, 0, i, j, mbev_grid.val[kk], &error_);
      }
    }

    /* else footprint gridding algorithm */
    else {
      /* calculate footprint */
      foot_dx = (ping->bathx[ibeam] - ping->navlonx);
      foot_dy = (ping->bathy[ibeam] - ping->navlaty);
      foot_lateral = sqrt(foot_dx * foot_dx + foot_dy * foot_dy);
      if (foot_lateral > 0.0) {
        foot_dxn = foot_dx / foot_lateral;
        foot_dyn = foot_dy / foot_lateral;
      }
      else {
        foot_dxn = 1.0;
        foot_dyn = 0.0;
      }
      foot_range = sqrt(foot_lateral * foot_lateral + ping->altitude * ping->altitude);
      foot_theta = RTD * atan2(foot_lateral, (ping->bathcorr[ibeam] - ping->sensordepth));
      foot_dtheta = 0.5 * file->beamwidth_xtrack;
      foot_dphi = 0.5 * file->beamwidth_ltrack;
      if (foot_dtheta <= 0.0)
        foot_dtheta = 1.0;
      if (foot_dphi <= 0.0)
        foot_dphi = 1.0;
      foot_hwidth = (ping->bathcorr[ibeam] - ping->sensordepth) * tan(DTR * (foot_theta + foot_dtheta)) - foot_lateral;
      foot_hlength = foot_range * tan(DTR * foot_dphi);

      /* get range of bins around footprint to examine */
      foot_wix = fabs(foot_hwidth * cos(DTR * foot_theta) / mbev_grid.dx);
      foot_wiy = fabs(foot_hwidth * sin(DTR * foot_theta) / mbev_grid.dx);
      foot_lix = fabs(foot_hlength * sin(DTR * foot_theta) / mbev_grid.dy);
      foot_liy = fabs(foot_hlength * cos(DTR * foot_theta) / mbev_grid.dy);
      foot_dix = 2 * MAX(foot_wix, foot_lix);
      foot_diy = 2 * MAX(foot_wiy, foot_liy);
      ix1 = MAX(i - foot_dix, 0);
      ix2 = MIN(i + foot_dix, mbev_grid.n_columns - 1);
      iy1 = MAX(j - foot_diy, 0);
      iy2 = MIN(j + foot_diy, mbev_grid.n_rows - 1);

      /* loop over neighborhood of bins */
      for (int ii = ix1; ii <= ix2; ii++)
        for (int jj = iy1; jj <= iy2; jj++) {
          /* find distance of bin center from sounding center */
          const double xx = (mbev_grid.boundsutm[0] + ii * mbev_grid.dx + 0.5 * mbev_grid.dx - ping->bathx[ibeam]);
          const double yy = (mbev_grid.boundsutm[2] + jj * mbev_grid.dy + 0.5 * mbev_grid.dy - ping->bathy[ibeam]);

          /* get center and corners of bin in meters from sounding center */
          const double xx0 = xx;
          const double yy0 = yy;
          const double bdx = 0.5 * mbev_grid.dx;
          const double bdy = 0.5 * mbev_grid.dy;
          const double xx1 = xx0 - bdx;
          const double xx2 = xx0 + bdx;
          const double yy1 = yy0 - bdy;
          const double yy2 = yy0 + bdy;

          /* rotate center and corners of bin to footprint coordinates */
          double prx[5];
          double pry[5];
          prx[0] = xx0 * foot_dxn + yy0 * foot_dyn;
          pry[0] = -xx0 * foot_dyn + yy0 * foot_dxn;
          prx[1] = xx1 * foot_dxn + yy1 * foot_dyn;
          pry[1] = -xx1 * foot_dyn + yy1 * foot_dxn;
          prx[2] = xx2 * foot_dxn + yy1 * foot_dyn;
          pry[2] = -xx2 * foot_dyn + yy1 * foot_dxn;
          prx[3] = xx1 * foot_dxn + yy2 * foot_dyn;
          pry[3] = -xx1 * foot_dyn + yy2 * foot_dxn;
          prx[4] = xx2 * foot_dxn + yy2 * foot_dyn;
          pry[4] = -xx2 * foot_dyn + yy2 * foot_dxn;

          /* get weight integrated over bin */
          double weight;
          int use_weight;
          Backend::bin_weight(foot_hwidth, foot_hlength, 1.0, prx[0], pry[0], bdx, bdy, &prx[1], &pry[1], &weight,
                               &use_weight);

          /* if beam affects cell apply using weight */
          if (use_weight == MBEV_USE_YES) {
            /* get location in grid arrays */
            const int kk = ii * mbev_grid.n_rows + jj;

            /* add to weights and sums */
            if (beam_ok) {
              mbev_grid.wgt[kk] += weight;
              mbev_grid.sum[kk] += weight * (-ping->bathcorr[ibeam]);
              mbev_grid.sgm[kk] += weight * ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
            }
            else {
              mbev_grid.wgt[kk] -= weight;
              mbev_grid.sum[kk] -= weight * (-ping->bathcorr[ibeam]);
              mbev_grid.sgm[kk] -= weight * ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
              if (mbev_grid.wgt[kk] < MBEV_GRID_WEIGHT_TINY)
                mbev_grid.wgt[kk] = 0.0;
            }

            /* recalculate grid cell if desired */
            if (apply_now) {
              /* recalculate grid cell */
              if (mbev_grid.wgt[kk] > 0.0) {
                mbev_grid.val[kk] = mbev_grid.sum[kk] / mbev_grid.wgt[kk];
                mbev_grid.sgm[kk] =
                    sqrt(fabs(mbev_grid.sgm[kk] / mbev_grid.wgt[kk] - mbev_grid.val[kk] * mbev_grid.val[kk]));
                mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[kk]);
                mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[kk]);
                mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[kk]);
                mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[kk]);
              }
              else {
                mbev_grid.val[kk] = mbev_grid.nodatavalue;
                mbev_grid.sgm[kk] = mbev_grid.nodatavalue;
              }

              /* update grid in mbview display */
              mbview_updateprimarygridcell(verbose_, 0, ii, jj, mbev_grid.val[kk], &error_);
            }
          }
        }
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}

/*--------------------------------------------------------------------*/
int Backend::make_grid_simple() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  double depth_min, depth_max;
  double altitude_min, altitude_max;
  bool first;

  /* find lon lat bounds of loaded files */
  if (num_files__loaded > 0) {
    first = true;
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        struct mb_info_struct *info;
        if (file->processed_info_loaded)
          info = &(file->processed_info);
        else
          info = &(file->raw_info);
        if (first) {
          mbev_grid.bounds[0] = info->lon_min;
          mbev_grid.bounds[1] = info->lon_max;
          mbev_grid.bounds[2] = info->lat_min;
          mbev_grid.bounds[3] = info->lat_max;
          depth_min = info->depth_min;
          depth_max = info->depth_max;
          altitude_min = info->altitude_min;
          altitude_max = info->altitude_max;
          first = false;
          if (verbose_ > 0)
            fprintf(stderr, "Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
                    file->processed_info_loaded, file->name, mbev_grid.bounds[0], mbev_grid.bounds[1],
                    mbev_grid.bounds[2], mbev_grid.bounds[3], info->lon_min, info->lon_max, info->lat_min,
                    info->lat_max);
        }
        else {
          mbev_grid.bounds[0] = MIN(mbev_grid.bounds[0], info->lon_min);
          mbev_grid.bounds[1] = MAX(mbev_grid.bounds[1], info->lon_max);
          mbev_grid.bounds[2] = MIN(mbev_grid.bounds[2], info->lat_min);
          mbev_grid.bounds[3] = MAX(mbev_grid.bounds[3], info->lat_max);
          depth_min = MIN(depth_min, info->depth_min);
          depth_max = MIN(depth_max, info->depth_max);
          altitude_min = MIN(altitude_min, info->altitude_min);
          altitude_max = MIN(altitude_max, info->altitude_max);
          if (verbose_ > 0)
            fprintf(stderr, "Processed:%d Name:%s Bounds: %f %f %f %F   File Bounds: %f %f %f %f\n",
                    file->processed_info_loaded, file->name, mbev_grid.bounds[0], mbev_grid.bounds[1],
                    mbev_grid.bounds[2], mbev_grid.bounds[3], info->lon_min, info->lon_max, info->lat_min,
                    info->lat_max);
        }
      }
    }
  }

  if (num_files__loaded <= 0 || mbev_grid.bounds[1] <= mbev_grid.bounds[0] || mbev_grid.bounds[3] <= mbev_grid.bounds[2]) {
    status_ = MB_FAILURE;
    error_ = MB_ERROR_BAD_PARAMETER;
  }
  else {
    status_ = MB_SUCCESS;
    error_ = MB_ERROR_NO_ERROR;
  }

  /* get projection */
  if (status_ == MB_SUCCESS) {
    /* get projection */
    double reference_lon = 0.5 * (mbev_grid.bounds[0] + mbev_grid.bounds[1]);
    const double reference_lat = 0.5 * (mbev_grid.bounds[2] + mbev_grid.bounds[3]);
    if (reference_lon < 180.0)
      reference_lon += 360.0;
    if (reference_lon >= 180.0)
      reference_lon -= 360.0;
    int utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
    if (reference_lat >= 0.0)
      snprintf(mbev_grid.projection_id, sizeof(mb_path), "UTM%2.2dN", utm_zone);
    else
      snprintf(mbev_grid.projection_id, sizeof(mb_path), "UTM%2.2dS", utm_zone);
    const int proj_status = mb_proj_init(verbose_, mbev_grid.projection_id, &(mbev_grid.pjptr), &error_);
    if (proj_status != MB_SUCCESS) {
      status_ = MB_FAILURE;
      error_ = MB_ERROR_BAD_PARAMETER;
    }
  }

  double xx, yy;
  int iping, ibeam;
  int filecount;

  /* get grid cell size and dimensions */
  if (status_ == MB_SUCCESS) {
    /* get projected bounds */

    /* first point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[2], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = xx;
    mbev_grid.boundsutm[1] = xx;
    mbev_grid.boundsutm[2] = yy;
    mbev_grid.boundsutm[3] = yy;

    /* second point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[2], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* third point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[0], mbev_grid.bounds[3], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* fourth point */
    mb_proj_forward(verbose_, mbev_grid.pjptr, mbev_grid.bounds[1], mbev_grid.bounds[3], &xx, &yy, &error_);
    mbev_grid.boundsutm[0] = MIN(mbev_grid.boundsutm[0], xx);
    mbev_grid.boundsutm[1] = MAX(mbev_grid.boundsutm[1], xx);
    mbev_grid.boundsutm[2] = MIN(mbev_grid.boundsutm[2], yy);
    mbev_grid.boundsutm[3] = MAX(mbev_grid.boundsutm[3], yy);

    /* get grid spacing */
    mbev_grid.dx = 0.14 * altitude_max;
    mbev_grid.dy = 0.14 * altitude_max;
    if (altitude_max > 0.0) {
      mbev_grid.dx = 0.02 * altitude_max;
      mbev_grid.dy = 0.02 * altitude_max;
    }
    else if (depth_max > 0.0) {
      mbev_grid.dx = 0.02 * depth_max;
      mbev_grid.dy = 0.02 * depth_max;
    }
    else {
      mbev_grid.dx = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / 250;
      mbev_grid.dy = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / 250;
    }

    /* get grid dimensions */
    mbev_grid.n_columns = (mbev_grid.boundsutm[1] - mbev_grid.boundsutm[0]) / mbev_grid.dx + 1;
    mbev_grid.n_rows = (mbev_grid.boundsutm[3] - mbev_grid.boundsutm[2]) / mbev_grid.dy + 1;
    mbev_grid.boundsutm[1] = mbev_grid.boundsutm[0] + (mbev_grid.n_columns - 1) * mbev_grid.dx;
    mbev_grid.boundsutm[3] = mbev_grid.boundsutm[2] + (mbev_grid.n_rows - 1) * mbev_grid.dy;
    if (verbose_ > 0)
      fprintf(stderr, "Grid bounds: %f %f %f %f    %f %f %f %f\n", mbev_grid.bounds[0], mbev_grid.bounds[1],
              mbev_grid.bounds[2], mbev_grid.bounds[3], mbev_grid.boundsutm[0], mbev_grid.boundsutm[1],
              mbev_grid.boundsutm[2], mbev_grid.boundsutm[3]);
    if (verbose_ > 0)
      fprintf(stderr, "cell size:%f %f dimensions: %d %d\n", mbev_grid.dx, mbev_grid.dy, mbev_grid.n_columns, mbev_grid.n_rows);
  }

  /* allocate memory for grid */
  if (status_ == MB_SUCCESS) {
    if ((mbev_grid.sum = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.wgt = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.val = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if ((mbev_grid.sgm = (float *)malloc(mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float))) == NULL)
      error_ = MB_ERROR_MEMORY_FAIL;
    if (error_ == MB_ERROR_NO_ERROR) {
      memset(mbev_grid.sum, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.wgt, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.val, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
      memset(mbev_grid.sgm, 0, mbev_grid.n_columns * mbev_grid.n_rows * sizeof(float));
    }
    else
      status_ = MB_FAILURE;
  }

  /* make grid */
  if (status_ == MB_SUCCESS) {
    /* loop over loaded files */
    filecount = 0;
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        filecount++;
        snprintf(message, sizeof(message), "Gridding file %d of %d...", filecount, num_files__loaded);
        (*showMessage)(message);
        for (iping = 0; iping < file->num_pings; iping++) {
          Ping *ping = &(file->pings[iping]);
          for (ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam])) {
              mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                              &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);
            }
            if (mb_beam_ok(ping->beamflag[ibeam])) {
              const int i = (ping->bathx[ibeam] - mbev_grid.boundsutm[0] + 0.5 * mbev_grid.dx) / mbev_grid.dx;
              const int j = (ping->bathy[ibeam] - mbev_grid.boundsutm[2] + 0.5 * mbev_grid.dy) / mbev_grid.dy;
              const int k = i * mbev_grid.n_rows + j;
              mbev_grid.sum[k] += (-ping->bathcorr[ibeam]);
              mbev_grid.wgt[k] += 1.0;
              mbev_grid.sgm[k] += ping->bathcorr[ibeam] * ping->bathcorr[ibeam];
            }
          }
        }
      }
    }
    mbev_grid.nodatavalue = MBEV_NODATA;
    first = true;
    for (int i = 0; i < mbev_grid.n_columns; i++)
      for (int j = 0; j < mbev_grid.n_rows; j++) {
        const int k = i * mbev_grid.n_rows + j;
        if (mbev_grid.wgt[k] > 0.0) {
          mbev_grid.val[k] = mbev_grid.sum[k] / mbev_grid.wgt[k];
          mbev_grid.sgm[k] = sqrt(fabs(mbev_grid.sgm[k] / mbev_grid.wgt[k] - mbev_grid.val[k] * mbev_grid.val[k]));
          if (first) {
            mbev_grid.min = mbev_grid.val[k];
            mbev_grid.max = mbev_grid.val[k];
            mbev_grid.smin = mbev_grid.sgm[k];
            mbev_grid.smax = mbev_grid.sgm[k];
            first = false;
          }
          else {
            mbev_grid.min = MIN(mbev_grid.min, mbev_grid.val[k]);
            mbev_grid.max = MAX(mbev_grid.max, mbev_grid.val[k]);
            mbev_grid.smin = MIN(mbev_grid.smin, mbev_grid.sgm[k]);
            mbev_grid.smax = MAX(mbev_grid.smax, mbev_grid.sgm[k]);
          }
        }
        else {
          mbev_grid.val[k] = mbev_grid.nodatavalue;
          mbev_grid.sgm[k] = mbev_grid.nodatavalue;
        }
      }
    mbev_grid.status = MBEV_GRID_NOTVIEWED;
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::destroy_grid() {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  if (verbose_ > 0)
    fprintf(stderr, "Backend::destroy_grid status:%d\n", status_);

  File *file;
  Ping *ping;
  int action;
  int ifile, iping, ibeam;

  /* loop over all files and output edits as necessary */
  for (ifile = 0; ifile < num_files_; ifile++) {
    file = &files_[ifile];
    if (verbose_ > 0)
      fprintf(stderr, "ifile:%d load_status:%d esf_open:%d esf_changed:%d\n",
              ifile, file->load_status, file->esf_open, file->esf_changed);
    if (file->load_status) {
      for (iping = 0; iping < file->num_pings; iping++) {
        ping = &(file->pings[iping]);
        for (ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
          if (ping->beamflag[ibeam] != ping->beamflagorg[ibeam]) {
            if (!file->esf_open) {
              // if too many esf files are open, close as many as needed
              if (num_esf_open_ >= num_esf_open__MAX) {
                /* close the first open esf file found */
                for (int itfile = 0; itfile < num_files_ && num_esf_open_ >= num_esf_open__MAX; itfile++) {
                  File *tfile = &files_[itfile];
                  if (tfile->load_status && tfile->esf_open) {
                    mb_esf_close(verbose_, &tfile->esf, &error_);
                    tfile->esf_open = false;
                    num_esf_open_--;
                  }
                }
              }

              status_ = mb_esf_load(verbose_, program_name, file->path,
                                        false, MBP_ESF_APPEND, file->esffile,
                                        &(file->esf), &error_);
              if (status_ == MB_SUCCESS) {
                file->esf_open = true;
                num_esf_open_++;
              }
              else {
                file->esf_open = false;
                status_ = MB_SUCCESS;
                error_ = MB_ERROR_NO_ERROR;
              }
            }

            if (mb_beam_ok(ping->beamflag[ibeam]))
              action = MBP_EDIT_UNFLAG;
            else if (mb_beam_check_flag_filter2(ping->beamflag[ibeam]))
              action = MBP_EDIT_FILTER;
            else if (mb_beam_check_flag_filter(ping->beamflag[ibeam]))
              action = MBP_EDIT_FILTER;
            else if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam]))
              action = MBP_EDIT_FLAG;
            else
              action = MBP_EDIT_ZERO;
            
            /* save the edits to the esf stream */
            if (file->esf_open) {
              if (verbose_ > 0)
                fprintf(stderr, "mb_esf_save: ifile:%d time_d:%.6f iping:%d multiplicity:%d ibeam:%d %d action:%d\n",
                        ifile, ping->time_d, iping, ping->multiplicity, ibeam,
                        ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action);
              mb_esf_save(verbose_, &(file->esf), ping->time_d,
                          ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action, &error_);
            }
            else {
              fprintf(stderr, "Error: Unable to save edit to edit save file: ifile:%d time_d:%.6f iping:%d multiplicity:%d ibeam:%d %d action:%d\n",
                        ifile, ping->time_d, iping, ping->multiplicity, ibeam,
                        ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action);
            }
          }
        }
      }

      /* update the process structure */
      file->process.mbp_edit_mode = MBP_EDIT_ON;
      strcpy(file->process.mbp_editfile, file->esf.esffile);

      /* close the esf file */
      if (file->esf_open) {
        mb_esf_close(verbose_, &(file->esf), &error_);
        file->esf_open = false;
        num_esf_open_--;

        /* update mbprocess parameter file */
        mb_pr_writepar(verbose_, file->path, &(file->process), &error_);
      }
    }
  }

  /* deallocate memory and reset status */
  if (mbev_grid.status != MBEV_GRID_NONE) {
    /* deallocate arrays */
    if (mbev_grid.sum != NULL)
      free(mbev_grid.sum);
    if (mbev_grid.wgt != NULL)
      free(mbev_grid.wgt);
    if (mbev_grid.val != NULL)
      free(mbev_grid.val);
    if (mbev_grid.sgm != NULL)
      free(mbev_grid.sgm);
    mbev_grid.sum = NULL;
    mbev_grid.wgt = NULL;
    mbev_grid.val = NULL;
    mbev_grid.sgm = NULL;

    /* release projection */
    mb_proj_free(verbose_, &(mbev_grid.pjptr), &error_);

    /* reset parameters */
    memset(mbev_grid.projection_id, 0, MB_PATH_MAXLINE);
    for (int i = 0; i < 4; i++) {
      mbev_grid.bounds[i] = 0.0;
      mbev_grid.boundsutm[i] = 0.0;
    }
    mbev_grid.dx = 0.0;
    mbev_grid.dy = 0.0;
    mbev_grid.n_columns = 0;
    mbev_grid.n_rows = 0;

    /* reset status */
    mbev_grid.status = MBEV_GRID_NONE;
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_: %d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::selectregion(size_t instance) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:     %zu\n", instance);
  }

  struct mbview_struct *mbviewdata = NULL;

  /* check data source for selected area */
  status_ = mbview_getdataptr(verbose_, instance, &mbviewdata, &error_);

  /* check if area is currently defined */
  if (status_ == MB_SUCCESS && mbviewdata->region_type == MBV_REGION_QUAD) {

    /* get area */
    struct mbview_region_struct *region = (struct mbview_region_struct *)&mbviewdata->region;

    /* get region bounds */
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectregion: rollbias:%f pitchbias:%f headingbias:%f timelag:%f snell:%f\n", rollBias_,
              pitchBias_, headingBias_, timeLag_, snell_);
    if (verbose_ > 0)
      fprintf(stderr, "REGION: %f %f   %f %f   %f %f   %f %f\n", region->cornerpoints[0].xgrid,
              region->cornerpoints[0].ygrid, region->cornerpoints[1].xgrid, region->cornerpoints[2].ygrid,
              region->cornerpoints[2].xgrid, region->cornerpoints[2].ygrid, region->cornerpoints[3].xgrid,
              region->cornerpoints[3].ygrid);
    double xmin = region->cornerpoints[0].xgrid;
    double xmax = region->cornerpoints[0].xgrid;
    double ymin = region->cornerpoints[0].ygrid;
    double ymax = region->cornerpoints[0].ygrid;
    double zmin = region->cornerpoints[0].zdata;
    double zmax = region->cornerpoints[0].zdata;
    for (int i = 1; i < 4; i++) {
      xmin = MIN(xmin, region->cornerpoints[i].xgrid);
      xmax = MAX(xmax, region->cornerpoints[i].xgrid);
      ymin = MIN(ymin, region->cornerpoints[i].ygrid);
      ymax = MAX(ymax, region->cornerpoints[i].ygrid);
      zmin = MIN(zmin, region->cornerpoints[i].zdata);
      zmax = MAX(zmax, region->cornerpoints[i].zdata);
    }

    /* get sounding bounds */
    selected_.xorigin = 0.5 * (xmin + xmax);
    selected_.yorigin = 0.5 * (ymin + ymax);
    selected_.zorigin = 0.5 * (zmin + zmax);
    double dx = xmax - xmin;
    double dy = ymax - ymin;
    selected_.xmin = -0.5 * dx;
    selected_.ymin = -0.5 * dy;
    selected_.xmax = 0.5 * dx;
    selected_.ymax = 0.5 * dy;
    selected_.bearing = 90.0;
    selected_.sinbearing = sin(DTR * selected_.bearing);
    selected_.cosbearing = cos(DTR * selected_.bearing);
    selected_.scale = 2.0 / sqrt((xmax - xmin) * (xmax - xmin) + (ymax - ymin) * (ymax - ymin));
    selected_.num_soundings = 0;
    selected_.num_soundings_unflagged = 0;
    selected_.num_soundings_flagged = 0;

    /* loop over all files */
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        for (int iping = 0; iping < file->num_pings; iping++) {
          Ping *ping = &(file->pings[iping]);
          double heading;
          double sensordepth;
          double rolldelta;
          double pitchdelta;
          Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_,
                                  timeLag_, &heading, &sensordepth, &rolldelta, &pitchdelta);
          double mtodeglon;
          double mtodeglat;
          mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
          for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            if (mb_beam_check_flag_usable2(ping->beamflag[ibeam])
              || (mbviewdata->state21 && mb_beam_check_flag_multipick(ping->beamflag[ibeam]))) {
              if (ping->bathx[ibeam] >= xmin && ping->bathx[ibeam] <= xmax && ping->bathy[ibeam] >= ymin &&
                  ping->bathy[ibeam] <= ymax) {
                /* allocate memory if needed */
                if (selected_.num_soundings >= selected_.num_soundings_alloc) {
                  selected_.num_soundings_alloc += MBEV_ALLOCK_NUM;
                  selected_.soundings =
                      realloc(selected_.soundings,
                              selected_.num_soundings_alloc * sizeof(Soundings));
                }

                /* same beam ids */
                selected_.soundings[selected_.num_soundings].ifile = ifile;
                selected_.soundings[selected_.num_soundings].iping = iping;
                selected_.soundings[selected_.num_soundings].ibeam = ibeam;
                selected_.soundings[selected_.num_soundings].beamflag = ping->beamflag[ibeam];
                selected_.soundings[selected_.num_soundings].beamflagorg = ping->beamflagorg[ibeam];
                selected_.soundings[selected_.num_soundings].beamcolor = ping->beamcolor[ibeam];

                /* get sounding relative to sonar */
                double beam_xtrack = ping->bathacrosstrack[ibeam];
                double beam_ltrack = ping->bathalongtrack[ibeam];
                double beam_z = ping->bath[ibeam] - ping->sensordepth;

                /* if beamforming sound speed correction to be applied */
                if (snell_ != 1.0) {
                  Backend::snell_correction(snell_, (ping->roll + rolldelta),
                                 &beam_xtrack, &beam_ltrack, &beam_z);
                }

                /* apply rotations and recalculate position */
                Backend::beam_position(
                    ping->navlon, ping->navlat, mtodeglon, mtodeglat, beam_z,
                    beam_xtrack, beam_ltrack, sensordepth, rolldelta, pitchdelta,
                    heading, &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
                mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                                &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);

                /* get local position in selected region */
                const double x = ping->bathx[ibeam] - selected_.xorigin;
                const double y = ping->bathy[ibeam] - selected_.yorigin;
                const double xx = x * selected_.sinbearing + y * selected_.cosbearing;
                const double yy = -x * selected_.cosbearing + y * selected_.sinbearing;
                selected_.soundings[selected_.num_soundings].x = xx;
                selected_.soundings[selected_.num_soundings].y = yy;
                /*selected_.soundings[selected_.num_soundings].x
                    = xx * selected_.cosbearing - yy * selected_.sinbearing;
                selected_.soundings[selected_.num_soundings].y
                    = xx * selected_.sinbearing + yy * selected_.cosbearing;*/
                selected_.soundings[selected_.num_soundings].z = -ping->bathcorr[ibeam];
                if (selected_.num_soundings == 0) {
                  zmin = -ping->bathcorr[ibeam];
                  zmax = -ping->bathcorr[ibeam];
                }
                else {
                  zmin = MIN(zmin, -ping->bathcorr[ibeam]);
                  zmax = MAX(zmax, -ping->bathcorr[ibeam]);
                }
                selected_.soundings[selected_.num_soundings].a = ping->amp[ibeam];

                /* get sounding color to be used if displayed colored by topography */
                mbview_colorvalue_instance(instance,
                      selected_.soundings[selected_.num_soundings].z,
                      &(selected_.soundings[selected_.num_soundings].r),
                      &(selected_.soundings[selected_.num_soundings].g),
                      &(selected_.soundings[selected_.num_soundings].b));

                /*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
                ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
                selected_.num_soundings,
                selected_.soundings[selected_.num_soundings].x,
                selected_.soundings[selected_.num_soundings].y,
                selected_.soundings[selected_.num_soundings].z);*/
                /* keep the counts right */
                selected_.num_soundings++;
                if (mb_beam_ok(ping->beamflag[ibeam]))
                  selected_.num_soundings_unflagged++;
                else
                  selected_.num_soundings_flagged++;
              }
            }
          }
        }
      }
    }

    /* get zscaling */
    selected_.zscale = selected_.scale;
    const double dz = zmax - zmin;
    selected_.zorigin = 0.5 * (zmin + zmax);
    selected_.zmin = -0.5 * dz;
    selected_.zmax = 0.5 * dz;
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectregion: num_soundings:%d\n", selected_.num_soundings);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::selectarea(size_t instance) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:     %zu\n", instance);
  }

  /* check data source for selected area */
  struct mbview_struct *mbviewdata;
  status_ = mbview_getdataptr(verbose_, instance, &mbviewdata, &error_);

  /* check if area is currently defined */
  if (status_ == MB_SUCCESS && mbviewdata->area_type == MBV_AREA_QUAD) {
    /* get area */
    struct mbview_area_struct *area = (struct mbview_area_struct *)&mbviewdata->area;
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectarea: rollbias:%f pitchbias:%f headingbias:%f timelag:%f snell:%f\n", rollBias_,
              pitchBias_, headingBias_, timeLag_, snell_);
    if (verbose_ > 0)
      fprintf(stderr, "AREA: %f %f   %f %f   %f %f   %f %f\n", area->cornerpoints[0].xgrid, area->cornerpoints[0].ygrid,
              area->cornerpoints[1].xgrid, area->cornerpoints[2].ygrid, area->cornerpoints[2].xgrid,
              area->cornerpoints[2].ygrid, area->cornerpoints[3].xgrid, area->cornerpoints[3].ygrid);

    /* get sounding bounds */
    selected_.xorigin = 0.5 * (area->endpoints[0].xgrid + area->endpoints[1].xgrid);
    selected_.yorigin = 0.5 * (area->endpoints[0].ygrid + area->endpoints[1].ygrid);
    selected_.zorigin = 0.5 * (area->endpoints[0].zdata + area->endpoints[1].zdata);
    selected_.xmin = -0.5 * area->length;
    selected_.ymin = -0.5 * area->width;
    selected_.xmax = 0.5 * area->length;
    selected_.ymax = 0.5 * area->width;
    selected_.bearing = area->bearing;
    selected_.sinbearing = sin(DTR * selected_.bearing);
    selected_.cosbearing = cos(DTR * selected_.bearing);
    selected_.scale = 2.0 / sqrt(area->length * area->length + area->width * area->width);
    selected_.num_soundings = 0;
    selected_.num_soundings_unflagged = 0;
    selected_.num_soundings_flagged = 0;

    double zmin;
    double zmax;
    // double heading, sensordepth;
    // double rolldelta, pitchdelta;

    /* loop over all files */
    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        for (int iping = 0; iping < file->num_pings; iping++) {
          Ping *ping = &(file->pings[iping]);
          double heading;
          double sensordepth;
          double rolldelta;
          double pitchdelta;
          Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_,
                                  timeLag_, &heading, &sensordepth, &rolldelta, &pitchdelta);
          double mtodeglon;
          double mtodeglat;
          mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
          for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            if (mb_beam_check_flag_usable2(ping->beamflag[ibeam])
              || (mbviewdata->state21 && mb_beam_check_flag_multipick(ping->beamflag[ibeam]))) {
              double x = ping->bathx[ibeam] - selected_.xorigin;
              double y = ping->bathy[ibeam] - selected_.yorigin;
              double yy = -x * selected_.cosbearing + y * selected_.sinbearing;
              double xx = x * selected_.sinbearing + y * selected_.cosbearing;
              if (xx >= selected_.xmin && xx <= selected_.xmax && yy >= selected_.ymin &&
                  yy <= selected_.ymax) {
                /* allocate memory if needed */
                if (selected_.num_soundings >= selected_.num_soundings_alloc) {
                  selected_.num_soundings_alloc += MBEV_ALLOCK_NUM;
                  selected_.soundings =
                      realloc(selected_.soundings,
                              selected_.num_soundings_alloc * sizeof(Sounding));
                }

                /* same beam ids */
                selected_.soundings[selected_.num_soundings].ifile = ifile;
                selected_.soundings[selected_.num_soundings].iping = iping;
                selected_.soundings[selected_.num_soundings].ibeam = ibeam;
                selected_.soundings[selected_.num_soundings].beamflag = ping->beamflag[ibeam];
                selected_.soundings[selected_.num_soundings].beamflagorg = ping->beamflagorg[ibeam];
                selected_.soundings[selected_.num_soundings].beamcolor = ping->beamcolor[ibeam];

                /* get sounding relative to sonar */
                double beam_xtrack = ping->bathacrosstrack[ibeam];
                double beam_ltrack = ping->bathalongtrack[ibeam];
                double beam_z = ping->bath[ibeam] - ping->sensordepth;

                /* if beamforming sound speed correction to be applied */
                if (snell_ != 1.0) {
                  Backend::snell_correction(snell_, (ping->roll + rolldelta),
                                 &beam_xtrack, &beam_ltrack, &beam_z);
                }

                /* apply rotations and recalculate position */
                Backend::beam_position(
                    ping->navlon, ping->navlat, mtodeglon, mtodeglat, beam_z,
                    beam_xtrack, beam_ltrack, sensordepth, rolldelta, pitchdelta,
                    heading, &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
                mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                                &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);
                x = ping->bathx[ibeam] - selected_.xorigin;
                y = ping->bathy[ibeam] - selected_.yorigin;
                yy = -x * selected_.cosbearing + y * selected_.sinbearing;
                xx = x * selected_.sinbearing + y * selected_.cosbearing;

                /* get local position in selected region */
                selected_.soundings[selected_.num_soundings].x = xx;
                selected_.soundings[selected_.num_soundings].y = yy;
                selected_.soundings[selected_.num_soundings].z = -ping->bathcorr[ibeam];
                if (selected_.num_soundings == 0) {
                  zmin = -ping->bathcorr[ibeam];
                  zmax = -ping->bathcorr[ibeam];
                }
                else {
                  zmin = MIN(zmin, -ping->bathcorr[ibeam]);
                  zmax = MAX(zmax, -ping->bathcorr[ibeam]);
                }
                selected_.soundings[selected_.num_soundings].a = ping->amp[ibeam];

                /* get sounding color to be used if displayed colored by topography */
                mbview_colorvalue_instance(instance,
                      selected_.soundings[selected_.num_soundings].z,
                      &(selected_.soundings[selected_.num_soundings].r),
                      &(selected_.soundings[selected_.num_soundings].g),
                      &(selected_.soundings[selected_.num_soundings].b));

                /*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
                ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
                selected_.num_soundings,
                selected_.soundings[selected_.num_soundings].x,
                selected_.soundings[selected_.num_soundings].y,
                selected_.soundings[selected_.num_soundings].z);*/
                selected_.num_soundings++;
                if (mb_beam_ok(ping->beamflag[ibeam]))
                  selected_.num_soundings_unflagged++;
                else
                  selected_.num_soundings_flagged++;
              }
            }
          }
        }
      }
    }

    /* get zscaling */
    selected_.zscale = selected_.scale;
    const double dz = zmax - zmin;
    selected_.zorigin = 0.5 * (zmin + zmax);
    selected_.zmin = -0.5 * dz;
    selected_.zmax = 0.5 * dz;
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectarea: num_soundings:%d\n", selected_.num_soundings);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
int Backend::selectnav(size_t instance) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       instance:     %zu\n", instance);
  }
  if (verbose_ > 0)
    fprintf(stderr, "Backend::selectnav: \n");

  /* check shared data source for selected nav */
  struct mbview_shareddata_struct *mbviewshared;
  status_ = mbview_getsharedptr(verbose_, &mbviewshared, &error_);
  struct mbview_struct *mbviewdata;
  status_ = mbview_getdataptr(verbose_, instance, &mbviewdata, &error_);

  /* check if any nav is currently selected */
  if (status_ == MB_SUCCESS) {
    /* reset sounding count */
    selected_.num_soundings = 0;
    selected_.num_soundings_unflagged = 0;
    selected_.num_soundings_flagged = 0;

    /* get sounding bearing */
    selected_.bearing = 90.0;
    selected_.sinbearing = sin(DTR * selected_.bearing);
    selected_.cosbearing = cos(DTR * selected_.bearing);

    /* loop over all files to get bounds */
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectnav: rollbias:%f pitchbias:%f headingbias:%f timelag:%f snell:%f\n", rollBias_,
              pitchBias_, headingBias_, timeLag_, snell_);
    int inavcount = 0;
    struct mbview_navpointw_struct *navpts;
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double zmin;
    double zmax;
    // double heading, sensordepth;
    // double rolldelta, pitchdelta;

    for (int ifile = 0; ifile < num_files_; ifile++) {
      File *file = &files_[ifile];
      if (file->load_status) {
        navpts = (struct mbview_navpointw_struct *)mbviewshared->navs[inavcount].navpts;
        for (int iping = 0; iping < file->num_pings; iping++) {
          if (navpts[iping].selected) {
            Ping *ping = &(file->pings[iping]);
            double heading;
            double sensordepth;
            double rolldelta;
            double pitchdelta;
            Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_,
                                    timeLag_, &heading, &sensordepth, &rolldelta, &pitchdelta);
            double mtodeglon;
            double mtodeglat;
            mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
            for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
            if (mb_beam_check_flag_usable2(ping->beamflag[ibeam])
              || (mbviewdata->state21 && mb_beam_check_flag_multipick(ping->beamflag[ibeam]))) {
              /* allocate memory if needed */
              if (selected_.num_soundings >= selected_.num_soundings_alloc) {
                selected_.num_soundings_alloc += MBEV_ALLOCK_NUM;
                selected_.soundings =
                    realloc(selected_.soundings,
                            selected_.num_soundings_alloc * sizeof(Sounding));
              }

              /* same beam ids */
              selected_.soundings[selected_.num_soundings].ifile = ifile;
              selected_.soundings[selected_.num_soundings].iping = iping;
              selected_.soundings[selected_.num_soundings].ibeam = ibeam;
              selected_.soundings[selected_.num_soundings].beamflag = ping->beamflag[ibeam];
              selected_.soundings[selected_.num_soundings].beamflagorg = ping->beamflagorg[ibeam];
              selected_.soundings[selected_.num_soundings].beamcolor = ping->beamcolor[ibeam];

              /* get sounding relative to sonar */
              double beam_xtrack = ping->bathacrosstrack[ibeam];
              double beam_ltrack = ping->bathalongtrack[ibeam];
              double beam_z = ping->bath[ibeam] - ping->sensordepth;

              /* if beamforming sound speed correction to be applied */
              if (snell_ != 1.0) {
                Backend::snell_correction(snell_, (ping->roll + rolldelta),
                               &beam_xtrack, &beam_ltrack, &beam_z);
              }

              /* apply rotations and recalculate position */
              Backend::beam_position(
                  ping->navlon, ping->navlat, mtodeglon, mtodeglat, beam_z,
                  beam_xtrack, beam_ltrack, sensordepth, rolldelta, pitchdelta,
                  heading, &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
              mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                              &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);

              /* get local position in selected region */
              selected_.soundings[selected_.num_soundings].x = ping->bathx[ibeam];
              selected_.soundings[selected_.num_soundings].y = ping->bathy[ibeam];
              selected_.soundings[selected_.num_soundings].z = -ping->bathcorr[ibeam];
              if (selected_.num_soundings == 0) {
                xmin = ping->bathx[ibeam];
                xmax = ping->bathx[ibeam];
                ymin = ping->bathy[ibeam];
                ymax = ping->bathy[ibeam];
                zmin = -ping->bathcorr[ibeam];
                zmax = -ping->bathcorr[ibeam];
              }
              else {
                xmin = MIN(xmin, ping->bathx[ibeam]);
                xmax = MAX(xmax, ping->bathx[ibeam]);
                ymin = MIN(ymin, ping->bathy[ibeam]);
                ymax = MAX(ymax, ping->bathy[ibeam]);
                zmin = MIN(zmin, -ping->bathcorr[ibeam]);
                zmax = MAX(zmax, -ping->bathcorr[ibeam]);
              }
              selected_.soundings[selected_.num_soundings].a = ping->amp[ibeam];

              /* get sounding color to be used if displayed colored by topography */
              mbview_colorvalue_instance(instance,
                    selected_.soundings[selected_.num_soundings].z,
                    &(selected_.soundings[selected_.num_soundings].r),
                    &(selected_.soundings[selected_.num_soundings].g),
                    &(selected_.soundings[selected_.num_soundings].b));

              /*fprintf(stderr,"SELECTED SOUNDING: %d %d %d  %f %f  |  %d %f %f %f\n",
              ifile,iping,ibeam,ping->bathx[ibeam],ping->bathy[ibeam],
              selected_.num_soundings,
              selected_.soundings[selected_.num_soundings].x,
              selected_.soundings[selected_.num_soundings].y,
              selected_.soundings[selected_.num_soundings].z);*/
              selected_.num_soundings++;
              if (mb_beam_ok(ping->beamflag[ibeam]))
                selected_.num_soundings_unflagged++;
              else
                selected_.num_soundings_flagged++;
              }
            }
          }
        }

        inavcount++;
      }
    }

    /* get origin and scaling */
    const double dx = xmax - xmin;
    const double dy = ymax - ymin;
    const double dz = zmax - zmin;
    const double xorigin = 0.5 * (xmin + xmax);
    const double yorigin = 0.5 * (ymin + ymax);
    selected_.zorigin = 0.5 * (zmin + zmax);
    selected_.scale = 2.0 / sqrt(dy * dy + dx * dx);
    selected_.zscale = selected_.scale;
    selected_.xmin = -0.5 * dx;
    selected_.xmax = 0.5 * dx;
    selected_.ymin = -0.5 * dy;
    selected_.ymax = 0.5 * dy;
    selected_.zmin = -0.5 * dz;
    selected_.zmax = 0.5 * dz;
    for (int i = 0; i < selected_.num_soundings; i++) {
      selected_.soundings[i].x = selected_.soundings[i].x - xorigin;
      selected_.soundings[i].y = selected_.soundings[i].y - yorigin;
    }
    if (verbose_ > 0)
      fprintf(stderr, "Backend::selectnav: num_soundings:%d\n", selected_.num_soundings);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }

  return (status_);
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_dismiss() {
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_dismiss\n");

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
  }

  /* release the memory of the soundings */
  selected_.displayed = false;
  if (selected_.num_soundings_alloc > 0) {
    if (selected_.soundings != NULL) {
      free(selected_.soundings);
      selected_.soundings = NULL;
    }
    selected_.xorigin = 0.0;
    selected_.yorigin = 0.0;
    selected_.zorigin = 0.0;
    selected_.bearing = 0.0;
    selected_.xmin = 0.0;
    selected_.ymin = 0.0;
    selected_.zmin = 0.0;
    selected_.xmax = 0.0;
    selected_.ymax = 0.0;
    selected_.zmax = 0.0;
    selected_.sinbearing = 0.0;
    selected_.cosbearing = 0.0;
    selected_.scale = 0.0;
    selected_.zscale = 0.0;
    selected_.num_soundings = 0;
    selected_.num_soundings_unflagged = 0;
    selected_.num_soundings_flagged = 0;
    selected_.num_soundings_alloc = 0;
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_edit(int ifile, int iping, int ibeam, char beamflag, int flush) {
  /* fprintf(stderr,"Backend::mb3dsoundings_edit:%d %d %d beamflag:%d flush:%d\n",
  ifile, iping, ibeam, beamflag, flush); */

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ifile:       %d\n", ifile);
    fprintf(stderr, "dbg2       iping:       %d\n", iping);
    fprintf(stderr, "dbg2       ibeam:       %d\n", ibeam);
    fprintf(stderr, "dbg2       beamflag:    %d\n", beamflag);
    fprintf(stderr, "dbg2       flush:       %d\n", flush);
  }

  /* apply current edit event */
  if (flush != MB3DSDG_EDIT_FLUSHPREVIOUS) {
    File *file = &files_[ifile];
    Ping *ping = &(file->pings[iping]);

    /* set esf change flag for file */
    file->esf_changed = true;

    /* check for real flag state change */
    if (mb_beam_ok(ping->beamflag[ibeam]) != mb_beam_ok(beamflag)) {
      /* apply change to grid */
      Backend::grid_beam(file, ping, ibeam, mb_beam_ok(beamflag), true);
    }

    /* output edits if desired */
    if (mode_output_ == MBEV_OUTPUT_MODE_EDIT) {
      /* open esf and ess files if not already open */
      if (!file->esf_open) {
        // if too many esf files are open, close as many as needed
        if (num_esf_open_ >= num_esf_open__MAX) {
          /* close the first open esf file found */
          for (int itfile = 0; itfile < num_files_ && num_esf_open_ >= num_esf_open__MAX; itfile++) {
            File *tfile = &files_[itfile];
            if (tfile->load_status && tfile->esf_open) {
              mb_esf_close(verbose_, &tfile->esf, &error_);
              tfile->esf_open = false;
              num_esf_open_--;
            }
          }
        }

        status_ = mb_esf_load(verbose_, program_name, file->path, false, MBP_ESF_APPEND, file->esffile,
                                  &(file->esf), &error_);
        if (status_ == MB_SUCCESS) {
          file->esf_open = true;
          num_esf_open_++;
        }
        else {
          file->esf_open = false;
          status_ = MB_SUCCESS;
          error_ = MB_ERROR_NO_ERROR;
        }
      }

      /* save the edits to the esf stream */
      if (file->esf_open) {
        int action;
        if (mb_beam_ok(beamflag))
          action = MBP_EDIT_UNFLAG;
        else if (mb_beam_check_flag_filter2(beamflag))
          action = MBP_EDIT_FILTER;
        else if (mb_beam_check_flag_filter(beamflag))
          action = MBP_EDIT_FILTER;
        else if (!mb_beam_check_flag_unusable(beamflag))
          action = MBP_EDIT_FLAG;
        else
          action = MBP_EDIT_ZERO;
        mb_ess_save(verbose_, &(file->esf), ping->time_d, ibeam + ping->multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error_);
      }
    }

    /* save new beamflag */
    ping->beamflag[ibeam] = beamflag;
  }

  /* redisplay grid if flush specified */
  if (flush != MB3DSDG_EDIT_NOFLUSH) {
    mbview_plothigh(0);
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_info(int ifile, int iping, int ibeam, char *infostring) {
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_info:%d %d %d\n", ifile, iping, ibeam);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ifile:       %d\n", ifile);
    fprintf(stderr, "dbg2       iping:       %d\n", iping);
    fprintf(stderr, "dbg2       ibeam:       %d\n", ibeam);
  }

  /* generate info string */
  File *file = &files_[ifile];
  Ping *ping = &(file->pings[iping]);
  snprintf(infostring, sizeof(mb_path), 
          "Beam %d of %d   Ping %d of %d   File:%s\nPing Time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %f\nLon:%.6f Lat:%.6f "
          "Depth:%.3f X:%.3f L:%.3f A:%.3f",
          ibeam, ping->beams_bath, iping, file->num_pings, file->name, ping->time_i[0], ping->time_i[1], ping->time_i[2],
          ping->time_i[3], ping->time_i[4], ping->time_i[5], ping->time_i[6], ping->time_d, ping->bathlon[ibeam],
          ping->bathlat[ibeam], ping->bath[ibeam], ping->bathacrosstrack[ibeam], ping->bathalongtrack[ibeam], ping->amp[ibeam]);
  //fprintf(stderr, "\nbathcorr:%f bath:%f sensordepth:%f", ping->bathcorr[ibeam], ping->bath[ibeam], ping->sensordepth);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2       infostring: %s\n", infostring);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_bias(double rollbias, double pitchbias, double headingbias, double timelag, double snell) {
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_bias:%f %f %f %f %f\n", rollbias, pitchbias, headingbias, timelag, snell);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       rollbias:    %f\n", rollbias);
    fprintf(stderr, "dbg2       pitchbias:   %f\n", pitchbias);
    fprintf(stderr, "dbg2       headingbias: %f\n", headingbias);
    fprintf(stderr, "dbg2       timelag:     %f\n", timelag);
    fprintf(stderr, "dbg2       snell:       %f\n", snell);
  }

  /* copy bias parameters */
  rollBias_ = rollbias;
  pitchBias_ = pitchbias;
  headingBias_ = headingbias;
  timeLag_ = timelag;
  snell_ = snell;
  int ifilelast = -1;
  int ipinglast = -1;

  double zmin = 0.0;
  double zmax = 0.0;

  double heading = 0.0;
  double sensordepth = 0.0;
  double rolldelta = 0.0;
  double pitchdelta = 0.0;
  double mtodeglon = 0.0;
  double mtodeglat = 0.0;

  /* apply bias parameters */
  for (int i = 0; i < selected_.num_soundings; i++) {
    const int ifile = selected_.soundings[i].ifile;
    const int iping = selected_.soundings[i].iping;
    const int ibeam = selected_.soundings[i].ibeam;
    File *file = &files_[ifile];
    Ping *ping = &(file->pings[iping]);

    if (ifile != ifilelast || iping != ipinglast) {
      Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_,
                              timeLag_, &heading, &sensordepth, &rolldelta, &pitchdelta);
      mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
      ifilelast = ifile;
      ipinglast = iping;
    }

    /* get sounding relative to sonar */
    double beam_xtrack = ping->bathacrosstrack[ibeam];
    double beam_ltrack = ping->bathalongtrack[ibeam];
    double beam_z = ping->bath[ibeam] - ping->sensordepth;

    /* if beamforming sound speed correction to be applied */
    if (snell_ != 1.0) {
      Backend::snell_correction(snell_, (ping->roll + rolldelta),
                     &beam_xtrack, &beam_ltrack, &beam_z);
    }

    /* apply rotations and recalculate position */
    Backend::beam_position(ping->navlon, ping->navlat, mtodeglon, mtodeglat, beam_z,
                            beam_xtrack, beam_ltrack, sensordepth, rolldelta, pitchdelta,
                            heading, &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
    mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam], &ping->bathx[ibeam],
                    &ping->bathy[ibeam], &error_);
    const double x = ping->bathx[ibeam] - selected_.xorigin;
    const double y = ping->bathy[ibeam] - selected_.yorigin;
    const double xx = x * selected_.sinbearing + y * selected_.cosbearing;
    const double yy = -x * selected_.cosbearing + y * selected_.sinbearing;

    /* get local position in selected region */
    selected_.soundings[i].x = xx;
    selected_.soundings[i].y = yy;
    selected_.soundings[i].z = -ping->bathcorr[ibeam];
    if (i == 0) {
      zmin = -ping->bathcorr[ibeam];
      zmax = -ping->bathcorr[ibeam];
    }
    else {
      zmin = MIN(zmin, -ping->bathcorr[ibeam]);
      zmax = MAX(zmax, -ping->bathcorr[ibeam]);
    }
  }

  /* get zscaling */
  selected_.zscale = selected_.scale;
  const double dz = zmax - zmin;
  selected_.zorigin = 0.5 * (zmin + zmax);
  selected_.zmin = -0.5 * dz;
  selected_.zmax = 0.5 * dz;
  for (int i = 0; i < selected_.num_soundings; i++)
    selected_.soundings[i].z = selected_.soundings[i].z - selected_.zorigin;

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_biasapply(double rollbias, double pitchbias, double headingbias, double timelag, double snell) {
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_biasapply:%f %f %f %f %f\n", rollbias, pitchbias, headingbias, timelag, snell);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       rollbias:    %f\n", rollbias);
    fprintf(stderr, "dbg2       pitchbias:   %f\n", pitchbias);
    fprintf(stderr, "dbg2       headingbias: %f\n", headingbias);
    fprintf(stderr, "dbg2       timelag:     %f\n", timelag);
    fprintf(stderr, "dbg2       snell:       %f\n", snell);
  }

  /* copy bias parameters */
  rollBias_ = rollbias;
  pitchBias_ = pitchbias;
  headingBias_ = headingbias;
  timeLag_ = timelag;
  snell_ = snell;

  /* turn message on */
  snprintf(message, sizeof(message), "Regridding using new bias parameters %f %f %f %f %f\n", rollBias_, pitchBias_, headingBias_,
          timeLag_, snell_);
  (*showMessage)(message);

  // double heading, sensordepth;
  // double rolldelta, pitchdelta;
  // double mtodeglon, mtodeglat;
  // double beam_xtrack, beam_ltrack, beam_z;

  /* apply bias parameters to swath data */
  for (int ifile = 0; ifile < num_files_; ifile++) {
    File *file = &files_[ifile];
    if (file->load_status) {
      for (int iping = 0; iping < file->num_pings; iping++) {
        Ping *ping = &(file->pings[iping]);
        double heading, sensordepth;
        double rolldelta, pitchdelta;
        Backend::apply_biasesandtimelag(file, ping, rollBias_, pitchBias_, headingBias_,
                                timeLag_, &heading, &sensordepth, &rolldelta, &pitchdelta);
        double mtodeglon, mtodeglat;
        mb_coor_scale(verbose_, ping->navlat, &mtodeglon, &mtodeglat);
        for (int ibeam = 0; ibeam < ping->beams_bath; ibeam++) {
          if (!mb_beam_check_flag_unusable(ping->beamflag[ibeam])) {
            /* get sounding relative to sonar */
            double beam_xtrack = ping->bathacrosstrack[ibeam];
            double beam_ltrack = ping->bathalongtrack[ibeam];
            double beam_z = ping->bath[ibeam] - ping->sensordepth;

            /* if beamforming sound speed correction to be applied */
            if (snell_ != 1.0) {
              Backend::snell_correction(snell_, (ping->roll + rolldelta),
                             &beam_xtrack, &beam_ltrack, &beam_z);
            }

            /* apply rotations and recalculate position */
            Backend::beam_position(ping->navlon, ping->navlat, mtodeglon, mtodeglat,
                        beam_z, beam_xtrack, beam_ltrack, sensordepth, rolldelta, pitchdelta, heading,
                        &(ping->bathcorr[ibeam]), &(ping->bathlon[ibeam]), &(ping->bathlat[ibeam]));
            mb_proj_forward(verbose_, mbev_grid.pjptr, ping->bathlon[ibeam], ping->bathlat[ibeam],
                    &ping->bathx[ibeam], &ping->bathy[ibeam], &error_);
          }
        }
      }
    }
  }

  /* recalculate grid */
  Backend::make_grid();

  /* update the grid to mbview */
  mbview_updateprimarygrid(verbose_, 0, mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.val, &error_);
  mbview_updatesecondarygrid(verbose_, 0, mbev_grid.n_columns, mbev_grid.n_rows, mbev_grid.sgm, &error_);

  /* turn message of */
  (*hideMessage)();

  /* redisplay grid */
  mbview_plothigh(0);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_flagsparsevoxels(int sizemultiplier, int nsoundingthreshold)

{
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_flagsparsevoxels: sizemultiplier:%d nsoundingthreshold:%d\n", sizemultiplier,
            nsoundingthreshold);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       sizemultiplier:        %d\n", sizemultiplier);
    fprintf(stderr, "dbg2       nsoundingthreshold:    %d\n", nsoundingthreshold);
  }

  /* copy bias parameters */
  sizeMultiplier_ = sizemultiplier;
  nSoundingThreshold_ = nsoundingthreshold;

  /* turn message on */
  snprintf(message, sizeof(message), "Filtering sparse (n<%d) voxels (%dXcell)", nsoundingthreshold, sizemultiplier);
  (*showMessage)(message);
  fprintf(stderr, "\nFlagging soundings in sparse voxels:\n");
  fprintf(stderr, "\tvoxel size: %d x cell size = %f meters\n", sizemultiplier, sizemultiplier * grid_cellsize_);
  fprintf(stderr, "\tflag threshold: n < %d soundings within 3X3X3 voxel volume\n", nsoundingthreshold);

  /* get number of voxels */
  const double dx = sizemultiplier * grid_cellsize_;
  const double dy = sizemultiplier * grid_cellsize_;
  const double dz = sizemultiplier * grid_cellsize_;
  int n_columns = (selected_.xmax - selected_.xmin) / dx;
  int n_rows = (selected_.ymax - selected_.ymin) / dy;
  int nz = (selected_.zmax - selected_.zmin) / dz;
  int cn_columns = n_columns / 10 + 1;
  int cn_rows = n_rows / 10 + 1;
  int cnz = nz / 10 + 1;
  n_columns = 10 * cn_columns;
  n_rows = 10 * cn_rows;
  nz = 10 * cnz;
  int nvoxels_occupied = 0;
  // fprintf(stderr,"Volume Bounds: %f %f  %f %f  %f %f  dxyz:%f  Dims: %d %d %d\n",
  // selected_.xmin,selected_.xmax,selected_.ymin,selected_.ymax,selected_.zmin,selected_.zmax,
  // dx,n_columns,n_rows,nz);

  /* allocate arrays for lists of occupied voxels */
  // fprintf(stderr,"\nArray pointers before: %p %p %p\n",ncoarsevoxels,ncoarsevoxels_alloc,coarsevoxels);
  size_t alloc_size = cn_columns * cn_rows * cnz * sizeof(int);
  // fprintf(stderr,"Alloc sizes: %zu ", alloc_size);
  int *ncoarsevoxels = NULL;
  if ((status_ = mb_mallocd(verbose_, __FILE__, __LINE__, alloc_size, (void **)&ncoarsevoxels, &error_)) ==
      MB_SUCCESS)
    memset(ncoarsevoxels, 0, alloc_size);
  int *ncoarsevoxels_alloc = NULL;
  if ((status_ = mb_mallocd(verbose_, __FILE__, __LINE__, alloc_size, (void **)&ncoarsevoxels_alloc, &error_)) ==
      MB_SUCCESS)
    memset(ncoarsevoxels_alloc, 0, alloc_size);
  alloc_size = cn_columns * cn_rows * cnz * sizeof(int *);
  // fprintf(stderr," %zu\n", alloc_size);
  int **coarsevoxels = NULL;
  if ((status_ = mb_mallocd(verbose_, __FILE__, __LINE__, alloc_size, (void **)&coarsevoxels, &error_)) ==
      MB_SUCCESS)
    memset(coarsevoxels, 0, alloc_size);
  const int voxel_size = (nSoundingThreshold_ + 5);
  const int nvoxels_alloc_chunk = n_columns * n_rows * 2 / 10; /* figure occupied voxels likely to number about twice a horizontal slice */
  // fprintf(stderr,"Array pointers after: %p %p %p\n",ncoarsevoxels,ncoarsevoxels_alloc,coarsevoxels);

  /* loop over all soundings setting occupied voxels as needed */
  if (status_ == MB_SUCCESS) {
    for (int isounding = 0; isounding < selected_.num_soundings; isounding++) {
      Sounding *sounding = &selected_.soundings[isounding];
      if (mb_beam_ok(sounding->beamflag)) {
        const int i = (sounding->x - selected_.xmin) / dx;
        const int j = (sounding->y - selected_.ymin) / dy;
        const int k = (sounding->z - selected_.zorigin - selected_.zmin) / dz;

        /* loop over the neighborhood (+/- 1) of the voxel containing
         * this sounding, setting occupancy for the containing voxel and
         * neighbor occupancy for the surrounding voxels */
        const int i0 = MAX(i - 1, 0);
        const int i1 = MIN(i + 1, n_columns - 1);
        const int j0 = MAX(j - 1, 0);
        const int j1 = MIN(j + 1, n_rows - 1);
        const int k0 = MAX(k - 1, 0);
        const int k1 = MIN(k + 1, nz - 1);
        for (int iii = i0; iii <= i1; iii++) {
          for (int jjj = j0; jjj <= j1; jjj++) {
            for (int kkk = k0; kkk <= k1; kkk++) {
              /* is this the occupied voxel or a neighbor */
              bool occupied_voxel = i == iii && j == jjj && k == kkk;

              /* get coarse voxel */
              const int ii = i / 10;
              const int jj = j / 10;
              const int kk = k / 10;
              const int ll = ii + jj * cn_columns + kk * cn_columns * cn_rows;

              /* look for voxel already set in the appropriate coarse voxel */
              int *voxels = NULL;
              int *voxel;
              int nvoxels = ncoarsevoxels[ll];
              int nvoxels_alloc = ncoarsevoxels_alloc[ll];
              voxels = coarsevoxels[ll];

              bool found = false;
              int ivoxeluse = 0;
              if (nvoxels > 0 && voxels != NULL) {
                for (int ivoxel = 0; ivoxel < nvoxels && !found; ivoxel++) {
                  voxel = &voxels[ivoxel * voxel_size];
                  if (iii == voxel[0] && jjj == voxel[1] && kkk == voxel[2]) {
                    found = true;
                    ivoxeluse = ivoxel;
                  }
                }
              }

              /* if needed allocate more space for a new voxel to the list */
              if (!found && nvoxels_alloc <= nvoxels) {
                nvoxels_alloc += nvoxels_alloc_chunk;
                alloc_size = nvoxels_alloc * voxel_size * sizeof(int);
                status_ = mb_reallocd(verbose_, __FILE__, __LINE__, alloc_size,
                                          (void **)&coarsevoxels[ll], &error_);

                if (status_ == MB_SUCCESS) {
                  voxels = coarsevoxels[ll];
                  alloc_size = nvoxels_alloc_chunk * voxel_size * sizeof(int);
                  memset(&voxels[ncoarsevoxels_alloc[ll] * voxel_size], 0, alloc_size);
                  ncoarsevoxels_alloc[ll] = nvoxels_alloc;
                }
              }

              /* if needed add a new voxel to the list */
              if (status_ == MB_SUCCESS && !found) {
                ivoxeluse = nvoxels;
                voxel = &voxels[ivoxeluse * voxel_size];
                voxel[0] = iii;
                voxel[1] = jjj;
                voxel[2] = kkk;
                voxel[3] = 0;
                voxel[4] = 0;
                nvoxels++;
                ncoarsevoxels[ll] = nvoxels;
              }

              /* add sounding to voxel list */
              if (status_ == MB_SUCCESS) {
                voxel = &voxels[ivoxeluse * voxel_size];
                if (occupied_voxel) {
                  int nsoundingsinvoxel = voxel[3];
                  if (nsoundingsinvoxel < nSoundingThreshold_) {
                    voxel[5 + nsoundingsinvoxel] = isounding;
                  }
                  voxel[3]++;
                  if (voxel[3] == 1)
                    nvoxels_occupied++;
                }
                else {
                  voxel[4]++;
                }
              }
              // fprintf(stderr,"Pt:%f %f %f  IJK:%d %d %d CIJK:%d %d %d LL:%d  NVOX:%d:%d Found:%d IVOXUSE:%d\n",
              // sounding->x,sounding->y,sounding->z,i,j,k,ii,jj,kk,ll,nvoxels,nvoxels_alloc,found,ivoxeluse);
            }
          }
        }
      }

      if (isounding % 100000 == 0 && isounding > 0) {
        /* update message */
        snprintf(message, sizeof(message), "Processed %d of %d soundings, %d voxels occupied", isounding, selected_.num_soundings,
                (int)nvoxels_occupied);
        (*showMessage)(message);
        fprintf(stderr, "%s\n", message);
      }
    }
  }

  /* turn message on */
  snprintf(message, sizeof(message), "Filtering sparse (n<%d) voxels (%dXcell)", nsoundingthreshold, sizemultiplier);
  (*showMessage)(message);
  fprintf(stderr, "%s\n", message);

  /* loop over all coarse voxels, within each coarse voxel loop over all
   * occupied voxels and flag soundings in any that have less
   * than the minimum number of soundings */
  if (status_ == MB_SUCCESS) {
    /* count occupied voxels */
    int ncoarsevoxelstot = 0;
    int nvoxelstot = 0;
    for (int ll = 0; ll < cn_columns * cn_rows * cnz; ll++) {
      if (ncoarsevoxels[ll] > 0) {
        ncoarsevoxelstot++;
        int *voxels = coarsevoxels[ll];
        for (int ivoxel = 0; ivoxel < ncoarsevoxels[ll]; ivoxel++) {
          int *voxel = &voxels[ivoxel * voxel_size];
          if (voxel[3] > 0)
            nvoxelstot++;
        }
      }
    }
    fprintf(stderr, "Number of occupied coarse voxels: %10d of %10d\n", ncoarsevoxelstot, cn_columns * cn_rows * cnz);
    fprintf(stderr, "Number of occupied voxels:        %10d of %10d\n", nvoxelstot, n_columns * n_rows * nz);

    /* loop over all occupied voxels */
    int nflagged = 0;
    int nvoxels = 0;
    for (int ll = 0; ll < cn_columns * cn_rows * cnz; ll++) {
      int *voxels = coarsevoxels[ll];
      for (int ivoxel = 0; ivoxel < ncoarsevoxels[ll]; ivoxel++) {
        int *voxel = &voxels[ivoxel * voxel_size];
        if (voxel[3] > 0 && (voxel[3] + voxel[4]) < nSoundingThreshold_) {
          for (int i = 0; i < voxel[3]; i++) {
            int isounding = voxel[5 + i];
            Sounding *sounding = &selected_.soundings[isounding];
            sounding->beamflag = MB_FLAG_FLAG + MB_FLAG_MANUAL;

            /* apply the flag in the primary application */
            Backend::mb3dsoundings_edit(sounding->ifile, sounding->iping, sounding->ibeam, sounding->beamflag,
                                         MB3DSDG_EDIT_NOFLUSH);

            selected_.num_soundings_unflagged--;
            selected_.num_soundings_flagged++;
            nflagged++;
          }
          // fprintf(stderr,"Flagged %d soundings in voxel %d:%d:%d\n", voxel[3],voxel[0], voxel[1], voxel[2]);
        }
        if (voxel[3] > 0)
          nvoxels++;
        if (nvoxels % 10000 == 0) {
          /* update message */
          snprintf(message, sizeof(message), "Processed %d of %d occupied voxels, %d soundings flagged", (int)nvoxels,
                  (int)nvoxels_occupied, nflagged);
          (*showMessage)(message);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* flush all edit events */
    Backend::mb3dsoundings_edit(0, 0, 0, MB_FLAG_NULL, MB3DSDG_EDIT_FLUSHPREVIOUS);
  }

  /* deallocate arrays */
  for (int ll = 0; ll < cn_columns * cn_rows * cnz; ll++)
    status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&coarsevoxels[ll], &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&ncoarsevoxels, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&ncoarsevoxels_alloc, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&coarsevoxels, &error_);

  /* turn message of */
  (*hideMessage)();

  /* redisplay grid */
  mbview_plothigh(0);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_colorsoundings(int color) {

  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_colorsoundings:%d\n", color);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       color:       %d\n", color);
  }

  /* apply the specified color to all unflagged, currently selected soundings
      (currently selected soundings are displayed in the 3D soundings view) */
  for (int isounding = 0; isounding < selected_.num_soundings; isounding++) {
    Sounding *sounding = &selected_.soundings[isounding];
    if (mb_beam_ok(sounding->beamflag)) {
      sounding->beamcolor = color;
      files_[sounding->ifile].pings[sounding->iping].beamcolor[sounding->ibeam] = color;
    }
  }

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error_);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:%d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_optimizebiasvalues(int mode, double *rollbias_best, double *pitchbias_best, double *headingbias_best,
                                                double *timelag_best, double *snell_best) {
  if (verbose_ > 0)
    fprintf(stderr, "Backend::mb3dsoundings_optimizebiasvalues: %d\n", mode);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       mode:       %d\n", mode);
    fprintf(stderr, "dbg2       rollbias_best:       %f\n", *rollbias_best);
    fprintf(stderr, "dbg2       pitchbias_best:      %f\n", *pitchbias_best);
    fprintf(stderr, "dbg2       headingbias_best:    %f\n", *headingbias_best);
    fprintf(stderr, "dbg2       timelag_best:        %f\n", *timelag_best);
    fprintf(stderr, "dbg2       snell_best:          %f\n", *snell_best);
  }

  /* get and save initial bias values */
  // const double rollbias_org = *rollbias_best;
  // const double pitchbias_org = *pitchbias_best;
  // const double headingbias_org = *headingbias_best;
  // const double timelag_org = *timelag_best;
  // const double snell_org = *snell_best;

  /* create grid of bins to calculate variance */
  const double local_grid_dx = 2 * mbev_grid.dx;
  const double local_grid_dy = 2 * mbev_grid.dy;
  const double local_grid_xmin = selected_.xmin - 0.25 * (selected_.xmax - selected_.xmin);
  double local_grid_xmax = selected_.xmax + 0.25 * (selected_.xmax - selected_.xmin);
  const double local_grid_ymin = selected_.ymin - 0.25 * (selected_.ymax - selected_.ymin);
  double local_grid_ymax = selected_.ymax + 0.25 * (selected_.ymax - selected_.ymin);
  const int local_grid_n_columns = (local_grid_xmax - local_grid_xmin) / local_grid_dx + 1;
  const int local_grid_n_rows = (local_grid_ymax - local_grid_ymin) / local_grid_dy + 1;
  local_grid_xmax = local_grid_xmin + local_grid_n_columns * local_grid_dx;
  local_grid_ymax = local_grid_ymin + local_grid_n_rows * local_grid_dy;

  /* allocate arrays for calculating variance */
  const size_t size_double = local_grid_n_columns * local_grid_n_rows * sizeof(double);
  const size_t size_int = local_grid_n_columns * local_grid_n_rows * sizeof(int);
  double *local_grid_first = NULL;
  status_ = mb_mallocd(verbose_, __FILE__, __LINE__, size_double, (void **)&local_grid_first, &error_);
  double *local_grid_sum = NULL;
  status_ = mb_mallocd(verbose_, __FILE__, __LINE__, size_double, (void **)&local_grid_sum, &error_);
  double *local_grid_sum2 = NULL;
  status_ = mb_mallocd(verbose_, __FILE__, __LINE__, size_double, (void **)&local_grid_sum2, &error_);
  double *local_grid_variance = NULL;
  status_ = mb_mallocd(verbose_, __FILE__, __LINE__, size_double, (void **)&local_grid_variance, &error_);
  int *local_grid_num = NULL;
  status_ = mb_mallocd(verbose_, __FILE__, __LINE__, size_int, (void **)&local_grid_num, &error_);

  /* now loop over all different values of bias parameters looking for the
   * combination that minimizes the overall variance
   * - if a good set of values is found (measured by variace reduction)
   * then set the values and apply them before returning */
  fprintf(stderr,"\nMBeditviz: Optimizing Bias Parameters\n");
  fprintf(stderr,"  Number of selected soundings: %d\n", selected_.num_soundings);
  if (mode == MB3DSDG_OPTIMIZEBIASVALUES_R)
    fprintf(stderr,"  Mode: Roll Bias\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_P)
    fprintf(stderr,"  Mode: Pitch Bias\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_H)
    fprintf(stderr,"  Mode: Heading Bias\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_P + MB3DSDG_OPTIMIZEBIASVALUES_P)
    fprintf(stderr,"  Mode: Roll Bias and Pitch Bias\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_P + MB3DSDG_OPTIMIZEBIASVALUES_P + MB3DSDG_OPTIMIZEBIASVALUES_H)
    fprintf(stderr,"  Mode: Roll Bias and Pitch Bias and Heading Bias\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_T)
    fprintf(stderr,"  Mode: Time Lag\n");
  else if (mode == MB3DSDG_OPTIMIZEBIASVALUES_S)
    fprintf(stderr,"  Mode: Snell Correction\n");
  fprintf(stderr,"------------------------\n");

  /* set flag to set best total variance on first calculation */
  bool first = true;

  // TODO(schwehr): Localize variables.
  mb_path message_string = "";
  double variance_total;
  double variance_total_best = 0.0;
  int variance_total_num = 0;
  double rollbias;
  double pitchbias;
  double headingbias;
  double timelag;
  double snell;
  double rollbias_start, rollbias_end, drollbias;
  double pitchbias_start, pitchbias_end, dpitchbias;
  double headingbias_start, headingbias_end, dheadingbias;
  // double timelag_start;
  // double timelag_end;
  // double dtimelag;
  // double snell_start, snell_end;
  // double dsnell;
  // int niterate;
  char *marker1 = "       ";
  char *marker2 = " ******";
  char *marker = NULL;

  /* Roll bias */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_R) {
    /* start with coarse roll bias */
    int niterate = 11;
    rollbias_start = *rollbias_best - 5.0;
    rollbias_end = *rollbias_best + 5.0;
    drollbias = (rollbias_end - rollbias_start) / (niterate - 1);
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      rollbias = rollbias_start + i * drollbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *rollbias_best = rollbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "COARSE ROLLBIAS:    | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: r:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            rollbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing Roll Bias:%.2f Variance: %.3f %.3f", rollbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }

    /* now do fine roll bias */
    niterate = 19;
    rollbias_start = *rollbias_best - 0.9;
    rollbias_end = *rollbias_best + 0.9;
    drollbias = (rollbias_end - rollbias_start) / (niterate - 1);
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      rollbias = rollbias_start + i * drollbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *rollbias_best = rollbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE ROLLBIAS:      | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: r:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            rollbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Roll Bias:%.2f Variance: %.3f %.3f", rollbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Pitch bias */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_P) {
    /* start with coarse pitch bias */
    rollbias = *rollbias_best;
    int niterate = 11;
    pitchbias_start = *pitchbias_best - 5.0;
    pitchbias_end = *pitchbias_best + 5.0;
    dpitchbias = (pitchbias_end - pitchbias_start) / (niterate - 1);
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      pitchbias = pitchbias_start + i * dpitchbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *pitchbias_best = pitchbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "COARSE PITCHBIAS:     | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: p:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            pitchbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Pitch Bias:%.2f Variance: %.3f %.3f", pitchbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }

    /* now do fine pitch bias */
    rollbias = *rollbias_best;
    niterate = 19;
    pitchbias_start = *pitchbias_best - 0.9;
    pitchbias_end = *pitchbias_best + 0.9;
    dpitchbias = (pitchbias_end - pitchbias_start) / (niterate - 1);
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      pitchbias = pitchbias_start + i * dpitchbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *pitchbias_best = pitchbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE PITCHBIAS:     | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: p:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            pitchbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Pitch Bias:%.2f Variance: %.3f %.3f", pitchbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Heading bias */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_H) {
    /* start with coarse heading bias */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    int niterate = 11;
    headingbias_start = *headingbias_best - 5.0;
    headingbias_end = *headingbias_best + 5.0;
    dheadingbias = (headingbias_end - headingbias_start) / (niterate - 1);
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      headingbias = headingbias_start + i * dheadingbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *headingbias_best = headingbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "COARSE HEADINGBIAS: | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: h:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            headingbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing Heading Bias:%.2f Variance: %.3f %.3f", headingbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }

    /* now do fine heading bias */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    niterate = 19;
    headingbias_start = *headingbias_best - 0.9;
    headingbias_end = *headingbias_best + 0.9;
    dheadingbias = (headingbias_end - headingbias_start) / (niterate - 1);
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      headingbias = headingbias_start + i * dheadingbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *headingbias_best = headingbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE HEADINGBIAS:   | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: h:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            headingbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Heading Bias:%.2f Variance: %.3f %.3f", headingbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Redo roll bias if doing a combination of bias parameters */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_R && mode != MB3DSDG_OPTIMIZEBIASVALUES_R) {
    /* now do fine roll bias */
    int niterate = 19;
    rollbias_start = *rollbias_best - 0.9;
    rollbias_end = *rollbias_best + 0.9;
    drollbias = (rollbias_end - rollbias_start) / (niterate - 1);
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      rollbias = rollbias_start + i * drollbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *rollbias_best = rollbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE ROLLBIAS:      | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: r:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            rollbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Roll Bias:%.2f Variance: %.3f %.3f", rollbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Redo pitch bias if doing a combination of bias parameters */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_P && mode != MB3DSDG_OPTIMIZEBIASVALUES_P) {
    /* now do fine pitch bias */
    rollbias = *rollbias_best;
    int niterate = 19;
    pitchbias_start = *pitchbias_best - 0.9;
    pitchbias_end = *pitchbias_best + 0.9;
    dpitchbias = (pitchbias_end - pitchbias_start) / (niterate - 1);
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      pitchbias = pitchbias_start + i * dpitchbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *pitchbias_best = pitchbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE PITCHBIAS:     | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: p:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            pitchbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Pitch Bias:%.2f Variance: %.3f %.3f", pitchbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Redo heading bias if doing a combination of bias parameters */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_H && mode != MB3DSDG_OPTIMIZEBIASVALUES_H) {
    /* now do fine heading bias */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    int niterate = 19;
    headingbias_start = *headingbias_best - 0.9;
    headingbias_end = *headingbias_best + 0.9;
    dheadingbias = (headingbias_end - headingbias_start) / (niterate - 1);
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      headingbias = headingbias_start + i * dheadingbias;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *headingbias_best = headingbias;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE HEADINGBIAS:   | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: h:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            headingbias, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Heading Bias:%.2f Variance: %.3f %.3f", headingbias, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Time lag */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_T) {
    /* start with coarse time lag */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    int niterate = 21;
          double timelag_start = *timelag_best - 1.0;
    double timelag_end = *timelag_best + 1.0;
    double dtimelag = (timelag_end - timelag_start) / (niterate - 1);
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      timelag = timelag_start + i * dtimelag;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *timelag_best = timelag;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "COARSE TIME LAG:    | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: t:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            timelag, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Time Lag:%.2f Variance: %.3f %.3f", timelag, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }

    /* now do fine time lag */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    niterate = 19;
    timelag_start = *timelag_best - 0.09;
    timelag_end = *timelag_best + 0.09;
    dtimelag = (timelag_end - timelag_start) / (niterate - 1);
    timelag = *timelag_best;
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      timelag = timelag_start + i * dtimelag;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *timelag_best = timelag;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE TIME LAG:      | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: t:%5.2f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            timelag, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Time Lag:%.2f Variance: %.3f %.3f", timelag, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* Snell */
  if (mode & MB3DSDG_OPTIMIZEBIASVALUES_S) {
    /* start with coarse snell */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    int niterate = 21;
    double snell_start = *snell_best - 0.1;
    double snell_end = *snell_best + 0.1;
    double dsnell = (snell_end - snell_start) / (niterate - 1);
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      snell = snell_start + i * dsnell;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *snell_best = snell;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "COARSE SNELL:       | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: s:%5.3f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            snell, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Snell correction:%.4f Variance: %.3f %.3f", snell, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }

    /* now do fine snell */
    rollbias = *rollbias_best;
    pitchbias = *pitchbias_best;
    headingbias = *headingbias_best;
    timelag = *timelag_best;
    niterate = 19;
    snell_start = *snell_best - 0.009;
    snell_end = *snell_best + 0.009;
    dsnell = (snell_end - snell_start) / (niterate - 1);
    snell = *snell_best;
    for (int i = 0; i < niterate; i++) {
      snell = snell_start + i * dsnell;
      Backend::mb3dsoundings_getbiasvariance(
          local_grid_xmin, local_grid_xmax, local_grid_ymin, local_grid_ymax, local_grid_n_columns, local_grid_n_rows, local_grid_dx,
          local_grid_dy, local_grid_first, local_grid_sum, local_grid_sum2, local_grid_variance, local_grid_num, rollbias,
          pitchbias, headingbias, timelag, snell, &variance_total_num, &variance_total);
      if (variance_total_num > 0 && (variance_total < variance_total_best || first)) {
        first = false;
        *snell_best = snell;
        variance_total_best = variance_total;
        marker = marker2;
      }
      else
        marker = marker1;
      fprintf(stderr, "FINE SNELL:         | Best: r:%5.2f p:%5.2f h:%5.2f t:%5.2f s:%5.3f  var:%12.5f | Test: s:%5.3f  N:%d Var:%12.5f %s\n",
            *rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best, variance_total_best,
            snell, variance_total_num, variance_total, marker);
      snprintf(message_string, sizeof(message_string), "Optimizing biases: Snell correction:%.4f Variance: %.3f %.3f", timelag, variance_total,
              variance_total_best);
      (*showMessage)(message_string);
    }
  }

  /* turn off message dialog */
  (*hideMessage)();

  /* deallocate arrays for calculating variance */
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&local_grid_first, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&local_grid_sum, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&local_grid_sum2, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&local_grid_num, &error_);
  status_ = mb_freed(verbose_, __FILE__, __LINE__, (void **)&local_grid_variance, &error_);

  Backend::mb3dsoundings_bias(*rollbias_best, *pitchbias_best, *headingbias_best, *timelag_best, *snell_best);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:              %d\n", error_);
    fprintf(stderr, "dbg2       rollbias_best:      %f\n", *rollbias_best);
    fprintf(stderr, "dbg2       pitchbias_best:     %f\n", *pitchbias_best);
    fprintf(stderr, "dbg2       headingbias_best:   %f\n", *headingbias_best);
    fprintf(stderr, "dbg2       timelag_best:       %f\n", *timelag_best);
    fprintf(stderr, "dbg2       snell_best:         %f\n", *snell_best);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:        %d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
void Backend::mb3dsoundings_getbiasvariance(double local_grid_xmin, double local_grid_xmax, double local_grid_ymin,
                                             double local_grid_ymax, int local_grid_n_columns, int local_grid_n_rows, double local_grid_dx,
                                             double local_grid_dy, double *local_grid_first, double *local_grid_sum,
                                             double *local_grid_sum2, double *local_grid_variance, int *local_grid_num,
                                             double rollbias, double pitchbias, double headingbias, double timelag, double snell,
                                             int *variance_total_num, double *variance_total) {
  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  Function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       local_grid_xmin:     %f\n", local_grid_xmin);
    fprintf(stderr, "dbg2       local_grid_xmax:     %f\n", local_grid_xmax);
    fprintf(stderr, "dbg2       local_grid_ymin:     %f\n", local_grid_ymin);
    fprintf(stderr, "dbg2       local_grid_ymax:     %f\n", local_grid_ymax);
    fprintf(stderr, "dbg2       local_grid_n_columns:       %d\n", local_grid_n_columns);
    fprintf(stderr, "dbg2       local_grid_n_rows:       %d\n", local_grid_n_rows);
    fprintf(stderr, "dbg2       local_grid_dx:       %f\n", local_grid_dx);
    fprintf(stderr, "dbg2       local_grid_dy:       %f\n", local_grid_dy);
    fprintf(stderr, "dbg2       local_grid_first:    %p\n", local_grid_first);
    fprintf(stderr, "dbg2       local_grid_sum:      %p\n", local_grid_sum);
    fprintf(stderr, "dbg2       local_grid_sum2:     %p\n", local_grid_sum2);
    fprintf(stderr, "dbg2       local_grid_variance: %p\n", local_grid_variance);
    fprintf(stderr, "dbg2       local_grid_num:      %p\n", local_grid_num);
    fprintf(stderr, "dbg2       rollbias:            %f\n", rollbias);
    fprintf(stderr, "dbg2       pitchbias:           %f\n", pitchbias);
    fprintf(stderr, "dbg2       headingbias:         %f\n", headingbias);
    fprintf(stderr, "dbg2       timelag:             %f\n", timelag);
    fprintf(stderr, "dbg2       snell:               %f\n", snell);
  }

  /* apply the current bias parameters to the selected soundings */
  Backend::mb3dsoundings_bias(rollbias, pitchbias, headingbias, timelag, snell);

  /* initialize variance */
  *variance_total = 0.0;
  *variance_total_num = 0;
  const size_t size_double = local_grid_n_columns * local_grid_n_rows * sizeof(double);
  const size_t size_int = local_grid_n_columns * local_grid_n_rows * sizeof(int);
  memset(local_grid_first, 0, size_double);
  memset(local_grid_sum, 0, size_double);
  memset(local_grid_sum2, 0, size_double);
  memset(local_grid_variance, 0, size_double);
  memset(local_grid_num, 0, size_int);

  /* calculate variance of soundings in each bin, and then the total variance */
  for (int isounding = 0; isounding < selected_.num_soundings; isounding++) {
    Sounding *sounding = &selected_.soundings[isounding];
    if (mb_beam_ok(sounding->beamflag)) {
      const int i = (sounding->x - local_grid_xmin) / local_grid_dx;
      const int j = (sounding->y - local_grid_ymin) / local_grid_dy;
      if (i >= 0 && i < local_grid_n_columns && j >= 0 && j < local_grid_n_rows) {
        const int k = i * local_grid_n_rows + j;
        if (local_grid_num[k] == 0)
          local_grid_first[k] = sounding->z;
        const double z = sounding->z - local_grid_first[k];
        local_grid_sum[k] += z;
        local_grid_sum2[k] += z * z;
        local_grid_num[k] += 1;
      }
    }
  }
  for (int i = 0; i < local_grid_n_columns; i++) {
    for (int j = 0; j < local_grid_n_rows; j++) {
      const int k = i * local_grid_n_rows + j;
      if (local_grid_num[k] > 0) {
        local_grid_variance[k] =
            (local_grid_sum2[k] - (local_grid_sum[k] * local_grid_sum[k] / local_grid_num[k])) / local_grid_num[k];
        (*variance_total_num)++;
        (*variance_total) += local_grid_variance[k];
      }
    }
  }
  if (*variance_total_num > 0)
    (*variance_total) /= (*variance_total_num);
  // fprintf(stderr,"variance_total_num:%d variance_total:%f\n",*variance_total_num,*variance_total);

  if (verbose_ >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:               %d\n", error_);
    fprintf(stderr, "dbg2       variance_total_num:  %d\n", *variance_total_num);
    fprintf(stderr, "dbg2       variance_total:      %f\n", *variance_total);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status_:         %d\n", status_);
  }
}
/*--------------------------------------------------------------------*/
