/*--------------------------------------------------------------------
 *    The MB-system:  mbnavadjust_prog.c  3/23/00
 *
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

#include <assert.h>
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
#  ifndef WIN32
#    define WIN32
#  endif
#  include <WinSock2.h>
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
mb_pathplusplus message;
mb_path error1;
mb_pathplus error2;
mb_path error3;

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
  memset(project.name, 0, sizeof(project.name));
  strcpy(project.name, "None");
  memset(project.path, 0, sizeof(project.path));
  memset(project.datadir, 0, sizeof(project.datadir));
  project.num_files = 0;
  project.num_files_alloc = 0;
  project.files = NULL;
  project.num_surveys = 0;
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
  project.refgrid_status = MBNA_REFGRID_UNLOADED;
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
  project.use_mode = MBNA_USE_MODE_PRIMARY;
  project.section_length = 0.14;
  project.section_soundings = 100000;
  project.decimation = 1;
  project.precision = SIGMA_MINIMUM;
  project.smoothing = MBNA_SMOOTHING_DEFAULT;
  project.zoffsetwidth = 1.0;
  project.triangle_scale = 0.0;
  mbna_file_id_1 = MBNA_SELECT_NONE;
  mbna_section_1 = MBNA_SELECT_NONE;
  mbna_file_id_2 = MBNA_SELECT_NONE;
  mbna_section_2 = MBNA_SELECT_NONE;
  mbna_current_crossing = MBNA_SELECT_NONE;
  mbna_current_tie = MBNA_SELECT_NONE;
  mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;
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
  mbna_allow_add_tie = false;
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
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
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
  mb_path ifile = "";

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
    snprintf(error2, sizeof(error2), "Project %s", project.name);
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
          snprintf(error3, sizeof(error3), "Data directory already exists.");
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
        project.zoffsetwidth = 1.0;
        project.bin_beams_bath = mbna_bin_beams_bath;
        project.bin_swathwidth = mbna_bin_swathwidth;
        project.bin_pseudobeamwidth = mbna_bin_pseudobeamwidth;
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
        else if ((status = mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error)) == MB_FAILURE) {
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
    snprintf(message, sizeof(message), "%s\n > %s\n", error1, error2);
    do_info_add(message, true);
  }
  else {
    /* open log file */
    snprintf(message, sizeof(message), "%s/log.txt", project.datadir);
    project.logfp = fopen(message, "w");

    /* add info text */
    snprintf(message, sizeof(message), "New project initialized: %s\n > Project home: %s\n", project.name, project.home);
    do_info_add(message, true);
    if (project.logfp != NULL) {
      snprintf(message, sizeof(message), "Log file %s/log.txt opened\n", project.datadir);
      do_info_add(message, true);
    }
    else {
      snprintf(message, sizeof(message), "Unable to open log file %s/log.txt\n", project.datadir);
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
    snprintf(error2, sizeof(error2), "Project %s", project.name);
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
        char *getcwd_result = getcwd(project.path, MB_PATH_MAXLINE);
        assert(getcwd_result != NULL && strlen(project.path) > 0);
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
          snprintf(error3, sizeof(error3), "Data directory does not exist.");
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
    snprintf(message, sizeof(message), "%s\n > %s\n", error1, error2);
    do_info_add(message, true);
  }
  else {
    /* open log file */
    snprintf(message, sizeof(message), "%s/log.txt", project.datadir);
    project.logfp = fopen(message, "a");

    /* add info text */
    snprintf(message, sizeof(message), 
            "Project opened: %s\n > Project home: %s\n > Number of Files: %d\n > Number of Crossings Found: %d\n > Number of "
            "Crossings Analyzed: %d\n > Number of Navigation Ties: %d\n",
            project.name, project.home, project.num_files, project.num_crossings, project.num_crossings_analyzed,
            project.num_ties);
    do_info_add(message, true);
    if (project.logfp != NULL) {
      snprintf(message, sizeof(message), "Log file %s/log.txt opened\n", project.datadir);
      do_info_add(message, true);
    }
    else {
      snprintf(message, sizeof(message), "Unable to open log file %s/log.txt\n", project.datadir);
      do_info_add(message, true);
    }

    /* update topography grid if it does not exist */
    mb_pathplus path;
    snprintf(path, sizeof(path), "%s/ProjectTopoAdj.grd", project.datadir);
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
      snprintf(message, sizeof(message), "Setting selected files to POOR nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_POORNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_POORNAV;

            /* add info text */
            snprintf(message, sizeof(message), "Set file %d to have poor nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      snprintf(message, sizeof(message), "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
      snprintf(message, sizeof(message), "Setting selected files to GOOD nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_GOODNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_GOODNAV;

            /* add info text */
            snprintf(message, sizeof(message), "Set file %d to have good nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      snprintf(message, sizeof(message), "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
      snprintf(message, sizeof(message), "Setting selected files to FIXED nav status...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDNAV;

            /* add info text */
            snprintf(message, sizeof(message), "Set file %d to have fixed nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      snprintf(message, sizeof(message), "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
      snprintf(message, sizeof(message), "Setting selected files to FIXED XY nav...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDXYNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDXYNAV;

            /* add info text */
            snprintf(message, sizeof(message), "Set file %d to have fixed xy nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      snprintf(message, sizeof(message), "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
      snprintf(message, sizeof(message), "Setting selected files to FIXED Z nav...");
      do_message_on(message);

      for (int i = 0; i < project.num_files; i++) {
        if (project.files[i].block == block) {
          if (project.files[i].status != MBNA_FILE_FIXEDZNAV) {
            project.inversion_status = MBNA_INVERSION_OLD;
            project.files[i].status = MBNA_FILE_FIXEDZNAV;

            /* add info text */
            snprintf(message, sizeof(message), "Set file %d to have fixed z nav: %s\n", mbna_file_select, project.files[mbna_file_select].file);
            fprintf(stderr, "%s", message);
            do_info_add(message, true);
          }
        }
      }
      snprintf(message, sizeof(message), "Writing project...");
      do_message_on(message);

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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

  if (project.open && project.num_files > 0) {

    bool status_change = false;

    /* deal with global tie case */
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      if (mbna_file_select != MBNA_SELECT_NONE
        && mbna_section_select != MBNA_SELECT_NONE) {
        struct mbna_file *file = &project.files[mbna_file_select];
        struct mbna_section *section = &file->sections[mbna_section_select];
        if (section->globaltie.status == MBNA_TIE_XY
            || section->globaltie.status == MBNA_TIE_Z) {
          section->globaltie.status = MBNA_TIE_XYZ;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to XYZ\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_XY_FIXED
            || section->globaltie.status == MBNA_TIE_Z_FIXED) {
          section->globaltie.status = MBNA_TIE_XYZ_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to XYZ\n", mbna_file_select, mbna_section_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* deal with crossing tie case */
    else {
      /* set selected file's block to good nav */
      if (mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
        struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
        struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
        if (tie->status == MBNA_TIE_XY
            || tie->status == MBNA_TIE_Z) {
          tie->status = MBNA_TIE_XYZ;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XYZ\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_XY_FIXED
            || tie->status == MBNA_TIE_Z_FIXED) {
          tie->status = MBNA_TIE_XYZ_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XYZ fixed\n", mbna_crossing_select, mbna_tie_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    if (status_change) {
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
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
int mbnavadjust_set_tie_xy() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  if (project.open && project.num_files > 0) {

    bool status_change = false;

    /* deal with global tie case */
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      if (mbna_file_select != MBNA_SELECT_NONE
        && mbna_section_select != MBNA_SELECT_NONE) {
        struct mbna_file *file = &project.files[mbna_file_select];
        struct mbna_section *section = &file->sections[mbna_section_select];
        if (section->globaltie.status == MBNA_TIE_XYZ
            || section->globaltie.status == MBNA_TIE_Z) {
          section->globaltie.status = MBNA_TIE_XY;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to XY\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_XYZ_FIXED
            || section->globaltie.status == MBNA_TIE_Z_FIXED) {
          section->globaltie.status = MBNA_TIE_XY_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to XY\n", mbna_file_select, mbna_section_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* deal with crossing tie case */
    else {
      /* set selected file's block to good nav */
      if (mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
        struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
        struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
        if (tie->status == MBNA_TIE_XYZ
            || tie->status == MBNA_TIE_Z) {
          tie->status = MBNA_TIE_XY;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XY\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_XYZ_FIXED
            || tie->status == MBNA_TIE_Z_FIXED) {
          tie->status = MBNA_TIE_XY_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XY fixed\n", mbna_crossing_select, mbna_tie_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    if (status_change) {
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
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
int mbnavadjust_set_tie_z() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  if (project.open && project.num_files > 0) {

    bool status_change = false;

    /* deal with global tie case */
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      if (mbna_file_select != MBNA_SELECT_NONE
        && mbna_section_select != MBNA_SELECT_NONE) {
        struct mbna_file *file = &project.files[mbna_file_select];
        struct mbna_section *section = &file->sections[mbna_section_select];
        if (section->globaltie.status == MBNA_TIE_XYZ
            || section->globaltie.status == MBNA_TIE_XY) {
          section->globaltie.status = MBNA_TIE_Z;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to Z\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_XYZ_FIXED
            || section->globaltie.status == MBNA_TIE_XY_FIXED) {
          section->globaltie.status = MBNA_TIE_Z_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d to Z\n", mbna_file_select, mbna_section_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* deal with crossing tie case */
    else {
      /* set selected file's block to good nav */
      if (mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
        struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
        struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
        if (tie->status == MBNA_TIE_XYZ
            || tie->status == MBNA_TIE_XY) {
          tie->status = MBNA_TIE_Z;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XYZ\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_XYZ_FIXED
            || tie->status == MBNA_TIE_XY_FIXED) {
          tie->status = MBNA_TIE_Z_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to XYZ fixed\n", mbna_crossing_select, mbna_tie_select);
        }
        if (status_change) {
          /* add info text */
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    if (status_change) {
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
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
int mbnavadjust_set_tie_fixed() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  if (project.open && project.num_files > 0) {

    bool status_change = false;

    /* deal with global tie case */
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      if (mbna_file_select != MBNA_SELECT_NONE
        && mbna_section_select != MBNA_SELECT_NONE) {
        struct mbna_file *file = &project.files[mbna_file_select];
        struct mbna_section *section = &file->sections[mbna_section_select];
        if (section->globaltie.status == MBNA_TIE_XYZ) {
          section->globaltie.status = MBNA_TIE_XYZ_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to XYZ fixed\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_XY) {
          section->globaltie.status = MBNA_TIE_XY_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to XY fixed\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_Z) {
          section->globaltie.status = MBNA_TIE_Z_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to Z fixed\n", mbna_file_select, mbna_section_select);
        }

          /* add info text */
        if (status_change) {
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* deal with crossing tie case */
    else {
      /* set selected file's block to good nav */
      if (mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
        struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
        struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
        if (tie->status == MBNA_TIE_XYZ) {
          tie->status = MBNA_TIE_XYZ_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to fix XYZ\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_XY) {
          tie->status = MBNA_TIE_XY_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to fix XY\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_Z) {
          tie->status = MBNA_TIE_Z_FIXED;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to fix Z\n", mbna_crossing_select, mbna_tie_select);
        }

          /* add info text */
        if (status_change) {
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    if (status_change) {
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
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
int mbnavadjust_set_tie_unfixed() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  if (project.open && project.num_files > 0) {

    bool status_change = false;

    /* deal with global tie case */
    if (mbna_view_list == MBNA_VIEW_LIST_FILESECTIONS
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      if (mbna_file_select != MBNA_SELECT_NONE
        && mbna_section_select != MBNA_SELECT_NONE) {
        struct mbna_file *file = &project.files[mbna_file_select];
        struct mbna_section *section = &file->sections[mbna_section_select];
        if (section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
          section->globaltie.status = MBNA_TIE_XYZ;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to XYZ fixed\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_XY_FIXED) {
          section->globaltie.status = MBNA_TIE_XY;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to XY fixed\n", mbna_file_select, mbna_section_select);
        }
        else if (section->globaltie.status == MBNA_TIE_Z_FIXED) {
          section->globaltie.status = MBNA_TIE_Z;
          status_change = true;
          snprintf(message, sizeof(message), "Set global tie file %d section %d  to Z fixed\n", mbna_file_select, mbna_section_select);
        }

          /* add info text */
        if (status_change) {
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    /* deal with crossing tie case */
    else {
      /* set selected file's block to good nav */
      if (mbna_crossing_select >= 0 && mbna_tie_select >= 0) {
        struct mbna_crossing *crossing = &(project.crossings[mbna_crossing_select]);
        struct mbna_tie *tie = (struct mbna_tie *)&crossing->ties[mbna_tie_select];
        if (tie->status == MBNA_TIE_XYZ_FIXED) {
          tie->status = MBNA_TIE_XYZ;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to unfix XYZ\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_XY_FIXED) {
          tie->status = MBNA_TIE_XY;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to unfix XY\n", mbna_crossing_select, mbna_tie_select);
        }
        else if (tie->status == MBNA_TIE_Z_FIXED) {
          tie->status = MBNA_TIE_Z;
          status_change = true;
          snprintf(message, sizeof(message), "Set crossing %d tie %d to unfix Z\n", mbna_crossing_select, mbna_tie_select);
        }

          /* add info text */
        if (status_change) {
          do_info_add(message, true);
          fprintf(stderr, "%s\n", message);
        }
      }
    }

    if (status_change) {
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* write out updated project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
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
int mbnavadjust_naverr_save() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

// fprintf(stderr, "%s:%d:%s start\n",
// __FILE__, __LINE__, __FUNCTION__);
// clock_t start = clock();

  /* save offsets if crossing loaded and ties set */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING
      && project.num_crossings > 0 && mbna_current_crossing >= 0
      && mbna_current_tie >= 0) {

    /* save offsets if ties set */
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
    if (crossing->num_ties > 0) {
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
        mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
        project.save_count = 0;
      }

      /* add info text */
      snprintf(message, sizeof(message), "Save Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_tie, mbna_current_crossing, crossing->file_id_1, crossing->section_1, tie->snav_1,
              crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);
    }
  }

  /* save offsets if section loaded and global tie set */
  else if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION
      && project.num_files > 0 && mbna_current_file >= 0
      && mbna_current_section >= 0
      && project.files[mbna_current_file].sections[mbna_current_section].status == MBNA_CROSSING_STATUS_SET) {

    /* save offsets if ties set */
    struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
    struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
    struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;

    /* get new tie values */
    fprintf(stderr, "global tie of section %2.2d:%2.2d:%2.2d:%2.2d saved...\n",
              file->block, mbna_current_file, mbna_current_section, globaltie->snav);
    globaltie->status = MBNA_TIE_XY;
    globaltie->snav = mbna_snav_2;
    globaltie->refgrid_id = project.refgrid_select;
    globaltie->snav_time_d = mbna_snav_2_time_d;
    if (globaltie->inversion_status == MBNA_INVERSION_CURRENT &&
        (globaltie->offset_x != mbna_offset_x || globaltie->offset_y != mbna_offset_y || globaltie->offset_z_m != mbna_offset_z)) {
      globaltie->inversion_status = MBNA_INVERSION_OLD;
              project.modelplot_uptodate = false;
    }
    globaltie->offset_x = mbna_offset_x;
    globaltie->offset_y = mbna_offset_y;
    globaltie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
    globaltie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
    globaltie->offset_z_m = mbna_offset_z;
    globaltie->sigmar1 = mbna_minmisfit_sr1;
    globaltie->sigmar2 = mbna_minmisfit_sr2;
    globaltie->sigmar3 = mbna_minmisfit_sr3;
    for (int i = 0; i < 3; i++) {
      globaltie->sigmax1[i] = mbna_minmisfit_sx1[i];
      globaltie->sigmax2[i] = mbna_minmisfit_sx2[i];
      globaltie->sigmax3[i] = mbna_minmisfit_sx3[i];
    }
    if (globaltie->sigmar1 < MBNA_SMALL) {
      globaltie->sigmar1 = MBNA_SMALL;
      globaltie->sigmax1[0] = 1.0;
      globaltie->sigmax1[1] = 0.0;
      globaltie->sigmax1[2] = 0.0;
    }
    if (globaltie->sigmar2 < MBNA_SMALL) {
      globaltie->sigmar2 = MBNA_SMALL;
      globaltie->sigmax2[0] = 0.0;
      globaltie->sigmax2[1] = 1.0;
      globaltie->sigmax2[2] = 0.0;
    }
    if (globaltie->sigmar3 < MBNA_ZSMALL) {
      globaltie->sigmar3 = MBNA_ZSMALL;
      globaltie->sigmax3[0] = 0.0;
      globaltie->sigmax3[1] = 0.0;
      globaltie->sigmax3[2] = 1.0;
    }
    if (project.inversion_status == MBNA_INVERSION_CURRENT)
      project.inversion_status = MBNA_INVERSION_OLD;

    if (globaltie->inversion_status != MBNA_INVERSION_NONE) {
        globaltie->dx_m = globaltie->offset_x_m - globaltie->inversion_offset_x_m;
        globaltie->dy_m = globaltie->offset_y_m - globaltie->inversion_offset_y_m;
        globaltie->dz_m = globaltie->offset_z_m - globaltie->inversion_offset_z_m;
        globaltie->sigma_m = sqrt(globaltie->dx_m * globaltie->dx_m + globaltie->dy_m * globaltie->dy_m + globaltie->dz_m * globaltie->dz_m);
        globaltie->dr1_m = fabs((globaltie->inversion_offset_x_m - globaltie->offset_x_m) * globaltie->sigmax1[0] +
                       (globaltie->inversion_offset_y_m - globaltie->offset_y_m) * globaltie->sigmax1[1] +
                       (globaltie->inversion_offset_z_m - globaltie->offset_z_m) * globaltie->sigmax1[2]) /
                  globaltie->sigmar1;
        globaltie->dr2_m = fabs((globaltie->inversion_offset_x_m - globaltie->offset_x_m) * globaltie->sigmax2[0] +
                       (globaltie->inversion_offset_y_m - globaltie->offset_y_m) * globaltie->sigmax2[1] +
                       (globaltie->inversion_offset_z_m - globaltie->offset_z_m) * globaltie->sigmax2[2]) /
                  globaltie->sigmar2;
        globaltie->dr3_m = fabs((globaltie->inversion_offset_x_m - globaltie->offset_x_m) * globaltie->sigmax3[0] +
                       (globaltie->inversion_offset_y_m - globaltie->offset_y_m) * globaltie->sigmax3[1] +
                       (globaltie->inversion_offset_z_m - globaltie->offset_z_m) * globaltie->sigmax3[2]) /
                  globaltie->sigmar3;
        globaltie->rsigma_m = sqrt(globaltie->dr1_m * globaltie->dr1_m
                                    + globaltie->dr2_m * globaltie->dr2_m
                                    + globaltie->dr3_m * globaltie->dr3_m);
    }

    /* write updated project */
    project.save_count++;
    if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
      project.save_count = 0;
    }

    /* add info text */
    snprintf(message, sizeof(message), "Save Global Tie of Section %d:%d:%d\n > Offsets: %f %f %f m\n",
            mbna_current_file, mbna_current_section, globaltie->snav,
            globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
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

// clock_t end = clock();
// double time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
// fprintf(stderr, "%s:%d:%s done in %.6f seconds...\n\n",
// __FILE__, __LINE__, __FUNCTION__, time_used);

  return (status);
}

/*--------------------------------------------------------------------*/
int mbnavadjust_naverr_specific_crossing(int new_crossing, int new_tie) {
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
      snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
int mbnavadjust_naverr_specific_section(int new_file, int new_section) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2               new_file:     %d\n", new_file);
    fprintf(stderr, "dbg2               new_section:  %d\n", new_section);
  }

  /* get current section */
  if (project.open && project.num_files > 0) {
    /* get next section */
    if (new_file >= 0 && new_file < project.num_files
        && new_section >= 0 && new_section < project.files[new_file].num_sections) {
      mbna_current_file = new_file;
      mbna_current_section = new_section;
    }
    else {
      mbna_current_file = 0;
      mbna_current_section = 0;
    }

    /* retrieve global tie parameters */
    if (mbna_current_file >= 0 && mbna_current_section >= 0) {
      struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
      struct mbna_globaltie *globaltie = &section->globaltie;

      /* file and section for reference grid are either -1 if no refgrid imported
         or 0 if refgrid imported */
      if (project.refgrid_status != MBNA_REFGRID_UNLOADED) {
        mbna_file_id_1 = 0;
        mbna_section_1 = 0;
      } else {
        mbna_file_id_1 = -1;
        mbna_section_1 = -1;
      }

      mbna_file_id_2 = mbna_current_file;
      mbna_section_2 = mbna_current_section;

      if (globaltie->status != MBNA_TIE_NONE ) {
        mbna_current_tie = 0;
        mbna_snav_1 = 0;
        mbna_snav_1_time_d = 0.0;
        mbna_snav_2 = globaltie->snav;
        mbna_snav_2_time_d = globaltie->snav_time_d;
        mbna_offset_x = globaltie->offset_x;
        mbna_offset_y = globaltie->offset_y;
        mbna_offset_z = globaltie->offset_z_m;
      } else {
        mbna_current_tie = -1;
        mbna_snav_1 = 0;
        mbna_snav_1_time_d = 0.0;
        mbna_snav_2 = 0;
        mbna_snav_2_time_d = 0.0;
        mbna_offset_x = 0.0;
        mbna_offset_y = 0.0;
        mbna_offset_z = 0.0;
      }

      /* reset survey file and section selections */
      mbna_section_select = mbna_current_section;
      mbna_file_select = mbna_current_file;
      mbna_survey_select = project.files[mbna_current_file].block;
    }

    /* load the section */
    if (mbna_current_file >= 0 && mbna_current_section >= 0) {
      /* put up message */
      snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
      do_message_on(message);
      mbnavadjust_referenceplussection_load();

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
int mbnavadjust_naverr_next_crossing() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* find next crossing */
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
    snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
int mbnavadjust_naverr_next_section() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* get previous section */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION
      && project.num_files > 0) {
    int ifile_next = -1;
    int isection_next = -1;
    if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      bool found = false;
      for (int ifile=project.num_files-1; ifile >= 0; ifile--) {
        for (int isection=project.files[ifile].num_sections-1; isection >= 0; isection--) {
          if (project.files[ifile].sections[isection].status == MBNA_CROSSING_STATUS_SET) {
            if (mbna_current_file >= 0 && mbna_current_section >= 0) {
              if (ifile > mbna_current_file || (ifile == mbna_current_file && isection > mbna_current_section)) {
                ifile_next = ifile;
                isection_next = isection;
                found = true;
              } else if (!found && ((ifile == mbna_current_file && isection < mbna_current_section) || ifile < mbna_current_file)) {
                ifile_next = ifile;
                isection_next = isection;
              }
            }
            else {
              ifile_next = ifile;
              isection_next = isection;
            }
          }
        }
      }
      if (ifile_next >= 0 && isection_next >= 0) {
        mbna_current_file = ifile_next;
        mbna_current_section = isection_next;
      }
    }
    else if (mbna_current_file >= 0 && mbna_current_section >= 0) {
      if (mbna_current_section < project.files[mbna_current_file].num_sections - 1) {
        mbna_current_section++;
      }
      else if (mbna_current_file < project.num_files - 1) {
        mbna_current_file++;
        mbna_current_section = 0;
      }
      else {
        mbna_current_file = 0;
        mbna_current_section = 0;
      }
    }
  }

  /* retrieve global tie parameters */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section->globaltie;

    /* file and section for reference grid are either -1 if no refgrid imported
       or 0 if refgrid imported */
    if (project.refgrid_status != MBNA_REFGRID_UNLOADED) {
      mbna_file_id_1 = 0;
      mbna_section_1 = 0;
    } else {
      mbna_file_id_1 = -1;
      mbna_section_1 = -1;
    }

    mbna_file_id_2 = mbna_current_file;
    mbna_section_2 = mbna_current_section;

    if (globaltie->status != MBNA_TIE_NONE ) {
      mbna_current_tie = 0;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      mbna_offset_x = globaltie->offset_x;
      mbna_offset_y = globaltie->offset_y;
      mbna_offset_z = globaltie->offset_z_m;
    } else {
      mbna_current_tie = -1;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = 0.0;
      mbna_offset_x = 0.0;
      mbna_offset_y = 0.0;
      mbna_offset_z = 0.0;
    }

    /* reset survey file and section selections */
    mbna_section_select = mbna_current_section;
    mbna_file_select = mbna_current_file;
    mbna_survey_select = project.files[mbna_current_file].block;
  }

  int status = MB_SUCCESS;

  /* load the section */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    /* put up message */
    snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_on(message);
    mbnavadjust_referenceplussection_load();

    /* turn off message */
    do_message_off();
  }

  /* else unload previously loaded section */
  else if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    status = mbnavadjust_referenceplussection_unload();
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
int mbnavadjust_naverr_previous_crossing() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* find previous crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING
      && project.num_crossings > 0) {

    /* get previous good crossing */
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
      snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
int mbnavadjust_naverr_previous_section() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* get previous section */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION
      && project.num_files > 0) {
    int ifile_previous = -1;
    int isection_previous = -1;
    if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
      bool found = false;
      for (int ifile=0; ifile < project.num_files; ifile++) {
        for (int isection=0; isection < project.files[ifile].num_sections; isection++) {
          if (project.files[ifile].sections[isection].status == MBNA_CROSSING_STATUS_SET) {
            if (mbna_current_file >= 0 && mbna_current_section >= 0) {
              if (ifile < mbna_current_file || (ifile == mbna_current_file && isection < mbna_current_section)) {
                ifile_previous = ifile;
                isection_previous = isection;
                found = true;
              } else if (!found && ((ifile == mbna_current_file && isection > mbna_current_section) || ifile > mbna_current_file)) {
                ifile_previous = ifile;
                isection_previous = isection;
              }
            }
            else if (!found) {
              ifile_previous = ifile;
              isection_previous = isection;
              found = true;
            }
          }
        }
      }
      if (ifile_previous >= 0 && isection_previous >= 0) {
        mbna_current_file = ifile_previous;
        mbna_current_section = isection_previous;
      }
    }
    else if (mbna_current_file >= 0 && mbna_current_section >= 0) {
      if (mbna_current_section > 0) {
        mbna_current_section--;
      }
      else if (mbna_current_file > 0) {
        mbna_current_file--;
        mbna_current_section = project.files[mbna_current_file].num_sections - 1;
      }
      else {
        mbna_current_file = project.num_files - 1;
        mbna_current_section = project.files[mbna_current_file].num_sections - 1;
      }
    }
  }

  /* retrieve global tie parameters */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section->globaltie;

    /* file and section for reference grid are either -1 if no refgrid imported
       or 0 if refgrid imported */
    if (project.refgrid_status != MBNA_REFGRID_UNLOADED) {
      mbna_file_id_1 = 0;
      mbna_section_1 = 0;
    } else {
      mbna_file_id_1 = -1;
      mbna_section_1 = -1;
    }

    mbna_file_id_2 = mbna_current_file;
    mbna_section_2 = mbna_current_section;

    if (globaltie->status != MBNA_TIE_NONE ) {
      mbna_current_tie = 0;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      mbna_offset_x = globaltie->offset_x;
      mbna_offset_y = globaltie->offset_y;
      mbna_offset_z = globaltie->offset_z_m;
    } else {
      mbna_current_tie = -1;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = 0.0;
      mbna_offset_x = 0.0;
      mbna_offset_y = 0.0;
      mbna_offset_z = 0.0;
    }

    /* reset survey file and section selections */
    mbna_section_select = mbna_current_section;
    mbna_file_select = mbna_current_file;
    mbna_survey_select = project.files[mbna_current_file].block;
  }

  int status = MB_SUCCESS;

  /* load the section */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    /* put up message */
    snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_on(message);

    mbnavadjust_referenceplussection_load();

    /* turn off message */
    do_message_off();
  }

  /* else unload previously loaded section */
  else if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    status = mbnavadjust_referenceplussection_unload();
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
int mbnavadjust_naverr_nextunset_crossing() {
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
    snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
    do_message_on(message);

    mbnavadjust_crossing_load();

    /* turn off message */
    do_message_off();
  }

  /* else unload previously loaded crossing */
  else if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
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
int mbnavadjust_naverr_nextunset_section() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* find next current unset section */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION
      && project.num_files > 0 && mbna_current_file >= 0
      && mbna_current_section >= 0) {
    /* find next unset section if any */
    int ifilenext = -1;
    int isectionnext = -1;
    for (int ifile = MAX(mbna_current_file, 0); ifile < project.num_files && ifilenext == -1; ifile++) {
      struct mbna_file *file = &project.files[ifile];
      int isectionstart = mbna_current_section + 1;
      if (ifile > mbna_current_file)
        isectionstart = 0;
      for (int isection = isectionstart; isection < project.files[ifile].num_sections; isection++) {
        struct mbna_section *section = &file->sections[isection];
        struct mbna_globaltie *globaltie = &section->globaltie;
        if (globaltie->status == MBNA_TIE_NONE
          && do_check_section_listok(ifile, isection) && ifilenext < 0) {
          ifilenext = ifile;
          isectionnext = isection;
        }
      }
    }
    if (ifilenext == -1) {
      for (int ifile = 0; ifile <= mbna_current_file; ifile++) {
        struct mbna_file *file = &project.files[ifile];
        int isectionend = project.files[ifile].num_sections - 1;
        if (ifile == mbna_current_file)
          isectionend = mbna_current_section - 1;
        for (int isection = 0; isection <= isectionend; isection++) {
          struct mbna_section *section = &file->sections[isection];
          struct mbna_globaltie *globaltie = &section->globaltie;
          if (globaltie->status == MBNA_TIE_NONE
            && do_check_section_listok(ifile, isection)
            && ifilenext < 0) {
            ifilenext = ifile;
            isectionnext = isection;
          }
        }
      }
    }
  mbna_current_file = ifilenext;
  mbna_current_section = isectionnext;
  }

  /* retrieve global tie parameters */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section->globaltie;

    /* file and section for reference grid are either -1 if no refgrid imported
       or 0 if refgrid imported */
    if (project.refgrid_status != MBNA_REFGRID_UNLOADED) {
      mbna_file_id_1 = 0;
      mbna_section_1 = 0;
    } else {
      mbna_file_id_1 = -1;
      mbna_section_1 = -1;
    }

    mbna_file_id_2 = mbna_current_file;
    mbna_section_2 = mbna_current_section;

    if (globaltie->status != MBNA_TIE_NONE ) {
      mbna_current_tie = 0;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      mbna_offset_x = globaltie->offset_x;
      mbna_offset_y = globaltie->offset_y;
      mbna_offset_z = globaltie->offset_z_m;
    } else {
      mbna_current_tie = -1;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = 0;
      mbna_snav_2_time_d = 0.0;
      mbna_offset_x = 0.0;
      mbna_offset_y = 0.0;
      mbna_offset_z = 0.0;
    }

    /* reset survey file and section selections */
    mbna_section_select = mbna_current_section;
    mbna_file_select = mbna_current_file;
    mbna_survey_select = project.files[mbna_current_file].block;
  }

  int status = MB_SUCCESS;

  /* load the section */
  if (mbna_current_file >= 0 && mbna_current_section >= 0) {
    /* put up message */
    snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_on(message);

    mbnavadjust_referenceplussection_load();

    /* turn off message */
    do_message_off();
  }

  /* else unload previously loaded section */
  else if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    status = mbnavadjust_referenceplussection_unload();
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

  /* deal with crossings */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    /* retrieve crossing parameters */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0
      && project.crossings[mbna_current_crossing].num_ties < MBNA_SNAV_NUM) {
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
      tie->icrossing = mbna_current_crossing;
      tie->itie = mbna_current_tie;
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
        mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
        project.save_count = 0;
      }

      /* add info text */
      snprintf(message, sizeof(message), "Add Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_tie, mbna_current_crossing, crossing->file_id_1, crossing->section_1, tie->snav_1,
              crossing->file_id_2, crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);

      if (mbna_verbose >= 2) {
        fprintf(stderr, "\ndbg2  Crossing tie added in MBnavadjust function <%s>\n", __func__);
        fprintf(stderr, "dbg2    mbna_current_crossing:        %d\n", mbna_current_crossing);
        fprintf(stderr, "dbg2    crossing->file_id_1:          %d\n", crossing->file_id_1);
        fprintf(stderr, "dbg2    crossing->section_1:          %d\n", crossing->section_1);
        fprintf(stderr, "dbg2    crossing->file_id_2:          %d\n", crossing->file_id_2);
        fprintf(stderr, "dbg2    crossing->section_2:          %d\n", crossing->section_2);
        fprintf(stderr, "dbg2    crossing->num_ties:           %d\n", crossing->num_ties);
        fprintf(stderr, "dbg2    mbna_current_tie:             %d\n", mbna_current_tie);
        fprintf(stderr, "dbg2    tie->status:                  %d\n", tie->status);
        fprintf(stderr, "dbg2    tie->icrossing:               %d\n", tie->icrossing);
        fprintf(stderr, "dbg2    tie->itie:                    %d\n", tie->itie);
        fprintf(stderr, "dbg2    tie->snav_1:                  %d\n", tie->snav_1);
        fprintf(stderr, "dbg2    tie->snav_1_time_d:           %f\n", tie->snav_1_time_d);
        fprintf(stderr, "dbg2    tie->snav_2:                  %d\n", tie->snav_2);
        fprintf(stderr, "dbg2    tie->snav_2_time_d:           %f\n", tie->snav_2_time_d);
        fprintf(stderr, "dbg2    tie->offset_x:                %f\n", tie->offset_x);
        fprintf(stderr, "dbg2    tie->offset_y:                %f\n", tie->offset_y);
        fprintf(stderr, "dbg2    tie->offset_x_m:              %f\n", tie->offset_x_m);
        fprintf(stderr, "dbg2    tie->offset_y_m:              %f\n", tie->offset_y_m);
        fprintf(stderr, "dbg2    tie->offset_z_m:              %f\n", tie->offset_z_m);
        fprintf(stderr, "dbg2    tie->sigmar1:                 %f\n", tie->sigmar1);
        fprintf(stderr, "dbg2    tie->sigmax1[0]:              %f\n", tie->sigmax1[0]);
        fprintf(stderr, "dbg2    tie->sigmax1[1]:              %f\n", tie->sigmax1[1]);
        fprintf(stderr, "dbg2    tie->sigmax1[2]:              %f\n", tie->sigmax1[2]);
        fprintf(stderr, "dbg2    tie->sigmar2:                 %f\n", tie->sigmar2);
        fprintf(stderr, "dbg2    tie->sigmax2[0]:              %f\n", tie->sigmax2[0]);
        fprintf(stderr, "dbg2    tie->sigmax2[1]:              %f\n", tie->sigmax2[1]);
        fprintf(stderr, "dbg2    tie->sigmax2[2]:              %f\n", tie->sigmax2[2]);
        fprintf(stderr, "dbg2    tie->sigmar3:                 %f\n", tie->sigmar3);
        fprintf(stderr, "dbg2    tie->sigmax3[0]:              %f\n", tie->sigmax3[0]);
        fprintf(stderr, "dbg2    tie->sigmax3[1]:              %f\n", tie->sigmax3[1]);
        fprintf(stderr, "dbg2    tie->sigmax3[2]:              %f\n", tie->sigmax3[2]);
        fprintf(stderr, "dbg2    tie->inversion_status:        %d\n", tie->inversion_status);
        fprintf(stderr, "dbg2    tie->inversion_offset_x:      %f\n", tie->inversion_offset_x);
        fprintf(stderr, "dbg2    tie->inversion_offset_y:      %f\n", tie->inversion_offset_y);
        fprintf(stderr, "dbg2    tie->inversion_offset_x_m:    %f\n", tie->inversion_offset_x_m);
        fprintf(stderr, "dbg2    tie->inversion_offset_y_m:    %f\n", tie->inversion_offset_y_m);
        fprintf(stderr, "dbg2    tie->inversion_offset_z_m:    %f\n", tie->inversion_offset_z_m);
        fprintf(stderr, "dbg2    tie->dx_m:                    %f\n", tie->dx_m);
        fprintf(stderr, "dbg2    tie->dy_m:                    %f\n", tie->dy_m);
        fprintf(stderr, "dbg2    tie->dz_m:                    %f\n", tie->dz_m);
        fprintf(stderr, "dbg2    tie->sigma_m:                 %f\n", tie->sigma_m);
        fprintf(stderr, "dbg2    tie->dr1_m:                   %f\n", tie->dr1_m);
        fprintf(stderr, "dbg2    tie->dr2_m:                   %f\n", tie->dr2_m);
        fprintf(stderr, "dbg2    tie->dr3_m:                   %f\n", tie->dr3_m);
        fprintf(stderr, "dbg2    tie->rsigma_m:                %f\n", tie->rsigma_m);
        fprintf(stderr, "dbg2    tie->block_1:                 %d\n", tie->block_1);
        fprintf(stderr, "dbg2    tie->block_1:                 %d\n", tie->block_1);
        fprintf(stderr, "dbg2    tie->isurveyplotindex:        %d\n", tie->isurveyplotindex);
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
  }

  /* deal with sections */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    /* retrieve crossing parameters */
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      /* add tie and set number */
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status != MBNA_CROSSING_STATUS_SET)
        project.num_globalties++;
      section->status = MBNA_CROSSING_STATUS_SET;

      /* set global tie parameters */
      globaltie->status = MBNA_TIE_XY;
      globaltie->snav = mbna_snav_2;
      globaltie->refgrid_id = project.refgrid_select;
      globaltie->snav_time_d = section->snav_time_d[globaltie->snav];
      mbna_snav_1 = -1;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      globaltie->offset_x = mbna_offset_x;
      globaltie->offset_y = mbna_offset_y;
      globaltie->offset_x_m = mbna_offset_x / mbna_mtodeglon;
      globaltie->offset_y_m = mbna_offset_y / mbna_mtodeglat;
      globaltie->offset_z_m = mbna_offset_z;
      globaltie->sigmar1 = mbna_minmisfit_sr1;
      globaltie->sigmar2 = mbna_minmisfit_sr2;
      globaltie->sigmar3 = mbna_minmisfit_sr3;
      for (int i = 0; i < 3; i++) {
        globaltie->sigmax1[i] = mbna_minmisfit_sx1[i];
        globaltie->sigmax2[i] = mbna_minmisfit_sx2[i];
        globaltie->sigmax3[i] = mbna_minmisfit_sx3[i];
      }
      if (globaltie->sigmar1 < MBNA_SMALL) {
        globaltie->sigmar1 = MBNA_SMALL;
        globaltie->sigmax1[0] = 1.0;
        globaltie->sigmax1[1] = 0.0;
        globaltie->sigmax1[2] = 0.0;
      }
      if (globaltie->sigmar2 < MBNA_SMALL) {
        globaltie->sigmar2 = MBNA_SMALL;
        globaltie->sigmax2[0] = 0.0;
        globaltie->sigmax2[1] = 1.0;
        globaltie->sigmax2[2] = 0.0;
      }
      if (globaltie->sigmar3 < MBNA_ZSMALL) {
        globaltie->sigmar3 = MBNA_ZSMALL;
        globaltie->sigmax3[0] = 0.0;
        globaltie->sigmax3[1] = 0.0;
        globaltie->sigmax3[2] = 1.0;
      }

      mbna_invert_offset_x = section->snav_lon_offset[mbna_snav_2];
      mbna_invert_offset_y = section->snav_lat_offset[mbna_snav_2];
      mbna_invert_offset_z = section->snav_z_offset[mbna_snav_2];
      globaltie->inversion_status = MBNA_INVERSION_NONE;
      globaltie->inversion_offset_x = mbna_invert_offset_x;
      globaltie->inversion_offset_y = mbna_invert_offset_y;
      globaltie->inversion_offset_x_m = mbna_invert_offset_x / mbna_mtodeglon;
      globaltie->inversion_offset_y_m = mbna_invert_offset_y / mbna_mtodeglat;
      globaltie->inversion_offset_z_m = mbna_invert_offset_z;
      if (project.inversion_status == MBNA_INVERSION_CURRENT)
        project.inversion_status = MBNA_INVERSION_OLD;

      /* set flag to update model plot */
      project.modelplot_uptodate = false;

      /* write updated project */
      project.save_count++;
      project.modelplot_uptodate = false;
      if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
        mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
        project.save_count = 0;
      }

      /* add info text */
      snprintf(message, sizeof(message), "Add Global Tie of file %d section %d\n > Nav point: %d:%d:%d\n > Offsets: %f %f %f m\n",
              mbna_current_file, mbna_current_section, mbna_current_file, mbna_current_section, globaltie->snav,
              globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);

      if (mbna_verbose >= 2) {
        fprintf(stderr, "\ndbg2  Global tie added in MBnavadjust function <%s>\n", __func__);
        fprintf(stderr, "dbg2    mbna_current_file:                  %d\n", mbna_current_file);
        fprintf(stderr, "dbg2    mbna_current_section:               %d\n", mbna_current_section);
        fprintf(stderr, "dbg2    section->status:                    %d\n", section->status);
        fprintf(stderr, "dbg2    globaltie->status:                  %d\n", globaltie->status);
        fprintf(stderr, "dbg2    globaltie->snav:                    %d\n", globaltie->snav);
        fprintf(stderr, "dbg2    globaltie->snav_time_d:             %f\n", globaltie->snav_time_d);
        fprintf(stderr, "dbg2    globaltie->offset_x:                %f\n", globaltie->offset_x);
        fprintf(stderr, "dbg2    globaltie->offset_y:                %f\n", globaltie->offset_y);
        fprintf(stderr, "dbg2    globaltie->offset_x_m:              %f\n", globaltie->offset_x_m);
        fprintf(stderr, "dbg2    globaltie->offset_y_m:              %f\n", globaltie->offset_y_m);
        fprintf(stderr, "dbg2    globaltie->offset_z_m:              %f\n", globaltie->offset_z_m);
        fprintf(stderr, "dbg2    globaltie->sigmar1:                 %f\n", globaltie->sigmar1);
        fprintf(stderr, "dbg2    globaltie->sigmax1[0]:              %f\n", globaltie->sigmax1[0]);
        fprintf(stderr, "dbg2    globaltie->sigmax1[1]:              %f\n", globaltie->sigmax1[1]);
        fprintf(stderr, "dbg2    globaltie->sigmax1[2]:              %f\n", globaltie->sigmax1[2]);
        fprintf(stderr, "dbg2    globaltie->sigmar2:                 %f\n", globaltie->sigmar2);
        fprintf(stderr, "dbg2    globaltie->sigmax2[0]:              %f\n", globaltie->sigmax2[0]);
        fprintf(stderr, "dbg2    globaltie->sigmax2[1]:              %f\n", globaltie->sigmax2[1]);
        fprintf(stderr, "dbg2    globaltie->sigmax2[2]:              %f\n", globaltie->sigmax2[2]);
        fprintf(stderr, "dbg2    globaltie->sigmar3:                 %f\n", globaltie->sigmar3);
        fprintf(stderr, "dbg2    globaltie->sigmax3[0]:              %f\n", globaltie->sigmax3[0]);
        fprintf(stderr, "dbg2    globaltie->sigmax3[1]:              %f\n", globaltie->sigmax3[1]);
        fprintf(stderr, "dbg2    globaltie->sigmax3[2]:              %f\n", globaltie->sigmax3[2]);
        fprintf(stderr, "dbg2    globaltie->inversion_status:        %d\n", globaltie->inversion_status);
        fprintf(stderr, "dbg2    globaltie->inversion_offset_x:      %f\n", globaltie->inversion_offset_x);
        fprintf(stderr, "dbg2    globaltie->inversion_offset_y:      %f\n", globaltie->inversion_offset_y);
        fprintf(stderr, "dbg2    globaltie->inversion_offset_x_m:    %f\n", globaltie->inversion_offset_x_m);
        fprintf(stderr, "dbg2    globaltie->inversion_offset_y_m:    %f\n", globaltie->inversion_offset_y_m);
        fprintf(stderr, "dbg2    globaltie->inversion_offset_z_m:    %f\n", globaltie->inversion_offset_z_m);
        fprintf(stderr, "dbg2    globaltie->dx_m:                    %f\n", globaltie->dx_m);
        fprintf(stderr, "dbg2    globaltie->dy_m:                    %f\n", globaltie->dy_m);
        fprintf(stderr, "dbg2    globaltie->dz_m:                    %f\n", globaltie->dz_m);
        fprintf(stderr, "dbg2    globaltie->sigma_m:                 %f\n", globaltie->sigma_m);
        fprintf(stderr, "dbg2    globaltie->dr1_m:                   %f\n", globaltie->dr1_m);
        fprintf(stderr, "dbg2    globaltie->dr2_m:                   %f\n", globaltie->dr2_m);
        fprintf(stderr, "dbg2    globaltie->dr3_m:                   %f\n", globaltie->dr3_m);
        fprintf(stderr, "dbg2    globaltie->rsigma_m:                %f\n", globaltie->rsigma_m);
      }
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
int mbnavadjust_naverr_deletetie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  /* deal with crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {

    /* get current crossing */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0
        && mbna_current_tie >= 0) {
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        /* delete the tie */
        mbnavadjust_deletetie(mbna_current_crossing, mbna_current_tie, MBNA_CROSSING_STATUS_SKIP);

        /* write updated project */
        project.save_count++;
        if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }
      }
    }

    /* set mbna_crossing_select */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0) {
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
  }

  /* deal with section */
  else if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status == MBNA_CROSSING_STATUS_SET) {
        /* delete global tie */
        section->status = MBNA_CROSSING_STATUS_SKIP;
        globaltie->status = MBNA_TIE_NONE;
        if (project.inversion_status == MBNA_INVERSION_CURRENT) {
          project.inversion_status = MBNA_INVERSION_OLD;
                    project.modelplot_uptodate = false;
        }

        /* set crossing unanalyzed */
        project.num_globalties--;
        project.num_globalties_analyzed--;

        /* write updated project */
        project.save_count++;
        if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }

        /* set flag to update model plot */
        project.modelplot_uptodate = false;

        /* add info text */
        snprintf(message, sizeof(message), "Unset file %d section %d\n", mbna_current_file, mbna_current_section);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
        do_info_add(message, true);
      }
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
int mbnavadjust_deletetie(int icrossing, int jtie, int delete_status) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       icrossing:     %d\n", icrossing);
    fprintf(stderr, "dbg2       jtie:          %d\n", jtie);
    fprintf(stderr, "dbg2       delete_status: %d\n", delete_status);
  }

  int status = MB_SUCCESS;

  /* work on current crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING
      && icrossing >= 0 && jtie >= 0) {
    /* retrieve crossing parameters */
    if (project.num_crossings > icrossing && project.crossings[icrossing].num_ties > jtie) {
      struct mbna_crossing *crossing = &project.crossings[icrossing];
      struct mbna_tie *tie = &crossing->ties[jtie];

      /* add info text */
      crossing = &project.crossings[icrossing];
      tie = &crossing->ties[jtie];
      if (delete_status == MBNA_CROSSING_STATUS_SKIP)
        snprintf(message, sizeof(message), "Delete Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
                jtie, icrossing, crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2,
                crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      else
        snprintf(message, sizeof(message), "Clear Tie Point %d of Crossing %d\n > Nav points: %d:%d:%d %d:%d:%d\n > Offsets: %f %f %f m\n",
                jtie, icrossing, crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2,
                crossing->section_2, tie->snav_2, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      do_info_add(message, true);

      /* reset tie counts for snavs */
      struct mbna_file *file1 = &project.files[crossing->file_id_1];
      struct mbna_section *section1 = &file1->sections[crossing->section_1];
      section1->snav_num_ties[tie->snav_1]--;
      struct mbna_file *file2 = &project.files[crossing->file_id_2];
      struct mbna_section *section2 = &file2->sections[crossing->section_2];
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


  /* deal with crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {

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
  }

  /* deal with section */
  else if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status == MBNA_CROSSING_STATUS_SET ) {
        mbna_current_tie = 0;
        mbna_snav_1 = 0;
        mbna_snav_1_time_d = 0.0;
        mbna_snav_2 = globaltie->snav;
        mbna_snav_2_time_d = globaltie->snav_time_d;
        mbna_invert_offset_x = section->snav_lon_offset[mbna_snav_2];
        mbna_invert_offset_y = section->snav_lat_offset[mbna_snav_2];
        mbna_invert_offset_z = section->snav_z_offset[mbna_snav_2];
        mbna_offset_x = globaltie->offset_x;
        mbna_offset_y = globaltie->offset_y;
        mbna_offset_z = globaltie->offset_z_m;
      } else if (project.inversion_status != MBNA_INVERSION_NONE) {
        mbna_current_tie = -1;
        mbna_snav_1 = 0;
        mbna_snav_1_time_d = 0.0;
        mbna_snav_2 = 0;
        mbna_snav_2_time_d = section->snav_time_d[mbna_snav_2];
        mbna_invert_offset_x = section->snav_lon_offset[mbna_snav_2];
        mbna_invert_offset_y = section->snav_lat_offset[mbna_snav_2];
        mbna_invert_offset_z = section->snav_z_offset[mbna_snav_2];
        mbna_offset_x = mbna_invert_offset_x;
        mbna_offset_y = mbna_invert_offset_y;
        mbna_offset_z = mbna_invert_offset_z;
      } else {
        mbna_current_tie = -1;
        mbna_snav_1 = 0;
        mbna_snav_1_time_d = 0.0;
        mbna_snav_2 = 0;
        mbna_snav_2_time_d = section->snav_time_d[mbna_snav_2];
        mbna_invert_offset_x = 0.0;
        mbna_invert_offset_y = 0.0;
        mbna_invert_offset_z = 0.0;
        mbna_offset_x = mbna_invert_offset_x;
        mbna_offset_y = mbna_invert_offset_y;
        mbna_offset_z = mbna_invert_offset_z;
      }
      mbna_tie_select = mbna_current_tie;
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
int mbnavadjust_naverr_checkoksettie() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  struct mbna_crossing *crossing;
  struct mbna_tie *tie;

  /* check for changed offsets */
  mbna_allow_set_tie = false;
  mbna_allow_add_tie = false;
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING
    && mbna_current_crossing >= 0) {
    crossing = &project.crossings[mbna_current_crossing];
    if (mbna_current_tie >= 0) {
      tie = &crossing->ties[mbna_current_tie];
      /* check for changed offset values */
      if (tie->snav_1 != mbna_snav_1 || tie->snav_2 != mbna_snav_2
          || tie->offset_x != mbna_offset_x || tie->offset_y != mbna_offset_y
          || tie->offset_z_m != mbna_offset_z) {
        mbna_allow_set_tie = true;
      }

      /* also check for unset sigma values */
      if (tie->sigmar1 == 100.0 && tie->sigmar2 == 100.0 && tie->sigmar3 == 100.0) {
        mbna_allow_set_tie = true;
      }
    }
    mbna_allow_add_tie = true;
  }

  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && mbna_current_file >= 0 && mbna_current_section >= 0) {
    struct mbna_file *file = &project.files[mbna_current_file];
    struct mbna_section *section = &file->sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section->globaltie;
    if (section->status == MBNA_CROSSING_STATUS_SET) {
      if (globaltie->snav != mbna_snav_2 || globaltie->offset_x != mbna_offset_x
            || globaltie->offset_y != mbna_offset_y || globaltie->offset_z_m != mbna_offset_z) {
              mbna_allow_set_tie = true;
      }

      /* also check for unset sigma values */
      else if (globaltie->sigmar1 == 100.0 && globaltie->sigmar2 == 100.0 && globaltie->sigmar3 == 100.0) {
        mbna_allow_set_tie = true;
      }
      mbna_allow_add_tie = false;
    }
    else {
      mbna_allow_add_tie = true;
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

  /* work on current crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    if (project.num_crossings > 0 && mbna_current_crossing >= 0) {
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
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
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }

        /* add info text */
        snprintf(message, sizeof(message), "Set crossing %d to be ignored\n", mbna_current_crossing);
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
  }

  /* work on current section */
  else if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status != MBNA_CROSSING_STATUS_SKIP) {
        if (section->status == MBNA_CROSSING_STATUS_NONE) {
          section->status = MBNA_CROSSING_STATUS_SKIP;
          project.num_globalties_analyzed--;
        }
        else if (section->status == MBNA_CROSSING_STATUS_SET) {
          /* delete global tie */
          section->status = MBNA_CROSSING_STATUS_SKIP;
          globaltie->status = MBNA_TIE_NONE;
          if (project.inversion_status == MBNA_INVERSION_CURRENT) {
            project.inversion_status = MBNA_INVERSION_OLD;
                      project.modelplot_uptodate = false;
          }
          project.num_globalties--;
          project.num_globalties_analyzed--;
        }

        /* write updated project */
        project.save_count++;
        if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }

        /* set flag to update model plot */
        project.modelplot_uptodate = false;

        /* add info text */
        snprintf(message, sizeof(message), "Set file %d section %d to be ignored\n", mbna_current_file, mbna_current_section);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
        do_info_add(message, true);
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
int mbnavadjust_naverr_unset() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* deal with crossing */
  if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {

    /* get current crossing */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0) {
      struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];
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
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }

        /* set flag to update model plot */
        project.modelplot_uptodate = false;

        /* add info text */
        snprintf(message, sizeof(message), "Unset crossing %d\n", mbna_current_crossing);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
        do_info_add(message, true);
      }
    }

    /* set mbna_crossing_select */
    if (project.num_crossings > 0 && mbna_current_crossing >= 0) {
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
  }

  /* deal with section */
  else if (project.open && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {
      struct mbna_file *file = (struct mbna_file *)&project.files[mbna_current_file];
      struct mbna_section *section = (struct mbna_section *)&file->sections[mbna_current_section];
      struct mbna_globaltie *globaltie = (struct mbna_globaltie *)&section->globaltie;
      if (section->status == MBNA_CROSSING_STATUS_SET) {
        /* delete global tie */
        section->status = MBNA_CROSSING_STATUS_NONE;
        globaltie->status = MBNA_TIE_NONE;
        if (project.inversion_status == MBNA_INVERSION_CURRENT) {
          project.inversion_status = MBNA_INVERSION_OLD;
                    project.modelplot_uptodate = false;
        }

        /* set crossing unanalyzed */
        project.num_globalties--;
        project.num_globalties_analyzed--;

        /* write updated project */
        project.save_count++;
        if (project.save_count < 0 || project.save_count >= mbna_save_frequency) {
          mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
          project.save_count = 0;
        }

        /* set flag to update model plot */
        project.modelplot_uptodate = false;

        /* add info text */
        snprintf(message, sizeof(message), "Unset file %d section %d\n", mbna_current_file, mbna_current_section);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
        do_info_add(message, true);
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
int mbnavadjust_crossing_load() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;
  struct mbna_tie *tie;
  struct mbna_file *file1, *file2;
  struct mbna_section *section1, *section2;

  /* unload loaded crossing or section */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    status = mbnavadjust_crossing_unload();
  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    status = mbnavadjust_referenceplussection_unload();
  }
  mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  /* load current crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && project.open &&
      project.num_crossings > 0 && mbna_current_crossing >= 0) {
    /* put up message */
    snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
// fprintf(stderr, "%s:%d:%s: section1  lonmin:%f lonmax:%f latmin:%f latmax:%f\n",
// __FILE__, __LINE__, __FUNCTION__, section1->lonmin, section1->lonmax, section1->latmin, section1->latmax);
// fprintf(stderr, "%s:%d:%s: section2  lonmin:%f lonmax:%f latmin:%f latmax:%f\n",
// __FILE__, __LINE__, __FUNCTION__, section2->lonmin, section2->lonmax, section2->latmin, section2->latmax);
// fprintf(stderr, "%s:%d:%s:  mbna_offset_x:%f mbna_offset_y:%f\n",
// __FILE__, __LINE__, __FUNCTION__, mbna_offset_x, mbna_offset_y);
// fprintf(stderr, "%s:%d:%s: mbna_plot_lon_min:%f mbna_plot_lon_max:%f mbna_plot_lat_min:%f mbna_plot_lat_max:%f\n",
// __FILE__, __LINE__, __FUNCTION__, mbna_plot_lon_min, mbna_plot_lon_max, mbna_plot_lat_min, mbna_plot_lat_max);
    mb_coor_scale(mbna_verbose, 0.5 * (mbna_lat_min + mbna_lat_max), &mbna_mtodeglon, &mbna_mtodeglat);

    /* load sections */
    snprintf(message, sizeof(message), "Loading section 1 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_1, mbna_section_1,
                                          (void **)&swathraw1, (void **)&swath1, &error);
    snprintf(message, sizeof(message), "Loading section 2 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_2, mbna_section_2,
                                          (void **)&swathraw2, (void **)&swath2, &error);

    /* get lon lat positions for soundings */
    snprintf(message, sizeof(message), "Transforming section 1 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    snprintf(message, sizeof(message), "Transforming section 2 of crossing %d...", mbna_current_crossing);
    do_message_update(message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      snprintf(message, sizeof(message), "Contouring section 1 of crossing %d...", mbna_current_crossing);
      do_message_update(message);
      mbna_contour = &mbna_contour1;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      snprintf(message, sizeof(message), "Contouring section 2 of crossing %d...", mbna_current_crossing);
      do_message_update(message);
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
    }

    /* set loaded flag */
    mbna_naverr_mode = MBNA_NAVERR_MODE_CROSSING;

    /* generate misfit grids */
    snprintf(message, sizeof(message), "Getting misfit for crossing %d...", mbna_current_crossing);
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
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
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
    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;
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

    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    status = mbnavadjust_referenceplussection_unload();
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
int mbnavadjust_naverr_replot() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
    mbnavadjust_crossing_replot();
  else
    mbnavadjust_referencesection_replot();

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
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
    /* get lon lat positions for soundings */
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_1, swathraw1, swath1, 0.0, &error);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      mbna_contour = &mbna_contour1;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
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
int mbnavadjust_referencesection_replot() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* replot loaded crossing */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK)
    && mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    /* get lon lat positions for soundings */
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
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
int mbnavadjust_referenceplussection_load() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* unload loaded crossing or section */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    status = mbnavadjust_crossing_unload();
  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    status = mbnavadjust_referenceplussection_unload();
  }
  mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  /* load current section */
  if ((mbna_status == MBNA_STATUS_NAVERR || mbna_status == MBNA_STATUS_AUTOPICK) && project.open &&
      project.num_files > 0 && mbna_current_file >= 0 && mbna_current_section >= 0) {

    /* first load the swath section, then extract bathymetry from the reference
        grid in a region around the section, place that reference bathymetry into
        the same swath section structure, and enable the cross correlation of the
        section relative to the reference. */

    /* put up message */
    snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_update(message);

    struct mbna_section *section2 = &project.files[mbna_current_file].sections[mbna_current_section];
    struct mbna_globaltie *globaltie = &section2->globaltie;
    mb_coor_scale(mbna_verbose, 0.5 * (section2->latmin + section2->latmax), &mbna_mtodeglon, &mbna_mtodeglat);

    mbna_file_id_2 = mbna_current_file;
    mbna_section_2 = mbna_current_section;
    int refgrid_id = project.refgrid_select;

    if (section2->status == MBNA_CROSSING_STATUS_SET ) {
      mbna_current_tie = 0;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = globaltie->snav;
      mbna_snav_2_time_d = globaltie->snav_time_d;
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2];
      mbna_offset_x = globaltie->offset_x;
      mbna_offset_y = globaltie->offset_y;
      mbna_offset_z = globaltie->offset_z_m;
      refgrid_id = globaltie->refgrid_id;
    } else if (project.inversion_status != MBNA_INVERSION_NONE) {
      mbna_current_tie = -1;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = section2->num_snav / 2;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_invert_offset_x = section2->snav_lon_offset[mbna_snav_2];
      mbna_invert_offset_y = section2->snav_lat_offset[mbna_snav_2];
      mbna_invert_offset_z = section2->snav_z_offset[mbna_snav_2];
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    } else {
      mbna_current_tie = -1;
      mbna_snav_1 = 0;
      mbna_snav_1_time_d = 0.0;
      mbna_snav_2 = section2->num_snav / 2;
      mbna_snav_2_time_d = section2->snav_time_d[mbna_snav_2];
      mbna_invert_offset_x = 0.0;
      mbna_invert_offset_y = 0.0;
      mbna_invert_offset_z = 0.0;
      mbna_offset_x = mbna_invert_offset_x;
      mbna_offset_y = mbna_invert_offset_y;
      mbna_offset_z = mbna_invert_offset_z;
    }

    /* if globaltie not set get misfit z-offset center value using average of
       set globalties for this survey and this reference grid */
    if (section2->status != MBNA_CROSSING_STATUS_SET) {
      double sumz = 0.0;
      int numz = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        struct mbna_file *file = &project.files[ifile];
        if (file->block == project.files[mbna_file_id_2].block) {
          for (int isection = 0; isection < file->num_sections; isection++) {
            struct mbna_section *section = &file->sections[isection];
            if (section->status == MBNA_CROSSING_STATUS_SET) {
              sumz += section->globaltie.offset_z_m;
              numz++;
            }
          }
        }
      }
      if (numz > 0) {
        mbna_offset_z = sumz / numz;
      }
    }

    /* reset survey file and section selections */
    mbna_section_select = mbna_current_section;
    mbna_file_select = mbna_current_file;
    mbna_survey_select = project.files[mbna_current_file].block;

    /* load section first because it defines the bounds of the reference bathymetry */
    snprintf(message, sizeof(message), "Loading file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_update(message);
    fprintf(stderr, "\n%s\n", message);
    status = mbnavadjust_section_load(mbna_verbose, &project, mbna_file_id_2, mbna_section_2,
                                          (void **)&swathraw2, (void **)&swath2, &error);
    snprintf(message, sizeof(message), "Transforming file %d section %d...", mbna_current_file, mbna_current_section);
    do_message_update(message);
    status = mbnavadjust_section_translate(mbna_verbose, &project, mbna_file_id_2, swathraw2, swath2, mbna_offset_z, &error);

    mbna_lon_min = section2->lonmin + mbna_offset_x;
    mbna_lon_max = section2->lonmax + mbna_offset_x;
    mbna_lat_min = section2->latmin + mbna_offset_y;
    mbna_lat_max = section2->latmax + mbna_offset_y;

    /* calculate the desired area for the reference grid subset to be loaded */
    double length_meters = MAX((section2->lonmax - section2->lonmin) / mbna_mtodeglon,
                                (section2->latmax - section2->latmin) / mbna_mtodeglat);
    double lon_size_deg = length_meters * mbna_mtodeglon;
    double lat_size_deg = length_meters * mbna_mtodeglat;
    project.reference_section.lonmin = mbna_lon_min - 2.0 * lon_size_deg;
    project.reference_section.lonmax = mbna_lon_max + 2.0 * lon_size_deg;
    project.reference_section.latmin = mbna_lat_min - 2.0 * lat_size_deg;
    project.reference_section.latmax = mbna_lat_max + 2.0 * lat_size_deg;

    /* load the specified reference grid if overlaps the desired area */
    if (!(project.refgrid_bounds[1][refgrid_id] < project.reference_section.lonmin
        || project.refgrid_bounds[0][refgrid_id] > project.reference_section.lonmax
        || project.refgrid_bounds[3][refgrid_id] < project.reference_section.latmin
        || project.refgrid_bounds[2][refgrid_id] > project.reference_section.latmax)) {
      snprintf(message, sizeof(message), "Reading reference grid: %s/%s\n", project.datadir, project.refgrid_names[refgrid_id]);
      do_message_update(message);
      int refgrid_status = mbnavadjust_reference_load(mbna_verbose, &project, refgrid_id,
                                &project.reference_section, (void **)&swath1, &error);
      if (refgrid_status == MB_SUCCESS) {
        project.refgrid_status = MBNA_REFGRID_LOADED;
        project.refgrid_select = refgrid_id;
        snprintf(message, sizeof(message), "Read reference grid: %s/%s",
                          project.datadir, project.refgrid_names[refgrid_id]);
        do_message_update(message);
        snprintf(message, sizeof(message), "Read reference grid: %s/%s \n\t Dimensions: %d %d\n\tBounds: %f %f   %f %f\n",
                          project.datadir, project.refgrid_names[refgrid_id],
                          project.refgrid.nx, project.refgrid.ny,
                          project.refgrid.bounds[0], project.refgrid.bounds[1],
                          project.refgrid.bounds[2], project.refgrid.bounds[3]);
        fprintf(stderr, "%s\n", message);
        mbna_lon_min = MIN(project.reference_section.lonmin, section2->lonmin + mbna_offset_x);
        mbna_lon_max = MAX(project.reference_section.lonmax, section2->lonmax + mbna_offset_x);
        mbna_lat_min = MIN(project.reference_section.latmin, section2->latmin + mbna_offset_y);
        mbna_lat_max = MAX(project.reference_section.latmax, section2->latmax + mbna_offset_y);
      } else {
        snprintf(message, sizeof(message), "Failed to read reference grid: %s/%s",
                          project.datadir, project.refgrid_names[refgrid_id]);
        do_message_update(message);
      }
    }

    mbna_plot_lon_min = mbna_lon_min;
    mbna_plot_lon_max = mbna_lon_max;
    mbna_plot_lat_min = mbna_lat_min;
    mbna_plot_lat_max = mbna_lat_max;
fprintf(stderr, "%s:%d:%s: mbna_plot_lon_min:%f mbna_plot_lon_max:%f mbna_plot_lat_min:%f mbna_plot_lat_max:%f\n",
__FILE__, __LINE__, __FUNCTION__, mbna_plot_lon_min, mbna_plot_lon_max, mbna_plot_lat_min, mbna_plot_lat_max);

    /* generate contour data */
    if (mbna_status != MBNA_STATUS_AUTOPICK) {
      if (project.refgrid_status == MBNA_REFGRID_LOADED) {
        snprintf(message, sizeof(message), "Contouring reference with bounds %f %f %f %f...",
                          project.reference_section.lonmin, project.reference_section.lonmax,
                          project.reference_section.latmin, project.reference_section.latmax);
        do_message_update(message);
        fprintf(stderr, "%s\n", message);
        mbna_contour = &mbna_contour1;
        status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_1, mbna_section_1, swath1, &mbna_contour1, &error);
      }
      snprintf(message, sizeof(message), "Contouring file %d section %d...", mbna_current_file, mbna_current_section);
      do_message_update(message);
      fprintf(stderr, "%s\n", message);
      mbna_contour = &mbna_contour2;
      status = mbnavadjust_section_contour(mbna_verbose, &project, mbna_file_id_2, mbna_section_2, swath2, &mbna_contour2, &error);
    }

    /* set loaded flag */
    mbna_naverr_mode = MBNA_NAVERR_MODE_SECTION;

    /* generate misfit grids */
    if (project.refgrid_status == MBNA_REFGRID_LOADED) {
      snprintf(message, sizeof(message), "Getting misfit for file %d section %d...", mbna_current_file, mbna_current_section);
      do_message_update(message);
      fprintf(stderr, "%s\n", message);
      status = mbnavadjust_get_misfit();
    }

    /* set flag to update model plot */
    project.modelplot_uptodate = false;
  }

  /* reset survey file and section selections */
  mbna_section_select = mbna_current_section;
  mbna_file_select = mbna_current_file;
  mbna_survey_select = project.files[mbna_current_file].block;

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
int mbnavadjust_referencegrid_unload() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  if (project.refgrid_status == MBNA_REFGRID_LOADED) {
    status = mbnavadjust_refgrid_unload(mbna_verbose, &project, &error);
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
int mbnavadjust_referenceplussection_unload() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;

  /* unload loaded section */
  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    if (swath1 != NULL)
      status = mbnavadjust_reference_unload(mbna_verbose, (void **)&swath1, &error);
    if (swathraw2 != NULL && swath2 != NULL)
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
    project.refgrid_status = MBNA_REFGRID_UNLOADED;
    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;
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

    mbna_naverr_mode = MBNA_NAVERR_MODE_UNLOADED;

  }
  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
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
int mbnavadjust_naverr_snavpoints(int ix, int iy) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       ix:           %d\n", ix);
    fprintf(stderr, "dbg2       iy:           %d\n", iy);
  }

  int status = MB_SUCCESS;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {
    /* get position in lon and lat */
    double x = ix / mbna_plotx_scale + mbna_plot_lon_min;
    double y = (cont_borders[3] - iy) / mbna_ploty_scale + mbna_plot_lat_min;
    struct mbna_crossing *crossing = &project.crossings[mbna_current_crossing];

    /* get closest snav point in swath 1 */
    struct mbna_section *section = &project.files[crossing->file_id_1].sections[crossing->section_1];
    double distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      double dx = (section->snav_lon[i] - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
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
      double dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_2 = i;
        mbna_snav_2_time_d = section->snav_time_d[i];
        mbna_snav_2_lon = section->snav_lon[i];
        mbna_snav_2_lat = section->snav_lat[i];
      }
    }

    if (mbna_verbose >= 2) {
      fprintf(stderr, "\ndbg2  snav point selection in MBnavadjust function <%s>\n", __func__);
      fprintf(stderr, "dbg2  mbna_naverr_mode:        %d\n", mbna_naverr_mode);
      fprintf(stderr, "dbg2  mbna_current_crossing:   %d\n", mbna_current_crossing);
      if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
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
  }

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {
    /* get position in lon and lat */
    double x = ix / mbna_plotx_scale + mbna_plot_lon_min;
    double y = (cont_borders[3] - iy) / mbna_ploty_scale + mbna_plot_lat_min;

    /* get closest snav point in swath 2 */
    struct mbna_section *section = &project.files[mbna_current_file].sections[mbna_current_section];
    mb_coor_scale(mbna_verbose, 0.5 * (section->latmin + section->latmax), &mbna_mtodeglon, &mbna_mtodeglat);
    double distance = 999999.999;
    for (int i = 0; i < section->num_snav; i++) {
      double dx = (section->snav_lon[i] + mbna_offset_x - x) / mbna_mtodeglon;
      double dy = (section->snav_lat[i] + mbna_offset_y - y) / mbna_mtodeglat;
      double d = sqrt(dx * dx + dy * dy);
      if (d < distance) {
        distance = d;
        mbna_snav_2 = i;
        mbna_snav_2_time_d = section->snav_time_d[i];
        mbna_snav_2_lon = section->snav_lon[i];
        mbna_snav_2_lat = section->snav_lat[i];
      }
    }

    if (mbna_verbose >= 2) {
      fprintf(stderr, "\ndbg2  snav point selection in MBnavadjust function <%s>\n", __func__);
      fprintf(stderr, "dbg2  mbna_naverr_mode:        %d\n", mbna_naverr_mode);
      fprintf(stderr, "dbg2  mbna_current_file:       %d\n", mbna_current_file);
      fprintf(stderr, "dbg2  mbna_current_section:    %d\n", mbna_current_section);
      if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
        fprintf(stderr, "dbg2  snav values:\n");
        fprintf(stderr, "dbg2       mbna_snav_2:        %d\n", mbna_snav_2);
        fprintf(stderr, "dbg2       mbna_snav_2_time_d: %f\n", mbna_snav_2_time_d);
        fprintf(stderr, "dbg2       mbna_snav_2_lon:    %.10f\n", mbna_snav_2_lon);
        fprintf(stderr, "dbg2       mbna_snav_2_lat:    %.10f\n", mbna_snav_2_lat);
        fprintf(stderr, "dbg2       section->num_snav:  %d\n", section->num_snav);
        for (int i = 0; i < section->num_snav; i++) {
          fprintf(stderr, "dbg2       section->snav_time_d[%d]: %f\n", i, section->snav_time_d[i]);
          fprintf(stderr, "dbg2       section->snav_lon[%d]:    %.10f\n", i, section->snav_lon[i]);
          fprintf(stderr, "dbg2       section->snav_lat[%d]:    %.10f\n", i, section->snav_lat[i]);
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

  if (project.open
      && ((mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && project.num_crossings > 0 && mbna_current_crossing >= 0)
          || (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && project.refgrid_status == MBNA_REFGRID_LOADED))) {

    /* set message on */
    if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING)
      snprintf(message, sizeof(message), "Making misfit grid for crossing %d", mbna_current_crossing);
    else
      snprintf(message, sizeof(message), "Making misfit grid for file %d section %d vs reference bathymetry",
              mbna_file_select, mbna_section_select);
    do_message_update(message);
    if (mbna_verbose > 0)
      fprintf(stderr, "%s\n", message);

    /* reset sounding density threshold for misfit calculation
        - will be tuned down if necessary */
    mbna_minmisfit_nthreshold = MBNA_MISFIT_NTHRESHOLD;

    /* figure out lateral extent of grids */
    grid_nx = MBNA_MISFIT_DIMXY;
    grid_ny = MBNA_MISFIT_DIMXY;
    if ((mbna_plot_lon_max - mbna_plot_lon_min) / mbna_mtodeglon > (mbna_plot_lat_max - mbna_plot_lat_min) / mbna_mtodeglat) {
      grid_dx = (mbna_plot_lon_max - mbna_plot_lon_min) / (grid_nx - 1);
      grid_dy = grid_dx * mbna_mtodeglat / mbna_mtodeglon;
//fprintf(stderr,"DEBUG %s %d: grid scale: grid_dx:%f grid_dy:%f\n",
//__FILE__,__LINE__,
//grid_dx,grid_dy);
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
//fprintf(stderr,"DEBUG %s %d: grid_olon:%.10f grid_olat:%.10f\n",
//__FILE__,__LINE__,
//grid_olon,grid_olat);

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
//fprintf(stderr,"DEBUG %s %d: GRID parameters: dx:%.10f dy:%.10f nx:%d ny:%d  bounds:  grid: %.10f %.10f %.10f %.10f  plot: %.10f %.10f %.10f %.10f\n",
//__FILE__,__LINE__,
//grid_dx, grid_dy, grid_nx, grid_ny,
//grid_olon, grid_olon + grid_nx * grid_dx,
//grid_olat, grid_olat + grid_ny * grid_dy,
//mbna_lon_min, mbna_lon_max, mbna_lat_min, mbna_lat_max);

    /* figure out range of z offsets */
    zmin = mbna_misfit_offset_z - 0.5 * project.zoffsetwidth;
    zmax = mbna_misfit_offset_z + 0.5 * project.zoffsetwidth;
    zoff_dz = project.zoffsetwidth / (nzmisfitcalc - 1);
//fprintf(stderr,"DEBUG %s:%d:%s: mbna_misfit_offset_z:%f project.zoffsetwidth:%f nzmisfitcalc:%d zmin:%f zmax:%f zoff_dz:%f\n",
//__FILE__,__LINE__, __FUNCTION__, 
//mbna_misfit_offset_z,project.zoffsetwidth,nzmisfitcalc,zmin,zmax,zoff_dz);

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
//else
//fprintf(stderr,"DEBUG %s %d: BAD swath1: %d %d  %.10f %.10f  %f %f  %d %d\n",
//__FILE__,__LINE__,
//i, j, swath1->pings[i].bathlon[j], swath1->pings[i].bathlat[j], x, y, igx, igy);
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
//else
//fprintf(stderr,"DEBUG %s %d: BAD swath2: %d %d  %.10f %.10f  %f %f  %d %d\n",
//__FILE__,__LINE__,
//i, j, swath2->pings[i].bathlon[j], swath2->pings[i].bathlat[j], x, y, igx, igy);
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
//int imin;
//int jmin;
//int kmin;
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
//imin = ic;
//jmin = jc;
//kmin = kc;
              found = true;
//zoff = zmin + zoff_dz * kc;
//fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d kc:%d misfit:%f %f %d  pos:%f %f %f zoff:%f mbna_ofset_z:%f\n",
//__FILE__,__LINE__,
//ic,jc,kc,misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z,
//zoff,mbna_offset_z);
            }
          }
//if (ic == jc && kc == 0)
//fprintf(stderr,"DEBUG %s %d: ic:%d jc:%d misfit:%d %f\n",
//__FILE__,__LINE__,
//ic,jc,gridnm[lc],gridm[lc]);
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
//imin = ic;
//jmin = jc;
//kmin = kc;
              found = true;
            }
//fprintf(stderr,"DEBUG %s %d: ijk:%d %d %d gridm:%d %f  misfit:%f %f %d  pos:%f %f %f\n",
//__FILE__,__LINE__,
//ic,jc,kc,gridnm[lc],gridm[lc],misfit_min,mbna_minmisfit,mbna_minmisfit_n,mbna_minmisfit_x,mbna_minmisfit_y,mbna_minmisfit_z);
          }
    }
    misfit_min = 0.99 * misfit_min;
    misfit_max = 1.01 * misfit_max;
//if (found)
//{
//lc = kmin + nzmisfitcalc * (imin + jmin * gridm_nx);
//fprintf(stderr,"DEBUG %s %d: min misfit: i:%d j:%d k:%d    n:%d m:%f   offsets: %f %f %f\n",
//__FILE__,__LINE__,
//imin, jmin, kmin, gridnm[lc], gridm[lc],
//mbna_minmisfit_x / mbna_mtodeglon,
//mbna_minmisfit_y / mbna_mtodeglat,
//mbna_minmisfit_z);
//}

//fprintf(stderr,"DEBUG %s %d: Misfit bounds: nmin:%d best:%f min:%f max:%f min loc: %f %f %f\n",
//__FILE__,__LINE__,
//mbna_minmisfit_n,mbna_minmisfit,misfit_min,misfit_max,
//mbna_minmisfit_x/mbna_mtodeglon,mbna_minmisfit_y/mbna_mtodeglat,mbna_minmisfit_z);

    /* set message on */
    if (mbna_verbose > 1)
      fprintf(stderr, "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
    snprintf(message, sizeof(message), "Histogram equalizing misfit grid for crossing %d\n", mbna_current_crossing);
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
      snprintf(message, sizeof(message), "Estimating 3D uncertainty for crossing %d\n", mbna_current_crossing);
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

  if (project.open
      && ((mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING && project.num_crossings > 0 && mbna_current_crossing >= 0)
          || (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION && project.refgrid_status == MBNA_REFGRID_LOADED))) {
    /* get minimum misfit in plane at current z offset */
// fprintf(stderr,"DEBUG %s:%d:%s: mbna_misfit_offset_z:%f project.zoffsetwidth:%f nzmisfitcalc:%d zmin:%f zmax:%f zoff_dz:%f\n",
// __FILE__,__LINE__, __FUNCTION__, 
// mbna_misfit_offset_z,project.zoffsetwidth,nzmisfitcalc,zmin,zmax,zoff_dz);
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
      //fprintf(stderr,"mbnavadjust_get_misfitxy a mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
      //mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh);
    }
    //fprintf(stderr,"mbnavadjust_get_misfitxy b mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
    //mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh);
  }
  //fprintf(stderr,"mbnavadjust_get_misfitxy c mbna_minmisfit_xh:%f mbna_minmisfit_yh:%f mbna_minmisfit_zh:%f\n",
  //mbna_minmisfit_xh,mbna_minmisfit_yh,mbna_minmisfit_zh);

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

  if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
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
  struct mbna_plot_vector *v = NULL;
  struct mbna_crossing *crossing = NULL;
  struct mbna_file *file1 = NULL;
  struct mbna_file *file2 = NULL;
  struct mbna_section *section1 = NULL;
  struct mbna_section *section2 = NULL;
  struct mbna_tie *tie = NULL;
  struct mbna_globaltie *globaltie = NULL;
  int ix, iy, ix1, ix2, iy1, iy2, idx, idy;
  int boxoff, boxwid;
  static int ixo, iyo;
  static int izx1, izy1, izx2, izy2;
  static int pixel, ipixel;
  int snav_1, snav_2;
  int fill;
  bool found;
  int i, j, k, l;

  if (mbna_naverr_mode == MBNA_NAVERR_MODE_CROSSING) {

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
    if (gridm_nx > 0 && gridm_ny > 0) {
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
    if (gridm_nx > 0 && gridm_ny > 0) {
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
    }

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

  else if (mbna_naverr_mode == MBNA_NAVERR_MODE_SECTION) {

    /* get structures */
    section1 = (struct mbna_section *)&project.reference_section;
    file2 = (struct mbna_file *)&project.files[mbna_current_file];
    section2 = (struct mbna_section *)&file2->sections[mbna_current_section];
    globaltie = (struct mbna_globaltie *)&section2->globaltie;

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

      /* replot tie point */
      if (globaltie->status != MBNA_TIE_NONE) {
        boxoff = 6;
        boxwid = 13;
        ix = (int)(mbna_plotx_scale * (section2->snav_lon[globaltie->snav] - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[globaltie->snav] - mbna_plot_lat_min));
        xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                         XG_SOLIDLINE);
        xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                         XG_SOLIDLINE);
        ixo = ix;
        iyo = iy;
        ix = (int)(mbna_plotx_scale * (section2->snav_lon[globaltie->snav] + mbna_offset_x_old - mbna_plot_lon_min));
        iy = (int)(cont_borders[3] -
                   mbna_ploty_scale * (section2->snav_lat[globaltie->snav] + mbna_offset_y_old - mbna_plot_lat_min));
        xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                         XG_SOLIDLINE);
        xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_background],
                         XG_SOLIDLINE);
        xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_background], XG_SOLIDLINE);
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
    if (project.refgrid_status == MBNA_REFGRID_LOADED) {
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

    /* plot tie point */
    mbnavadjust_naverr_checkoksettie();
    if (globaltie->status != MBNA_TIE_NONE) {
      boxoff = 6;
      boxwid = 13;
      if (mbna_allow_set_tie)
        fill = pixel_values[RED];
      else
        fill = pixel_values[6];
      ix = (int)(mbna_plotx_scale * (section2->snav_lon[mbna_snav_2] - mbna_plot_lon_min));
      iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[mbna_snav_2] - mbna_plot_lat_min));
      xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
      xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground],
                       XG_SOLIDLINE);
      ixo = ix;
      iyo = iy;
      ix = (int)(mbna_plotx_scale * (section2->snav_lon[mbna_snav_2] + mbna_offset_x - mbna_plot_lon_min));
      iy = (int)(cont_borders[3] - mbna_ploty_scale * (section2->snav_lat[mbna_snav_2] + mbna_offset_y - mbna_plot_lat_min));
      xg_fillrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, fill, XG_SOLIDLINE);
      xg_drawrectangle(pcont_xgid, ix - boxoff, iy - boxoff, boxwid, boxwid, pixel_values[mbna_color_foreground],
                       XG_SOLIDLINE);
      xg_drawline(pcont_xgid, ixo, iyo, ix, iy, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
    }

    /* plot overlap box */
    mbnavadjust_section_overlapbounds(mbna_verbose, &project, mbna_current_file, mbna_current_section,
                                      mbna_offset_x, mbna_offset_y,
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
    if (project.refgrid_status == MBNA_REFGRID_LOADED) {
      if (gridm_nx > 0 && gridm_ny > 0) {
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
      if (gridm_nx > 0 && gridm_ny > 0) {
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
      }

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
  // sufficiently long (track length >=0.25 * project.section_length)
  if (project.open && project.num_crossings > 0) {
    /* set message dialog on */
    snprintf(message, sizeof(message), "Autopicking offsets...");
    do_message_on(message);
    snprintf(message, sizeof(message), "Autopicking offsets...\n");
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
      // current desired section length for the project) - this excludes trying to
      // match short sections at the end of a file */
      if (process) {
        file1 = &project.files[crossing->file_id_1];
        section1 = &file1->sections[crossing->section_1];
        file2 = &project.files[crossing->file_id_2];
        section2 = &file2->sections[crossing->section_2];
        if (section1->distance < 0.25 * project.section_length
            || section2->distance < 0.25 * project.section_length) {
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
        snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
// fprintf(stderr, "%s:%d:%s: mbna_plot_lon_min:%f mbna_plot_lon_max:%f mbna_plot_lat_min:%f mbna_plot_lat_max:%f\n",
// __FILE__, __LINE__, __FUNCTION__, mbna_plot_lon_min, mbna_plot_lon_max, mbna_plot_lat_min, mbna_plot_lat_max);

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
// fprintf(stderr, "%s:%d:%s: mbna_plot_lon_min:%f mbna_plot_lon_max:%f mbna_plot_lat_min:%f mbna_plot_lat_max:%f\n",
// __FILE__, __LINE__, __FUNCTION__, mbna_plot_lon_min, mbna_plot_lon_max, mbna_plot_lat_min, mbna_plot_lat_max);

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
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
  struct mbna_matrix matrix = { 0, 0, 0, NULL, NULL, NULL };
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
    snprintf(message, sizeof(message), "Setting up navigation inversion...");
    do_message_on(message);

    /*----------------------------------------------------------------*/
    /* Initialize arrays, solution, perturbation                      */
    /*----------------------------------------------------------------*/

    /* count number of nav points, discontinuities, and blocks */
    nnav = 0;
    nblock = 0;
    nsmooth = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
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
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XYZ)
            nglobal += 3;
          else if (section->globaltie.status == MBNA_TIE_XY)
            nglobal += 2;
          else if (section->globaltie.status == MBNA_TIE_Z)
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
          if (section->globaltie.status != MBNA_TIE_NONE) {
            if (section->globaltie.status == MBNA_TIE_XYZ || section->globaltie.status == MBNA_TIE_XY) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->globaltie.offset_x_m;
              byfixstatus[file->block]++;
              byfix[file->block] += section->globaltie.offset_y_m;
            }
            if (section->globaltie.status == MBNA_TIE_XYZ || section->globaltie.status == MBNA_TIE_Z) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->globaltie.offset_z_m;
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
      atol = 5.0e-7;   // releative precision of A matrix
      btol = 5.0e-7;   // relative precision of data array
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
        snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
fprintf(stderr, "%s:%d:%s: mbna_plot_lon_min:%f mbna_plot_lon_max:%f mbna_plot_lat_min:%f mbna_plot_lat_max:%f\n",
__FILE__, __LINE__, __FUNCTION__, mbna_plot_lon_min, mbna_plot_lon_max, mbna_plot_lat_min, mbna_plot_lat_max);
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
        }
        else {
          snprintf(message, sizeof(message), "Failed to reset Tie Point %d of Crossing %d\n", 0, mbna_current_crossing);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
          do_info_add(message, true);
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

      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
    snprintf(message, sizeof(message), "Zeroing all z offsets in list...");
    do_message_on(message);
    snprintf(message, sizeof(message), "Zeroing all z offsets in list.\n");
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
        else if (mbna_view_list == MBNA_VIEW_LIST_TIES
                  || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDALL
                  || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDWORST
                  || mbna_view_list == MBNA_VIEW_LIST_TIESSORTEDBAD) {
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
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
int mbnavadjust_unsetskipped() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_crossing *crossing;

  /* loop over all crossings */
  if (project.open && project.num_crossings > 0) {
    /* set message dialog on */
    snprintf(message, sizeof(message), "Unsetting all skipped crossings in list...");
    do_message_on(message);
    snprintf(message, sizeof(message), "Unsetting all skipped crossings in list.\n");
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
    do_info_add(message, true);

    /* loop over all crossings */
    for (int i = 0; i < project.num_crossings; i++) {
      /* get structure */
      crossing = &(project.crossings[i]);

      /* any crossing that is skipped and in the current list will be changed to unset */
      if (crossing->status == MBNA_CROSSING_STATUS_SKIP) {
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
        else {
          process = false;
        }

        /* if process true and crossing skipped change to unset */
        if (process) {
          crossing->status = MBNA_CROSSING_STATUS_NONE;
        }
      }
    }

    /* write updated project */
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
  struct mbna_matrix matrix = { 0, 0, 0, NULL, NULL, NULL };
  bool *x_continuity = NULL;
  int *x_quality = NULL;
  int *x_num_ties = NULL;
  int *x_chunk = NULL;
  double *x_time_d = NULL;
  int *chunk_center = NULL;
  bool *chunk_continuity = NULL;
  int *global_ties_xy_files = NULL;
  int *global_ties_xy_sections = NULL;
  int *global_ties_z_files = NULL;
  int *global_ties_z_sections = NULL;
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
  int nglobaltiexy = 0;
  int nglobaltiez = 0;
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

  int nchunk, nchunk_start;
  double distance_sum, chunk_distance;
  double damping;

  int n_iteration;
  double convergence;
  double convergence_prior;
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
                    "PROBLEM WITH CROSSING TIE: %4d %2d %2.2d:%3.3d:%3.3d:%2.2d %2.2d:%3.3d:%3.3d:%2.2d %8.2f %8.2f %8.2f | "
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
    snprintf(message, sizeof(message), "Setting up navigation inversion...");
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

    /* zero the fixed tie structures for all file sections */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        section->fixedtie.status = MBNA_TIE_NONE;
      }
    }

    /* loop over all crossings looking for those in which one of the files
       involved is fixed and the other isn't - copy these to the fixedtie structure
       of the section structure. */
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        file1 = &project.files[crossing->file_id_1];
        file2 = &project.files[crossing->file_id_2];
        if ((file1->status == MBNA_FILE_FIXEDNAV || file1->status == MBNA_FILE_FIXEDXYNAV || file1->status == MBNA_FILE_FIXEDZNAV)
            && (file2->status == MBNA_FILE_POORNAV || file2->status == MBNA_FILE_GOODNAV)) {
          section2 = &file2->sections[crossing->section_2];
          if (file1->status == MBNA_FILE_FIXEDNAV)
            section2->fixedtie.status = MBNA_TIE_XYZ_FIXED;
          else if (file1->status == MBNA_FILE_FIXEDXYNAV)
            section2->fixedtie.status = MBNA_TIE_XY_FIXED;
          else if (file1->status == MBNA_FILE_FIXEDZNAV)
            section2->fixedtie.status = MBNA_TIE_Z_FIXED;
          else
            section2->fixedtie.status = MBNA_TIE_NONE;
          tie = &crossing->ties[0];
          section2->fixedtie.snav = tie->snav_2;
          section2->fixedtie.refgrid_id = 0;
          section2->fixedtie.snav_time_d = tie->snav_2_time_d;
          section2->fixedtie.offset_x = tie->offset_x;
          section2->fixedtie.offset_y = tie->offset_y;
          section2->fixedtie.offset_x_m = tie->offset_x_m;
          section2->fixedtie.offset_y_m = tie->offset_y_m;
          section2->fixedtie.offset_z_m = tie->offset_z_m;
          section2->fixedtie.sigmar1 = tie->sigmar1;
          section2->fixedtie.sigmax1[0] = tie->sigmax1[0];
          section2->fixedtie.sigmax1[1] = tie->sigmax1[1];
          section2->fixedtie.sigmax1[2] = tie->sigmax1[2];
          section2->fixedtie.sigmar2 = tie->sigmar2;
          section2->fixedtie.sigmax2[0] = tie->sigmax2[0];
          section2->fixedtie.sigmax2[1] = tie->sigmax2[1];
          section2->fixedtie.sigmax2[2] = tie->sigmax2[2];
          section2->fixedtie.sigmar3 = tie->sigmar3;
          section2->fixedtie.sigmax3[0] = tie->sigmax3[0];
          section2->fixedtie.sigmax3[1] = tie->sigmax3[1];
          section2->fixedtie.sigmax3[2] = tie->sigmax3[2];
          section2->fixedtie.inversion_status = tie->inversion_status;
          section2->fixedtie.inversion_offset_x = tie->inversion_offset_x;
          section2->fixedtie.inversion_offset_y = tie->inversion_offset_y;
          section2->fixedtie.inversion_offset_x_m = tie->inversion_offset_x_m;
          section2->fixedtie.inversion_offset_y_m = tie->inversion_offset_y_m;
          section2->fixedtie.inversion_offset_z_m = tie->inversion_offset_z_m;
          section2->fixedtie.dx_m = tie->dx_m;
          section2->fixedtie.dy_m = tie->dy_m;
          section2->fixedtie.dz_m = tie->dz_m;
          section2->fixedtie.sigma_m = tie->sigma_m;
          section2->fixedtie.dr1_m = tie->dr1_m;
          section2->fixedtie.dr2_m = tie->dr2_m;
          section2->fixedtie.dr3_m = tie->dr3_m;
          section2->fixedtie.rsigma_m = tie->rsigma_m;
          section2->fixedtie.isurveyplotindex  = tie->isurveyplotindex;
        }
        else if ((file2->status == MBNA_FILE_FIXEDNAV || file2->status == MBNA_FILE_FIXEDXYNAV)
            && (file1->status != MBNA_FILE_FIXEDNAV && file1->status != MBNA_FILE_FIXEDXYNAV)) {
          section1 = &file1->sections[crossing->section_1];
          if (file2->status == MBNA_FILE_FIXEDNAV)
            section1->fixedtie.status = MBNA_TIE_XYZ_FIXED;
          else if (file2->status == MBNA_FILE_FIXEDXYNAV)
            section1->fixedtie.status = MBNA_TIE_XY_FIXED;
          else if (file2->status == MBNA_FILE_FIXEDZNAV)
            section1->fixedtie.status = MBNA_TIE_Z_FIXED;
          else
            section1->fixedtie.status = MBNA_TIE_NONE;
          tie = &crossing->ties[0];
          section1->fixedtie.snav = tie->snav_1;
          section1->fixedtie.refgrid_id = 0;
          section1->fixedtie.snav_time_d = tie->snav_1_time_d;
          section1->fixedtie.offset_x = -tie->offset_x;
          section1->fixedtie.offset_y = -tie->offset_y;
          section1->fixedtie.offset_x_m = -tie->offset_x_m;
          section1->fixedtie.offset_y_m = -tie->offset_y_m;
          section1->fixedtie.offset_z_m = -tie->offset_z_m;
          section1->fixedtie.sigmar1 = tie->sigmar1;
          section1->fixedtie.sigmax1[0] = tie->sigmax1[0];
          section1->fixedtie.sigmax1[1] = tie->sigmax1[1];
          section1->fixedtie.sigmax1[2] = tie->sigmax1[2];
          section1->fixedtie.sigmar2 = tie->sigmar2;
          section1->fixedtie.sigmax2[0] = tie->sigmax2[0];
          section1->fixedtie.sigmax2[1] = tie->sigmax2[1];
          section1->fixedtie.sigmax2[2] = tie->sigmax2[2];
          section1->fixedtie.sigmar3 = tie->sigmar3;
          section1->fixedtie.sigmax3[0] = tie->sigmax3[0];
          section1->fixedtie.sigmax3[1] = tie->sigmax3[1];
          section1->fixedtie.sigmax3[2] = tie->sigmax3[2];
          section1->fixedtie.inversion_status = tie->inversion_status;
          section1->fixedtie.inversion_offset_x = -tie->inversion_offset_x;
          section1->fixedtie.inversion_offset_y = -tie->inversion_offset_y;
          section1->fixedtie.inversion_offset_x_m = -tie->inversion_offset_x_m;
          section1->fixedtie.inversion_offset_y_m = -tie->inversion_offset_y_m;
          section1->fixedtie.inversion_offset_z_m = -tie->inversion_offset_z_m;
          section1->fixedtie.dx_m = -tie->dx_m;
          section1->fixedtie.dy_m = -tie->dy_m;
          section1->fixedtie.dz_m = -tie->dz_m;
          section1->fixedtie.sigma_m = tie->sigma_m;
          section1->fixedtie.dr1_m = tie->dr1_m;
          section1->fixedtie.dr2_m = tie->dr2_m;
          section1->fixedtie.dr3_m = tie->dr3_m;
          section1->fixedtie.rsigma_m = tie->rsigma_m;
          section1->fixedtie.isurveyplotindex  = tie->isurveyplotindex;
        }
      }
    }

    /* count number of nav points, discontinuities, blocks, and global ties */
    nnav = 0;
    nblock = 0;
    nglobaltiexy = 0;
    nglobaltiez = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (!file->sections[0].continuity)
        nblock++;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        nnav += section->num_snav - section->continuity;
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiexy++;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiez++;
          }
        }
        else if (section->fixedtie.status != MBNA_TIE_NONE) {
          if (section->fixedtie.status == MBNA_TIE_XY || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_XY_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiexy++;
          }
          if (section->fixedtie.status == MBNA_TIE_Z || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_Z_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            nglobaltiez++;
          }
        }
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
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiexy * sizeof(int), (void **)&global_ties_xy_files, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiexy * sizeof(int), (void **)&global_ties_xy_sections, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiez * sizeof(int), (void **)&global_ties_z_files, &error);
    status = mb_mallocd(mbna_verbose, __FILE__, __LINE__, nglobaltiez * sizeof(int), (void **)&global_ties_z_sections, &error);
    memset(x_continuity, 0, nnav * sizeof(bool));
    memset(x_quality, 0, nnav * sizeof(int));
    memset(x_num_ties, 0, nnav * sizeof(int));
    memset(x_chunk, 0, nnav * sizeof(int));
    memset(x_time_d, 0, nnav * sizeof(double));
    memset(chunk_center, 0, nnav * sizeof(int));
    memset(chunk_continuity, 0, nnav * sizeof(bool));
    memset(global_ties_xy_files, 0, nglobaltiexy * sizeof(int));
    memset(global_ties_xy_sections, 0, nglobaltiexy * sizeof(int));
    memset(global_ties_z_files, 0, nglobaltiez * sizeof(int));
    memset(global_ties_z_sections, 0, nglobaltiez * sizeof(int));

    /* loop over all files getting tables of time, continuity and global ties */
    int inav = 0;
    nchunk = 0;
    nchunk_start = 0;
    chunk_distance = 25 * project.section_length;
    distance_sum = 0.0;
    nglobaltiexy = 0;
    nglobaltiez = 0;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      chunk_distance = 10 * file->sections[0].distance;
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_xy_files[nglobaltiexy] = ifile;
            global_ties_xy_sections[nglobaltiexy] = isection;
//fprintf(stderr, "%s:%d:%s: Adding global tie to XY list: %d   %4.4d:%2.2d\n",
//__FILE__, __LINE__, __FUNCTION__, nglobaltiexy, ifile, isection);
            nglobaltiexy++;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_z_files[nglobaltiez] = ifile;
            global_ties_z_sections[nglobaltiez] = isection;
//fprintf(stderr, "%s:%d:%s: Adding global tie to Z list: %d   %4.4d:%2.2d\n",
//__FILE__, __LINE__, __FUNCTION__, nglobaltiexy, ifile, isection);
            nglobaltiez++;
          }
        }
        else if (section->fixedtie.status != MBNA_TIE_NONE) {
          if (section->fixedtie.status == MBNA_TIE_XY || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_XY_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_xy_files[nglobaltiexy] = ifile;
            global_ties_xy_sections[nglobaltiexy] = isection;
//fprintf(stderr, "%s:%d:%s: Adding fixed tie to XY list: %d   %4.4d:%2.2d\n",
//__FILE__, __LINE__, __FUNCTION__, nglobaltiexy, ifile, isection);
            nglobaltiexy++;
          }
          if (section->fixedtie.status == MBNA_TIE_Z || section->fixedtie.status == MBNA_TIE_XYZ
              || section->fixedtie.status == MBNA_TIE_Z_FIXED || section->fixedtie.status == MBNA_TIE_XYZ_FIXED) {
            global_ties_z_files[nglobaltiez] = ifile;
            global_ties_z_sections[nglobaltiez] = isection;
//fprintf(stderr, "%s:%d:%s: Adding fixed tie to Z list: %d   %4.4d:%2.2d\n",
//__FILE__, __LINE__, __FUNCTION__, nglobaltiexy, ifile, isection);
            nglobaltiez++;
          }
        }
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
        nsmooth += 3;
      }
    }

    /* count second derivative smoothing points */
    for (int inav = 0; inav < nnav - 2; inav++) {
        if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
            nsmooth += 3;
        }
    }

    /*----------------------------------------------------------------*/
    /* Generate starting adjustment model by applying global ties. If a block
       has a single global tie then that will be applied uniformly to all nav
       points in that block. If a block has more than one global tie then the
       offsets are applied by linear interpolation in time over the block. */
    /*----------------------------------------------------------------*/

/*
fprintf(stderr, "\nGlobal ties XY %d:\n", nglobaltiexy);
    for (int igtie = 0; igtie < nglobaltiexy; igtie++) {
      if (project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.status != MBNA_TIE_NONE) {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Global Tie\n",
                  igtie, project.files[global_ties_xy_files[igtie]].block,
                  global_ties_xy_files[igtie],
                  global_ties_xy_sections[igtie],
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.snav,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.snav_time_d,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_x_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_y_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_z_m);
      }
      else {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Fixed Tie\n",
                  igtie, project.files[global_ties_xy_files[igtie]].block,
                  global_ties_xy_files[igtie],
                  global_ties_xy_sections[igtie],
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.snav,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.snav_time_d,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_x_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_y_m,
                  project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].fixedtie.offset_z_m);

      }
    }
fprintf(stderr, "\nGlobal ties Z %d:\n", nglobaltiez);
    for (int igtie = 0; igtie < nglobaltiez; igtie++) {
      if (project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.status != MBNA_TIE_NONE) {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Global Tie\n",
                  igtie, project.files[global_ties_z_files[igtie]].block,
                  global_ties_z_files[igtie],
                  global_ties_z_sections[igtie],
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.snav,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.snav_time_d,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_x_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_y_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_z_m);
      }
      else {
        fprintf(stderr, "%d %4.4d:%4.4d:%4.4d:%2.2d  %.6f  %.3f %.3f %.3f  Fixed Tie\n",
                  igtie, project.files[global_ties_z_files[igtie]].block,
                  global_ties_z_files[igtie],
                  global_ties_z_sections[igtie],
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.snav,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.snav_time_d,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_x_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_y_m,
                  project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].fixedtie.offset_z_m);
      }
    }
*/

    /* deal with xy global and/or fixed ties */
    for (int igtie = 0; igtie < nglobaltiexy; igtie++) {

      /* Note a conflict if the file for this global tie has xy navigation fixed */
      if (project.files[global_ties_xy_files[igtie]].status == MBNA_FILE_FIXEDNAV
          || project.files[global_ties_xy_files[igtie]].status == MBNA_FILE_FIXEDXYNAV) {
        fprintf(stdout, "MBnavadjust warning: An xy global tie has been defined for a file with xy navigation fixed.\n");
        fprintf(stdout, "  File: %2.2d:%5.5d %s   Section: %d  Offset: %f m east  %f m north  %f m vertical\n",
                project.files[global_ties_xy_sections[igtie]].block,
                global_ties_xy_files[igtie],
                project.files[global_ties_xy_sections[igtie]].file,
                global_ties_xy_sections[igtie],
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_x_m,
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_y_m,
                project.files[global_ties_xy_files[igtie]].sections[global_ties_xy_sections[igtie]].globaltie.offset_z_m);
        fprintf(stdout, "  This global tie will be ignored because the solution offset is constrained to be zero.\n\n");
      }

      /* deal with this global or fixed tie (global takes precedence if both exist) */
      int iblock_gtie = project.files[global_ties_xy_files[igtie]].block;
      int ifile_gtie = global_ties_xy_files[igtie];
      int isection_gtie = global_ties_xy_sections[igtie];
      int isnav_gtie;
      double global_offset_time_d;
      double global_offset_x_m;
      double global_offset_y_m;
      if (project.files[ifile_gtie].sections[isection_gtie].globaltie.status != MBNA_TIE_NONE) {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav_time_d;
        global_offset_x_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_x_m;
        global_offset_y_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_y_m;
      }
      else /* if (project.files[ifile_gtie].sections[isection_gtie].fixedtie.status != MBNA_TIE_NONE) */ {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav_time_d;
        global_offset_x_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_x_m;
        global_offset_y_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_y_m;
      }
      int ifile_gtie0 = -1;
      int isection_gtie0 = -1;
      int isnav_gtie0 = -1;
      double global_offset0_time_d = 0.0;
      double global_offset0_x_m = 0.0;
      double global_offset0_y_m = 0.0;
      int iblock_gtie1 = -1;
      int ifile_gtie1 = -1;
      if (igtie > 0) {
        ifile_gtie0 = global_ties_xy_files[igtie-1];
        isection_gtie0 = global_ties_xy_sections[igtie-1];
        if (project.files[ifile_gtie0].sections[isection_gtie0].globaltie.status != MBNA_TIE_NONE) {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav_time_d;
          global_offset0_x_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_x_m;
          global_offset0_y_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_y_m;
        }
        else /* if (project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.status != MBNA_TIE_NONE) */ {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav_time_d;
          global_offset0_x_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_x_m;
          global_offset0_y_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_y_m;
        }
      }
      if (igtie < nglobaltiexy - 1) {
        iblock_gtie1 = project.files[global_ties_xy_files[igtie+1]].block;
        ifile_gtie1 = global_ties_xy_files[igtie+1];
      }

      /* if this is the first global tie in a survey/block then set all previous nav
          in this block to the same offsets */
      if (igtie == 0 || project.files[global_ties_xy_files[igtie-1]].block != iblock_gtie) {
        /* loop over all files and sections up to this point - any in the same block
            will have the offsets set */
        for (int ifile = 0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = 0; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = 0; isnav <= isnav_max; isnav++) {
                section->snav_lon_offset[isnav] = global_offset_x_m * project.mtodeglon;
                section->snav_lat_offset[isnav] = global_offset_y_m * project.mtodeglat;
              }
            }
          }
        }
      }

      /* else if the previous global tie is in the same block linearly interpolate
          the offsets to the current global tie */
      else {
        for (int ifile = ifile_gtie0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie0)
              isectionmin = isection_gtie0;
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              double fraction = 0.0;
              int isnav_min = 0;
              if (ifile == ifile_gtie0 && isection == isection_gtie0)
                isnav_min = isnav_gtie0;
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                if (global_offset_time_d > global_offset0_time_d)
                  fraction = (section->snav_time_d[isnav] - global_offset0_time_d)
                                    / (global_offset_time_d - global_offset0_time_d);
                section->snav_lon_offset[isnav] = (global_offset0_x_m
                      + fraction * (global_offset_x_m - global_offset0_x_m)) * project.mtodeglon;
                section->snav_lat_offset[isnav] = (global_offset0_y_m
                      + fraction * (global_offset_y_m - global_offset0_y_m)) * project.mtodeglat;
              }
            }
          }
        }
      }

      /* if this is the last global tie in a survey/block then set all following nav
          in this block to the same offsets */
      if (igtie == nglobaltiexy - 1 || iblock_gtie != iblock_gtie1) {
        /* loop over all files and sections following this point - any in the same block
            will have the offsets set */
        int ifilemax = project.num_files - 1;
        if (iblock_gtie1 > 0 && ifile_gtie1 > ifile_gtie)
          ifilemax = ifile_gtie1 - 1;
        for (int ifile = ifile_gtie; ifile <= ifilemax; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie)
              isectionmin = isection_gtie;
            int isectionmax = file->num_sections - 1;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_min = 0;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_min = isnav_gtie;
              int isnav_max = section->num_snav - 1;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                section->snav_lon_offset[isnav] = global_offset_x_m * project.mtodeglon;
                section->snav_lat_offset[isnav] = global_offset_y_m * project.mtodeglat;
              }
            }
          }
        }
      }
    } // end nglobaltiexy

    /* deal with z global ties */
    for (int igtie = 0; igtie < nglobaltiez; igtie++) {

      /* Note a conflict if the file for this global tie has z navigation fixed */
      if (project.files[global_ties_z_files[igtie]].status == MBNA_FILE_FIXEDNAV
          || project.files[global_ties_z_files[igtie]].status == MBNA_FILE_FIXEDZNAV) {
        fprintf(stdout, "MBnavadjust warning: A z global tie has been defined for a file with z navigation fixed.\n");
        fprintf(stdout, "  File: %2.2d:%5.5d %s   Section: %d  Offset: %f m east  %f m north  %f m vertical\n",
                project.files[global_ties_z_sections[igtie]].block,
                global_ties_z_files[igtie],
                project.files[global_ties_z_sections[igtie]].file,
                global_ties_z_sections[igtie],
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_x_m,
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_y_m,
                project.files[global_ties_z_files[igtie]].sections[global_ties_z_sections[igtie]].globaltie.offset_z_m);
        fprintf(stdout, "  This global tie will be ignored because the solution offset is constrained to be zero.\n\n");
      }

      /* deal with this global or fixed tie (global takes precedence if both exist) */
      int iblock_gtie = project.files[global_ties_xy_files[igtie]].block;
      int ifile_gtie = global_ties_xy_files[igtie];
      int isection_gtie = global_ties_xy_sections[igtie];
      int isnav_gtie = -1;
      double global_offset_time_d = 0.0;
      double global_offset_z_m = 0.0;
      if (project.files[ifile_gtie].sections[isection_gtie].globaltie.status != MBNA_TIE_NONE) {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].globaltie.snav_time_d;
        global_offset_z_m = project.files[ifile_gtie].sections[isection_gtie].globaltie.offset_z_m;
      }
      else /* if (project.files[ifile_gtie].sections[isection_gtie].fixedtie.status != MBNA_TIE_NONE) */ {
        isnav_gtie = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav;
        global_offset_time_d = project.files[ifile_gtie].sections[isection_gtie].fixedtie.snav_time_d;
        global_offset_z_m = project.files[ifile_gtie].sections[isection_gtie].fixedtie.offset_z_m;
      }
      int ifile_gtie0 = -1;
      int isection_gtie0 = -1;
      int isnav_gtie0 = -1;
      double global_offset0_time_d = 0.0;
      double global_offset0_z_m = 0.0;
      int iblock_gtie1 = -1;
      int ifile_gtie1 = -1;
      if (igtie > 0) {
        ifile_gtie0 = global_ties_z_files[igtie-1];
        isection_gtie0 = global_ties_z_sections[igtie-1];
        if (project.files[ifile_gtie0].sections[isection_gtie0].globaltie.status != MBNA_TIE_NONE) {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.snav_time_d;
          global_offset0_z_m = project.files[ifile_gtie0].sections[isection_gtie0].globaltie.offset_z_m;
        }
        else /* if (project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.status != MBNA_TIE_NONE) */ {
          isnav_gtie0 = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav;
          global_offset0_time_d = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.snav_time_d;
          global_offset0_z_m = project.files[ifile_gtie0].sections[isection_gtie0].fixedtie.offset_z_m;
        }
      }
      if (igtie < nglobaltiez - 1) {
        iblock_gtie1 = project.files[global_ties_z_files[igtie+1]].block;
        ifile_gtie1 = global_ties_z_files[igtie+1];
      }

      /* if this is the first global tie in a survey/block then set all previous nav
          in this block to the same offsets */
      if (igtie == 0 || project.files[global_ties_z_files[igtie-1]].block != iblock_gtie) {
        /* loop over all files and sections up to this point - any in the same block
            will have the offsets set */
        for (int ifile = 0; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = 0; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              for (int isnav = 0; isnav < section->num_snav; isnav++) {
                section->snav_z_offset[isnav] = global_offset_z_m;
              }
            }
          }
        }
      }

      /* else if the previous global tie is in the same block linearly interpolate
          the offsets to the current global tie */
      else {
        for (int ifile = global_ties_z_files[igtie-1]; ifile <= ifile_gtie; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie0)
              isectionmin = isection_gtie0;
            int isectionmax = file->num_sections - 1;
            if (ifile == ifile_gtie)
              isectionmax = isection_gtie;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              double fraction = 0.0;
              int isnav_min = 0;
              if (ifile == ifile_gtie0 && isection == isection_gtie0)
                isnav_min = isnav_gtie0;
              int isnav_max = section->num_snav - 1;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_max = isnav_gtie;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                if (global_offset_time_d > global_offset0_time_d)
                  fraction = (section->snav_time_d[isnav] - global_offset0_time_d)
                                    / (global_offset_time_d - global_offset0_time_d);
                section->snav_z_offset[isnav] = (global_offset0_z_m
                      + fraction * (global_offset_z_m - global_offset0_z_m));
              }
            }
          }
        }
      }

      /* if this is the last global tie in a survey/block then set all following nav
          in this block to the same offsets */
      if (igtie == nglobaltiexy - 1 || iblock_gtie != iblock_gtie1) {
        /* loop over all files and sections following this point - any in the same block
            will have the offsets set */
        int ifilemax = project.num_files - 1;
        if (iblock_gtie1 > 0 && ifile_gtie1 > ifile_gtie)
          ifilemax = ifile_gtie1 - 1;
        for (int ifile = ifile_gtie; ifile <= ifilemax; ifile++) {
          file = &project.files[ifile];
          if (file->block == iblock_gtie) {
            int isectionmin = 0;
            if (ifile == ifile_gtie)
              isectionmin = isection_gtie;
            int isectionmax = file->num_sections - 1;
            for (int isection = isectionmin; isection <= isectionmax; isection++) {
              section = &file->sections[isection];
              int isnav_min = 0;
              if (ifile == ifile_gtie && isection == isection_gtie)
                isnav_min = isnav_gtie;
              int isnav_max = section->num_snav - 1;
              for (int isnav = isnav_min; isnav <= isnav_max; isnav++) {
                section->snav_z_offset[isnav] = global_offset_z_m;
              }
            }
          }
        }
      }
    } // end nglobaltiez

    fprintf(stderr, "\nApplied global ties to initial adjustment model:\n\tnglobaltiexy:%d\n\tnglobaltiez:%d\n",
            nglobaltiexy, nglobaltiez);

    /*
    fprintf(stderr, "\nInitial adjustment model:\n");
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (section->globaltie.status != MBNA_TIE_NONE && isnav == 0) {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f    %d %9.3f %9.3f %7.3f\n",
                  file->block, ifile, isection, isnav,
                  section->snav_lon_offset[isnav] / project.mtodeglon,
                  section->snav_lat_offset[isnav] / project.mtodeglat,
                  section->snav_z_offset[isnav],
                  section->globaltie.status,
                  section->globaltie.offset_x_m,
                  section->globaltie.offset_y_m,
                  section->globaltie.offset_z_m);
          } else {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f\n",
                    file->block, ifile, isection, isnav,
                    section->snav_lon_offset[isnav] / project.mtodeglon,
                    section->snav_lat_offset[isnav] / project.mtodeglat,
                    section->snav_z_offset[isnav]);
          }
        }
      }
    }
    */


    /*----------------------------------------------------------------*/
    /* Modify starting adjustment model by applying any fixed         */
    /* navigation. Note that global ties and fixed files can be in    */
    /* conflict. This stage could overwrite global tie constraints    */
    /* applied above.                                                 */
    /*----------------------------------------------------------------*/

    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
        for (int isection = 0; isection < file->num_sections; isection++) {
          section = &file->sections[isection];
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            section->snav_lon_offset[isnav] = 0.0;
            section->snav_lat_offset[isnav] = 0.0;
          }
        }
      }
      if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
        for (int isection = 0; isection < file->num_sections; isection++) {
          section = &file->sections[isection];
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            section->snav_z_offset[isnav] = 0.0;
          }
        }
      }
    }

    /*----------------------------------------------------------------*/
    /* Modify starting adjustment model by solving for average adjustments
       between the survey/blocks using the offset signal that remains after
       applying the current adjustment model */
    /*----------------------------------------------------------------*/

    /* get dimensions of inversion problem and initial misfit */
    ntie = 0;
    nrms = 0;
    nglobal = 0;
    nfixed = 0;
    rms_misfit_initial = 0.0;
    for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
      crossing = &project.crossings[icrossing];

      /* for block vs block averages use only set crossings between
       * different blocks - calculate the original rms  */
      if (crossing->status == MBNA_CROSSING_STATUS_SET) {
        for (int itie = 0; itie < crossing->num_ties; itie++) {
          /* get tie */
          tie = (struct mbna_tie *)&crossing->ties[itie];

          if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XYZ
              || tie->status == MBNA_TIE_XY_FIXED || tie->status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (tie->offset_x_m * tie->offset_x_m) + (tie->offset_y_m * tie->offset_y_m);
            nrms += 2;
            //ntie += 2;
          }
          if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_XYZ
              || tie->status == MBNA_TIE_Z_FIXED || tie->status == MBNA_TIE_XYZ_FIXED) {
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
        if (section->globaltie.status != MBNA_TIE_NONE) {
          if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (section->globaltie.offset_x_m * section->globaltie.offset_x_m)
                                    + (section->globaltie.offset_y_m * section->globaltie.offset_y_m);
            nrms += 2;
            nglobal += 2;
          }
          if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
              || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
            rms_misfit_initial += (section->globaltie.offset_z_m * section->globaltie.offset_z_m);
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

              file1 = &project.files[crossing->file_id_1];
              section1 = &file1->sections[crossing->section_1];
              file2 = &project.files[crossing->file_id_2];
              section2 = &file2->sections[crossing->section_2];

              if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                bxavg[jbvb] += tie->offset_x_m
                            - (section2->snav_lon_offset[tie->snav_2]
                                - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                byavg[jbvb] += tie->offset_y_m
                            - (section2->snav_lat_offset[tie->snav_2]
                                - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                nbxy[jbvb]++;
              }
              if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
                bzavg[jbvb] += tie->offset_z_m
                            - (section2->snav_z_offset[tie->snav_2]
                                - section1->snav_z_offset[tie->snav_1]);
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
          if (section->globaltie.status != MBNA_TIE_NONE) {
            if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
              bxfixstatus[file->block]++;
              bxfix[file->block] += section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
              byfixstatus[file->block]++;
              byfix[file->block] += section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
            }
            if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
              bzfixstatus[file->block]++;
              bzfix[file->block] += section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
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

    /* We do a three stage inversion: first for block averages, then a slow relaxation
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
      atol = 5.0e-7;   // releative precision of A matrix
      btol = 5.0e-7;   // relative precision of data array
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
                  section->snav_lon_offset[isnav] += file->block_offset_x * project.mtodeglon;
                  section->snav_lat_offset[isnav] += file->block_offset_y * project.mtodeglat;
                  section->snav_z_offset[isnav] += file->block_offset_z;
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
      fprintf(stderr, "\nBlock offsets (meters):\n");
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
                  if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                      offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                      offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                      rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                      nrms += 2;
                  }
                  if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
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
              if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                  offset_x =
                      section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                  offset_y =
                      section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                  rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                  nrms += 2;
              }
              if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status == MBNA_TIE_XY_FIXED) {
                  offset_z = section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
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

    /*
    fprintf(stderr, "\nAdjustment model after block inversion stage:\n");
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        for (int isnav = 0; isnav < section->num_snav; isnav++) {
          if (section->globaltie.status != MBNA_TIE_NONE && isnav == 0) {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f    %d %9.3f %9.3f %7.3f\n",
                  file->block, ifile, isection, isnav,
                  section->snav_lon_offset[isnav] / project.mtodeglon,
                  section->snav_lat_offset[isnav] / project.mtodeglat,
                  section->snav_z_offset[isnav],
                  section->globaltie.status,
                  section->globaltie.offset_x_m,
                  section->globaltie.offset_y_m,
                  section->globaltie.offset_z_m);
          } else {
            fprintf(stderr, "\t%4.4d:%4.4d:%4.4d:%2.2d %9.3f %9.3f %7.3f\n",
                    file->block, ifile, isection, isnav,
                    section->snav_lon_offset[isnav] / project.mtodeglon,
                    section->snav_lat_offset[isnav] / project.mtodeglat,
                    section->snav_z_offset[isnav]);
          }
        }
      }
    }
    */


        /* Stage 2 - Iteratively relax towards a coarse offset model in which
         * nav specified as poor is downweighted relative to good nav. The
         * coarseness is to solve for a navigation offset that is large scale
         * using a coarseness defined as 10 times the section length (which is
         * taken from the first section of the current file, as it can vary amongst
         * surveys in a project). The nav offsets of this coarse model will be
         * added to the global tie offsets and the block offsets to provide the
         * starting model for the the final inversion. Only the portions of the
         * crossing tie offsets not fit by this stage 2 model will be incorporated
         * into the inversion. The point of this multi-stage approach is to use
         * the least squares inversion to solve for zero mean, Gaussian distributed
         * offsets rather than the large scale offsets and drift characterizing
         * many multi-survey navigation adjustment problems.
         */

        /* loop over all ties applying the offsets to the chunks partitioned according to survey quality */
        n_iteration = 100000;
        convergence = 1000.0;
        convergence_prior = 1000.0;
        convergence_threshold = 0.000005;
        damping = 0.02;
        for (int iteration=0;
            iteration < n_iteration
                && convergence > convergence_threshold
                && convergence <= convergence_prior;
            iteration ++) {
            fprintf(stderr,"\nStage 2 relaxation iteration %d\n", iteration);

            /* zero the working average offset array */
            convergence_prior = convergence;
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
                        if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
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
                        if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
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
                    if (section->globaltie.status != MBNA_TIE_NONE) {

                        /* get absolute id for snav point */
                        int k = x_chunk[section->snav_invert_id[section->globaltie.snav]];

                        /* count global tie impact on chunks */
                        nx[k]++;

                        /* get and apply offset vector for this tie */
                        if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                            offset_x = section->globaltie.offset_x_m
                                - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                            offset_y = section->globaltie.offset_y_m
                                - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                            rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                            x[3*k]   += -offset_x;
                            x[3*k+1] += -offset_y;
                        }
                        if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
                            offset_z = section->globaltie.offset_z_m
                                - section->snav_z_offset[section->globaltie.snav];
                            rms_misfit_previous += offset_z * offset_z;
                            nrms += 1;
                            x[3*k+2] += -offset_z;
                        }
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
                        if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                            offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                            offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                            rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                            nrms += 2;
                        }
                        if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
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
                    if (section->globaltie.status != MBNA_TIE_Z && section->globaltie.status != MBNA_TIE_Z_FIXED) {
                        offset_x =
                            section->globaltie.offset_x_m - section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon;
                        offset_y =
                            section->globaltie.offset_y_m - section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat;
                        rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                        nrms += 2;
                    }
                    if (section->globaltie.status != MBNA_TIE_XY && section->globaltie.status != MBNA_TIE_XY_FIXED) {
                        offset_z = section->globaltie.offset_z_m - section->snav_z_offset[section->globaltie.snav];
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
    snprintf(message, sizeof(message), "Completed chunk inversion...");
    do_message_update(message);

    /*-------------------------------------------------------------------------*/
    /* Create complete inversion matrix problem to solve with LSQR             */
    /* - this is solving for a perturbation in addition to the model           */
    /* already constructed by the block inversion and then the chunk           */
    /* relaxation. Do this inversion with smoothing set by                     */
    /* project.smoothing using all crossing and global ties.                   */
    /*                                                                         */
    /* Invert each of the surveys separately first using only the ties within  */
    /* each survey and adding the result to the solution model. Then invert    */
    /* the whole project using all ties to fit the remmaining misfit.          */
    /*-------------------------------------------------------------------------*/

    for (int isurvey = -1; isurvey <= project.num_surveys; isurvey++) {
      matrix_scale = 1000.0;
      convergence = 1000.0;
      smooth_exp = project.smoothing;
      smoothweight = pow(10.0, smooth_exp) / 100.0;

      bool full_inversion = false;
      int inavstart = 0;
      int inavend = nnav - 1;
      if (isurvey == -1 || isurvey == project.num_surveys) {
        full_inversion = true;
        matrix.m = nrows;
        matrix.n = ncols;

        /* set message dialog on */
        if (isurvey == -1)
          snprintf(message, sizeof(message), "Performing initial navigation inversion using all crossing and global ties...");
        else
          snprintf(message, sizeof(message), "Performing final navigation inversion using all crossing and global ties...");
        do_message_on(message);
        fprintf(stderr, "\n------------------------------\n\nPreparing inversion of all surveys with smoothing %f ==> %f\n\t\tnfixed: %d  ntie: %d  nglobal: %d  nsmooth: %d\n\t\trows: %d  cols: %d\n",
              smooth_exp, smoothweight, nfixed, ntie, nglobal, nsmooth, matrix.m, matrix.n);
      }
      else {
        full_inversion = false;
        bool first = true;
        int ntie_surveyonly = 0;
        int nsmooth_surveyonly = 0;
        int nfixed_surveyonly = 0;
        int nglobal_surveyonly = 0;
        for (int ifile= 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          if (file->block == isurvey) {
            /* count fixed and global ties */
            for (int isection = 0; isection < file->num_sections; isection++) {
              section = &file->sections[isection];

              if (first) {
                inavstart = section->snav_invert_id[0];
                first = false;
              }
              inavend = section->snav_invert_id[section->num_snav-1];

              if (section->globaltie.status != MBNA_TIE_NONE) {
                if (section->globaltie.status == MBNA_TIE_XY || section->globaltie.status == MBNA_TIE_XYZ
                    || section->globaltie.status == MBNA_TIE_XY_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
                  rms_misfit_initial += (section->globaltie.offset_x_m * section->globaltie.offset_x_m)
                                          + (section->globaltie.offset_y_m * section->globaltie.offset_y_m);
                  nglobal_surveyonly += 2;
                }
                if (section->globaltie.status == MBNA_TIE_Z || section->globaltie.status == MBNA_TIE_XYZ
                    || section->globaltie.status == MBNA_TIE_Z_FIXED || section->globaltie.status == MBNA_TIE_XYZ_FIXED) {
                  rms_misfit_initial += (section->globaltie.offset_z_m * section->globaltie.offset_z_m);
                  nglobal_surveyonly += 1;
                }
              }

              /* count fixed sections for full inversion */
              if (file->status == MBNA_FILE_FIXEDNAV)
                nfixed_surveyonly += 3 * section->num_snav;
              else if (file->status == MBNA_FILE_FIXEDXYNAV)
                nfixed_surveyonly += 2 * section->num_snav;
              else if (file->status == MBNA_FILE_FIXEDZNAV)
                nfixed_surveyonly += 1 * section->num_snav;
            }
          }
        }

        /* count first derivative smoothing points */
        for (int inav = inavstart; inav < inavend - 1; inav++) {
          if (x_continuity[inav + 1]) {
            nsmooth_surveyonly += 3;
          }
        }

        /* count second derivative smoothing points */
        for (int inav = inavstart; inav < inavend - 2; inav++) {
            if (x_continuity[inav + 1] && x_continuity[inav + 2]) {
                nsmooth_surveyonly += 3;
            }
        }

        for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          if (crossing->status == MBNA_CROSSING_STATUS_SET
            && project.files[crossing->file_id_1].block == isurvey
            && project.files[crossing->file_id_2].block == isurvey) {
            for (int itie = 0; itie < crossing->num_ties; itie++) {
              ntie_surveyonly += 3;
            }
          }
        }

        matrix.m = ntie_surveyonly + nsmooth_surveyonly;
        matrix.n = 3 * (inavend - inavstart + 1);

        /* set message dialog on */
        snprintf(message, sizeof(message), "Performing navigation inversion for survey %d crossing ties only...", isurvey);
        do_message_on(message);
        fprintf(stderr, "\n------------------------------\n\nPreparing inversion of survey %d with smoothing %f ==> %f\n\t\tnfixed: %d  ntie: %d  nglobal: %d  nsmooth: %d\n\t\trows: %d  cols: %d\n",
              isurvey, smooth_exp, smoothweight, nfixed_surveyonly, ntie_surveyonly, nglobal_surveyonly, nsmooth_surveyonly, matrix.m, matrix.n);
      }

      int irow = 0;
      nrms = 0;
      rms_misfit_previous = 0.0;
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

      fprintf(stderr, "\n----------\n\nPreparing inversion of survey %d with smoothing %f ==> %f\n\t\trows: %d  cols: %d\n",
              isurvey, smooth_exp, smoothweight, matrix.m, matrix.n);

      /* loop over each crossing, applying offsets evenly to both points */
      for (int icrossing = 0; icrossing < project.num_crossings; icrossing++) {
          crossing = &project.crossings[icrossing];
          int nc1;
          int nc2;

          /* use only set crossings */
          if (crossing->status == MBNA_CROSSING_STATUS_SET
              && (full_inversion || (project.files[crossing->file_id_1].block == isurvey
                  && project.files[crossing->file_id_2].block == isurvey)))
              for (int itie = 0; itie < crossing->num_ties; itie++) {
                  /* A: get tie */
                  tie = (struct mbna_tie *)&crossing->ties[itie];
                  int index_m;
                  int index_n;

                  /* A1: get absolute id for first snav point */
                  file1 = &project.files[crossing->file_id_1];
                  section1 = &file1->sections[crossing->section_1];
                  nc1 = section1->snav_invert_id[tie->snav_1] - inavstart;

                  /* A2: get absolute id for second snav point */
                  file2 = &project.files[crossing->file_id_2];
                  section2 = &file2->sections[crossing->section_2];
                  nc2 = section2->snav_invert_id[tie->snav_2] - inavstart;

                  /* get uncertainty ellipsoid component magnitudes,
                      make them small if tie is set fixed so that
                      the solution actually closely matches the tie */
                  double sigmar1 = tie->sigmar1;
                  double sigmar2 = tie->sigmar2;
                  double sigmar3 = tie->sigmar3;
                  if (tie->status == MBNA_TIE_XY_FIXED
                      || tie->status == MBNA_TIE_Z_FIXED
                      || tie->status == MBNA_TIE_XYZ_FIXED) {
                      sigmar1 = 0.01;
                      sigmar2 = 0.01;
                      sigmar3 = 0.01;
                      }

                  if (section1->snav_time_d[tie->snav_1] ==
                      section2->snav_time_d[tie->snav_2])
                      fprintf(stderr, "ZERO TIME BETWEEN TIED POINTS!!  file:section:snav - %d:%d:%d   %d:%d:%d  DIFF:%f\n",
                              crossing->file_id_1, crossing->section_1, tie->snav_1, crossing->file_id_2, crossing->section_2,
                              tie->snav_2,
                              (section1->snav_time_d[tie->snav_1] -
                               section2->snav_time_d[tie->snav_2]));

                  /* A3: get offset vector for this tie */
                  if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
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
                  if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
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
                  if (fabs(sigmar1) > 0.0)
                      weight = 1.0 / sigmar1;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax1[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax1[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax1[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax1[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY
                      || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax1[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED || tie->status == MBNA_TIE_XY
                      || tie->status == MBNA_TIE_XY_FIXED)
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
                  if (fabs(sigmar2) > 0.0)
                      weight = 1.0 / sigmar2;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax2[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax2[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax2[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_Z || tie->status == MBNA_TIE_Z_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = weight * tie->sigmax2[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax2[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
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
                  if (fabs(sigmar3) > 0.0)
                      weight = 1.0 / sigmar3;
                  else
                      weight = 0.0;
                  weight *= matrix_scale;

                  index_m = irow * 6;
                  index_n = nc1 * 3;
                  matrix.ia[index_m] = index_n;
                  matrix.a[index_m] = -weight * tie->sigmax3[0];
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax3[0];

                  index_m = irow * 6 + 1;
                  index_n = nc2 * 3;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax3[0];

                  index_m = irow * 6 + 2;
                  index_n = nc1 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = -weight * tie->sigmax3[1];

                  index_m = irow * 6 + 3;
                  index_n = nc2 * 3 + 1;
                  matrix.ia[index_m] = index_n;
                  if (mbna_invert_mode == MBNA_INVERT_ZISOLATED
                      || tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else // if (mbna_invert_mode == MBNA_INVERT_ZFULL)
                      matrix.a[index_m] = weight * tie->sigmax3[1];

                  index_m = irow * 6 + 4;
                  index_n = nc1 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
                      matrix.a[index_m] = 0.0;
                  else
                      matrix.a[index_m] = -weight * tie->sigmax3[2];

                  index_m = irow * 6 + 5;
                  index_n = nc2 * 3 + 2;
                  matrix.ia[index_m] = index_n;
                  if (tie->status == MBNA_TIE_XY || tie->status == MBNA_TIE_XY_FIXED)
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
      if (full_inversion) {
          for (int ifile = 0; ifile < project.num_files; ifile++) {
              file = &project.files[ifile];
              for (int isection = 0; isection < file->num_sections; isection++) {
                  section = &file->sections[isection];
                  int index_m;
                  int index_n;
                  struct mbna_globaltie *globaltie = &section->globaltie;

                  /* get uncertainty ellipsoid component magnitudes,
                      make them small if tie is set fixed so that
                      the solution actually closely matches the tie */
                  double sigmar1 = globaltie->sigmar1;
                  double sigmar2 = globaltie->sigmar2;
                  double sigmar3 = globaltie->sigmar3;
                  if (globaltie->status == MBNA_TIE_XY_FIXED
                      || globaltie->status == MBNA_TIE_Z_FIXED
                      || globaltie->status == MBNA_TIE_XYZ_FIXED) {
                      sigmar1 = 0.01;
                      sigmar2 = 0.01;
                      sigmar3 = 0.01;
                      }

                  if (globaltie->status == MBNA_TIE_XYZ
                      || globaltie->status == MBNA_TIE_XY
                      || globaltie->status == MBNA_TIE_XYZ_FIXED
                      || globaltie->status == MBNA_TIE_XY_FIXED) {
                      offset_x = globaltie->offset_x_m - section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                      weight = 1.0 / sigmar1;
                      weight *= matrix_scale;
      //fprintf(stderr,"APPLYING WEIGHT: %f  ifile:%d isection:%d\n",weight,ifile,isection);

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_x;
                      //b[irow] = weight * (globaltie->offset_x_m - file->block_offset_x);
                      matrix.nia[irow] = 1;
                      irow++;

                      offset_y = globaltie->offset_y_m - section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                      weight = 1.0 / sigmar2;
                      weight *= matrix_scale;

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3 + 1;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_y;
                      //b[irow] = weight * (globaltie->offset_y_m - file->block_offset_y);
                      matrix.nia[irow] = 1;
                      irow++;

                      rms_misfit_previous += offset_x * offset_x + offset_y * offset_y;
                      nrms += 2;
                  }

                  if (globaltie->status == MBNA_TIE_XYZ
                      || globaltie->status == MBNA_TIE_Z
                      || globaltie->status == MBNA_TIE_XYZ_FIXED
                      || globaltie->status == MBNA_TIE_Z_FIXED) {
                      offset_z = globaltie->offset_z_m - section->snav_z_offset[globaltie->snav];
                      weight = 1.0 / sigmar3;
                      weight *= matrix_scale;

                      index_m = irow * 6;
                      index_n = (section->snav_invert_id[globaltie->snav] - inavstart) * 3 + 2;
                      matrix.ia[index_m] = index_n;
                      matrix.a[index_m] = weight;
                      b[irow] = weight * offset_z;
                      //b[irow] = weight * (globaltie->offset_z_m - file->block_offset_z);
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
              if (file->status == MBNA_FILE_FIXEDNAV
                  || file->status == MBNA_FILE_FIXEDXYNAV
                  || file->status == MBNA_FILE_FIXEDZNAV) {
                  for (int isection = 0; isection < file->num_sections; isection++) {
                      section = &file->sections[isection];
                      for (int isnav = 0; isnav < section->num_snav; isnav++) {
                          if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDXYNAV) {
                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3;
                              matrix.ia[index_m] = index_n;
                              matrix.a[index_m] = weight;
                              b[irow] = -file->block_offset_x;
                              matrix.nia[irow] = 1;
                              irow++;

                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3 + 1;
                              matrix.ia[index_m] = index_n;
                              matrix.a[index_m] = weight;
                              b[irow] = -file->block_offset_y;
                              matrix.nia[irow] = 1;
                              irow++;
                          }

                          if (file->status == MBNA_FILE_FIXEDNAV || file->status == MBNA_FILE_FIXEDZNAV) {
                              index_m = irow * 6;
                              index_n = (section->snav_invert_id[isnav] - inavstart) * 3 + 2;
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
      }

      /* E1: loop over all navigation applying first derivative smoothing */
      for (int inav = inavstart; inav < inavend; inav++) {
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

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -zweight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              b[irow] = 0.0;
              matrix.nia[irow] = 2;
              irow++;
          }
      }

      /* E1: loop over all navigation applying second derivative smoothing */
      for (int inav = inavstart; inav < inavend - 1; inav++) {
          int index_m;
          int index_n;
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

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * weight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * weight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3 + 1;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = weight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;

              index_m = irow * 6;
              index_n = (inav - inavstart) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              index_m = irow * 6 + 1;
              index_n = ((inav - inavstart) + 1) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = -2.0 * zweight;
              index_m = irow * 6 + 2;
              index_n = ((inav - inavstart) + 2) * 3 + 2;
              matrix.ia[index_m] = index_n;
              matrix.a[index_m] = zweight;
              b[irow] = 0.0;
              matrix.nia[irow] = 3;
              irow++;
          }
      }

      fprintf(stderr, "\nAbout to call LSQR rows: %d  cols: %d\n", matrix.m, matrix.n);

      /* F: call lsqr to solve the matrix problem */
      for (int irow = 0; irow < matrix.m; irow++)
          u[irow] = b[irow];
      damp = 0.0;
      atol = 5.0e-7;   // releative precision of A matrix
      btol = 5.0e-7;   // relative precision of data array
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
      for (int inav = inavstart; inav <= inavend; inav++) {
          int iinv = inav - inavstart;

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
                  x[iinv * 3] = x[(itielast - inavstart) * 3] + factor * (x[(itienext - inavstart) * 3] - x[(itielast - inavstart) * 3]);
                  x[iinv * 3 + 1] = x[(itielast - inavstart) * 3 + 1] + factor * (x[(itienext - inavstart) * 3 + 1] - x[(itielast - inavstart) * 3 + 1]);
                  x[iinv * 3 + 2] = x[(itielast - inavstart) * 3 + 2] + factor * (x[(itienext - inavstart) * 3 + 2] - x[(itielast - inavstart) * 3 + 2]);
              }
              else if (itielast >= 0) {
                  x[iinv * 3] = x[(itielast - inavstart) * 3];
                  x[iinv * 3 + 1] = x[(itielast - inavstart) * 3 + 1];
                  x[iinv * 3 + 2] = x[(itielast - inavstart) * 3 + 2];
              }
              else if (itienext >= 0) {
                  x[iinv * 3] = x[(itienext - inavstart) * 3];
                  x[iinv * 3 + 1] = x[(itienext - inavstart) * 3 + 1];
                  x[iinv * 3 + 2] = x[(itienext - inavstart) * 3 + 2];
              }
          }
      }

      /* save solution */
      rms_solution = 0.0;
      rms_solution_total = 0.0;
      nrms = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
          file = &project.files[ifile];
          if (full_inversion || file->block == isurvey) {
              for (int isection = 0; isection < file->num_sections; isection++) {
                  section = &file->sections[isection];
                  for (int isnav = 0; isnav < section->num_snav; isnav++) {
                      int k = section->snav_invert_id[isnav] - inavstart;
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
      }
      if (nrms > 0) {
          rms_solution = sqrt(rms_solution);
          rms_solution_total = sqrt(rms_solution_total);
      }
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

                  if (full_inversion || (file1->block == isurvey && file2->block == isurvey)) {
                      offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                      offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                      offset_z = tie->offset_z_m - (section2->snav_z_offset[tie->snav_2] - section1->snav_z_offset[tie->snav_1]);
/* fprintf(stderr, "Tie Offset: %5d:%1d  %4.4d:%2.2d:%2.2d:%2.2d  %4.4d:%2.2d:%2.2d:%2.2d  %10.3f %10.3f %10.3f\n",
icrossing, itie, file1->block, crossing->file_id_1, crossing->section_1, section1->snav_invert_id[tie->snav_1],
file2->block, crossing->file_id_2, crossing->section_2, section2->snav_invert_id[tie->snav_2],
offset_x, offset_y, offset_z); */
                  }
            }
      }


    } /* end loop over surveys */

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
                if (tie->status != MBNA_TIE_Z && tie->status != MBNA_TIE_Z_FIXED) {
                    offset_x = tie->offset_x_m - (section2->snav_lon_offset[tie->snav_2] - section1->snav_lon_offset[tie->snav_1]) / project.mtodeglon;
                    offset_y = tie->offset_y_m - (section2->snav_lat_offset[tie->snav_2] - section1->snav_lat_offset[tie->snav_1]) / project.mtodeglat;
                    rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                    nrms += 2;
                }
                if (tie->status != MBNA_TIE_XY && tie->status != MBNA_TIE_XY_FIXED) {
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
            struct mbna_globaltie *globaltie = &section->globaltie;
            if (globaltie->status != MBNA_TIE_NONE) {
                if (globaltie->status != MBNA_TIE_Z && globaltie->status != MBNA_TIE_Z_FIXED) {
                    offset_x =
                        globaltie->offset_x_m - section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                    offset_y =
                        globaltie->offset_y_m - section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                    rms_misfit_current += offset_x * offset_x + offset_y * offset_y;
                    nrms += 2;
                }
                if (globaltie->status != MBNA_TIE_XY || globaltie->status != MBNA_TIE_XY_FIXED) {
                    offset_z = globaltie->offset_z_m - section->snav_z_offset[globaltie->snav];
                    rms_misfit_current += offset_z * offset_z;
                    nrms += 1;
                }
            }
        }
    }
    if (nrms > 0) {
        rms_misfit_current = sqrt(rms_misfit_current) / nrms;
        convergence = (rms_misfit_previous - rms_misfit_current) / rms_misfit_previous;
    }

    fprintf(stderr, "\nInversion %d:\n > Solution size:        %12g\n"
            " > Total solution size:  %12g\n > Initial misfit:       %12g\n"
            " > Previous misfit:      %12g\n > Final misfit:         %12g\n"
            " > Convergence:          %12g\n",
            1, rms_solution, rms_solution_total, rms_misfit_initial,
            rms_misfit_previous, rms_misfit_current, convergence);

    /*-------------------------------------------------------------------------*/

    /* set message dialog on */
    snprintf(message, sizeof(message), "Completed inversion...");
    do_message_update(message);

    /* update model plot */
    if (project.modelplot)
      mbnavadjust_modelplot_plot(__FILE__, __LINE__);

    /* now output inverse solution */
    snprintf(message, sizeof(message), "Outputting navigation solution...");
    do_message_update(message);

    snprintf(message, sizeof(message), " > Final misfit:%12g\n > Initial misfit:%12g\n", rms_misfit_current, rms_misfit_initial);
    do_info_add(message, false);

    /* get crossing offset results */
    snprintf(message, sizeof(message), " > Nav Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    mb_path tie_file;
    strcpy(tie_file, project.path);
    strcat(tie_file, project.name);
    strcat(tie_file, "_tiesoln.txt");
    FILE *ofp = fopen(tie_file, "w");
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

          snprintf(message, sizeof(message), " >     %4d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f\n",
                  icrossing, tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                            tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                            tie->dx_m, tie->dy_m, tie->dz_m, tie->sigma_m);
          do_info_add(message, false);
          int snav_1_time_i[7], snav_2_time_i[7];
          double snav_1_time_d = project.files[crossing->file_id_1].sections[crossing->section_1].snav_time_d[tie->snav_1];
          double snav_2_time_d = project.files[crossing->file_id_2].sections[crossing->section_2].snav_time_d[tie->snav_2];
          mb_get_date(mbna_verbose, snav_1_time_d, snav_1_time_i);
          mb_get_date(mbna_verbose, snav_2_time_d, snav_2_time_i);
          double avg_tie_lon = 0.5 * (project.files[crossing->file_id_1].sections[crossing->section_1].snav_lon[tie->snav_1]
                                      + project.files[crossing->file_id_2].sections[crossing->section_2].snav_lon[tie->snav_2]);
          double avg_tie_lat = 0.5 * (project.files[crossing->file_id_1].sections[crossing->section_1].snav_lat[tie->snav_1]
                                      + project.files[crossing->file_id_2].sections[crossing->section_2].snav_lat[tie->snav_2]);
          fprintf(ofp,  "%2.2d:%4.4d:%3.3d:%2.2d %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d  %.6f "
                        "%2.2d:%4.4d:%3.3d:%2.2d %4.4d %2.2d %2.2d %2.2d:%2.2d:%2.2d.%6.6d  %.6f "
                        "%14.9f %14.9f "
                        "%8.2f %8.2f %8.2f   %8.2f %8.2f %8.2f   %8.2f %8.2f %8.2f\n",
                      project.files[crossing->file_id_1].block, crossing->file_id_1, crossing->section_1, tie->snav_1,
                      snav_1_time_i[0], snav_1_time_i[1], snav_1_time_i[2], snav_1_time_i[3], snav_1_time_i[4], snav_1_time_i[5], snav_1_time_i[6],
                      snav_1_time_d,
                      project.files[crossing->file_id_2].block, crossing->file_id_2, crossing->section_2, tie->snav_2,
                      snav_2_time_i[0], snav_2_time_i[1], snav_2_time_i[2], snav_2_time_i[3], snav_2_time_i[4], snav_2_time_i[5], snav_2_time_i[6],
                      snav_2_time_d,
                      avg_tie_lon, avg_tie_lat,
                      tie->offset_x_m, tie->offset_y_m, tie->offset_z_m,
                      tie->inversion_offset_x_m, tie->inversion_offset_y_m, tie->inversion_offset_z_m,
                      tie->dx_m, tie->dy_m, tie->dz_m);
        }
      }
    }
    fclose(ofp);

    /* get global tie results */
    snprintf(message, sizeof(message), " > Global Tie Offsets (m):  id  observed  solution  error\n");
    do_info_add(message, false);
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int isection = 0; isection < file->num_sections; isection++) {
        section = &file->sections[isection];
        struct mbna_globaltie *globaltie = &section->globaltie;
        if (globaltie->status != MBNA_TIE_NONE) {

          /* discard outrageous inversion_offset values - this happens if the inversion blows up */
          if (fabs(section->snav_lon_offset[globaltie->snav]) > 10000.0
                        || fabs(section->snav_lat_offset[globaltie->snav]) > 10000.0
                        || fabs(section->snav_z_offset[globaltie->snav]) > 10000.0) {
            globaltie->inversion_status = MBNA_INVERSION_OLD;
            globaltie->inversion_offset_x = 0.0;
            globaltie->inversion_offset_y = 0.0;
            globaltie->inversion_offset_x_m = 0.0;
            globaltie->inversion_offset_y_m = 0.0;
            globaltie->inversion_offset_z_m = 0.0;
            globaltie->dx_m = 0.0;
            globaltie->dy_m = 0.0;
            globaltie->dz_m = 0.0;
            globaltie->sigma_m = 0.0;
            globaltie->dr1_m = 0.0;
            globaltie->dr2_m = 0.0;
            globaltie->dr3_m = 0.0;
            globaltie->rsigma_m = 0.0;
          }
          else {
            globaltie->inversion_status = MBNA_INVERSION_CURRENT;
                        globaltie->inversion_offset_x = section->snav_lon_offset[globaltie->snav];
                        globaltie->inversion_offset_y = section->snav_lat_offset[globaltie->snav];
                        globaltie->inversion_offset_x_m = section->snav_lon_offset[globaltie->snav] / project.mtodeglon;
                        globaltie->inversion_offset_y_m = section->snav_lat_offset[globaltie->snav] / project.mtodeglat;
                        globaltie->inversion_offset_z_m = section->snav_z_offset[globaltie->snav];
                        globaltie->dx_m = globaltie->offset_x_m - globaltie->inversion_offset_x_m;
                        globaltie->dy_m = globaltie->offset_y_m - globaltie->inversion_offset_y_m;
                        globaltie->dz_m = globaltie->offset_z_m - globaltie->inversion_offset_z_m;
                        globaltie->sigma_m = sqrt(globaltie->dx_m * globaltie->dx_m + globaltie->dy_m * globaltie->dy_m + globaltie->dz_m * globaltie->dz_m);
                        globaltie->dr1_m = globaltie->inversion_offset_x_m / globaltie->sigmar1;
                        globaltie->dr2_m = globaltie->inversion_offset_y_m / globaltie->sigmar2;
                        globaltie->dr3_m = globaltie->inversion_offset_z_m / globaltie->sigmar3;
                        globaltie->rsigma_m = sqrt(globaltie->dr1_m * globaltie->dr1_m + globaltie->dr2_m * globaltie->dr2_m + globaltie->dr3_m * globaltie->dr3_m);
          }
          snprintf(message, sizeof(message), 
                  " >     %2.2d:%2.2d:%2.2d %d   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f\n",
                  ifile, isection, globaltie->snav, globaltie->status,
                  globaltie->offset_x_m, globaltie->offset_y_m, globaltie->offset_z_m,
                  globaltie->inversion_offset_x_m, globaltie->inversion_offset_y_m, globaltie->inversion_offset_z_m,
                  globaltie->dx_m, globaltie->dy_m, globaltie->dz_m);
          do_info_add(message, false);
        }
      }
    }

    /* write updated project */
    project.inversion_status = MBNA_INVERSION_CURRENT;
        project.modelplot_uptodate = false;
    project.grid_status = MBNA_GRID_OLD;
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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

int mbnavadjust_updategrid() {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
  }

  int status = MB_SUCCESS;
  struct mbna_file *file;
  struct mbna_section *section;
  mb_pathplus npath;
  mb_pathplus apath;
  mb_pathplus spath;
  mb_pathplus command;
  FILE *nfp, *afp;
  char *result;
  mb_command buffer;
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
  mb_pathplus ostring;
  int isection, isnav;
  double seconds;
  double lon_min, lon_max, lat_min, lat_max;

  /* generate current topography grid */
  if (project.open && project.num_files > 0 && error == MB_ERROR_NO_ERROR) {
    /* set message */
    snprintf(message, sizeof(message), "Setting up to generate current topography grid...");
    do_message_on(message);
    do_info_add(message, false);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);

    /* update datalist files and mbgrid commands */
    snprintf(apath, sizeof(apath), "%s/datalist.mb-1", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        for (int j = 0; j < file->num_sections; j++) {
          fprintf(afp, "nvs_%4.4d_%4.4d.mb71 71\n", file->id, j);
        }
      }
      fclose(afp);
    }
    for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
      snprintf(apath, sizeof(apath), "%s/datalist_%4.4d.mb-1", project.datadir, isurvey);
      if ((afp = fopen(apath, "w")) != NULL) {
        for (int ifile = 0; ifile < project.num_files; ifile++) {
          if (project.files[ifile].block == isurvey) {
            file = &project.files[ifile];
            for (int j = 0; j < file->num_sections; j++) {
              fprintf(afp, "nvs_%4.4d_%4.4d.mb71 71\n", file->id, j);
            }
          }
        }
        fclose(afp);
      }
    }

    double dlon = 0.1 * (project.lon_max - project.lon_min);
    double dlat = 0.1 * (project.lat_max - project.lat_min);
    lon_min = project.lon_min - dlon;
    lon_max = project.lon_max + dlon;
    lat_min = project.lat_min - dlat;
    lat_max = project.lat_max + dlat;
    snprintf(apath, sizeof(apath), "%s/mbgrid_adj.cmd", project.datadir);
    if ((afp = fopen(apath, "w")) != NULL) {
      fprintf(afp, "mbgrid -I datalistp.mb-1 \\\n\t-R%.8f/%.8f/%.8f/%.8f \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopoAdj\n\n",
              lon_min, lon_max, lat_min, lat_max);

      for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
        bool first_file = true;
        for (int ifile = 0; ifile < project.num_files; ifile++) {
          if (project.files[ifile].block == isurvey) {
            for (int isection=0; isection < project.files[ifile].num_sections; isection++) {
              if (first_file && isection == 0) {
                first_file = false;
                lon_min = project.files[ifile].sections[isection].lonmin;
                lon_max = project.files[ifile].sections[isection].lonmax;
                lat_min = project.files[ifile].sections[isection].latmin;
                lat_max = project.files[ifile].sections[isection].latmax;
              } else {
                lon_min = MIN(project.files[ifile].sections[isection].lonmin, lon_min);
                lon_max = MAX(project.files[ifile].sections[isection].lonmax, lon_max);
                lat_min = MIN(project.files[ifile].sections[isection].latmin, lat_min);
                lat_max = MAX(project.files[ifile].sections[isection].latmax, lat_max);
              }
            }
          }
        }
        lon_min -= dlon;
        lon_max += dlon;
        lat_min -= dlat;
        lat_max += dlat;
        fprintf(afp, "mbgrid -I datalist_%4.4dp.mb-1 \\\n\t-A2 -F5 -N -C2 \\\n\t-O ProjectTopoAdj_%4.4d\n\n",
                isurvey, isurvey);
      }
      fclose(afp);
    }

    snprintf(command, sizeof(command), "chmod +x %s/mbgrid_adj.cmd", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);

    /* run mbdatalist to create datalistp.mb-1, update ancillary files,
        and clear any processing lock files */
    snprintf(message, sizeof(message), " > Running mbdatalist in project\n");
    do_info_add(message, false);
    if (mbna_verbose == 0)
      fprintf(stderr, "%s", message);
    snprintf(command, sizeof(command), "cd %s ; mbdatalist -Idatalist.mb-1 -O -Y -Z -V", project.datadir);
    fprintf(stderr, "Executing:\n%s\n\n", command);
    /* const int shellstatus = */ system(command);
    for (int isurvey = 0; isurvey < project.num_surveys; isurvey++) {
      snprintf(command, sizeof(command), "cd %s ; mbdatalist -Idatalist_%4.4d.mb-1 -Z -V", project.datadir, isurvey);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
    }

    if (project.inversion_status != MBNA_INVERSION_NONE) {
      /* set message */
      snprintf(message, sizeof(message), "Applying navigation solution within the project...");
      do_message_on(message);
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);

      /* generate new nav files */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        snprintf(npath, sizeof(npath), "%s/nvs_%4.4d.mb166", project.datadir, ifile);
        snprintf(apath, sizeof(apath), "%s/nvs_%4.4d.na0", project.datadir, ifile);
        if ((nfp = fopen(npath, "r")) == NULL) {
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          snprintf(message, sizeof(message), " > Unable to read initial nav file %s\n", npath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else if ((afp = fopen(apath, "w")) == NULL) {
          fclose(nfp);
          status = MB_FAILURE;
          error = MB_ERROR_OPEN_FAIL;
          snprintf(message, sizeof(message), " > Unable to open output nav file %s\n", apath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);
        }
        else {
          snprintf(message, sizeof(message), " > Output updated nav to %s\n", apath);
          do_info_add(message, false);
          if (mbna_verbose == 0)
            fprintf(stderr, "%s", message);

          /* write file header */
          char user[256], host[256], date[32];
          status = mb_user_host_date(mbna_verbose, user, host, date, &error);
          gethostname(host, MBP_FILENAMESIZE);
          snprintf(ostring, sizeof(ostring), "# Adjusted navigation generated using MBnavadjust\n");
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MB-System version:        %s\n", MB_VERSION);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MB-System build data:     %s\n", MB_VERSION_DATE);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project name: %s\n", project.name);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project path: %s\n", project.path);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# MBnavadjust project home: %s\n", project.home);
          fprintf(afp, "%s", ostring);
          snprintf(ostring, sizeof(ostring), "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
          fprintf(afp, "%s", ostring);

          /* read the input nav */
          isection = 0;
          section = &file->sections[isection];
          isnav = 0;
          bool done = false;
          while (!done) {
            if ((result = fgets(buffer, sizeof(buffer), nfp)) != buffer) {
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
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
//ifile, isection, isnav);
              }
              else if (time_d > section->snav_time_d[isnav + 1]) {
                factor = 1.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
//ifile, isection, isnav+1);
              }
              else {
                if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                  factor = (time_d - section->snav_time_d[isnav]) /
                           (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
                }
                else {
                  factor = 0.0;
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
                snprintf(ostring, sizeof(ostring), 
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

            snprintf(spath, sizeof(spath), "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, isection);

            status = mb_pr_update_format(mbna_verbose, spath, true, 71, &error);
            status = mb_pr_update_navadj(mbna_verbose, spath, MBP_NAVADJ_LLZ, apath, MBP_NAV_LINEAR, &error);
          }
        }
      }

      /* run mbprocess */
      snprintf(message, sizeof(message), " > Running mbprocess in project\n");
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      snprintf(command, sizeof(command), "cd %s ; mbprocess -C4", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
    }

    if (project.grid_status != MBNA_GRID_CURRENT) {
      /* run mbgrid */
      snprintf(message, sizeof(message), " > Running mbgrid_adj\n");
      do_info_add(message, false);
      if (mbna_verbose == 0)
        fprintf(stderr, "%s", message);
      snprintf(command, sizeof(command), "cd %s ; ./mbgrid_adj.cmd", project.datadir);
      fprintf(stderr, "Executing:\n%s\n\n", command);
      /* const int shellstatus = */ system(command);
      project.grid_status = MBNA_GRID_CURRENT;

      /* write current project */
      mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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

  /* output results from navigation solution */
  if (project.open && project.num_crossings > 0 &&
      (project.num_crossings_analyzed >= 10 || project.num_truecrossings_analyzed == project.num_truecrossings) &&
      error == MB_ERROR_NO_ERROR) {

    mb_pathplus npath;
    mb_pathplus opath;
    mb_path ppath;
    FILE *nfp, *ofp;

    /* now output inverse solution */
    snprintf(message, sizeof(message), "Applying navigation solution...");
    do_message_on(message);

    /* generate new nav files */
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      struct mbna_file *file = &project.files[ifile];

      snprintf(npath, sizeof(npath), "%s/nvs_%4.4d.mb166", project.datadir, ifile);
      if (project.use_mode == MBNA_USE_MODE_PRIMARY) {
        snprintf(opath, sizeof(opath), "%s.na%d", file->path, 0);
      }
      else {
        status = mb_pr_get_output(mbna_verbose, &file->format, file->path, ppath, &error);
        if (project.use_mode == MBNA_USE_MODE_SECONDARY) {
          snprintf(opath, sizeof(ppath), "%s.na%d", ppath, 1);
        }
        else {
          snprintf(opath, sizeof(ppath), "%s.na%d", ppath, 2);
        }
      }
      if ((nfp = fopen(npath, "r")) == NULL) {
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        snprintf(message, sizeof(message), " > Unable to read initial nav file %s\n", npath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else if ((ofp = fopen(opath, "w")) == NULL) {
        fclose(nfp);
        status = MB_FAILURE;
        error = MB_ERROR_OPEN_FAIL;
        snprintf(message, sizeof(message), " > Unable to open output nav file %s\n", opath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);
      }
      else {
        snprintf(message, sizeof(message), " > Output updated nav to %s\n", opath);
        do_info_add(message, false);
        if (mbna_verbose == 0)
          fprintf(stderr, "%s", message);

        /* write file header */
        char user[256], host[256], date[32];
        mb_pathplus ostring;
        status = mb_user_host_date(mbna_verbose, user, host, date, &error);
        snprintf(ostring, sizeof(ostring), "# Adjusted navigation generated using MBnavadjust\n");
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MB-System version:        %s\n", MB_VERSION);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MB-System build data:     %s\n", MB_VERSION_DATE);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project name: %s\n", project.name);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project path: %s\n", project.path);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# MBnavadjust project home: %s\n", project.home);
        fprintf(ofp, "%s", ostring);
        snprintf(ostring, sizeof(ostring), "# Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
        fprintf(ofp, "%s", ostring);

        /* read the input nav */
        int isection = 0;
        struct mbna_section *section = &file->sections[isection];
        int isnav = 0;
        bool done = false;
        char *result = NULL;
        mb_command buffer;
        int nscan;
        int time_i[7];
        double seconds;
        double time_d;
        double navlon;
        double navlat;
        double heading;
        double speed;
        double draft;
        double roll;
        double pitch;
        double heave;
        double zoffset;
        while (!done) {
          if ((result = fgets(buffer, sizeof(buffer), nfp)) != buffer) {
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
            double factor;
            if (time_d < section->snav_time_d[isnav]) {
              factor = 0.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f < %f ifile:%d isection:%d isnav:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav],
//ifile, isection, isnav);
            }
            else if (time_d > section->snav_time_d[isnav + 1]) {
              factor = 1.0;
//fprintf(stderr,"%s:%4.4d:%s: Nav time outside expected section: %f > %f ifile:%d isection:%d isnav+1:%d\n",
//__FILE__, __LINE__, __func__, time_d, section->snav_time_d[isnav + 1],
//ifile, isection, isnav+1);
            }
            else {
              if (section->snav_time_d[isnav + 1] > section->snav_time_d[isnav]) {
                factor = (time_d - section->snav_time_d[isnav]) /
                         (section->snav_time_d[isnav + 1] - section->snav_time_d[isnav]);
              }
              else {
                factor = 0.0;
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
              snprintf(ostring, sizeof(ostring), 
                      "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d.%6.6d %16.6f %.10f %.10f %.2f %.2f %.3f %.2f %.2f %.2f "
                      "%.3f\r\n",
                      time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], time_d, navlon,
                      navlat, heading, speed, draft, roll, pitch, heave, zoffset);
              fprintf(ofp, "%s", ostring);
              /* fprintf(stderr, "NAV OUT: %3.3d:%3.3d:%2.2d factor:%f | %s", i,isection,isnav,factor,ostring); */
            }
          }
        }
        fclose(nfp);
        fclose(ofp);

        if (project.use_mode == MBNA_USE_MODE_PRIMARY) {
          int mbp_heading_mode;
          double mbp_headingbias;
          int mbp_rollbias_mode;
          double mbp_rollbias;
          double mbp_rollbias_port;
          double mbp_rollbias_stbd;

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
    else if (project.modelplot_style == MBNA_MODELPLOT_TIEOFFSETS) {
      if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
              || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {
        mbnavadjust_modelplot_pick_globaltieoffsets(x, y);
      }
      else {
        mbnavadjust_modelplot_pick_tieoffsets(x, y);
      }
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
               (int)(mbna_modelplot_yscale
                 * (section->snav_lon_offset[tie->snav_1] / project.mtodeglon - mbna_modelplot_yxmid));
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
               (int)(mbna_modelplot_yscale
                 * (section->snav_lat_offset[tie->snav_1] / project.mtodeglat - mbna_modelplot_yymid));
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
                (int)(mbna_modelplot_yzscale
                  * (section->snav_z_offset[tie->snav_1] - mbna_modelplot_yzmid));
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
               (int)(mbna_modelplot_yscale
                 * (section->snav_lon_offset[tie->snav_2] / project.mtodeglon - mbna_modelplot_yxmid));
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
               (int)(mbna_modelplot_yscale
                 * (section->snav_lat_offset[tie->snav_2] / project.mtodeglat - mbna_modelplot_yymid));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            pick_file = crossing->file_id_2;
            pick_section = crossing->section_2;
            pick_snav = tie->snav_2;
          }

          iy = mbna_modelplot_yo_z
                - (int)(mbna_modelplot_yzscale
                  * (section->snav_z_offset[tie->snav_2] - mbna_modelplot_yzmid));
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
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
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
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
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

        iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (tie->offset_x_m - mbna_modelplot_yxmid));
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
        }

        iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (tie->offset_y_m - mbna_modelplot_yymid));
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
        }

        iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (tie->offset_z_m - mbna_modelplot_yzmid));
        range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
        if (range < rangemin) {
          rangemin = range;
          pick_crossing = i;
          pick_tie = j;
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
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
      }

      /* else if naverr window is up, load selected crossing */
      else {
        mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
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

int mbnavadjust_modelplot_pick_globaltieoffsets(int x, int y) {
  if (mbna_verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2       x:           %d\n", x);
    fprintf(stderr, "dbg2       y:           %d\n", y);
  }

  int status = MB_SUCCESS;

  /* find nearest global tie in model plot */
  if (project.open && project.modelplot
    && project.modelplot_style == MBNA_MODELPLOT_TIEOFFSETS
    && (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
        || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED)) {
    struct mbna_file *file;
    struct mbna_section *section;
    int range;
    int pick_file;
    int pick_section;
    int ix, iy;
    int rangemin = 10000000;
    for (int ifile = 0; ifile < project.num_files; ifile++) {
      file = &project.files[ifile];
      for (int jsection = 0; jsection < file->num_sections; jsection++) {
        section = &file->sections[jsection];
        if (section->globaltie.isurveyplotindex >= 0) {
          if (section->globaltie.isurveyplotindex >= mbna_modelplot_tiestart &&
              section->globaltie.isurveyplotindex <= mbna_modelplot_tieend) {
            ix = mbna_modelplot_xo +
                 (int)(mbna_modelplot_xscale * (section->globaltie.isurveyplotindex - mbna_modelplot_tiestart + 1));

            iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->globaltie.offset_x_m - mbna_modelplot_yxmid));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_file = ifile;
              pick_section = jsection;
            }

            iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->globaltie.offset_y_m - mbna_modelplot_yymid));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_file = ifile;
              pick_section = jsection;
            }

            iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->globaltie.offset_z_m - mbna_modelplot_yzmid));
            range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
            if (range < rangemin) {
              rangemin = range;
              pick_file = ifile;
              pick_section = jsection;
            }
          }
        }
      }
    }

    /* deal with successful pick */
    if (rangemin < 10000000) {
      mbna_file_select = pick_file;
      mbna_section_select = pick_section;
      mbna_modelplot_pickfile = pick_file;
      mbna_modelplot_picksection = pick_section;
      mbna_modelplot_picksnav = 0;
      mbna_crossing_select = MBNA_SELECT_NONE;
      mbna_tie_select = MBNA_SELECT_NONE;

      /* bring up naverr window if required */
      if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
        do_naverr_init(MBNA_NAVERR_MODE_SECTION);
      }

      /* else if naverr window is up, load selected crossing */
      else {
        mbnavadjust_naverr_specific_section(mbna_file_select, mbna_section_select);
        mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
        do_naverr_update();
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
          if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
            do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_naverr_update();
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
          if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
            do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_naverr_update();
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
          if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
            do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_naverr_update();
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
          if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
            do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
          }

          /* else if naverr window is up, load selected crossing */
          else {
            mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
            mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
            do_naverr_update();
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

          iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (tie->offset_x_m - mbna_modelplot_yxmid));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            // pick_file = crossing->file_id_1;
            // pick_section = crossing->section_1;
            // pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (tie->offset_y_m - mbna_modelplot_yymid));
          range = (ix - x) * (ix - x) + (iy - y) * (iy - y);
          if (range < rangemin) {
            rangemin = range;
            pick_crossing = i;
            pick_tie = j;
            // pick_file = crossing->file_id_1;
            // pick_section = crossing->section_1;
            // pick_snav = tie->snav_1;
          }

          iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (tie->offset_z_m - mbna_modelplot_yzmid));
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
        if (mbna_naverr_mode == MBNA_NAVERR_MODE_UNLOADED) {
          do_naverr_init(MBNA_NAVERR_MODE_CROSSING);
        }

        /* else if naverr window is up, load selected crossing */
        else {
          mbnavadjust_naverr_specific_crossing(mbna_crossing_select, mbna_tie_select);
          mbnavadjust_naverr_plot(MBNA_PLOT_MODE_FIRST);
          do_naverr_update();
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
    mbnavadjust_write_project(mbna_verbose, &project, __FILE__, __LINE__, __FUNCTION__, &error);
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
  mb_path label;
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
      snprintf(label, sizeof(label), "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
      snprintf(label, sizeof(label), "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      snprintf(label, sizeof(label), "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      snprintf(label, sizeof(label), "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      snprintf(label, sizeof(label), "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
              mbna_file_select, mbna_section_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
      snprintf(label, sizeof(label), "Display All Data");
    }

    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* plot labels */
    snprintf(label, sizeof(label), "East-West Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "North-South Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "Vertical Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * yzmax);
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
        if (section->show_in_modelplot && section->globaltie.status != MBNA_TIE_NONE) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[section->globaltie.snav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            if (section->globaltie.status != MBNA_TIE_Z) {
              /* east-west offsets */
              iy = mbna_modelplot_yo_lon -
                   (int)(mbna_modelplot_yscale * section->globaltie.offset_x /
                         project.mtodeglon);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

              /* north-south offsets */
              iy = mbna_modelplot_yo_lat -
                   (int)(mbna_modelplot_yscale * section->globaltie.offset_y /
                         project.mtodeglat);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }

            if (section->globaltie.status != MBNA_TIE_XY) {
              /* vertical offsets */
              iy = mbna_modelplot_yo_z -
                   (int)(mbna_modelplot_yzscale * section->globaltie.offset_z_m);
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
  mb_path label;
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
      snprintf(label, sizeof(label), "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
      snprintf(label, sizeof(label), "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
      snprintf(label, sizeof(label), "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
      snprintf(label, sizeof(label), "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
      snprintf(label, sizeof(label), "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
              mbna_file_select, mbna_section_select);
    }
    else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
      snprintf(label, sizeof(label), "Display All Data");
    }

    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    /* plot labels */
    snprintf(label, sizeof(label), "East-West Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "North-South Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * xymax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "Vertical Offset (meters) vs. Ping Count");
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
    iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_start);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%d", mbna_modelplot_end);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
    iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 1.1 * yzmax);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", 0.0);
    xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
    ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
    iy = mbna_modelplot_yo_z + stringascent / 2;
    xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

    snprintf(label, sizeof(label), "%.2f", -1.1 * yzmax);
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
        if (section->show_in_modelplot && section->globaltie.status != MBNA_TIE_NONE) {
          for (int isnav = 0; isnav < section->num_snav; isnav++) {
            iping = section->modelplot_start_count + section->snav_id[section->globaltie.snav];
            ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (iping - mbna_modelplot_start));

            if (section->globaltie.status != MBNA_TIE_Z) {
              /* east-west offsets */
              iy = mbna_modelplot_yo_lon -
                   (int)(mbna_modelplot_yscale *
                         (section->snav_lon_offset[section->globaltie.snav] / project.mtodeglon -
                          file->block_offset_x));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

              /* north-south offsets */
              iy = mbna_modelplot_yo_lat -
                   (int)(mbna_modelplot_yscale *
                         (section->snav_lat_offset[section->globaltie.snav] / project.mtodeglat -
                          file->block_offset_y));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat, ix, iy, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_fillrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[ORANGE], XG_SOLIDLINE);
              xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel_values[mbna_color_foreground], XG_SOLIDLINE);
            }

            if (section->globaltie.status != MBNA_TIE_XY) {
              /* vertical offsets */
              iy = mbna_modelplot_yo_z -
                   (int)(mbna_modelplot_yzscale *
                         (section->snav_z_offset[section->globaltie.snav] - file->block_offset_z));
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
  int plot_width;
  int plot_height;
  bool first;
  mb_path label;
  int stringwidth, stringascent, stringdescent;
  int pixel;
  int itiestart, itieend;
  int ix, iy;
  int num_ties_block;
  int plot_index;

  /* plot global tie offsets */
  if (project.open && project.modelplot
      && project.modelplot_style == MBNA_MODELPLOT_TIEOFFSETS) {
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

    /* plot global tie offsets */
    if (mbna_view_list == MBNA_VIEW_LIST_GLOBALTIES
          || mbna_view_list == MBNA_VIEW_LIST_GLOBALTIESSORTED) {

      /* count global ties */
      mbna_num_ties_plot = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        file->show_in_modelplot = -1;
        for (int jsection = 0; jsection < file->num_sections; jsection++) {
          section = &file->sections[jsection];
          section->globaltie.isurveyplotindex = -1;
          if (section->status == MBNA_CROSSING_STATUS_SET) {

            /* check to see if this tie should be plotted */
            if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK) {
              if (file->block == mbna_block_select1 || file->block == mbna_block_select2) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_SURVEY) {
              if (file->block == mbna_survey_select) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
              if (file->block == mbna_survey_select) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
              if (ifile == mbna_file_select) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
              if (ifile == mbna_file_select) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
              if (ifile == mbna_file_select && jsection == mbna_section_select) {
                section->globaltie.isurveyplotindex = 1;
                mbna_num_ties_plot++;
              }
            }
            else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
              section->globaltie.isurveyplotindex = 1;
              mbna_num_ties_plot++;
            }
          }
        }
      }

      /* Get min maxes by looping over files to locate each tie in plot */
      plot_index = 0;
      first = true;
      if (mbna_modelplot_tiezoom) {
        mbna_modelplot_tiestart = mbna_modelplot_tiestartzoom;
        mbna_modelplot_tieend = mbna_modelplot_tieendzoom;
      }
      else {
        /* set to plot from 1 to mbna_num_ties_plot inside the bounds */
        mbna_modelplot_tiestart = 0;
        mbna_modelplot_tieend = mbna_num_ties_plot + 1;
      }

      /* scale global ties to be plotted */
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        file->show_in_modelplot = -1;
        for (int jsection = 0; jsection < file->num_sections; jsection++) {
          section = &file->sections[jsection];
          if (section->globaltie.isurveyplotindex == 1) {
            /* set plot index for tie */
            section->globaltie.isurveyplotindex = plot_index;

            /* increment plot_index */
            plot_index++;

            if (first) {
              lon_offset_min = section->globaltie.offset_x_m;
              lon_offset_max = section->globaltie.offset_x_m;
              lat_offset_min = section->globaltie.offset_y_m;
              lat_offset_max = section->globaltie.offset_y_m;
              z_offset_min = section->globaltie.offset_z_m;
              z_offset_max = section->globaltie.offset_z_m;
              first = false;
            }
            else {
              lon_offset_min = MIN(lon_offset_min, section->globaltie.offset_x_m);
              lon_offset_max = MAX(lon_offset_max, section->globaltie.offset_x_m);
              lat_offset_min = MIN(lat_offset_min, section->globaltie.offset_y_m);
              lat_offset_max = MAX(lat_offset_max, section->globaltie.offset_y_m);
              z_offset_min = MIN(z_offset_min, section->globaltie.offset_z_m);
              z_offset_max = MAX(z_offset_max, section->globaltie.offset_z_m);
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
      double yxrange = 1.1 * MAX(lon_offset_max - lon_offset_min, 1.0);
      double yyrange = 1.1 * MAX(lat_offset_max - lat_offset_min, 1.0);
      double yrange = MAX(yxrange, yyrange);
      double yzrange = 1.1 * MAX(z_offset_max - z_offset_min, 0.5);
      mbna_modelplot_yxmid = 0.5 * (lon_offset_max + lon_offset_min);
      mbna_modelplot_yymid = 0.5 * (lat_offset_max + lat_offset_min);
      mbna_modelplot_yzmid = 0.5 * (z_offset_max + z_offset_min);
      mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_tieend - mbna_modelplot_tiestart);
      mbna_modelplot_yscale = ((double)plot_height) / (yrange);
      mbna_modelplot_yzscale = ((double)plot_height) / (yzrange);

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
        snprintf(label, sizeof(label), "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
        snprintf(label, sizeof(label), "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
        snprintf(label, sizeof(label), "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
        snprintf(label, sizeof(label), "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
        snprintf(label, sizeof(label), "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
                mbna_file_select, mbna_section_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
        snprintf(label, sizeof(label), "Display All Data");
      }

      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      /* plot labels */
      snprintf(label, sizeof(label), "Global Tie East-West Offset (meters)");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid + 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid - 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "Global Tie North-South Offset (meters)");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid + 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid - 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "GLobal Tie Vertical Offset (meters)");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid + 0.5 * yzrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid - 0.5 * yzrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      /* set clipping */
      xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

      plot_index = 0;
      for (int ifile = 0; ifile < project.num_files; ifile++) {
        file = &project.files[ifile];
        for (int jsection = 0; jsection < file->num_sections; jsection++) {
          section = &file->sections[jsection];
          if (section->globaltie.isurveyplotindex >= 0) {
            if (section->globaltie.isurveyplotindex >= mbna_modelplot_tiestart &&
                section->globaltie.isurveyplotindex <= mbna_modelplot_tieend) {
              /* plot tie */
              if (section->globaltie.inversion_status == MBNA_INVERSION_CURRENT)
                pixel = pixel_values[mbna_color_foreground];
              else
                pixel = pixel_values[BLUE];

              ix = mbna_modelplot_xo +
                   (int)(mbna_modelplot_xscale * (section->globaltie.isurveyplotindex - mbna_modelplot_tiestart + 1));

              iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (section->globaltie.offset_x_m - mbna_modelplot_yxmid));
              if (ifile == mbna_current_file && jsection == mbna_current_section) {
                xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
              else
                xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

              iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (section->globaltie.offset_y_m - mbna_modelplot_yymid));
              if (ifile == mbna_current_file && jsection == mbna_current_section) {
                xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
              else
                xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

              iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (section->globaltie.offset_z_m - mbna_modelplot_yzmid));
              if (ifile == mbna_current_file && jsection == mbna_current_section) {
                xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                 XG_SOLIDLINE);
              }
              else
                xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);
            }

            /* increment plot_index */
            plot_index++;
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

        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itiestart - mbna_modelplot_tiestart + 1));
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);

        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itieend - mbna_modelplot_tiestart + 1));
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

    /* plot crossing tie offsets */
    else {

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

      /* deal with single block, survey, file, or section */
      if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK
          || mbna_view_mode == MBNA_VIEW_MODE_SURVEY
          || mbna_view_mode == MBNA_VIEW_MODE_FILE) {
        /* loop over all ties looking for ones in current plot block
            - current plot block is for ties between
            surveys isurvey1 and isurvey2 */
        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &project.crossings[i];
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &crossing->ties[j];

            /* check if this tie is between the desired surveys */
            if (tie->isurveyplotindex >= 0) {
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

      /* deal with showing multiple blocks */
      else {
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
      }

      /* get scaling */
      plot_width = mbna_modelplot_width - 8 * MBNA_MODELPLOT_X_SPACE;
      plot_height = (mbna_modelplot_height - 4 * MBNA_MODELPLOT_Y_SPACE) / 3;
      mbna_modelplot_xo = 5 * MBNA_MODELPLOT_X_SPACE;
      mbna_modelplot_yo_lon = MBNA_MODELPLOT_Y_SPACE + plot_height / 2;
      mbna_modelplot_yo_lat = 2 * MBNA_MODELPLOT_Y_SPACE + 3 * plot_height / 2;
      mbna_modelplot_yo_z = 3 * MBNA_MODELPLOT_Y_SPACE + 5 * plot_height / 2;
      double yxrange = 1.1 * MAX(lon_offset_max - lon_offset_min, 1.0);
      double yyrange = 1.1 * MAX(lat_offset_max - lat_offset_min, 1.0);
      double yrange = MAX(yxrange, yyrange);
      double yzrange = 1.1 * MAX(z_offset_max - z_offset_min, 0.5);
      mbna_modelplot_yxmid = 0.5 * (lon_offset_max + lon_offset_min);
      mbna_modelplot_yymid = 0.5 * (lat_offset_max + lat_offset_min);
      mbna_modelplot_yzmid = 0.5 * (z_offset_max + z_offset_min);
      mbna_modelplot_xscale = ((double)plot_width) / (mbna_modelplot_tieend - mbna_modelplot_tiestart);
      mbna_modelplot_yscale = ((double)plot_height) / (yrange);
      mbna_modelplot_yzscale = ((double)plot_height) / (yzrange);
      /*
      xymax = MAX(fabs(lon_offset_min), fabs(lon_offset_max));
      xymax = MAX(fabs(lat_offset_min), xymax);
      xymax = MAX(fabs(lat_offset_max), xymax);
      yzmax = MAX(fabs(z_offset_min), fabs(z_offset_max));
      yzmax = MAX(yzmax, 0.5);
      mbna_modelplot_yscale = ((double)plot_height) / (2.2 * xymax);
      mbna_modelplot_yzscale = ((double)plot_height) / (2.2 * yzmax);
      */

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
        snprintf(label, sizeof(label), "Display Only Selected Survey - Selected Survey:%d", mbna_survey_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_FILE) {
        snprintf(label, sizeof(label), "Display Only Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSURVEY) {
        snprintf(label, sizeof(label), "Display With Selected Survey - Selected Survey:%d", mbna_survey_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHFILE) {
        snprintf(label, sizeof(label), "Display With Selected File - Selected Survey/File:%d/%d", mbna_survey_select, mbna_file_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_WITHSECTION) {
        snprintf(label, sizeof(label), "Display With Selected Section: Selected Survey/File/Section:%d/%d/%d", mbna_survey_select,
                mbna_file_select, mbna_section_select);
      }
      else if (mbna_view_mode == MBNA_VIEW_MODE_ALL) {
        snprintf(label, sizeof(label), "Display All Data");
      }

      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = MBNA_MODELPLOT_Y_SPACE - 2 * stringascent;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      /* plot labels */
      snprintf(label, sizeof(label), "Tie East-West Offset (meters) Grouped by Surveys");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_lon - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid + 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yxmid - 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lon + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "Tie North-South Offset (meters) Grouped by Surveys");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_lat - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid + 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yymid - 0.5 * yrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_lat + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "Tie Vertical Offset (meters) Grouped by Surveys");
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + (plot_width - stringwidth) / 2;
      iy = mbna_modelplot_yo_z - plot_height / 2 - stringascent / 4;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tiestart);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth / 2;
      iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%d", mbna_modelplot_tieend);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo + plot_width - stringwidth / 2;
      iy = mbna_modelplot_yo_z + plot_height / 2 + 3 * stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid + 0.5 * yzrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z - plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      snprintf(label, sizeof(label), "%.2f", mbna_modelplot_yzmid - 0.5 * yzrange);
      xg_justify(pmodp_xgid, label, &stringwidth, &stringascent, &stringdescent);
      ix = mbna_modelplot_xo - stringwidth - stringascent / 4;
      iy = mbna_modelplot_yo_z + plot_height / 2 + stringascent / 2;
      xg_drawstring(pmodp_xgid, ix, iy, label, pixel_values[mbna_color_foreground], XG_SOLIDLINE);

      /* set clipping */
      xg_setclip(pmodp_xgid, mbna_modelplot_xo, 0, plot_width, mbna_modelplot_height);

      /* plot ties from a single block, survey, file, or section */
      if (mbna_view_mode == MBNA_VIEW_MODE_BLOCK
          || mbna_view_mode == MBNA_VIEW_MODE_SURVEY
          || mbna_view_mode == MBNA_VIEW_MODE_FILE) {
        /* loop over all ties looking for ones in current plot block
            - current plot block for a specified survey, a specified file, or for
              ties between two specified surveys */

        plot_index = 0;
        num_ties_block = 0;

        for (int i = 0; i < project.num_crossings; i++) {
          crossing = &project.crossings[i];
          for (int j = 0; j < crossing->num_ties; j++) {
            tie = &crossing->ties[j];

            /* check if this tie is between the desired start and end */
            if (tie->isurveyplotindex >= 0) {
              if (tie->isurveyplotindex >= mbna_modelplot_tiestart &&
                  tie->isurveyplotindex <= mbna_modelplot_tieend) {
                /* plot tie */
                if (tie->inversion_status == MBNA_INVERSION_CURRENT)
                  pixel = pixel_values[mbna_color_foreground];
                else
                  pixel = pixel_values[BLUE];

                ix = mbna_modelplot_xo +
                     (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart + 1));

                iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (tie->offset_x_m - mbna_modelplot_yxmid));
                if (i == mbna_current_crossing && j == mbna_current_tie) {
                  xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                  xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                   XG_SOLIDLINE);
                }
                else
                  xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (tie->offset_y_m - mbna_modelplot_yymid));
                if (i == mbna_current_crossing && j == mbna_current_tie) {
                  xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                  xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                   XG_SOLIDLINE);
                }
                else
                  xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (tie->offset_z_m - mbna_modelplot_yzmid));
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
      }

      /* else loop over survey by survey blocks to locate each tie in plot */
      else {
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
                         (int)(mbna_modelplot_xscale * (tie->isurveyplotindex - mbna_modelplot_tiestart + 1));

                    iy = mbna_modelplot_yo_lon - (int)(mbna_modelplot_yscale * (tie->offset_x_m - mbna_modelplot_yxmid));
                    if (i == mbna_current_crossing && j == mbna_current_tie) {
                      xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                      xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                       XG_SOLIDLINE);
                    }
                    else
                      xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                    iy = mbna_modelplot_yo_lat - (int)(mbna_modelplot_yscale * (tie->offset_y_m - mbna_modelplot_yymid));
                    if (i == mbna_current_crossing && j == mbna_current_tie) {
                      xg_fillrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[RED], XG_SOLIDLINE);
                      xg_drawrectangle(pmodp_xgid, ix - 3, iy - 3, 7, 7, pixel_values[mbna_color_foreground],
                                       XG_SOLIDLINE);
                    }
                    else
                      xg_drawrectangle(pmodp_xgid, ix - 2, iy - 2, 5, 5, pixel, XG_SOLIDLINE);

                    iy = mbna_modelplot_yo_z - (int)(mbna_modelplot_yzscale * (tie->offset_z_m - mbna_modelplot_yzmid));
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
              ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (plot_index - mbna_modelplot_tiestart + 0.5));
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix,
                          mbna_modelplot_yo_lon + plot_height / 2, pixel_values[GREEN], XG_DASHLINE);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix,
                          mbna_modelplot_yo_lat + plot_height / 2, pixel_values[GREEN], XG_DASHLINE);
              xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                          pixel_values[GREEN], XG_DASHLINE);
            }
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

        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itiestart - mbna_modelplot_tiestart + 1));
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lon - plot_height / 2, ix, mbna_modelplot_yo_lon + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_lat - plot_height / 2, ix, mbna_modelplot_yo_lat + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);
        xg_drawline(pmodp_xgid, ix, mbna_modelplot_yo_z - plot_height / 2, ix, mbna_modelplot_yo_z + plot_height / 2,
                    pixel_values[mbna_color_foreground], XG_DASHLINE);

        ix = mbna_modelplot_xo + (int)(mbna_modelplot_xscale * (itieend - mbna_modelplot_tiestart + 1));
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
  mb_pathplus mbv_file_name;
  mb_pathplus mbv_title;
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
  unsigned int *navline = NULL;
  unsigned int *navshot = NULL;
  unsigned int *navcdp = NULL;
  int mbv_navcolor;
  int mbv_navsize;
  mb_path mbv_navname;
  int mbv_navpathstatus;
  mb_pathplus mbv_navpathraw;
  mb_pathplus mbv_navpathprocessed;
  int mbv_navformatorg;
  int mbv_navswathbounds;
  unsigned int mbv_navline;
  unsigned int mbv_navshot;
  unsigned int mbv_navcdp;
  int mbv_navdecimation;

  FILE *nfp;
  struct mbna_file *file;
  struct mbna_section *section;
  int year, month, day, hour, minute;
  double seconds, roll, pitch, heave;
  double sensordepth;
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
    /* visualize grid using data from one survey in project */
    if (which_grid >= 0) {
      snprintf(mbv_file_name, sizeof(mbv_file_name), "%s/ProjectTopoAdj_%4.4d.grd", project.datadir, which_grid);
      snprintf(mbv_title, sizeof(mbv_title), "MBnavadjust: %s - survey %4.4d\n", project.name, which_grid);
    }

    /* else visualize grid using all data in project */
    else { // which_grid == MBNA_GRID_PROJECT == -1
      snprintf(mbv_file_name, sizeof(mbv_file_name), "%s/ProjectTopoAdj.grd", project.datadir);
      snprintf(mbv_title, sizeof(mbv_title), "MBnavadjust: %s\n", project.name);
    }

    /* set parameters */
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
        snprintf(mbv_display_projection_id, sizeof(mbv_display_projection_id), "SPHEROID");
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
        snprintf(mbv_display_projection_id, sizeof(mbv_display_projection_id), "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more northerly than 84 deg N then use
              North Universal Polar Stereographic Projection */
      else if (mbv_primary_ymin > 84.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32661;
        snprintf(mbv_display_projection_id, sizeof(mbv_display_projection_id), "EPSG:%d", projectionid);
      }

      /* else if grid geographic and more southerly than 80 deg S then use
              South Universal Polar Stereographic Projection */
      else if (mbv_primary_ymax < 80.0) {
        mbv_display_projection_mode = MBV_PROJECTION_PROJECTED;
        projectionid = 32761;
        snprintf(mbv_display_projection_id, sizeof(mbv_display_projection_id), "EPSG:%d", projectionid);
      }

      /* else just use geographic */
      else {
        mbv_display_projection_mode = MBV_PROJECTION_GEOGRAPHIC;
        snprintf(mbv_display_projection_id, sizeof(mbv_display_projection_id), "EPSG:%d", GCS_WGS_84);
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
        int num_files_active = 0;
        for (int i = 0; i< project.num_files; i++) {
          bool found = false;
          for (int j = 0; j < project.files[i].num_sections && !found; j++) {
            if (do_check_nav_active(i, j)) {
              found = true;
            }
          }
          if (found) {
            num_files_active++;
          }
        }
        int count_files_active = 0;
        snprintf(message, sizeof(message), "Loading nav %d of %d...", count_files_active + 1, num_files_active);
        do_message_on(message);
        for (int i = 0; i < project.num_files; i++) {
          file = &project.files[i];
          mbv_navformatorg = file->format;
          bool found = false;
          for (int j = 0; j < project.files[i].num_sections; j++) {
            if (do_check_nav_active(i, j)) {
              section = &file->sections[j];
              snprintf(mbv_file_name, sizeof(mbv_file_name), "%s/nvs_%4.4d_%4.4dp.mb71.fnv", project.datadir, file->id, j);
              snprintf(mbv_navname, sizeof(mbv_navname), "%4.4d:%4.4d", file->id, j);
              snprintf(mbv_navpathraw, sizeof(mbv_navpathraw), "%s/nvs_%4.4d_%4.4d.mb71", project.datadir, file->id, j);
              snprintf(mbv_navpathprocessed, sizeof(mbv_navpathprocessed), "%s/nvs_%4.4d_%4.4dp.mb71", project.datadir, file->id, j);

              /* reset message only for first active section in an active file */
              if (!found) {
                count_files_active++;
                snprintf(message, sizeof(message), "Loading nav %d of %d...", count_files_active, num_files_active);
                do_message_on(message);
                found = true;
              }

              mbv_navpings = 0;
              if ((nfp = fopen(mbv_file_name, "r")) == NULL) {
                snprintf(mbv_file_name, sizeof(mbv_file_name), "%s/nvs_%4.4d_%4.4d.mb71.fnv", project.datadir, file->id, j);
                nfp = fopen(mbv_file_name, "r");
              }
              if (nfp != NULL) {
                bool done = false;
                char line[MB_PATH_MAXLINE];
                while (!done) {
                	char *line_ptr = fgets(line, MB_PATH_MAXLINE, nfp);
                	if (line_ptr == NULL) {
                    done = true;
                  }
                  else if (line[0] != '#') {
                    nscan = sscanf(line, "%d %d %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                                         &year, &month, &day, &hour, &minute, &seconds, &navtime_d[mbv_navpings],
                                         &navlon[mbv_navpings], &navlat[mbv_navpings], &navheading[mbv_navpings],
                                         &navspeed[mbv_navpings], &sensordepth, &roll, &pitch, &heave,
                                         &navportlon[mbv_navpings], &navportlat[mbv_navpings],
                                         &navstbdlon[mbv_navpings], &navstbdlat[mbv_navpings]);
                    if (nscan >= 15) {
                      navz[mbv_navpings] = -sensordepth;
                      navline[mbv_navpings] = i;
                      navshot[mbv_navpings] = j;
                      navcdp[mbv_navpings] = mbv_navpings;
                      mbv_navpings++;
                    }
                  }
                }
              }
              fclose(nfp);
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
          tie = (struct mbna_tie *)&crossing->ties[j];
          if (tie->sigma_m >= project.tiessortedthreshold) {
            file_1 = (struct mbna_file *)&project.files[crossing->file_id_1];
            file_2 = (struct mbna_file *)&project.files[crossing->file_id_2];
            section_1 = (struct mbna_section *)&file_1->sections[crossing->section_1];
            section_2 = (struct mbna_section *)&file_2->sections[crossing->section_2];
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
            snprintf(navtiename, sizeof(navtiename), "%4.4d:%1d %2.2d:%4.4d:%2.2d %2.2d:%4.4d:%2.2d", i, j, file_1->block, crossing->file_id_1,
                    crossing->section_1, file_2->block, crossing->file_id_2, crossing->section_2);
            status = mbview_addroute(mbna_verbose, instance, npoint, navtielon, navtielat, waypoint, navtiecolor,
                                     navtiesize, navtieeditmode, navtiename, &id, &error);
          }
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
        if (section->globaltie.status != MBNA_TIE_NONE && do_check_globaltie_listok(i, j)) {
          navtielon[0] =
              section->snav_lon[section->globaltie.snav] + section->snav_lon_offset[section->globaltie.snav];
          navtielat[0] =
              section->snav_lat[section->globaltie.snav] + section->snav_lat_offset[section->globaltie.snav];
          navtielon[1] =
              section->snav_lon[section->globaltie.snav] + section->snav_lon_offset[section->globaltie.snav];
          navtielat[1] =
              section->snav_lat[section->globaltie.snav] + section->snav_lat_offset[section->globaltie.snav];
          snprintf(navtiename, sizeof(navtiename), "%2.2d:%4.4d:%2.2d", file->block, i, j);
          status = mbview_addroute(mbna_verbose, instance, npoint, navtielon, navtielat, waypoint, navtiecolor,
                                   navtiesize, navtieeditmode, navtiename, &id, &error);
        }
      }
    }

//   No longer loading all nav into mbview when loading single survey grid - only
//   load nav for that survey now - DWC 20221113
//    /* set active/inactive status of section navigation */
//    bool updatelist = false;
//    for (int i = 0; i < project.num_files; i++) {
//      file = &project.files[i];
//      for (int j = 0; j < project.files[i].num_sections; j++) {
//        section = &file->sections[j];
//        snprintf(navtiename, sizeof(navtiename), "%4.4d:%4.4d", file->id, j);
//        if (i == project.num_files - 1 && j == project.files[i].num_sections - 1)
//          updatelist = true;
//        mbview_setnavactivebyname(mbna_verbose, instance, navtiename, do_check_nav_active(i, j), updatelist, &error);
//      }
//    }

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
    snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
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
      snprintf(message, sizeof(message), "Loading crossing %d...", mbna_current_crossing);
      do_message_on(message);

      mbnavadjust_crossing_load();

      /* turn off message */
      do_message_off();
    }

    /* else unload previously loaded crossing */
    else if (mbna_naverr_mode != MBNA_NAVERR_MODE_UNLOADED) {
      /* put up message */
      snprintf(message, sizeof(message), "Unloading crossing...");
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
