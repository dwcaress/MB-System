/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_prog.c  3/23/00
 *
 *    Copyright (c) 2000-2020 by
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
 * Date:  March 23, 2000
 */

/*--------------------------------------------------------------------*/

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/* Need to include windows.h BEFORE the the Xm stuff otherwise VC14+ barf with conflicts */
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#	ifndef WIN32
#		define WIN32
#	endif
#	include <WinSock2.h>
#include <windows.h>
#endif
#include <X11/Intrinsic.h>

#include "mb_aux.h"
#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_xgraphics.h"
#include "mbnavadjust_io.h"
#include "mbnavadjust.h"
#include "mbview.h"
#include "mbsys_ldeoih.h"

/* swath bathymetry raw data structures */
struct pingraw {
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double draft;
  double beams_bath;
  char *beamflag;
  double *bath;
  double *bathacrosstrack;
  double *bathalongtrack;
};

struct swathraw {
  /* raw swath data */
  int file_id;
  int npings;
  int npings_max;
  int beams_bath;
  struct pingraw *pingraws;
};

/* id variables */
static const char program_name[] = "mbnavadjust";
static const char help_message[] = "mbnavadjust is an interactive navigation adjustment package for swath sonar data.\n";
static const char usage_message[] = "mbnavadjust [-Iproject -V -H]";

/* status variables */
int error = MB_ERROR_NO_ERROR;
char *error_message;
char message[STRING_MAX];
char error1[STRING_MAX];
char error2[STRING_MAX];
char error3[STRING_MAX];

/* data file parameters */
void *datalist;

/* MBIO control parameters */
int pings;
int lonflip;
double bounds[4];
int btime_i[7];
int etime_i[7];
double btime_d;
double etime_d;
double speedmin;
double timegap;

/* route color defines (colors different in MBgrdviz than in MBnavadjust) */
#define ROUTE_COLOR_BLACK 0
#define ROUTE_COLOR_WHITE 1
#define ROUTE_COLOR_RED 2
#define ROUTE_COLOR_YELLOW 3
#define ROUTE_COLOR_GREEN 4
#define ROUTE_COLOR_BLUEGREEN 5
#define ROUTE_COLOR_BLUE 6
#define ROUTE_COLOR_PURPLE 7

/* color control values */
#define WHITE 0
#define BLACK 1
#define RED 2
#define GREEN 3
#define BLUE 4
#define CORAL 5
#define YELLOW 6
#define ORANGE 23
#define PURPLE 255

#define XG_SOLIDLINE 0
#define XG_DASHLINE 1
void *pcont_xgid;
void *pcorr_xgid;
void *pzoff_xgid;
void *pmodp_xgid;
int ncolors;
int pixel_values[256];

/* Set these to the dimensions of your canvas drawing */
/* areas, minus 1, located in the uil file             */
static int corr_borders[4];
static int cont_borders[4];
static int zoff_borders[4];
static int modp_borders[4];

/* Projection defines */
#define ModelTypeProjected 1
#define ModelTypeGeographic 2
#define GCS_WGS_84 4326

/* mb_contour parameters */
struct swathraw *swathraw1 = NULL;
struct swathraw *swathraw2 = NULL;
struct swath *swath1 = NULL;
struct swath *swath2 = NULL;
struct ping *ping = NULL;

/* misfit grid parameters */
int grid_nx = 0;
int grid_ny = 0;
int grid_nxy = 0;
int grid_nxyzeq = 0;
double grid_dx = 0.0;
double grid_dy = 0.0;
double grid_olon = 0.0;
double grid_olat = 0.0;
double misfit_min, misfit_max;
int gridm_nx = 0;
int gridm_ny = 0;
int gridm_nxyz = 0;
double *grid1 = NULL;
double *grid2 = NULL;
double *gridm = NULL;
double *gridmeq = NULL;
int *gridn1 = NULL;
int *gridn2 = NULL;
int *gridnm = NULL;
#define NINTERVALS_MISFIT 80
int nmisfit_intervals = NINTERVALS_MISFIT;
double misfit_intervals[NINTERVALS_MISFIT];
int nzmisfitcalc;
double zoff_dz;
double zmin, zmax;
double zmisfitmin;
double zmisfitmax;

/* time, user, host variables */
time_t right_now;
char date[32], user[MBP_FILENAMESIZE], *user_ptr, host[MBP_FILENAMESIZE];

/* local prototypes */
void mbnavadjust_plot(double xx, double yy, int ipen);
void mbnavadjust_newpen(int icolor);
void mbnavadjust_setline(int linewidth);
void mbnavadjust_justify_string(double height, char *string, double *s);
void mbnavadjust_plot_string(double x, double y, double hgt, double angle, char *label);

/*--------------------------------------------------------------------*/
int mbnavadjust_init_globals() {
  /* set default global control parameters */
  project.open = false;
  memset(project.name, 0, STRING_MAX);
  strcpy(project.name, "None");
  memset(project.path, 0, STRING_MAX);
  memset(project.datadir, 0, STRING_MAX);
  project.num_files = 0;
  project.num_files_alloc = 0;
  project.files = NULL;
  project.num_blocks = 0;
  project.num_snavs = 0;
  project.num_pings = 0;
  project.num_beams = 0;
  project.num_crossings = 0;
  project.num_crossings_alloc = 0;
  project.num_crossings_analyzed = 0;
  project.num_goodcrossings = 0;
  project.num_truecrossings = 0;
  project.num_truecrossings_analyzed = 0;
  project.crossings = NULL;
  project.num_ties = 0;
  project.inversion_status = MBNA_INVERSION_NONE;
  project.ref_grid_status = MBNA_GRID_NONE;
  project.grid_status = MBNA_GRID_NONE;
  project.modelplot = false;
  project.modelplot_style = MBNA_MODELPLOT_TIMESERIES;
  project.logfp = NULL;
  mbna_status = MBNA_STATUS_GUI;
  mbna_view_list = MBNA_VIEW_LIST_FILES;
  mbna_view_mode = MBNA_VIEW_MODE_ALL;
  mbna_invert_mode = MBNA_INVERT_ZISOLATED;
  mbna_save_frequency = 10;
  mbna_color_foreground = BLACK;
  mbna_color_background = WHITE;
  project.section_length = 0.14;
  project.section_soundings = 100000;
  project.decimation = 1;
  project.precision = SIGMA_MINIMUM;
  project.smoothing = MBNA_SMOOTHING_DEFAULT;
  project.zoffsetwidth = 5.0;
  project.triangle_scale = 0.0;
  mbna_file_id_1 = MBNA_SELECT_NONE;
  mbna_section_1 = MBNA_SELECT_NONE;
  mbna_file_id_2 = MBNA_SELECT_NONE;
  mbna_section_2 = MBNA_SELECT_NONE;
  mbna_current_crossing = MBNA_SELECT_NONE;
  mbna_current_tie = MBNA_SELECT_NONE;
  mbna_naverr_load = false;
  mbna_file_select = MBNA_SELECT_NONE;
  mbna_survey_select = MBNA_SELECT_NONE;
  mbna_section_select = MBNA_SELECT_NONE;
  mbna_crossing_select = MBNA_SELECT_NONE;
  mbna_tie_select = MBNA_SELECT_NONE;
  project.cont_int = 1.;
  project.col_int = 5.;
  project.tick_int = 5.;
  project.label_int = 100000.;
  mbna_contour = NULL;
  mbna_contour1.nvector = 0;
  mbna_contour1.nvector_alloc = 0;
  mbna_contour1.vector = NULL;
  mbna_contour2.nvector = 0;
  mbna_contour2.nvector_alloc = 0;
  mbna_contour2.vector = NULL;
  mbna_offsetweight = 0.01;
  mbna_zweightfactor = 1.0;
  mbna_misfit_center = MBNA_MISFIT_AUTOCENTER;
  mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;
  mbna_minmisfit = 0.0;
  mbna_bias_mode = MBNA_BIAS_SAME;
  mbna_allow_set_tie = false;
  mbna_modelplot_zoom = false;
  mbna_modelplot_zoom_x1 = 0;
  mbna_modelplot_zoom_x2 = 0;
  mbna_modelplot_tiezoom = false;
  mbna_modelplot_tiestart = 0;
  mbna_modelplot_tieend = 0;
  mbna_modelplot_tiestartzoom = 0;
  mbna_modelplot_tieendzoom = 0;
  mbna_modelplot_pickfile = MBNA_SELECT_NONE;
  mbna_modelplot_picksection = MBNA_SELECT_NONE;
  mbna_modelplot_picksnav = MBNA_SELECT_NONE;
  mbna_block_select = MBNA_SELECT_NONE;
  mbna_block_select1 = MBNA_SELECT_NONE;
  mbna_block_select2 = MBNA_SELECT_NONE;
  mbna_reset_crossings = false;
  mbna_bin_swathwidth = 160.0;
  mbna_bin_pseudobeamwidth = 1.0;
  mbna_bin_beams_bath = mbna_bin_swathwidth / mbna_bin_pseudobeamwidth + 1;

  /* set mbio default values */
  int iformat;
  const int status = mb_defaults(mbna_verbose, &iformat, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
  pings = 1;
  lonflip = 0;
  bounds[0] = -360.;
  bounds[1] = 360.;
  bounds[2] = -90.;
  bounds[3] = 90.;
  btime_i[0] = 1962;
  btime_i[1] = 2;
  btime_i[2] = 21;
  btime_i[3] = 10;
  btime_i[4] = 30;
  btime_i[5] = 0;
  btime_i[6] = 0;
  etime_i[0] = 2062;
  etime_i[1] = 2;
  etime_i[2] = 21;
  etime_i[3] = 10;
  etime_i[4] = 30;
  etime_i[5] = 0;
  etime_i[6] = 0;
  speedmin = 0.0;
  timegap = 1000000000.0;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_init(int argc, char **argv) {
  bool fileflag = false;
  char ifile[STRING_MAX] = "";

  bool errflg = false;
  int c;
  bool help = false;
  // bool flag = false;

  /* process argument list */
  while ((c = getopt(argc, argv, "VvHhDdI:i:Rr")) != -1)
    switch (c) {
    case 'H':
    case 'h':
      help = true;
      break;
    case 'V':
    case 'v':
      mbna_verbose++;
      break;
    case 'D':
    case 'd':
      mbna_color_foreground = WHITE;
      mbna_color_background = BLACK;
      break;
    case 'I':
    case 'i':
      sscanf(optarg, "%s", ifile);
      // flag = true;
      fileflag = true;
      break;
    case 'R':
    case 'r':
      mbna_reset_crossings = true;
      break;
    case '?':
      errflg = true;
    }

  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  if (mbna_verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       mbna_verbose:         %d\n", mbna_verbose);
    fprintf(stderr, "dbg2       help:            %d\n", help);
    fprintf(stderr, "dbg2       input file:      %s\n", ifile);
  }

  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       argc:      %d\n", argc);
    for (int i = 0; i < argc; i++)
      fprintf(stderr, "dbg2       argv[%d]:    %s\n", i, argv[i]);
  }

  /* if file specified then use it */
  int status = MB_SUCCESS;
  if (fileflag) {
    status = mbnavadjust_file_open(ifile);
    do_update_status();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_colors(int ncol, int *pixels) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ncolors:      %d\n", ncol);
    for (int i = 0; i < ncol; i++)
      fprintf(stderr, "dbg2       pixel[%d]:     %d\n", i, pixels[i]);
  }

  ncolors = ncol;
  for (int i = 0; i < ncolors; i++)
    pixel_values[i] = pixels[i];

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_set_borders(int *cn_brdr, int *cr_brdr, int *zc_brdr) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       cn_brdr:      %d %d %d %d\n", cn_brdr[0], cn_brdr[1], cn_brdr[2], cn_brdr[3]);
    fprintf(stderr, "dbg2       cr_brdr:      %d %d %d %d\n", cr_brdr[0], cr_brdr[1], cr_brdr[2], cr_brdr[3]);
    fprintf(stderr, "dbg2       zc_brdr:      %d %d %d %d\n", zc_brdr[0], zc_brdr[1], zc_brdr[2], zc_brdr[3]);
  }

  for (int i = 0; i < 4; i++) {
    cont_borders[i] = cn_brdr[i];
    corr_borders[i] = cr_brdr[i];
    zoff_borders[i] = zc_brdr[i];
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_set_graphics(void *cn_xgid, void *cr_xgid, void *zc_xgid) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       cn_xgid:      %p\n", cn_xgid);
    fprintf(stderr, "dbg2       cr_xgid:      %p\n", cr_xgid);
    fprintf(stderr, "dbg2       zc_xgid:      %p\n", zc_xgid);
  }

  /* set graphics id */
  pcont_xgid = cn_xgid;
  pcorr_xgid = cr_xgid;
  pzoff_xgid = zc_xgid;

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_file_new(char *projectname) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       projectname:  %s\n", projectname);
  }

  /* no new project if one already open */
  int status = MB_SUCCESS;
  if (project.open) {
    strcpy(error1, "Unable to create new project!");
    sprintf(error2, "Project %s", project.name);
    strcpy(error3, "is already open.");
    status = MB_FAILURE;
  }

  /* get filenames and see if they can be generated */
  else {
    char *slashptr = strrchr(projectname, '/');
    char *nameptr = slashptr != NULL ? slashptr + 1 : projectname;
    if (strlen(nameptr) > 4 && strcmp(&nameptr[strlen(nameptr) - 4], ".nvh") == 0)
      nameptr[strlen(nameptr) - 4] = '\0';
    if (strlen(nameptr) > 0) {
      strcpy(project.name, nameptr);
      strncpy(project.path, projectname, strlen(projectname) - strlen(nameptr));
      strcpy(project.home, project.path);
      strcat(project.home, project.name);
      strcat(project.home, ".nvh");
      strcpy(project.datadir, project.path);
      strcat(project.datadir, project.name);
      strcat(project.datadir, ".dir");

      /* no new project if file or directory already exist */
      struct stat statbuf;
      if (stat(project.home, &statbuf) == 0) {
        strcpy(error1, "Unable to create new project!");
        strcpy(error2, "Home file already exists.");
        strcpy(error3, " ");
        if (stat(project.datadir, &statbuf) == 0)
          sprintf(error3, "Data directory already exists.");
        status = MB_FAILURE;
      }
      else if (stat(project.datadir, &statbuf) == 0) {
        strcpy(error1, "Unable to create new project!");
        strcpy(error2, "Data directory already exists.");
        strcpy(error3, " ");
        status = MB_FAILURE;
      }

      /* initialize new project */
      else {
        /* set values */
        project.open = true;
        project.num_files = 0;
        project.num_files_alloc = 0;
        project.files = NULL;
        project.num_snavs = 0;
        project.num_pings = 0;
        project.num_beams = 0;
        project.num_crossings = 0;
        project.num_crossings_alloc = 0;
        project.num_crossings_analyzed = 0;
        project.num_goodcrossings = 0;
        project.num_truecrossings = 0;
        project.num_truecrossings_analyzed = 0;
        project.crossings = NULL;
        project.num_ties = 0;
        project.inversion_status = MBNA_INVERSION_NONE;
        project.grid_status = MBNA_GRID_NONE;
        project.precision = SIGMA_MINIMUM;
        project.smoothing = MBNA_SMOOTHING_DEFAULT;
        project.zoffsetwidth = 5.0;
        project.save_count = 0;

                /* set the plotting function pointers */
                project.mbnavadjust_plot = &mbnavadjust_plot;
                project.mbnavadjust_newpen = &mbnavadjust_newpen;
                project.mbnavadjust_setline = &mbnavadjust_setline;
                project.mbnavadjust_justify_string = &mbnavadjust_justify_string;
                project.mbnavadjust_plot_string = &mbnavadjust_plot_string;

/* create data directory */
#ifdef _WIN32
        if (_mkdir(project.datadir) != 0)
#else
        if (mkdir(project.datadir, 00775) != 0)
#endif
        {
          strcpy(error1, "Unable to create new project!");
          strcpy(error2, "Error creating data directory.");
          strcpy(error3, " ");
          status = MB_FAILURE;
        }

        /* write home file and other files */
        else if ((status = mbnavadjust_write_project(mbna_verbose, &project, &error)) == MB_FAILURE) {
          strcpy(error1, "Unable to create new project!");
          strcpy(error2, "Error writing data.");
          strcpy(error3, " ");
          status = MB_FAILURE;
        }
      }
    }
    else {
      strcpy(error1, "Unable to create new project!");
      strcpy(error2, "No project name was provided.");
      strcpy(error3, " ");
      status = MB_FAILURE;
    }
  }

  /* display error message if needed */
  if (status == MB_FAILURE) {
    do_error_dialog(error1, error2, error3);
    sprintf(message, "%s\n > %s\n", error1, error2);
    do_info_add(message, true);
  }
  else {
    /* open log file */
    sprintf(message, "%s/log.txt", project.datadir);
    project.logfp = fopen(message, "w");

    /* add info text */
    sprintf(message, "New project initialized: %s\n > Project home: %s\n", project.name, project.home);
    do_info_add(message, true);
    if (project.logfp != NULL) {
      sprintf(message, "Log file %s/log.txt opened\n", project.datadir);
      do_info_add(message, true);
    }
    else {
      sprintf(message, "Unable to open log file %s/log.txt\n", project.datadir);
      do_info_add(message, true);
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_file_open(char *projectname) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       projectname:  %s\n", projectname);
  }

  /* no new project if one already open */
  int status = MB_SUCCESS;
  if (project.open) {
    strcpy(error1, "Unable to open project!");
    sprintf(error2, "Project %s", project.name);
    strcpy(error3, "is already open.");
    status = MB_FAILURE;
  } else {
    /* get filenames and see if they can be generated */
    char *slashptr = strrchr(projectname, '/');
    char *nameptr = slashptr != NULL ? slashptr + 1 : projectname;
    if (strlen(nameptr) > 4 && strcmp(&nameptr[strlen(nameptr) - 4], ".nvh") == 0)
      nameptr[strlen(nameptr) - 4] = '\0';
    if (strlen(nameptr) > 0) {
      strcpy(project.name, nameptr);
      if (slashptr != NULL) {
        strncpy(project.path, projectname, strlen(projectname) - strlen(nameptr));
      }
      else {
        /* char *bufptr = */ getcwd(project.path, MB_PATH_MAXLINE);
        strcat(project.path, "/");
      }
      strcpy(project.home, project.path);
      strcat(project.home, project.name);
      strcat(project.home, ".nvh");
      strcpy(project.datadir, project.path);
      strcat(project.datadir, project.name);
      strcat(project.datadir, ".dir");
      fprintf(stderr, "\nOpening MBnavadjust project:\n\tname:%s\n\tpath:%s\n\thome:%s\n\tdatadir:%s\n",
                    project.name, project.path, project.home, project.datadir);

      /* can't open unless file or directory already exist */
      struct stat statbuf;
      if (stat(project.home, &statbuf) != 0) {
        strcpy(error1, "Unable to open project!");
        strcpy(error2, "Home file does not exist.");
        strcpy(error3, " ");
        if (stat(project.datadir, &statbuf) != 0)
          sprintf(error3, "Data directory does not exist.");
        status = MB_FAILURE;
      }
      else if (stat(project.datadir, &statbuf) != 0) {
        strcpy(error1, "Unable to open project!");
        strcpy(error2, "Data directory does not exist.");
        strcpy(error3, " ");
        status = MB_FAILURE;
      }

      /* open project */
      else {
        /* set values */
        project.num_files = 0;
        project.num_files_alloc = 0;
        project.files = NULL;
        project.num_snavs = 0;
        project.num_pings = 0;
        project.num_beams = 0;
        project.num_crossings = 0;
        project.num_crossings_alloc = 0;
        project.crossings = NULL;
        project.num_ties = 0;
        project.save_count = 0;

        /* read home file and other files */
        if ((status = mbnavadjust_read_project(mbna_verbose, projectname, &project, &error)) == MB_FAILURE) {
          strcpy(error1, "Unable to open project!");
          strcpy(error2, "Error reading data.");
          strcpy(error3, " ");
          status = MB_FAILURE;
        }

        /* reset crossings to unanalyzed if flag is set */
        else if (mbna_reset_crossings) {
          for (int i = 0; i < project.num_crossings; i++) {
            /* read each crossing */
            struct mbna_crossing *crossing = &project.crossings[i];

            /* reset status */
            crossing->status = MBNA_CROSSING_STATUS_NONE;
            crossing->num_ties = 0;
            project.num_crossings_analyzed = 0;
            project.num_truecrossings_analyzed = 0;
            project.num_ties = 0;
            project.inversion_status = MBNA_INVERSION_NONE;
            project.grid_status = MBNA_GRID_OLD;
          }
          for (int i = 0; i < project.num_files; i++) {
            struct mbna_file *file = &project.files[i];
            for (int j = 0; j < file->num_sections; j++) {
              struct mbna_section *section = &file->sections[j];
              for (int k = 0; k < section->num_snav; k++) {
                section->snav_lon_offset[section->num_snav] = 0.0;
                section->snav_lat_offset[section->num_snav] = 0.0;
                section->snav_z_offset[section->num_snav] = 0.0;
              }
            }
          }
        }

        /* set plotting functions */
        mbnavadjust_set_plot_functions(mbna_verbose, &project,
                                       &mbnavadjust_plot,
                                       &mbnavadjust_newpen,
                                       &mbnavadjust_setline,
                                       &mbnavadjust_justify_string,
                                       &mbnavadjust_plot_string, &error);
      }
    }
    else {
      strcpy(error1, "Unable to open project!");
      strcpy(error2, "No project name was provided.");
      strcpy(error3, " ");
      status = MB_FAILURE;
    }
  }

  /* display error message if needed */
  if (status == MB_FAILURE) {
    do_error_dialog(error1, error2, error3);
    sprintf(message, "%s\n > %s\n", error1, error2);
    do_info_add(message, true);
  }
  else {
    /* open log file */
    sprintf(message, "%s/log.txt", project.datadir);
    project.logfp = fopen(message, "a");

    /* add info text */
    sprintf(message,
            "Project opened: %s\n > Project home: %s\n > Number of Files: %d\n > Number of Crossings Found: %d\n > Number of "
            "Crossings Analyzed: %d\n > Number of Navigation Ties: %d\n",
            project.name, project.home, project.num_files, project.num_crossings, project.num_crossings_analyzed,
            project.num_ties);
    do_info_add(message, true);
    if (project.logfp != NULL) {
      sprintf(message, "Log file %s/log.txt opened\n", project.datadir);
      do_info_add(message, true);
    }
    else {
      sprintf(message, "Unable to open log file %s/log.txt\n", project.datadir);
      do_info_add(message, true);
    }

    /* update topography grid if it does not exist */
    char path[STRING_MAX];
    sprintf(path, "%s/ProjectTopoAdj.grd", project.datadir);
    struct stat file_status;
    if (stat(path, &file_status) != 0) {
      status = mbnavadjust_updategrid();
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_data(char *path, int iformat) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2               path:     %s\n", path);
    fprintf(stderr, "dbg2               format:   %d\n", iformat);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  char filename[STRING_MAX];
  char dfile[STRING_MAX];
  double weight;
  int form;

  /* loop until all files read */
  bool done = false;
  bool firstfile = true;
  while (!done) {
    if (iformat > 0) {
      status = mbnavadjust_import_file(path, iformat, firstfile);
      done = true;
      firstfile = false;
    }
    else if (iformat == -1) {
      if ((status = mb_datalist_open(mbna_verbose, &datalist, path, MB_DATALIST_LOOK_NO, &error)) == MB_SUCCESS) {
        while (!done) {
          if ((status = mb_datalist_read(mbna_verbose, datalist, filename, dfile, &form, &weight, &error)) ==
              MB_SUCCESS) {
            status = mbnavadjust_import_file(filename, form, firstfile);
            firstfile = false;
          }
          else {
            mb_datalist_close(mbna_verbose, &datalist, &error);
            done = true;
          }
        }
      }
    }
  }

  /* look for new crossings */
  status = mbnavadjust_findcrossings();

  /* count the number of blocks */
  project.num_blocks = 0;
  for (int i = 0; i < project.num_files; i++) {
    file = &project.files[i];
    if (i == 0 || !file->sections[0].continuity) {
      project.num_blocks++;
    }
    file->block = project.num_blocks - 1;
    file->block_offset_x = 0.0;
    file->block_offset_y = 0.0;
    file->block_offset_z = 0.0;
  }

  /* set project bounds and scaling */
  for (int i = 0; i < project.num_files; i++) {
    file = &project.files[i];
    for (int j = 0; j < file->num_sections; j++) {
      section = &file->sections[j];
      if (i == 0 && j == 0) {
        project.lon_min = section->lonmin;
        project.lon_max = section->lonmax;
        project.lat_min = section->latmin;
        project.lat_max = section->latmax;
      }
      else {
        project.lon_min = MIN(project.lon_min, section->lonmin);
        project.lon_max = MAX(project.lon_max, section->lonmax);
        project.lat_min = MIN(project.lat_min, section->latmin);
        project.lat_max = MAX(project.lat_max, section->latmax);
      }
    }
  }
  mb_coor_scale(mbna_verbose, 0.5 * (project.lat_min + project.lat_max), &project.mtodeglon, &project.mtodeglat);

  /* write updated project */
  mbnavadjust_write_project(mbna_verbose, &project, &error);
  project.save_count = 0;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_file(char *path, int iformat, bool firstfile) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2               path:      %s\n", path);
    fprintf(stderr, "dbg2               format:    %d\n", iformat);
    fprintf(stderr, "dbg2               firstfile: %d\n", firstfile);
  }

  int status = MB_SUCCESS;
  char ipath[STRING_MAX];
  char mb_suffix[STRING_MAX];
  int iform;
  int format_error = MB_ERROR_NO_ERROR;

  /* get potential processed file name - if success ipath is fileroot string */
  if (mb_get_format(mbna_verbose, path, ipath, &iform, &format_error) != MB_SUCCESS) {
    strcpy(ipath, path);
  }
  sprintf(mb_suffix, "p.mb%d", iformat);
  strcat(ipath, mb_suffix);

  struct stat file_status;
  char npath[STRING_MAX];
  char opath[STRING_MAX];

  /* mbio read and write values */
  void *imbio_ptr = NULL;
  void *ombio_ptr = NULL;
  void *istore_ptr = NULL;
  void *ostore_ptr = NULL;
  int kind = MB_DATA_NONE;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sonardepth;
  double draft;
  double roll;
  double pitch;
  double heave;
  int beams_bath = 0;
  int beams_amp = 0;
  int pixels_ss = 0;
  char comment[MB_COMMENT_MAXLINE];

  int sonartype = MB_TOPOGRAPHY_TYPE_UNKNOWN;
    int sensorhead = 0;
  int *bin_nbath = NULL;
  double *bin_bath = NULL;
  double *bin_bathacrosstrack = NULL;
  double *bin_bathalongtrack = NULL;
  int side;
  double port_time_d, stbd_time_d;
  double angle, dt, alongtrackdistance, xtrackavg, xtrackmax;
  int nxtrack;

  int obeams_bath, obeams_amp, opixels_ss;
  int nread;
  bool first;
  double headingx, headingy, mtodeglon, mtodeglat;
  double lon, lat;
  double navlon_old, navlat_old;
  FILE *nfp;
  struct mbna_file *file = NULL;
  struct mbna_section *section = NULL;
  struct mbna_file *cfile = NULL;
  struct mbna_section *csection = NULL;
  struct mbsys_ldeoih_struct *ostore = NULL;
  struct mb_io_struct *omb_io_ptr = NULL;
  double dx1, dy1;
  int mbp_heading_mode;
  double mbp_headingbias;
  int mbp_rollbias_mode;
  double mbp_rollbias;
  double mbp_rollbias_port;
  double mbp_rollbias_stbd;
  double depthmax, distmax, depthscale, distscale;
  int error_sensorhead = MB_ERROR_NO_ERROR;
  void *tptr;
  int ii1, jj1;

  /* look for processed file and use if available */
  const int fstat = stat(ipath, &file_status);
  if (fstat != 0 || (file_status.st_mode & S_IFMT) == S_IFDIR) {
    strcpy(ipath, path);
  }

  /* now look for existing mbnavadjust output files
   * - increment output id so this mbnavadjust project outputs
   *   a unique nav file for this input file */
  /* Now only use a value of 0 for output_id  - all output navigation should be *.na0 files */
  int output_id = 0;
  //found = false;
  //while (!found) {
  //  sprintf(opath, "%s.na%d", path, output_id);
  //  fstat = stat(opath, &file_status);
  //  if (fstat != 0) {
  //    found = true;
  //  }
  //  else {
  //    output_id++;
  //  }
  //}

  /* turn on message */
  char *root = (char *)strrchr(ipath, '/');
  if (root == NULL)
    root = ipath;
  sprintf(message, "Importing format %d data from %s", iformat, root);
  do_message_on(message);
  fprintf(stderr, "%s\n", message);
  bool output_open = false;
  project.inversion_status = MBNA_INVERSION_NONE;
  project.grid_status = MBNA_GRID_OLD;
  int new_sections = 0;
  int new_pings = 0;
  int new_crossings = 0;
  int good_beams = 0;

  /* allocate mbna_file array if needed */
  if (project.num_files_alloc <= project.num_files) {
    tptr = realloc(project.files, sizeof(struct mbna_file) * (project.num_files_alloc + ALLOC_NUM));
    if (tptr != NULL) {
      project.files = (struct mbna_file *)tptr;
      project.num_files_alloc += ALLOC_NUM;
    }
    else {
      if (project.files != NULL)
        free(project.files);
      status = MB_FAILURE;
      error = MB_ERROR_MEMORY_FAIL;
      mb_error(mbna_verbose, error, &error_message);
      fprintf(stderr, "\nError in function <%s>:\n%s\n", __func__, error_message);
      fprintf(stderr, "\nImportation of <%s> not attempted\n", ipath);
    }
  }

  if (status == MB_SUCCESS) {
    /* initialize reading the swath file */
    if ((status = mb_read_init(mbna_verbose, ipath, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                               &imbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) != MB_SUCCESS) {
      mb_error(mbna_verbose, error, &error_message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", error_message);
      fprintf(stderr, "\nSwath sonar File <%s> not initialized for reading\n", path);
    }
  }

  char *beamflag = NULL;
  double *bath = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *amp = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;

  /* allocate memory for data arrays */
  if (status == MB_SUCCESS) {
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack,
                                 &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack,
                                 &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(mbna_verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)

    /* if error initializing memory then don't read the file */
    if (error != MB_ERROR_NO_ERROR) {
      mb_error(mbna_verbose, error, &error_message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
    }
  }

  /* open nav file */
  if (status == MB_SUCCESS) {
    sprintf(npath, "%s/nvs_%4.4d.mb166", project.datadir, project.num_files);
    if ((nfp = fopen(npath, "w")) == NULL) {
      status = MB_FAILURE;
      error = MB_ERROR_OPEN_FAIL;
    }
  }

  /* read data */
  if (status == MB_SUCCESS) {
    nread = 0;
    bool new_segment = false;
    first = true;
    while (error <= MB_ERROR_NO_ERROR) {
      /* read a ping of data */
      status = mb_get_all(mbna_verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      /* extract all nav values */
      if (error == MB_ERROR_NO_ERROR && (kind == MB_DATA_NAV || kind == MB_DATA_DATA)) {
        status = mb_extract_nav(mbna_verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                                &heading, &draft, &roll, &pitch, &heave, &error);
      }

      /* ignore minor errors */
      if (kind == MB_DATA_DATA && (error == MB_ERROR_TIME_GAP || error == MB_ERROR_OUT_BOUNDS ||
                                   error == MB_ERROR_OUT_TIME || error == MB_ERROR_SPEED_TOO_SMALL)) {
        status = MB_SUCCESS;
        error = MB_ERROR_NO_ERROR;
      }

      /* do not ignore null navigation */
      if (kind == MB_DATA_DATA && navlon == 0.0 && navlat == 0.0) {
        error = MB_ERROR_IGNORE;
      }

            /* deal with survey data */
      if (kind == MB_DATA_DATA) {
        /* int status_sensorhead = */
        mb_sensorhead(mbna_verbose, imbio_ptr, istore_ptr, &sensorhead, &error_sensorhead);
        if (sonartype == MB_TOPOGRAPHY_TYPE_UNKNOWN) {
          status = mb_sonartype(mbna_verbose, imbio_ptr, istore_ptr, &sonartype, &error);
        }

                /* if sonar is interferometric, bin the bathymetry */
        if (sonartype == MB_TOPOGRAPHY_TYPE_INTERFEROMETRIC) {
          /* allocate bin arrays if needed */
          if (bin_nbath == NULL) {
            status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbna_bin_beams_bath * sizeof(int),
                                (void **)&bin_nbath, &error);
            status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbna_bin_beams_bath * sizeof(double),
                                (void **)&bin_bath, &error);
            status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbna_bin_beams_bath * sizeof(double),
                                (void **)&bin_bathacrosstrack, &error);
            status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbna_bin_beams_bath * sizeof(double),
                                (void **)&bin_bathalongtrack, &error);
            for (int i = 0; i < mbna_bin_beams_bath; i++) {
              bin_nbath[i] = 0;
              bin_bath[i] = 0.0;
              bin_bathacrosstrack[i] = 0.0;
              bin_bathalongtrack[i] = 0.0;
            }
          }

          /* figure out if this is a ping to one side or across the whole swath */
          xtrackavg = 0.0;
          xtrackmax = 0.0;
          nxtrack = 0;
          for (int i = 0; i < beams_bath; i++) {
            if (mb_beam_ok(beamflag[i])) {
              xtrackavg += bathacrosstrack[i];
              xtrackmax = MAX(xtrackmax, fabs(bathacrosstrack[i]));
              nxtrack++;
            }
          }
          if (nxtrack > 0) {
            xtrackavg /= nxtrack;
          }
          if (xtrackavg > 0.25 * xtrackmax) {
            side = SIDE_STBD;
            port_time_d = time_d;
          }
          else if (xtrackavg < -0.25 * xtrackmax) {
            side = SIDE_PORT;
            stbd_time_d = time_d;
          }
          else {
            side = SIDE_FULLSWATH;
            stbd_time_d = time_d;
          }

          /* if side = PORT or FULLSWATH then initialize bin arrays */
          if (side == SIDE_PORT || side == SIDE_FULLSWATH) {
            for (int i = 0; i < mbna_bin_beams_bath; i++) {
              bin_nbath[i] = 0;
              bin_bath[i] = 0.0;
              bin_bathacrosstrack[i] = 0.0;
              bin_bathalongtrack[i] = 0.0;
            }
          }

          /* bin the bathymetry */
          for (int i = 0; i < beams_bath; i++) {
            if (mb_beam_ok(beamflag[i])) {
              /* get apparent acrosstrack beam angle and bin accordingly */
              angle = RTD * atan(bathacrosstrack[i] / (bath[i] - sonardepth));
              const int j = (int)floor((angle + 0.5 * mbna_bin_swathwidth + 0.5 * mbna_bin_pseudobeamwidth) /
                             mbna_bin_pseudobeamwidth);
              /* fprintf(stderr,"i:%d bath:%f %f %f sonardepth:%f angle:%f j:%d\n",
              i,bath[i],bathacrosstrack[i],bathalongtrack[i],sonardepth,angle,j); */
              if (j >= 0 && j < mbna_bin_beams_bath) {
                bin_bath[j] += bath[i];
                bin_bathacrosstrack[j] += bathacrosstrack[i];
                bin_bathalongtrack[j] += bathalongtrack[i];
                bin_nbath[j]++;
              }
            }
          }

          /* if side = STBD or FULLSWATH calculate output bathymetry
              - add alongtrack offset to port data from previous ping */
          if (side == SIDE_STBD || side == SIDE_FULLSWATH) {
            dt = port_time_d - stbd_time_d;
            if (dt > 0.0 && dt < 0.5)
              alongtrackdistance = -(port_time_d - stbd_time_d) * speed / 3.6;
            else
              alongtrackdistance = 0.0;
            beams_bath = mbna_bin_beams_bath;
            for (int j = 0; j < mbna_bin_beams_bath; j++) {
              /* fprintf(stderr,"j:%d angle:%f n:%d bath:%f %f %f\n",
              j,j*mbna_bin_pseudobeamwidth - 0.5 *
              mbna_bin_swathwidth,bin_nbath[j],bin_bath[j],bin_bathacrosstrack[j],bin_bathalongtrack[j]); */
              if (bin_nbath[j] > 0) {
                bath[j] = bin_bath[j] / bin_nbath[j];
                bathacrosstrack[j] = bin_bathacrosstrack[j] / bin_nbath[j];
                bathalongtrack[j] = bin_bathalongtrack[j] / bin_nbath[j];
                beamflag[j] = MB_FLAG_NONE;
                if (bin_bathacrosstrack[j] < 0.0)
                  bathalongtrack[j] += alongtrackdistance;
              }
              else {
                beamflag[j] = MB_FLAG_NULL;
                bath[j] = 0.0;
                bathacrosstrack[j] = 0.0;
                bathalongtrack[j] = 0.0;
              }
            }
          }

          /* if side = PORT set nonfatal error so that bathymetry isn't output until
              the STBD data are read too */
          else if (side == SIDE_PORT) {
            error = MB_ERROR_IGNORE;
          }
        }
      }

      /* deal with new file */
      if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && first) {
        file = &project.files[project.num_files];
        file->status = MBNA_FILE_GOODNAV;
        file->id = project.num_files;
        file->output_id = output_id;
        strcpy(file->path, path);
        strcpy(file->file, path);
        mb_get_relative_path(mbna_verbose, file->file, project.path, &error);
        file->format = iformat;
        file->heading_bias = 0.0;
        file->roll_bias = 0.0;
        file->num_snavs = 0;
        file->num_pings = 0;
        file->num_beams = 0;
        file->num_sections = 0;
        file->num_sections_alloc = 0;
        file->sections = NULL;
        project.num_files++;
        new_segment = true;
        first = false;

        /* get bias values */
        mb_pr_get_heading(mbna_verbose, file->path, &mbp_heading_mode, &mbp_headingbias, &error);
        mb_pr_get_rollbias(mbna_verbose, file->path, &mbp_rollbias_mode, &mbp_rollbias, &mbp_rollbias_port,
                           &mbp_rollbias_stbd, &error);
        if (mbp_heading_mode == MBP_HEADING_OFFSET || mbp_heading_mode == MBP_HEADING_CALCOFFSET) {
          file->heading_bias_import = mbp_headingbias;
        }
        else {
          file->heading_bias_import = 0.0;
        }
        if (mbp_rollbias_mode == MBP_ROLLBIAS_SINGLE) {
          file->roll_bias_import = mbp_rollbias;
        }
        else if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
          file->roll_bias_import = 0.5 * (mbp_rollbias_port + mbp_rollbias_stbd);
        }
        else {
          file->roll_bias_import = 0.0;
        }
      }

      /* check if new segment needed */
      else if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && file != NULL
               && (section->distance + distance >= project.section_length ||
                section->num_beams >= project.section_soundings)) {
        new_segment = true;
        /*fprintf(stderr, "NEW SEGMENT: section->distance:%f distance:%f project.section_length:%f\n",
        section->distance, distance, project.section_length);*/
      }

      /* if end of segment or end of file resolve position
          of last snav point in last segment */
      if ((error > MB_ERROR_NO_ERROR || new_segment) && project.num_files > 0
          && file != NULL && (file->num_sections > 0 && section->num_pings > 0)) {
        /* resolve position of last snav point in last segment */
        if (section->num_snav == 1 ||
            (section->distance >= (section->num_snav - 0.5) * project.section_length / (MBNA_SNAV_NUM - 1))) {
          section->snav_id[section->num_snav] = section->num_pings - 1;
          section->snav_num_ties[section->num_snav] = 0;
          section->snav_distance[section->num_snav] = section->distance;
          section->snav_time_d[section->num_snav] = section->etime_d;
          section->snav_lon[section->num_snav] = navlon_old;
          section->snav_lat[section->num_snav] = navlat_old;
          section->snav_lon_offset[section->num_snav] = 0.0;
          section->snav_lat_offset[section->num_snav] = 0.0;
          section->snav_z_offset[section->num_snav] = 0.0;
          section->num_snav++;
          file->num_snavs++;
          project.num_snavs++;
        }
        else if (section->num_snav > 1) {
          section->snav_id[section->num_snav - 1] = section->num_pings - 1;
          section->snav_num_ties[section->num_snav] = 0;
          section->snav_distance[section->num_snav - 1] = section->distance;
          section->snav_time_d[section->num_snav - 1] = section->etime_d;
          section->snav_lon[section->num_snav - 1] = navlon_old;
          section->snav_lat[section->num_snav - 1] = navlat_old;
          section->snav_lon_offset[section->num_snav - 1] = 0.0;
          section->snav_lat_offset[section->num_snav - 1] = 0.0;
          section->snav_z_offset[section->num_snav - 1] = 0.0;
        }
      }

      /* deal with new segment */
      if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && new_segment) {
        /* end old segment */
        if (output_open) {
          /* close the swath file */
          status = mb_close(mbna_verbose, &ombio_ptr, &error);
          output_open = false;
        }

        /* allocate mbna_section array if needed */
        if (file->num_sections_alloc <= file->num_sections) {
          file->sections = (struct mbna_section *)realloc(file->sections, sizeof(struct mbna_section) *
                                                                              (file->num_sections_alloc + ALLOC_NUM));
          if (file->sections != NULL)
            file->num_sections_alloc += ALLOC_NUM;
          else {
            status = MB_FAILURE;
            error = MB_ERROR_MEMORY_FAIL;
          }
        }

        /* initialize new section */
        file->num_sections++;
        new_sections++;
        section = &file->sections[file->num_sections - 1];
        section->num_pings = 0;
        section->num_beams = 0;
        section->continuity = false;
        section->global_start_ping = project.num_pings;
        section->global_start_snav = project.num_snavs;
        for (int i = 0; i < MBNA_MASK_DIM * MBNA_MASK_DIM; i++)
          section->coverage[i] = 0;
        section->num_snav = 0;
        if (file->num_sections > 1) {
          csection = &file->sections[file->num_sections - 2];
          if (fabs(time_d - csection->etime_d) < MBNA_TIME_GAP_MAX) {
            section->continuity = true;
            section->global_start_snav--;
            file->num_snavs--;
            project.num_snavs--;
          }
        }
        else if (project.num_files > 1 && !firstfile) {
          cfile = &project.files[project.num_files - 2];
          csection = &cfile->sections[cfile->num_sections - 1];
          if (fabs(time_d - csection->etime_d) < MBNA_TIME_GAP_MAX) {
            section->continuity = true;
            section->global_start_snav--;
            file->num_snavs--;
            project.num_snavs--;
          }
        }
        section->distance = 0.0;
        section->btime_d = time_d;
        section->etime_d = time_d;
        section->lonmin = navlon;
        section->lonmax = navlon;
        section->latmin = navlat;
        section->latmax = navlat;
        section->depthmin = 0.0;
        section->depthmax = 0.0;
        section->contoursuptodate = false;
        section->global_tie_status = MBNA_TIE_NONE;
        section->global_tie_snav = MBNA_SELECT_NONE;
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
        new_segment = false;

        /* open output section file */
        sprintf(opath, "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, file->num_sections - 1);
        if ((status = mb_write_init(mbna_verbose, opath, 71, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss,
                                    &error)) != MB_SUCCESS) {
          mb_error(mbna_verbose, error, &error_message);
          fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", error_message);
          fprintf(stderr, "\nSwath sonar File <%s> not initialized for writing\n", path);
        }
        else {
          omb_io_ptr = (struct mb_io_struct *)ombio_ptr;
          ostore_ptr = omb_io_ptr->store_data;
          ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;
          ostore->kind = MB_DATA_DATA;
          ostore->beams_bath = obeams_bath;
          ostore->beams_amp = 0;
          ostore->pixels_ss = 0;
                    ostore->sensorhead = sensorhead;
                    ostore->topo_type = sonartype;
          output_open = true;
          status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, obeams_bath * sizeof(char), (void **)&ostore->beamflag,
                              &error);
          status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, obeams_bath * sizeof(double), (void **)&ostore->bath,
                              &error);
          status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, obeams_bath * sizeof(double),
                              (void **)&ostore->bath_acrosstrack, &error);
          status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, obeams_bath * sizeof(double),
                              (void **)&ostore->bath_alongtrack, &error);

          /* if error initializing memory then don't write the file */
          if (error != MB_ERROR_NO_ERROR) {
            mb_error(mbna_verbose, error, &error_message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
            status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->beamflag, &error);
            status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath, &error);
            status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath_acrosstrack, &error);
            status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath_alongtrack, &error);
            status = mb_close(mbna_verbose, &ombio_ptr, &error);
            output_open = false;
          }
        }
      }

      /* update section distance for each data ping */
      if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && section->num_pings > 1)
        section->distance += distance;

      /* handle good bathymetry */
      if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR) {
        /* get statistics */
        mb_coor_scale(mbna_verbose, navlat, &mtodeglon, &mtodeglat);
        headingx = sin(DTR * heading);
        headingy = cos(DTR * heading);
        navlon_old = navlon;
        navlat_old = navlat;
        section->etime_d = time_d;
        section->num_pings++;
        file->num_pings++;
        project.num_pings++;
        new_pings++;
        if (section->distance >= section->num_snav * project.section_length / (MBNA_SNAV_NUM - 1)) {
          section->snav_id[section->num_snav] = section->num_pings - 1;
          section->snav_num_ties[section->num_snav] = 0;
          section->snav_distance[section->num_snav] = section->distance;
          section->snav_time_d[section->num_snav] = time_d;
          section->snav_lon[section->num_snav] = navlon;
          section->snav_lat[section->num_snav] = navlat;
          section->snav_lon_offset[section->num_snav] = 0.0;
          section->snav_lat_offset[section->num_snav] = 0.0;
          section->snav_z_offset[section->num_snav] = 0.0;
          section->num_snav++;
          file->num_snavs++;
          project.num_snavs++;
        }
        for (int i = 0; i < beams_bath; i++) {
          if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0) {
            good_beams++;
            project.num_beams++;
            file->num_beams++;
            section->num_beams++;
            lon = navlon + headingy * mtodeglon * bathacrosstrack[i] + headingx * mtodeglon * bathalongtrack[i];
            lat = navlat - headingx * mtodeglat * bathacrosstrack[i] + headingy * mtodeglat * bathalongtrack[i];
            if (lon != 0.0)
              section->lonmin = MIN(section->lonmin, lon);
            if (lon != 0.0)
              section->lonmax = MAX(section->lonmax, lon);
            if (lat != 0.0)
              section->latmin = MIN(section->latmin, lat);
            if (lat != 0.0)
              section->latmax = MAX(section->latmax, lat);
            if (section->depthmin == 0.0)
              section->depthmin = bath[i];
            else
              section->depthmin = MIN(section->depthmin, bath[i]);
            if (section->depthmin == 0.0)
              section->depthmax = bath[i];
            else
              section->depthmax = MAX(section->depthmax, bath[i]);
          }
          else {
            beamflag[i] = MB_FLAG_NULL;
            bath[i] = 0.0;
            bathacrosstrack[i] = 0.0;
            bathalongtrack[i] = 0.0;
          }
        }

        /* write out bath data only to format 71 file */
        if (output_open) {
          /*if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
          fprintf(stderr,"%3d %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d %10f %10f %5.2f %6.2f %7.3f %7.3f %4d %4d %4d\n",
          file->num_sections,
          time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
          navlon,navlat,speed,heading,distance,section->distance,
          beams_bath,beams_amp,pixels_ss);*/

          /* get depth and distance scaling */
          depthmax = 0.0;
          distmax = 0.0;
          for (int i = 0; i < beams_bath; i++) {
            depthmax = MAX(depthmax, fabs(bath[i]));
            distmax = MAX(distmax, fabs(bathacrosstrack[i]));
            distmax = MAX(distmax, fabs(bathalongtrack[i]));
          }
          depthscale = MAX(0.001, depthmax / 32000);
          distscale = MAX(0.001, distmax / 32000);
          ostore->depth_scale = depthscale;
          ostore->distance_scale = distscale;
          ostore->sonardepth = draft - heave;
          ostore->roll = roll;
          ostore->pitch = pitch;
          ostore->heave = heave;

          /* write out data */
          status = mb_put_all(mbna_verbose, ombio_ptr, ostore_ptr,
                                        true, MB_DATA_DATA, time_i, time_d, navlon, navlat,
                              speed, heading, beams_bath, 0, 0,
                                        beamflag, bath, amp,
                                        bathacrosstrack, bathalongtrack,
                              ss, ssacrosstrack, ssalongtrack, comment, &error);
        }
      }

      /* write out all nav data to format 166 file */
      if ((kind == MB_DATA_DATA || kind == MB_DATA_NAV) && time_d > 0.0 && time_i[0] > 0 && nfp != NULL) {
        /*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f\r\n",
            time_i[0], time_i[1], time_i[2], time_i[3],
            time_i[4], time_i[5], time_i[6], time_d,
            navlon, navlat, heading, speed);
        fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f\r\n",
            time_i[0], time_i[1], time_i[2], time_i[3],
            time_i[4], time_i[5], time_i[6], time_d,
            navlon, navlat, heading, speed);*/
        /*fprintf(stderr, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.2f %.2f %.2f
           %.2f\r\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d, navlon, navlat,
           heading, speed, draft, roll, pitch, heave);*/
        fprintf(nfp, "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.2f %.2f %.2f %.2f\r\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d, navlon, navlat,
                heading, speed, draft, roll, pitch, heave);
      }

      /* increment counters */
      if (error == MB_ERROR_NO_ERROR)
        nread++;

      /* print debug statements */
      if (mbna_verbose >= 2) {
        fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }
      if (mbna_verbose >= 2 && kind == MB_DATA_COMMENT) {
        fprintf(stderr, "dbg2       comment:        %s\n", comment);
      }
      if (mbna_verbose >= 2 && error <= 0 && kind == MB_DATA_DATA) {
        fprintf(stderr, "dbg2       time_i:         %4d/%2d/%2d %2.2d:%2.2d:%2.2d.%6.6d\n", time_i[0], time_i[1],
                time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
        fprintf(stderr, "dbg2       time_d:         %f\n", time_d);
        fprintf(stderr, "dbg2       navlon:         %.10f\n", navlon);
        fprintf(stderr, "dbg2       navlat:         %.10f\n", navlat);
        fprintf(stderr, "dbg2       speed:          %f\n", speed);
        fprintf(stderr, "dbg2       heading:        %f\n", heading);
        fprintf(stderr, "dbg2       distance:       %f\n", distance);
        fprintf(stderr, "dbg2       beams_bath:     %d\n", beams_bath);
        fprintf(stderr, "dbg2       beams_amp:      %d\n", beams_amp);
        fprintf(stderr, "dbg2       pixels_ss:      %d\n", pixels_ss);
            }
    }

    /* close the swath file */
    status = mb_close(mbna_verbose, &imbio_ptr, &error);
    if (nfp != NULL)
      fclose(nfp);
    if (output_open) {
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->beamflag, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath_acrosstrack, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&ostore->bath_alongtrack, &error);
      status = mb_close(mbna_verbose, &ombio_ptr, &error);
    }

    /* deallocate bin arrays if needed */
    if (bin_nbath != NULL) {
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bin_nbath, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bin_bath, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bin_bathacrosstrack, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bin_bathalongtrack, &error);
    }

    /* get coverage masks for each section */
    if (file != NULL && !first) {
      /* loop over all sections */
      for (int k = 0; k < file->num_sections; k++) {
        /* set section data to be read */
        section = (struct mbna_section *)&file->sections[k];
        sprintf(opath, "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, k);

        /* initialize reading the swath file */
        if ((status = mb_read_init(mbna_verbose, opath, 71, 1, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                                   &ombio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error)) !=
            MB_SUCCESS) {
          mb_error(mbna_verbose, error, &error_message);
          fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", error_message);
          fprintf(stderr, "\nSwath sonar File <%s> not initialized for reading\n", path);
        }

        /* allocate memory for data arrays */
        if (status == MB_SUCCESS) {
          beamflag = NULL;
          bath = NULL;
          amp = NULL;
          bathacrosstrack = NULL;
          bathalongtrack = NULL;
          ss = NULL;
          ssacrosstrack = NULL;
          ssalongtrack = NULL;
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                       (void **)&beamflag, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                       (void **)&bath, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                       (void **)&bathacrosstrack, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                       (void **)&bathalongtrack, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                       (void **)&ssacrosstrack, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(mbna_verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
                                       (void **)&ssalongtrack, &error);

          /* if error initializing memory then don't read the file */
          if (error != MB_ERROR_NO_ERROR) {
            mb_error(mbna_verbose, error, &error_message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", error_message);
          }
        }

        /* loop over reading data */
        dx1 = (section->lonmax - section->lonmin) / MBNA_MASK_DIM;
        dy1 = (section->latmax - section->latmin) / MBNA_MASK_DIM;
        while (error <= MB_ERROR_NO_ERROR) {
          /* read a ping of data */
          status =
              mb_get_all(mbna_verbose, ombio_ptr, &ostore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                         &heading, &distance, &altitude, &sonardepth, &beams_bath, &beams_amp, &pixels_ss, beamflag,
                         bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

          /* ignore minor errors */
          if (kind == MB_DATA_DATA && (error == MB_ERROR_TIME_GAP || error == MB_ERROR_OUT_BOUNDS ||
                                       error == MB_ERROR_OUT_TIME || error == MB_ERROR_SPEED_TOO_SMALL)) {
            status = MB_SUCCESS;
            error = MB_ERROR_NO_ERROR;
          }

          /* check for good bathymetry */
          if (kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR) {
            mb_coor_scale(mbna_verbose, navlat, &mtodeglon, &mtodeglat);
            headingx = sin(DTR * heading);
            headingy = cos(DTR * heading);
            for (int i = 0; i < beams_bath; i++) {
              if (mb_beam_ok(beamflag[i]) && bath[i] != 0.0) {
                lon =
                    navlon + headingy * mtodeglon * bathacrosstrack[i] + headingx * mtodeglon * bathalongtrack[i];
                lat =
                    navlat - headingx * mtodeglat * bathacrosstrack[i] + headingy * mtodeglat * bathalongtrack[i];
                ii1 = (lon - section->lonmin) / dx1;
                jj1 = (lat - section->latmin) / dy1;
                if (ii1 >= 0 && ii1 < MBNA_MASK_DIM && jj1 >= 0 && jj1 < MBNA_MASK_DIM) {
                  section->coverage[ii1 + jj1 * MBNA_MASK_DIM] = 1;
                }
              }
            }
          }
        }
        /*fprintf(stderr,"%s/nvs_%4.4d_%4.4d.mb71\n",
        project.datadir,file->id,k);
        for (int jj1=MBNA_MASK_DIM-1;jj1>=0;jj1--)
        {
        for (int ii1=0;ii1<MBNA_MASK_DIM;ii1++)
        {
        kk1 = ii1 + jj1 * MBNA_MASK_DIM;
        fprintf(stderr, "%1d", section->coverage[kk1]);
        }
        fprintf(stderr, "\n");
        }*/

        /* deallocate memory used for data arrays */
        status = mb_close(mbna_verbose, &ombio_ptr, &error);
      }
    }
  }

  /* add info text */
  if (status == MB_SUCCESS && new_pings > 0) {
    sprintf(message, "Imported format %d file: %s\n > Read %d pings\n > Added %d sections %d crossings\n", iformat, path,
            new_pings, new_sections, new_crossings);
    do_info_add(message, true);
  }
  else {
    sprintf(message, "Unable to import format %d file: %s\n", iformat, path);
    do_info_add(message, true);
  }

  /* turn off message */
  do_message_off();

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_import_reference(char *path) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2               path:     %s\n", path);
  }

  // TODO(schwehr): Was there supposed to be some actual work?

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_bin_bathymetry(double altitude, int beams_bath, char *beamflag, double *bath, double *bathacrosstrack,
                               double *bathalongtrack, int mbna_bin_beams_bath, double mbna_bin_pseudobeamwidth,
                               double mbna_bin_swathwidth, char *bin_beamflag, double *bin_bath, double *bin_bathacrosstrack,
                               double *bin_bathalongtrack, int *error) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2                       mbna_verbose: %d\n", mbna_verbose);
    fprintf(stderr, "dbg2                       altitude:     %f\n", altitude);
    fprintf(stderr, "dbg2                       beams_bath:   %d\n", beams_bath);
    for (int i = 0; i < beams_bath; i++)
      fprintf(stderr, "dbg2                       beam[%d]: %f %f %f %d\n", i, bath[i], bathacrosstrack[i],
              bathalongtrack[i], beamflag[i]);
    fprintf(stderr, "dbg2                       mbna_bin_beams_bath:      %d\n", mbna_bin_beams_bath);
    fprintf(stderr, "dbg2                       mbna_bin_pseudobeamwidth: %f\n", mbna_bin_pseudobeamwidth);
    fprintf(stderr, "dbg2                       mbna_bin_swathwidth:      %f\n", mbna_bin_swathwidth);
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    for (int i = 0; i < mbna_bin_beams_bath; i++)
      fprintf(stderr, "dbg2                       beam[%d]: %f %f %f %d\n", i, bin_bath[i], bin_bathacrosstrack[i],
              bin_bathalongtrack[i], bin_beamflag[i]);
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_findcrossings() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* turn on message */
  sprintf(message, "Checking for crossings...");
  do_message_on(message);

  int status = MB_SUCCESS;

  /* loop over files looking for new crossings */
  if (project.open && project.num_files > 0) {
    /* look for new crossings through all files */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      sprintf(message, "Checking for crossings with file %d of %d...", ifile, project.num_files);
      do_message_update(message);

      status = mbnavadjust_findcrossingsfile(ifile);
    }

    /* resort crossings */
    sprintf(message, "Sorting crossings....");
    do_message_update(message);
    if (project.num_crossings > 1)
      mb_mergesort((void *)project.crossings, (size_t)project.num_crossings, sizeof(struct mbna_crossing),
                   mbnavadjust_crossing_compare);

    /* recalculate overlap fractions, true crossings, good crossing statistics */
    sprintf(message, "Calculating crossing overlaps...");
    do_message_update(message);

    project.num_crossings_analyzed = 0;
    project.num_goodcrossings = 0;
    project.num_truecrossings = 0;
    project.num_truecrossings_analyzed = 0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      struct mbna_crossing *crossing = &(project.crossings[icrossing]);

      /* recalculate crossing overlap */
      mbnavadjust_crossing_overlap(mbna_verbose, &project, icrossing, &error);
      if (crossing->overlap >= 25)
        project.num_goodcrossings++;

      /* check if this is a true crossing */
      if (mbnavadjust_sections_intersect(icrossing)) {
        crossing->truecrossing = true;
        project.num_truecrossings++;
        if (crossing->status != MBNA_CROSSING_STATUS_NONE)
          project.num_truecrossings_analyzed++;
      }
      else
        crossing->truecrossing = false;
      if (crossing->status != MBNA_CROSSING_STATUS_NONE)
        project.num_crossings_analyzed++;
    }

        project.modelplot_uptodate = false;
  }

  /* turn off message */
  do_message_off();

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_compare(const void *a, const void *b) {
  struct mbna_crossing *aa = (struct mbna_crossing *)a;
  struct mbna_crossing *bb = (struct mbna_crossing *)b;

  const int a1id = aa->file_id_1 * 1000 + aa->section_1;
  const int a2id = aa->file_id_2 * 1000 + aa->section_2;
  const int aid = a1id > a2id ? a1id : a2id;

  const int b1id = bb->file_id_1 * 1000 + bb->section_1;
  const int b2id = bb->file_id_2 * 1000 + bb->section_2;
  const int bid =  b1id > b2id ? b1id : b2id;

  if (aid > bid)
    return (1);
  else if (aid < bid)
    return (-1);
  else if (a1id > b1id)
    return (1);
  else if (a1id < b1id)
    return (-1);
  else if (a2id > b2id)
    return (1);
  else if (a2id < b2id)
    return (-1);
  else
    return (0);
}
/*--------------------------------------------------------------------*/
/*   function mbnavadjust_tie_compare compares ties according to the
    misfit magnitude between the tie offset and the inversion model */
int mbnavadjust_tie_compare(const void *a, const void *b) {
  /* get the crossing and tie ids encoded in *a and *b */
  const int id_a = *((int *) a);
  const int id_b = *((int *) b);
  const int icrossing_a = id_a / 100;
  const int icrossing_b = id_b / 100;
  const int itie_a = id_a % 100;
  const int itie_b = id_b % 100;
  struct mbna_crossing *crossing_a = (struct mbna_crossing *)&project.crossings[icrossing_a];
  struct mbna_crossing *crossing_b = (struct mbna_crossing *)&project.crossings[icrossing_b];
  struct mbna_tie *tie_a = &crossing_a->ties[itie_a];
  struct mbna_tie *tie_b = &crossing_b->ties[itie_b];

  /* return according to the misfit magnitude */
  if (tie_a->inversion_status != MBNA_INVERSION_NONE
      && tie_b->inversion_status != MBNA_INVERSION_NONE) {
    if (tie_a->sigma_m > tie_b->sigma_m) {
      return(1);
    } else {
      return(-1);
    }
  } else if (tie_a->inversion_status != MBNA_INVERSION_NONE) {
    return(1);
  } else {
    return(-1);
  }
}

/*--------------------------------------------------------------------*/
int mbnavadjust_findcrossingsfile(int ifile) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2                       ifile: %d\n", ifile);
  }

  int status = MB_SUCCESS;

  /* loop over files and sections, comparing sections from project.files[ifile] with all previous sections */
  if (project.open && project.num_files > 0) {
    struct mbna_file *file2 = &(project.files[ifile]);

    /* loop over all sections */
    for (int isection = 0; isection < file2->num_sections; isection++) {
      // double cell1lonmin, cell1lonmax, cell1latmin, cell1latmax;
      // double cell2lonmin, cell2lonmax, cell2latmin, cell2latmax;
      struct mbna_section *section2 = &(file2->sections[isection]);
      const double lonoffset2 = section2->snav_lon_offset[section2->num_snav / 2];
      const double latoffset2 = section2->snav_lat_offset[section2->num_snav / 2];

      /* get coverage mask adjusted for most recent inversion solution */
      const double lonmin2 = section2->lonmin + lonoffset2;
      const double lonmax2 = section2->lonmax + lonoffset2;
      const double latmin2 = section2->latmin + latoffset2;
      const double latmax2 = section2->latmax + latoffset2;
      const double dx2 = (section2->lonmax - section2->lonmin) / (MBNA_MASK_DIM - 1);
      const double dy2 = (section2->latmax - section2->latmin) / (MBNA_MASK_DIM - 1);

      /* now loop over all previous sections looking for crossings */
      for (int jfile = 0; jfile <= ifile; jfile++) {
        struct mbna_file *file1 = &(project.files[jfile]);
        const int jsectionmax = jfile < ifile ? file1->num_sections : isection;
        for (int jsection = 0; jsection < jsectionmax; jsection++) {
          struct mbna_section *section1 = &(file1->sections[jsection]);
          const double lonoffset1 = section1->snav_lon_offset[section1->num_snav / 2];
          const double latoffset1 = section1->snav_lat_offset[section1->num_snav / 2];

          /* get coverage mask adjusted for most recent inversion solution */
          const double lonmin1 = section1->lonmin + lonoffset1;
          const double lonmax1 = section1->lonmax + lonoffset1;
          const double latmin1 = section1->latmin + latoffset1;
          const double latmax1 = section1->latmax + latoffset1;
          const double dx1 = (section1->lonmax - section1->lonmin) / (MBNA_MASK_DIM - 1);
          const double dy1 = (section1->latmax - section1->latmin) / (MBNA_MASK_DIM - 1);

          /* check if there is overlap given the current navigation model */
          int overlap = 0;
          bool disqualify = false;
          if (jfile == ifile && jsection == isection - 1 && section2->continuity)
            disqualify = true;
          else if (jfile == ifile - 1 && jsection == file1->num_sections - 1 && isection == 0 &&
                   section2->continuity)
            disqualify = true;
          else if (!(lonmin2 < lonmax1 && lonmax2 > lonmin1 && latmin2 < latmax1 && latmax2 > latmin1)) {
            disqualify = true;
          } else {
            /* loop over the coverage mask cells looking for overlap */
            for (int ii2 = 0; ii2 < MBNA_MASK_DIM && overlap == 0; ii2++)
              for (int jj2 = 0; jj2 < MBNA_MASK_DIM && overlap == 0; jj2++) {
                const int kk2 = ii2 + jj2 * MBNA_MASK_DIM;
                if (section2->coverage[kk2] == 1) {
                  const double cell2lonmin = lonmin2 + ii2 * dx2;
                  const double cell2lonmax = lonmin2 + (ii2 + 1) * dx2;
                  const double cell2latmin = latmin2 + jj2 * dy2;
                  const double cell2latmax = latmin2 + (jj2 + 1) * dy2;

                  for (int ii1 = 0; ii1 < MBNA_MASK_DIM && overlap == 0; ii1++) {
                    for (int jj1 = 0; jj1 < MBNA_MASK_DIM && overlap == 0; jj1++) {
                      const int kk1 = ii1 + jj1 * MBNA_MASK_DIM;
                      if (section1->coverage[kk1] == 1) {
                        const double cell1lonmin = lonmin1 + ii1 * dx1;
                        const double cell1lonmax = lonmin1 + (ii1 + 1) * dx1;
                        const double cell1latmin = latmin1 + jj1 * dy2;
                        const double cell1latmax = latmin1 + (jj1 + 1) * dy1;

                        /* check if these two cells overlap */
                        if (cell2lonmin < cell1lonmax && cell2lonmax > cell1lonmin &&
                            cell2latmin < cell1latmax && cell2latmax > cell1latmin) {
                          overlap++;
                        }
                      }
                    }
                  }
                }
              }
          }

          /* if not disqualified and overlap found, then this is a crossing */
          /* check to see if the crossing exists, if not add it */
          if (!disqualify && overlap > 0) {
            bool found = false;
            for (int icrossing = 0; icrossing < project.num_crossings && !found; icrossing++) {
              struct mbna_crossing *crossing = &(project.crossings[icrossing]);
              if (crossing->file_id_2 == ifile && crossing->file_id_1 == jfile && crossing->section_2 == isection &&
                  crossing->section_1 == jsection) {
                found = true;
              }
              else if (crossing->file_id_1 == ifile && crossing->file_id_2 == jfile &&
                       crossing->section_1 == isection && crossing->section_2 == jsection) {
                found = true;
              }
            }
            if (!found) {
              /* allocate mbna_crossing array if needed */
              if (project.num_crossings_alloc <= project.num_crossings) {
                project.crossings = (struct mbna_crossing *)realloc(
                    project.crossings, sizeof(struct mbna_crossing) * (project.num_crossings_alloc + ALLOC_NUM));
                if (project.crossings != NULL)
                  project.num_crossings_alloc += ALLOC_NUM;
                else {
                  status = MB_FAILURE;
                  error = MB_ERROR_MEMORY_FAIL;
                }
              }

              /* add crossing to list */
              struct mbna_crossing *crossing = (struct mbna_crossing *)&project.crossings[project.num_crossings];
              crossing->status = MBNA_CROSSING_STATUS_NONE;
              crossing->truecrossing = false;
              crossing->overlap = 0;
              crossing->file_id_1 = file1->id;
              crossing->section_1 = jsection;
              crossing->file_id_2 = file2->id;
              crossing->section_2 = isection;
              crossing->num_ties = 0;
              project.num_crossings++;

              fprintf(stderr, "added crossing: %d  %4d %4d   %4d %4d\n", project.num_crossings - 1,
                      crossing->file_id_1, crossing->section_1, crossing->file_id_2, crossing->section_2);
            }
            /*else
            fprintf(stderr,"no new crossing:    %4d %4d   %4d %4d   duplicate\n",
            file2->id,isection,
            file1->id,jsection);*/
          }
          /*else
          fprintf(stderr,"disqualified:       %4d %4d   %4d %4d   disqualify:%d overlaptxt list:%d\n",
          file2->id,isection,
          file1->id,jsection, disqualify, overlap);*/
        }
      }
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_addcrossing(int ifile1, int isection1, int ifile2, int isection2) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2                       ifile1:        %d\n", ifile1);
    fprintf(stderr, "dbg2                       isection1:     %d\n", isection1);
    fprintf(stderr, "dbg2                       ifile2:        %d\n", ifile2);
    fprintf(stderr, "dbg2                       isection2:     %d\n", isection2);
  }

  /* check that file and section id's are reasonable */
  bool disqualify = false;
  if (ifile1 == ifile2 && isection1 == isection2)
    disqualify = true;
  if (!disqualify && (ifile1 < 0 || ifile1 >= project.num_files || ifile2 < 0 || ifile2 >= project.num_files))
    disqualify = true;
  if (!disqualify && (isection1 < 0 || isection1 >= project.files[ifile1].num_sections || isection2 < 0 ||
                                isection2 >= project.files[ifile2].num_sections))
    disqualify = true;

  /* check that this crossing does not already exist */
  if (!disqualify) {
    for (int icrossing = 0; icrossing < project.num_crossings && !disqualify; icrossing++) {
      struct mbna_crossing *crossing = (struct mbna_crossing *)&project.crossings[icrossing];
      if ((ifile1 == crossing->file_id_1 && isection1 == crossing->section_1 && ifile2 == crossing->file_id_2 &&
           isection2 == crossing->section_2) ||
          (ifile1 == crossing->file_id_1 && isection1 == crossing->section_1 && ifile2 == crossing->file_id_2 &&
           isection2 == crossing->section_2))
        disqualify = true;
    }
  }

  int status = MB_SUCCESS;

  /* if crossing ok and not yet defined then create it */
  if (!disqualify) {
    /* allocate mbna_crossing array if needed */
    if (project.num_crossings_alloc <= project.num_crossings) {
      project.crossings = (struct mbna_crossing *)realloc(project.crossings, sizeof(struct mbna_crossing) *
                                                                                 (project.num_crossings_alloc + ALLOC_NUM));
      if (project.crossings != NULL)
        project.num_crossings_alloc += ALLOC_NUM;
      else {
        status = MB_FAILURE;
        error = MB_ERROR_MEMORY_FAIL;
      }
    }

    if (status == MB_SUCCESS) {
      /* add crossing to list */
      struct mbna_crossing *crossing = (struct mbna_crossing *)&project.crossings[project.num_crossings];
      crossing->status = MBNA_CROSSING_STATUS_NONE;
      crossing->truecrossing = false;
      crossing->overlap = 0;
      if (ifile1 < ifile2 || (ifile1 == ifile2 && isection1 < isection2)) {
        crossing->file_id_1 = ifile1;
        crossing->section_1 = isection1;
        crossing->file_id_2 = ifile2;
        crossing->section_2 = isection2;
      }
      else {
        crossing->file_id_1 = ifile2;
        crossing->section_1 = isection2;
        crossing->file_id_2 = ifile1;
        crossing->section_2 = isection1;
      }
      crossing->num_ties = 0;
      mbna_crossing_select = project.num_crossings;
      mbna_tie_select = MBNA_SELECT_NONE;
      project.num_crossings++;

      fprintf(stderr, "added crossing: %d  %4d %4d   %4d %4d\n", project.num_crossings - 1, crossing->file_id_1,
              crossing->section_1, crossing->file_id_2, crossing->section_2);

      project.modelplot_uptodate = false;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_poornav_file() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to poor nav */
  if (project.open && project.num_files > 0) {
    /* try to identify affected survey block from selections */
    int block = MBNA_SELECT_NONE;

    if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && mbna_survey_select > MBNA_SELECT_NONE) {
      block = mbna_survey_select;
    }
    else if ((mbna_view_list == MBNA_VIEW_LIST_FILES || mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS)
        && mbna_file_select > MBNA_SELECT_NONE && mbna_file_select < project.num_files) {
      block = project.files[mbna_file_select].block;
    }

    /* set all files in block of selected file to poor nav */
    if (block > MBNA_SELECT_NONE) {
      sprintf(message, "Setting selected files to POOR nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_POORNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_POORNAV;

            /* add info text */
            sprintf(message, "Set file %d to have poor nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      sprintf(message, "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_goodnav_file() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to poor nav */
  if (project.open && project.num_files > 0) {
    /* try to identify affected survey block from selections */
    int block = MBNA_SELECT_NONE;

    if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && mbna_survey_select > MBNA_SELECT_NONE) {
      block = mbna_survey_select;
    }
    else if ((mbna_view_list == MBNA_VIEW_LIST_FILES || mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS)
        && mbna_file_select > MBNA_SELECT_NONE && mbna_file_select < project.num_files) {
      block = project.files[mbna_file_select].block;
    }

    /* set all files in block of selected file to good nav */
    if (block > MBNA_SELECT_NONE) {
      sprintf(message, "Setting selected files to GOOD nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_GOODNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_GOODNAV;

            /* add info text */
            sprintf(message, "Set file %d to have good nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      sprintf(message, "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixednav_file() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to fixed nav */
  if (project.open && project.num_files > 0) {
    /* try to identify affected survey block from selections */
    int block = MBNA_SELECT_NONE;

    if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && mbna_survey_select > MBNA_SELECT_NONE) {
      block = mbna_survey_select;
    }
    else if ((mbna_view_list == MBNA_VIEW_LIST_FILES || mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS)
        && mbna_file_select > MBNA_SELECT_NONE && mbna_file_select < project.num_files) {
      block = project.files[mbna_file_select].block;
    }

    /* set all files in block of selected file to fixed nav */
    if (block > MBNA_SELECT_NONE) {
      sprintf(message, "Setting selected files to FIXED nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDNAV;

            /* add info text */
            sprintf(message, "Set file %d to have fixed nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      sprintf(message, "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixedxynav_file() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to fixed nav */
  if (project.open && project.num_files > 0) {
    /* try to identify affected survey block from selections */
    int block = MBNA_SELECT_NONE;

    if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && mbna_survey_select > MBNA_SELECT_NONE) {
      block = mbna_survey_select;
    }
    else if ((mbna_view_list == MBNA_VIEW_LIST_FILES || mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS)
        && mbna_file_select > MBNA_SELECT_NONE && mbna_file_select < project.num_files) {
      block = project.files[mbna_file_select].block;
    }

    /* set all files in block of selected file to fixed xy nav */
    if (block > MBNA_SELECT_NONE) {
      sprintf(message, "Setting selected files to FIXED XY nav...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDXYNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDXYNAV;

            /* add info text */
            sprintf(message, "Set file %d to have fixed xy nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      sprintf(message, "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_fixedznav_file() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to fixed nav */
  if (project.open && project.num_files > 0) {
    /* try to identify affected survey block from selections */
    int block = MBNA_SELECT_NONE;

    if (mbna_view_list == MBNA_VIEW_LIST_SURVEYS && mbna_survey_select > MBNA_SELECT_NONE) {
      block = mbna_survey_select;
    }
    else if ((mbna_view_list == MBNA_VIEW_LIST_FILES || mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS)
        && mbna_file_select > MBNA_SELECT_NONE && mbna_file_select < project.num_files) {
      block = project.files[mbna_file_select].block;
    }

    /* set all files in block of selected file to fixed z nav */
    if (block > MBNA_SELECT_NONE) {
      sprintf(message, "Setting selected files to FIXED Z nav...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDZNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDZNAV;

            /* add info text */
            sprintf(message, "Set file %d to have fixed z nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      sprintf(message, "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_xyz() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to good nav */
  if (project.open && project.num_files > 0 && mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
    /* set tie to fix xyz */
    struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
    struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
    tie->status = MBNA_TIE_XYZ;
    fprintf(stderr, "Set crossing %d tie %d to fix XYZ\n", mbna_crossing_select, mbna_tie_select);
    if (project.inversion_status == MBNA_INVERSION_CURRENT)
      project.inversion_status = MBNA_INVERSION_OLD;

    /* write out updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* add info text */
    sprintf(message, "Set crossing %d tie %d to fix XYZ\n", mbna_crossing_select, mbna_tie_select);
    do_info_add(message, true);
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_xy() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* set selected file's block to good nav */
  if (project.open && project.num_files > 0 && mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
    /* set tie to fix xy */
    struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
    struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
    tie->status = MBNA_TIE_XY;
    fprintf(stderr, "Set crossing %d tie %d to fix XY\n", mbna_crossing_select, mbna_tie_select);
    if (project.inversion_status == MBNA_INVERSION_CURRENT)
      project.inversion_status = MBNA_INVERSION_OLD;

    /* write out updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* add info text */
    sprintf(message, "Set crossing %d tie %d to fix XY\n", mbna_crossing_select, mbna_tie_select);
    do_info_add(message, true);
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_set_tie_z() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  const int status = MB_SUCCESS;

  /* set selected file's block to good nav */
  if (project.open && project.num_files > 0 && mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
    /* set tie to fix z */
    struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
    struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
    tie->status = MBNA_TIE_Z;
    fprintf(stderr, "Set crossing %d tie %d to fix Z\n", mbna_crossing_select, mbna_tie_select);
    if (project.inversion_status == MBNA_INVERSION_CURRENT)
      project.inversion_status = MBNA_INVERSION_OLD;

    /* write out updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* add info text */
    sprintf(message, "Set crossing %d tie %d to fix Z\n", mbna_crossing_select, mbna_tie_select);
    do_info_add(message, true);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_save() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* save offsets if crossing loaded and ties set */
  if (project.open && project.num_crossings > 0 && mbna_naverr_load && mbna_current_crossing >= 0 &&
      mbna_current_tie >= 0) {
    /* save offsets if ties set */
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    if (crossing->num_ties > 0 && mbna_current_tie >= 0) {
      /* get relevant tie */
      struct mbna_tie *tie = &crossing->ties[mbna_current_tie];

      /* reset tie counts for snavs */
      struct mbna_file *file = &project.files[crossing->file_id_1];
      struct mbna_section *section = &file->sections[crossing->section_1];
      section->snav_num_ties[tie->snav_1]--;
      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      section->snav_num_ties[tie->snav_2]--;

      /* get new tie values */
      /* fprintf(stderr, "tie %d of crossing %d saved...\n", mbna_current_tie, mbna_current_crossing); */
      /* tie->status = tie->status; */
      tie->snav_1 = mbna_snav_1;
      tie->snav_1_time_d = mbna_snav_1_time_d;
      tie->snav_2 = mbna_snav_2;
      tie->snav_2_time_d = mbna_snav_2_time_d;
      if (tie->inversion_status == MBNA_INVERSION_CURRENT &&
          (tie->offset_x != mbna_offset_x || tie->offset_y != mbna_offset_y || tie->offset_z_m != mbna_offset_z)) {
        tie->inversion_status = MBNA_INVERSION_OLD;
                project.modelplot_uptodate = false;
      }
      tie->offset_x = mbna_offset_x;
      tie->offset_y = mbna_offset_y;
      tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      tie->offset_z_m = mbna_offset_z;
      tie->sigmar1 = mbna_minmisfit_sr1;
      tie->sigmar2 = mbna_minmisfit_sr2;
      tie->sigmar3 = mbna_minmisfit_sr3;
      for (int i = 0; i < 3; i++) {
        tie->sigmax1[i] = mbna_minmisfit_sx1[i];
        tie->sigmax2[i] = mbna_minmisfit_sx2[i];
        tie->sigmax3[i] = mbna_minmisfit_sx3[i];
      }
      if (tie->sigmar1 < MBNA_SMALL) {
        tie->sigmar1 = MBNA_SMALL;
        tie->sigmax1[0] = 1.0;
        tie->sigmax1[1] = 0.0;
        tie->sigmax1[2] = 0.0;
      }
      if (tie->sigmar2 < MBNA_SMALL) {
        tie->sigmar2 = MBNA_SMALL;
        tie->sigmax2[0] = 0.0;
        tie->sigmax2[1] = 1.0;
        tie->sigmax2[2] = 0.0;
      }
      if (tie->sigmar3 < MBNA_ZSMALL) {
        tie->sigmar3 = MBNA_ZSMALL;
        tie->sigmax3[0] = 0.0;
        tie->sigmax3[1] = 0.0;
        tie->sigmax3[2] = 1.0;
      }
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

            if (tie->inversion_status != MBNA_INVERSION_NONE) {
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

      /* reset tie counts for snavs */
      file = &project.files[crossing->file_id_1];
      section = &file->sections[crossing->section_1];
      section->snav_num_ties[tie->snav_1]++;
      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      section->snav_num_ties[tie->snav_2]++;

      /* write updated project */
      project.save_count++;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, &error);
        project.save_count = 0;
      }

      /* add info text */
      sprintf(message, "Save Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_tie, mbna_current_crossing, crossing->file_id_1, crossing->section_1, tie->snav_1,
              crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_specific(int new_crossing, int new_tie) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2               new_crossing: %d\n", new_crossing);
    fprintf(stderr, "dbg2               new_tie:      %d\n", new_tie);
  }

  /* get current crossing */
  if (project.open && project.num_crossings > 0) {
    /* get next crossing */
    if (new_crossing >= 0 && new_crossing < project.num_crossings) {
      mbna_current_crossing = new_crossing;
      if (new_tie >= 0 && new_tie < project.crossings[mbna_current_crossing].num_ties)
        mbna_current_tie = new_tie;
      else
        mbna_current_tie = -1;
    }
    else {
      mbna_current_crossing = 0;
      mbna_current_tie = -1;
    }

    /* retrieve crossing parameters */
    if (mbna_current_crossing >= 0) {
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      mbna_file_id_1 = crossing->file_id_1;
      mbna_section_1 = crossing->section_1;
      mbna_file_id_2 = crossing->file_id_2;
      mbna_section_2 = crossing->section_2;
      if (crossing->num_ties > 0) {
        if (mbna_current_tie < 0)
          mbna_current_tie = 0;
        struct mbna_tie *tie = &crossing->ties[mbna_current_tie];
        mbna_snav_1 = tie->snav_1;
        mbna_snav_1_time_d = tie->snav_1_time_d;
        mbna_snav_2 = tie->snav_2;
        mbna_snav_2_time_d = tie->snav_2_time_d;
        mbna_offset_x = tie->offset_x;
        mbna_offset_y = tie->offset_y;
        mbna_offset_z = tie->offset_z_m;
        /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
      }
      else {
        mbna_current_tie = -1;
      }

      /* reset survey file and section selections */
      if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
        if (mbna_survey_select == project.files[crossing->file_id_1].block) {
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }
        else if (mbna_survey_select == project.files[crossing->file_id_2].block) {
          mbna_file_select = crossing->file_id_2;
          mbna_section_select = crossing->section_2;
        }
        else {
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
        if (mbna_file_select == crossing->file_id_1) {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_section_select = crossing->section_1;
        }
        else if (mbna_file_select == crossing->file_id_2) {
          mbna_survey_select = project.files[crossing->file_id_2].block;
          mbna_section_select = crossing->section_2;
        }
        else {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_section_select = crossing->section_1;
        }
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
        if (mbna_file_select == crossing->file_id_1 && mbna_section_select == crossing->section_1) {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_file_select = crossing->file_id_1;
        }
        else if (mbna_file_select == crossing->file_id_2 && mbna_section_select == crossing->section_2) {
          mbna_survey_select = project.files[crossing->file_id_2].block;
          mbna_file_select = crossing->file_id_2;
        }
        else {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_file_select = crossing->file_id_1;
        }
      }
    }

    /* load the crossing */
    if (mbna_current_crossing >= 0) {
      /* put up message */
      sprintf(message, "Loading crossing %d...", mbna_current_crossing);
      do_message_on(message);

      mbnavadjust_crossing_load();

      /* turn off message */
      do_message_off();
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_next() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* find next current crossing */
  if (project.open && project.num_crossings > 0) {
    /* get next good crossing */
    int j = -1;
    int k = -1;
    for (int i = 0; i < project.num_crossings; i++) {
      if (do_check_crossing_listok(i) && i != mbna_current_crossing) {
        if (j < 0)
          j = i;
        if (k < 0 && i > mbna_current_crossing)
          k = i;
      }
    }
    if (k >= 0)
      mbna_current_crossing = k;
    else if (j >= 0)
      mbna_current_crossing = j;
    else
      mbna_current_crossing = -1;
    mbna_current_tie = -1;
  }

  /* retrieve crossing parameters */
  if (mbna_current_crossing >= 0) {
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    if (crossing->num_ties > 0) {
      if (mbna_current_tie == -1)
        mbna_current_tie = 0;
      struct mbna_tie *tie = &crossing->ties[0];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

      /* reset survey file and section selections */
      if (mbna_file_select == crossing->file_id_1) {
        mbna_section_select = crossing->section_1;
      }
      else if (mbna_file_select == crossing->file_id_2) {
        mbna_section_select = crossing->section_2;
      }
      else {
        mbna_file_select = crossing->file_id_1;
        mbna_survey_select = project.files[crossing->file_id_1].block;
        mbna_section_select = crossing->section_1;
      }
    }
    else {
      mbna_current_tie = -1;
    }
  }

  /* load the crossing */
  if (mbna_current_crossing >= 0) {
    /* put up message */
    sprintf(message, "Loading crossing %d...", mbna_current_crossing);
    do_message_on(message);

    mbnavadjust_crossing_load();

    /* turn off message */
    do_message_off();
  }
  // fprintf(stderr,"Done with mbnavadjust_naverr_next: mbna_current_crossing:%d mbna_crossing_select:%d mbna_current_tie:%d
  // mbna_tie_select:%d\n",  mbna_current_crossing,mbna_crossing_select,mbna_current_tie,mbna_tie_select);

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_previous() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* find previous current crossing */
  if (project.open && project.num_crossings > 0) {
    /* get next good crossing */
    int j = -1;
    int k = -1;
    for (int i = 0; i < project.num_crossings; i++) {
      if (do_check_crossing_listok(i) && i != mbna_current_crossing) {
        if (i < mbna_current_crossing)
          j = i;
        k = i;
      }
    }
    if (j >= 0)
      mbna_current_crossing = j;
    else if (k >= 0)
      mbna_current_crossing = k;
    else
      mbna_current_crossing = -1;
    mbna_current_tie = -1;
  }

  /* retrieve crossing parameters */
  if (mbna_current_crossing >= 0) {
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    if (crossing->num_ties > 0) {
      if (mbna_current_tie == -1)
        mbna_current_tie = 0;
      struct mbna_tie *tie = &crossing->ties[0];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

      /* reset survey file and section selections */
      if (mbna_file_select == crossing->file_id_1) {
        mbna_section_select = crossing->section_1;
      }
      else if (mbna_file_select == crossing->file_id_2) {
        mbna_section_select = crossing->section_2;
      }
      else {
        mbna_file_select = crossing->file_id_1;
        mbna_survey_select = project.files[crossing->file_id_1].block;
        mbna_section_select = crossing->section_1;
      }
    }
    else {
      mbna_current_tie = -1;
    }
  }

  /* load the crossing */
  if (mbna_current_crossing >= 0) {
    /* put up message */
    sprintf(message, "Loading crossing %d...", mbna_current_crossing);
    do_message_on(message);

    mbnavadjust_crossing_load();

    /* turn off message */
    do_message_off();
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_nextunset() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* find next current unset crossing */
  if (project.open && project.num_crossings > 0) {
    /* get next good crossing */
    int j = -1;
    int k = -1;
    for (int i = 0; i < project.num_crossings; i++) {
      if (do_check_crossing_listok(i) && i != mbna_current_crossing) {
        struct mbna_crossing *crossing = &(project.crossings[i]);
        if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
          if (j < 0)
            j = i;
          if (k < 0 && i > mbna_current_crossing)
            k = i;
        }
      }
    }
    if (k >= 0)
      mbna_current_crossing = k;
    else if (j >= 0)
      mbna_current_crossing = j;
    else
      mbna_current_crossing = -1;
    mbna_current_tie = -1;
  }

  /* retrieve crossing parameters */
  if (mbna_current_crossing >= 0) {
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    if (crossing->num_ties > 0) {
      mbna_current_tie = 0;
      struct mbna_tie *tie = &crossing->ties[0];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

      /* reset survey file and section selections */
      if (mbna_file_select == crossing->file_id_1) {
        mbna_section_select = crossing->section_1;
      }
      else if (mbna_file_select == crossing->file_id_2) {
        mbna_section_select = crossing->section_2;
      }
      else {
        mbna_file_select = crossing->file_id_1;
        mbna_survey_select = project.files[crossing->file_id_1].block;
        mbna_section_select = crossing->section_1;
      }
    }
    else {
      mbna_current_tie = -1;
    }
  }

  int status = MB_SUCCESS;

  /* load the crossing */
  if (mbna_current_crossing >= 0) {
    /* put up message */
    sprintf(message, "Loading crossing %d...", mbna_current_crossing);
    do_message_on(message);

    mbnavadjust_crossing_load();

    /* turn off message */
    do_message_off();
  }

  /* else unload previously loaded crossing */
  else if (mbna_naverr_load) {
    status = mbnavadjust_crossing_unload();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_selecttie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* get current crossing */
  if (project.open && project.num_crossings > 0) {
    /* retrieve crossing parameters */
    if (mbna_current_crossing >= 0 && project.crossings[mbna_current_crossing].num_ties > 0) {
      /* select next tie */
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      mbna_current_tie++;
      if (mbna_current_tie > crossing->num_ties - 1)
        mbna_current_tie = 0;
      struct mbna_tie *tie = &crossing->ties[mbna_current_tie];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      struct mbna_file *file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      struct mbna_file *file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      struct mbna_section *section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      struct mbna_section *section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      project.modelplot_uptodate = false;
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_addtie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* get current crossing */
  if (project.open && project.num_crossings > 0) {
    /* retrieve crossing parameters */
    if (mbna_current_crossing >= 0 && project.crossings[mbna_current_crossing].num_ties < MBNA_SNAV_NUM) {
      /* add tie and set number */
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      struct mbna_file *file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      struct mbna_file *file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      struct mbna_section *section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      struct mbna_section *section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      mbna_current_tie = crossing->num_ties;
      crossing->num_ties++;
      project.num_ties++;
      struct mbna_tie *tie = &crossing->ties[mbna_current_tie];

      if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
        project.num_crossings_analyzed++;
        if (crossing->truecrossing)
          project.num_truecrossings_analyzed++;
      }
      crossing->status = MBNA_CROSSING_STATUS_SET;

      /* look for unused pair of nav points */
      tie->snav_1 = -1;
      bool found = false;
      while (!found) {
        found = true;
        tie->snav_1++;
        for (int i = 0; i < crossing->num_ties - 1; i++) {
          if (crossing->ties[i].snav_1 == tie->snav_1)
            found = false;
        }
      }
      tie->snav_2 = -1;
      found = false;
      while (!found) {
        found = true;
        tie->snav_2++;
        for (int i = 0; i < crossing->num_ties - 1; i++) {
          if (crossing->ties[i].snav_2 == tie->snav_2)
            found = false;
        }
      }

      /* get rest of tie parameters */
      tie->status = MBNA_TIE_XYZ;
      tie->snav_1_time_d = section1->snav_time_d[tie->snav_1];
      tie->snav_2_time_d = section2->snav_time_d[tie->snav_2];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      tie->offset_x = mbna_offset_x;
      tie->offset_y = mbna_offset_y;
      tie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      tie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      tie->offset_z_m = mbna_offset_z;
      tie->sigmar1 = mbna_minmisfit_sr1;
      tie->sigmar2 = mbna_minmisfit_sr2;
      tie->sigmar3 = mbna_minmisfit_sr3;
      for (int i = 0; i < 3; i++) {
        tie->sigmax1[i] = mbna_minmisfit_sx1[i];
        tie->sigmax2[i] = mbna_minmisfit_sx2[i];
        tie->sigmax3[i] = mbna_minmisfit_sx3[i];
      }
      if (tie->sigmar1 < MBNA_SMALL) {
        tie->sigmar1 = MBNA_SMALL;
        tie->sigmax1[0] = 1.0;
        tie->sigmax1[1] = 0.0;
        tie->sigmax1[2] = 0.0;
      }
      if (tie->sigmar2 < MBNA_SMALL) {
        tie->sigmar2 = MBNA_SMALL;
        tie->sigmax2[0] = 0.0;
        tie->sigmax2[1] = 1.0;
        tie->sigmax2[2] = 0.0;
      }
      if (tie->sigmar3 < MBNA_ZSMALL) {
        tie->sigmar3 = MBNA_ZSMALL;
        tie->sigmax3[0] = 0.0;
        tie->sigmax3[1] = 0.0;
        tie->sigmax3[2] = 1.0;
      }

      file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      tie->inversion_status = MBNA_INVERSION_NONE;
      tie->inversion_offset_x = mbna_invert_offset_x;
      tie->inversion_offset_y = mbna_invert_offset_y;
      tie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
      tie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
      tie->inversion_offset_z_m = mbna_invert_offset_z;
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* now put tie in center of plot */
      const int ix = (int)(0.5 * (mbna_plot_lon_max - mbna_plot_lon_min) * mbna_plotx_scale);
      const int iy = (int)(cont_borders[3] - (0.5 * (mbna_plot_lat_max - mbna_plot_lat_min) * mbna_ploty_scale));
      mbnavadjust_naverr_snavpoints(ix, iy);
      tie->snav_1 = mbna_snav_1;
      tie->snav_2 = mbna_snav_2;
      tie->snav_1_time_d = mbna_snav_1_time_d;
      tie->snav_2_time_d = mbna_snav_2_time_d;

      /* reset tie counts for snavs */
      section1->snav_num_ties[tie->snav_1]++;
      section2->snav_num_ties[tie->snav_2]++;

      /* set flag to update model plot */
      project.modelplot_uptodate = false;

      /* write updated project */
      project.save_count++;
            project.modelplot_uptodate = false;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, &error);
        project.save_count = 0;
      }

      /* add info text */
      sprintf(message, "Add Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_tie, mbna_current_crossing, crossing->file_id_1, crossing->section_1, tie->snav_1,
              crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);

          if (mbna_verbose >= 2) {
        fprintf(stderr, "\ndbg2  snav point selected in MBnavadjust function <%s>\n", __func__);
        fprintf(stderr, "dbg2  snav values:\n");
        fprintf(stderr, "dbg2       mbna_snav_1:        %d\n", mbna_snav_1);
        fprintf(stderr, "dbg2       mbna_snav_1_time_d: %f\n", mbna_snav_1_time_d);
        fprintf(stderr, "dbg2       mbna_snav_1_lon:    %f\n", mbna_snav_1_lon);
        fprintf(stderr, "dbg2       mbna_snav_1_lat:    %f\n", mbna_snav_1_lat);
        fprintf(stderr, "dbg2       section1->num_snav:  %d\n", section1->num_snav);
        for (int i = 0; i < section1->num_snav; i++) {
          fprintf(stderr, "dbg2       section1->snav_time_d[%d]: %f\n", i, section1->snav_time_d[i]);
          fprintf(stderr, "dbg2       section1->snav_lon[%d]:    %.10f\n", i, section1->snav_lon[i]);
          fprintf(stderr, "dbg2       section1->snav_lat[%d]:    %.10f\n", i, section1->snav_lat[i]);
        }
        fprintf(stderr, "dbg2       mbna_snav_2:        %d\n", mbna_snav_2);
        fprintf(stderr, "dbg2       mbna_snav_2_time_d: %f\n", mbna_snav_2_time_d);
        fprintf(stderr, "dbg2       mbna_snav_2_lon:    %.10f\n", mbna_snav_2_lon);
        fprintf(stderr, "dbg2       mbna_snav_2_lat:    %.10f\n", mbna_snav_2_lat);
        fprintf(stderr, "dbg2       section2->num_snav:  %d\n", section2->num_snav);
        for (int i = 0; i < section2->num_snav; i++) {
          fprintf(stderr, "dbg2       section2->snav_time_d[%d]: %f\n", i, section2->snav_time_d[i]);
          fprintf(stderr, "dbg2       section2->snav_lon[%d]:    %.10f\n", i, section2->snav_lon[i]);
          fprintf(stderr, "dbg2       section2->snav_lat[%d]:    %.10f\n", i, section2->snav_lat[i]);
        }
      }
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_deletetie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* get current crossing */
  if (project.open && project.num_crossings > 0) {
    /* retrieve crossing parameters */
    if (mbna_current_crossing >= 0 && mbna_current_tie >= 0) {
      /* delete the tie */
      mbnavadjust_deletetie(mbna_current_crossing, mbna_current_tie, MBNA_CROSSING_STATUS_SKIP);

      /* write updated project */
      project.save_count++;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, &error);
        project.save_count = 0;
      }
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_deletetie(int icrossing, int jtie, int delete_status) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       icrossing:     %d\n", icrossing);
    fprintf(stderr, "dbg2       jtie:          %d\n", jtie);
    fprintf(stderr, "dbg2       delete_status: %d\n", delete_status);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_section *section1, *section2;
  struct mbna_file *file1, *file2;

  /* get current crossing */
  if (project.open && icrossing >= 0 && jtie >= 0) {
    /* retrieve crossing parameters */
    if (project.num_crossings > icrossing && project.crossings[icrossing].num_ties > jtie) {
      /* add info text */
      crossing = &project.crossings[icrossing];
      tie = &crossing->ties[jtie];
      if (delete_status == MBNA_CROSSING_STATUS_SKIP)
        sprintf(message, "Delete Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
                jtie, icrossing, crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2,
                crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      else
        sprintf(message, "Clear Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
                jtie, icrossing, crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2,
                crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);

      /* reset tie counts for snavs */
      file1 = &project.files[crossing->file_id_1];
      section1 = &file1->sections[crossing->section_1];
      section1->snav_num_ties[tie->snav_1]--;
      file2 = &project.files[crossing->file_id_2];
      section2 = &file2->sections[crossing->section_2];
      section2->snav_num_ties[tie->snav_2]--;

      /* delete tie and set number */
      for (int i = mbna_current_tie; i < crossing->num_ties - 1; i++) {
        crossing->ties[i].status = crossing->ties[i + 1].status;
        crossing->ties[i].snav_1 = crossing->ties[i + 1].snav_1;
        crossing->ties[i].snav_1_time_d = crossing->ties[i + 1].snav_1_time_d;
        crossing->ties[i].snav_2 = crossing->ties[i + 1].snav_2;
        crossing->ties[i].snav_2_time_d = crossing->ties[i + 1].snav_2_time_d;
        crossing->ties[i].offset_x = crossing->ties[i + 1].offset_x;
        crossing->ties[i].offset_y = crossing->ties[i + 1].offset_y;
        crossing->ties[i].offset_x_m = crossing->ties[i + 1].offset_x_m;
        crossing->ties[i].offset_y_m = crossing->ties[i + 1].offset_y_m;
        crossing->ties[i].offset_z_m = crossing->ties[i + 1].offset_z_m;
      }
      crossing->num_ties--;
      project.num_ties--;
      if (mbna_current_tie > crossing->num_ties - 1)
        mbna_current_tie--;

      /* set tie parameters */
      if (crossing->num_ties <= 0) {
        crossing->num_ties = 0;
        crossing->status = delete_status;
      }
      else if (mbna_current_tie >= 0) {
        tie = &crossing->ties[mbna_current_tie];
        mbna_snav_1 = tie->snav_1;
        mbna_snav_1_time_d = tie->snav_1_time_d;
        mbna_snav_2 = tie->snav_2;
        mbna_snav_2_time_d = tie->snav_2_time_d;
        mbna_offset_x = tie->offset_x;
        mbna_offset_y = tie->offset_y;
        mbna_offset_z = tie->offset_z_m;
        /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
        file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
        file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
        section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
        section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
        mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
        mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
        mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      }
      if (project.inversion_status == MBNA_INVERSION_CURRENT) {
        project.inversion_status = MBNA_INVERSION_OLD;
      }
            project.modelplot_uptodate = false;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_resettie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* get current crossing */
  if (project.open && project.num_crossings > 0) {
    /* retrieve crossing parameters */
    if (mbna_current_crossing >= 0 && mbna_current_tie >= 0) {
      /* reset tie */
      file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
      file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
      section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
      section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
      crossing = &project.crossings[mbna_current_crossing];
      tie = &crossing->ties[mbna_current_tie];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */
      mbna_minmisfit_sr1 = tie->sigmar1;
      mbna_minmisfit_sr2 = tie->sigmar2;
      mbna_minmisfit_sr3 = tie->sigmar3;
      for (int i = 0; i < 3; i++) {
        mbna_minmisfit_sx1[i] = tie->sigmax1[i];
        mbna_minmisfit_sx2[i] = tie->sigmax2[i];
        mbna_minmisfit_sx3[i] = tie->sigmax3[i];
      }
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_checkoksettie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* check for changed offsets */
  mbna_allow_set_tie = false;
  if (mbna_current_crossing >= 0 && mbna_current_tie >= 0) {
    crossing = &project.crossings[mbna_current_crossing];
    tie = &crossing->ties[mbna_current_tie];
    if (tie->snav_1 != mbna_snav_1 || tie->snav_2 != mbna_snav_2 || tie->offset_x != mbna_offset_x ||
        tie->offset_y != mbna_offset_y || tie->offset_z_m != mbna_offset_z) {
      mbna_allow_set_tie = true;
    }

    /* also check for unset sigma values */
    if (tie->sigmar1 == 100.0 && tie->sigmar2 == 100.0 && tie->sigmar3 == 100.0) {
      mbna_allow_set_tie = true;
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_skip() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;

  /* get current crossing */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    crossing = &project.crossings[mbna_current_crossing];
    if (crossing->status != MBNA_CROSSING_STATUS_SKIP) {
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        project.num_ties -= crossing->num_ties;
        crossing->num_ties = 0;
        if (project.inversion_status == MBNA_INVERSION_CURRENT) {
          project.inversion_status = MBNA_INVERSION_OLD;
                    project.modelplot_uptodate = false;
        }
      }
      else if (crossing->status == MBNA_CROSSING_STATUS_NONE) {
        project.num_crossings_analyzed++;
        if (crossing->truecrossing)
          project.num_truecrossings_analyzed++;
      }
      crossing->status = MBNA_CROSSING_STATUS_SKIP;
      mbna_current_tie = MBNA_SELECT_NONE;

      /* write updated project */
      project.save_count++;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, &error);
        project.save_count = 0;
      }

      /* add info text */
      sprintf(message, "Set crossing %d to be ignored\n", mbna_current_crossing);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_unset() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;

  /* get current crossing */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    crossing = &project.crossings[mbna_current_crossing];
    if (crossing->status != MBNA_CROSSING_STATUS_NONE) {
      /* delete any ties */
      if (crossing->num_ties > 0) {
        project.num_ties -= crossing->num_ties;
        crossing->num_ties = 0;
        if (project.inversion_status == MBNA_INVERSION_CURRENT) {
          project.inversion_status = MBNA_INVERSION_OLD;
                    project.modelplot_uptodate = false;
        }
        mbna_current_tie = MBNA_SELECT_NONE;
      }

      /* set crossing unanalyzed */
      project.num_crossings_analyzed--;
      if (crossing->truecrossing)
        project.num_truecrossings_analyzed--;
      crossing->status = MBNA_CROSSING_STATUS_NONE;

      /* write updated project */
      project.save_count++;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, &error);
        project.save_count = 0;
      }

            /* set flag to update model plot */
            project.modelplot_uptodate = false;

      /* add info text */
      sprintf(message, "Unset crossing %d\n", mbna_current_crossing);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);
    }
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_load() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;

  /* unload loaded crossing */
  if (mbna_naverr_load) {
    status = mbnavadjust_crossing_unload();
  }

  /* load current crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && project.open &&
      project.num_crossings > 0 && mbna_current_crossing >= 0) {
    /* put up message */
    sprintf(message, "Loading crossing %d...", mbna_current_crossing);
    do_message_update(message);

    /* retrieve crossing parameters */
    crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    file1 = (struct mbna_file *)&project.files[mbna_file_id_1];
    file2 = (struct mbna_file *)&project.files[mbna_file_id_2];
    section1 = (struct mbna_section *)&file1->sections[mbna_section_1];
    section2 = (struct mbna_section *)&file2->sections[mbna_section_2];
    if (crossing->num_ties > 0 && mbna_current_tie >= 0) {
      /* get basic crossing parameters */
      tie = &crossing->ties[mbna_current_tie];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
    }
    else if (project.inversion_status != MBNA_INVERSION_NONE) {
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = section1->snav_time_d[mbna_snav_1];
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2] - section1->snav_lon_offset[mbna_snav_1];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2] - section1->snav_lat_offset[mbna_snav_1];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2] - section1->snav_z_offset[mbna_snav_1];
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    }
    else {
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = section1->snav_time_d[mbna_snav_1];
      mbna_snav_1_lon = section1->snav_lon[mbna_snav_1];
      mbna_snav_1_lat = section1->snav_lat[mbna_snav_1];
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_snav_2_lon = section2->snav_lon[mbna_snav_2];
      mbna_snav_2_lat = section2->snav_lat[mbna_snav_2];
      mbna_invert_offset_x = 0.0;
      mbna_invert_offset_y = 0.0;
      mbna_invert_offset_z = 0.0;
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    }
    mbna_lon_min = MIN(section1->lonmin, section2->lonmin + mbna_offset_x);
    mbna_lon_max = MAX(section1->lonmax, section2->lonmax + mbna_offset_x);
    mbna_lat_min = MIN(section1->latmin, section2->latmin + mbna_offset_y);
    mbna_lat_max = MAX(section1->latmax, section2->latmax + mbna_offset_y);
    mbna_plot_lon_min = mbna_lon_min;
    mbna_plot_lon_max = mbna_lon_max;
    mbna_plot_lat_min = mbna_lat_min;
    mbna_plot_lat_max = mbna_lat_max;
    mb_coor_scale(mbna_verbose, 0.5 * (mbna_lat_min + mbna_lat_max), &mbna_mtodeglon, &mbna_mtodeglat);

    /* load sections */
    sprintf(message, "Loading section 1 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_1, mbna_section_1,
                                          (void **)&swathraw1, (void **)&swath1, section1->num_pings, &error);
    sprintf(message, "Loading section 2 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_2, mbna_section_2,
                                          (void **)&swathraw2, (void **)&swath2, section2->num_pings, &error);

    /* get lon lat positions for soundings */
    sprintf(message, "Transforming section 1 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    sprintf(message, "Transforming section 2 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      sprintf(message, "Contouring section 1 of crossing %d...", mbna_current_crossing);
      do_message_update(message);
            mbna_contour = &mbna_contour1;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      sprintf(message, "Contouring section 2 of crossing %d...", mbna_current_crossing);
      do_message_update(message);
            mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
    }

    /* set loaded flag */
    mbna_naverr_load = true;

    /* generate misfit grids */
    sprintf(message, "Getting misfit for crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_get_misfit();

    /* get overlap region */
    mbnavadjust_crossing_overlap(mbna_verbose, &project, mbna_current_crossing, &error);

        /* set flag to update model plot */
        project.modelplot_uptodate = false;
  }

  /* set mbna_crossing_select */
  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0) {
    mbna_crossing_select = mbna_current_crossing;
    if (mbna_current_tie >= 0)
      mbna_tie_select = mbna_current_tie;
    else
      mbna_tie_select = MBNA_SELECT_NONE;
  }
  else {
    mbna_crossing_select = MBNA_SELECT_NONE;
    mbna_tie_select = MBNA_SELECT_NONE;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_unload() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* unload loaded crossing */
  if (mbna_naverr_load) {
    status = mbnavadjust_section_unload(mbna_verbose, (void **)&swathraw1, (void **)&swath1, &error);
    status = mbnavadjust_section_unload(mbna_verbose, (void **)&swathraw2, (void **)&swath2, &error);
    if (mbna_contour1.vector != NULL && mbna_contour1.nvector_alloc > 0) {
      free(mbna_contour1.vector);
    }
    if (mbna_contour2.vector != NULL && mbna_contour2.nvector_alloc > 0) {
      free(mbna_contour2.vector);
    }
    mbna_contour1.vector = NULL;
    mbna_contour1.nvector = 0;
    mbna_contour1.nvector_alloc = 0;
    mbna_contour2.vector = NULL;
    mbna_contour2.nvector = 0;
    mbna_contour2.nvector_alloc = 0;
    mbna_naverr_load = false;
    grid_nx = 0;
    grid_ny = 0;
    grid_nxy = 0;
    grid_nxyzeq = 0;
    gridm_nx = 0;
    gridm_ny = 0;
    gridm_nxyz = 0;
    if (grid1 != NULL) {
      free(grid1);
    }
    if (grid2 != NULL) {
      free(grid2);
    }
    if (gridm != NULL) {
      free(gridm);
    }
    if (gridmeq != NULL) {
      free(gridmeq);
    }
    if (gridn1 != NULL) {
      free(gridn1);
    }
    if (gridn2 != NULL) {
      free(gridn2);
    }
    if (gridnm != NULL) {
      free(gridnm);
    }
    grid1 = NULL;
    grid2 = NULL;
    gridm = NULL;
    gridmeq = NULL;
    gridn1 = NULL;
    gridn2 = NULL;
    gridnm = NULL;

        /* set flag to update model plot */
        project.modelplot_uptodate = false;
  }

  // fprintf(stderr,"\nend %s: mbna_current_crossing:%d mbna_current_tie:%d\n",
  // __func__,mbna_current_crossing,mbna_current_tie);

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_crossing_replot() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* replot loaded crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && mbna_naverr_load) {
    /* get lon lat positions for soundings */
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    mbna_contour = &mbna_contour1;
    status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
    mbna_contour = &mbna_contour2;
    status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_snavpoints(int ix, int iy) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ix:           %d\n", ix);
    fprintf(stderr, "dbg2       iy:           %d\n", iy);
  }

  int status = MB_SUCCESS;
  double x, y, dx, dy, d;
  struct mbna_crossing *crossing;
  struct mbna_section *section;
  double distance;

  if (mbna_naverr_load) {
    /* get position in lon and lat */
    x = ix / mbna_plotx_scale + mbna_plot_lon_min;
    y = (cont_borders[3] - iy) / mbna_ploty_scale + mbna_plot_lat_min;
    crossing = &project.crossings[mbna_current_crossing];

    /* get closest snav point in swath 1 */
    section = &project.files[crossing->file_id_1].sections[crossing->section_1];
    distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      dx = (section->snav_lon[i] - x) / mbna_mtodeglon;
      dy = (section->snav_lat[i] - y) / mbna_mtodeglat;
      d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_1 = i;
        mbna_snav_1_time_d = section->snav_time_d[i];
        mbna_snav_1_lon = section->snav_lon[i];
        mbna_snav_1_lat = section->snav_lat[i];
      }
    }

    /* get closest snav point in swath 2 */
    section = &project.files[crossing->file_id_2].sections[crossing->section_2];
    distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
      dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
      d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_2 = i;
        mbna_snav_2_time_d = section->snav_time_d[i];
        mbna_snav_2_lon = section->snav_lon[i];
        mbna_snav_2_lat = section->snav_lat[i];
      }
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  snav point selection in MBnavadjust function <%s>\n", __func__);
    fprintf(stderr, "dbg2  mbna_naverr_load:        %d\n", mbna_naverr_load);
    fprintf(stderr, "dbg2  mbna_current_crossing:   %d\n", mbna_current_crossing);
    if (mbna_naverr_load) {
      fprintf(stderr, "dbg2  snav values:\n");
      section = &project.files[crossing->file_id_1].sections[crossing->section_1];
      fprintf(stderr, "dbg2       mbna_snav_1:        %d\n", mbna_snav_1);
      fprintf(stderr, "dbg2       mbna_snav_1_time_d: %f\n", mbna_snav_1_time_d);
      fprintf(stderr, "dbg2       mbna_snav_1_lon:    %.10f\n", mbna_snav_1_lon);
      fprintf(stderr, "dbg2       mbna_snav_1_lat:    %.10f\n", mbna_snav_1_lat);
      fprintf(stderr, "dbg2       section->num_snav:  %d\n", section->num_snav);
      for (int i = 0; i < section->num_snav; i++) {
        fprintf(stderr, "dbg2       section1->snav_time_d[%d]: %f\n", i, section->snav_time_d[i]);
        fprintf(stderr, "dbg2       section1->snav_lon[%d]:    %.10f\n", i, section->snav_lon[i]);
        fprintf(stderr, "dbg2       section1->snav_lat[%d]:    %.10f\n", i, section->snav_lat[i]);
      }
      section = &project.files[crossing->file_id_2].sections[crossing->section_2];
      fprintf(stderr, "dbg2       mbna_snav_2:        %d\n", mbna_snav_2);
      fprintf(stderr, "dbg2       mbna_snav_2_time_d: %f\n", mbna_snav_2_time_d);
      fprintf(stderr, "dbg2       mbna_snav_2_lon:    %.10f\n", mbna_snav_2_lon);
      fprintf(stderr, "dbg2       mbna_snav_2_lat:    %.10f\n", mbna_snav_2_lat);
      fprintf(stderr, "dbg2       section->num_snav:  %d\n", section->num_snav);
      for (int i = 0; i < section->num_snav; i++) {
        fprintf(stderr, "dbg2       section2->snav_time_d[%d]: %f\n", i, section->snav_time_d[i]);
        fprintf(stderr, "dbg2       section2->snav_lon[%d]:    %.10f\n", i, section->snav_lon[i]);
        fprintf(stderr, "dbg2       section2->snav_lat[%d]:    %.10f\n", i, section->snav_lat[i]);
      }
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
bool mbnavadjust_sections_intersect(int crossing_id) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       crossing_id:  %d\n", crossing_id);
  }

  /* get crossing */
  struct mbna_crossing *crossing = (struct mbna_crossing *)&project.crossings[crossing_id];

  /* get section endpoints */
  struct mbna_file *file = &project.files[crossing->file_id_1];
  struct mbna_section *section = &file->sections[crossing->section_1];
  const double xa1 = section->snav_lon[0] + section->snav_lon_offset[0];
  const double ya1 = section->snav_lat[0] + section->snav_lat_offset[0];
  const double xa2 = section->snav_lon[section->num_snav - 1] + section->snav_lon_offset[section->num_snav - 1];
  const double ya2 = section->snav_lat[section->num_snav - 1] + section->snav_lat_offset[section->num_snav - 1];
  file = &project.files[crossing->file_id_2];
  section = &file->sections[crossing->section_2];
  const double xb1 = section->snav_lon[0] + section->snav_lon_offset[0];
  const double yb1 = section->snav_lat[0] + section->snav_lat_offset[0];
  const double xb2 = section->snav_lon[section->num_snav - 1] + section->snav_lon_offset[section->num_snav - 1];
  const double yb2 = section->snav_lat[section->num_snav - 1] + section->snav_lat_offset[section->num_snav - 1];

  /* check for parallel sections */
  const double dxa = xa2 - xa1;
  const double dya = ya2 - ya1;
  const double dxb = xb2 - xb1;
  const double dyb = yb2 - yb1;
  bool answer = false;
  if ((dxb * dya - dyb * dxa) != 0.0) {
    /* check for crossing sections */
    const double s = (dxa * (yb1 - ya1) + dya * (xa1 - xb1)) / (dxb * dya - dyb * dxa);
    const double t = (dxb * (ya1 - yb1) + dyb * (xb1 - xa1)) / (dyb * dxa - dxb * dya);
    answer = s >= 0.0 && s <= 1.0 && t >= 0.0 && t <= 1.0;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       answer:      %d\n", answer);
  }

  return (answer);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfit() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  double dinterval;
  double zoff;
  double minmisfitthreshold, dotproduct;
  double x, y, z, r;
  double dotproductsave2;
  double rsave2;
  double dotproductsave3;
  double rsave3;
  bool found;
  int igx, igy;
  int lc;
  int ioff, joff, istart, iend, jstart, jend;
  int i2, j2, k1, k2;
  int k, ll;
  void *tptr;

  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0 && mbna_naverr_load) {
    /* fprintf(stderr,"\nDEBUG %s %d: mbnavadjust_get_misfit: mbna_plot minmax: %f %f %f %f\n",
    __FILE__,__LINE__,
    mbna_plot_lon_min,mbna_plot_lon_max,mbna_plot_lat_min,mbna_plot_lat_max); */

    /* set message on */
    if (mbna_verbose > 1)
      fprintf(stderr, "Making misfit grid for crossing %d\n", mbna_current_crossing);
    sprintf(message, "Making misfit grid for crossing %d\n", mbna_current_crossing);
    do_message_update(message);

    /* reset sounding density threshold for misfit calculation
        - will be tuned down if necessary */
    mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;

    /* figure out lateral extent of grids */
    grid_nx = MBNA_MISFIT_DIMXY;
    grid_ny = MBNA_MISFIT_DIMXY;
    if ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon > (mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat) {
      grid_dx = (mbna_plot_lon_max - mbna_plot_lon_min) / (grid_nx - 1);
      grid_dy = grid_dx * mbna_mtodeglat / mbna_mtodeglon;
      /* fprintf(stderr,"DEBUG %s %d: grid scale: grid_dx:%f grid_dy:%f\n",
      __FILE__,__LINE__,
      grid_dx,grid_dy); */
    }
    else {
      grid_dy = (mbna_plot_lat_max - mbna_plot_lat_min) / (grid_ny - 1);
      grid_dx = grid_dy * mbna_mtodeglon / mbna_mtodeglat;
      /* fprintf(stderr,"DEBUG %s %d: grid scale: grid_dx:%f grid_dy:%f\n",
      __FILE__,__LINE__,
      grid_dx,grid_dy); */
    }
    grid_nxy = grid_nx * grid_ny;
    grid_olon = 0.5 * (mbna_plot_lon_min + mbna_plot_lon_max) - (grid_nx / 2 + 0.5) * grid_dx;
    grid_olat = 0.5 * (mbna_plot_lat_min + mbna_plot_lat_max) - (grid_ny / 2 + 0.5) * grid_dy;
    /* fprintf(stderr,"DEBUG %s %d: grid_olon:%.10f grid_olat:%.10f\n",
    __FILE__,__LINE__,
    grid_olon,grid_olat); */

    /* get 3d misfit grid */
    nzmisfitcalc = MBNA_MISFIT_DIMZ;
    gridm_nx = grid_nx / 2 + 1;
    gridm_ny = gridm_nx;
    gridm_nxyz = gridm_nx * gridm_ny * nzmisfitcalc;
    if (mbna_misfit_center == MBNA_MISFIT_ZEROCENTER) {
      mbna_misfit_offset_x = 0.0;
      mbna_misfit_offset_y = 0.0;
      mbna_misfit_offset_z = 0.0;
    }
    else {
      mbna_misfit_offset_x = mbna_offset_x;
      mbna_misfit_offset_y = mbna_offset_y;
      mbna_misfit_offset_z = mbna_offset_z;
    }
    /* fprintf(stderr,"DEBUG %s %d: GRID parameters: dx:%.10f dy:%.10f nx:%d ny:%d  bounds:  grid: %.10f %.10f %.10f %.10f
    plot: %.10f %.10f %.10f %.10f\n",
    __FILE__,__LINE__,
    grid_dx, grid_dy, grid_nx, grid_ny,
    grid_olon, grid_olon + grid_nx * grid_dx,
    grid_olat, grid_olat + grid_ny * grid_dy,
    mbna_lon_min, mbna_lon_max, mbna_lat_min, mbna_lat_max); */

    /* figure out range of z offsets */
    zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
    zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
    zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);
    /* fprintf(stderr,"DEBUG %s %d: mbna_misfit_offset_z:%f project.zoffsetwidth:%f nzmisfitcalc:%d zmin:%f zmax:%f
    zoff_dz:%f\n",
    __FILE__,__LINE__,
    mbna_misfit_offset_z,project.zoffsetwidth,nzmisfitcalc,zmin,zmax,zoff_dz); */

    /* allocate and initialize grids and arrays */
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(grid1, sizeof(double) * (grid_nxy));
      if (tptr != NULL) {
        grid1 = tptr;
        memset(grid1, 0, sizeof(double) * (grid_nxy));
      }
      else {
        free(grid1);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(grid2, sizeof(double) * (grid_nxy));
      if (tptr != NULL) {
        grid2 = tptr;
        memset(grid2, 0, sizeof(double) * (grid_nxy));
      }
      else {
        free(grid2);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(gridm, sizeof(double) * (gridm_nxyz));
      if (tptr != NULL) {
        gridm = tptr;
        memset(gridm, 0, sizeof(double) * (gridm_nxyz));
      }
      else {
        free(gridm);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (double *)realloc(gridmeq, sizeof(double) * (gridm_nxyz));
      if (tptr != NULL) {
        gridmeq = tptr;
        memset(gridmeq, 0, sizeof(double) * (gridm_nxyz));
      }
      else {
        free(gridmeq);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridn1, sizeof(int) * (grid_nxy));
      if (tptr != NULL) {
        gridn1 = tptr;
        memset(gridn1, 0, sizeof(int) * (grid_nxy));
      }
      else {
        free(gridn1);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridn2, sizeof(int) * (grid_nxy));
      if (tptr != NULL) {
        gridn2 = tptr;
        memset(gridn2, 0, sizeof(int) * (grid_nxy));
      }
      else {
        free(gridn2);
        status = MB_FAILURE;
      }
    }
    if (status == MB_SUCCESS) {
      tptr = (int *)realloc(gridnm, sizeof(int) * (gridm_nxyz));
      if (tptr != NULL) {
        gridnm = tptr;
        memset(gridnm, 0, sizeof(int) * (gridm_nxyz));
      }
      else {
        free(gridnm);
        status = MB_FAILURE;
      }
    }

    /* loop over all beams */
    for (int i = 0; i < swath1->npings; i++) {
      for (int j = 0; j < swath1->pings[i].beams_bath; j++) {
        if (mb_beam_ok(swath1->pings[i].beamflag[j])) {
          x = (swath1->pings[i].bathlon[j] - grid_olon);
          y = (swath1->pings[i].bathlat[j] - grid_olat);
          igx = (int)(x / grid_dx);
          igy = (int)(y / grid_dy);
          k = igx + igy * grid_nx;
          if (igx >= 0 && igx < grid_nx && igy >= 0 && igy < grid_ny) {
            grid1[k] += swath1->pings[i].bath[j];
            gridn1[k]++;
          }
          /* else
          fprintf(stderr,"DEBUG %s %d: BAD swath1: %d %d  %.10f %.10f  %f %f  %d %d\n",
          __FILE__,__LINE__,
          i, j, swath1->pings[i].bathlon[j], swath1->pings[i].bathlat[j], x, y, igx, igy); */
        }
      }
    }

    /* loop over all beams */
    for (int i = 0; i < swath2->npings; i++) {
      for (int j = 0; j < swath2->pings[i].beams_bath; j++) {
        if (mb_beam_ok(swath2->pings[i].beamflag[j])) {
          x = (swath2->pings[i].bathlon[j] + mbna_misfit_offset_x - grid_olon);
          y = (swath2->pings[i].bathlat[j] + mbna_misfit_offset_y - grid_olat);
          igx = (int)(x / grid_dx);
          igy = (int)(y / grid_dy);
          k = igx + igy * grid_nx;
          if (igx >= 0 && igx < grid_nx && igy >= 0 && igy < grid_ny) {
            grid2[k] += swath2->pings[i].bath[j];
            gridn2[k]++;
          }
          /* else
          fprintf(stderr,"DEBUG %s %d: BAD swath2: %d %d  %.10f %.10f  %f %f  %d %d\n",
          __FILE__,__LINE__,
          i, j, swath2->pings[i].bathlon[j], swath2->pings[i].bathlat[j], x, y, igx, igy); */
        }
      }
    }

    /* calculate gridded bath */
    for (int k = 0; k < grid_nxy; k++) {
      if (gridn1[k] > 0) {
        grid1[k] = (grid1[k] / gridn1[k]);
      }
      if (gridn2[k] > 0) {
        grid2[k] = (grid2[k] / gridn2[k]);
      }
      /* fprintf(stderr,"GRIDDED BATH: k:%d 1:%d %f   2:%d %f\n",
      k,gridn1[k],grid1[k],gridn2[k],grid2[k]); */
    }

    /* calculate gridded misfit over lateral and z offsets */
    for (int ic = 0; ic < gridm_nx; ic++)
      for (int jc = 0; jc < gridm_ny; jc++)
        for (int kc = 0; kc < nzmisfitcalc; kc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          gridm[lc] = 0.0;
          gridnm[lc] = 0;

          ioff = (gridm_nx / 2) - ic;
          joff = (gridm_ny / 2) - jc;
          zoff = zmin + zoff_dz * kc;

          istart = MAX(-ioff, 0);
          iend = grid_nx - MAX(0, ioff);
          jstart = MAX(-joff, 0);
          jend = grid_ny - MAX(0, joff);
          for (int i1 = istart; i1 < iend; i1++)
            for (int j1 = jstart; j1 < jend; j1++) {
              i2 = i1 + ioff;
              j2 = j1 + joff;
              k1 = i1 + j1 * grid_nx;
              k2 = i2 + j2 * grid_nx;
              if (gridn1[k1] > 0 && gridn2[k2] > 0) {
                gridm[lc] += (grid2[k2] - grid1[k1] + zoff - mbna_offset_z) *
                             (grid2[k2] - grid1[k1] + zoff - mbna_offset_z);
                gridnm[lc]++;
              }
            }
        }
    misfit_min = 0.0;
    misfit_max = 0.0;
    mbna_minmisfit = 0.0;
    mbna_minmisfit_n = 0;
    mbna_minmisfit_x = 0.0;
    mbna_minmisfit_y = 0.0;
    mbna_minmisfit_z = 0.0;
    found = false;
    for (int ic = 0; ic < gridm_nx; ic++)
      for (int jc = 0; jc < gridm_ny; jc++)
        for (int kc = 0; kc < nzmisfitcalc; kc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          if (gridnm[lc] > 0) {
            gridm[lc] = sqrt(gridm[lc]) / gridnm[lc];
            if (misfit_max == 0.0) {
              misfit_min = gridm[lc];
            }
            misfit_min = MIN(misfit_min, gridm[lc]);
            misfit_max = MAX(misfit_max, gridm[lc]);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit)) {
              mbna_minmisfit = gridm[lc];
              mbna_minmisfit_n = gridnm[lc];
              mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_z = zmin + zoff_dz * kc;
              // const int imin = ic;
              // const int jmin = jc;
              // const int kmin = kc;
              found = true;
              /* zoff = zmin + zoff_dz * kc;
              fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d kc:%d misfit:%f %f %d  pos:%f %f %f zoff:%f
              mbna_ofset_z:%f\n",
              __FILE__,__LINE__,
              ic,jc,kc,misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z,
              zoff,mbna_offset_z); */
            }
          }
          /* if (ic == jc && kc == 0)
          fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d misfit:%d %f\n",
          __FILE__,__LINE__,
          ic,jc,gridnm[lc],gridm[lc]);*/
        }
    if (!found) {
      mbna_minmisfit_nthreshold /= 10.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold / 10 &&
                (mbna_minmisfit_n == 0 || gridm[lc] < mbna_minmisfit)) {
              mbna_minmisfit = gridm[lc];
              mbna_minmisfit_n = gridnm[lc];
              mbna_minmisfit_x = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_y = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_z = zmin + zoff_dz * kc;
              // imin = ic;
              // jmin = jc;
              // kmin = kc;
              found = true;
            }
            /* fprintf(stderr,"DEBUG %s %d: ijk:%d %d %d gridm:%d %f  misfit:%f %f %d  pos:%f %f %f\n",
            __FILE__,__LINE__,
            ic,jc,kc,gridnm[lc],gridm[lc],misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z);
            */
          }
    }
    misfit_min = 0.99 * misfit_min;
    misfit_max = 1.01 * misfit_max;
    /* if (found)
    {
    lc = kmin + nzmisfitcalc * (imin + jmin * gridm_nx);
    fprintf(stderr,"DEBUG %s %d: min misfit: i:%d j:%d k:%d    n:%d m:%f   offsets: %f %f %f\n",
    __FILE__,__LINE__,
    imin, jmin, kmin, gridnm[lc], gridm[lc],
    mbna_minmisfit_x / mbna_mtodeglon,
    mbna_minmisfit_y / mbna_mtodeglat,
    mbna_minmisfit_z);
    } */

    /* fprintf(stderr,"DEBUG %s %d: Misfit bounds: nmin:%d best:%f min:%f max:%f min loc: %f %f %f\n",
    __FILE__,__LINE__,
    mbna_minmisfit_n,mbna_minmisfit,misfit_min,misfit_max,
    mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z); */

    /* set message on */
    if (mbna_verbose > 1)
      fprintf(stderr, "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
    sprintf(message, "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
    do_message_update(message);

    /* sort the misfit to get histogram equalization */
    grid_nxyzeq = 0;
    for (int l = 0; l < gridm_nxyz; l++) {
      if (gridm[l] > 0.0) {
        gridmeq[grid_nxyzeq] = gridm[l];
        grid_nxyzeq++;
      }
    }

    if (grid_nxyzeq > 0) {
      qsort((char *)gridmeq, grid_nxyzeq, sizeof(double), mb_double_compare);
      dinterval = ((double)grid_nxyzeq) / ((double)(nmisfit_intervals - 1));
      if (dinterval < 1.0) {
        for (int l = 0; l < grid_nxyzeq; l++)
          misfit_intervals[l] = gridmeq[l];
        for (int l = grid_nxyzeq; l < nmisfit_intervals; l++)
          misfit_intervals[l] = gridmeq[grid_nxyzeq - 1];
      }
      else {
        misfit_intervals[0] = misfit_min;
        misfit_intervals[nmisfit_intervals - 1] = misfit_max;
        for (int l = 1; l < nmisfit_intervals - 1; l++) {
          ll = (int)(l * dinterval);
          misfit_intervals[l] = gridmeq[ll];
        }
      }

      /* get minimum misfit in 2D plane at current z offset */
      mbnavadjust_get_misfitxy();

      /* set message on */
      if (mbna_verbose > 1)
        fprintf(stderr, "Estimating 3D uncertainty for crossing %d\n", mbna_current_crossing);
      sprintf(message, "Estimating 3D uncertainty for crossing %d\n", mbna_current_crossing);
      do_message_update(message);

      /* estimating 3 component uncertainty vector at minimum misfit point */
      /* first get the longest vector to a misfit value <= 2 times minimum misfit */
      minmisfitthreshold = mbna_minmisfit * 3.0;
      mbna_minmisfit_sr1 = 0.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold) {
              x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
              y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
              z = zmin + zoff_dz * kc - mbna_minmisfit_z;
              r = sqrt(x * x + y * y + z * z);
              /* fprintf(stderr,"DEBUG %s %d: %d %d %d gridm[%d]:%f minmisfitthreshold:%f x: %f %f %f  r:%f\n",
              __FILE__,__LINE__,
              ic,jc,kc,lc,gridm[lc],minmisfitthreshold,x,y,z,r); */
              if (r > mbna_minmisfit_sr1) {
                mbna_minmisfit_sx1[0] = x;
                mbna_minmisfit_sx1[1] = y;
                mbna_minmisfit_sx1[2] = z;
                mbna_minmisfit_sr1 = r;
              }
            }
          }
      mbna_minmisfit_sx1[0] /= mbna_minmisfit_sr1;
      mbna_minmisfit_sx1[1] /= mbna_minmisfit_sr1;
      mbna_minmisfit_sx1[2] /= mbna_minmisfit_sr1;
      /* fprintf(stderr,"DEBUG %s %d: Longest vector in misfit space. %f %f %f  r:%f\n",
      __FILE__,__LINE__,
      mbna_minmisfit_sx1[0],mbna_minmisfit_sx1[1],mbna_minmisfit_sx1[2],mbna_minmisfit_sr1); */

      /* now get a horizontal unit vector perpendicular to the the longest vector
          and then find the largest r associated with that vector */
      mbna_minmisfit_sr2 =
          sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
      if (mbna_minmisfit_sr2 < MBNA_SMALL) {
        mbna_minmisfit_sx2[0] = 0.0;
        mbna_minmisfit_sx2[1] = 1.0;
        mbna_minmisfit_sx2[2] = 0.0;
        mbna_minmisfit_sr2 = MBNA_SMALL;
      }
      else {
        mbna_minmisfit_sx2[0] = mbna_minmisfit_sx1[1] / mbna_minmisfit_sr2;
        mbna_minmisfit_sx2[1] = -mbna_minmisfit_sx1[0] / mbna_minmisfit_sr2;
        mbna_minmisfit_sx2[2] = 0.0;
        mbna_minmisfit_sr2 =
            sqrt(mbna_minmisfit_sx2[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx2[1] * mbna_minmisfit_sx2[1] +
                 mbna_minmisfit_sx2[2] * mbna_minmisfit_sx2[2]);
      }
      /* dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx2[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx2[1] +
      mbna_minmisfit_sx1[2] * mbna_minmisfit_sx2[2]); fprintf(stderr,"DEBUG %s %d: Horizontal perpendicular vector in misfit
      space. %f %f %f  r:%f dotproduct:%f\n",
      __FILE__,__LINE__,
      mbna_minmisfit_sx2[0],mbna_minmisfit_sx2[1],mbna_minmisfit_sx2[2],mbna_minmisfit_sr2,dotproduct); */

      /* now get a near-vertical unit vector perpendicular to the the longest vector
          and then find the largest r associated with that vector */
      mbna_minmisfit_sr3 =
          sqrt(mbna_minmisfit_sx1[0] * mbna_minmisfit_sx1[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx1[1]);
      if (mbna_minmisfit_sr3 < MBNA_ZSMALL) {
        mbna_minmisfit_sx3[0] = 0.0;
        mbna_minmisfit_sx3[1] = 0.0;
        mbna_minmisfit_sx3[2] = 1.0;
        mbna_minmisfit_sr3 = MBNA_ZSMALL;
      }
      else {
        if (mbna_minmisfit_sx1[2] >= 0.0) {
          mbna_minmisfit_sx3[0] =
              -mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
          mbna_minmisfit_sx3[1] =
              -mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
        }
        else {
          mbna_minmisfit_sx3[0] =
              mbna_minmisfit_sx1[0] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
          mbna_minmisfit_sx3[1] =
              mbna_minmisfit_sx1[1] * sqrt(1.0 - mbna_minmisfit_sr3 * mbna_minmisfit_sr3) / mbna_minmisfit_sr3;
        }
        mbna_minmisfit_sx3[2] = mbna_minmisfit_sr3;
        mbna_minmisfit_sr3 =
            sqrt(mbna_minmisfit_sx3[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx3[1] * mbna_minmisfit_sx3[1] +
                 mbna_minmisfit_sx3[2] * mbna_minmisfit_sx3[2]);
      }
      /* dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx3[0] + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx3[1] +
      mbna_minmisfit_sx1[2] * mbna_minmisfit_sx3[2]); fprintf(stderr,"DEBUG %s %d: Perpendicular near-vertical vector in
      misfit space. %f %f %f  r:%f dotproduct:%f\n",
      __FILE__,__LINE__,
      mbna_minmisfit_sx3[0],mbna_minmisfit_sx3[1],mbna_minmisfit_sx3[2],mbna_minmisfit_sr2,dotproduct); */

      /* now get the longest r values to a misfit value <= 2 times minimum misfit
          for both secondary vectors */
      mbna_minmisfit_sr2 = 0.0;
      mbna_minmisfit_sr3 = 0.0;
      dotproductsave2 = 0.0;
      rsave2 = 0.0;
      dotproductsave3 = 0.0;
      rsave3 = 0.0;
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++)
          for (int kc = 0; kc < nzmisfitcalc; kc++) {
            lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
            if (gridnm[lc] > mbna_minmisfit_nthreshold && gridm[lc] <= minmisfitthreshold) {
              x = ((ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x - mbna_minmisfit_x) / mbna_mtodeglon;
              y = ((jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y - mbna_minmisfit_y) / mbna_mtodeglat;
              z = zmin + zoff_dz * kc - mbna_minmisfit_z;
              r = sqrt(x * x + y * y + z * z);
              if (r > mbna_minmisfit_sr2) {
                dotproduct =
                    (x * mbna_minmisfit_sx2[0] + y * mbna_minmisfit_sx2[1] + z * mbna_minmisfit_sx2[2]) / r;
                if (fabs(dotproduct) > 0.8) {
                  /* fprintf(stderr,"DEBUG %s %d: Vector2: %d %d %d gridm[%d]:%f minmisfitthreshold:%f
                  dotproduct:%f x: %f %f %f  r:%f\n",
                  __FILE__,__LINE__,
                  ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r); */
                  mbna_minmisfit_sr2 = r;
                }
                if (fabs(dotproduct) > dotproductsave2) {
                  dotproductsave2 = fabs(dotproduct);
                  rsave2 = r;
                }
              }
              if (r > mbna_minmisfit_sr3) {
                dotproduct =
                    (x * mbna_minmisfit_sx3[0] + y * mbna_minmisfit_sx3[1] + z * mbna_minmisfit_sx3[2]) / r;
                if (fabs(dotproduct) > 0.8) {
                  /* fprintf(stderr,"DEBUG %s %d: Vector3: %d %d %d gridm[%d]:%f minmisfitthreshold:%f
                  dotproduct:%f x: %f %f %f  r:%f\n",
                  __FILE__,__LINE__,
                  ic,jc,kc,lc,gridm[lc],minmisfitthreshold,dotproduct,x,y,z,r); */
                  mbna_minmisfit_sr3 = r;
                }
                if (fabs(dotproduct) > dotproductsave3) {
                  dotproductsave3 = fabs(dotproduct);
                  rsave3 = r;
                }
              }
            }
          }
      if (mbna_minmisfit_sr2 < MBNA_SMALL)
        mbna_minmisfit_sr2 = rsave2;
      if (mbna_minmisfit_sr3 < MBNA_ZSMALL)
        mbna_minmisfit_sr3 = rsave3;
    }
    else {
      mbna_minmisfit_sx1[0] = 1.0;
      mbna_minmisfit_sx1[1] = 0.0;
      mbna_minmisfit_sx1[2] = 0.0;
      mbna_minmisfit_sr1 = 100.0;
      mbna_minmisfit_sx2[0] = 0.0;
      mbna_minmisfit_sx2[1] = 1.0;
      mbna_minmisfit_sx2[2] = 0.0;
      mbna_minmisfit_sr2 = 100.0;
      mbna_minmisfit_sx3[0] = 0.0;
      mbna_minmisfit_sx3[1] = 0.0;
      mbna_minmisfit_sx3[2] = 1.0;
      mbna_minmisfit_sr3 = 100.0;
    }
    /* fprintf(stderr,"DEBUG %s %d: \nVector1: %f %f %f  mbna_minmisfit_sr1:%f\n",
    __FILE__,__LINE__,
    mbna_minmisfit_sx1[0],mbna_minmisfit_sx1[1],mbna_minmisfit_sx1[2],mbna_minmisfit_sr1);
    fprintf(stderr,"Vector2: %f %f %f  mbna_minmisfit_sr2:%f\n",
    mbna_minmisfit_sx2[0],mbna_minmisfit_sx2[1],mbna_minmisfit_sx2[2],mbna_minmisfit_sr2);
    fprintf(stderr,"Vector3: %f %f %f  mbna_minmisfit_sr3:%f\n",
    mbna_minmisfit_sx3[0],mbna_minmisfit_sx3[1],mbna_minmisfit_sx3[2],mbna_minmisfit_sr3);
    dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx2[0]
            + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx2[1]
            + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx2[2]);
    fprintf(stderr,"Dot products: 1v2:%f ",dotproduct);
    dotproduct = (mbna_minmisfit_sx2[0] * mbna_minmisfit_sx3[0]
            + mbna_minmisfit_sx2[1] * mbna_minmisfit_sx3[1]
            + mbna_minmisfit_sx2[2] * mbna_minmisfit_sx3[2]);
    fprintf(stderr,"2v3:%f ",dotproduct);
    dotproduct = (mbna_minmisfit_sx1[0] * mbna_minmisfit_sx3[0]
            + mbna_minmisfit_sx1[1] * mbna_minmisfit_sx3[1]
            + mbna_minmisfit_sx1[2] * mbna_minmisfit_sx3[2]);
    fprintf(stderr,"3v2:%f\n",dotproduct); */
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_get_misfitxy() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  int kc, lc;

  if (project.open && project.num_crossings > 0 && mbna_current_crossing >= 0 && mbna_naverr_load) {
    /* get minimum misfit in plane at current z offset */
    if (grid_nxyzeq > 0) {
      /* get closest to current zoffset in existing 3d grid */
      misfit_max = 0.0;
      misfit_min = 0.0;
      kc = (int)((mbna_offset_z - zmin) / zoff_dz);
      for (int ic = 0; ic < gridm_nx; ic++)
        for (int jc = 0; jc < gridm_ny; jc++) {
          lc = kc + nzmisfitcalc * (ic + jc * gridm_nx);
          if (gridnm[lc] > mbna_minmisfit_nthreshold) {
            if (misfit_max == 0.0) {
              misfit_min = gridm[lc];
              misfit_max = gridm[lc];
            }
            else if (gridm[lc] < misfit_min) {
              misfit_min = gridm[lc];
              mbna_minmisfit_xh = (ic - gridm_nx / 2) * grid_dx + mbna_misfit_offset_x;
              mbna_minmisfit_yh = (jc - gridm_ny / 2) * grid_dy + mbna_misfit_offset_y;
              mbna_minmisfit_zh = zmin + zoff_dz * kc;
            }
            else if (gridm[lc] > misfit_max) {
              misfit_max = gridm[lc];
            }
          }
        }
      /* fprintf(stderr,"mbnavadjust_get_misfitxy a mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
      mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */
    }
    /* fprintf(stderr,"mbnavadjust_get_misfitxy b mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
    mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */
  }
  /* fprintf(stderr,"mbnavadjust_get_misfitxy c mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
  mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh); */

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
void mbnavadjust_plot(double xx, double yy, int ipen) {
  if (mbna_contour->nvector >= mbna_contour->nvector_alloc) {
    mbna_contour->nvector_alloc += MBNA_VECTOR_ALLOC_INC;
    mbna_contour->vector = (struct mbna_plot_vector *)realloc(mbna_contour->vector, sizeof(struct mbna_plot_vector) *
                                                                                        (mbna_contour->nvector_alloc));
    if (mbna_contour->vector == NULL)
      mbna_contour->nvector_alloc = 0;
  }

  struct mbna_plot_vector *v;
  double x, y;

  if (mbna_contour->nvector_alloc > mbna_contour->nvector) {
    /* add current origin */
    x = xx + mbna_ox;
    y = yy + mbna_oy;

    /* move pen */
    if (ipen == MBNA_PEN_UP) {
      /* save move in vector array */
      v = &mbna_contour->vector[mbna_contour->nvector];
      v->command = ipen;
      v->x = xx;
      v->y = yy;
      mbna_contour->nvector++;
    }

    /* plot */
    else if (ipen == MBNA_PEN_DOWN) {
      /* save move in vector array */
      v = &mbna_contour->vector[mbna_contour->nvector];
      v->command = ipen;
      v->x = xx;
      v->y = yy;
      mbna_contour->nvector++;
    }

    /* change origin */
    else if (ipen == MBNA_PEN_ORIGIN) {
      mbna_ox = x;
      mbna_oy = y;
    }
  }
}
/*--------------------------------------------------------------------*/
void mbnavadjust_newpen(int icolor) {
  if (mbna_contour->nvector >= mbna_contour->nvector_alloc) {
    mbna_contour->nvector_alloc += MBNA_VECTOR_ALLOC_INC;
    mbna_contour->vector = (struct mbna_plot_vector *)realloc(
        mbna_contour->vector, sizeof(struct mbna_plot_vector) * (mbna_contour->nvector_alloc + MBNA_VECTOR_ALLOC_INC));
    if (mbna_contour->vector == NULL)
      mbna_contour->nvector_alloc = 0;
  }

  if (mbna_contour->nvector_alloc > mbna_contour->nvector) {
    /* save pen change in vector array */
    struct mbna_plot_vector *v = &mbna_contour->vector[mbna_contour->nvector];
    v->command = MBNA_PEN_COLOR;
    v->color = pixel_values[icolor * 8 + 7];
    mbna_contour->nvector++;
  }
}
/*--------------------------------------------------------------------*/
void mbnavadjust_setline(int linewidth) {
  (void)linewidth;  // Unused parameter
}
/*--------------------------------------------------------------------*/
void mbnavadjust_justify_string(double height, char *string, double *s) {
  const int len = strlen(string);
  s[0] = 0.0;
  s[1] = 0.185 * height * len;
  s[2] = 0.37 * len * height;
  s[3] = 0.37 * len * height;
}
/*--------------------------------------------------------------------*/
void mbnavadjust_plot_string(double x, double y, double hgt, double angle, char *label) {
  (void)x;  // Unused parameter
  (void)y;  // Unused parameter
  (void)hgt;  // Unused parameter
  (void)angle;  // Unused parameter
  (void)label;  // Unused parameter
}
/*--------------------------------------------------------------------*/

void mbnavadjust_naverr_scale() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  double xscale, yscale;

  if (mbna_naverr_load) {
    /* set scaling for contour window */
    xscale = (cont_borders[1] - cont_borders[0]) / ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon);
    yscale = (cont_borders[3] - cont_borders[2]) / ((mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat);
    if (xscale < yscale) {
      mbna_plotx_scale = xscale / mbna_mtodeglon;
      mbna_ploty_scale = xscale / mbna_mtodeglat;
      mbna_plot_lat_min =
          0.5 * (mbna_plot_lat_min + mbna_plot_lat_max) - 0.5 * (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
      mbna_plot_lat_max = mbna_plot_lat_min + (cont_borders[3] - cont_borders[2]) / mbna_ploty_scale;
    }
    else {
      mbna_plotx_scale = yscale / mbna_mtodeglon;
      mbna_ploty_scale = yscale / mbna_mtodeglat;
      mbna_plot_lon_min =
          0.5 * (mbna_plot_lon_min + mbna_plot_lon_max) - 0.5 * (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
      mbna_plot_lon_max = mbna_plot_lon_min + (cont_borders[1] - cont_borders[0]) / mbna_plotx_scale;
    }

    /* set scaling for misfit window */
    mbna_misfit_xscale = (corr_borders[1] - corr_borders[0]) / (grid_dx * (gridm_nx - 1));
    mbna_misfit_yscale = (corr_borders[3] - corr_borders[2]) / (grid_dy * (gridm_ny - 1));
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }
}
/*--------------------------------------------------------------------*/

void mbnavadjust_naverr_plot(int plotmode) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_plot_vector *v;
  struct mbna_crossing *crossing;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;
  struct mbna_tie *tie;
  int ix, iy, ix1, ix2, iy1, iy2, idx, idy;
  int boxoff, boxwid;
  static int ixo, iyo;
  static int izx1, izy1, izx2, izy2;
  static int pixel, ipixel;
  int snav_1, snav_2;
  int fill;
  bool found;
  int i, j, k, l;

  if (mbna_naverr_load) {
    /* get structures */
    crossing = (struct mbna_crossing *)&project.crossings[mbna_current_crossing];
    file1 = (struct mbna_file *)&project.files[crossing->file_id_1];
    file2 = (struct mbna_file *)&project.files[crossing->file_id_2];
    section1 = (struct mbna_section *)&file1->sections[crossing->section_1];
    section2 = (struct mbna_section *)&file2->sections[crossing->section_2];

    /* get naverr plot scaling */
    mbnavadjust_naverr_scale();

    /* clear screens for first plot */
    if (plotmode == MBNA_PLOT_MODE_FIRST) {
      xg_fillrectangle(pcont_xgid, 0, 0, cont_borders[1], cont_borders[3], pixel_values[mbna_color_background],
                       XG_SOLIDLINE);
      xg_fillrectangle(pcorr_xgid, 0, 0, corr_borders[1], corr_borders[3], pixel_values[mbna_color_background],
                       XG_SOLIDLINE);
    }
    xg_fillrectangle(pzoff_xgid, 0, 0, zoff_borders[1], zoff_borders[3], pixel_values[mbna_color_background], XG_SOLIDLINE);

    /* replot section 2 and tie points in background if moving that section */
    if (plotmode == MBNA_PLOT_MODE_MOVE) {
      for (int i = 0; i < mbna_contour2.nvector; i++) {
        v = &mbna_contour2.vector[i];

        if (v->command == MBNA_PEN_UP) {
          ixo = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
          iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
        }
        else if (v->command == MBNA_PEN_DOWN) {
          ix = (int)(mbna_plotx_scale * (v->x + mbna_offset_x_old - mbna_plot_lon_min));
          iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y_old - mbna_plot_lat_min));
          xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
          ixo = ix;
          iyo = iy;
        }
      }
      ixo = (int)(mbna_plotx_scale * (swathraw2->pingraws[0].navlon + mbna_offset_x_old - mbna_plot_lon_min));
      iyo = (int)(cont_borders[3] -
                  mbna_ploty_scale * (swathraw2->pingraws[0].navlat + mbna_offset_y_old - mbna_plot_lat_min));
      for (int i = 1; i < swathraw2->npings; i++) {
        ix = (int)(mbna_plotx_scale * (swathraw2->pingraws[i].navlon + mbna_offset_x_old - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] -
                   mbna_ploty_scale * (swathraw2->pingraws[i].navlat + mbna_offset_y_old - mbna_plot_lat_min));
        xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
        ixo = ix;
        iyo = iy;
      }

      /* replot tie points */
      if (crossing->num_ties > 0) {
        for (int i = 0; i < crossing->num_ties; i++) {
          tie = &crossing->ties[i];
          if (i == mbna_current_tie) {
            boxoff = 6;
            boxwid = 13;
            snav_1 = mbna_snav_1;
            snav_2 = mbna_snav_2;
          }
          else {
            boxoff = 3;
            boxwid = 7;
            snav_1 = tie->snav_1;
            snav_2 = tie->snav_2;
          }
          ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] - mbna_plot_lon_min));
          iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] - mbna_plot_lat_min));
          xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                           XG_SOLIDLINE);
          xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                           XG_SOLIDLINE);
          ixo = ix;
          iyo = iy;
          ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] + mbna_offset_x_old - mbna_plot_lon_min));
          iy = (int)(cont_borders[3] -
                     mbna_ploty_scale * (section2->snav_lat[snav_2] + mbna_offset_y_old - mbna_plot_lat_min));
          xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                           XG_SOLIDLINE);
          xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                           XG_SOLIDLINE);
          xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
        }
      }
    }

    /* replot zoom box in background if moving that box */
    if (plotmode == MBNA_PLOT_MODE_ZOOM) {
      xg_drawrectangle(pcont_xgid, MIN(izx1, izx2), MIN(izy1, izy2), MAX(izx1, izx2) - MIN(izx1, izx2),
                       MAX(izy1, izy2) - MIN(izy1, izy2), pixel_values[mbna_color_background], XG_SOLIDLINE);
    }

    /* replot overlap box in background */
    if (mbna_overlap_lon_max > mbna_overlap_lon_min && mbna_overlap_lat_max > mbna_overlap_lat_min) {
      ix1 = (int)(mbna_plotx_scale * (mbna_overlap_lon_min - mbna_plot_lon_min));
      iy1 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_min - mbna_plot_lat_min));
      ix2 = (int)(mbna_plotx_scale * (mbna_overlap_lon_max - mbna_plot_lon_min));
      iy2 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_max - mbna_plot_lat_min));
      ix = MIN(ix1, ix2);
      iy = MIN(iy1, iy2);
      idx = MAX(ix1, ix2) - ix;
      idy = MAX(iy1, iy2) - iy;
      xg_drawrectangle(pcont_xgid, ix, iy, idx, idy, pixel_values[mbna_color_background], XG_DASHLINE);
    }

    /* plot section 1 */
    for (int i = 0; i < mbna_contour1.nvector; i++) {
      v = &mbna_contour1.vector[i];

      if (v->command == MBNA_PEN_COLOR) {
        pixel = v->color;
      }
      else if (v->command == MBNA_PEN_UP) {
        ixo = (int)(mbna_plotx_scale * (v->x - mbna_plot_lon_min));
        iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y - mbna_plot_lat_min));
      }
      else if (v->command == MBNA_PEN_DOWN) {
        ix = (int)(mbna_plotx_scale * (v->x - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y - mbna_plot_lat_min));
        xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
        ixo = ix;
        iyo = iy;
      }
    }
    ixo = (int)(mbna_plotx_scale * (swathraw1->pingraws[0].navlon - mbna_plot_lon_min));
    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[0].navlat - mbna_plot_lat_min));
    for (int i = 1; i < swathraw1->npings; i++) {
      ix = (int)(mbna_plotx_scale * (swathraw1->pingraws[i].navlon - mbna_plot_lon_min));
      iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw1->pingraws[i].navlat - mbna_plot_lat_min));
      xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      ixo = ix;
      iyo = iy;
    }

    /* plot section 2 */
    for (int i = 0; i < mbna_contour2.nvector; i++) {
      v = &mbna_contour2.vector[i];

      if (v->command == MBNA_PEN_COLOR) {
        pixel = v->color;
      }
      else if (v->command == MBNA_PEN_UP) {
        ixo = (int)(mbna_plotx_scale * (v->x + mbna_offset_x - mbna_plot_lon_min));
        iyo = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y - mbna_plot_lat_min));
      }
      else if (v->command == MBNA_PEN_DOWN) {
        ix = (int)(mbna_plotx_scale * (v->x + mbna_offset_x - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] - mbna_ploty_scale * (v->y + mbna_offset_y - mbna_plot_lat_min));
        xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel, XG_SOLIDLINE);
        ixo = ix;
        iyo = iy;
      }
    }
    ixo = (int)(mbna_plotx_scale * (swathraw2->pingraws[0].navlon + mbna_offset_x - mbna_plot_lon_min));
    iyo = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[0].navlat + mbna_offset_y - mbna_plot_lat_min));
    for (int i = 1; i < swathraw2->npings; i++) {
      ix = (int)(mbna_plotx_scale * (swathraw2->pingraws[i].navlon + mbna_offset_x - mbna_plot_lon_min));
      iy = (int)(cont_borders[3] - mbna_ploty_scale * (swathraw2->pingraws[i].navlat + mbna_offset_y - mbna_plot_lat_min));
      xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      ixo = ix;
      iyo = iy;
    }

    /* plot tie points */
    mbnavadjust_naverr_checkoksettie();
    crossing = &project.crossings[mbna_current_crossing];
    if (crossing->num_ties > 0) {
      for (int i = 0; i < crossing->num_ties; i++) {
        tie = &crossing->ties[i];
        if (i == mbna_current_tie) {
          boxoff = 6;
          boxwid = 13;
          snav_1 = mbna_snav_1;
          snav_2 = mbna_snav_2;
          if (mbna_allow_set_tie)
            fill = pixel_values[RED];
          else
            fill = pixel_values[6];
        }
        else {
          boxoff = 3;
          boxwid = 7;
          snav_1 = tie->snav_1;
          snav_2 = tie->snav_2;
          fill = pixel_values[6];
        }
        ix = (int)(mbna_plotx_scale * (section1->snav_lon[snav_1] - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] - mbna_ploty_scale * (section1->snav_lat[snav_1] - mbna_plot_lat_min));
        xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
        xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground],
                         XG_SOLIDLINE);
        ixo = ix;
        iyo = iy;
        ix = (int)(mbna_plotx_scale * (section2->snav_lon[snav_2] + mbna_offset_x - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[snav_2] + mbna_offset_y - mbna_plot_lat_min));
        xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
        xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground],
                         XG_SOLIDLINE);
        xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }
    }

    /* plot overlap box */
    mbnavadjust_crossing_overlapbounds(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                       &mbna_overlap_lon_min, &mbna_overlap_lon_max,
                                           &mbna_overlap_lat_min, &mbna_overlap_lat_max,
                                           &error);
    ix1 = (int)(mbna_plotx_scale * (mbna_overlap_lon_min - mbna_plot_lon_min));
    iy1 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_min - mbna_plot_lat_min));
    ix2 = (int)(mbna_plotx_scale * (mbna_overlap_lon_max - mbna_plot_lon_min));
    iy2 = (int)(cont_borders[3] - mbna_ploty_scale * (mbna_overlap_lat_max - mbna_plot_lat_min));
    ix = MIN(ix1, ix2);
    iy = MIN(iy1, iy2);
    idx = MAX(ix1, ix2) - ix;
    idy = MAX(iy1, iy2) - iy;
    xg_drawrectangle(pcont_xgid, ix, iy, idx, idy, pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* plot zoom box if in zoom mode */
    if (plotmode == MBNA_PLOT_MODE_ZOOMFIRST || plotmode == MBNA_PLOT_MODE_ZOOM) {
      xg_drawrectangle(pcont_xgid, MIN(mbna_zoom_x1, mbna_zoom_x2), MIN(mbna_zoom_y1, mbna_zoom_y2),
                       MAX(mbna_zoom_x1, mbna_zoom_x2) - MIN(mbna_zoom_x1, mbna_zoom_x2),
                       MAX(mbna_zoom_y1, mbna_zoom_y2) - MIN(mbna_zoom_y1, mbna_zoom_y2),
                       pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      izx1 = mbna_zoom_x1;
      izy1 = mbna_zoom_y1;
      izx2 = mbna_zoom_x2;
      izy2 = mbna_zoom_y2;
    }

    /* plot misfit */
    ixo = corr_borders[0] + (corr_borders[1] - corr_borders[0]) / 2;
    iyo = corr_borders[2] + (corr_borders[3] - corr_borders[2]) / 2;
    // const double dmisfit = log10(misfit_max - misfit_min) / 79.99;
    k = (int)((mbna_offset_z - zmin) / zoff_dz);
    for (int i = 0; i < gridm_nx; i++)
      for (int j = 0; j < gridm_ny; j++) {
        l = k + nzmisfitcalc * (i + j * gridm_nx);
        if (gridnm[l] > 0) {
          ix = ixo + (int)(mbna_misfit_xscale * grid_dx * (i - gridm_nx / 2 - 0.5));
          iy = iyo - (int)(mbna_misfit_yscale * grid_dy * (j - gridm_ny / 2 + 0.5));
          idx = ixo + (int)(mbna_misfit_xscale * grid_dx * (i - gridm_nx / 2 + 0.5)) - ix;
          idy = iyo - (int)(mbna_misfit_yscale * grid_dy * (j - gridm_ny / 2 - 0.5)) - iy;

          /* histogram equalized coloring */
          if (gridm[l] <= misfit_intervals[0])
            ipixel = 7;
          else if (gridm[l] >= misfit_intervals[nmisfit_intervals - 1])
            ipixel = 7 + nmisfit_intervals - 1;
          else {
            found = false;
            for (int kk = 0; kk < nmisfit_intervals && !found; kk++) {
              if (gridm[l] > misfit_intervals[kk] && gridm[l] <= misfit_intervals[kk + 1]) {
                ipixel = 7 + kk;
                found = true;
              }
            }
          }
          /*fprintf(stderr, "%d %d %f %f %f   %f %d\n",
              i, j, misfit_min, misfit_max, dmisfit, gridm[l], ipixel);*/

          xg_fillrectangle(pcorr_xgid, ix, iy, idx, idy, pixel_values[ipixel], XG_SOLIDLINE);
        }
      }

    /* draw dashed crosshair across origin */
    xg_drawline(pcorr_xgid, ixo - (int)(mbna_misfit_xscale * mbna_misfit_offset_x), corr_borders[2],
                ixo - (int)(mbna_misfit_xscale * mbna_misfit_offset_x), corr_borders[3], pixel_values[mbna_color_foreground],
                XG_DASHLINE);
    xg_drawline(pcorr_xgid, corr_borders[0], iyo + (int)(mbna_misfit_yscale * mbna_misfit_offset_y), corr_borders[1],
                iyo + (int)(mbna_misfit_yscale * mbna_misfit_offset_y), pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* draw working offset */
    ix = ixo + (int)(mbna_misfit_xscale * (mbna_offset_x - mbna_misfit_offset_x));
    iy = iyo - (int)(mbna_misfit_yscale * (mbna_offset_y - mbna_misfit_offset_y));
    xg_fillrectangle(pcorr_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
    xg_drawrectangle(pcorr_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* draw uncertainty estimate */
    if (mbna_minmisfit_n > 0) {
      ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
      iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
      idx = (int)(mbna_misfit_xscale * (mbna_mtodeglon * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[0]));
      idy = -(int)(mbna_misfit_yscale * (mbna_mtodeglat * mbna_minmisfit_sr1 * mbna_minmisfit_sx1[1]));
      xg_drawline(pcorr_xgid, ix - idx, iy - idy, ix + idx, iy + idy, pixel_values[mbna_color_background], XG_SOLIDLINE);

      ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
      iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
      idx = (int)(mbna_misfit_xscale * (mbna_mtodeglon * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[0]));
      idy = -(int)(mbna_misfit_yscale * (mbna_mtodeglat * mbna_minmisfit_sr2 * mbna_minmisfit_sx2[1]));
      xg_drawline(pcorr_xgid, ix - idx, iy - idy, ix + idx, iy + idy, pixel_values[mbna_color_background], XG_SOLIDLINE);
    }

    /* draw x at minimum misfit */
    if (mbna_minmisfit_n > 0) {
      ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_x - mbna_misfit_offset_x));
      iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_y - mbna_misfit_offset_y));
      xg_drawline(pcorr_xgid, ix - 10, iy + 10, ix + 10, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      xg_drawline(pcorr_xgid, ix + 10, iy + 10, ix - 10, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }

    /* draw small x at minimum misfit for current z offset */
    if (mbna_minmisfit_n > 0) {
      ix = ixo + (int)(mbna_misfit_xscale * (mbna_minmisfit_xh - mbna_misfit_offset_x));
      iy = iyo - (int)(mbna_misfit_yscale * (mbna_minmisfit_yh - mbna_misfit_offset_y));
      xg_drawline(pcorr_xgid, ix - 5, iy + 5, ix + 5, iy - 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      xg_drawline(pcorr_xgid, ix + 5, iy + 5, ix - 5, iy - 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }

    /* draw + at inversion solution */
    if (project.inversion_status != MBNA_INVERSION_NONE) {
      ix = ixo + (int)(mbna_misfit_xscale * (mbna_invert_offset_x - mbna_misfit_offset_x));
      iy = iyo - (int)(mbna_misfit_yscale * (mbna_invert_offset_y - mbna_misfit_offset_y));
      xg_drawline(pcorr_xgid, ix - 10, iy, ix + 10, iy, pixel_values[GREEN], XG_SOLIDLINE);
      xg_drawline(pcorr_xgid, ix, iy + 10, ix, iy - 10, pixel_values[GREEN], XG_SOLIDLINE);
      xg_drawline(pcorr_xgid, ix - 10, iy, ix + 10, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      xg_drawline(pcorr_xgid, ix, iy + 10, ix, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }

    /* plot zoff */
    ixo = zoff_borders[0];
    iyo = zoff_borders[3];
    i = (int)((mbna_offset_x - mbna_misfit_offset_x) / grid_dx) + (int)(gridm_nx / 2);
    i = MAX(0, MIN(gridm_nx - 1, i));
    j = (int)((mbna_offset_y - mbna_misfit_offset_y) / grid_dy) + (int)(gridm_ny / 2);
    j = MAX(0, MIN(gridm_ny - 1, j));
    found = false;
    zmisfitmin = 10000000.0;
    zmisfitmax = 0.0;
    for (int k = 0; k < nzmisfitcalc; k++) {
      l = k + nzmisfitcalc * (i + j * gridm_nx);
      if (gridnm[l] > 0) {
        if (!found) {
          zmisfitmin = gridm[l];
          zmisfitmax = gridm[l];
          found = true;
        }
        else {
          zmisfitmin = MIN(zmisfitmin, gridm[l]);
          zmisfitmax = MAX(zmisfitmax, gridm[l]);
        }
      }
    }
    /*fprintf(stderr,"Current offset: %f %f %f\n",
    mbna_offset_x / mbna_mtodeglon, mbna_offset_y / mbna_mtodeglat, mbna_offset_z);
    fprintf(stderr,"Current misfit grid offset: %f %f %f\n",
    mbna_misfit_offset_x / mbna_mtodeglon, mbna_misfit_offset_y / mbna_mtodeglat, mbna_misfit_offset_z);
    fprintf(stderr,"Current min misfit position: %f %f %f\n",
    mbna_minmisfit_x / mbna_mtodeglon, mbna_minmisfit_y / mbna_mtodeglat, mbna_minmisfit_z);
    fprintf(stderr,"misfitmin:%f misfitmax:%f  zmisfitmin:%f zmisfitmax:%f\n\n",
    misfit_min,misfit_max,zmisfitmin,zmisfitmax);*/
    zmisfitmin = zmisfitmin - 0.05 * (zmisfitmax - zmisfitmin);
    zmisfitmax = zmisfitmax + 0.04 * (zmisfitmax - zmisfitmin);
    mbna_zoff_scale_x = (zoff_borders[1] - zoff_borders[0]) / (project.zoffsetwidth);
    mbna_zoff_scale_y = (zoff_borders[3] - zoff_borders[2]) / (zmisfitmax - zmisfitmin);
    for (int k = 0; k < nzmisfitcalc; k++) {
      l = k + nzmisfitcalc * (i + j * gridm_nx);
      if (gridnm[l] > 0) {
        /* histogram equalized coloring */
        if (gridm[l] <= misfit_intervals[0])
          ipixel = 7;
        else if (gridm[l] >= misfit_intervals[nmisfit_intervals - 1])
          ipixel = 7 + nmisfit_intervals - 1;
        else {
          found = false;
          for (int kk = 0; kk < nmisfit_intervals && !found; kk++) {
            if (gridm[l] > misfit_intervals[kk] && gridm[l] <= misfit_intervals[kk + 1]) {
              ipixel = 7 + kk;
              found = true;
            }
          }
        }
        ix = ixo + (int)(mbna_zoff_scale_x * zoff_dz * (k - 0.5));
        iy = (int)(mbna_zoff_scale_y * (gridm[l] - zmisfitmin));
        idx = (int)(mbna_zoff_scale_x * zoff_dz);
        idx = MAX(idx, 1);
        idy = iyo - iy;
        /* fprintf(stderr,"Fill Zoff: %d %d %d %d  pixel:%d\n",
        ix, iy, idx, idy, pixel_values[ipixel]);*/
        xg_fillrectangle(pzoff_xgid, ix, iy, idx, idy, pixel_values[ipixel], XG_SOLIDLINE);
      }
    }

    /* plot zero zoff */
    ix = ixo - (int)(mbna_zoff_scale_x * zmin);
    xg_drawline(pzoff_xgid, ix, zoff_borders[2], ix, zoff_borders[3], pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* draw working offset */
    ix = ixo + (int)(mbna_zoff_scale_x * (mbna_offset_z - zmin));
    xg_drawline(pzoff_xgid, ix, zoff_borders[2], ix, zoff_borders[3], pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* draw x at minimum misfit */
    if (mbna_minmisfit_n > 0) {
      ix = ixo + (int)(mbna_zoff_scale_x * (mbna_minmisfit_z - zmin));
      iy = zoff_borders[3] / 2;
      xg_drawline(pzoff_xgid, ix - 10, iy + 10, ix + 10, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      xg_drawline(pzoff_xgid, ix + 10, iy + 10, ix - 10, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }

    /* draw + at inversion solution */
    if (project.inversion_status != MBNA_INVERSION_NONE) {
      ix = ixo + (int)(mbna_zoff_scale_x * (mbna_invert_offset_z - zmin));
      iy = zoff_borders[3] / 2;
      xg_drawline(pzoff_xgid, ix - 10, iy, ix + 10, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      xg_drawline(pzoff_xgid, ix, iy + 10, ix, iy - 10, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }
}
/*--------------------------------------------------------------------*/

int mbnavadjust_autopick(bool do_vertical) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       do_vertical: %d\n", do_vertical);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;
  double dlon, dlat, overlap_scale;
  bool process;
  int nprocess;
  int isnav1_focus, isnav2_focus;
  double lon_focus, lat_focus;

  // loop over all crossings, autopick those that are in the current view,
  // unanalyzed, have sufficient overlap, and for which both sections are
  // sufficiently long (track length >=0.25 * file->sections[0].distance)
  if (project.open && project.num_crossings > 0) {
    /* set message dialog on */
    sprintf(message, "Autopicking offsets...");
    do_message_on(message);
    sprintf(message, "Autopicking offsets...\n");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s\n", message);
    do_info_add(message, true);

    /* loop over all crossings */
    nprocess = 0;
    for (int i = 0; i < project.num_crossings; i++) {
      /* get structure */
      crossing = &(project.crossings[i]);

      // check crossing is in current view and has sufficient overlap
      process = false;
      if (crossing->status == MBNA_CROSSING_STATUS_NONE && crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
        if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
          if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
              (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
               mbna_survey_select == project.files[crossing->file_id_1].block &&
               mbna_survey_select == project.files[crossing->file_id_2].block) ||
              (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
               mbna_file_select == crossing->file_id_2) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
               (mbna_survey_select == project.files[crossing->file_id_1].block ||
                mbna_survey_select == project.files[crossing->file_id_2].block)) ||
              (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
               ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                 mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                 mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
               (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
               mbna_section_select == crossing->section_1) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
               mbna_section_select == crossing->section_2))
            process = true;
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
          if (crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2))
              process = true;
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
          if (crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2))
              process = true;
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
          if (crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2))
              process = true;
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
          if (crossing->truecrossing) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2))
              process = true;
          }
        }
        else
          process = true;
      }

      // check if section track lengths are long enough (at least 0.25 of the
      // length of the first section for the file) - this excludes trying to
      // match short sections at the end of a file */
      if (process) {
        file1 = &project.files[crossing->file_id_1];
        section1 = &file1->sections[crossing->section_1];
        file2 = &project.files[crossing->file_id_2];
        section2 = &file2->sections[crossing->section_2];
        if (section1->distance < 0.25 * file1->sections[0].distance
            || section2->distance < 0.25 * file2->sections[0].distance) {
          process = false;
        }
      }

      /* load the crossing */
      if (process) {
        /* fprintf(stderr,"AUTOPICK crossing:%d do_vertical:%d process:%d\n",i,do_vertical,process); */
        mbna_current_crossing = i;
        mbna_file_id_1 = crossing->file_id_1;
        mbna_section_1 = crossing->section_1;
        mbna_file_id_2 = crossing->file_id_2;
        mbna_section_2 = crossing->section_2;
        mbna_current_tie = -1;

        /* reset survey file and section selections */
        if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
          if (mbna_survey_select == project.files[crossing->file_id_1].block) {
            mbna_file_select = crossing->file_id_1;
            mbna_section_select = crossing->section_1;
          }
          else if (mbna_survey_select == project.files[crossing->file_id_2].block) {
            mbna_file_select = crossing->file_id_2;
            mbna_section_select = crossing->section_2;
          }
          else {
            mbna_file_select = crossing->file_id_1;
            mbna_section_select = crossing->section_1;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
          if (mbna_file_select == crossing->file_id_1) {
            mbna_survey_select = project.files[crossing->file_id_1].block;
            mbna_section_select = crossing->section_1;
          }
          else if (mbna_file_select == crossing->file_id_2) {
            mbna_survey_select = project.files[crossing->file_id_2].block;
            mbna_section_select = crossing->section_2;
          }
          else {
            mbna_survey_select = project.files[crossing->file_id_1].block;
            mbna_section_select = crossing->section_1;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
          if (mbna_file_select == crossing->file_id_1 && mbna_section_select == crossing->section_1) {
            mbna_survey_select = project.files[crossing->file_id_1].block;
            mbna_file_select = crossing->file_id_1;
          }
          else if (mbna_file_select == crossing->file_id_2 && mbna_section_select == crossing->section_2) {
            mbna_survey_select = project.files[crossing->file_id_2].block;
            mbna_file_select = crossing->file_id_2;
          }
          else {
            mbna_survey_select = project.files[crossing->file_id_1].block;
            mbna_file_select = crossing->file_id_1;
          }
        }
        else if (mbna_file_select == crossing->file_id_1) {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }
        else if (mbna_file_select == crossing->file_id_2) {
          mbna_survey_select = project.files[crossing->file_id_2].block;
          mbna_file_select = crossing->file_id_2;
          mbna_section_select = crossing->section_2;
        }
        else {
          mbna_survey_select = project.files[crossing->file_id_1].block;
          mbna_file_select = crossing->file_id_1;
          mbna_section_select = crossing->section_1;
        }

        /* set message dialog on */
        sprintf(message, "Loading crossing %d...", mbna_current_crossing);
        fprintf(stderr, "\n%s: %s\n", __func__, message);
        do_message_update(message);

        /* load crossing */
        mbnavadjust_crossing_load();
        /* fprintf(stderr,"mbnavadjust_autopick AA crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f
        minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n", mbna_current_crossing,crossing->overlap,overlap_scale,
        mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
        mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
        mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */
        nprocess++;

        /* update status */
        do_update_status();
        /* fprintf(stderr,"mbnavadjust_autopick A crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f
        minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n", mbna_current_crossing,crossing->overlap,overlap_scale,
        mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
        mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
        mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */

        /* if this is a >50% overlap crossing then first set offsets to
            minimum misfit and then recalculate misfit */
        if (crossing->overlap > 50) {
          /* set offsets to minimum misfit */
          if (do_vertical) {
            mbna_offset_x = mbna_minmisfit_x;
            mbna_offset_y = mbna_minmisfit_y;
            mbna_offset_z = mbna_minmisfit_z;
          }
          else {
            mbna_offset_x = mbna_minmisfit_xh;
            mbna_offset_y = mbna_minmisfit_yh;
            mbna_offset_z = mbna_minmisfit_zh;
          }
          mbna_misfit_offset_x = mbna_offset_x;
          mbna_misfit_offset_y = mbna_offset_y;
          mbna_misfit_offset_z = mbna_offset_z;
          mbnavadjust_crossing_replot();

          /* get misfit */
          mbnavadjust_get_misfit();
          /* fprintf(stderr,"mbnavadjust_autopick B crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f
          minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n", mbna_current_crossing,crossing->overlap,overlap_scale,
          mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
          mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
          mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh); */
        }

        /* set plot bounds to overlap region and recalculate misfit */
                mbnavadjust_crossing_overlapbounds(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                                   &mbna_overlap_lon_min, &mbna_overlap_lon_max,
                                                   &mbna_overlap_lat_min, &mbna_overlap_lat_max,
                                                   &error);
        mbna_plot_lon_min = mbna_overlap_lon_min;
        mbna_plot_lon_max = mbna_overlap_lon_max;
        mbna_plot_lat_min = mbna_overlap_lat_min;
        mbna_plot_lat_max = mbna_overlap_lat_max;

                /* get characteristic scale of the overlap region */
        overlap_scale = MIN((mbna_overlap_lon_max - mbna_overlap_lon_min) / mbna_mtodeglon,
                            (mbna_overlap_lat_max - mbna_overlap_lat_min) / mbna_mtodeglat);

        /* get naverr plot scaling */
        mbnavadjust_naverr_scale();

        /* get misfit */
        mbnavadjust_get_misfit();

                /* The overlap focus point is currently the center of the line
                 * connecting the two closest approach nav points. It could also
                 * be the centroid of the overlap regions. */
                mbnavadjust_crossing_focuspoint(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                                   &isnav1_focus, &isnav2_focus, &lon_focus, &lat_focus,
                                                   &error);

        /* If nonzero overlap region and focus point inside the overlap region,
                 * set plot bounds to cover one-quarter
                 * of overlap region centered on the focus point and
                 * recalculate misfit. */
                if (mbna_overlap_lon_max > mbna_overlap_lon_min && mbna_overlap_lat_max > mbna_overlap_lat_min
                    && lon_focus >= mbna_overlap_lon_min && lon_focus <= mbna_overlap_lon_max
                    && lat_focus >= mbna_overlap_lat_min && lat_focus <= mbna_overlap_lat_max) {
                    dlon =  0.25 * (mbna_overlap_lon_max - mbna_overlap_lon_min);
                    dlat =  0.25 * (mbna_overlap_lat_max - mbna_overlap_lat_min);
                    mbna_plot_lon_min = MAX((lon_focus - dlon), mbna_overlap_lon_min);
                    mbna_plot_lon_max = MIN((lon_focus + dlon), mbna_overlap_lon_max);
                    mbna_plot_lat_min = MAX((lat_focus - dlat), mbna_overlap_lat_min);
                    mbna_plot_lat_max = MIN((lat_focus + dlat), mbna_overlap_lat_max);

                    /* get naverr plot scaling */
                    mbnavadjust_naverr_scale();

                    /* get misfit */
                    mbnavadjust_get_misfit();
                }

        /* check uncertainty estimate for a good pick */
        /* fprintf(stderr,"mbnavadjust_autopick C crossing:%d overlap:%d overlap_scale:%f current offsets:%f %f %f
        minmisfit3D:%f %f %f  minmisfit2D:%f %f %f\n", mbna_current_crossing,crossing->overlap,overlap_scale,
        mbna_offset_x/mbna_mtodeglon,mbna_offset_y/mbna_mtodeglat,mbna_offset_z,
        mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z,
        mbna_minmisfit_xh/mbna_mtodeglon,mbna_minmisfit_yh/mbna_mtodeglat,mbna_minmisfit_zh);
        fprintf(stderr,"crossing:%d overlap:%d overlap_scale:%f uncertainty axes: %f %f %f",
        mbna_current_crossing,crossing->overlap,overlap_scale,mbna_minmisfit_sr1,mbna_minmisfit_sr2,mbna_minmisfit_sr3);
        if (MAX(mbna_minmisfit_sr1,mbna_minmisfit_sr2) < 0.5 * overlap_scale
        && MIN(mbna_minmisfit_sr1,mbna_minmisfit_sr2) > 0.0)
        fprintf(stderr," USE PICK");
        fprintf(stderr,"\n"); */

        fprintf(stderr, "Long misfit axis:%.3f Threshold:%.3f", MAX(mbna_minmisfit_sr1, mbna_minmisfit_sr2),
                0.5 * overlap_scale);

        if (MAX(mbna_minmisfit_sr1, mbna_minmisfit_sr2) < 0.5 * overlap_scale &&
            MIN(mbna_minmisfit_sr1, mbna_minmisfit_sr2) > 0.0) {
          fprintf(stderr, " AUTOPICK SUCCEEDED\n");

          /* set offsets to minimum misfit */
          if (do_vertical) {
            mbna_offset_x = mbna_minmisfit_x;
            mbna_offset_y = mbna_minmisfit_y;
            mbna_offset_z = mbna_minmisfit_z;
          }
          else {
            mbna_offset_x = mbna_minmisfit_xh;
            mbna_offset_y = mbna_minmisfit_yh;
            mbna_offset_z = mbna_minmisfit_zh;
          }

          /* add tie */
          mbnavadjust_naverr_addtie();
        } else {
          fprintf(stderr, " AUTOPICK FAILED\n");
        }

        /* update status periodically */
        if (nprocess % 10 == 0) {
          do_update_status();

          /* update model plot */
          if (project.modelplot) {
                        project.modelplot_uptodate = false;

            /* update model status */
            do_update_modelplot_status();

            /* replot the model */
            mbnavadjust_modelplot_plot(__FILE__, __LINE__);
          }

          /* update visualization */
          if (project.visualization_status)
            do_update_visualization_status();
        }

        /* unload crossing */
        mbnavadjust_crossing_unload();

        fprintf(stderr, "mbna_file_select:%d mbna_survey_select:%d mbna_section_select:%d\n", mbna_file_select,
                mbna_survey_select, mbna_section_select);
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* turn off message dialog */
    do_message_off();

    /* update status */
    do_update_status();

    /* update model plot */
    if (project.modelplot) {
            project.modelplot_uptodate = false;

      /* update model status */
      do_update_modelplot_status();

      /* replot the model */
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    }

    /* update visualization */
    if (project.visualization_status)
      do_update_visualization_status();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_autosetsvsvertical() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_file *file1;
  struct mbna_file *file2;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_matrix matrix;
  bool *x_continuity = NULL;
  double *x_time_d = NULL;
  double *u = NULL;
  double *v = NULL;
  double *w = NULL;
  double *x = NULL;
  double *se = NULL;
  double *b = NULL;
  int *nbxy = NULL;
  int *nbz = NULL;
  double *bxavg = NULL;
  double *byavg = NULL;
  double *bzavg = NULL;
  bool *bpoornav = NULL;
  int *bxfixstatus = NULL;
  int *byfixstatus = NULL;
  int *bzfixstatus = NULL;
  double *bxfix = NULL;
  double *byfix = NULL;
  double *bzfix = NULL;

  int nnav = 0;
  int nblock = 0;
  int ndiscontinuity = 0;
  int nsmooth = 0;
  int ntie = 0;
  int nglobal = 0;
  int nfixed = 0;
  int nrows, ncols;
  int nblockties = 0;
  int nblockglobalties = 0;
  int nrows_ba = 0;
  int ncols_ba = 0;
  int nrows_alloc = 0;
  int ncols_alloc = 0;
  int irow, inav;
  int index_m, index_n;
  int jbvb, jbvb1, jbvb2;

  double damp;
  double atol;
  double btol;
  double relpr;
  double conlim;
  int itnlim;
  int istop_out;
  int itn_out;
  double anorm_out;
  double acond_out;
  double rnorm_out;
  double arnorm_out;
  double xnorm_out;

  int nprocess;
  double offset_z_m;
  double overlap_scale;

  /* set up and solve overdetermined least squares problem for a navigation offset model
   * with constant offsets within each block (survey) using the current
   * navigation ties. Then, loop over all ties between different blocks to
   * auto-repick the crossings using the z-offset in the new model (that is,
   * pick the best lateral offset that can be found using the z-offset in the
   * new model). This should replace the initial ties with ties that have
   * self-consistent z-offsets between surveys. Using this option only makes
   * sense if the bathymetry was correctly tide-corrected before import into
   * mbnavadjust.
   */

  /* check if it is ok to invert
      - if there is a project
      - enough crossings have been analyzed
      - no problems with offsets and offset uncertainties */
  bool ok_to_invert;
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings))

  {
    /* check that all uncertainty magnitudes are nonzero */
    ok_to_invert = true;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigmar1 <= 0.0 || tie->sigmar2 <= 0.0 || tie->sigmar3 <= 0.0) {
            ok_to_invert = false;
            fprintf(stderr,
                    "PROBLEM WITH TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
                    "%8.2f %8.2f %8.2f\n",
                    icrossing, j, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
                    tie->snav_1, project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2,
                    tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1, tie->sigmar2,
                    tie->sigmar3);
          }
        }
      }
    }

    /* print out warning */
    if (!ok_to_invert) {
      fprintf(stderr, "\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
      fprintf(stderr, "Please fix the ties with problems noted above before trying again.\n\n");
    }
  }

  /* invert if there is a project and enough crossings have been analyzed */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      ok_to_invert)

  {
    fprintf(stderr, "\nInverting for navigation adjustment model...\n");

    /* set message dialog on */
    sprintf(message, "Setting up navigation inversion...");
    do_message_on(message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

    /* count number of nav points, discontinuities, and blocks */
    nnav = 0;
    nblock = 0;
    ndiscontinuity = 0;
    nsmooth = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (!section->continuity)
          ndiscontinuity++;
      }
      file->block = nblock - 1;
      file->block_offset_x = 0.0;
      file->block_offset_y = 0.0;
      file->block_offset_z = 0.0;
    }

    /* allocate nav time and continuity arrays */
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&x_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&x_time_d, &error);
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_time_d, 0, nnav * sizeof(double));

    /* loop over all files getting tables of time and continuity */
    inav = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (isnav == 0 && section->continuity) {
            section->snav_invert_id[isnav] = inav - 1;
            nsmooth++;
          }
          else {
            section->snav_invert_id[isnav] = inav;
            if (isnav == 0) {
              x_continuity[inav] = false;
            }
            else {
              x_continuity[inav] = true;
              nsmooth++;
            }
            x_time_d[inav] = section->snav_time_d[isnav];
            inav++;
          }
        }
      }
    }
    nsmooth = 3 * (nsmooth - 1);

    /* count ties for full inversion */
    ntie = 0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XYZ)
            ntie += 3;
          else if (tie->status == MBNA_TIE_XY)
            ntie += 2;
          else if (tie->status == MBNA_TIE_Z)
            ntie += 1;
        }
      }
    }

    /* count dimensions of full inversion problem */
    nglobal = 0;
    nfixed = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      /* get file */
      file = &project.files[ifile];

      /* count fixed and global ties for full inversion */
      for (int isection = 0; isection < file->num_sections; isection++) {
        /* get section */
        section = &file->sections[isection];

        /* count global ties for full inversion */
        if (section->global_tie_status != MBNA_TIE_NONE) {
          if (section->global_tie_status == MBNA_TIE_XYZ)
            nglobal += 3;
          else if (section->global_tie_status == MBNA_TIE_XY)
            nglobal += 2;
          else if (section->global_tie_status == MBNA_TIE_Z)
            nglobal += 1;
        }

        /* count fixed sections for full inversion */
        if (file->status == MBNA_FILE_FIXEDNAV)
          nfixed += 3;
        else if (file->status == MBNA_FILE_FIXEDXYNAV)
          nfixed += 2;
        else if (file->status == MBNA_FILE_FIXEDZNAV)
          nfixed += 1;
      }
    }

    /* only do block average solution if there is more than one block */
    if (nblock > 1) {

      /* allocate block average offset arrays */
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbxy, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbz, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bxavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&byavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bzavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(bool), (void **)&bpoornav, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bxfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&byfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bzfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bxfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&byfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bzfix, &error);
      memset(nbxy, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(nbz, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(bxavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(byavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bzavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bpoornav, 0, nblock * sizeof(bool));
      memset(bxfixstatus, 0, nblock * sizeof(int));
      memset(byfixstatus, 0, nblock * sizeof(int));
      memset(bzfixstatus, 0, nblock * sizeof(int));
      memset(bxfix, 0, nblock * sizeof(double));
      memset(byfix, 0, nblock * sizeof(double));
      memset(bzfix, 0, nblock * sizeof(double));

      /* count ties for all block vs block pairs and calculate average offsets
       * and count dimensions of full inversion problem */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];

        /* for block vs block averages use only set crossings between
         * different blocks */
        if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            /* get tie */
            tie = (struct mbna_tie *)&crossing->ties[itie];

            /* if blocks differ get id for block vs block */
            if (project.files[crossing->file_id_1].block != project.files[crossing->file_id_2].block) {
              if (project.files[crossing->file_id_2].block > project.files[crossing->file_id_1].block) {
                jbvb1 = project.files[crossing->file_id_1].block;
                jbvb2 = project.files[crossing->file_id_2].block;
              }
              else {
                jbvb1 = project.files[crossing->file_id_2].block;
                jbvb2 = project.files[crossing->file_id_1].block;
              }
              jbvb = (jbvb2) * (jbvb2 + 1) / 2 + jbvb1;

              if (tie->status != MBNA_TIE_Z) {
                bxavg[jbvb] += tie->offset_x_m;
                byavg[jbvb] += tie->offset_y_m;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY) {
                bzavg[jbvb] += tie->offset_z_m;
                nbz[jbvb]++;
              }
            }
          }
        }
      }

      /* calculate block vs block tie averages */
      fprintf(stderr, "Survey vs Survey tie counts and average offsets:\n");
      nblockties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            bxavg[jbvb] /= nbxy[jbvb];
            byavg[jbvb] /= nbxy[jbvb];
            nblockties += 2;
          }
          if (nbz[jbvb] > 0) {
            bzavg[jbvb] /= nbz[jbvb];
            nblockties++;
          }
          fprintf(stderr, "%2d vs %2d: %5d xy ties  %5d z ties  Avg offsets: %8.3f %8.3f %8.3f\n", jblock, iblock,
                  nbxy[jbvb], nbz[jbvb], bxavg[jbvb], byavg[jbvb], bzavg[jbvb]);
        }
      }

      /* get fixed blocks and average global ties for blocks */
      mbna_global_tie_influence = 6000;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed and global ties for full inversion */
        for (int isection = 0; isection < file->num_sections; isection++) {
          /* get section */
          section = &file->sections[isection];

          /* count global ties for block offset inversion */
          if (section->global_tie_status != MBNA_TIE_NONE) {
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->offset_x_m;
              byfixstatus[file->block]++;
              byfix[file->block] += section->offset_y_m;
            }
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->offset_z_m;
            }
          }
        }
      }

      /* count fixed sections for block average inversion,
       * overwriting global ties if they conflict */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed sections for block average inversion,
         * overwriting global ties if they conflict */
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
          bxfixstatus[file->block] = 1;
          bxfix[file->block] = 0.0;
          byfixstatus[file->block] = 1;
          byfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
          bzfixstatus[file->block] = 1;
          bzfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_POORNAV) {
          bpoornav[file->block] = true;
        }
      }
      nblockglobalties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          bxfix[iblock] /= (double)bxfixstatus[iblock];
          nblockglobalties++;
        }
        if (byfixstatus[iblock] > 0) {
          byfix[iblock] /= (double)byfixstatus[iblock];
          nblockglobalties++;
        }
        if (bzfixstatus[iblock] > 0) {
          bzfix[iblock] /= (double)bzfixstatus[iblock];
          nblockglobalties++;
        }
      }
    }

    /* We do a two stage inversion first for block averages, then for full
     * adjustement vector on top of the averages. Make sure arrays are
     * allocated large enough for both stages. */
    nrows = nfixed + ntie + nglobal + nsmooth;
    ncols = 3 * nnav;
    nrows_ba = nblockties + nblockglobalties + 3;
    ncols_ba = 3 * nblock;
    nrows_alloc = MAX(nrows, nrows_ba);
    ncols_alloc = MAX(ncols, ncols_ba);
    fprintf(stderr, "\nMBnavadjust block average inversion preparation:\n");
    fprintf(stderr, "     nblock:            %d\n", nblock);
    fprintf(stderr, "     nblockties:        %d\n", nblockties);
    fprintf(stderr, "     nblockglobalties:  %d\n", nblockglobalties);
    fprintf(stderr, "     nrows_ba:          %d\n", nrows_ba);
    fprintf(stderr, "     ncols_ba:          %d\n", ncols_ba);
    fprintf(stderr, "\nMBnavadjust full inversion preparation:\n");
    fprintf(stderr, "     nnav:              %d\n", nnav);
    fprintf(stderr, "     ntie:              %d\n", ntie);
    fprintf(stderr, "     nglobal:           %d\n", nglobal);
    fprintf(stderr, "     nfixed:            %d\n", nfixed);
    fprintf(stderr, "     nsmooth:           %d\n", nsmooth);
    fprintf(stderr, "     nrows:             %d\n", nrows);
    fprintf(stderr, "     ncols:             %d\n", ncols);
    fprintf(stderr, "\nMBnavadjust inversion array allocation dimensions:\n");
    fprintf(stderr, "     nrows_alloc:       %d\n", nrows_alloc);
    fprintf(stderr, "     ncols_alloc:       %d\n", ncols_alloc);

    /* allocate solution vector x, perturbation vector xx, and average solution vector xa */
    matrix.nia = NULL;
    matrix.ia = NULL;
    matrix.a = NULL;
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&u, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&v, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&w, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&x, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&se, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&b, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(int), (void **)&matrix.nia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(int), (void **)&matrix.ia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(double), (void **)&matrix.a, &error);
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
    memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));

    /*----------------------------------------------------------------*/
    /* Create block offset inversion matrix problem                   */
    /*----------------------------------------------------------------*/
    if (nblock > 1) {
      matrix.m = nrows_ba;
      matrix.n = ncols_ba;
      matrix.ia_dim = ncols_ba;

      /* loop over each crossing, applying offsets evenly to both points
          for all ties that involve different blocks
          - weight inversely by number of ties for each block vs block pair
          so that each has same importance whether connected by one tie
          or many */

      /* set up inversion for block offsets
       * - start with average offsets between all block vs block pairs for
       *   x y and z wherever defined by one or more ties
       * - next apply average global ties for each block if they exist
       * - finally add a constraint for x y and z that the sum of all
       *   block offsets must be zero (ignoring blocks tagged as having
       *   poor navigation) */
      irow = 0;

      /* start with average block vs block offsets */
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bxavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;

            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = byavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
          if (nbz[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bzavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
        }
      }

      /* next apply average global offsets for each block */
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = bxfix[jbvb];
          matrix.nia[irow] = 1;
          irow++;
        }
        if (byfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = byfix[jbvb];
          matrix.nia[irow] = 1;
          irow++;
        }
        if (bzfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = bzfix[jbvb];
          matrix.nia[irow] = 1;
          irow++;
        }
      }

      /* add constraint that overall average offset must be zero, ignoring
       * blocks with poor navigation */
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3 + 1;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3 + 2;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;

      // fprintf(stderr,"\nAbout to call LSQR for preliminary block solution   rows: %d cols: %d  (expected rows:%d
      // cols:%d)\n",       irow, nblock * 3, nrows_ba, ncols_ba);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < nrows_ba; irow++)
        u[irow] = b[irow];
      damp = 0.0;
      atol = 1.0e-6;   // releative precision of A matrix
      btol = 1.0e-6;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;

      // fprintf(stderr,"damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      //    damp,atol,btol,conlim,itnlim);
      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    k = i * ncols_ba + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(nrows_ba, ncols_ba, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr,
                  &istop_out, &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

      /* save solution */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        file->block_offset_x = x[3 * file->block];
        file->block_offset_y = x[3 * file->block + 1];
        file->block_offset_z = x[3 * file->block + 2];
      }

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out,
              sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
      for (int i = 0; i < nblock; i++) {
        fprintf(stderr, "block[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n", i, x[3 * i], x[3 * i + 1],
                x[3 * i + 2]);
      }
      // for (int ifile=0;ifile<project.num_files;ifile++)
      //  {
      //  file = &project.files[ifile];
      //  fprintf(stderr,"file[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n",
      //      ifile,file->block_offset_x,file->block_offset_y,file->block_offset_z);
      //  }

      /* deallocate arrays used only for block inversion */
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbxy, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbz, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bpoornav, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_continuity, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_time_d, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&u, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&v, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&se, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&b, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.nia, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.ia, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.a, &error);
    }

    /* loop over over all crossings - reset existing ties using block z-offsets */
    nprocess = 0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      /* set the crossing */
      crossing = &project.crossings[icrossing];
      file1 = &project.files[crossing->file_id_1];
      file2 = &project.files[crossing->file_id_2];
      offset_z_m = file2->block_offset_z - file1->block_offset_z;

      /* check if any ties exist and are inconsistent with the new survey z-offset model
       * - if the zoffset is close enough, then just reset it without repicking the x and y offsets
       * - otherwise repick the tie using the desired z offset */
      bool reset_tie = false;
      for (int itie = 0; itie < crossing->num_ties; itie++) {
        tie = &(crossing->ties[itie]);
        if (fabs(tie->offset_z_m - offset_z_m) < MBNA_Z_OFFSET_RESET_THRESHOLD)
          tie->offset_z_m = offset_z_m;
        else
          reset_tie = true;
      }

      /* if one or more ties exist and at , load crossing and reset the ties using the new z-offset */
      if (reset_tie) {
        mbna_current_crossing = icrossing;
        mbna_file_id_1 = crossing->file_id_1;
        mbna_section_1 = crossing->section_1;
        mbna_file_id_2 = crossing->file_id_2;
        mbna_section_2 = crossing->section_2;
        mbna_current_tie = 0;
        file1 = &project.files[mbna_file_id_1];
        file2 = &project.files[mbna_file_id_2];

        /* set message dialog on */
        sprintf(message, "Loading crossing %d...", mbna_current_crossing);
        fprintf(stderr, "%s: %s\n", __func__, message);
        do_message_update(message);

        /* load crossing */
        mbnavadjust_crossing_load();
        nprocess++;

        /* update status and model plot */
        do_update_status();
        if (project.modelplot) {
          do_update_modelplot_status();
          mbnavadjust_modelplot_plot(__FILE__, __LINE__);
        }

        /* delete each tie */
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          mbnavadjust_deletetie(mbna_current_crossing, itie, MBNA_CROSSING_STATUS_NONE);
        }

        /* update status and model plot */
        do_update_status();
        if (project.modelplot) {
          do_update_modelplot_status();
          mbnavadjust_modelplot_plot(__FILE__, __LINE__);
        }

        /* reset z offset */
        mbna_offset_z = file2->block_offset_z - file1->block_offset_z;

        /* get misfit */
        mbnavadjust_get_misfit();

        /* set offsets to minimum horizontal misfit */
        mbna_offset_x = mbna_minmisfit_xh;
        mbna_offset_y = mbna_minmisfit_yh;
        mbna_offset_z = mbna_minmisfit_zh;
        mbna_misfit_offset_x = mbna_offset_x;
        mbna_misfit_offset_y = mbna_offset_y;
        mbna_misfit_offset_z = mbna_offset_z;
        mbnavadjust_crossing_replot();

        /* get misfit */
        mbnavadjust_get_misfit();

        /* set plot bounds to overlap region */
                mbnavadjust_crossing_overlapbounds(mbna_verbose, &project, mbna_current_crossing, mbna_offset_x, mbna_offset_y,
                                                   &mbna_overlap_lon_min, &mbna_overlap_lon_max,
                                                   &mbna_overlap_lat_min, &mbna_overlap_lat_max,
                                                   &error);
        mbna_plot_lon_min = mbna_overlap_lon_min;
        mbna_plot_lon_max = mbna_overlap_lon_max;
        mbna_plot_lat_min = mbna_overlap_lat_min;
        mbna_plot_lat_max = mbna_overlap_lat_max;
        overlap_scale = MIN((mbna_overlap_lon_max - mbna_overlap_lon_min) / mbna_mtodeglon,
                            (mbna_overlap_lat_max - mbna_overlap_lat_min) / mbna_mtodeglat);

        /* get naverr plot scaling */
        mbnavadjust_naverr_scale();

        /* get misfit */
        mbnavadjust_get_misfit();

        /* check uncertainty estimate for a good pick */
        if (MAX(mbna_minmisfit_sr1, mbna_minmisfit_sr2) < 0.5 * overlap_scale &&
            MIN(mbna_minmisfit_sr1, mbna_minmisfit_sr2) > 0.0) {

          /* set offsets to minimum horizontal misfit */
          mbna_offset_x = mbna_minmisfit_xh;
          mbna_offset_y = mbna_minmisfit_yh;
          mbna_offset_z = mbna_minmisfit_zh;

          /* add tie */
          mbnavadjust_naverr_addtie();
fprintf(stderr,"Done adding tie\n");
        }
        else {
          sprintf(message, "Failed to reset Tie Point %d of Crossing %d\n", 0, mbna_current_crossing);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
          do_info_add(message, true);
        }
fprintf(stderr,"nprocess:%d\n",nprocess);

        /* update status periodically */
        if (nprocess % 10 == 0) {
          do_update_status();

          /* update model plot */
          if (project.modelplot) {
                        project.modelplot_uptodate = false;

            /* update model status */
            do_update_modelplot_status();

            /* replot the model */
            mbnavadjust_modelplot_plot(__FILE__, __LINE__);
          }

          /* update visualization */
          if (project.visualization_status)
            do_update_visualization_status();
        }

        /* unload crossing */
fprintf(stderr,"calling mbnavadjust_crossing_unload\n");
        mbnavadjust_crossing_unload();
fprintf(stderr,"done with mbnavadjust_crossing_unload\n");

fprintf(stderr,"mbna_file_select:%d mbna_survey_select:%d mbna_section_select:%d\n",
mbna_file_select,mbna_survey_select,mbna_section_select);
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* turn off message dialog */
    do_message_off();

    /* update status */
    do_update_status();

    /* update model plot */
    if (project.modelplot) {
            project.modelplot_uptodate = false;

      /* update model status */
      do_update_modelplot_status();

      /* replot the model */
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);
    }

    /* update visualization */
    if (project.visualization_status)
      do_update_visualization_status();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbnavadjust_zerozoffsets() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* loop over all crossings */
  if (project.open && project.num_crossings > 0) {
    /* set message dialog on */
    sprintf(message, "Zeroing all z offsets...");
    do_message_on(message);
    sprintf(message, "Zeroing all z offsets.\n");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
    do_info_add(message, true);

    /* loop over all crossings */
    for (int i = 0; i < project.num_crossings; i++) {
      /* get structure */
      crossing = &(project.crossings[i]);

      /* if one or more ties exist and crossing is in the current view then zero the z-offsets */
      if (crossing->num_ties > 0) {
        bool process = false;
        if (mbna_view_list == MBNA_VIEW_LIST_CROSSINGS) {
          if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
              (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
               mbna_survey_select == project.files[crossing->file_id_1].block &&
               mbna_survey_select == project.files[crossing->file_id_2].block) ||
              (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
               mbna_file_select == crossing->file_id_2) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
               (mbna_survey_select == project.files[crossing->file_id_1].block ||
                mbna_survey_select == project.files[crossing->file_id_2].block)) ||
              (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
               ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                 mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                 mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
               (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
               mbna_section_select == crossing->section_1) ||
              (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
               mbna_section_select == crossing->section_2)) {
            process = true;
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_MEDIOCRECROSSINGS) {
          if (crossing->overlap >= MBNA_MEDIOCREOVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2)) {
              process = true;
            }
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_GOODCROSSINGS) {
          if (crossing->overlap >= MBNA_GOODOVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2)) {
              process = true;
            }
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_BETTERCROSSINGS) {
          if (crossing->overlap >= MBNA_BETTEROVERLAP_THRESHOLD) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2)) {
              process = true;
            }
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_TRUECROSSINGS) {
          if (crossing->truecrossing) {
            if ((mbna_view_mode == MBNA_VIEW_MODE_ALL) ||
                (mbna_view_mode == MBNA_VIEW_MODE_SURVEY &&
                 mbna_survey_select == project.files[crossing->file_id_1].block &&
                 mbna_survey_select == project.files[crossing->file_id_2].block) ||
                (mbna_view_mode == MBNA_VIEW_MODE_FILE && mbna_file_select == crossing->file_id_1 &&
                 mbna_file_select == crossing->file_id_2) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY &&
                 (mbna_survey_select == project.files[crossing->file_id_1].block ||
                  mbna_survey_select == project.files[crossing->file_id_2].block)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_BLOCK &&
                 ((mbna_block_select1 == project.files[crossing->file_id_1].block &&
                   mbna_block_select2 == project.files[crossing->file_id_2].block) ||
                  (mbna_block_select2 == project.files[crossing->file_id_1].block &&
                   mbna_block_select1 == project.files[crossing->file_id_2].block))) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE &&
                 (mbna_file_select == crossing->file_id_1 || mbna_file_select == crossing->file_id_2)) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_1 &&
                 mbna_section_select == crossing->section_1) ||
                (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION && mbna_file_select == crossing->file_id_2 &&
                 mbna_section_select == crossing->section_2)) {
              process = true;
            }
          }
        }
        else if (mbna_view_list == MBNA_VIEW_LIST_TIES || mbna_view_list == MBNA_VIEW_LIST_TIESSORTED) {
          process = true;
        }
        else {
          process = false;
        }

        /* zero the z offset if process true */
        if (process) {
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &(crossing->ties[j]);

            if (tie->offset_z_m != 0.0) {
              tie->offset_z_m = 0.0;

              /* set inversion out of date */
              if (project.inversion_status == MBNA_INVERSION_CURRENT)
                project.inversion_status = MBNA_INVERSION_OLD;
            }
          }

        }
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* turn off message dialog */
    do_message_off();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
void mb_aprod(int mode, int m, int n, double x[], double y[], void *UsrWrk) {
  (void)n;  // Unused parameter
  // mode == 1 : compute y = y + A*x
  // mode == 2 : compute x = x + A(transpose)*y
  struct mbna_matrix *matrix;
  int k;

  matrix = (struct mbna_matrix *)UsrWrk;

  if (mode == 1) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < matrix->nia[i]; j++) {
        k = matrix->ia[matrix->ia_dim * i + j];
        y[i] += matrix->a[matrix->ia_dim * i + j] * x[k];
      }
    }
  }

  else if (mode == 2) {
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < matrix->nia[i]; j++) {
        k = matrix->ia[matrix->ia_dim * i + j];
        x[k] += matrix->a[matrix->ia_dim * i + j] * y[i];
      }
    }
  }
}
/*--------------------------------------------------------------------*/

int mbnavadjust_invertnav() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_file *file1;
  struct mbna_file *file2;
  struct mbna_section *section;
  struct mbna_section *section1;
  struct mbna_section *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_matrix matrix;
  bool *x_continuity = NULL;
  int *x_quality = NULL;
  int *x_num_ties = NULL;
  int *x_chunk = NULL;
  double *x_time_d = NULL;
  int *chunk_center = NULL;
  bool *chunk_continuity = NULL;
  double *u = NULL;
  double *v = NULL;
  double *w = NULL;
  double *x = NULL;
  int *nx = NULL;
  double *se = NULL;
  double *b = NULL;
  int *nbxy = NULL;
  int *nbz = NULL;
  double *bxavg = NULL;
  double *byavg = NULL;
  double *bzavg = NULL;
  bool *bpoornav = NULL;
  int *bxfixstatus = NULL;
  int *byfixstatus = NULL;
  int *bzfixstatus = NULL;
  double *bxfix = NULL;
  double *byfix = NULL;
  double *bzfix = NULL;
  double matrix_scale = 1000.0;
  double rms_solution, rms_solution_total, rms_misfit_initial, rms_misfit_previous, rms_misfit_current;
  int nrms;

  int nnav = 0;
  int nblock = 0;
  int ndiscontinuity = 0;
  int nsmooth = 0;
  int nnsmooth = 0;
  int ntie = 0;
  int nglobal = 0;
  int nfixed = 0;
  int nrows, ncols;
  int nblockties = 0;
  int nblockglobalties = 0;
  int nrows_ba = 0;
  int ncols_ba = 0;
  int nrows_alloc = 0;
  int ncols_alloc = 0;

  int nchunk, nchunk_start;
  double distance_sum, chunk_distance;
  double damping;

  int n_iteration;
  double convergence;
  double convergence_threshold;
  double offset_x, offset_y, offset_z, projected_offset;
  double weight, zweight;
  double smooth_exp;
  double smoothweight;
  bool ok_to_invert;
  bool found;
  double factor;
  int itielast, itienext;
  double damp;
  double atol;
  double btol;
  double relpr;
  double conlim;
  int itnlim;
  int istop_out;
  int itn_out;
  double anorm_out;
  double acond_out;
  double rnorm_out;
  double arnorm_out;
  double xnorm_out;

  /* check if it is ok to invert
      - if there is a project
      - enough crossings have been analyzed
      - no problems with offsets and offset uncertainties */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings))

  {
    /* check that all uncertainty magnitudes are nonzero */
    ok_to_invert = true;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigmar1 <= 0.0 || tie->sigmar2 <= 0.0 || tie->sigmar3 <= 0.0) {
            ok_to_invert = false;
            fprintf(stderr,
                    "PROBLEM WITH TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
                    "%8.2f %8.2f %8.2f\n",
                    icrossing, j, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
                    tie->snav_1, project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2,
                    tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1, tie->sigmar2,
                    tie->sigmar3);
          }
        }
      }
    }

    /* print out warning */
    if (!ok_to_invert) {
      fprintf(stderr, "\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
      fprintf(stderr, "Please fix the ties with problems noted above before trying again.\n\n");
    }
  }

  /* invert if there is a project and enough crossings have been analyzed */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      ok_to_invert)

  {
    fprintf(stderr, "\nInverting for navigation adjustment model...\n");

    /* set message dialog on */
    sprintf(message, "Setting up navigation inversion...");
    do_message_on(message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

        /* zero solution across all navigation */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          section->snav_lon_offset[isnav] = 0.0;
          section->snav_lat_offset[isnav] = 0.0;
          section->snav_z_offset[isnav] = 0.0;
        }
      }
    }

    /* count number of nav points, discontinuities, and blocks */
    nnav = 0;
    nblock = 0;
    ndiscontinuity = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (!section->continuity)
          ndiscontinuity++;
      }
      file->block = nblock - 1;
      file->block_offset_x = 0.0;
      file->block_offset_y = 0.0;
      file->block_offset_z = 0.0;
    }

    /* allocate nav time and continuity arrays */
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&x_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_quality, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_num_ties, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_chunk, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&x_time_d, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&chunk_center, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&chunk_continuity, &error);
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_quality, 0, nnav * sizeof(int));
    memset(x_num_ties, 0, nnav * sizeof(int));
    memset(x_chunk, 0, nnav * sizeof(int));
    memset(x_time_d, 0, nnav * sizeof(double));
    memset(chunk_center, 0, nnav * sizeof(int));
    memset(chunk_continuity, 0, nnav * sizeof(bool));

    /* loop over all files getting tables of time and continuity */
    int inav = 0;
    nchunk = 0;
    nchunk_start = 0;
    chunk_distance = 25 * project.section_length;
    distance_sum = 0.0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      chunk_distance = 10 * file->sections[0].distance;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (isnav == 0 && section->continuity) {
            section->snav_invert_id[isnav] = inav - 1;
          }
          else {
            section->snav_invert_id[isnav] = inav;
            if (isnav == 0) {
              x_continuity[inav] = false;
              distance_sum = 0.0;
            }
            else {
              x_continuity[inav] = true;
            }
            x_time_d[inav] = section->snav_time_d[isnav];
            x_quality[inav] = file->status;
            x_num_ties[inav] = section->snav_num_ties[isnav];
            distance_sum += section->snav_distance[isnav];
            if ((!x_continuity[inav] && inav > 0) || distance_sum > chunk_distance) {
                chunk_center[nchunk] = (nchunk_start + inav - 1) / 2;
//fprintf(stderr, "---chunk_center[%d]: %d\n", nchunk, chunk_center[nchunk]);
                nchunk++;
                chunk_continuity[nchunk] = x_continuity[inav];
                nchunk_start = inav;
                distance_sum = 0.0;
            }
            x_chunk[inav] = nchunk;
//fprintf(stderr,"inav:%d   %2.2d:%3.3d:%3.3d:%2.2d distance:  %f %f %f  chunk:%d:%d continuity:%d\n",
//        inav, file->block, ifile, isection, isnav, section->snav_distance[isnav], distance_sum, chunk_distance,
//        nchunk,x_chunk[inav],chunk_continuity[nchunk]);
            inav++;
          }
        }
      }
    }
    nchunk++;

    /* count first derivative smoothing points */
    nsmooth = 0;
    for (int inav = 0; inav < nnav - 1; inav++) {
      if (x_continuity[inav + 1]) {
        nsmooth++;
      }
    }
    nsmooth = 3 * nsmooth;

    /* count second derivative smoothing points */
    /*nsmooth = 0;
    for (int inav = 0; inav < nnav - 2; inav++) {
        if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
            nsmooth++;
        }
    }
    nsmooth = 3 * nsmooth;*/

    /* get dimensions of inversion problem and initial misfit */
    ntie = 0;
    nrms = 0;
    nglobal = 0;
    nfixed = 0;
    rms_misfit_initial = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (tie->offset_x_m * tie->offset_x_m) + (tie->offset_y_m * tie->offset_y_m);
            nrms += 2;
            //ntie += 2;
          }
          if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (tie->offset_z_m * tie->offset_z_m);
            nrms += 1;
            //ntie += 1;
          }
                    ntie += 3;
        }
      }
    }
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      /* get file */
      file = &project.files[ifile];

      /* count fixed and global ties for full inversion */
      for (int isection = 0; isection < file->num_sections; isection++) {
        /* get section */
        section = &file->sections[isection];

        /* count global ties for full inversion */
        if (section->global_tie_status != MBNA_TIE_NONE) {
          if (section->global_tie_status == MBNA_TIE_XY || section->global_tie_status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (section->offset_x_m * section->offset_x_m) + (section->offset_y_m * section->offset_y_m);
            nrms += 2;
            nglobal += 2;
          }
          if (section->global_tie_status == MBNA_TIE_Z || section->global_tie_status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (section->offset_z_m * section->offset_z_m);
            nrms += 1;
            nglobal += 1;
          }
        }

        /* count fixed sections for full inversion */
        if (file->status == MBNA_FILE_FIXEDNAV)
          nfixed += 3 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDXYNAV)
          nfixed += 2 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDZNAV)
          nfixed += 1 * section->num_snav;
      }
    }
        if (nrms > 0) {
            rms_misfit_initial /= nrms;
            rms_misfit_previous = rms_misfit_initial;
            rms_misfit_current = rms_misfit_initial;
        }

    /* only do block average solution if there is more than one block */
    if (nblock > 1) {

      /* allocate block average offset arrays */
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbxy, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbz, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bxavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&byavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bzavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(bool), (void **)&bpoornav, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bxfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&byfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bzfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bxfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&byfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bzfix, &error);
      memset(nbxy, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(nbz, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(bxavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(byavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bzavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bpoornav, 0, nblock * sizeof(bool));
      memset(bxfixstatus, 0, nblock * sizeof(int));
      memset(byfixstatus, 0, nblock * sizeof(int));
      memset(bzfixstatus, 0, nblock * sizeof(int));
      memset(bxfix, 0, nblock * sizeof(double));
      memset(byfix, 0, nblock * sizeof(double));
      memset(bzfix, 0, nblock * sizeof(double));

      /* count ties for all block vs block pairs and calculate average offsets
       * and count dimensions of full inversion problem */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];

        /* for block vs block averages use only set crossings between
         * different blocks */
        if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          int jbvb1 = 0;
          int jbvb2 = 0;
          int jbvb = 0;
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            /* get tie */
            tie = (struct mbna_tie *)&crossing->ties[itie];

            /* if blocks differ get id for block vs block */
            if (project.files[crossing->file_id_1].block != project.files[crossing->file_id_2].block) {
              if (project.files[crossing->file_id_2].block > project.files[crossing->file_id_1].block) {
                jbvb1 = project.files[crossing->file_id_1].block;
                jbvb2 = project.files[crossing->file_id_2].block;
              }
              else {
                jbvb1 = project.files[crossing->file_id_2].block;
                jbvb2 = project.files[crossing->file_id_1].block;
              }
              jbvb = (jbvb2) * (jbvb2 + 1) / 2 + jbvb1;

              if (tie->status != MBNA_TIE_Z) {
                bxavg[jbvb] += tie->offset_x_m;
                byavg[jbvb] += tie->offset_y_m;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY) {
                bzavg[jbvb] += tie->offset_z_m;
                nbz[jbvb]++;
              }
            }
          }
        }
      }

      /* calculate block vs block tie averages */
      fprintf(stderr, "Survey vs Survey tie counts and average offsets:\n");
      nblockties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            bxavg[jbvb] /= nbxy[jbvb];
            byavg[jbvb] /= nbxy[jbvb];
            nblockties += 2;
          }
          if (nbz[jbvb] > 0) {
            bzavg[jbvb] /= nbz[jbvb];
            nblockties++;
          }
          fprintf(stderr, "%2d vs %2d: %5d xy ties  %5d z ties  Avg offsets: %8.3f %8.3f %8.3f\n", jblock, iblock,
                  nbxy[jbvb], nbz[jbvb], bxavg[jbvb], byavg[jbvb], bzavg[jbvb]);
        }
      }

      /* get fixed blocks and average global ties for blocks */
      mbna_global_tie_influence = 6000;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed and global ties for full inversion */
        for (int isection = 0; isection < file->num_sections; isection++) {
          /* get section */
          section = &file->sections[isection];

          /* count global ties for block offset inversion */
          if (section->global_tie_status != MBNA_TIE_NONE) {
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->offset_x_m;
              byfixstatus[file->block]++;
              byfix[file->block] += section->offset_y_m;
            }
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->offset_z_m;
            }
          }
        }
      }

      /* count fixed sections for block average inversion,
       * overwriting global ties if they conflict */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed sections for block average inversion,
         * overwriting global ties if they conflict */
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
          bxfixstatus[file->block] = 1;
          bxfix[file->block] = 0.0;
          byfixstatus[file->block] = 1;
          byfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
          bzfixstatus[file->block] = 1;
          bzfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_POORNAV) {
          bpoornav[file->block] = true;
        }
      }
      nblockglobalties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          bxfix[iblock] /= (double)bxfixstatus[iblock];
          nblockglobalties++;
        }
        if (byfixstatus[iblock] > 0) {
          byfix[iblock] /= (double)byfixstatus[iblock];
          nblockglobalties++;
        }
        if (bzfixstatus[iblock] > 0) {
          bzfix[iblock] /= (double)bzfixstatus[iblock];
          nblockglobalties++;
        }
      }
    }

    /* We do a three stage inversion first for block averages, then a slow relaxation
     * towards a coarse solution, and finally an overdetermined least squares
     * solution for an additional perturbation to satisfy the remaining signal.
     * Make sure arrays are allocated large enough for both stages. */
    nrows = nfixed + ntie + nglobal + nsmooth;
    ncols = 3 * nnav;
    nrows_ba = nblockties + nblockglobalties + 3;
    ncols_ba = 3 * nblock;
    nrows_alloc = MAX(nrows, nrows_ba);
    ncols_alloc = MAX(ncols, ncols_ba);
    fprintf(stderr, "\nMBnavadjust block average inversion preparation:\n");
    fprintf(stderr, "     nblock:            %d\n", nblock);
    fprintf(stderr, "     nblockties:        %d\n", nblockties);
    fprintf(stderr, "     nblockglobalties:  %d\n", nblockglobalties);
    fprintf(stderr, "     nrows_ba:          %d\n", nrows_ba);
    fprintf(stderr, "     ncols_ba:          %d\n", ncols_ba);
    fprintf(stderr, "\nMBnavadjust full inversion preparation:\n");
    fprintf(stderr, "     nnav:              %d\n", nnav);
    fprintf(stderr, "     ntie:              %d\n", ntie);
    fprintf(stderr, "     nglobal:           %d\n", nglobal);
    fprintf(stderr, "     nfixed:            %d\n", nfixed);
    fprintf(stderr, "     nsmooth:           %d\n", nsmooth);
    fprintf(stderr, "     nrows:             %d\n", nrows);
    fprintf(stderr, "     ncols:             %d\n", ncols);
    fprintf(stderr, "\nMBnavadjust inversion array allocation dimensions:\n");
    fprintf(stderr, "     nrows_alloc:       %d\n", nrows_alloc);
    fprintf(stderr, "     ncols_alloc:       %d\n", ncols_alloc);

    /* allocate solution vector x, perturbation vector xx, and average solution vector xa */
    matrix.nia = NULL;
    matrix.ia = NULL;
    matrix.a = NULL;
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&u, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&v, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&w, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&x, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(int), (void **)&nx, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&se, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&b, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(int), (void **)&matrix.nia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(int), (void **)&matrix.ia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(double), (void **)&matrix.a, &error);
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(nx, 0, ncols_alloc * sizeof(int));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
    memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));

    /*----------------------------------------------------------------*/
    /* Create block offset inversion matrix problem                   */
    /*----------------------------------------------------------------*/
    if (nblock > 1) {
      matrix.m = nrows_ba;
      matrix.n = ncols_ba;
      matrix.ia_dim = ncols_ba;

      /* loop over each crossing, applying offsets evenly to both points
          for all ties that involve different blocks
          - weight inversely by number of ties for each block vs block pair
          so that each has same importance whether connected by one tie
          or many */

      /* set up inversion for block offsets
       * - start with average offsets between all block vs block pairs for
       *   x y and z wherever defined by one or more ties
       * - next apply average global ties for each block if they exist
       * - finally add a constraint for x y and z that the sum of all
       *   block offsets must be zero (ignoring blocks tagged as having
       *   poor navigation) */
      int irow = 0;

      /* start with average block vs block offsets */
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int index_m;
          int index_n;
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bxavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;

            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = byavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
          if (nbz[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bzavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
        }
      }

      /* next apply average global offsets for each block */
      mbna_global_tie_influence = 6000.0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m;
        int index_n;
        if (bxfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bxfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix X block %d to %f\n", iblock, bxfix[iblock]);
        }
        if (byfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * byfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix Y block %d to %f\n", iblock, byfix[iblock]);
        }
        if (bzfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bzfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
//fprintf(stderr, "Fix Z block %d to %f\n", iblock, bzfix[iblock]);
        }
      }

      /* add constraint that overall average offset must be zero, ignoring
       * blocks with poor navigation */
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 1;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 2;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;

      fprintf(stderr,
              "\nAbout to call LSQR for preliminary block solution   rows: %d cols: %d  (expected rows:%d cols:%d)\n", irow,
              nblock * 3, nrows_ba, ncols_ba);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < nrows_ba; irow++)
        u[irow] = b[irow];
      damp = 0.0;
      atol = 1.0e-6;   // releative precision of A matrix
      btol = 1.0e-6;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;
      // fprintf(stderr,"damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      //    damp,atol,btol,conlim,itnlim);

      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    int k = i * ncols_ba + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(nrows_ba, ncols_ba, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr,
                  &istop_out, &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

        /* save solution */
        double rms_solution = 0.0;
        double rms_solution_total = 0.0;
        int nrms = 0;
        for (int ifile = 0; ifile < project.num_files; ifile++) {
            file = &project.files[ifile];
        file->block_offset_x = x[3 * file->block];
        file->block_offset_y = x[3 * file->block + 1];
        file->block_offset_z = x[3 * file->block + 2];
        for (int isection = 0; isection < file->num_sections; isection++) {
            section = &file->sections[isection];
            for (int isnav = 0; isnav < section->num_snav; isnav++) {
                section->snav_lon_offset[isnav] = file->block_offset_x * project.mtodeglon;
                section->snav_lat_offset[isnav] = file->block_offset_y * project.mtodeglat;
                section->snav_z_offset[isnav] = file->block_offset_z;
                rms_solution += file->block_offset_x * file->block_offset_x;
                rms_solution += file->block_offset_y * file->block_offset_y;
                rms_solution += file->block_offset_z * file->block_offset_z;
                nrms += 3;
            }
        }
    }
    if (nrms > 0) {
        rms_solution = sqrt(rms_solution);
        rms_solution_total = rms_solution;
    }

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out,
              sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
      for (int i = 0; i < nblock; i++) {
        fprintf(stderr, "block[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n", i, x[3 * i], x[3 * i + 1],
                x[3 * i + 2]);
      }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
            }

            fprintf(stderr, "\nBlock inversion:\n > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n",
                    rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current);

      /* deallocate arrays used only for block inversion */
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbxy, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbz, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bpoornav, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfix, &error);
    }

        /* stage 2 - iteratively relax towards a coarse offset model in which
         * nav specified as poor is downweighted relative to good nav. The
         * nav offsets of this coarse model will be added to the block offsets
         * and the total removed from the tie offsets used in the final inversion.
         * The approach is to use the least squares inversion to solve for zero
         * mean, Gaussian distributed offsets rather than the large scale offsets
         * and drift.
         * The coarseness is to solve for a navigation offset that is large scale
         * using a coarseness defined as 10 times the section length (which is
         * taken from the first section of the current file, as it can vary amongst
         * surveys in a project).
         */

        /* loop over all ties applying the offsets to the chunks partitioned according to survey quality */
        n_iteration = 10000;
        convergence = 1000.0;
        convergence_threshold = 0.0001;
        damping = 0.001;
        for (int iteration=0; iteration < n_iteration && convergence > convergence_threshold; iteration ++) {
            fprintf(stderr,"\nStage 2 relaxation iteration %d\n", iteration);

            /* zero the working average offset array */
            memset(x, 0, ncols_alloc * sizeof(double));
            memset(nx, 0, ncols_alloc * sizeof(int));
            rms_misfit_previous = 0.0;
            nrms = 0;

            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];

                /* apply crossing ties */
                if (crossing->status == MBNA_CROSSING_STATUS_SET) {
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        int k1 = x_chunk[section1->snav_invert_id[tie->snav_1]];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        int k2 = x_chunk[section2->snav_invert_id[tie->snav_2]];

                        /* count tie impact on chunks */
                        nx[k1]++;
                        nx[k2]++;

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m
                                        - (section2->snav_lon_offset[tie->snav_2]
                                            - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m
                                        - (section2->snav_lat_offset[tie->snav_2]
                                            - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;

                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        else {
                            offset_x = 0.0;
                            offset_y = 0.0;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m
                                        - (section2->snav_z_offset[tie->snav_2]
                                            - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                        }
                        else {
                            offset_z = 0.0;
                        }

                        /* apply offsets to relevant chunks partitioned according to
                         * relative survey quality */
                        if (file1->status == MBNA_FILE_GOODNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_POORNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_FIXEDNAV
                                || file2->status == MBNA_FILE_FIXEDXYNAV
                                || file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDXYNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDZNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                    }
                }
            }

            /* apply global ties */
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];

                    /* get absolute id for snav point */
                    int k = x_chunk[section->snav_invert_id[section->global_tie_snav]];

                    /* count global tie impact on chunks */
                    nx[k]++;

                    /* get and apply offset vector for this tie */
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x = section->offset_x_m
                            - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y = section->offset_y_m
                            - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                        x[3*k]   += -offset_x;
                        x[3*k+1] += -offset_y;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m
                            - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_previous += offset_z * offset_z;
                        nrms += 1;
                        x[3*k+2] += -offset_z;
                    }
                }
            }

            /* linearly interpolate over gaps between impacted chunks */
            int klast = 0;
            for (int k=0; k < nchunk; k++) {
              if (nx[k] > 0) {
                if (k - klast > 1) {
                  if (chunk_continuity[klast+1] && chunk_continuity[k]) {
                    double factor0 = (x[3*k]   - x[3*klast])   / ((double)(k - klast));
                    double factor1 = (x[3*k+1] - x[3*klast+1]) / ((double)(k - klast));
                    double factor2 = (x[3*k+2] - x[3*klast+2]) / ((double)(k - klast));
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*klast]   + factor0 * ((double)(kk - klast));
                      x[3*kk+1] = x[3*klast+1] + factor1 * ((double)(kk - klast));
                      x[3*kk+2] = x[3*klast+2] + factor2 * ((double)(kk - klast));
                    }
                  }
                  else if (chunk_continuity[klast+1]) {
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*klast];
                      x[3*kk+1] = x[3*klast+1];
                      x[3*kk+2] = x[3*klast+2];
                    }
                  }
                  else if (chunk_continuity[k]) {
                    for (int kk=klast+1; kk<k; kk++) {
                      x[3*kk]   = x[3*k];
                      x[3*kk+1] = x[3*k+1];
                      x[3*kk+2] = x[3*k+2];
                    }
                  }
                }
                klast = k;
              }
            }

            /* apply damping to solution vector */
            for (int k=0; k<3 * nchunk; k++) {
                x[k] *= damping;
            }

            /* penalize change between continuous chunks using the w work array */
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                w[3*k] = x[3*k] - x[3*(k-1)];
                w[3*k+1] = x[3*k+1] - x[3*(k-1)+1];
                w[3*k+2] = x[3*k+2] - x[3*(k-1)+2];
              }
            }
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                x[3*(k-1)] += 10.0 * damping * 0.5 * w[3*k];
                x[3*(k-1)+1] += 10.0 * damping * 0.5 * w[3*k+1];
                x[3*(k-1)+2] += 10.0 * damping * 0.5 * w[3*k+2];
                x[3*k] -= 10.0 * damping * 0.5 * w[3*k];
                x[3*k+1] -= 10.0 * damping * 0.5 * w[3*k+1];
                x[3*k+2] -= 10.0 * damping * 0.5 * w[3*k+2];
              }
            }

            /* get previous misfit measure */
            rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

            /* add average offsets back into the model */
            rms_solution = 0.0;
            rms_solution_total = 0.0;
            nrms = 0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    for (int isnav = 0; isnav < section->num_snav; isnav++) {
                        inav = section->snav_invert_id[isnav];
                        int k = x_chunk[inav];
                        if (inav == chunk_center[k]
                            || (k == 0 && inav <= chunk_center[k])
                            ||  (k == nchunk - 1 && inav >= chunk_center[k])) {
                            offset_x = x[3 * k];
                            offset_y = x[3 * k + 1];
                            offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                        }
                        else if  (inav <= chunk_center[k]) {
                            if (chunk_continuity[k]) {
                                factor = ((double)(inav - chunk_center[k-1])) / ((double)(chunk_center[k] - chunk_center[k-1]));
                                offset_x = x[3 * (k - 1)] + factor * (x[3 * k] - x[3 * (k - 1)]);
                                offset_y = x[3 * (k - 1) + 1] + factor * (x[3 * k + 1] - x[3 * (k - 1) + 1]);
                                offset_z = x[3 * (k - 1) + 2] + factor * (x[3 * k + 2] - x[3 * (k - 1) + 2]);
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            } else {
                                offset_x = x[3 * k];
                                offset_y = x[3 * k + 1];
                                offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            }
                        }
                        else if (inav >= chunk_center[k]) {
                            if (chunk_continuity[k+1]) {
                                factor = ((double)(inav - chunk_center[k])) / ((double)(chunk_center[k+1] - chunk_center[k]));
                                offset_x = x[3 * k] + factor * (x[3 * (k + 1)] - x[3 * k]);
                                offset_y = x[3 * k + 1] + factor * (x[3 * (k + 1) + 1] - x[3 * k + 1]);
                                offset_z = x[3 * k + 2] + factor * (x[3 * (k + 1) + 2] - x[3 * k + 2]);
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            } else {
                                offset_x = x[3 * k];
                                offset_y = x[3 * k + 1];
                                offset_z = x[3 * k + 2];
//fprintf(stderr,"%s:%d:%s inav:%d k:%d offsets: %f %f %f\n", __FILE__, __LINE__, __func__, inav, k, offset_x, offset_y, offset_z);
                            }
                        }
//fprintf(stderr,"inav:%d %2.2d:%4.4d:%2.2d:%2.2d chunk:%d of %d cont:%d offsets:%f %f %f\n",
//inav,file->block, ifile, isection, isnav, k,nchunk,chunk_continuity[k],offset_x,offset_y,offset_z);
                        section->snav_lon_offset[isnav] += offset_x * project.mtodeglon;
                        section->snav_lat_offset[isnav] += offset_y * project.mtodeglat;
                        section->snav_z_offset[isnav] += offset_z;
                        rms_solution += offset_x * offset_x;
                        rms_solution += offset_y * offset_y;
                        rms_solution += offset_z * offset_z;
                        rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                        rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                        rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                        nrms += 3;
                    }
                }
            }
            if (nrms > 0) {
                rms_solution = sqrt(rms_solution);
                rms_solution_total = sqrt(rms_solution_total);
            }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                // int nc1;
                // int nc2;
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // const int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // const int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = fabs(rms_misfit_previous - rms_misfit_current) / rms_misfit_previous;
            }

            fprintf(stderr, " > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);
        } // iteration

    /* set message dialog on */
    sprintf(message, "Completed chunk inversion...");
    do_message_update(message);

    /*-------------------------------------------------------------------------*/
    /* Create complete inversion matrix problem to solve with LSQR             */
    /* - this is solving for the perturbation in addition to the model         */
    /* already constructed by the block inversion and then the chunk           */
    /* relaxation. If n_iteration == 1 then do this inversion once with        */
    /* smoothing defined by project.smoothing.                                 */
    /* If n_iteration is set > 1 then do this inversion multiple               */
    /* times using different smoothing parameters, starting large              */
    /* and getting smaller with the final smoothing == project.smoothing.      */
    /*-------------------------------------------------------------------------*/

        matrix_scale = 1000.0;
        n_iteration = 1;  // tested with n_iteration = 5, found same result just doing one iteration
        const int n_iteration_tot = (n_iteration == 1 ? n_iteration : 2 * n_iteration);
        const double smooth_max = 2.0 * project.smoothing;
        const double d_smooth = (smooth_max - project.smoothing) / n_iteration;
        convergence = 1000.0;
        convergence_threshold = 0.01;
        for (int iteration=0;
              (iteration < n_iteration && convergence > 0.0)
                || (iteration < n_iteration_tot && convergence > convergence_threshold);
              iteration ++) {

            /* set message dialog on */
            sprintf(message, "Performing navigation inversion iteration %d of %d...", iteration +1, n_iteration);
            do_message_on(message);

            if (n_iteration == 1) {
              smooth_exp = project.smoothing;
            } else {
              smooth_exp = MAX((smooth_max - iteration * d_smooth), project.smoothing);
            }
            smoothweight = pow(10.0, smooth_exp) / 100.0;
            fprintf(stderr, "\n----------\n\nPreparing inversion iteration %d of %d with smoothing %f ==> %f\n\t\trows: %d %d  cols: %d %d\n",
                    iteration, n_iteration_tot, smooth_exp,
                    smoothweight, matrix.m, nrows, matrix.n, ncols);

            /* loop over each crossing, applying offsets evenly to both points */
            int irow = 0;
            nrms = 0;
            rms_misfit_previous = 0.0;
            matrix.m = nrows;
            matrix.n = ncols;
            matrix.ia_dim = 6;
            memset(u, 0, nrows_alloc * sizeof(double));
            memset(v, 0, ncols_alloc * sizeof(double));
            memset(w, 0, ncols_alloc * sizeof(double));
            memset(x, 0, ncols_alloc * sizeof(double));
            memset(se, 0, ncols_alloc * sizeof(double));
            memset(b, 0, nrows_alloc * sizeof(double));
            memset(matrix.nia, 0, nrows_alloc * sizeof(int));
            memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
            memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                int nc1;
                int nc2;

                /* use only set crossings */
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* A: get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];
                        int index_m;
                        int index_n;

                        /* A1: get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        nc1 = section1->snav_invert_id[tie->snav_1];

                        /* A2: get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        nc2 = section2->snav_invert_id[tie->snav_2];

                        if (section1->snav_time_d[tie->snav_1] ==
                            section2->snav_time_d[tie->snav_2])
                            fprintf(stderr, "ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
                                    crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2, crossing->section_2,
                                    tie->snav_2,
                                    (section1->snav_time_d[tie->snav_1] -
                                     section2->snav_time_d[tie->snav_2]));

                        /* A3: get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m
                                        - (section2->snav_lon_offset[tie->snav_2]
                                            - section1->snav_lon_offset[tie->snav_1])
                                            / project.mtodeglon;
                            offset_y = tie->offset_y_m
                                        - (section2->snav_lat_offset[tie->snav_2]
                                            - section1->snav_lat_offset[tie->snav_1])
                                            / project.mtodeglat;

                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                            //offset_x = tie->offset_x_m - (file2->block_offset_x - file1->block_offset_x);
                            //offset_y = tie->offset_y_m - (file2->block_offset_y - file1->block_offset_y);
                        }
                        else {
                            offset_x = 0.0;
                            offset_y = 0.0;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m
                                        - (section2->snav_z_offset[tie->snav_2]
                                            - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                            //offset_z = tie->offset_z_m - (file2->block_offset_z - file1->block_offset_z);
                        }
                        else {
                            offset_z = 0.0;
                        }

                        /* deal with each component of the error ellipse
                            - project offset vector onto each component by dot-product
                        - weight inversely by size of error for that component */

                        /* B1: deal with long axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1] + offset_z * tie->sigmax1[2];
                        if (fabs(tie->sigmar1) > 0.0)
                            weight = 1.0 / tie->sigmar1;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax1[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax1[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax1[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax1[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax1[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax1[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;

                        /* B2: deal with horizontal axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1] + offset_z * tie->sigmax2[2];
                        if (fabs(tie->sigmar2) > 0.0)
                            weight = 1.0 / tie->sigmar2;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax2[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax2[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax2[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax2[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax2[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax2[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;

                        /* B3:  deal with semi-vertical axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_z * tie->sigmax3[2];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax3[0] + offset_y * tie->sigmax3[1] + offset_z * tie->sigmax3[2];
                        if (fabs(tie->sigmar3) > 0.0)
                            weight = 1.0 / tie->sigmar3;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = -weight * tie->sigmax3[0];
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax3[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax3[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax3[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax3[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax3[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax3[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;
                    }
            }

            /* C1: loop over all files applying any global ties */
            //weight = 10.0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    int index_m;
                    int index_n;
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x = section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        weight = 1.0 / section->xsigma;
                        weight *= matrix_scale;
fprintf(stderr,"APPLYING WEIGHT: %f  ifile:%d isection:%d\n",weight,ifile,isection);

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_x;
                        //b[irow] = weight * (section->offset_x_m - file->block_offset_x);
                        matrix.nia[irow] = 1;
                        irow++;

                        offset_y = section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        weight = 1.0 / section->ysigma;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_y;
                        //b[irow] = weight * (section->offset_y_m - file->block_offset_y);
                        matrix.nia[irow] = 1;
                        irow++;

                        rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }

                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        weight = 1.0 / section->zsigma;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_z;
                        //b[irow] = weight * (section->offset_z_m - file->block_offset_z);
                        matrix.nia[irow] = 1;
                        irow++;

                        rms_misfit_previous += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

            /* D1: loop over all files applying ties for any fixed files */
            weight = 1000.0;
            weight *= matrix_scale;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                int index_m;
                int index_n;
                if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV ||
                    file->status == MBNA_FILE_FIXEDZNAV) {
                    for (int isection = 0; isection < file->num_sections; isection++) {
                        section = &file->sections[isection];
                        for (int isnav = 0; isnav < section->num_snav; isnav++) {
                            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_x;
                                matrix.nia[irow] = 1;
                                irow++;

                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3 + 1;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_y;
                                matrix.nia[irow] = 1;
                                irow++;
                            }

                            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3 + 2;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_z;
                                matrix.nia[irow] = 1;
                                irow++;
                            }
                        }
                    }
                }
            }

            /* E1: loop over all navigation applying first derivative smoothing */
            nnsmooth = 0;
            for (int inav = 0; inav < nnav - 1; inav++) {
                int index_m;
                int index_n;
                if (x_continuity[inav + 1]) {
                    if (x_time_d[inav + 1] - x_time_d[inav] > 0.0) {
                        weight = smoothweight / (x_time_d[inav + 1] - x_time_d[inav]);
                        if (x_quality[inav] == MBNA_FILE_POORNAV || x_quality[inav+1] == MBNA_FILE_POORNAV){
                            weight *= 0.25;
                        }
                    }
                    else {
                        weight = 0.0000001;
                    }
                    weight *= matrix_scale;
                    zweight = 10.0 * weight;
                    nnsmooth++;

                    index_m = irow * 6;
                    index_n = inav * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -zweight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;
                }
            }

            /* E1: loop over all navigation applying second derivative smoothing */
            /* nnsmooth = 0;
            for (int inav = 0; inav < nnav - 2; inav++) {
                if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
                    if (x_time_d[inav + 2] - x_time_d[inav] > 0.0) {
                        weight = smoothweight / (x_time_d[inav + 2] - x_time_d[inav]);
                        if (x_quality[inav] == MBNA_FILE_POORNAV
                            || x_quality[inav+1] == MBNA_FILE_POORNAV
                            || x_quality[inav+2] == MBNA_FILE_POORNAV) {
                            weight *= 0.25;
                        }
                    }
                    else {
                        weight = 0.0000001;
                    }
                    weight *= matrix_scale;
                    zweight = 10.0 * weight;
                    nnsmooth++;

                    index_m = irow * 6;
                    index_n = inav * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * weight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * weight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * zweight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;
                }
            }*/

            /* F1: loop over all navigation applying L1 norm - minimize size of offset */
            /*for (int inav = 0; inav < nnav; inav++) {
                weight = 0.001;
                weight *= matrix_scale;
                zweight = 10.0 * weight;

                index_m = irow * 6;
                index_n = inav * 3;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = weight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;

                index_m = irow * 6;
                index_n = inav * 3 + 1;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = weight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;

                index_m = irow * 6;
                index_n = inav * 3 + 2;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = zweight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;
            }*/

            fprintf(stderr, "\nAbout to call LSQR rows: %d %d %d  cols: %d %d\n", matrix.m, nrows, irow, matrix.n, ncols);

            /* F: call lsqr to solve the matrix problem */
            for (int irow = 0; irow < matrix.m; irow++)
                u[irow] = b[irow];
            damp = 0.0;
            atol = 1.0e-6;   // releative precision of A matrix
            btol = 1.0e-6;   // relative precision of data array
            relpr = 1.0e-16; // relative precision of double precision arithmetic
            conlim = 1 / (10 * sqrt(relpr));
            itnlim = 4 * matrix.n;
            // fprintf(stderr, "damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
            // damp, atol, btol, conlim, itnlim);

            // for (int i=0;i<matrix.m;i++)
            //  {
            //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
            //  for (int j=0;j<matrix.nia[i];j++)
            //    {
            //    k = i * 6 + j;
            //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
            //    }
            //  fprintf(stderr," | b:%10.6f\n",u[i]);
            //  }

            mblsqr_lsqr(matrix.m, matrix.n, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr, &istop_out,
                        &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

            fprintf(stderr, "\nInversion by LSQR completed\n");
            fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
            fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
            fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out, sqrt((double)matrix.n));
            fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
            fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
            fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
            fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);

            /* interpolate solution */
            itielast = -1;
            itienext = -1;
            for (int inav = 0; inav < nnav; inav++) {
                if (x_num_ties[inav] > 0) {
                    itielast = inav;
                }
                else {
                    /* look for the next tied point or the next discontinuity */
                    found = false;
                    itienext = -1;
                    for (int iinav=inav+1; iinav < nnav && !found; iinav++) {
                        if (!x_continuity[iinav]) {
                            found = true;
                            itienext = -1;
                        }
                        else if (x_num_ties[iinav] > 0) {
                            found = true;
                            itienext = iinav;
                        }
                    }
                    if (!x_continuity[inav]) {
                        itielast = -1;
                    }

                    /* now interpolate or extrapolate */
                    if (itielast >= 0 && itienext > itielast) {
                        factor = (x_time_d[inav] - x_time_d[itielast] ) / (x_time_d[itienext] - x_time_d[itielast]);
                        x[inav * 3] = x[itielast * 3] + factor * (x[itienext * 3] - x[itielast * 3]);
                        x[inav * 3 + 1] = x[itielast * 3 + 1] + factor * (x[itienext * 3 + 1] - x[itielast * 3 + 1]);
                        x[inav * 3 + 2] = x[itielast * 3 + 2] + factor * (x[itienext * 3 + 2] - x[itielast * 3 + 2]);
                    }
                    else if (itielast >= 0) {
                        x[inav * 3] = x[itielast * 3];
                        x[inav * 3 + 1] = x[itielast * 3 + 1];
                        x[inav * 3 + 2] = x[itielast * 3 + 2];
                    }
                    else if (itienext >= 0) {
                        x[inav * 3] = x[itienext * 3];
                        x[inav * 3 + 1] = x[itienext * 3 + 1];
                        x[inav * 3 + 2] = x[itienext * 3 + 2];
                    }
                }
            }

            /* save solution */
            rms_solution = 0.0;
            rms_solution_total = 0.0;
            nrms = 0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    for (int isnav = 0; isnav < section->num_snav; isnav++) {
                        int k = section->snav_invert_id[isnav];
                        section->snav_lon_offset[isnav] += x[3 * k] * project.mtodeglon;
                        section->snav_lat_offset[isnav] += x[3 * k + 1] * project.mtodeglat;
                        section->snav_z_offset[isnav] += x[3 * k + 2];
                        rms_solution += x[3 * k] * x[3 * k];
                        rms_solution += x[3 * k + 1] * x[3 * k + 1];
                        rms_solution += x[3 * k + 2] * x[3 * k + 2];
                        rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                        rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                        rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                        nrms += 3;
                    }
                }
            }
            if (nrms > 0) {
                rms_solution = sqrt(rms_solution);
                rms_solution_total = sqrt(rms_solution_total);
            }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous;
            }

            fprintf(stderr, "\nIteration %d:\n > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    iteration, rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);
        }

    if (convergence < 0.0) {
      fprintf(stderr, "WARNING: Inversion iteration terminated because misfit has increased!\n");
    }

    /* set message dialog on */
    sprintf(message, "Completed inversion...");
    do_message_update(message);

    /* update model plot */
    if (project.modelplot)
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);

    /* now output inverse solution */
    sprintf(message, "Outputting navigation solution...");
    do_message_update(message);

    sprintf(message, " > Final misfit:%12g\n > Initial misfit:%12g\n", rms_misfit_current, rms_misfit_initial);
    do_info_add(message, false);

    /* get crossing offset results */
    sprintf(message, " > Nav Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* check only set ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          offset_x = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon_offset[tie->snav_1];
          offset_y = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat_offset[tie->snav_1];
          offset_z = project.files[crossing->file_id_2].sections[crossing->section_2].snav_z_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_z_offset[tie->snav_1];

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(offset_x) > 10000.0 || fabs(offset_y) > 10000.0 || fabs(offset_z) > 10000.0) {
            tie->inversion_status = MBNA_INVERSION_OLD;
            tie->inversion_offset_x = 0.0;
            tie->inversion_offset_y = 0.0;
            tie->inversion_offset_x_m = 0.0;
            tie->inversion_offset_y_m = 0.0;
            tie->inversion_offset_z_m = 0.0;
            tie->dx_m = 0.0;
            tie->dy_m = 0.0;
            tie->dz_m = 0.0;
            tie->sigma_m = 0.0;
            tie->dr1_m = 0.0;
            tie->dr2_m = 0.0;
            tie->dr3_m = 0.0;
            tie->rsigma_m = 0.0;
          }
          else {
            tie->inversion_status = MBNA_INVERSION_CURRENT;
            tie->inversion_offset_x = offset_x;
            tie->inversion_offset_y = offset_y;
            tie->inversion_offset_x_m = offset_x / project.mtodeglon;
            tie->inversion_offset_y_m = offset_y / project.mtodeglat;
            tie->inversion_offset_z_m = offset_z;
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

          sprintf(message, " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f\n",
                  icrossing, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                            tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                            tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m);
          do_info_add(message, false);
        }
      }
    }

    /* get global tie results */
    sprintf(message, " > Global Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status != MBNA_TIE_NONE) {

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(section->snav_lon_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_lat_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_z_offset[section->global_tie_snav]) > 10000.0) {
            section->global_tie_inversion_status = MBNA_INVERSION_OLD;
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
          }
          else {
            section->global_tie_inversion_status = MBNA_INVERSION_CURRENT;
                        section->inversion_offset_x = section->snav_lon_offset[section->global_tie_snav];
                        section->inversion_offset_y = section->snav_lat_offset[section->global_tie_snav];
                        section->inversion_offset_x_m = section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        section->inversion_offset_y_m = section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
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
                    sprintf(message,
                            " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
                            ifile, isection, section->global_tie_snav, section->global_tie_status,
                            section->offset_x_m, section->offset_y_m, section->offset_z_m,
                            section->inversion_offset_x_m, section->inversion_offset_y_m, section->inversion_offset_z_m,
                            section->dx_m, section->dy_m, section->dz_m);
                    do_info_add(message, false);
        }
      }
    }

    /* write updated project */
    project.inversion_status = MBNA_INVERSION_CURRENT;
        project.modelplot_uptodate = false;
    project.grid_status = MBNA_GRID_OLD;
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* deallocate arrays */
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_quality, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_num_ties, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_chunk, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_time_d, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_center, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&u, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&v, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&se, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&b, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.nia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.ia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.a, &error);

    /* turn off message dialog */
    do_message_off();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_invertnavold20200525() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_file *file1;
  struct mbna_file *file2;
  struct mbna_section *section;
  struct mbna_section *section1;
  struct mbna_section *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_matrix matrix;
  bool *x_continuity = NULL;
  int *x_quality = NULL;
  int *x_num_ties = NULL;
  int *x_chunk = NULL;
  double *x_time_d = NULL;
  int *chunk_center = NULL;
  bool *chunk_continuity = NULL;
  double *u = NULL;
  double *v = NULL;
  double *w = NULL;
  double *x = NULL;
  int *nx = NULL;
  double *se = NULL;
  double *b = NULL;
  int *nbxy = NULL;
  int *nbz = NULL;
  double *bxavg = NULL;
  double *byavg = NULL;
  double *bzavg = NULL;
  bool *bpoornav = NULL;
  int *bxfixstatus = NULL;
  int *byfixstatus = NULL;
  int *bzfixstatus = NULL;
  double *bxfix = NULL;
  double *byfix = NULL;
  double *bzfix = NULL;
  double matrix_scale = 1000.0;
  double rms_solution, rms_solution_total, rms_misfit_initial, rms_misfit_previous, rms_misfit_current;
  int nrms;

  int nnav = 0;
  int nblock = 0;
  int ndiscontinuity = 0;
  int nsmooth = 0;
  int nnsmooth = 0;
  int ntie = 0;
  int nglobal = 0;
  int nfixed = 0;
  int nrows, ncols;
  int nblockties = 0;
  int nblockglobalties = 0;
  int nrows_ba = 0;
  int ncols_ba = 0;
  int nrows_alloc = 0;
  int ncols_alloc = 0;

  int nchunk, nchunk_start;
  double distance_sum, chunk_distance;
  double damping;

  int n_iteration;
  double convergence = 1000.0;
  double offset_x, offset_y, offset_z, projected_offset;
  double weight, zweight;
  double smooth_exp;
  double smoothweight;
  bool ok_to_invert;
  bool found;
  double factor;
  int itielast, itienext;
  double damp;
  double atol;
  double btol;
  double relpr;
  double conlim;
  int itnlim;
  int istop_out;
  int itn_out;
  double anorm_out;
  double acond_out;
  double rnorm_out;
  double arnorm_out;
  double xnorm_out;

  /* check if it is ok to invert
      - if there is a project
      - enough crossings have been analyzed
      - no problems with offsets and offset uncertainties */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings))

  {
    /* check that all uncertainty magnitudes are nonzero */
    ok_to_invert = true;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigmar1 <= 0.0 || tie->sigmar2 <= 0.0 || tie->sigmar3 <= 0.0) {
            ok_to_invert = false;
            fprintf(stderr,
                    "PROBLEM WITH TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
                    "%8.2f %8.2f %8.2f\n",
                    icrossing, j, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
                    tie->snav_1, project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2,
                    tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1, tie->sigmar2,
                    tie->sigmar3);
          }
        }
      }
    }

    /* print out warning */
    if (!ok_to_invert) {
      fprintf(stderr, "\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
      fprintf(stderr, "Please fix the ties with problems noted above before trying again.\n\n");
    }
  }

  /* invert if there is a project and enough crossings have been analyzed */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      ok_to_invert)

  {
    fprintf(stderr, "\nInverting for navigation adjustment model...\n");

    /* set message dialog on */
    sprintf(message, "Setting up navigation inversion...");
    do_message_on(message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

        /* zero solution across all navigation */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          section->snav_lon_offset[isnav] = 0.0;
          section->snav_lat_offset[isnav] = 0.0;
          section->snav_z_offset[isnav] = 0.0;
        }
      }
    }

    /* count number of nav points, discontinuities, and blocks */
    nnav = 0;
    nblock = 0;
    ndiscontinuity = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (!section->continuity)
          ndiscontinuity++;
      }
      file->block = nblock - 1;
      file->block_offset_x = 0.0;
      file->block_offset_y = 0.0;
      file->block_offset_z = 0.0;
    }

    /* allocate nav time and continuity arrays */
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&x_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_quality, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_num_ties, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&x_chunk, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&x_time_d, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(int), (void **)&chunk_center, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&chunk_continuity, &error);
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_quality, 0, nnav * sizeof(int));
    memset(x_num_ties, 0, nnav * sizeof(int));
    memset(x_chunk, 0, nnav * sizeof(int));
    memset(x_time_d, 0, nnav * sizeof(double));
    memset(chunk_center, 0, nnav * sizeof(int));
    memset(chunk_continuity, 0, nnav * sizeof(bool));

    /* loop over all files getting tables of time and continuity */
    int inav = 0;
    nchunk = 0;
    nchunk_start = 0;
    chunk_distance = 25 * project.section_length;
    distance_sum = 0.0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (isnav == 0 && section->continuity) {
            section->snav_invert_id[isnav] = inav - 1;
          }
          else {
            section->snav_invert_id[isnav] = inav;
            if (isnav == 0) {
              x_continuity[inav] = false;
              distance_sum = 0.0;
            }
            else {
              x_continuity[inav] = true;
            }
            x_time_d[inav] = section->snav_time_d[isnav];
            x_quality[inav] = file->status;
            x_num_ties[inav] = section->snav_num_ties[isnav];
            distance_sum += section->snav_distance[isnav];
            if ((!x_continuity[inav] && inav > 0) || distance_sum > chunk_distance) {
                distance_sum = 0.0;
                chunk_center[nchunk] = (nchunk_start + inav) / 2;
                chunk_continuity[nchunk] = x_continuity[inav];
                nchunk++;
                nchunk_start = inav + 1;
            }
            x_chunk[inav] = nchunk;
//fprintf(stderr,"inav:%d   %2.2d:%3.3d:%3.3d:%2.2d distance:  %f %f %f  nchunk:%d\n",
//        inav, file->block, ifile, isection, isnav, section->snav_distance[isnav], distance_sum, chunk_distance, nchunk);
            inav++;
          }
        }
      }
    }
    nchunk++;

    /* count first derivative smoothing points */
    nsmooth = 0;
    for (int inav = 0; inav < nnav - 1; inav++) {
      if (x_continuity[inav + 1]) {
        nsmooth++;
      }
    }
    nsmooth = 3 * nsmooth;

    /* count second derivative smoothing points */
    /*nsmooth = 0;
    for (int inav = 0; inav < nnav - 2; inav++) {
        if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
            nsmooth++;
        }
    }
    nsmooth = 3 * nsmooth;*/

    /* get dimensions of inversion problem and initial misfit */
    ntie = 0;
    nrms = 0;
    nglobal = 0;
    nfixed = 0;
    rms_misfit_initial = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (tie->offset_x_m * tie->offset_x_m) + (tie->offset_y_m * tie->offset_y_m);
            nrms += 2;
            //ntie += 2;
          }
          if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (tie->offset_z_m * tie->offset_z_m);
            nrms += 1;
            //ntie += 1;
          }
                    ntie += 3;
        }
      }
    }
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      /* get file */
      file = &project.files[ifile];

      /* count fixed and global ties for full inversion */
      for (int isection = 0; isection < file->num_sections; isection++) {
        /* get section */
        section = &file->sections[isection];

        /* count global ties for full inversion */
        if (section->global_tie_status != MBNA_TIE_NONE) {
          if (section->global_tie_status == MBNA_TIE_XY || section->global_tie_status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (section->offset_x_m * section->offset_x_m) + (section->offset_y_m * section->offset_y_m);
            nrms += 2;
            nglobal += 2;
          }
          if (section->global_tie_status == MBNA_TIE_Z || section->global_tie_status == MBNA_TIE_XYZ) {
                        rms_misfit_initial += (section->offset_z_m * section->offset_z_m);
            nrms += 1;
            nglobal += 1;
          }
        }

        /* count fixed sections for full inversion */
        if (file->status == MBNA_FILE_FIXEDNAV)
          nfixed += 3 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDXYNAV)
          nfixed += 2 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDZNAV)
          nfixed += 1 * section->num_snav;
      }
    }
        if (nrms > 0) {
            rms_misfit_initial /= nrms;
            rms_misfit_previous = rms_misfit_initial;
            rms_misfit_current = rms_misfit_initial;
        }

    /* only do block average solution if there is more than one block */
    if (nblock > 1) {

      /* allocate block average offset arrays */
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbxy, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbz, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bxavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&byavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bzavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(bool), (void **)&bpoornav, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bxfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&byfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bzfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bxfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&byfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bzfix, &error);
      memset(nbxy, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(nbz, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(bxavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(byavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bzavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bpoornav, 0, nblock * sizeof(bool));
      memset(bxfixstatus, 0, nblock * sizeof(int));
      memset(byfixstatus, 0, nblock * sizeof(int));
      memset(bzfixstatus, 0, nblock * sizeof(int));
      memset(bxfix, 0, nblock * sizeof(double));
      memset(byfix, 0, nblock * sizeof(double));
      memset(bzfix, 0, nblock * sizeof(double));

      /* count ties for all block vs block pairs and calculate average offsets
       * and count dimensions of full inversion problem */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];

        /* for block vs block averages use only set crossings between
         * different blocks */
        if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          int jbvb1 = 0;
          int jbvb2 = 0;
          int jbvb = 0;
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            /* get tie */
            tie = (struct mbna_tie *)&crossing->ties[itie];

            /* if blocks differ get id for block vs block */
            if (project.files[crossing->file_id_1].block != project.files[crossing->file_id_2].block) {
              if (project.files[crossing->file_id_2].block > project.files[crossing->file_id_1].block) {
                jbvb1 = project.files[crossing->file_id_1].block;
                jbvb2 = project.files[crossing->file_id_2].block;
              }
              else {
                jbvb1 = project.files[crossing->file_id_2].block;
                jbvb2 = project.files[crossing->file_id_1].block;
              }
              jbvb = (jbvb2) * (jbvb2 + 1) / 2 + jbvb1;

              if (tie->status != MBNA_TIE_Z) {
                bxavg[jbvb] += tie->offset_x_m;
                byavg[jbvb] += tie->offset_y_m;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY) {
                bzavg[jbvb] += tie->offset_z_m;
                nbz[jbvb]++;
              }
            }
          }
        }
      }

      /* calculate block vs block tie averages */
      fprintf(stderr, "Survey vs Survey tie counts and average offsets:\n");
      nblockties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            bxavg[jbvb] /= nbxy[jbvb];
            byavg[jbvb] /= nbxy[jbvb];
            nblockties += 2;
          }
          if (nbz[jbvb] > 0) {
            bzavg[jbvb] /= nbz[jbvb];
            nblockties++;
          }
          fprintf(stderr, "%2d vs %2d: %5d xy ties  %5d z ties  Avg offsets: %8.3f %8.3f %8.3f\n", jblock, iblock,
                  nbxy[jbvb], nbz[jbvb], bxavg[jbvb], byavg[jbvb], bzavg[jbvb]);
        }
      }

      /* get fixed blocks and average global ties for blocks */
      mbna_global_tie_influence = 6000;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed and global ties for full inversion */
        for (int isection = 0; isection < file->num_sections; isection++) {
          /* get section */
          section = &file->sections[isection];

          /* count global ties for block offset inversion */
          if (section->global_tie_status != MBNA_TIE_NONE) {
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->offset_x_m;
              byfixstatus[file->block]++;
              byfix[file->block] += section->offset_y_m;
            }
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->offset_z_m;
            }
          }
        }
      }

      /* count fixed sections for block average inversion,
       * overwriting global ties if they conflict */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed sections for block average inversion,
         * overwriting global ties if they conflict */
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
          bxfixstatus[file->block] = 1;
          bxfix[file->block] = 0.0;
          byfixstatus[file->block] = 1;
          byfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
          bzfixstatus[file->block] = 1;
          bzfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_POORNAV) {
          bpoornav[file->block] = true;
        }
      }
      nblockglobalties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          bxfix[iblock] /= (double)bxfixstatus[iblock];
          nblockglobalties++;
        }
        if (byfixstatus[iblock] > 0) {
          byfix[iblock] /= (double)byfixstatus[iblock];
          nblockglobalties++;
        }
        if (bzfixstatus[iblock] > 0) {
          bzfix[iblock] /= (double)bzfixstatus[iblock];
          nblockglobalties++;
        }
      }
    }

    /* We do a three stage inversion first for block averages, then a slow relaxation
     * towards a coarse solution, and finally an overdetermined least squares
     * solution for an additional perturbation to satisfy the remaining signal.
     * Make sure arrays are allocated large enough for both stages. */
    nrows = nfixed + ntie + nglobal + nsmooth;
    ncols = 3 * nnav;
    nrows_ba = nblockties + nblockglobalties + 3;
    ncols_ba = 3 * nblock;
    nrows_alloc = MAX(nrows, nrows_ba);
    ncols_alloc = MAX(ncols, ncols_ba);
    fprintf(stderr, "\nMBnavadjust block average inversion preparation:\n");
    fprintf(stderr, "     nblock:            %d\n", nblock);
    fprintf(stderr, "     nblockties:        %d\n", nblockties);
    fprintf(stderr, "     nblockglobalties:  %d\n", nblockglobalties);
    fprintf(stderr, "     nrows_ba:          %d\n", nrows_ba);
    fprintf(stderr, "     ncols_ba:          %d\n", ncols_ba);
    fprintf(stderr, "\nMBnavadjust full inversion preparation:\n");
    fprintf(stderr, "     nnav:              %d\n", nnav);
    fprintf(stderr, "     ntie:              %d\n", ntie);
    fprintf(stderr, "     nglobal:           %d\n", nglobal);
    fprintf(stderr, "     nfixed:            %d\n", nfixed);
    fprintf(stderr, "     nsmooth:           %d\n", nsmooth);
    fprintf(stderr, "     nrows:             %d\n", nrows);
    fprintf(stderr, "     ncols:             %d\n", ncols);
    fprintf(stderr, "\nMBnavadjust inversion array allocation dimensions:\n");
    fprintf(stderr, "     nrows_alloc:       %d\n", nrows_alloc);
    fprintf(stderr, "     ncols_alloc:       %d\n", ncols_alloc);

    /* allocate solution vector x, perturbation vector xx, and average solution vector xa */
    matrix.nia = NULL;
    matrix.ia = NULL;
    matrix.a = NULL;
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&u, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&v, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&w, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&x, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(int), (void **)&nx, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&se, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&b, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(int), (void **)&matrix.nia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(int), (void **)&matrix.ia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(double), (void **)&matrix.a, &error);
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(nx, 0, ncols_alloc * sizeof(int));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
    memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));

    /*----------------------------------------------------------------*/
    /* Create block offset inversion matrix problem                   */
    /*----------------------------------------------------------------*/
    if (nblock > 1) {
      matrix.m = nrows_ba;
      matrix.n = ncols_ba;
      matrix.ia_dim = ncols_ba;

      /* loop over each crossing, applying offsets evenly to both points
          for all ties that involve different blocks
          - weight inversely by number of ties for each block vs block pair
          so that each has same importance whether connected by one tie
          or many */

      /* set up inversion for block offsets
       * - start with average offsets between all block vs block pairs for
       *   x y and z wherever defined by one or more ties
       * - next apply average global ties for each block if they exist
       * - finally add a constraint for x y and z that the sum of all
       *   block offsets must be zero (ignoring blocks tagged as having
       *   poor navigation) */
      int irow = 0;

      /* start with average block vs block offsets */
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          int index_m;
          int index_n;
          int jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bxavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;

            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = byavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
          if (nbz[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bzavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
        }
      }

      /* next apply average global offsets for each block */
      mbna_global_tie_influence = 6000.0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m;
        int index_n;
        if (bxfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bxfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
fprintf(stderr, "Fix X block %d to %f\n", iblock, bxfix[iblock]);
        }
        if (byfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * byfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
fprintf(stderr, "Fix Y block %d to %f\n", iblock, byfix[iblock]);
        }
        if (bzfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = mbna_global_tie_influence * 1.0;

          b[irow] = mbna_global_tie_influence * bzfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
fprintf(stderr, "Fix Z block %d to %f\n", iblock, bzfix[iblock]);
        }
      }

      /* add constraint that overall average offset must be zero, ignoring
       * blocks with poor navigation */
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 1;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        int index_m = irow * ncols_ba + iblock;
        int index_n = iblock * 3 + 2;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;

      fprintf(stderr,
              "\nAbout to call LSQR for preliminary block solution   rows: %d cols: %d  (expected rows:%d cols:%d)\n", irow,
              nblock * 3, nrows_ba, ncols_ba);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < nrows_ba; irow++)
        u[irow] = b[irow];
      damp = 0.0;
      atol = 1.0e-6;   // releative precision of A matrix
      btol = 1.0e-6;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;
      // fprintf(stderr,"damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      //    damp,atol,btol,conlim,itnlim);

      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    int k = i * ncols_ba + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(nrows_ba, ncols_ba, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr,
                  &istop_out, &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

        /* save solution */
        double rms_solution = 0.0;
        double rms_solution_total = 0.0;
        int nrms = 0;
        for (int ifile = 0; ifile < project.num_files; ifile++) {
            file = &project.files[ifile];
        file->block_offset_x = x[3 * file->block];
        file->block_offset_y = x[3 * file->block + 1];
        file->block_offset_z = x[3 * file->block + 2];
        for (int isection = 0; isection < file->num_sections; isection++) {
            section = &file->sections[isection];
            for (int isnav = 0; isnav < section->num_snav; isnav++) {
                section->snav_lon_offset[isnav] = file->block_offset_x * project.mtodeglon;
                section->snav_lat_offset[isnav] = file->block_offset_y * project.mtodeglat;
                section->snav_z_offset[isnav] = file->block_offset_z;
                rms_solution += file->block_offset_x * file->block_offset_x;
                rms_solution += file->block_offset_y * file->block_offset_y;
                rms_solution += file->block_offset_z * file->block_offset_z;
                nrms += 3;
            }
        }
    }
    if (nrms > 0) {
        rms_solution = sqrt(rms_solution);
        rms_solution_total = rms_solution;
    }

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out,
              sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
      for (int i = 0; i < nblock; i++) {
        fprintf(stderr, "block[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n", i, x[3 * i], x[3 * i + 1],
                x[3 * i + 2]);
      }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous + 0.90;
            }

            fprintf(stderr, "\nBlock inversion:\n > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);

      /* deallocate arrays used only for block inversion */
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbxy, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbz, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bpoornav, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfix, &error);
    }

        /* stage 2 - iteratively relax towards a coarse offset model in which
         * nav specified as poor is downweighted relative to good nav. The
         * nav offsets of this coarse model will be added to the block offsets
         * and the total removed from the tie offsets used in the final inversion.
         * The approach is to use the least squares inversion to solve for zero
         * mean, Gaussian distributed offsets rather than the large scale offsets
         * and drift.
         * The coarseness is to solve for a navigation offset that is large scale
         * using a coarseness defined as 10 times the import section length
         */

        /* loop over all ties applying the offsets to the chunks partitioned according to survey quality */
        n_iteration = 100;
        convergence = 1000.0;
        damping = 0.01;
        for (int iteration=0; iteration < n_iteration && convergence > 1.0; iteration ++) {
            fprintf(stderr,"\nStarting stage 2 relaxation iteration %d\n", iteration);

            /* zero the working average offset array */
            memset(x, 0, ncols_alloc * sizeof(double));
            memset(nx, 0, ncols_alloc * sizeof(int));
            rms_misfit_previous = 0.0;
            nrms = 0;

            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];

                /* apply crossing ties */
                if (crossing->status == MBNA_CROSSING_STATUS_SET) {
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        int k1 = x_chunk[section1->snav_invert_id[tie->snav_1]];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        int k2 = x_chunk[section2->snav_invert_id[tie->snav_2]];

                        /* count tie impact on chunks */
                        nx[k1]++;
                        nx[k2]++;

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m
                                        - (section2->snav_lon_offset[tie->snav_2]
                                            - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m
                                        - (section2->snav_lat_offset[tie->snav_2]
                                            - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;

                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        else {
                            offset_x = 0.0;
                            offset_y = 0.0;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m
                                        - (section2->snav_z_offset[tie->snav_2]
                                            - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                        }
                        else {
                            offset_z = 0.0;
                        }

                        /* apply offsets to relevant chunks partitioned according to
                         * relative survey quality */
                        if (file1->status == MBNA_FILE_GOODNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_POORNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_FIXEDNAV
                                || file2->status == MBNA_FILE_FIXEDXYNAV
                                || file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV
                                || file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDXYNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                //x[3*k1]   +=  0.0
                                //x[3*k2]   +=  0.0;
                                //x[3*k1+1] +=  0.0
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                 //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] +=  0.0;
                                x[3*k2+1] +=  offset_y;
                                x[3*k1+2] += -offset_z;
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                        else if (file1->status == MBNA_FILE_FIXEDZNAV) {
                            if (file2->status == MBNA_FILE_GOODNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_POORNAV) {
                                //x[3*k1]   +=  0.0;
                                x[3*k2]   +=  offset_x;
                                //x[3*k1+1] += 0.0;
                                x[3*k2+1] +=  offset_y;
                                //x[3*k1+2] += 0.0;
                                x[3*k2+2] +=  offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDXYNAV) {
                                x[3*k1]   += -offset_x;
                                //x[3*k2]   +=  0.0;
                                x[3*k1+1] += -offset_y;
                                //x[3*k2+1] +=  0.0;
                                x[3*k1+2] += -0.5 * offset_z;
                                x[3*k2+2] +=  0.5 * offset_z;
                            }
                            else if (file2->status == MBNA_FILE_FIXEDZNAV) {
                                x[3*k1]   += -0.5 * offset_x;
                                x[3*k2]   +=  0.5 * offset_x;
                                x[3*k1+1] += -0.5 * offset_y;
                                x[3*k2+1] +=  0.5 * offset_y;
                                //x[3*k1+2] +=  0.0
                                //x[3*k2+2] +=  0.0;
                            }
                        }
                    }
                }
            }

            /* apply global ties */
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];

                    /* get absolute id for snav point */
                    int k = x_chunk[section->snav_invert_id[section->global_tie_snav]];

                    /* count global tie impact on chunks */
                    nx[k]++;

                    /* get and apply offset vector for this tie */
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x = section->offset_x_m
                            - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y = section->offset_y_m
                            - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                        x[3*k]   += -5.0*offset_x;
                        x[3*k+1] += -5.0*offset_y;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m
                            - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_previous += offset_z * offset_z;
                        nrms += 1;
                        x[3*k+2] += -5.0*offset_z;
                    }
                }
            }

            /* linearly interpolate over gaps between impacted chunks */
            int klast = 0;
            for (int k=0; k < nchunk; k++) {
              if (nx[k] > 0) {
                if (k - klast > 1) {
                  double factor0 = (x[3*k]   - x[3*klast])   / ((double)(k - klast));
                  double factor1 = (x[3*k+1] - x[3*klast+1]) / ((double)(k - klast));
                  double factor2 = (x[3*k+2] - x[3*klast+2]) / ((double)(k - klast));
                  for (int kk=klast+1; kk<k; kk++) {
                    x[3*kk]   = x[3*klast]   + factor0 * ((double)(kk - klast));
                    x[3*kk+1] = x[3*klast+1] + factor1 * ((double)(kk - klast));
                    x[3*kk+2] = x[3*klast+2] + factor2 * ((double)(kk - klast));
                  }
                }
                klast = k;
              }
            }

            /* apply damping to solution vector */
            for (int k=0; k<3 * nchunk; k++) {
                x[k] *= damping;
            }

            /* penalize change between continuous chunks using the w work array */
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                w[3*k] = x[3*k] - x[3*(k-1)];
                w[3*k+1] = x[3*k+1] - x[3*(k-1)+1];
                w[3*k+2] = x[3*k+2] - x[3*(k-1)+2];
              }
            }
            for (int k=1; k < nchunk; k++) {
              if (chunk_continuity[k]) {
                x[3*(k-1)] += 10.0 * damping * 0.5 * w[3*k];
                x[3*(k-1)+1] += 10.0 * damping * 0.5 * w[3*k+1];
                x[3*(k-1)+2] += 10.0 * damping * 0.5 * w[3*k+2];
                x[3*k] -= 10.0 * damping * 0.5 * w[3*k];
                x[3*k+1] -= 10.0 * damping * 0.5 * w[3*k+1];
                x[3*k+2] -= 10.0 * damping * 0.5 * w[3*k+2];
              }
            }

            /* get previous misfit measure */
            rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

            /* add average offsets back into the model */
            rms_solution = 0.0;
            rms_solution_total = 0.0;
            nrms = 0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    for (int isnav = 0; isnav < section->num_snav; isnav++) {
                        inav = section->snav_invert_id[isnav];
                        int k = x_chunk[inav];
                        if (inav == chunk_center[k]
                            || (k == 0 && inav <= chunk_center[k])
                            ||  (k == nchunk - 1 && inav >= chunk_center[k])) {
                            offset_x = x[3 * k];
                            offset_y = x[3 * k + 1];
                            offset_z = x[3 * k + 2];
                        }
                        else if (inav <= chunk_center[k]) {
                            factor = ((double)(inav - chunk_center[k-1])) / ((double)(chunk_center[k] - chunk_center[k-1]));
                            offset_x = x[3 * (k - 1)] + factor * (x[3 * k] - x[3 * (k - 1)]);
                            offset_y = x[3 * (k - 1) + 1] + factor * (x[3 * k + 1] - x[3 * (k - 1) + 1]);
                            offset_z = x[3 * (k - 1) + 2] + factor * (x[3 * k + 2] - x[3 * (k - 1) + 2]);
                        }
                        else if (inav >= chunk_center[k]) {
                            factor = ((double)(inav - chunk_center[k])) / ((double)(chunk_center[k+1] - chunk_center[k]));
                            offset_x = x[3 * k] + factor * (x[3 * (k + 1)] - x[3 * k]);
                            offset_y = x[3 * k + 1] + factor * (x[3 * (k + 1) + 1] - x[3 * k + 1]);
                            offset_z = x[3 * k + 2] + factor * (x[3 * (k + 1) + 2] - x[3 * k + 2]);
                        }
//fprintf(stderr,"inav:%d chunk:%d of %d offsets:%f %f %f\n", inav,k,nchunk,offset_x,offset_y,offset_z);
                        section->snav_lon_offset[isnav] += offset_x * project.mtodeglon;
                        section->snav_lat_offset[isnav] += offset_y * project.mtodeglat;
                        section->snav_z_offset[isnav] += offset_z;
                        rms_solution += offset_x * offset_x;
                        rms_solution += offset_y * offset_y;
                        rms_solution += offset_z * offset_z;
                        rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                        rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                        rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                        nrms += 3;
                    }
                }
            }
            if (nrms > 0) {
                rms_solution = sqrt(rms_solution);
                rms_solution_total = sqrt(rms_solution_total);
            }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                // int nc1;
                // int nc2;
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // const int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // const int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous + 0.98;
            }

            fprintf(stderr, "\nStage 2 iteration %d:\n > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    iteration, rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);
        } // iteration

    /* set message dialog on */
    sprintf(message, "Completed chunk inversion...");
    do_message_update(message);

    /*----------------------------------------------------------------*/
    /* Create complete inversion matrix problem                       */
        /* Do this inversion multiple times using different smoothing     */
        /* parameters, starting large and getting smaller. Repeat the     */
        /* specified smoothing parameter several times.                   */
    /*----------------------------------------------------------------*/

        matrix_scale = 1000.0;
        n_iteration = 1;
        // const double smooth_max = 2.0 * project.smoothing;
        // const double d_smooth = (smooth_max - project.smoothing) / 5;
        convergence = 1000.0;
        for (int iteration=0; iteration < n_iteration && convergence > 1.0; iteration ++) {

            /* set message dialog on */
            sprintf(message, "Performing navigation inversion iteration %d of %d...", iteration +1, n_iteration);
            do_message_on(message);


            //smoothweight = pow(10.0, project.smoothing) / 100.0;
            //smooth_exp = MAX((smooth_max - iteration * d_smooth), project.smoothing);
            smooth_exp = project.smoothing;
            smoothweight = pow(10.0, smooth_exp) / 100.0;
            fprintf(stderr, "\n----------\n\nPreparing inversion iteration %d with smoothing %f ==> %f\n\t\trows: %d %d  cols: %d %d\n",
                    iteration, smooth_exp,
                    smoothweight, matrix.m, nrows, matrix.n, ncols);

            /* loop over each crossing, applying offsets evenly to both points */
            int irow = 0;
            nrms = 0;
            rms_misfit_previous = 0.0;
            matrix.m = nrows;
            matrix.n = ncols;
            matrix.ia_dim = 6;
            memset(u, 0, nrows_alloc * sizeof(double));
            memset(v, 0, ncols_alloc * sizeof(double));
            memset(w, 0, ncols_alloc * sizeof(double));
            memset(x, 0, ncols_alloc * sizeof(double));
            memset(se, 0, ncols_alloc * sizeof(double));
            memset(b, 0, nrows_alloc * sizeof(double));
            memset(matrix.nia, 0, nrows_alloc * sizeof(int));
            memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
            memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                int nc1;
                int nc2;

                /* use only set crossings */
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* A: get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];
                        int index_m;
                        int index_n;

                        /* A1: get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        nc1 = section1->snav_invert_id[tie->snav_1];

                        /* A2: get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        nc2 = section2->snav_invert_id[tie->snav_2];

                        if (section1->snav_time_d[tie->snav_1] ==
                            section2->snav_time_d[tie->snav_2])
                            fprintf(stderr, "ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
                                    crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2, crossing->section_2,
                                    tie->snav_2,
                                    (section1->snav_time_d[tie->snav_1] -
                                     section2->snav_time_d[tie->snav_2]));

                        /* A3: get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m
                                        - (section2->snav_lon_offset[tie->snav_2]
                                            - section1->snav_lon_offset[tie->snav_1])
                                            / project.mtodeglon;
                            offset_y = tie->offset_y_m
                                        - (section2->snav_lat_offset[tie->snav_2]
                                            - section1->snav_lat_offset[tie->snav_1])
                                            / project.mtodeglat;

                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                            //offset_x = tie->offset_x_m - (file2->block_offset_x - file1->block_offset_x);
                            //offset_y = tie->offset_y_m - (file2->block_offset_y - file1->block_offset_y);
                        }
                        else {
                            offset_x = 0.0;
                            offset_y = 0.0;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m
                                        - (section2->snav_z_offset[tie->snav_2]
                                            - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                            //offset_z = tie->offset_z_m - (file2->block_offset_z - file1->block_offset_z);
                        }
                        else {
                            offset_z = 0.0;
                        }

                        /* deal with each component of the error ellipse
                            - project offset vector onto each component by dot-product
                        - weight inversely by size of error for that component */

                        /* B1: deal with long axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1] + offset_z * tie->sigmax1[2];
                        if (fabs(tie->sigmar1) > 0.0)
                            weight = 1.0 / tie->sigmar1;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax1[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax1[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax1[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax1[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax1[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax1[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;

                        /* B2: deal with horizontal axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1] + offset_z * tie->sigmax2[2];
                        if (fabs(tie->sigmar2) > 0.0)
                            weight = 1.0 / tie->sigmar2;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax2[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax2[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax2[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_Z)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax2[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax2[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax2[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;

                        /* B3:  deal with semi-vertical axis */
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
                            projected_offset = offset_z * tie->sigmax3[2];
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            projected_offset = offset_x * tie->sigmax3[0] + offset_y * tie->sigmax3[1] + offset_z * tie->sigmax3[2];
                        if (fabs(tie->sigmar3) > 0.0)
                            weight = 1.0 / tie->sigmar3;
                        else
                            weight = 0.0;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = nc1 * 3;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = -weight * tie->sigmax3[0];
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax3[0];

                        index_m = irow * 6 + 1;
                        index_n = nc2 * 3;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax3[0];

                        index_m = irow * 6 + 2;
                        index_n = nc1 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = -weight * tie->sigmax3[1];

                        index_m = irow * 6 + 3;
                        index_n = nc2 * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                            matrix.a[index_m] = weight * tie->sigmax3[1];

                        index_m = irow * 6 + 4;
                        index_n = nc1 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = -weight * tie->sigmax3[2];

                        index_m = irow * 6 + 5;
                        index_n = nc2 * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        if (tie->status == MBNA_TIE_XY)
                            matrix.a[index_m] = 0.0;
                        else
                            matrix.a[index_m] = weight * tie->sigmax3[2];

                        b[irow] = weight * projected_offset;
                        matrix.nia[irow] = 6;
                        irow++;
                    }
            }

            /* C1: loop over all files applying any global ties */
            //weight = 10.0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    int index_m;
                    int index_n;
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x = section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        weight = 1.0 / section->xsigma;
                        weight *= matrix_scale;
fprintf(stderr,"APPLYING WEIGHT: %f  ifile:%d isection:%d\n",weight,ifile,isection);

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_x;
                        //b[irow] = weight * (section->offset_x_m - file->block_offset_x);
                        matrix.nia[irow] = 1;
                        irow++;

                        offset_y = section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        weight = 1.0 / section->ysigma;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 1;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_y;
                        //b[irow] = weight * (section->offset_y_m - file->block_offset_y);
                        matrix.nia[irow] = 1;
                        irow++;

                        rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }

                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        weight = 1.0 / section->zsigma;
                        weight *= matrix_scale;

                        index_m = irow * 6;
                        index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 2;
                        matrix.ia[index_m] = index_n;
                        matrix.a[index_m] = weight;
                        b[irow] = weight * offset_z;
                        //b[irow] = weight * (section->offset_z_m - file->block_offset_z);
                        matrix.nia[irow] = 1;
                        irow++;

                        rms_misfit_previous += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            rms_misfit_previous = sqrt(rms_misfit_previous) / nrms;

            /* D1: loop over all files applying ties for any fixed files */
            weight = 1000.0;
            weight *= matrix_scale;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                int index_m;
                int index_n;
                if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV ||
                    file->status == MBNA_FILE_FIXEDZNAV) {
                    for (int isection = 0; isection < file->num_sections; isection++) {
                        section = &file->sections[isection];
                        for (int isnav = 0; isnav < section->num_snav; isnav++) {
                            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_x;
                                matrix.nia[irow] = 1;
                                irow++;

                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3 + 1;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_y;
                                matrix.nia[irow] = 1;
                                irow++;
                            }

                            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
                                index_m = irow * 6;
                                index_n = section->snav_invert_id[isnav] * 3 + 2;
                                matrix.ia[index_m] = index_n;
                                matrix.a[index_m] = weight;
                                b[irow] = -file->block_offset_z;
                                matrix.nia[irow] = 1;
                                irow++;
                            }
                        }
                    }
                }
            }

            /* E1: loop over all navigation applying first derivative smoothing */
            nnsmooth = 0;
            for (int inav = 0; inav < nnav - 1; inav++) {
                int index_m;
                int index_n;
                if (x_continuity[inav + 1]) {
                    if (x_time_d[inav + 1] - x_time_d[inav] > 0.0) {
                        weight = smoothweight / (x_time_d[inav + 1] - x_time_d[inav]);
                        if (x_quality[inav] == MBNA_FILE_POORNAV || x_quality[inav+1] == MBNA_FILE_POORNAV){
                            weight *= 0.25;
                        }
                    }
                    else {
                        weight = 0.0000001;
                    }
                    weight *= matrix_scale;
                    zweight = 10.0 * weight;
                    nnsmooth++;

                    index_m = irow * 6;
                    index_n = inav * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -zweight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 2;
                    irow++;
                }
            }

            /* E1: loop over all navigation applying second derivative smoothing */
            /* nnsmooth = 0;
            for (int inav = 0; inav < nnav - 2; inav++) {
                if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
                    if (x_time_d[inav + 2] - x_time_d[inav] > 0.0) {
                        weight = smoothweight / (x_time_d[inav + 2] - x_time_d[inav]);
                        if (x_quality[inav] == MBNA_FILE_POORNAV
                            || x_quality[inav+1] == MBNA_FILE_POORNAV
                            || x_quality[inav+2] == MBNA_FILE_POORNAV) {
                            weight *= 0.25;
                        }
                    }
                    else {
                        weight = 0.0000001;
                    }
                    weight *= matrix_scale;
                    zweight = 10.0 * weight;
                    nnsmooth++;

                    index_m = irow * 6;
                    index_n = inav * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * weight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * weight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3 + 1;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = weight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;

                    index_m = irow * 6;
                    index_n = inav * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    index_m = irow * 6 + 1;
                    index_n = (inav + 1) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = -2.0 * zweight;
                    index_m = irow * 6 + 2;
                    index_n = (inav + 2) * 3 + 2;
                    matrix.ia[index_m] = index_n;
                    matrix.a[index_m] = zweight;
                    b[irow] = 0.0;
                    matrix.nia[irow] = 3;
                    irow++;
                }
            }*/

            /* F1: loop over all navigation applying L1 norm - minimize size of offset */
            /*for (int inav = 0; inav < nnav; inav++) {
                weight = 0.001;
                weight *= matrix_scale;
                zweight = 10.0 * weight;

                index_m = irow * 6;
                index_n = inav * 3;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = weight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;

                index_m = irow * 6;
                index_n = inav * 3 + 1;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = weight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;

                index_m = irow * 6;
                index_n = inav * 3 + 2;
                matrix.ia[index_m] = index_n;
                matrix.a[index_m] = zweight;
                b[irow] = 0.0;
                matrix.nia[irow] = 1;
                irow++;
            }*/

            fprintf(stderr, "\nAbout to call LSQR rows: %d %d %d  cols: %d %d\n", matrix.m, nrows, irow, matrix.n, ncols);

            /* F: call lsqr to solve the matrix problem */
            for (int irow = 0; irow < matrix.m; irow++)
                u[irow] = b[irow];
            damp = 0.0;
            atol = 1.0e-6;   // releative precision of A matrix
            btol = 1.0e-6;   // relative precision of data array
            relpr = 1.0e-16; // relative precision of double precision arithmetic
            conlim = 1 / (10 * sqrt(relpr));
            itnlim = 4 * matrix.n;
            // fprintf(stderr, "damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
            // damp, atol, btol, conlim, itnlim);

            // for (int i=0;i<matrix.m;i++)
            //  {
            //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
            //  for (int j=0;j<matrix.nia[i];j++)
            //    {
            //    k = i * 6 + j;
            //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
            //    }
            //  fprintf(stderr," | b:%10.6f\n",u[i]);
            //  }

            mblsqr_lsqr(matrix.m, matrix.n, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr, &istop_out,
                        &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

            fprintf(stderr, "\nInversion by LSQR completed\n");
            fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
            fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
            fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out, sqrt((double)matrix.n));
            fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
            fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
            fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
            fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);

            /* interpolate solution */
            itielast = -1;
            itienext = -1;
            for (int inav = 0; inav < nnav; inav++) {
                if (x_num_ties[inav] > 0) {
                    itielast = inav;
                }
                else {
                    /* look for the next tied point or the next discontinuity */
                    found = false;
                    itienext = -1;
                    for (int iinav=inav+1; iinav < nnav && !found; iinav++) {
                        if (!x_continuity[iinav]) {
                            found = true;
                            itienext = -1;
                        }
                        else if (x_num_ties[iinav] > 0) {
                            found = true;
                            itienext = iinav;
                        }
                    }
                    if (!x_continuity[inav]) {
                        itielast = -1;
                    }

                    /* now interpolate or extrapolate */
                    if (itielast >= 0 && itienext > itielast) {
                        factor = (x_time_d[inav] - x_time_d[itielast] ) / (x_time_d[itienext] - x_time_d[itielast]);
                        x[inav * 3] = x[itielast * 3] + factor * (x[itienext * 3] - x[itielast * 3]);
                        x[inav * 3 + 1] = x[itielast * 3 + 1] + factor * (x[itienext * 3 + 1] - x[itielast * 3 + 1]);
                        x[inav * 3 + 2] = x[itielast * 3 + 2] + factor * (x[itienext * 3 + 2] - x[itielast * 3 + 2]);
                    }
                    else if (itielast >= 0) {
                        x[inav * 3] = x[itielast * 3];
                        x[inav * 3 + 1] = x[itielast * 3 + 1];
                        x[inav * 3 + 2] = x[itielast * 3 + 2];
                    }
                    else if (itienext >= 0) {
                        x[inav * 3] = x[itienext * 3];
                        x[inav * 3 + 1] = x[itienext * 3 + 1];
                        x[inav * 3 + 2] = x[itienext * 3 + 2];
                    }
                }
            }

            /* save solution */
            rms_solution = 0.0;
            rms_solution_total = 0.0;
            nrms = 0;
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    for (int isnav = 0; isnav < section->num_snav; isnav++) {
                        int k = section->snav_invert_id[isnav];
                        section->snav_lon_offset[isnav] += x[3 * k] * project.mtodeglon;
                        section->snav_lat_offset[isnav] += x[3 * k + 1] * project.mtodeglat;
                        section->snav_z_offset[isnav] += x[3 * k + 2];
                        rms_solution += x[3 * k] * x[3 * k];
                        rms_solution += x[3 * k + 1] * x[3 * k + 1];
                        rms_solution += x[3 * k + 2] * x[3 * k + 2];
                        rms_solution_total += section->snav_lon_offset[isnav] * section->snav_lon_offset[isnav] / project.mtodeglon / project.mtodeglon;
                        rms_solution_total += section->snav_lat_offset[isnav] * section->snav_lat_offset[isnav] / project.mtodeglat / project.mtodeglat;
                        rms_solution_total += section->snav_z_offset[isnav] * section->snav_z_offset[isnav];
                        nrms += 3;
                    }
                }
            }
            if (nrms > 0) {
                rms_solution = sqrt(rms_solution);
                rms_solution_total = sqrt(rms_solution_total);
            }

            /* calculate final misfit */
            nrms = 0;
            rms_misfit_current = 0.0;
            for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
                crossing = &project.crossings[icrossing];
                if (crossing->status == MBNA_CROSSING_STATUS_SET)
                    for (int itie = 0; itie < crossing->num_ties; itie++) {
                        /* get tie */
                        tie = (struct mbna_tie *)&crossing->ties[itie];

                        /* get absolute id for first snav point */
                        file1 = &project.files[crossing->file_id_1];
                        section1 = &file1->sections[crossing->section_1];
                        // int nc1 = section1->snav_invert_id[tie->snav_1];

                        /* get absolute id for second snav point */
                        file2 = &project.files[crossing->file_id_2];
                        section2 = &file2->sections[crossing->section_2];
                        // int nc2 = section2->snav_invert_id[tie->snav_2];

                        /* get offset vector for this tie */
                        if (tie->status != MBNA_TIE_Z) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY) {
                            offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
                            rms_misfit_current += offset_z * offset_z;
                            nrms += 1;
                        }
                    }
            }
            for (int ifile = 0; ifile < project.num_files; ifile++) {
                file = &project.files[ifile];
                for (int isection = 0; isection < file->num_sections; isection++) {
                    section = &file->sections[isection];
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
                        offset_x =
                            section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        offset_y =
                            section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
                        offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
                        rms_misfit_current += offset_z * offset_z;
                        nrms += 1;
                    }
                }
            }
            if (nrms > 0) {
                rms_misfit_current = sqrt(rms_misfit_current) / nrms;
                convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous + 0.98;
            }

            fprintf(stderr, "\nIteration %d:\n > Solution size:        %12g\n"
                    " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
                    " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
                    " > Convergence:          %12g\n",
                    iteration, rms_solution, rms_solution_total, rms_misfit_initial,
                    rms_misfit_previous, rms_misfit_current, convergence);
        }

    /* set message dialog on */
    sprintf(message, "Completed inversion...");
    do_message_update(message);

    /* update model plot */
    if (project.modelplot)
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);

    /* now output inverse solution */
    sprintf(message, "Outputting navigation solution...");
    do_message_update(message);

    sprintf(message, " > Final misfit:%12g\n > Initial misfit:%12g\n", rms_misfit_current, rms_misfit_initial);
    do_info_add(message, false);

    /* get crossing offset results */
    sprintf(message, " > Nav Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* check only set ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          offset_x = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon_offset[tie->snav_1];
          offset_y = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat_offset[tie->snav_1];
          offset_z = project.files[crossing->file_id_2].sections[crossing->section_2].snav_z_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_z_offset[tie->snav_1];

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(offset_x) > 10000.0 || fabs(offset_y) > 10000.0 || fabs(offset_z) > 10000.0) {
            tie->inversion_status = MBNA_INVERSION_OLD;
            tie->inversion_offset_x = 0.0;
            tie->inversion_offset_y = 0.0;
            tie->inversion_offset_x_m = 0.0;
            tie->inversion_offset_y_m = 0.0;
            tie->inversion_offset_z_m = 0.0;
            tie->dx_m = 0.0;
            tie->dy_m = 0.0;
            tie->dz_m = 0.0;
            tie->sigma_m = 0.0;
            tie->dr1_m = 0.0;
            tie->dr2_m = 0.0;
            tie->dr3_m = 0.0;
            tie->rsigma_m = 0.0;
          }
          else {
            tie->inversion_status = MBNA_INVERSION_CURRENT;
            tie->inversion_offset_x = offset_x;
            tie->inversion_offset_y = offset_y;
            tie->inversion_offset_x_m = offset_x / project.mtodeglon;
            tie->inversion_offset_y_m = offset_y / project.mtodeglat;
            tie->inversion_offset_z_m = offset_z;
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

          sprintf(message, " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f\n",
                  icrossing, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                            tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                            tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m);
          do_info_add(message, false);
        }
      }
    }

    /* get global tie results */
    sprintf(message, " > Global Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status != MBNA_TIE_NONE) {

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(section->snav_lon_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_lat_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_z_offset[section->global_tie_snav]) > 10000.0) {
            section->global_tie_inversion_status = MBNA_INVERSION_OLD;
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
          }
          else {
            section->global_tie_inversion_status = MBNA_INVERSION_CURRENT;
                        section->inversion_offset_x = section->snav_lon_offset[section->global_tie_snav];
                        section->inversion_offset_y = section->snav_lat_offset[section->global_tie_snav];
                        section->inversion_offset_x_m = section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        section->inversion_offset_y_m = section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
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
                    sprintf(message,
                            " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
                            ifile, isection, section->global_tie_snav, section->global_tie_status,
                            section->offset_x_m, section->offset_y_m, section->offset_z_m,
                            section->inversion_offset_x_m, section->inversion_offset_y_m, section->inversion_offset_z_m,
                            section->dx_m, section->dy_m, section->dz_m);
                    do_info_add(message, false);
        }
      }
    }

    /* write updated project */
    project.inversion_status = MBNA_INVERSION_CURRENT;
        project.modelplot_uptodate = false;
    project.grid_status = MBNA_GRID_OLD;
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* deallocate arrays */
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_quality, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_num_ties, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_chunk, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_time_d, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_center, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&chunk_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&u, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&v, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nx, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&se, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&b, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.nia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.ia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.a, &error);

    /* turn off message dialog */
    do_message_off();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_invertnav_old() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_file *file1;
  struct mbna_file *file2;
  struct mbna_section *section;
  struct mbna_section *section1;
  struct mbna_section *section2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_matrix matrix;
  bool *x_continuity = NULL;
  double *x_time_d = NULL;
  double *u = NULL;
  double *v = NULL;
  double *w = NULL;
  double *x = NULL;
  double *se = NULL;
  double *b = NULL;
  int *nbxy = NULL;
  int *nbz = NULL;
  double *bxavg = NULL;
  double *byavg = NULL;
  double *bzavg = NULL;
  bool *bpoornav = NULL;
  int *bxfixstatus = NULL;
  int *byfixstatus = NULL;
  int *bzfixstatus = NULL;
  double *bxfix = NULL;
  double *byfix = NULL;
  double *bzfix = NULL;
  double rms_misfit_initial, rms_misfit_current;

  int nnav = 0;
  int nblock = 0;
  int ndiscontinuity = 0;
  int nsmooth = 0;
  int ntie = 0;
  int nglobal = 0;
  int nfixed = 0;
  int nrows, ncols;
  int nblockties = 0;
  int nblockglobalties = 0;
  int nrows_ba = 0;
  int ncols_ba = 0;
  int nrows_alloc = 0;
  int ncols_alloc = 0;
  int irow, inav, nc1, nc2;
  int index_m, index_n;
  int jbvb, jbvb1, jbvb2;
  int k;

  double offset_x, offset_y, offset_z, projected_offset;
  double weight;
  double smoothweight;
  bool ok_to_invert;
  double damp;
  double atol;
  double btol;
  double relpr;
  double conlim;
  int itnlim;
  int istop_out;
  int itn_out;
  double anorm_out;
  double acond_out;
  double rnorm_out;
  double arnorm_out;
  double xnorm_out;

  /* check if it is ok to invert
      - if there is a project
      - enough crossings have been analyzed
      - no problems with offsets and offset uncertainties */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings))

  {
    /* check that all uncertainty magnitudes are nonzero */
    ok_to_invert = true;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigmar1 <= 0.0 || tie->sigmar2 <= 0.0 || tie->sigmar3 <= 0.0) {
            ok_to_invert = false;
            fprintf(stderr,
                    "PROBLEM WITH TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
                    "%8.2f %8.2f %8.2f\n",
                    icrossing, j, project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1,
                    tie->snav_1, project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2,
                    tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m, tie->sigmar1, tie->sigmar2,
                    tie->sigmar3);
          }
        }
      }
    }

    /* print out warning */
    if (!ok_to_invert) {
      fprintf(stderr, "\nThe inversion was not performed because there are one or more zero offset uncertainty values.\n");
      fprintf(stderr, "Please fix the ties with problems noted above before trying again.\n\n");
    }
  }

  /* invert if there is a project and enough crossings have been analyzed */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      ok_to_invert)

  {
    fprintf(stderr, "\nInverting for navigation adjustment model...\n");

    /* set message dialog on */
    sprintf(message, "Setting up navigation inversion...");
    do_message_on(message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

    /* count number of nav points, discontinuities, and blocks */
    nnav = 0;
    nblock = 0;
    ndiscontinuity = 0;
    nsmooth = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (!section->continuity)
          ndiscontinuity++;
      }
      file->block = nblock - 1;
      file->block_offset_x = 0.0;
      file->block_offset_y = 0.0;
      file->block_offset_z = 0.0;
    }

    /* allocate nav time and continuity arrays */
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(bool), (void **)&x_continuity, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nnav * sizeof(double), (void **)&x_time_d, &error);
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_time_d, 0, nnav * sizeof(double));

    /* loop over all files getting tables of time and continuity */
    inav = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (isnav == 0 && section->continuity) {
            section->snav_invert_id[isnav] = inav - 1;
            nsmooth++;
          }
          else {
            section->snav_invert_id[isnav] = inav;
            if (isnav == 0) {
              x_continuity[inav] = false;
            }
            else {
              x_continuity[inav] = true;
              nsmooth++;
            }
            x_time_d[inav] = section->snav_time_d[isnav];
            inav++;
          }
        }
      }
    }
    nsmooth = 3 * (nsmooth - 1);

    /* count ties for full inversion */
    ntie = 0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XYZ)
            ntie += 3;
          else if (tie->status == MBNA_TIE_XY)
            ntie += 2;
          else if (tie->status == MBNA_TIE_Z)
            ntie += 1;
        }
      }
    }

    /* count dimensions of full inversion problem */
    nglobal = 0;
    nfixed = 0;
    mbna_global_tie_influence = 6000;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      /* get file */
      file = &project.files[ifile];

      /* count fixed and global ties for full inversion */
      for (int isection = 0; isection < file->num_sections; isection++) {
        /* get section */
        section = &file->sections[isection];

        /* count global ties for full inversion */
        if (section->global_tie_status != MBNA_TIE_NONE) {
          if (section->global_tie_status == MBNA_TIE_XYZ)
            nglobal += 3;
          else if (section->global_tie_status == MBNA_TIE_XY)
            nglobal += 2;
          else if (section->global_tie_status == MBNA_TIE_Z)
            nglobal += 1;
        }

        /* count fixed sections for full inversion */
        if (file->status == MBNA_FILE_FIXEDNAV)
          nfixed += 3 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDXYNAV)
          nfixed += 2 * section->num_snav;
        else if (file->status == MBNA_FILE_FIXEDZNAV)
          nfixed += 1 * section->num_snav;
      }
    }

    /* only do block average solution if there is more than one block */
    if (nblock > 1) {

      /* allocate block average offset arrays */
      status =
          mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbxy, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(int), (void **)&nbz, &error);
      status =
          mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bxavg, &error);
      status =
          mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&byavg, &error);
      status =
          mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * (nblock + 1) / 2 * sizeof(double), (void **)&bzavg, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(bool), (void **)&bpoornav, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bxfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&byfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(int), (void **)&bzfixstatus, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bxfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&byfix, &error);
      status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nblock * sizeof(double), (void **)&bzfix, &error);
      memset(nbxy, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(nbz, 0, nblock * (nblock + 1) / 2 * sizeof(int));
      memset(bxavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(byavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bzavg, 0, nblock * (nblock + 1) / 2 * sizeof(double));
      memset(bpoornav, 0, nblock * sizeof(bool));
      memset(bxfixstatus, 0, nblock * sizeof(int));
      memset(byfixstatus, 0, nblock * sizeof(int));
      memset(bzfixstatus, 0, nblock * sizeof(int));
      memset(bxfix, 0, nblock * sizeof(double));
      memset(byfix, 0, nblock * sizeof(double));
      memset(bzfix, 0, nblock * sizeof(double));

      /* count ties for all block vs block pairs and calculate average offsets
       * and count dimensions of full inversion problem */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
        crossing = &project.crossings[icrossing];

        /* for block vs block averages use only set crossings between
         * different blocks */
        if (crossing->status == MBNA_CROSSING_STATUS_SET) {
          for (int itie = 0; itie < crossing->num_ties; itie++) {
            /* get tie */
            tie = (struct mbna_tie *)&crossing->ties[itie];

            /* if blocks differ get id for block vs block */
            if (project.files[crossing->file_id_1].block != project.files[crossing->file_id_2].block) {
              if (project.files[crossing->file_id_2].block > project.files[crossing->file_id_1].block) {
                jbvb1 = project.files[crossing->file_id_1].block;
                jbvb2 = project.files[crossing->file_id_2].block;
              }
              else {
                jbvb1 = project.files[crossing->file_id_2].block;
                jbvb2 = project.files[crossing->file_id_1].block;
              }
              jbvb = (jbvb2) * (jbvb2 + 1) / 2 + jbvb1;

              if (tie->status != MBNA_TIE_Z) {
                bxavg[jbvb] += tie->offset_x_m;
                byavg[jbvb] += tie->offset_y_m;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY) {
                bzavg[jbvb] += tie->offset_z_m;
                nbz[jbvb]++;
              }
            }
          }
        }
      }

      /* calculate block vs block tie averages */
      fprintf(stderr, "Survey vs Survey tie counts and average offsets:\n");
      nblockties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            bxavg[jbvb] /= nbxy[jbvb];
            byavg[jbvb] /= nbxy[jbvb];
            nblockties += 2;
          }
          if (nbz[jbvb] > 0) {
            bzavg[jbvb] /= nbz[jbvb];
            nblockties++;
          }
          fprintf(stderr, "%2d vs %2d: %5d xy ties  %5d z ties  Avg offsets: %8.3f %8.3f %8.3f\n", jblock, iblock,
                  nbxy[jbvb], nbz[jbvb], bxavg[jbvb], byavg[jbvb], bzavg[jbvb]);
        }
      }

      /* get fixed blocks and average global ties for blocks */
      mbna_global_tie_influence = 6000;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed and global ties for full inversion */
        for (int isection = 0; isection < file->num_sections; isection++) {
          /* get section */
          section = &file->sections[isection];

          /* count global ties for block offset inversion */
          if (section->global_tie_status != MBNA_TIE_NONE) {
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->offset_x_m;
              byfixstatus[file->block]++;
              byfix[file->block] += section->offset_y_m;
            }
            if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->offset_z_m;
            }
          }
        }
      }

      /* count fixed sections for block average inversion,
       * overwriting global ties if they conflict */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        /* get file */
        file = &project.files[ifile];

        /* count fixed sections for block average inversion,
         * overwriting global ties if they conflict */
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
          bxfixstatus[file->block] = 1;
          bxfix[file->block] = 0.0;
          byfixstatus[file->block] = 1;
          byfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
          bzfixstatus[file->block] = 1;
          bzfix[file->block] = 0.0;
        }
        if (file->status == MBNA_FILE_POORNAV) {
          bpoornav[file->block] = true;
        }
      }
      nblockglobalties = 0;
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          bxfix[iblock] /= (double)bxfixstatus[iblock];
          nblockglobalties++;
        }
        if (byfixstatus[iblock] > 0) {
          byfix[iblock] /= (double)byfixstatus[iblock];
          nblockglobalties++;
        }
        if (bzfixstatus[iblock] > 0) {
          bzfix[iblock] /= (double)bzfixstatus[iblock];
          nblockglobalties++;
        }
      }
    }

    /* We do a two stage inversion first for block averages, then for full
     * adjustement vector on top of the averages. Make sure arrays are
     * allocated large enough for both stages. */
    nrows = nfixed + ntie + nglobal + nsmooth;
    ncols = 3 * nnav;
    nrows_ba = nblockties + nblockglobalties + 3;
    ncols_ba = 3 * nblock;
    nrows_alloc = MAX(nrows, nrows_ba);
    ncols_alloc = MAX(ncols, ncols_ba);
    fprintf(stderr, "\nMBnavadjust block average inversion preparation:\n");
    fprintf(stderr, "     nblock:            %d\n", nblock);
    fprintf(stderr, "     nblockties:        %d\n", nblockties);
    fprintf(stderr, "     nblockglobalties:  %d\n", nblockglobalties);
    fprintf(stderr, "     nrows_ba:          %d\n", nrows_ba);
    fprintf(stderr, "     ncols_ba:          %d\n", ncols_ba);
    fprintf(stderr, "\nMBnavadjust full inversion preparation:\n");
    fprintf(stderr, "     nnav:              %d\n", nnav);
    fprintf(stderr, "     ntie:              %d\n", ntie);
    fprintf(stderr, "     nglobal:           %d\n", nglobal);
    fprintf(stderr, "     nfixed:            %d\n", nfixed);
    fprintf(stderr, "     nsmooth:           %d\n", nsmooth);
    fprintf(stderr, "     nrows:             %d\n", nrows);
    fprintf(stderr, "     ncols:             %d\n", ncols);
    fprintf(stderr, "\nMBnavadjust inversion array allocation dimensions:\n");
    fprintf(stderr, "     nrows_alloc:       %d\n", nrows_alloc);
    fprintf(stderr, "     ncols_alloc:       %d\n", ncols_alloc);

    /* allocate solution vector x, perturbation vector xx, and average solution vector xa */
    matrix.nia = NULL;
    matrix.ia = NULL;
    matrix.a = NULL;
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&u, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&v, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&w, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&x, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, ncols_alloc * sizeof(double), (void **)&se, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(double), (void **)&b, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nrows_alloc * sizeof(int), (void **)&matrix.nia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(int), (void **)&matrix.ia, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, 6 * nrows_alloc * sizeof(double), (void **)&matrix.a, &error);
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
    memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));

    /*----------------------------------------------------------------*/
    /* Create block offset inversion matrix problem                   */
    /*----------------------------------------------------------------*/
    if (nblock > 1) {
      matrix.m = nrows_ba;
      matrix.n = ncols_ba;
      matrix.ia_dim = ncols_ba;

      /* loop over each crossing, applying offsets evenly to both points
          for all ties that involve different blocks
          - weight inversely by number of ties for each block vs block pair
          so that each has same importance whether connected by one tie
          or many */

      /* set up inversion for block offsets
       * - start with average offsets between all block vs block pairs for
       *   x y and z wherever defined by one or more ties
       * - next apply average global ties for each block if they exist
       * - finally add a constraint for x y and z that the sum of all
       *   block offsets must be zero (ignoring blocks tagged as having
       *   poor navigation) */
      irow = 0;

      /* start with average block vs block offsets */
      for (int iblock = 0; iblock < nblock; iblock++) {
        for (int jblock = 0; jblock <= iblock; jblock++) {
          jbvb = (iblock) * (iblock + 1) / 2 + jblock;
          if (nbxy[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bxavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;

            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 1;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = byavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
          if (nbz[jbvb] > 0) {
            index_m = irow * ncols_ba;
            index_n = jblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = -1.0;

            index_m = irow * ncols_ba + 1;
            index_n = iblock * 3 + 2;
            matrix.ia[index_m] = index_n;
            matrix.a[index_m] = 1.0;

            b[irow] = bzavg[jbvb];
            matrix.nia[irow] = 2;
            irow++;
          }
        }
      }

      /* next apply average global offsets for each block */
      for (int iblock = 0; iblock < nblock; iblock++) {
        if (bxfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = bxfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
        }
        if (byfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = byfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
        }
        if (bzfixstatus[iblock] > 0) {
          index_m = irow * ncols_ba;
          index_n = iblock * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = 1.0;

          b[irow] = bzfix[iblock];
          matrix.nia[irow] = 1;
          irow++;
        }
      }

      /* add constraint that overall average offset must be zero, ignoring
       * blocks with poor navigation */
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3 + 1;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;
      for (int iblock = 0; iblock < nblock; iblock++) {
        index_m = irow * ncols_ba + iblock;
        index_n = iblock * 3 + 2;
        matrix.ia[index_m] = index_n;
        if (bpoornav[iblock]) {
          matrix.a[index_m] = 0.0;
        }
        else {
          matrix.a[index_m] = 1.0;
        }
      }
      b[irow] = 0.0;
      matrix.nia[irow] = nblock;
      irow++;

      fprintf(stderr,
              "\nAbout to call LSQR for preliminary block solution   rows: %d cols: %d  (expected rows:%d cols:%d)\n", irow,
              nblock * 3, nrows_ba, ncols_ba);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < nrows_ba; irow++)
        u[irow] = b[irow];
      damp = 0.0;
      atol = 1.0e-6;   // releative precision of A matrix
      btol = 1.0e-6;   // relative precision of data array
      relpr = 1.0e-16; // relative precision of double precision arithmetic
      conlim = 1 / (10 * sqrt(relpr));
      itnlim = 4 * matrix.n;
      // fprintf(stderr,"damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
      //    damp,atol,btol,conlim,itnlim);

      // for (int i=0;i<matrix.m;i++)
      //  {
      //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
      //  for (int j=0;j<matrix.nia[i];j++)
      //    {
      //    k = i * ncols_ba + j;
      //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
      //    }
      //  fprintf(stderr," | b:%10.6f\n",u[i]);
      //  }

      mblsqr_lsqr(nrows_ba, ncols_ba, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr,
                  &istop_out, &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

      /* save solution */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        file->block_offset_x = x[3 * file->block];
        file->block_offset_y = x[3 * file->block + 1];
        file->block_offset_z = x[3 * file->block + 2];
      }

      fprintf(stderr, "\nInversion by LSQR completed\n");
      fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
      fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
      fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out,
              sqrt((double)matrix.n));
      fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
      fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
      fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
      fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
      for (int i = 0; i < nblock; i++) {
        fprintf(stderr, "block[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n", i, x[3 * i], x[3 * i + 1],
                x[3 * i + 2]);
      }
      // for (int ifile=0;ifile<project.num_files;ifile++)
      //  {
      //  file = &project.files[ifile];
      //  fprintf(stderr,"file[%d]:  block_offset_x:%f block_offset_y:%f block_offset_z:%f\n",
      //      ifile,file->block_offset_x,file->block_offset_y,file->block_offset_z);
      //  }

      /* deallocate arrays used only for block inversion */
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbxy, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&nbz, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzavg, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bpoornav, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfixstatus, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bxfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&byfix, &error);
      status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&bzfix, &error);
    }

    /*----------------------------------------------------------------*/
    /* Create complete inversion matrix problem                       */
    /*----------------------------------------------------------------*/

    /* loop over each crossing, applying offsets evenly to both points */
    irow = 0;
    rms_misfit_initial = 0.0;
    matrix.m = nrows;
    matrix.n = ncols;
    matrix.ia_dim = 6;
    memset(u, 0, nrows_alloc * sizeof(double));
    memset(v, 0, ncols_alloc * sizeof(double));
    memset(w, 0, ncols_alloc * sizeof(double));
    memset(x, 0, ncols_alloc * sizeof(double));
    memset(se, 0, ncols_alloc * sizeof(double));
    memset(b, 0, nrows_alloc * sizeof(double));
    memset(matrix.nia, 0, nrows_alloc * sizeof(int));
    memset(matrix.ia, 0, 6 * nrows_alloc * sizeof(int));
    memset(matrix.a, 0, 6 * nrows_alloc * sizeof(double));
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* use only set crossings */
      if (crossing->status == MBNA_CROSSING_STATUS_SET)
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* A: get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          /* A1: get absolute id for first snav point */
          file1 = &project.files[crossing->file_id_1];
          section1 = &file1->sections[crossing->section_1];
          nc1 = section1->snav_invert_id[tie->snav_1];

          /* A2: get absolute id for second snav point */
          file2 = &project.files[crossing->file_id_2];
          section2 = &file2->sections[crossing->section_2];
          nc2 = section2->snav_invert_id[tie->snav_2];

          if (file1->sections[crossing->section_1].snav_time_d[tie->snav_1] ==
              file2->sections[crossing->section_2].snav_time_d[tie->snav_2])
            fprintf(stderr, "ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
                    crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2, crossing->section_2,
                    tie->snav_2,
                    (file1->sections[crossing->section_1].snav_time_d[tie->snav_1] -
                     file2->sections[crossing->section_2].snav_time_d[tie->snav_2]));

          /* A3: get offset vector for this tie */
          if (tie->status != MBNA_TIE_Z) {
            offset_x = tie->offset_x_m - (file2->block_offset_x - file1->block_offset_x);
            offset_y = tie->offset_y_m - (file2->block_offset_y - file1->block_offset_y);
          }
          else {
            offset_x = 0.0;
            offset_y = 0.0;
          }
          if (tie->status != MBNA_TIE_XY) {
            offset_z = tie->offset_z_m - (file2->block_offset_z - file1->block_offset_z);
          }
          else {
            offset_z = 0.0;
          }
          rms_misfit_initial += offset_x * offset_x + offset_y * offset_y + offset_z * offset_z;

          /* deal with each component of the error ellipse
              - project offset vector onto each component by dot-product
          - weight inversely by size of error for that component */

          /* B1: deal with long axis */
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1];
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            projected_offset = offset_x * tie->sigmax1[0] + offset_y * tie->sigmax1[1] + offset_z * tie->sigmax1[2];
          if (fabs(tie->sigmar1) > 0.0)
            weight = 1.0 / tie->sigmar1;
          else
            weight = 0.0;

          index_m = irow * 6;
          index_n = nc1 * 3;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = -weight * tie->sigmax1[0];

          index_m = irow * 6 + 1;
          index_n = nc2 * 3;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = weight * tie->sigmax1[0];

          index_m = irow * 6 + 2;
          index_n = nc1 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = -weight * tie->sigmax1[1];

          index_m = irow * 6 + 3;
          index_n = nc2 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = weight * tie->sigmax1[1];

          index_m = irow * 6 + 4;
          index_n = nc1 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = -weight * tie->sigmax1[2];

          index_m = irow * 6 + 5;
          index_n = nc2 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = weight * tie->sigmax1[2];

          b[irow] = weight * projected_offset;
          matrix.nia[irow] = 6;
          irow++;

          /* B2: deal with horizontal axis */
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1];
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            projected_offset = offset_x * tie->sigmax2[0] + offset_y * tie->sigmax2[1] + offset_z * tie->sigmax2[2];
          if (fabs(tie->sigmar2) > 0.0)
            weight = 1.0 / tie->sigmar2;
          else
            weight = 0.0;

          index_m = irow * 6;
          index_n = nc1 * 3;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = -weight * tie->sigmax2[0];

          index_m = irow * 6 + 1;
          index_n = nc2 * 3;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = weight * tie->sigmax2[0];

          index_m = irow * 6 + 2;
          index_n = nc1 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = -weight * tie->sigmax2[1];

          index_m = irow * 6 + 3;
          index_n = nc2 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_Z)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = weight * tie->sigmax2[1];

          index_m = irow * 6 + 4;
          index_n = nc1 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = -weight * tie->sigmax2[2];

          index_m = irow * 6 + 5;
          index_n = nc2 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = weight * tie->sigmax2[2];

          b[irow] = weight * projected_offset;
          matrix.nia[irow] = 6;
          irow++;

          /* B3:  deal with semi-vertical axis */
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED)
            projected_offset = offset_z * tie->sigmax3[2];
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            projected_offset = offset_x * tie->sigmax3[0] + offset_y * tie->sigmax3[1] + offset_z * tie->sigmax3[2];
          if (fabs(tie->sigmar3) > 0.0)
            weight = 1.0 / tie->sigmar3;
          else
            weight = 0.0;

          index_m = irow * 6;
          index_n = nc1 * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = -weight * tie->sigmax3[0];
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = -weight * tie->sigmax3[0];

          index_m = irow * 6 + 1;
          index_n = nc2 * 3;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = weight * tie->sigmax3[0];

          index_m = irow * 6 + 2;
          index_n = nc1 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = -weight * tie->sigmax3[1];

          index_m = irow * 6 + 3;
          index_n = nc2 * 3 + 1;
          matrix.ia[index_m] = index_n;
          if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
            matrix.a[index_m] = weight * tie->sigmax3[1];

          index_m = irow * 6 + 4;
          index_n = nc1 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = -weight * tie->sigmax3[2];

          index_m = irow * 6 + 5;
          index_n = nc2 * 3 + 2;
          matrix.ia[index_m] = index_n;
          if (tie->status == MBNA_TIE_XY)
            matrix.a[index_m] = 0.0;
          else
            matrix.a[index_m] = weight * tie->sigmax3[2];

          b[irow] = weight * projected_offset;
          matrix.nia[irow] = 6;
          irow++;
        }
    }

    /* C1: loop over all files applying any global ties */
    weight = 10.0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
          index_m = irow * 6;
          index_n = section->snav_invert_id[section->global_tie_snav] * 3;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = weight;
          b[irow] = weight * (section->offset_x_m - file->block_offset_x);
          matrix.nia[irow] = 1;
          irow++;

          index_m = irow * 6;
          index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 1;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = weight;
          b[irow] = weight * (section->offset_y_m - file->block_offset_y);
          matrix.nia[irow] = 1;
          rms_misfit_initial += section->offset_x_m * section->offset_x_m +
                                section->offset_y_m * section->offset_y_m;
          irow++;
        }

        if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
          index_m = irow * 6;
          index_n = section->snav_invert_id[section->global_tie_snav] * 3 + 2;
          matrix.ia[index_m] = index_n;
          matrix.a[index_m] = weight;
          b[irow] = weight * (section->offset_z_m - file->block_offset_z);
          matrix.nia[irow] = 1;
          rms_misfit_initial += section->offset_z_m * section->offset_z_m;
          irow++;
        }
      }
    }
    rms_misfit_initial = sqrt(rms_misfit_initial) / irow;

    /* D1: loop over all files applying ties for any fixed files */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV ||
          file->status == MBNA_FILE_FIXEDZNAV) {
        for (int isection = 0; isection < file->num_sections; isection++) {
          section = &file->sections[isection];
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
              index_m = irow * 6;
              index_n = section->snav_invert_id[isnav] * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = -file->block_offset_x;
              matrix.nia[irow] = 1;
              irow++;

              index_m = irow * 6;
              index_n = section->snav_invert_id[isnav] * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = -file->block_offset_y;
              matrix.nia[irow] = 1;
              irow++;
            }

            if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
              index_m = irow * 6;
              index_n = section->snav_invert_id[isnav] * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = -file->block_offset_z;
              matrix.nia[irow] = 1;
              irow++;
            }
          }
        }
      }
    }

    /* E1: loop over all navigation applying first derivative smoothing */
    smoothweight = pow(10.0, project.smoothing) / 100.0;
    for (int inav = 0; inav < nnav - 1; inav++) {
      if (x_continuity[inav + 1]) {
        if (x_time_d[inav + 1] - x_time_d[inav] > 0.0)
          weight = smoothweight / (x_time_d[inav + 1] - x_time_d[inav]);
        else
          weight = 0.0000001;

        index_m = irow * 6;
        index_n = inav * 3;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = -weight;
        index_m = irow * 6 + 1;
        index_n = (inav + 1) * 3;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = weight;
        b[irow] = 0.0;
        matrix.nia[irow] = 2;
        irow++;

        index_m = irow * 6;
        index_n = inav * 3 + 1;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = -weight;
        index_m = irow * 6 + 1;
        index_n = (inav + 1) * 3 + 1;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = weight;
        b[irow] = 0.0;
        matrix.nia[irow] = 2;
        irow++;

        index_m = irow * 6;
        index_n = inav * 3 + 2;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = -weight;
        index_m = irow * 6 + 1;
        index_n = (inav + 1) * 3 + 2;
        matrix.ia[index_m] = index_n;
        matrix.a[index_m] = weight;
        b[irow] = 0.0;
        matrix.nia[irow] = 2;
        irow++;
      }
    }

    fprintf(stderr, "\nAbout to call LSQR rows: %d %d %d  cols: %d %d\n", matrix.m, nrows, irow, matrix.n, ncols);

    /* F: call lsqr to solve the matrix problem */
    for (int irow = 0; irow < matrix.m; irow++)
      u[irow] = b[irow];
    damp = 0.0;
    atol = 1.0e-6;   // releative precision of A matrix
    btol = 1.0e-6;   // relative precision of data array
    relpr = 1.0e-16; // relative precision of double precision arithmetic
    conlim = 1 / (10 * sqrt(relpr));
    itnlim = 4 * matrix.n;
    // fprintf(stderr, "damp:%f\natol:%f\nbtol:%f\nconlim:%f\nitnlim:%d\n",
    // damp, atol, btol, conlim, itnlim);

    // for (int i=0;i<matrix.m;i++)
    //  {
    //  fprintf(stderr,"A row:%6d nia:%d ",i,matrix.nia[i]);
    //  for (int j=0;j<matrix.nia[i];j++)
    //    {
    //    k = i * 6 + j;
    //    fprintf(stderr,"| %d ia[%5d]:%5d a[%5d]:%10.6f ", j,k,matrix.ia[k],k,matrix.a[k]);
    //    }
    //  fprintf(stderr," | b:%10.6f\n",u[i]);
    //  }

    mblsqr_lsqr(matrix.m, matrix.n, &mb_aprod, damp, &matrix, u, v, w, x, se, atol, btol, conlim, itnlim, stderr, &istop_out,
                &itn_out, &anorm_out, &acond_out, &rnorm_out, &arnorm_out, &xnorm_out);

    fprintf(stderr, "\nInversion by LSQR completed\n");
    fprintf(stderr, "\tReason for termination:       %d\n", istop_out);
    fprintf(stderr, "\tNumber of iterations:         %d\n", itn_out);
    fprintf(stderr, "\tFrobenius norm:               %f\n (expected to be about %f)\n", anorm_out, sqrt((double)matrix.n));
    fprintf(stderr, "\tCondition number of A:        %f\n", acond_out);
    fprintf(stderr, "\tRbar norm:                    %f\n", rnorm_out);
    fprintf(stderr, "\tResidual norm:                %f\n", arnorm_out);
    fprintf(stderr, "\tSolution norm:                %f\n", xnorm_out);
    // for (int i=0;i<matrix.n;i++)
    //  {
    //  j=i/3;
    //  if (i % 3 == 0)
    //    fprintf(stderr,"x[%d]:  x[%d]:%f",i,j,x[i]);
    //  else if (i % 3 == 1)
    //    fprintf(stderr,"  y[%d]:%f",j,x[i]);
    //  else
    //    fprintf(stderr,"  z[%d]:%f\n",j,x[i]);
    //  }

    /* save solution */
    k = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          k = section->snav_invert_id[isnav];
          section->snav_lon_offset[isnav] = (x[3 * k] + file->block_offset_x) * project.mtodeglon;
          section->snav_lat_offset[isnav] = (x[3 * k + 1] + file->block_offset_y) * project.mtodeglat;
          section->snav_z_offset[isnav] = (x[3 * k + 2] + file->block_offset_z);
        }
      }
    }

    /* calculate final misfit */
    irow = 0;
    rms_misfit_current = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* use only set crossings */
      if (crossing->status == MBNA_CROSSING_STATUS_SET)
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          /* get absolute id for first snav point */
          file1 = &project.files[crossing->file_id_1];
          section1 = &file1->sections[crossing->section_1];
          nc1 = section1->snav_invert_id[tie->snav_1];

          /* get absolute id for second snav point */
          file2 = &project.files[crossing->file_id_2];
          section2 = &file2->sections[crossing->section_2];
          nc2 = section2->snav_invert_id[tie->snav_2];

          /* get offset vector for this tie */
          if (tie->status != MBNA_TIE_Z) {
            offset_x = tie->offset_x_m;
            offset_y = tie->offset_y_m;
          }
          else {
            offset_x = 0.0;
            offset_y = 0.0;
          }
          if (tie->status != MBNA_TIE_XY) {
            offset_z = tie->offset_z_m;
          }
          else {
            offset_z = 0.0;
          }
          offset_x -=
              (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
          offset_y -=
              (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
          offset_z -= (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
          rms_misfit_current += offset_x * offset_x + offset_y * offset_y + offset_z * offset_z;
          irow += 3;
        }
    }

    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_XY) {
          offset_x =
              section->offset_x_m - section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
          offset_y =
              section->offset_y_m - section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
          rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
          irow += 2;
        }
        if (section->global_tie_status == MBNA_TIE_XYZ || section->global_tie_status == MBNA_TIE_Z) {
          offset_z = section->offset_z_m - section->snav_z_offset[section->global_tie_snav];
          rms_misfit_current += offset_z * offset_z;
          irow++;
        }
      }
    }
    rms_misfit_current = sqrt(rms_misfit_current) / irow;

    /* set message dialog on */
    sprintf(message, "Completed inversion...");
    do_message_update(message);

    /* update model plot */
    if (project.modelplot)
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);

    /* now output inverse solution */
    sprintf(message, "Outputting navigation solution...");
    do_message_update(message);

    sprintf(message, " > Final misfit:%12g\n > Initial misfit:%12g\n", rms_misfit_current, rms_misfit_initial);
    do_info_add(message, false);

    /* get crossing offset results */
    sprintf(message, " > Nav Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* check only set ties */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = (struct mbna_tie *)&crossing->ties[j];
          offset_x = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon_offset[tie->snav_1];
          offset_y = project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat_offset[tie->snav_1];
          offset_z = project.files[crossing->file_id_2].sections[crossing->section_2].snav_z_offset[tie->snav_2] -
                     project.files[crossing->file_id_1].sections[crossing->section_1].snav_z_offset[tie->snav_1];

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(offset_x) > 10000.0 || fabs(offset_y) > 10000.0 || fabs(offset_z) > 10000.0) {
            tie->inversion_status = MBNA_INVERSION_OLD;
            tie->inversion_offset_x = 0.0;
            tie->inversion_offset_y = 0.0;
            tie->inversion_offset_x_m = 0.0;
            tie->inversion_offset_y_m = 0.0;
            tie->inversion_offset_z_m = 0.0;
            tie->dx_m = 0.0;
            tie->dy_m = 0.0;
            tie->dz_m = 0.0;
            tie->sigma_m = 0.0;
            tie->dr1_m = 0.0;
            tie->dr2_m = 0.0;
            tie->dr3_m = 0.0;
            tie->rsigma_m = 0.0;
          }
          else {
            tie->inversion_status = MBNA_INVERSION_CURRENT;
            tie->inversion_offset_x = offset_x;
            tie->inversion_offset_y = offset_y;
            tie->inversion_offset_x_m = offset_x / project.mtodeglon;
            tie->inversion_offset_y_m = offset_y / project.mtodeglat;
            tie->inversion_offset_z_m = offset_z;
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

          sprintf(message, " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f\n",
                  icrossing, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                            tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                            tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m);
          do_info_add(message, false);
        }
      }
    }

    /* get global tie results */
    sprintf(message, " > Global Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->global_tie_status != MBNA_TIE_NONE) {

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(section->snav_lon_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_lat_offset[section->global_tie_snav]) > 10000.0
                        || fabs(section->snav_z_offset[section->global_tie_snav]) > 10000.0) {
            section->global_tie_inversion_status = MBNA_INVERSION_OLD;
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
          }
          else {
            section->global_tie_inversion_status = MBNA_INVERSION_CURRENT;
                        section->inversion_offset_x = section->snav_lon_offset[section->global_tie_snav];
                        section->inversion_offset_y = section->snav_lat_offset[section->global_tie_snav];
                        section->inversion_offset_x_m = section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon;
                        section->inversion_offset_y_m = section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat;
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
                    sprintf(message,
                            " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
                            ifile, isection, section->global_tie_snav, section->global_tie_status,
                            section->offset_x_m, section->offset_y_m, section->offset_z_m,
                            section->inversion_offset_x_m, section->inversion_offset_y_m, section->inversion_offset_z_m,
                            section->dx_m, section->dy_m, section->dz_m);
                    do_info_add(message, false);
        }
      }
    }

    /* write updated project */
    project.inversion_status = MBNA_INVERSION_CURRENT;
    project.grid_status = MBNA_GRID_OLD;
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;

    /* deallocate arrays */
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_continuity, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x_time_d, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&u, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&v, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&w, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&x, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&se, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&b, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.nia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.ia, &error);
    status = mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&matrix.a, &error);

    /* turn off message dialog */
    do_message_off();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_updategrid() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  char npath[STRING_MAX];
  char apath[STRING_MAX];
  char spath[STRING_MAX];
  char command[STRING_MAX];
  FILE *nfp, *afp;
  char *result;
  char buffer[BUFFER_MAX];
  int nscan;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double speed;
  double draft;
  double roll;
  double pitch;
  double heave;
  double factor;
  double zoffset;
  char ostring[STRING_MAX];
  int isection, isnav;
  double seconds;
  double lon_min, lon_max, lat_min, lat_max;

  /* generate current topography grid */
  if (project.open && project.num_files > 0 && error == MB_ERROR_NO_ERROR) {
    /* set message */
    sprintf(message, "Setting up to generate current topography grid...");
    do_message_on(message);
    do_info_add(message, false);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);

    /* update datalist files and mbgrid commands */
    sprintf(apath, "%s/datalist.mb-1", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      for (int i = 0; i < project.num_files; i++) {
        file = &project.files[i];
        //if (file->status != MBNA_FILE_FIXEDNAV) {
          for (int j = 0; j < file->num_sections; j++) {
            fprintf(afp, "nvs_%4.4d_%4.4d.mb71 71\n", file->id, j);
          }
        //}
      }
      fclose(afp);
    }

    lon_min = project.lon_min - 0.1 * (project.lon_max - project.lon_min);
    lon_max = project.lon_max + 0.1 * (project.lon_max - project.lon_min);
    lat_min = project.lat_min - 0.1 * (project.lat_max - project.lat_min);
    lat_max = project.lat_max + 0.1 * (project.lat_max - project.lat_min);
    sprintf(apath, "%s/mbgrid_adj.cmd", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      fprintf(afp, "mbgrid -I datalistp.mb-1 \\\n\t-R%.8f/%.8f/%.8f/%.8f \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopoAdj\n\n",
              lon_min, lon_max, lat_min, lat_max);
      fclose(afp);
    }

    sprintf(command, "chmod +x %s/mbgrid_adj.cmd", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);

    /* run mbdatalist to create datalistp.mb-1, update ancillary files,
        and clear any processing lock files */
    sprintf(message, " > Running mbdatalist in project\n");
    do_info_add(message, false);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
    sprintf(command, "cd %s ; mbdatalist -Idatalist.mb-1 -O -Y -Z -V", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);

    if (project.inversion_status != MBNA_INVERSION_NONE) {
      /* set message */
      sprintf(message, "Applying navigation solution within the project...");
      do_message_on(message);
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);

      /* generate new nav files */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        sprintf(npath, "%s/nvs_%4.4d.mb166", project.datadir, ifile);
        sprintf(apath, "%s/nvs_%4.4d.na%d", project.datadir, ifile, file->output_id);
        /*
        if (file->status == MBNA_FILE_FIXEDNAV) {
          sprintf(message, " > Not outputting updated nav to fixed file %s\n", apath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else */
        if ((nfp = fopen(npath, "r")) == NULL) {
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          sprintf(message, " > Unable to read initial nav file %s\n", npath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else if ((afp = fopen(apath, "w")) == NULL) {
          fclose(nfp);
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          sprintf(message, " > Unable to open output nav file %s\n", apath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else {
          sprintf(message, " > Output updated nav to %s\n", apath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);

          /* write file header */
          right_now = time((time_t *)0);
          strcpy(date, ctime(&right_now));
          date[strlen(date) - 1] = '\0';
          if ((user_ptr = getenv("USER")) == NULL)
            user_ptr = getenv("LOGNAME");
          if (user_ptr != NULL)
            strcpy(user, user_ptr);
          else
            strcpy(user, "unknown");
          gethostname(host, MBP_FILENAMESIZE);
          sprintf(ostring, "# Adjusted navigation generated using MBnavadjust\n");
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# MB-System version:        %s\n", MB_VERSION);
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# MB-System build data:     %s\n", MB_BUILD_DATE);
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# MBnavadjust project name: %s\n", project.name);
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# MBnavadjust project path: %s\n", project.path);
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# MBnavadjust project home: %s\n", project.home);
          fprintf(afp, "%s", ostring);
          sprintf(ostring, "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
          fprintf(afp, "%s", ostring);

          /* read the input nav */
          isection = 0;
          section = &file->sections[isection];
          isnav = 0;
          bool done = false;
          while (!done) {
            if ((result = fgets(buffer, BUFFER_MAX, nfp)) != buffer) {
              done = true;
            }
            else if ((nscan = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0],
                                     &time_i[1], &time_i[2], &time_i[3], &time_i[4], &seconds, &time_d, &navlon,
                                     &navlat, &heading, &speed, &draft, &roll, &pitch, &heave)) >= 11) {
              /* get integer seconds and microseconds */
              time_i[5] = (int)floor(seconds);
              time_i[6] = (int)((seconds - (double)time_i[5]) * 1000000);

              /* fix nav from early version */
              if (nscan < 15) {
                draft = 0.0;
                roll = 0.0;
                pitch = 0.0;
                heave = 0.0;
              }

              /* get next snav interval if needed */
              while (time_d > section->snav_time_d[isnav + 1] &&
                     !(isection == file->num_sections - 1 && isnav == section->num_snav - 2)) {
                if (isnav < section->num_snav - 2) {
                  isnav++;
                }
                else if (isection < file->num_sections - 1) {
                  isection++;
                  section = &file->sections[isection];
                  isnav = 0;
                }
              }

              /* update the nav if possible (and it should be...) */
              if (time_d < section->snav_time_d[isnav]) {
                factor = 0.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
ifile, isection, isnav);
              }
              else if (time_d > section->snav_time_d[isnav + 1]) {
                factor = 1.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
ifile, isection, isnav+1);
              }
              else {
                if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                  factor = (time_d - section->snav_time_d[isnav]) /
                           (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
                  /*if (fabs(time_d - section->snav_time_d[isnav]) < 1.0)
                  fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav]);
                  else if (fabs(time_d - section->snav_time_d[isnav+1]) < 1.0)
                  fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav+1]);*/
                }
                else {
                  factor = 0.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f >= %f ifile:%d isection:%d isnav:%d\n",
__FILE__, __LINE__, __func__, section->snav_time_d[isnav], section->snav_time_d[isnav + 1],
ifile, isection, isnav);
                }
              }

              /* update and output only nonzero navigation */
              if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001) {
                navlon += section->snav_lon_offset[isnav] +
                          factor * (section->snav_lon_offset[isnav + 1] - section->snav_lon_offset[isnav]);
                navlat += section->snav_lat_offset[isnav] +
                          factor * (section->snav_lat_offset[isnav + 1] - section->snav_lat_offset[isnav]);
                zoffset = section->snav_z_offset[isnav] +
                          factor * (section->snav_z_offset[isnav + 1] - section->snav_z_offset[isnav]);

                /* write the updated nav out */
                sprintf(ostring,
                        "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f "
                        "%.2f %.3f\r\n",
                        time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d,
                        navlon, navlat, heading, speed, draft, roll, pitch, heave, zoffset);
                fprintf(afp, "%s", ostring);
                /* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring);
                 */
              }
            }
          }
          fclose(nfp);
          fclose(afp);

          /* update output file in mbprocess parameter file for each section file */
          for (int isection = 0; isection < file->num_sections; isection++) {
            section = &(file->sections[isection]);

            sprintf(spath, "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, isection);

            status = mb_pr_update_format(mbna_verbose, spath, true, 71, &error);
            status = mb_pr_update_navadj(mbna_verbose, spath, MBP_NAVADJ_LLZ, apath, MBP_NAV_LINEAR, &error);
          }
        }
      }

      /* run mbprocess */
      sprintf(message, " > Running mbprocess in project\n");
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      sprintf(command, "cd %s ; mbprocess", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
    }

    if (project.grid_status != MBNA_GRID_CURRENT) {
      /* run mbgrid */
      sprintf(message, " > Running mbgrid_adj\n");
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      sprintf(command, "cd %s ; ./mbgrid_adj.cmd", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
      project.grid_status = MBNA_GRID_CURRENT;

      /* write current project */
      mbnavadjust_write_project(mbna_verbose, &project, &error);
      project.save_count = 0;

      /* turn off message dialog */
      do_message_off();
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_applynav() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  char npath[STRING_MAX];
  char apath[STRING_MAX];
  char opath[STRING_MAX];
  FILE *nfp, *afp, *ofp;
  char *result;
  char buffer[BUFFER_MAX];
  int nscan;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double heading;
  double speed;
  double draft;
  double roll;
  double pitch;
  double heave;
  double factor;
  double zoffset;
  char ostring[STRING_MAX];
  int mbp_heading_mode;
  double mbp_headingbias;
  int mbp_rollbias_mode;
  double mbp_rollbias;
  double mbp_rollbias_port;
  double mbp_rollbias_stbd;
  int isection, isnav;
  double seconds;

  /* output results from navigation solution */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      error == MB_ERROR_NO_ERROR) {

    /* now output inverse solution */
    sprintf(message, "Applying navigation solution...");
    do_message_on(message);

    /* generate new nav files */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      sprintf(npath, "%s/nvs_%4.4d.mb166", project.datadir, ifile);
      sprintf(apath, "%s/nvs_%4.4d.na%d", project.datadir, ifile, file->output_id);
      sprintf(opath, "%s.na%d", file->path, file->output_id);
      /*
      if (file->status == MBNA_FILE_FIXEDNAV) {
        sprintf(message, " > Not outputting updated nav to fixed file %s\n", opath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      } else */
      if ((nfp = fopen(npath, "r")) == NULL) {
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        sprintf(message, " > Unable to read initial nav file %s\n", npath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else if ((afp = fopen(apath, "w")) == NULL) {
        fclose(nfp);
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        sprintf(message, " > Unable to open output nav file %s\n", apath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else if ((ofp = fopen(opath, "w")) == NULL) {
        fclose(nfp);
        fclose(afp);
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        sprintf(message, " > Unable to open output nav file %s\n", opath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else {
        sprintf(message, " > Output updated nav to %s\n", opath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);

        /* write file header */
        right_now = time((time_t *)0);
        strcpy(date, ctime(&right_now));
        date[strlen(date) - 1] = '\0';
        if ((user_ptr = getenv("USER")) == NULL)
          user_ptr = getenv("LOGNAME");
        if (user_ptr != NULL)
          strcpy(user, user_ptr);
        else
          strcpy(user, "unknown");
        gethostname(host, MBP_FILENAMESIZE);
        sprintf(ostring, "# Adjusted navigation generated using MBnavadjust\n");
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# MB-System version:        %s\n", MB_VERSION);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# MB-System build data:     %s\n", MB_BUILD_DATE);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# MBnavadjust project name: %s\n", project.name);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# MBnavadjust project path: %s\n", project.path);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# MBnavadjust project home: %s\n", project.home);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);
        sprintf(ostring, "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(ofp, "%s", ostring);
        fprintf(afp, "%s", ostring);

        /* read the input nav */
        isection = 0;
        section = &file->sections[isection];
        isnav = 0;
        bool done = false;
        while (!done) {
          if ((result = fgets(buffer, BUFFER_MAX, nfp)) != buffer) {
            done = true;
          }
          else if ((nscan = sscanf(buffer, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf", &time_i[0],
                                   &time_i[1], &time_i[2], &time_i[3], &time_i[4], &seconds, &time_d, &navlon, &navlat,
                                   &heading, &speed, &draft, &roll, &pitch, &heave)) >= 11) {
            /* get integer seconds and microseconds */
            time_i[5] = (int)floor(seconds);
            time_i[6] = (int)((seconds - (double)time_i[5]) * 1000000);

            /* fix nav from early version */
            if (nscan < 15) {
              draft = 0.0;
              roll = 0.0;
              pitch = 0.0;
              heave = 0.0;
            }

            /* get next snav interval if needed */
            while (time_d > section->snav_time_d[isnav + 1] &&
                   !(isection == file->num_sections - 1 && isnav == section->num_snav - 2)) {
              if (isnav < section->num_snav - 2) {
                isnav++;
              }
              else if (isection < file->num_sections - 1) {
                isection++;
                section = &file->sections[isection];
                isnav = 0;
              }
            }

            /* update the nav if possible (and it should be...) */
            if (time_d < section->snav_time_d[isnav]) {
              factor = 0.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
ifile, isection, isnav);
            }
            else if (time_d > section->snav_time_d[isnav + 1]) {
              factor = 1.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
ifile, isection, isnav+1);
            }
            else {
              if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                factor = (time_d - section->snav_time_d[isnav]) /
                         (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
                /*if (fabs(time_d - section->snav_time_d[isnav]) < 1.0)
                fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav]);
                else if (fabs(time_d - section->snav_time_d[isnav+1]) < 1.0)
                fprintf(stderr,"%f %f\n",pitch,section->snav_z_offset[isnav+1]);*/
              }
                else {
                  factor = 0.0;
fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f >= %f ifile:%d isection:%d isnav:%d\n",
__FILE__, __LINE__, __func__, section->snav_time_d[isnav], section->snav_time_d[isnav + 1],
ifile, isection, isnav);
                }
            }

            /* update and output only nonzero navigation */
            if (fabs(navlon) > 0.0000001 && fabs(navlat) > 0.0000001) {
              navlon += section->snav_lon_offset[isnav] +
                        factor * (section->snav_lon_offset[isnav + 1] - section->snav_lon_offset[isnav]);
              navlat += section->snav_lat_offset[isnav] +
                        factor * (section->snav_lat_offset[isnav + 1] - section->snav_lat_offset[isnav]);
              zoffset = section->snav_z_offset[isnav] +
                        factor * (section->snav_z_offset[isnav + 1] - section->snav_z_offset[isnav]);

              /* write the updated nav out */
              sprintf(ostring,
                      "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f %.2f "
                      "%.3f\r\n",
                      time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d, navlon,
                      navlat, heading, speed, draft, roll, pitch, heave, zoffset);
              fprintf(ofp, "%s", ostring);
              fprintf(afp, "%s", ostring);
              /* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring); */
            }
          }
        }
        fclose(nfp);
        fclose(afp);
        fclose(ofp);

        /* get bias values */
        mb_pr_get_heading(mbna_verbose, file->path, &mbp_heading_mode, &mbp_headingbias, &error);
        mb_pr_get_rollbias(mbna_verbose, file->path, &mbp_rollbias_mode, &mbp_rollbias, &mbp_rollbias_port,
                           &mbp_rollbias_stbd, &error);

        /* update output file in mbprocess parameter file */
        status = mb_pr_update_format(mbna_verbose, file->path, true, file->format, &error);
        status = mb_pr_update_navadj(mbna_verbose, file->path, MBP_NAVADJ_LLZ, opath, MBP_NAV_LINEAR, &error);

        /* update heading bias in mbprocess parameter file */
        mbp_headingbias = file->heading_bias + file->heading_bias_import;
        if (mbp_headingbias == 0.0) {
          if (mbp_heading_mode == MBP_HEADING_OFF || mbp_heading_mode == MBP_HEADING_OFFSET)
            mbp_heading_mode = MBP_HEADING_OFF;
          else if (mbp_heading_mode == MBP_HEADING_CALC || mbp_heading_mode == MBP_HEADING_CALCOFFSET)
            mbp_heading_mode = MBP_HEADING_CALC;
        }
        else {
          if (mbp_heading_mode == MBP_HEADING_OFF || mbp_heading_mode == MBP_HEADING_OFFSET)
            mbp_heading_mode = MBP_HEADING_OFFSET;
          else if (mbp_heading_mode == MBP_HEADING_CALC || mbp_heading_mode == MBP_HEADING_CALCOFFSET)
            mbp_heading_mode = MBP_HEADING_CALCOFFSET;
        }
        status = mb_pr_update_heading(mbna_verbose, file->path, mbp_heading_mode, mbp_headingbias, &error);

        /* update roll bias in mbprocess parameter file */
        mbp_rollbias = file->roll_bias + file->roll_bias_import;
        if (mbp_rollbias == 0.0) {
          if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
            mbp_rollbias_port = mbp_rollbias + mbp_rollbias_port - file->roll_bias_import;
            mbp_rollbias_stbd = mbp_rollbias + mbp_rollbias_stbd - file->roll_bias_import;
          }
          else
            mbp_rollbias_mode = MBP_ROLLBIAS_OFF;
        }
        else {
          if (mbp_rollbias_mode == MBP_ROLLBIAS_DOUBLE) {
            mbp_rollbias_port = mbp_rollbias + mbp_rollbias_port - file->roll_bias_import;
            mbp_rollbias_stbd = mbp_rollbias + mbp_rollbias_stbd - file->roll_bias_import;
          }
          else {
            mbp_rollbias_mode = MBP_ROLLBIAS_SINGLE;
          }
        }
        status = mb_pr_update_rollbias(mbna_verbose, file->path, mbp_rollbias_mode, mbp_rollbias, mbp_rollbias_port,
                                       mbp_rollbias_stbd, &error);
      }
    }

    /* turn off message dialog */
    do_message_off();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_set_modelplot_graphics(void *mp_xgid, int *mp_brdr) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       mp_xgid:      %p\n", mp_xgid);
    fprintf(stderr, "dbg2       mp_brdr:      %d %d %d %d\n", mp_brdr[0], mp_brdr[1], mp_brdr[2], mp_brdr[3]);
  }

  /* set graphics id */
  pmodp_xgid = mp_xgid;

  /* set borders */
  for (int i = 0; i < 4; i++) {
    modp_borders[i] = mp_brdr[i];
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_setzoom() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  int xo;
  int plot_width;
  double xscale;
  int ipingstart, ipingend;
  int itiestart, itieend;

  /* plot zoom if active */
  if ((mbna_modelplot_zoom_x1 >= 0 || mbna_modelplot_zoom_x2 >= 0) && mbna_modelplot_zoom_x1 != mbna_modelplot_zoom_x2) {
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
      xo = 5 * MBNA_MODELPLOT_X_SPACE;
      xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);

      ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_start;
      ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
      ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_start;
      ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

      if (ipingend > ipingstart) {
        mbna_modelplot_zoom = true;
        mbna_modelplot_startzoom = ipingstart;
        mbna_modelplot_endzoom = ipingend;
      }
      else
        mbna_modelplot_zoom = false;
    }

    else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
      plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
      xo = 5 * MBNA_MODELPLOT_X_SPACE;
      xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);

      ipingstart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_start;
      ipingstart = MIN(MAX(ipingstart, 0), project.num_pings - 1);
      ipingend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - xo) / xscale + mbna_modelplot_start;
      ipingend = MIN(MAX(ipingend, 0), project.num_pings - 1);

      if (ipingend > ipingstart) {
        mbna_modelplot_zoom = true;
        mbna_modelplot_startzoom = ipingstart;
        mbna_modelplot_endzoom = ipingend;
      }
      else
        mbna_modelplot_zoom = false;
    }

    else {
      itiestart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale;
      itieend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale;
      itiestart = MAX(0, itiestart);
      itieend = MIN(mbna_num_ties_plot - 1, itieend);
      if (itieend > itiestart) {
        mbna_modelplot_tiezoom = true;
        mbna_modelplot_tiestartzoom = itiestart;
        mbna_modelplot_tieendzoom = itieend;
      }
      else
        mbna_modelplot_tiezoom = false;
    }

    mbna_modelplot_zoom_x1 = 0;
    mbna_modelplot_zoom_x2 = 0;
  }

  /* reset zoom to off otherwise */
  else {
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      mbna_modelplot_zoom = false;
      mbna_modelplot_start = 0;
      mbna_modelplot_end = project.num_pings - 1;
    }
    else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
      mbna_modelplot_zoom = false;
      mbna_modelplot_start = 0;
      mbna_modelplot_end = project.num_pings - 1;
    }
    else {
      mbna_modelplot_tiezoom = false;
      mbna_modelplot_tiestart = 0;
      mbna_modelplot_tieend = mbna_num_ties_plot - 1;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_pick(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  /* find nearest snav pt with tie */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      mbnavadjust_modelplot_pick_timeseries(x, y);
    }
    else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
      mbnavadjust_modelplot_pick_perturbation(x, y);
    }
    else {
      mbnavadjust_modelplot_pick_tieoffsets(x, y);
    }
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_pick_timeseries(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  int range;
  int rangemin;
  int pick_crossing;
  int pick_tie;
  int pick_file;
  int pick_section;
  int pick_snav;
  int ntieselect;
  int ix, iy, iping;

  /* find nearest snav pt with tie */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    rangemin = 10000000;
    fprintf(stderr, "mbnavadjust_modelplot_pick_timeseries: %d %d\n", x, y);
    /* search by looping over crossings */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);

      /* loop over all ties for this crossing */
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &(crossing->ties[j]);

        /* handle first snav point */
        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];

        if (section->show_in_modelplot) {
          iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }
        }

        /* handle second snav point */
        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];

        if (section->show_in_modelplot) {
          iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }

          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }

          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }
        }
      }
    }

    /* deal with successful pick */
    if (rangemin < 10000000) {
      /* count the number of ties associated with the selected snav point */
      ntieselect = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* loop over all ties for this crossing */
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = &(crossing->ties[j]);

          /* handle first snav point */
          file = &project.files[crossing->file_id_1];
          section = &file->sections[crossing->section_1];
          if (pick_file == crossing->file_id_1 && pick_section == crossing->section_1 && pick_snav == tie->snav_1)
            ntieselect++;

          /* handle second snav point */
          file = &project.files[crossing->file_id_2];
          section = &file->sections[crossing->section_2];
          if (pick_file == crossing->file_id_2 && pick_section == crossing->section_2 && pick_snav == tie->snav_2)
            ntieselect++;
        }
      }

      /* if only one tie go ahead and select it and open it in naverr */
      if (ntieselect == 1) {
        mbna_crossing_select = pick_crossing;
        mbna_tie_select = pick_tie;
        mbna_modelplot_pickfile = MBNA_SELECT_NONE;
        mbna_modelplot_picksection = MBNA_SELECT_NONE;
        mbna_modelplot_picksnav = MBNA_SELECT_NONE;

        /* bring up naverr window if required */
        if (!mbna_naverr_load) {
          do_naverr_init();
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_update_naverr();
          do_update_status();
        }
      }

      /* else if multiple ties */
      else if (ntieselect > 1) {
        mbna_modelplot_pickfile = pick_file;
        mbna_modelplot_picksection = pick_section;
        mbna_modelplot_picksnav = pick_snav;
      }

            /* set flag to replot modelplot */
            project.modelplot_uptodate = false;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_pick_perturbation(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  int range;
  int rangemin;
  int pick_crossing;
  int pick_tie;
  int pick_file;
  int pick_section;
  int pick_snav;
  int ntieselect;
  int ix, iy, iping;

  /* find nearest snav pt with tie */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    rangemin = 10000000;
    fprintf(stderr, "mbnavadjust_modelplot_pick_perturbation: %d %d\n", x, y);
    /* search by looping over crossings */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);

      /* loop over all ties for this crossing */
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &(crossing->ties[j]);

        /* handle first snav point */
        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];

        if (section->show_in_modelplot) {
          iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_1;
            pick_section = crossing->section_1;
            pick_snav = tie->snav_1;
          }
        }

        /* handle second snav point */
        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];

        if (section->show_in_modelplot) {
          iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }

          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }

          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }
        }
      }
    }

    /* deal with successful pick */
    if (rangemin < 10000000) {
      /* count the number of ties associated with the selected snav point */
      ntieselect = 0;
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* loop over all ties for this crossing */
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = &(crossing->ties[j]);

          /* handle first snav point */
          file = &project.files[crossing->file_id_1];
          section = &file->sections[crossing->section_1];
          if (pick_file == crossing->file_id_1 && pick_section == crossing->section_1 && pick_snav == tie->snav_1)
            ntieselect++;

          /* handle second snav point */
          file = &project.files[crossing->file_id_2];
          section = &file->sections[crossing->section_2];
          if (pick_file == crossing->file_id_2 && pick_section == crossing->section_2 && pick_snav == tie->snav_2)
            ntieselect++;
        }
      }

      /* if only one tie go ahead and select it and open it in naverr */
      if (ntieselect == 1) {
        mbna_crossing_select = pick_crossing;
        mbna_tie_select = pick_tie;
        mbna_modelplot_pickfile = MBNA_SELECT_NONE;
        mbna_modelplot_picksection = MBNA_SELECT_NONE;
        mbna_modelplot_picksnav = MBNA_SELECT_NONE;

        /* bring up naverr window if required */
        if (!mbna_naverr_load) {
          do_naverr_init();
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_update_naverr();
          do_update_status();
        }
      }

      /* else if multiple ties */
      else if (ntieselect > 1) {
        mbna_modelplot_pickfile = pick_file;
        mbna_modelplot_picksection = pick_section;
        mbna_modelplot_picksnav = pick_snav;
      }

            /* set flag to replot modelplot */
            project.modelplot_uptodate = false;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_pick_tieoffsets(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  int range;
  int rangemin;
  int pick_crossing;
  int pick_tie;
  // int pick_file;
  // int pick_section;
  // int pick_snav;
  int ix, iy;

  /* find nearest snav pt with tie */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    rangemin = 10000000;

    /* search by looping over crossings */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);

      /* loop over all ties for this crossing */
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &(crossing->ties[j]);

        /* handle first snav point */
        // struct mbna_file *file = &project.files[crossing->file_id_1];
        // struct mbna_section *section = &file->sections[crossing->section_1];

        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

        iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
          // pick_file = crossing->file_id_1;
          // pick_section = crossing->section_1;
          // pick_snav = tie->snav_1;
        }

        iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * tie->offset_y_m);
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
          // pick_file = crossing->file_id_1;
          // pick_section = crossing->section_1;
          // pick_snav = tie->snav_1;
        }

        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * tie->offset_z_m);
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
          // pick_file = crossing->file_id_1;
          // pick_section = crossing->section_1;
          // pick_snav = tie->snav_1;
        }
      }
    }

    /* deal with successful pick */
    if (rangemin < 10000000) {
      mbna_crossing_select = pick_crossing;
      mbna_tie_select = pick_tie;
      /* mbna_modelplot_pickfile = MBNA_SELECT_NONE; */
      mbna_modelplot_picksection = MBNA_SELECT_NONE;
      mbna_modelplot_picksnav = MBNA_SELECT_NONE;

      /* bring up naverr window if required */
      if (!mbna_naverr_load) {
        do_naverr_init();
      }

      /* else if naverr window is up, load selected crossing */
      else {
        mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_update_naverr();
        do_update_status();
      }

            /* set flag to replot modelplot */
            project.modelplot_uptodate = false;
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_middlepick(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  int range;
  int rangemin;
  int pick_crossing;
  int pick_tie;
  // int pick_file;
  // int pick_section;
  // int pick_snav;
  int ix, iy, iping;

  /* handle middle button pick */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    /* middle pick for timeseries plot is either choosing one of multiple available
        ties from a tied crossing (left button) pick, or if that is not the
        situation, picking the nearest untied crossing */
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      /* first snav pick had multiple ties - now pick which one to use */
      if (mbna_modelplot_pickfile != MBNA_SELECT_NONE) {
        rangemin = 10000000;

        for (int i = 0; i < project.num_crossings; i++) {
          /* check if this crossing includes the picked snav */
          crossing = &(project.crossings[i]);

          /* check first snav */
          if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection) {
            /* loop over the ties */
            for (int j = 0; j < crossing->num_ties; j++) {
              tie = &(crossing->ties[j]);
              if (tie->snav_1 == mbna_modelplot_picksnav) {
                file = &project.files[crossing->file_id_2];
                section = &file->sections[crossing->section_2];
                iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
              }
            }
          }

          /* check second snav */
          if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection) {
            /* loop over the ties */
            for (int j = 0; j < crossing->num_ties; j++) {
              tie = &(crossing->ties[j]);
              if (tie->snav_2 == mbna_modelplot_picksnav) {
                file = &project.files[crossing->file_id_1];
                section = &file->sections[crossing->section_1];
                iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
              }
            }
          }
        }

        /* deal with successful pick */
        if (rangemin < 10000000) {
          /* select tie and open it in naverr */
          mbna_crossing_select = pick_crossing;
          mbna_tie_select = pick_tie;
          mbna_modelplot_pickfile = MBNA_SELECT_NONE;
          mbna_modelplot_picksection = MBNA_SELECT_NONE;
          mbna_modelplot_picksnav = MBNA_SELECT_NONE;

          /* bring up naverr window if required */
          if (!mbna_naverr_load) {
            do_naverr_init();
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_update_naverr();
            do_update_status();
          }

                /* set flag to replot modelplot */
                project.modelplot_uptodate = false;
        }
      }

      /* else pick closest untied crossing */
      else {
        rangemin = 10000000;

        /* search by looping over crossings */
        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &(project.crossings[i]);

          /* check only untied crossings */
          if (crossing->num_ties == 0) {
            file = &project.files[crossing->file_id_1];
            section = &file->sections[crossing->section_1];

            iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            /* handle second snav point */
            file = &project.files[crossing->file_id_2];
            section = &file->sections[crossing->section_2];

            iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }
          }
        }

        /* deal with successful pick */
        if (rangemin < 10000000) {
          mbna_crossing_select = pick_crossing;
          mbna_tie_select = MBNA_SELECT_NONE;
          mbna_modelplot_pickfile = MBNA_SELECT_NONE;
          mbna_modelplot_picksection = MBNA_SELECT_NONE;
          mbna_modelplot_picksnav = MBNA_SELECT_NONE;

          /* bring up naverr window if required */
          if (!mbna_naverr_load) {
            do_naverr_init();
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_update_naverr();
            do_update_status();
          }

                /* set flag to replot modelplot */
                project.modelplot_uptodate = false;
        }
      }
    }

    /* middle pick for perturbation plot is either choosing one of multiple available
        ties from a tied crossing (left button) pick, or if that is not the
        situation, picking the nearest untied crossing */
    else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
      /* first snav pick had multiple ties - now pick which one to use */
      if (mbna_modelplot_pickfile != MBNA_SELECT_NONE) {
        rangemin = 10000000;

        for (int i = 0; i < project.num_crossings; i++) {
          /* check if this crossing includes the picked snav */
          crossing = &(project.crossings[i]);

          /* check first snav */
          if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection) {
            /* loop over the ties */
            for (int j = 0; j < crossing->num_ties; j++) {
              tie = &(crossing->ties[j]);
              if (tie->snav_1 == mbna_modelplot_picksnav) {
                file = &project.files[crossing->file_id_2];
                section = &file->sections[crossing->section_2];
                iping = section->modelplot_start_count + section->snav_id[tie->snav_2];
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_2;
                  // pick_section = crossing->section_2;
                  // pick_snav = tie->snav_2;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
              }
            }
          }

          /* check second snav */
          if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection) {
            /* loop over the ties */
            for (int j = 0; j < crossing->num_ties; j++) {
              tie = &(crossing->ties[j]);
              if (tie->snav_2 == mbna_modelplot_picksnav) {
                file = &project.files[crossing->file_id_1];
                section = &file->sections[crossing->section_1];
                iping = section->modelplot_start_count + section->snav_id[tie->snav_1];
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/

                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
                range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
                if (range < rangemin) {
                  rangemin = range;
                  pick_crossing = i;
                  pick_tie = j;
                  // pick_file = crossing->file_id_1;
                  // pick_section = crossing->section_1;
                  // pick_snav = tie->snav_1;
                }
                /*fprintf(stderr,"range:%d rangemin:%d\n",range,rangemin);*/
              }
            }
          }
        }

        /* deal with successful pick */
        if (rangemin < 10000000) {
          /* select tie and open it in naverr */
          mbna_crossing_select = pick_crossing;
          mbna_tie_select = pick_tie;
          mbna_modelplot_pickfile = MBNA_SELECT_NONE;
          mbna_modelplot_picksection = MBNA_SELECT_NONE;
          mbna_modelplot_picksnav = MBNA_SELECT_NONE;

          /* bring up naverr window if required */
          if (!mbna_naverr_load) {
            do_naverr_init();
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_update_naverr();
            do_update_status();
          }

                /* set flag to replot modelplot */
                project.modelplot_uptodate = false;
        }
      }

      /* else pick closest untied crossing */
      else {
        rangemin = 10000000;

        /* search by looping over crossings */
        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &(project.crossings[i]);

          /* check only untied crossings */
          if (crossing->num_ties == 0) {
            file = &project.files[crossing->file_id_1];
            section = &file->sections[crossing->section_1];

            iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale *
                                             (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_1;
              // pick_section = crossing->section_1;
              // pick_snav = section->num_snav / 2;
            }

            /* handle second snav point */
            file = &project.files[crossing->file_id_2];
            section = &file->sections[crossing->section_2];

            iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }

            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale *
                                             (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_crossing = i;
              pick_tie = MBNA_SELECT_NONE;
              // pick_file = crossing->file_id_2;
              // pick_section = crossing->section_2;
              // pick_snav = section->num_snav / 2;
            }
          }
        }

        /* deal with successful pick */
        if (rangemin < 10000000) {
          mbna_crossing_select = pick_crossing;
          mbna_tie_select = MBNA_SELECT_NONE;
          mbna_modelplot_pickfile = MBNA_SELECT_NONE;
          mbna_modelplot_picksection = MBNA_SELECT_NONE;
          mbna_modelplot_picksnav = MBNA_SELECT_NONE;

          /* bring up naverr window if required */
          if (!mbna_naverr_load) {
            do_naverr_init();
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_update_naverr();
            do_update_status();
          }

                /* set flag to replot modelplot */
                project.modelplot_uptodate = false;
        }
      }
    }

    /* middle pick for tie offsets plot is choosing which survey vs survey group (block)
        to plot by itself */
    else {
      rangemin = 10000000;

      /* search by looping over crossings */
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* loop over all ties for this crossing */
        for (int j = 0; j < crossing->num_ties; j++) {
          tie = &(crossing->ties[j]);

          /* handle first snav point */
          file = &project.files[crossing->file_id_1];
          section = &file->sections[crossing->section_1];

          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

          iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            // pick_file = crossing->file_id_1;
            // pick_section = crossing->section_1;
            // pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * tie->offset_y_m);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            // pick_file = crossing->file_id_1;
            // pick_section = crossing->section_1;
            // pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * tie->offset_z_m);
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            // pick_file = crossing->file_id_1;
            // pick_section = crossing->section_1;
            // pick_snav = tie->snav_1;
          }
        }
      }

      /* deal with successful pick */
      if (rangemin < 10000000) {
        crossing = &(project.crossings[pick_crossing]);
        mbna_crossing_select = pick_crossing;
        mbna_tie_select = pick_tie;
        mbna_modelplot_pickfile = MBNA_SELECT_NONE;
        mbna_modelplot_picksection = MBNA_SELECT_NONE;
        mbna_modelplot_picksnav = MBNA_SELECT_NONE;
        mbna_block_select1 = project.files[crossing->file_id_1].block;
        mbna_block_select2 = project.files[crossing->file_id_2].block;
        mbna_block_select = (mbna_block_select2 * (mbna_block_select2 + 1) / 2) + mbna_block_select1;
        mbna_modelplot_tiezoom = false;

        /* bring up naverr window if required */
        if (!mbna_naverr_load) {
          do_naverr_init();
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_update_naverr();
          do_update_status();
        }

                /* set flag to replot modelplot */
                project.modelplot_uptodate = false;
      }
    }

    /* update visualization */
    if (project.visualization_status)
      do_update_visualization_status();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_clearblock() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  int block1, block2;

  /* only proceed if model plot is active and a crossing is selected */
  if (project.open && project.modelplot && mbna_current_crossing != MBNA_SELECT_NONE) {
    /* delete all ties associated with the same pair of surveys as the currently selected crossing */
    crossing = &(project.crossings[mbna_current_crossing]);
    block1 = project.files[crossing->file_id_1].block;
    block2 = project.files[crossing->file_id_2].block;
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);
      if (crossing->num_ties > 0 &&
          ((project.files[crossing->file_id_1].block == block1 && project.files[crossing->file_id_2].block == block2) ||
           (project.files[crossing->file_id_1].block == block2 && project.files[crossing->file_id_2].block == block1))) {
        for (int j = crossing->num_ties - 1; j >= 0; j--) {
          mbnavadjust_deletetie(i, j, MBNA_CROSSING_STATUS_NONE);

                    /* set flag to replot modelplot */
                    project.modelplot_uptodate = false;
        }
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, &error);
    project.save_count = 0;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_plot(const char *sourcefile, int sourceline) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       sourcefile: %s\n", sourcefile);
    fprintf(stderr, "dbg2       sourceline: %d\n", sourceline);
  }

  int status = MB_SUCCESS;

  /* plot model if an inversion has been performed */
//fprintf(stderr,"Called modelplot_plot() from source file:<%s> line:<%d> style:%d uptodate:%d\n",
//sourcefile, sourceline, project.modelplot_style, project.modelplot_uptodate);
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE
        && project.modelplot && !project.modelplot_uptodate) {
    if (project.modelplot_style == MBNA_MODELPLOT_TIMESERIES) {
      mbnavadjust_modelplot_plot_timeseries();
    }
    else if (project.modelplot_style == MBNA_MODELPLOT_PERTURBATION) {
      mbnavadjust_modelplot_plot_perturbation();
    }
    else {
      mbnavadjust_modelplot_plot_tieoffsets();
    }

    project.modelplot_uptodate = true;
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_plot_timeseries() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  double lon_offset_min;
  double lon_offset_max;
  double lat_offset_min;
  double lat_offset_max;
  double z_offset_min;
  double z_offset_max;
  double xymax, yzmax;
  int plot_width;
  int plot_height;
  bool first;
  int iping;
  char label[STRING_MAX];
  int stringwidth, stringascent, stringdescent;
  int pixel;
  int ixo, iyo, ix, iy;
  int imodelplot_start, imodelplot_end;

  /* plot model if an inversion has been performed */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    /* first loop over files setting all plot flags off for both files and sections */
    first = true;
    mbna_modelplot_count = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      file->show_in_modelplot = false;
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        section->show_in_modelplot = false;
      }
    }

    /* now loop over files setting file or section plot flags on as necessary */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];

      /* check if this file will be plotted */
      if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
        if (file->block == mbna_survey_select) {
          file->show_in_modelplot = true;
        }
      }

      /* check if this file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK) {
        if (file->block == mbna_block_select1 || file->block == mbna_block_select2) {
          file->show_in_modelplot = true;
        }
      }

      /* check if this file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
        if (i == mbna_file_select) {
          file->show_in_modelplot = true;
        }
      }

      /* check if each section in this file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
        for (int j = 0; j < file->num_sections; j++) {
          section = &file->sections[j];
          if (i == mbna_file_select && j == mbna_section_select) {
            section->show_in_modelplot = true;
          }
        }
      }

      /* else every file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
        file->show_in_modelplot = true;
      }
    }

    /* if view mode is with survey loop over all crossings */
    if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either file is part of the selected survey
            then set plot flags on for both files */
        if (project.files[crossing->file_id_1].block == mbna_survey_select ||
            project.files[crossing->file_id_2].block == mbna_survey_select) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* else if view mode is with file loop over all crossings */
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either file is selected
            then set plot flags on for both files */
        if (crossing->file_id_1 == mbna_file_select || crossing->file_id_2 == mbna_file_select) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* else if view mode is with section loop over all crossings */
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either section is selected
            then set plot flags on for both files */
        if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select) ||
            (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select)) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* finally loop over files again setting all section plot flags on for files with plot flags on */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      if (file->show_in_modelplot) {
        for (int j = 0; j < file->num_sections; j++) {
          section = &file->sections[j];
          section->show_in_modelplot = true;
        }
      }
    }

    /* get min maxes by looping over files and sections checking for sections to be plotted */
    first = true;
    mbna_modelplot_count = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        /* if this section will be plotted use the snav values */
        if (section->show_in_modelplot) {
          section->modelplot_start_count = mbna_modelplot_count;
          for (int isnav = 0; isnav < section->num_snav; isnav++) {

            if (!mbna_modelplot_zoom || (mbna_modelplot_count >= mbna_modelplot_startzoom &&
                                                 mbna_modelplot_count <= mbna_modelplot_endzoom)) {
              if (first) {
                lon_offset_min = section->snav_lon_offset[isnav] / project.mtodeglon;
                lon_offset_max = section->snav_lon_offset[isnav] / project.mtodeglon;
                lat_offset_min = section->snav_lat_offset[isnav] / project.mtodeglat;
                lat_offset_max = section->snav_lat_offset[isnav] / project.mtodeglat;
                z_offset_min = section->snav_z_offset[isnav];
                z_offset_max = section->snav_z_offset[isnav];
                first = false;
              }
              else {
                lon_offset_min = MIN(lon_offset_min, section->snav_lon_offset[isnav] / project.mtodeglon);
                lon_offset_max = MAX(lon_offset_max, section->snav_lon_offset[isnav] / project.mtodeglon);
                lat_offset_min = MIN(lat_offset_min, section->snav_lat_offset[isnav] / project.mtodeglat);
                lat_offset_max = MAX(lat_offset_max, section->snav_lat_offset[isnav] / project.mtodeglat);
                z_offset_min = MIN(z_offset_min, section->snav_z_offset[isnav]);
                z_offset_max = MAX(z_offset_max, section->snav_z_offset[isnav]);
              }
            }
          }
          mbna_modelplot_count += section->snav_id[section->num_snav - 1];
        }
      }
    }

    /* set plot bounds */
    if (mbna_modelplot_zoom) {
      mbna_modelplot_start = mbna_modelplot_startzoom;
      mbna_modelplot_end = mbna_modelplot_endzoom;
    }
    else {
      mbna_modelplot_start = 0;
      mbna_modelplot_end = mbna_modelplot_count - 1;
    }

    /* get scaling */
    plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
    plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
    mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
    mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
    mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
    mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
    xymax = MAX(fabs(lon_offset_min), fabs(lon_offset_max));
    xymax = MAX(fabs(lat_offset_min), xymax);
    xymax = MAX(fabs(lat_offset_max), xymax);
    mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);
    mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
    yzmax = MAX(fabs(z_offset_min), fabs(z_offset_max));
    yzmax = MAX(yzmax, 0.5);
    mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

    /* clear screens for first plot */
    xg_fillrectangle(pmodp_xgid, 0, 0, modp_borders[1], modp_borders[3], pixel_values[mbna_color_background], XG_SOLIDLINE);

    /* plot the bounds */
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z,
                pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* plot title */
    if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY) {
      sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
      sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
              mbna_file_select, mbna_section_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
      sprintf(label, "Display All Data");
    }

    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* plot labels */
    sprintf(label, "East-West Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "North-South Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "Vertical Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* set clipping */
    xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

    /* loop over all crossings and plot and plot those without ties in green */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);
      if (crossing->num_ties == 0) {
        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];
        iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
        }

        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];
        iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
        }
      }
    }

    /* Now plot the east-west offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale * section->snav_lon_offset[isnav] / project.mtodeglon);
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix,
                          mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* Now plot the north-south offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale * section->snav_lat_offset[isnav] / project.mtodeglat);
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix,
                          mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* Now plot the vertical offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[isnav]);
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix,
                          mbna_modelplot_yo_z + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* loop over all crossings and plot ties */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &(crossing->ties[j]);

        if (tie->inversion_status == MBNA_INVERSION_CURRENT)
          pixel = pixel_values[mbna_color_foreground];
        else
          pixel = pixel_values[BLUE];

        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];
        iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
        }

        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];
        iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
        }
      }
    }

    /* Loop over all files plotting global ties */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot && section->global_tie_status != MBNA_TIE_NONE) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[section->global_tie_snav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            if (section->global_tie_status != MBNA_TIE_Z) {
              /* east-west offsets */
              iy = mbna_modelplot_yo_lon -
                   (int)(mbna_modelplot_yscale * section->offset_x /
                         project.mtodeglon);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

              /* north-south offsets */
              iy = mbna_modelplot_yo_lat -
                   (int)(mbna_modelplot_yscale * section->offset_y /
                         project.mtodeglat);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }

            if (section->global_tie_status != MBNA_TIE_XY) {
              /* vertical offsets */
              iy = mbna_modelplot_yo_z -
                   (int)(mbna_modelplot_yzscale * section->offset_z_m);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
          }
        }
      }
    }

    /* plot current tie in red */
    if (mbna_current_crossing != MBNA_SELECT_NONE && mbna_current_tie != MBNA_SELECT_NONE) {
      crossing = &(project.crossings[mbna_current_crossing]);
      tie = &(crossing->ties[mbna_current_tie]);

      file = &project.files[crossing->file_id_1];
      section = &file->sections[crossing->section_1];
      iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }

      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }
    }

    /* or if tie not selected then plot current crossing in red */
    else if (mbna_current_crossing != MBNA_SELECT_NONE) {
      crossing = &(project.crossings[mbna_current_crossing]);

      file = &project.files[crossing->file_id_1];
      section = &file->sections[crossing->section_1];
      iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }

      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale * section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale * section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[section->num_snav / 2]);
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }
    }

    /* if a modelplot pick did not resolve a single tie, plot the options for a second pick */
    if (mbna_modelplot_pickfile != MBNA_SELECT_NONE) {
      for (int i = 0; i < project.num_crossings; i++) {
        /* check if this crossing includes the picked snav */
        crossing = &(project.crossings[i]);

        /* check first snav */
        if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection) {
          /* loop over the ties */
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &(crossing->ties[j]);
            if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection &&
                tie->snav_1 == mbna_modelplot_picksnav) {
              file = &project.files[crossing->file_id_1];
              section = &file->sections[crossing->section_1];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }

              file = &project.files[crossing->file_id_2];
              section = &file->sections[crossing->section_2];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
            }
          }
        }

        /* check second snav */
        if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection) {
          /* loop over the ties */
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &(crossing->ties[j]);
            if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection &&
                tie->snav_2 == mbna_modelplot_picksnav) {
              file = &project.files[crossing->file_id_2];
              section = &file->sections[crossing->section_2];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_2] / project.mtodeglon);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_2] / project.mtodeglat);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_2]);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }

              file = &project.files[crossing->file_id_1];
              section = &file->sections[crossing->section_1];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale * section->snav_lon_offset[tie->snav_1] / project.mtodeglon);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale * section->snav_lat_offset[tie->snav_1] / project.mtodeglat);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * section->snav_z_offset[tie->snav_1]);
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
            }
          }
        }
      }
    }

    /* plot zoom if active */
    if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0) {
      imodelplot_start = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                         mbna_modelplot_start;
      imodelplot_start = MIN(MAX(imodelplot_start, 0), project.num_pings - 1);
      imodelplot_end = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                       mbna_modelplot_start;
      imodelplot_end = MIN(MAX(imodelplot_end, 0), project.num_pings - 1);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_start - mbna_modelplot_start));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_end - mbna_modelplot_start));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
    }

    /* reset clipping */
    xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_plot_perturbation() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  double lon_offset_min;
  double lon_offset_max;
  double lat_offset_min;
  double lat_offset_max;
  double z_offset_min;
  double z_offset_max;
  double xymax, yzmax;
  int plot_width;
  int plot_height;
  bool first;
  int iping;
  char label[STRING_MAX];
  int stringwidth, stringascent, stringdescent;
  int pixel;
  int ixo, iyo, ix, iy;
  int imodelplot_start, imodelplot_end;

  /* plot model if an inversion has been performed */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    /* first loop over files setting all plot flags off for both files and sections */
    first = true;
    mbna_modelplot_count = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      file->show_in_modelplot = false;
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        section->show_in_modelplot = false;
      }
    }

    /* now loop over files setting file or section plot flags on as necessary */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];

      /* check if this file will be plotted */
      if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY || mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
        if (file->block == mbna_survey_select) {
          file->show_in_modelplot = true;
        }
      }

      /* check if this file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_FILE || mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
        if (i == mbna_file_select) {
          file->show_in_modelplot = true;
        }
      }

      /* check if each section in this file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
        for (int j = 0; j < file->num_sections; j++) {
          section = &file->sections[j];
          if (i == mbna_file_select && j == mbna_section_select) {
            section->show_in_modelplot = true;
          }
        }
      }

      /* else every file will be plotted */
      else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
        file->show_in_modelplot = true;
      }
    }

    /* if view mode is with survey loop over all crossings */
    if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either file is part of the selected survey
            then set plot flags on for both files */
        if (project.files[crossing->file_id_1].block == mbna_survey_select ||
            project.files[crossing->file_id_2].block == mbna_survey_select) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* else if view mode is with file loop over all crossings */
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either file is selected
            then set plot flags on for both files */
        if (crossing->file_id_1 == mbna_file_select || crossing->file_id_2 == mbna_file_select) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* else if view mode is with section loop over all crossings */
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      for (int i = 0; i < project.num_crossings; i++) {
        crossing = &(project.crossings[i]);

        /* if either section is selected
            then set plot flags on for both files */
        if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select) ||
            (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select)) {
          project.files[crossing->file_id_1].show_in_modelplot = true;
          project.files[crossing->file_id_2].show_in_modelplot = true;
        }
      }
    }

    /* finally loop over files again setting all section plot flags on for files with plot flags on */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      if (file->show_in_modelplot) {
        for (int j = 0; j < file->num_sections; j++) {
          section = &file->sections[j];
          section->show_in_modelplot = true;
        }
      }
    }

    /* get min maxes by looping over files and sections checking for sections to be plotted */
    first = true;
    mbna_modelplot_count = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];

        /* if this section will be plotted use the snav values */
        if (section->show_in_modelplot) {
          section->modelplot_start_count = mbna_modelplot_count;
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            if (!mbna_modelplot_zoom || (mbna_modelplot_count >= mbna_modelplot_startzoom &&
                                                 mbna_modelplot_count <= mbna_modelplot_endzoom)) {
              if (first) {
                lon_offset_min = section->snav_lon_offset[isnav] / project.mtodeglon - file->block_offset_x;
                lon_offset_max = section->snav_lon_offset[isnav] / project.mtodeglon - file->block_offset_x;
                lat_offset_min = section->snav_lat_offset[isnav] / project.mtodeglat - file->block_offset_y;
                lat_offset_max = section->snav_lat_offset[isnav] / project.mtodeglat - file->block_offset_y;
                z_offset_min = section->snav_z_offset[isnav] - file->block_offset_z;
                z_offset_max = section->snav_z_offset[isnav] - file->block_offset_z;
                first = false;
              }
              else {
                lon_offset_min = MIN(lon_offset_min,
                                     section->snav_lon_offset[isnav] / project.mtodeglon - file->block_offset_x);
                lon_offset_max = MAX(lon_offset_max,
                                     section->snav_lon_offset[isnav] / project.mtodeglon - file->block_offset_x);
                lat_offset_min = MIN(lat_offset_min,
                                     section->snav_lat_offset[isnav] / project.mtodeglat - file->block_offset_y);
                lat_offset_max = MAX(lat_offset_max,
                                     section->snav_lat_offset[isnav] / project.mtodeglat - file->block_offset_y);
                z_offset_min = MIN(z_offset_min, section->snav_z_offset[isnav] - file->block_offset_z);
                z_offset_max = MAX(z_offset_max, section->snav_z_offset[isnav] - file->block_offset_z);
              }
            }
          }
          mbna_modelplot_count += section->snav_id[section->num_snav - 1];
        }
      }
    }

    /* set plot bounds */
    if (mbna_modelplot_zoom) {
      mbna_modelplot_start = mbna_modelplot_startzoom;
      mbna_modelplot_end = mbna_modelplot_endzoom;
    }
    else {
      mbna_modelplot_start = 0;
      mbna_modelplot_end = mbna_modelplot_count - 1;
    }

    /* get scaling */
    plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
    plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
    mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
    mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
    mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
    mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
    xymax = MAX(fabs(lon_offset_min), fabs(lon_offset_max));
    xymax = MAX(fabs(lat_offset_min), xymax);
    xymax = MAX(fabs(lat_offset_max), xymax);
    mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_end - mbna_modelplot_start + 1);
    mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
    yzmax = MAX(fabs(z_offset_min), fabs(z_offset_max));
    yzmax = MAX(yzmax, 0.5);
    mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

    /* clear screens for first plot */
    xg_fillrectangle(pmodp_xgid, 0, 0, modp_borders[1], modp_borders[3], pixel_values[mbna_color_background], XG_SOLIDLINE);

    /* plot the bounds */
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z,
                pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* plot title */
    if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY) {
      sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
      sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
              mbna_file_select, mbna_section_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
      sprintf(label, "Display All Data");
    }

    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* plot labels */
    sprintf(label, "East-West Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "North-South Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "Vertical Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* set clipping */
    xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

    /* loop over all crossings and plot and plot those without ties in green */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);
      if (crossing->num_ties == 0) {
        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];
        iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
        }

        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];
        iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
          xg_drawrectangle(pmodp_xgid, ix - 3, iy - 1, 3, 3, pixel_values[GREEN], XG_SOLIDLINE);
        }
      }
    }

    /* Now plot the east-west offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_lon -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lon_offset[isnav] / project.mtodeglon - file->block_offset_x));
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix,
                          mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* Now plot the north-south offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_lat -
                 (int)(mbna_modelplot_yscale *
                       (section->snav_lat_offset[isnav] / project.mtodeglat - file->block_offset_y));
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix,
                          mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* Now plot the vertical offsets */
    ixo = 0;
    iyo = 0;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[isnav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
            iy = mbna_modelplot_yo_z -
                 (int)(mbna_modelplot_yzscale * (section->snav_z_offset[isnav] - file->block_offset_z));
            if ((i > 0 || j > 0) && !section->continuity && isnav == 0)
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix,
                          mbna_modelplot_yo_z + plot_height / 2, pixel_values[GREEN], XG_SOLIDLINE);
            else if (i > 0 || j > 0) {
              /* if (j == 0 && isnav == 0 && section->continuity)
                  xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z +
                 plot_height / 2, pixel_values[CORAL], XG_DASHLINE); */
              xg_drawline(pmodp_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
            ixo = ix;
            iyo = iy;
          }
        }
      }
    }

    /* loop over all crossings and plot ties */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &(project.crossings[i]);
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &(crossing->ties[j]);

        if (tie->inversion_status == MBNA_INVERSION_CURRENT)
          pixel = pixel_values[mbna_color_foreground];
        else
          pixel = pixel_values[BLUE];

        file = &project.files[crossing->file_id_1];
        section = &file->sections[crossing->section_1];
        iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
        }

        file = &project.files[crossing->file_id_2];
        section = &file->sections[crossing->section_2];
        iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

        if (section->show_in_modelplot &&
            (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
          iy = mbna_modelplot_yo_lon -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_lat -
               (int)(mbna_modelplot_yscale *
                     (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
          iy = mbna_modelplot_yo_z -
               (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
          xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
        }
      }
    }

    /* Loop over all files plotting global ties */
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if (section->show_in_modelplot && section->global_tie_status != MBNA_TIE_NONE) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[section->global_tie_snav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            if (section->global_tie_status != MBNA_TIE_Z) {
              /* east-west offsets */
              iy = mbna_modelplot_yo_lon -
                   (int)(mbna_modelplot_yscale *
                         (section->snav_lon_offset[section->global_tie_snav] / project.mtodeglon -
                          file->block_offset_x));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

              /* north-south offsets */
              iy = mbna_modelplot_yo_lat -
                   (int)(mbna_modelplot_yscale *
                         (section->snav_lat_offset[section->global_tie_snav] / project.mtodeglat -
                          file->block_offset_y));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }

            if (section->global_tie_status != MBNA_TIE_XY) {
              /* vertical offsets */
              iy = mbna_modelplot_yo_z -
                   (int)(mbna_modelplot_yzscale *
                         (section->snav_z_offset[section->global_tie_snav] - file->block_offset_z));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }
          }
        }
      }
    }

    /* plot current tie in red */
    if (mbna_current_crossing != MBNA_SELECT_NONE && mbna_current_tie != MBNA_SELECT_NONE) {
      crossing = &(project.crossings[mbna_current_crossing]);
      tie = &(crossing->ties[mbna_current_tie]);

      file = &project.files[crossing->file_id_1];
      section = &file->sections[crossing->section_1];
      iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z -
             (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }

      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z -
             (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }
    }

    /* or if tie not selected then plot current crossing in red */
    else if (mbna_current_crossing != MBNA_SELECT_NONE) {
      crossing = &(project.crossings[mbna_current_crossing]);

      file = &project.files[crossing->file_id_1];
      section = &file->sections[crossing->section_1];
      iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z -
             (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }

      file = &project.files[crossing->file_id_2];
      section = &file->sections[crossing->section_2];
      iping = section->modelplot_start_count + section->snav_id[section->num_snav / 2];

      if (section->show_in_modelplot &&
          (!mbna_modelplot_zoom || (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
        iy = mbna_modelplot_yo_lon -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lon_offset[section->num_snav / 2] / project.mtodeglon - file->block_offset_x));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_lat -
             (int)(mbna_modelplot_yscale *
                   (section->snav_lat_offset[section->num_snav / 2] / project.mtodeglat - file->block_offset_y));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
        iy = mbna_modelplot_yo_z -
             (int)(mbna_modelplot_yzscale * (section->snav_z_offset[section->num_snav / 2] - file->block_offset_z));
        xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
        xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
      }
    }

    /* if a modelplot pick did not resolve a single tie, plot the options for a second pick */
    if (mbna_modelplot_pickfile != MBNA_SELECT_NONE) {
      for (int i = 0; i < project.num_crossings; i++) {
        /* check if this crossing includes the picked snav */
        crossing = &(project.crossings[i]);

        /* check first snav */
        if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection) {
          /* loop over the ties */
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &(crossing->ties[j]);
            if (crossing->file_id_1 == mbna_modelplot_pickfile && crossing->section_1 == mbna_modelplot_picksection &&
                tie->snav_1 == mbna_modelplot_picksnav) {
              file = &project.files[crossing->file_id_1];
              section = &file->sections[crossing->section_1];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }

              file = &project.files[crossing->file_id_2];
              section = &file->sections[crossing->section_2];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
            }
          }
        }

        /* check second snav */
        if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection) {
          /* loop over the ties */
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &(crossing->ties[j]);
            if (crossing->file_id_2 == mbna_modelplot_pickfile && crossing->section_2 == mbna_modelplot_picksection &&
                tie->snav_2 == mbna_modelplot_picksnav) {
              file = &project.files[crossing->file_id_2];
              section = &file->sections[crossing->section_2];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_2];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - file->block_offset_x));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - file->block_offset_y));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_2] - file->block_offset_z));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }

              file = &project.files[crossing->file_id_1];
              section = &file->sections[crossing->section_1];
              iping = section->modelplot_start_count + section->snav_id[tie->snav_1];

              if (section->show_in_modelplot &&
                  (!mbna_modelplot_zoom ||
                   (iping >= mbna_modelplot_startzoom && iping <= mbna_modelplot_endzoom))) {
                ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));
                iy = mbna_modelplot_yo_lon -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - file->block_offset_x));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_lat -
                     (int)(mbna_modelplot_yscale *
                           (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - file->block_offset_y));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
                iy = mbna_modelplot_yo_z -
                     (int)(mbna_modelplot_yzscale * (section->snav_z_offset[tie->snav_1] - file->block_offset_z));
                xg_fillrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[6], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 5, iy - 5, 11, 11, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
            }
          }
        }
      }
    }

    /* plot zoom if active */
    if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0) {
      imodelplot_start = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                         mbna_modelplot_start;
      imodelplot_start = MIN(MAX(imodelplot_start, 0), project.num_pings - 1);
      imodelplot_end = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                       mbna_modelplot_start;
      imodelplot_end = MIN(MAX(imodelplot_end, 0), project.num_pings - 1);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_start - mbna_modelplot_start));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (imodelplot_end - mbna_modelplot_start));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
    }

    /* reset clipping */
    xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_modelplot_plot_tieoffsets() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  int num_surveys;
  double lon_offset_min;
  double lon_offset_max;
  double lat_offset_min;
  double lat_offset_max;
  double z_offset_min;
  double z_offset_max;
  double xymax, yzmax;
  int plot_width;
  int plot_height;
  bool first;
  char label[STRING_MAX];
  int stringwidth, stringascent, stringdescent;
  int pixel;
  int itiestart, itieend;
  int ix, iy;
  int num_ties_block;
  int num_blocks;
  int plot_index;

  /* plot model if an inversion has been performed */
  if (project.open && project.inversion_status != MBNA_INVERSION_NONE && project.modelplot) {
    /* get number of ties and surveys */
    // const int num_ties = project.num_ties;
    mbna_num_ties_plot = 0;
    num_surveys = 0;

    /* count surveys and plot blocks by looping over files */
    num_surveys = 1;
    for (int i = 0; i < project.num_files; i++) {
      file = &project.files[i];
      file->show_in_modelplot = -1;
      for (int j = 0; j < file->num_sections; j++) {
        section = &file->sections[j];
        if ((i > 0 || j > 0) && !section->continuity)
          num_surveys++;
      }
    }
    num_blocks = 0;
    for (int i = 0; i < num_surveys; i++)
      num_blocks += i + 1;

    /* Figure out which ties might be plotted */
    for (int i = 0; i < project.num_crossings; i++) {
      crossing = &project.crossings[i];
      for (int j = 0; j < crossing->num_ties; j++) {
        tie = &crossing->ties[j];
        tie->block_1 = project.files[crossing->file_id_1].block;
        tie->block_2 = project.files[crossing->file_id_2].block;
        tie->isurveyplotindex = -1;

        /* check to see if this tie should be plotted */
        if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK) {
          if (tie->block_1 == mbna_block_select1 && tie->block_2 == mbna_block_select2) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY) {
          if (tie->block_1 == mbna_survey_select && tie->block_2 == mbna_survey_select) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
          if (tie->block_1 == mbna_survey_select || tie->block_2 == mbna_survey_select) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
          if (crossing->file_id_1 == mbna_file_select && crossing->file_id_2 == mbna_file_select) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
          if (crossing->file_id_1 == mbna_file_select || crossing->file_id_2 == mbna_file_select) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
          if ((crossing->file_id_1 == mbna_file_select && crossing->section_1 == mbna_section_select) ||
              (crossing->file_id_2 == mbna_file_select && crossing->section_2 == mbna_section_select)) {
            tie->isurveyplotindex = 1;
            mbna_num_ties_plot++;
          }
        }
        else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
          tie->isurveyplotindex = 1;
          mbna_num_ties_plot++;
        }
      }
    }

    /* Get min maxes by looping over surveys to locate each tie in plot */
    plot_index = 0;
    first = true;
    if (mbna_modelplot_tiezoom) {
      mbna_modelplot_tiestart = mbna_modelplot_tiestartzoom;
      mbna_modelplot_tieend = mbna_modelplot_tieendzoom;
    }
    else {
      mbna_modelplot_tiestart = 0;
      mbna_modelplot_tieend = mbna_num_ties_plot - 1;
    }
    for (int isurvey2 = 0; isurvey2 < num_surveys; isurvey2++) {
      for (int isurvey1 = 0; isurvey1 <= isurvey2; isurvey1++) {
        /* loop over all ties looking for ones in current plot block
            - current plot block is for ties between
            surveys isurvey1 and isurvey2 */
        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &project.crossings[i];
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &crossing->ties[j];

            /* check if this tie is between the desired surveys */
            if (tie->isurveyplotindex >= 0 && ((tie->block_1 == isurvey1 && tie->block_2 == isurvey2) ||
                                               (tie->block_2 == isurvey1 && tie->block_1 == isurvey2))) {
              /* set plot index for tie */
              tie->isurveyplotindex = plot_index;

              /* increment plot_index */
              plot_index++;

              /* if tie will be plotted (check for zoom) use it for min max */
              if (tie->isurveyplotindex >= 0 && (tie->isurveyplotindex >= mbna_modelplot_tiestart &&
                                                 tie->isurveyplotindex <= mbna_modelplot_tieend)) {
                if (first) {
                  lon_offset_min = tie->offset_x_m;
                  lon_offset_max = tie->offset_x_m;
                  lat_offset_min = tie->offset_y_m;
                  lat_offset_max = tie->offset_y_m;
                  z_offset_min = tie->offset_z_m;
                  z_offset_max = tie->offset_z_m;
                  first = false;
                }
                else {
                  lon_offset_min = MIN(lon_offset_min, tie->offset_x_m);
                  lon_offset_max = MAX(lon_offset_max, tie->offset_x_m);
                  lat_offset_min = MIN(lat_offset_min, tie->offset_y_m);
                  lat_offset_max = MAX(lat_offset_max, tie->offset_y_m);
                  z_offset_min = MIN(z_offset_min, tie->offset_z_m);
                  z_offset_max = MAX(z_offset_max, tie->offset_z_m);
                }
              }
            }
          }
        }
      }
    }

    /* get scaling */
    plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
    plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
    mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
    mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
    mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
    mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
    xymax = MAX(fabs(lon_offset_min), fabs(lon_offset_max));
    xymax = MAX(fabs(lat_offset_min), xymax);
    xymax = MAX(fabs(lat_offset_max), xymax);
    mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_tieend - mbna_modelplot_tiestart + 1);
    mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
    yzmax = MAX(fabs(z_offset_min), fabs(z_offset_max));
    yzmax = MAX(yzmax, 0.5);
    mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);

    /* clear screens for first plot */
    xg_fillrectangle(pmodp_xgid, 0, 0, modp_borders[1], modp_borders[3], pixel_values[mbna_color_background], XG_SOLIDLINE);

    /* plot the bounds */
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lon, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lon,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_lat, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_lat,
                pixel_values[mbna_color_foreground], XG_DASHLINE);
    xg_drawrectangle(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z - plot_height / 2, plot_width, plot_height,
                     pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    xg_drawline(pmodp_xgid, mbna_modelplot_xo, mbna_modelplot_yo_z, mbna_modelplot_xo + plot_width, mbna_modelplot_yo_z,
                pixel_values[mbna_color_foreground], XG_DASHLINE);

    /* plot title */
    if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY) {
      sprintf(label, "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
      sprintf(label, "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      sprintf(label, "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      sprintf(label, "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      sprintf(label, "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
              mbna_file_select, mbna_section_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
      sprintf(label, "Display All Data");
    }

    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* plot labels */
    sprintf(label, "Tie East-West Offset (meters) Grouped by Surveys");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tiestart);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tieend);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "Tie North-South Offset (meters) Grouped by Surveys");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tiestart);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tieend);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "Tie Vertical Offset (meters) Grouped by Surveys");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tiestart);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%d", mbna_modelplot_tieend);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    sprintf(label, "%.2f", -1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* set clipping */
    xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

    /* loop over surveys to locate each tie in plot */
    plot_index = 0;
    for (int isurvey2 = 0; isurvey2 < num_surveys; isurvey2++) {
      for (int isurvey1 = 0; isurvey1 <= isurvey2; isurvey1++) {
        num_ties_block = 0;

        /* loop over all ties looking for ones in current plot block
            - current plot block is for ties between
            surveys isurvey1 and isurvey2 */
        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &project.crossings[i];
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &crossing->ties[j];

            /* check if this tie is between the desired surveys */
            if (tie->isurveyplotindex >= 0 && ((tie->block_1 == isurvey1 && tie->block_2 == isurvey2) ||
                                               (tie->block_2 == isurvey1 && tie->block_1 == isurvey2))) {
              if (tie->isurveyplotindex >= mbna_modelplot_tiestart &&
                  tie->isurveyplotindex <= mbna_modelplot_tieend) {
                /* plot tie */
                if (tie->inversion_status == MBNA_INVERSION_CURRENT)
                  pixel = pixel_values[mbna_color_foreground];
                else
                  pixel = pixel_values[BLUE];

                ix = mbna_modelplot_xo +
                     (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart));

                iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * tie->offset_x_m);
                if (i == mbna_current_crossing && j == mbna_current_tie) {
                  xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                  xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                   XG_SOLIDLINE);
                }
                else
                  xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * tie->offset_y_m);
                if (i == mbna_current_crossing && j == mbna_current_tie) {
                  xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                  xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                   XG_SOLIDLINE);
                }
                else
                  xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * tie->offset_z_m);
                if (i == mbna_current_crossing && j == mbna_current_tie) {
                  xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                  xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                   XG_SOLIDLINE);
                }
                else
                  xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
              }

              /* increment plot_index */
              plot_index++;

              /* increment num_ties_block */
              num_ties_block++;
            }
          }
        }

        /* plot line for boundary between plot blocks */
        if (num_ties_block > 0) {
          /* plot line for boundary between plot blocks */
          ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (plot_index - mbna_modelplot_tiestart - 0.5));
          xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix,
                      mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_DASHLINE);
          xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix,
                      mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_DASHLINE);
          xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                      pixel_values[GREEN], XG_DASHLINE);
        }
      }
    }

    /* plot zoom if active */
    if (mbna_modelplot_zoom_x1 != 0 || mbna_modelplot_zoom_x2 != 0) {
      itiestart = (MIN(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                  mbna_modelplot_tiestart;
      itiestart = MIN(MAX(itiestart, 0), mbna_num_ties_plot - 1);
      itieend = (MAX(mbna_modelplot_zoom_x1, mbna_modelplot_zoom_x2) - mbna_modelplot_xo) / mbna_modelplot_xscale +
                mbna_modelplot_tiestart;
      itieend = MIN(MAX(itieend, 0), mbna_num_ties_plot - 1);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itiestart - mbna_modelplot_tiestart));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);

      ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itieend - mbna_modelplot_tiestart));
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
      xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                  pixel_values[mbna_color_foreground], XG_DASHLINE);
    }

    /* reset clipping */
    xg_setclip(pmodp_xgid, 0, 0, mbna_modelplot_width, mbna_modelplot_height);
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_open_visualization(int which_grid) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       which_grid:  %d\n", which_grid);
  }

  int status = MB_SUCCESS;
  size_t instance;
  int projectionid, utmzone;
  double reference_lon;

  /* mbview parameters */
  char mbv_file_name[STRING_MAX];
  char mbv_title[STRING_MAX];
  int mbv_xo;
  int mbv_yo;
  int mbv_width;
  int mbv_height;
  int mbv_lorez_dimension;
  int mbv_hirez_dimension;
  int mbv_lorez_navdecimate;
  int mbv_hirez_navdecimate;
  int mbv_display_mode;
  int mbv_mouse_mode;
  int mbv_grid_mode;
  int mbv_primary_histogram;
  int mbv_primaryslope_histogram;
  int mbv_secondary_histogram;
  int mbv_primary_shade_mode;
  int mbv_slope_shade_mode;
  int mbv_secondary_shade_mode;
  int mbv_grid_contour_mode;
  int mbv_site_view_mode;
  int mbv_route_view_mode;
  int mbv_nav_view_mode;
  int mbv_navdrape_view_mode;
  int mbv_vector_view_mode;
  int mbv_primary_colortable;
  int mbv_primary_colortable_mode;
  double mbv_primary_colortable_min;
  double mbv_primary_colortable_max;
  int mbv_slope_colortable;
  int mbv_slope_colortable_mode;
  double mbv_slope_colortable_min;
  double mbv_slope_colortable_max;
  int mbv_secondary_colortable;
  int mbv_secondary_colortable_mode;
  double mbv_exageration;
  double mbv_modelelevation3d;
  double mbv_modelazimuth3d;
  double mbv_viewelevation3d;
  double mbv_viewazimuth3d;
  double mbv_illuminate_magnitude;
  double mbv_illuminate_elevation;
  double mbv_illuminate_azimuth;
  double mbv_slope_magnitude;
  double mbv_overlay_shade_magnitude;
  double mbv_overlay_shade_center;
  int mbv_overlay_shade_mode;
  double mbv_contour_interval;
  int mbv_primary_grid_projection_mode;
  char mbv_primary_grid_projection_id[MB_PATH_MAXLINE];
  int mbv_display_projection_mode;
  char mbv_display_projection_id[MB_PATH_MAXLINE];
  float mbv_primary_nodatavalue = 0.0;
  int mbv_primary_nxy = 0;
  int mbv_primary_nx = 0;
  int mbv_primary_ny = 0;
  double mbv_primary_min = 0.0;
  double mbv_primary_max = 0.0;
  double mbv_primary_xmin = 0.0;
  double mbv_primary_xmax = 0.0;
  double mbv_primary_ymin = 0.0;
  double mbv_primary_ymax = 0.0;
  double mbv_primary_dx = 0.0;
  double mbv_primary_dy = 0.0;
  float *mbv_primary_data = NULL;

  int mbv_navpings = 0;
  int mbv_navpings_alloc = 0;
  double *navtime_d = NULL;
  double *navlon = NULL;
  double *navlat = NULL;
  double *navz = NULL;
  double *navheading = NULL;
  double *navspeed = NULL;
  double *navportlon = NULL;
  double *navportlat = NULL;
  double *navstbdlon = NULL;
  double *navstbdlat = NULL;
  int *navline = NULL;
  int *navshot = NULL;
  int *navcdp = NULL;
  int mbv_navcolor;
  int mbv_navsize;
  mb_path mbv_navname;
  int mbv_navpathstatus;
  mb_path mbv_navpathraw;
  mb_path mbv_navpathprocessed;
  int mbv_navformatorg;
  int mbv_navswathbounds;
  int mbv_navline;
  int mbv_navshot;
  int mbv_navcdp;
  int mbv_navdecimation;

  FILE *nfp;
  struct mbna_file *file;
  struct mbna_section *section;
  int year, month, day, hour, minute;
  double seconds, roll, pitch, heave;
  double sonardepth;
  int nscan;

  /* destroy any pre-existing visualization */
  if (project.visualization_status) {
    mbview_destroy(mbna_verbose, 0, true, &error);
    project.visualization_status = false;
  }

  /* get next instance number */
  status = mbview_init(mbna_verbose, &instance, &error);
  if (instance == MBV_NO_WINDOW) {
    fprintf(stderr, "Unable to create mbview - %d mbview windows already created\n", MBV_MAX_WINDOWS);
  }

  else {
    if (which_grid == MBNA_GRID_RAW)
      sprintf(mbv_file_name, "%s/ProjectTopoRaw.grd", project.datadir);
    else
      sprintf(mbv_file_name, "%s/ProjectTopoAdj.grd", project.datadir);

    /* set parameters */
    sprintf(mbv_title, "MBnavadjust: %s\n", project.name);
    mbv_xo = 200;
    mbv_yo = 200;
    mbv_width = 560;
    mbv_height = 500;
    mbv_lorez_dimension = 100;
    mbv_hirez_dimension = 500;
    mbv_lorez_navdecimate = 5;
    mbv_hirez_navdecimate = 1;

    /* set basic mbview window parameters */
    status = mbview_setwindowparms(mbna_verbose, instance, &do_visualize_dismiss_notify, mbv_title, mbv_xo, mbv_yo, mbv_width,
                                   mbv_height, mbv_lorez_dimension, mbv_hirez_dimension, mbv_lorez_navdecimate,
                                   mbv_hirez_navdecimate, &error);

    /* read in the grd file */
    if (status == MB_SUCCESS && strlen(mbv_file_name) > 0)
      status =
          mb_read_gmt_grd(mbna_verbose, mbv_file_name, &mbv_primary_grid_projection_mode, mbv_primary_grid_projection_id,
                          &mbv_primary_nodatavalue, &mbv_primary_nxy, &mbv_primary_nx, &mbv_primary_ny, &mbv_primary_min,
                          &mbv_primary_max, &mbv_primary_xmin, &mbv_primary_xmax, &mbv_primary_ymin, &mbv_primary_ymax,
                          &mbv_primary_dx, &mbv_primary_dy, &mbv_primary_data, NULL, NULL, &error);

    /* set parameters */
    if (status == MB_SUCCESS) {
      mbv_display_mode = MBV_DISPLAY_2D;
      mbv_mouse_mode = MBV_MOUSE_MOVE;
      mbv_grid_mode = MBV_GRID_VIEW_PRIMARY;
      mbv_primary_histogram = false;
      mbv_primaryslope_histogram = false;
      mbv_secondary_histogram = false;
      mbv_primary_shade_mode = MBV_SHADE_VIEW_SLOPE;
      mbv_slope_shade_mode = MBV_SHADE_VIEW_NONE;
      mbv_secondary_shade_mode = MBV_SHADE_VIEW_NONE;
      mbv_grid_contour_mode = MBV_VIEW_OFF;
      mbv_site_view_mode = MBV_VIEW_OFF;
      mbv_route_view_mode = MBV_VIEW_OFF;
      mbv_nav_view_mode = MBV_VIEW_OFF;
      mbv_navdrape_view_mode = MBV_VIEW_OFF;
      mbv_vector_view_mode = MBV_VIEW_OFF;
      mbv_primary_colortable = MBV_COLORTABLE_HAXBY;
      mbv_primary_colortable_mode = MBV_COLORTABLE_NORMAL;
      mbv_primary_colortable_min = mbv_primary_min;
      mbv_primary_colortable_max = mbv_primary_max;
      mbv_slope_colortable = MBV_COLORTABLE_HAXBY;
      mbv_slope_colortable_mode = MBV_COLORTABLE_REVERSED;
      mbv_slope_colortable_min = 0.0;
      mbv_slope_colortable_max = 0.5;
      mbv_secondary_colortable = MBV_COLORTABLE_HAXBY;
      mbv_secondary_colortable_mode = MBV_COLORTABLE_NORMAL;
      // double mbv_secondary_colortable_min = 0.0;
      // double mbv_secondary_colortable_max = 0.0;
      mbv_exageration = 1.0;
      mbv_modelelevation3d = 90.0;
      mbv_modelazimuth3d = 0.0;
      mbv_viewelevation3d = 90.0;
      mbv_viewazimuth3d = 0.0;
      mbv_illuminate_magnitude = 1.0;
      mbv_illuminate_elevation = 5.0;
      mbv_illuminate_azimuth = 90.0;
      mbv_slope_magnitude = 1.0;
      mbv_overlay_shade_magnitude = 1.0;
      mbv_overlay_shade_center = 0.0;
      mbv_overlay_shade_mode = MBV_COLORTABLE_NORMAL;
      mbv_contour_interval = pow(10.0, floor(log10(mbv_primary_max - mbv_primary_min)) - 1.0);

      /* set mbview default values */
      status = mb_mbview_defaults(mbna_verbose, &mbv_primary_colortable, &mbv_primary_colortable_mode,
                                  &mbv_primary_shade_mode, &mbv_slope_colortable, &mbv_slope_colortable_mode,
                                  &mbv_secondary_colortable, &mbv_secondary_colortable_mode, &mbv_illuminate_magnitude,
                                  &mbv_illuminate_elevation, &mbv_illuminate_azimuth, &mbv_slope_magnitude);
    }

    /* set the display projection */
    if (status == MB_SUCCESS) {
      /* if grid projected then use the same projected coordinate system by default */
      if (mbv_primary_grid_projection_mode == MBV_PROJECTION_PROJECTED) {
        mbv_display_projection_mode = mbv_primary_grid_projection_mode;
        strcpy(mbv_display_projection_id, mbv_primary_grid_projection_id);
      }

      /* else if grid geographic and covers much of the world use spheroid */
      else if (mbv_primary_xmax - mbv_primary_xmin > 15.0 || mbv_primary_ymax - mbv_primary_ymin > 15.0) {
        mbv_display_projection_mode = MBV_PROJECTION_SPHEROID;
        sprintf(mbv_display_projection_id, "SPHEROID");
      }

      /* else if grid geographic then use appropriate UTM zone for non-polar grids */
      else if (mbv_primary_ymax > -80.0 && mbv_primary_ymin < 84.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        reference_lon = 0.5 * (mbv_primary_xmin + mbv_primary_xmax);
        if (reference_lon > 180.0)
          reference_lon -= 360.0;
        utmzone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
        if (0.5 * (mbv_primary_ymin + mbv_primary_ymax) >= 0.0)
          projectionid = 32600 + utmzone;
        else
          projectionid = 32700 + utmzone;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more northerly than 84 deg N then use
              North Universal Polar Stereographic Projection */
      else if (mbv_primary_ymin > 84.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32661;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more southerly than 80 deg S then use
              South Universal Polar Stereographic Projection */
      else if (mbv_primary_ymax < 80.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32761;
        sprintf(mbv_display_projection_id, "EPSG:%d", projectionid);
      }

      /* else just use geographic */
      else {
        mbv_display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
        sprintf(mbv_display_projection_id, "EPSG:%d", GCS_WGS_84);
      }
    }

    /* set basic mbview view controls */
    if (status == MB_SUCCESS)
      status = mbview_setviewcontrols(
          mbna_verbose, instance, mbv_display_mode, mbv_mouse_mode, mbv_grid_mode, mbv_primary_histogram,
          mbv_primaryslope_histogram, mbv_secondary_histogram, mbv_primary_shade_mode, mbv_slope_shade_mode,
          mbv_secondary_shade_mode, mbv_grid_contour_mode, mbv_site_view_mode, mbv_route_view_mode, mbv_nav_view_mode,
          mbv_navdrape_view_mode, mbv_vector_view_mode, mbv_exageration, mbv_modelelevation3d, mbv_modelazimuth3d,
          mbv_viewelevation3d, mbv_viewazimuth3d, mbv_illuminate_magnitude, mbv_illuminate_elevation,
          mbv_illuminate_azimuth, mbv_slope_magnitude, mbv_overlay_shade_magnitude, mbv_overlay_shade_center,
          mbv_overlay_shade_mode, mbv_contour_interval, mbv_display_projection_mode, mbv_display_projection_id, &error);

    /* set primary grid data */
    if (status == MB_SUCCESS)
      status = mbview_setprimarygrid(
          mbna_verbose, instance, mbv_primary_grid_projection_mode, mbv_primary_grid_projection_id, mbv_primary_nodatavalue,
          mbv_primary_nx, mbv_primary_ny, mbv_primary_min, mbv_primary_max, mbv_primary_xmin, mbv_primary_xmax,
          mbv_primary_ymin, mbv_primary_ymax, mbv_primary_dx, mbv_primary_dy, mbv_primary_data, &error);
    mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&mbv_primary_data, &error);

    /* set more mbview control values */
    if (status == MB_SUCCESS)
      status = mbview_setprimarycolortable(mbna_verbose, instance, mbv_primary_colortable, mbv_primary_colortable_mode,
                                           mbv_primary_colortable_min, mbv_primary_colortable_max, &error);
    if (status == MB_SUCCESS)
      status = mbview_setslopecolortable(mbna_verbose, instance, mbv_slope_colortable, mbv_slope_colortable_mode,
                                         mbv_slope_colortable_min, mbv_slope_colortable_max, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableadjustnavs(mbna_verbose, instance, &error);
    if (status == MB_SUCCESS)
      status = mbview_enableviewties(mbna_verbose, instance, &error);
    mbview_addpicknotify(mbna_verbose, 0, MBV_PICK_ROUTE, do_pickroute_notify, &error);
    mbview_addpicknotify(mbna_verbose, 0, MBV_PICK_NAV, do_picknav_notify, &error);

    /* open up mbview window */
    if (status == MB_SUCCESS) {
      status = mbview_open(mbna_verbose, instance, &error);

      /* set sensitivity callback routine */
      if (status == MB_SUCCESS) {
        mbview_setsensitivitynotify(mbna_verbose, instance, do_visualize_sensitivity, &error);
      }
    }

    /* add action button */
    if (status == MB_SUCCESS) {
      mbview_addaction(mbna_verbose, instance, do_mbnavadjust_addcrossing, "Add and open new crossing", MBV_PICKMASK_NAVANY,
                       &error);
    }

    /* set visualization status */
    if (status == MB_SUCCESS) {
      project.visualization_status = true;
    }

    /* add navigation to the visualization */
    if (status == MB_SUCCESS) {
      /* allocate arrays for navigation of longest section */
      mbv_navpings_alloc = 0;
      for (int i = 0; i < project.num_files; i++) {
        file = &project.files[i];
        for (int j = 0; j < project.files[i].num_sections; j++) {
          section = &file->sections[j];
          mbv_navpings_alloc = MAX(mbv_navpings_alloc, section->num_pings);
        }
      }
      status =
          mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navtime_d, &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navlon, &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navlat, &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navz, &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navheading,
                            &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navspeed, &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navportlon,
                            &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navportlat,
                            &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navstbdlon,
                            &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(double), (void **)&navstbdlat,
                            &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(int), (void **)&navline, &error);
      if (status == MB_SUCCESS)
        status =
            mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(int), (void **)&navshot, &error);
      if (status == MB_SUCCESS)
        status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, mbv_navpings_alloc * sizeof(int), (void **)&navcdp, &error);

      /* read and add navigation for each section */
      if (status == MB_SUCCESS) {
        mbv_navcolor = MBV_COLOR_BLACK;
        mbv_navsize = 2;
        mbv_navswathbounds = true;
        mbv_navline = true;
        mbv_navshot = true;
        mbv_navcdp = true;
        mbv_navdecimation = 1;
        mbv_navpathstatus = MB_PROCESSED_USE;
        for (int i = 0; i < project.num_files; i++) {
          /* set message */
          sprintf(message, "Loading nav %d of %d...", i + 1, project.num_files);
          do_message_on(message);

          file = &project.files[i];
          mbv_navformatorg = file->format;
          for (int j = 0; j < project.files[i].num_sections; j++) {
            section = &file->sections[j];
            sprintf(mbv_file_name, "%s/nvs_%4.4d_%4.4dp.mb71.fnv", project.datadir, file->id, j);
            sprintf(mbv_navname, "%4.4d:%4.4d", file->id, j);
            sprintf(mbv_navpathraw, "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, j);
            sprintf(mbv_navpathprocessed, "%s/nvs_%4.4d_%4.4dp.mb71", project.datadir, file->id, j);
            mbv_navpings = 0;
            if ((nfp = fopen(mbv_file_name, "r")) == NULL) {
              sprintf(mbv_file_name, "%s/nvs_%4.4d_%4.4d.mb71.fnv", project.datadir, file->id, j);
              nfp = fopen(mbv_file_name, "r");
            }
            if (nfp != NULL) {
              while ((nscan = fscanf(nfp, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                                     &year, &month, &day, &hour, &minute, &seconds, &navtime_d[mbv_navpings],
                                     &navlon[mbv_navpings], &navlat[mbv_navpings], &navheading[mbv_navpings],
                                     &navspeed[mbv_navpings], &sonardepth, &roll, &pitch, &heave,
                                     &navportlon[mbv_navpings], &navportlat[mbv_navpings],
                                     &navstbdlon[mbv_navpings], &navstbdlat[mbv_navpings])) > 0) {
                navz[mbv_navpings] = -sonardepth;
                navline[mbv_navpings] = i;
                navshot[mbv_navpings] = j;
                navcdp[mbv_navpings] = mbv_navpings;
                mbv_navpings++;
              }
              fclose(nfp);
            }
            if (mbv_navpings > 0) {
              status = mbview_addnav(mbna_verbose, instance, mbv_navpings, navtime_d, navlon, navlat, navz,
                                     navheading, navspeed, navportlon, navportlat, navstbdlon, navstbdlat, navline,
                                     navshot, navcdp, mbv_navcolor, mbv_navsize, mbv_navname, mbv_navpathstatus,
                                     mbv_navpathraw, mbv_navpathprocessed, mbv_navformatorg, mbv_navswathbounds,
                                     mbv_navline, mbv_navshot, mbv_navcdp, mbv_navdecimation, &error);
            }
          }
        }
      }

      /* deallocate navigation arrays */
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navtime_d, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navlon, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navlat, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navz, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navheading, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navspeed, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navportlon, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navportlat, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navstbdlon, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navstbdlat, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navline, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navshot, &error);
      mb_freed(mbna_verbose, __FILE__, __LINE__, (void **)&navcdp, &error);
    }

    /* add nav ties to the visualization */
    mbnavadjust_reset_visualization_navties();

    /* turn off message */
    do_message_off();

    /* update widgets */
    if (status == MB_SUCCESS)
      status = mbview_update(mbna_verbose, instance, &error);
  }

  /* set sensitivity of widgets that require an mbview instance to be active */
  do_visualize_sensitivity();

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_dismiss_visualization() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  if (project.visualization_status) {
    size_t instance = 0;
    mbview_destroy(mbna_verbose, instance, true, &error);
    project.visualization_status = false;
  }

  const int status = MB_SUCCESS;

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_reset_visualization_navties() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file, *file_1, *file_2;
  struct mbna_section *section, *section_1, *section_2;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  size_t instance;
  int num_navties, num_globalties;
  int npoint;
  int snav_1, snav_2;
  double navtielon[2];
  double navtielat[2];
  int waypoint[2];
  int navtiecolor;
  int navtiesize;
  int navtieeditmode;
  mb_path navtiename;
  int id;

  /* operate on a current visualization */
  if (project.visualization_status) {
    /* delete and deallocate all existing routes */
    mbview_deleteallroutes(mbna_verbose, 0, &error);

    /* count and allocate for the the navties to be displayed according
        to the current settings */
    instance = 0;
    num_navties = 0;
    num_globalties = 0;
    npoint = 2;
    waypoint[0] = 1;
    waypoint[1] = 1;
    navtiesize = 1;
    navtieeditmode = 0;

    /* add crossing ties */
    for (int i = 0; i < project.num_crossings; i++) {
      if (do_check_crossing_listok(i)) {
        crossing = &(project.crossings[i]);
        for (int j = 0; j < crossing->num_ties; j++) {
          file_1 = (struct mbna_file *)&project.files[crossing->file_id_1];
          file_2 = (struct mbna_file *)&project.files[crossing->file_id_2];
          section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
          section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
          tie = (struct mbna_tie *)&crossing->ties[j];
          snav_1 = tie->snav_1;
          snav_2 = tie->snav_2;
          navtielon[0] = section_1->snav_lon[snav_1] + section_1->snav_lon_offset[snav_1];
          navtielat[0] = section_1->snav_lat[snav_1] + section_1->snav_lat_offset[snav_1];
          navtielon[1] = section_2->snav_lon[snav_2] + section_2->snav_lon_offset[snav_2];
          navtielat[1] = section_2->snav_lat[snav_2] + section_2->snav_lat_offset[snav_2];
          if (file_1->block == file_2->block)
            navtiecolor = ROUTE_COLOR_BLUEGREEN;
          else
            navtiecolor = ROUTE_COLOR_BLUE;
          sprintf(navtiename, "%4.4d:%1d %2.2d:%4.4d:%2.2d %2.2d:%4.4d:%2.2d", i, j, file_1->block, crossing->file_id_1,
                  crossing->section_1, file_2->block, crossing->file_id_2, crossing->section_2);
          status = mbview_addroute(mbna_verbose, instance, npoint, navtielon, navtielat, waypoint, navtiecolor,
                                   navtiesize, navtieeditmode, navtiename, &id, &error);
          if (status == MB_SUCCESS)
            num_navties++;
        }
      }
    }

    /* add global ties */
    npoint = 2;
    navtiecolor = ROUTE_COLOR_PURPLE;
    for (int i = 0; i < project.num_files; i++) {
      file = &(project.files[i]);
      for (int j = 0; j < file->num_sections; j++) {
        section = &(file->sections[j]);
        if (section->global_tie_status != MBNA_TIE_NONE && do_check_globaltie_listok(i, j)) {
          navtielon[0] =
              section->snav_lon[section->global_tie_snav] + section->snav_lon_offset[section->global_tie_snav];
          navtielat[0] =
              section->snav_lat[section->global_tie_snav] + section->snav_lat_offset[section->global_tie_snav];
          navtielon[1] =
              section->snav_lon[section->global_tie_snav] + section->snav_lon_offset[section->global_tie_snav];
          navtielat[1] =
              section->snav_lat[section->global_tie_snav] + section->snav_lat_offset[section->global_tie_snav];
          sprintf(navtiename, "%2.2d:%4.4d:%2.2d", file->block, i, j);
          status = mbview_addroute(mbna_verbose, instance, npoint, navtielon, navtielat, waypoint, navtiecolor,
                                   navtiesize, navtieeditmode, navtiename, &id, &error);
          if (status == MB_SUCCESS)
            num_globalties++;
        }
      }
    }

    /* reset the visualization display */
    do_update_visualization_status();
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_visualization_selectcrossingfromroute(int icrossing, int itie) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       icrossing:     %d\n", icrossing);
    fprintf(stderr, "dbg2       itie:          %d\n", itie);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* find next current crossing */
  if (project.open && project.num_crossings > 0 && icrossing >= 0 && icrossing < project.num_crossings) {
    mbna_current_crossing = icrossing;
    mbna_current_tie = itie;

    crossing = &project.crossings[mbna_current_crossing];
    mbna_file_id_1 = crossing->file_id_1;
    mbna_section_1 = crossing->section_1;
    mbna_file_id_2 = crossing->file_id_2;
    mbna_section_2 = crossing->section_2;
    if (crossing->num_ties > 0) {
      if (mbna_current_tie == -1 || mbna_current_tie >= crossing->num_ties)
        mbna_current_tie = 0;
      tie = &crossing->ties[mbna_current_tie];
      mbna_snav_1 = tie->snav_1;
      mbna_snav_1_time_d = tie->snav_1_time_d;
      mbna_snav_2 = tie->snav_2;
      mbna_snav_2_time_d = tie->snav_2_time_d;
      mbna_offset_x = tie->offset_x;
      mbna_offset_y = tie->offset_y;
      mbna_offset_z = tie->offset_z_m;
      /* fprintf(stderr,"%s %d: mbna_offset_z:%f\n",__FILE__,__LINE__,mbna_offset_z); */

      /* reset survey file and section selections */
      if (mbna_file_select == crossing->file_id_1) {
        mbna_section_select = crossing->section_1;
      }
      else if (mbna_file_select == crossing->file_id_2) {
        mbna_section_select = crossing->section_2;
      }
      else {
        mbna_file_select = crossing->file_id_1;
        mbna_survey_select = project.files[crossing->file_id_1].block;
        mbna_section_select = crossing->section_1;
      }
    }
    else {
      mbna_current_tie = -1;
    }
  }

  /* load the crossing */
  if (mbna_current_crossing >= 0) {
    /* put up message */
    sprintf(message, "Loading crossing %d...", mbna_current_crossing);
    do_message_on(message);

    mbnavadjust_crossing_load();

    /* turn off message */
    do_message_off();
  }
  // fprintf(stderr,"Done with mbnavadjust_visualization_selectcrossingfromroute: mbna_current_crossing:%d
  // mbna_crossing_select:%d mbna_current_tie:%d mbna_tie_select:%d\n",
  // mbna_current_crossing,mbna_crossing_select,mbna_current_tie,mbna_tie_select);

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mbnavadjust_visualization_selectcrossingfromnav(int ifile1, int isection1, int ifile2, int isection2) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       ifile1:         %d\n", ifile1);
    fprintf(stderr, "dbg2       isection1:      %d\n", isection1);
    fprintf(stderr, "dbg2       ifile2:         %d\n", ifile2);
    fprintf(stderr, "dbg2       isection2:      %d\n", isection2);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* figure out if a valid crossing has been selected */
  // fprintf(stderr,"%s: looking for crossing in nav picks: %4.4d:%2.2d %4.4d:%2.2d\n",
  // __func__, ifile1, isection1, ifile2, isection2);
  bool found = false;
  if (ifile1 >= 0 && ifile1 < project.num_files && isection1 >= 0 && isection1 < project.files[ifile1].num_sections &&
      ifile2 >= 0 && ifile2 < project.num_files && isection2 >= 0 && isection2 < project.files[ifile2].num_sections) {
    for (int icrossing = 0; icrossing < project.num_crossings && !found; icrossing++) {
      crossing = &project.crossings[icrossing];
      if ((crossing->file_id_1 == ifile1 && crossing->section_1 == isection1 && crossing->file_id_2 == ifile2 &&
           crossing->section_2 == isection2) ||
          (crossing->file_id_1 == ifile2 && crossing->section_1 == isection2 && crossing->file_id_2 == ifile1 &&
           crossing->section_2 == isection1)) {
        found = true;
        mbna_current_crossing = icrossing;
        mbna_current_tie = -1;
      }
    }

    if (found) {
      // fprintf(stderr,"%s: found crossing %d\n",__func__,mbna_current_crossing);
      crossing = &project.crossings[mbna_current_crossing];
      mbna_file_id_1 = crossing->file_id_1;
      mbna_section_1 = crossing->section_1;
      mbna_file_id_2 = crossing->file_id_2;
      mbna_section_2 = crossing->section_2;
      if (crossing->num_ties > 0) {
        mbna_current_tie = 0;
        tie = &crossing->ties[0];
        mbna_snav_1 = tie->snav_1;
        mbna_snav_1_time_d = tie->snav_1_time_d;
        mbna_snav_2 = tie->snav_2;
        mbna_snav_2_time_d = tie->snav_2_time_d;
        mbna_offset_x = tie->offset_x;
        mbna_offset_y = tie->offset_y;
        mbna_offset_z = tie->offset_z_m;
      }

      /* reset survey file and section selections */
      if (mbna_file_select == crossing->file_id_1) {
        mbna_section_select = crossing->section_1;
      }
      else if (mbna_file_select == crossing->file_id_2) {
        mbna_section_select = crossing->section_2;
      }
      else {
        mbna_file_select = crossing->file_id_1;
        mbna_survey_select = project.files[crossing->file_id_1].block;
        mbna_section_select = crossing->section_1;
      }
    }
    else {
      // fprintf(stderr,"%s: no crossing found, unselect any crossing and tie\n",__func__);
      mbna_current_crossing = MBNA_SELECT_NONE;
      mbna_current_tie = MBNA_SELECT_NONE;
    }

    /* load the crossing */
    if (mbna_current_crossing >= 0) {
      /* put up message */
      sprintf(message, "Loading crossing %d...", mbna_current_crossing);
      do_message_on(message);

      mbnavadjust_crossing_load();

      /* turn off message */
      do_message_off();
    }

    /* else unload previously loaded crossing */
    else if (mbna_naverr_load) {
      /* put up message */
      sprintf(message, "Unloading crossing...");
      do_message_on(message);

      mbnavadjust_crossing_unload();

      /* turn off message */
      do_message_off();
    }
  }

  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBnavadjust function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
