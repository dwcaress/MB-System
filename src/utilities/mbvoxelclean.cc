/*--------------------------------------------------------------------
 *    The MB-system:  mbvoxelclean.c  8/27/2018
 *
 *    Copyright (c) 2018-2023 by
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
 * mbvoxelclean identifies and flags artifacts in swath bathymetry data
 * using a 3D density filter. The notion applied is that true targets
 * (e.g. the seafloor) result in dense regions of soundings while sparse soundings
 * in the water column or the subsurface are erroneous and can be flagged as bad.
 * This technique is more appropriate for lidar data than multibeam sonar data.
 * The resulting sounding edit events are output to edit save files which can be
 * applied to the data by the program mbprocess. These are the same edit save
 * files created and/or modified by mbvoxelclean and mbedit.
 * The input data are one swath file or a datalist referencing multiple
 * swath files. Each file is read and processed separately.
 * The rectangular prism including all of the flagged and unflagged soundings
 * is divided into 3D voxels of the specified size. All of the soundings are
 * read into memory and associated with one of the voxels. Once all of
 * data are read, a density filter is applied such that containing more than a
 * specified threshold of soundings are considered to be occupied by a valid target and
 * voxels containing less than the threshold are considered to be empty.
 * The user may specify one or both of the following actions:
 *   1) Previously unflagged soundings in an empty voxel are flagged as bad.
 *   2) Previously flagged soundings in a full voxel are unflagged.
 * This program will also apply specified range minimum and maximum filters.
 * If a sounding's flag status is changed, that flagging action is output
 * to the edit save file of the swath file containing that sounding. This
 * program will create edit save files if necessary, or append to those that
 * already exist.
 *
 * Author:  D. W. Caress
 * Date:  August 3, 2018
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

/* ping structure definition */
struct mbvoxelclean_ping_struct {
  int time_i[7];
  double time_d;
  int multiplicity;
  double navlon;
  double navlat;
  double heading;
  double sensordepth;
  int beams_bath;
  int beams_bath_alloc;
  char *beamflag;
  char *beamflagorg;
  double *bathacrosstrack;
  double *bathz;
  double *bathx;
  double *bathy;
  double *bathr;
};

typedef enum {
    MBVC_EMPTY_IGNORE = 0,
    MBVC_EMPTY_FLAG = 1,
} empty_mode_t;
typedef enum {
    MBVC_OCCUPIED_IGNORE = 0,
    MBVC_OCCUPIED_UNFLAG = 1,
} occupied_mode_t;

constexpr char program_name[] = "mbvoxelclean";
constexpr char help_message[] =
    "mbvoxelclean parses recursive datalist files and outputs the\n"
    "complete list of data files and formats.\n"
    "The results are dumped to stdout.";
constexpr char usage_message[] =
    "mbvoxelclean \n"
    "\t[\n"
    "\t--verbose\n"
    "\t--help\n"
    "\t--input=datalist\n"
    "\t--format=value\n"
    "\t--voxel-size=xysize[/zsize]\n"
    "\t--occupy-threshold=value\n"
    "\t--count-flagged\n"
    "\t--flag-empty\n"
    "\t--ignore-empty\n"
    "\t--unflag-occupied\n"
    "\t--ignore-occupied\n"
    "\t--neighborhood=value\n"
    "\t--range-minimum=value\n"
    "\t--range-maximum=value]\n"
    "\t--acrosstrack-minimum=value\n"
    "\t--acrosstrack-maximum=value\n"
    "\t--amplitude-minimum=value\n"
    "\t--amplitude-maximum=value]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int defaultpings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &defaultpings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  /* reset all defaults but the format and lonflip */
  defaultpings = 1;
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

  char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
  double voxel_size_xy = 0.05;
  double voxel_size_z = 0.05;
  int occupy_threshold = 5;
  bool count_flagged = false;
  empty_mode_t empty_mode = MBVC_EMPTY_FLAG;
  occupied_mode_t occupied_mode = MBVC_OCCUPIED_IGNORE;
  int neighborhood = 0;

  /* other mbvoxelclean control parameters */
  bool apply_range_minimum = false;
  double range_minimum = 0.0;
  bool apply_range_maximum = false;
  double range_maximum = 0.0;
  bool apply_acrosstrack_minimum = false;
  double acrosstrack_minimum = 0.0;
  bool apply_acrosstrack_maximum = false;
  double acrosstrack_maximum = 0.0;
  bool apply_amplitude_minimum = false;
  double amplitude_minimum = 0.0;
  bool apply_amplitude_maximum = false;
  double amplitude_maximum = 0.0;

  /* output stream for basic stuff (stdout if verbose <= 1, stderr if verbose > 1) */
  FILE *outfp;

  {
    static struct option options[] = {
        {"verbose", no_argument, nullptr, 0},
        {"help", no_argument, nullptr, 0},
        {"input", required_argument, nullptr, 0},
        {"format", required_argument, nullptr, 0},
        {"voxel-size", required_argument, nullptr, 0},
        {"occupy-threshold", required_argument, nullptr, 0},
        {"count-flagged", required_argument, nullptr, 0},
        {"flag-empty", no_argument, nullptr, 0},
        {"ignore-empty", no_argument, nullptr, 0},
        {"unflag-occupied", no_argument, nullptr, 0},
        {"ignore-occupied", no_argument, nullptr, 0},
        {"neighborhood", required_argument, nullptr, 0},
        {"range-minimum", required_argument, nullptr, 0},
        {"range-maximum", required_argument, nullptr, 0},
        {"acrosstrack-minimum", required_argument, nullptr, 0},
        {"acrosstrack-maximum", required_argument, nullptr, 0},
        {"amplitude-minimum", required_argument, nullptr, 0},
        {"amplitude-maximum", required_argument, nullptr, 0},
        {nullptr, 0, nullptr, 0}};
    int option_index;
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1) {
      switch (c) {
      /* long options */
      case 0:
        if (strcmp("verbose", options[option_index].name) == 0) {
          verbose++;
        }
        else if (strcmp("help", options[option_index].name) == 0) {
          help = true;
        }
        else if (strcmp("input", options[option_index].name) == 0) {
          sscanf(optarg, "%1023s", read_file);
        }
        else if (strcmp("format", options[option_index].name) == 0) {
          sscanf(optarg, "%d", &format);
        }
        else if (strcmp("voxel-size", options[option_index].name) == 0) {
          double d1;
          double d2;
          const int nscan = sscanf(optarg, "%lf/%lf", &d1, &d2);
          if (nscan > 0) {
            voxel_size_xy = d1;
            if (nscan > 1) {
              voxel_size_z = d2;
            } else {
              voxel_size_z = d1;
            }
          }
        }
        else if (strcmp("occupy-threshold", options[option_index].name) == 0) {
          sscanf(optarg, "%d", &occupy_threshold);
        }
        else if (strcmp("count-flagged", options[option_index].name) == 0) {
          count_flagged = true;
        }
        else if (strcmp("flag-empty", options[option_index].name) == 0) {
          empty_mode = MBVC_EMPTY_FLAG;
        }
        else if (strcmp("ignore-empty", options[option_index].name) == 0) {
          empty_mode = MBVC_EMPTY_IGNORE;
        }
        else if (strcmp("unflag-occupied", options[option_index].name) == 0) {
          occupied_mode = MBVC_OCCUPIED_UNFLAG;
        }
        else if (strcmp("ignore-occupied", options[option_index].name) == 0) {
          occupied_mode = MBVC_OCCUPIED_IGNORE;
        }
        else if (strcmp("neighborhood", options[option_index].name) == 0) {
          sscanf(optarg, "%d", &neighborhood);
        }
        else if (strcmp("range-minimum", options[option_index].name) == 0) {
          apply_range_minimum = true;
          sscanf(optarg, "%lf", &range_minimum);
        }
        else if (strcmp("range-maximum", options[option_index].name) == 0) {
          apply_range_maximum = true;
          sscanf(optarg, "%lf", &range_maximum);
        }
        else if (strcmp("acrosstrack-minimum", options[option_index].name) == 0) {
          apply_acrosstrack_minimum = true;
          sscanf(optarg, "%lf", &acrosstrack_minimum);
        }
        else if (strcmp("acrosstrack-maximum", options[option_index].name) == 0) {
          apply_acrosstrack_maximum = true;
          sscanf(optarg, "%lf", &acrosstrack_maximum);
        }
        else if (strcmp("amplitude-minimum", options[option_index].name) == 0) {
          apply_amplitude_minimum = true;
          sscanf(optarg, "%lf", &amplitude_minimum);
        }
        else if (strcmp("amplitude-maximum", options[option_index].name) == 0) {
          apply_amplitude_maximum = true;
          sscanf(optarg, "%lf", &amplitude_maximum);
        }

        break;

      case '?':
        errflg = true;
      }
    }

    if (verbose <= 1)
      outfp = stdout;
    else
      outfp = stderr;

    if (errflg) {
      fprintf(outfp, "usage: %s\n", usage_message);
      fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (verbose == 1 || help) {
      fprintf(outfp, "\nProgram %s\n", program_name);
      fprintf(outfp, "MB-system Version %s\n", MB_VERSION);
    }

    if (verbose >= 2) {
      fprintf(outfp, "\ndbg2  Program <%s>\n", program_name);
      fprintf(outfp, "dbg2  MB-system Version %s\n", MB_VERSION);
      fprintf(outfp, "dbg2  Control Parameters:\n");
      fprintf(outfp, "dbg2       verbose:                     %d\n", verbose);
      fprintf(outfp, "dbg2       help:                        %d\n", help);
      fprintf(outfp, "dbg2       defaultpings:                %d\n", defaultpings);
      fprintf(outfp, "dbg2       lonflip:                     %d\n", lonflip);
      fprintf(outfp, "dbg2       btime_i[0]:                  %d\n", btime_i[0]);
      fprintf(outfp, "dbg2       btime_i[1]:                  %d\n", btime_i[1]);
      fprintf(outfp, "dbg2       btime_i[2]:                  %d\n", btime_i[2]);
      fprintf(outfp, "dbg2       btime_i[3]:                  %d\n", btime_i[3]);
      fprintf(outfp, "dbg2       btime_i[4]:                  %d\n", btime_i[4]);
      fprintf(outfp, "dbg2       btime_i[5]:                  %d\n", btime_i[5]);
      fprintf(outfp, "dbg2       btime_i[6]:                  %d\n", btime_i[6]);
      fprintf(outfp, "dbg2       etime_i[0]:                  %d\n", etime_i[0]);
      fprintf(outfp, "dbg2       etime_i[1]:                  %d\n", etime_i[1]);
      fprintf(outfp, "dbg2       etime_i[2]:                  %d\n", etime_i[2]);
      fprintf(outfp, "dbg2       etime_i[3]:                  %d\n", etime_i[3]);
      fprintf(outfp, "dbg2       etime_i[4]:                  %d\n", etime_i[4]);
      fprintf(outfp, "dbg2       etime_i[5]:                  %d\n", etime_i[5]);
      fprintf(outfp, "dbg2       etime_i[6]:                  %d\n", etime_i[6]);
      fprintf(outfp, "dbg2       speedmin:                    %f\n", speedmin);
      fprintf(outfp, "dbg2       timegap:                     %f\n", timegap);
      fprintf(outfp, "dbg2       file:                        %s\n", read_file);
      fprintf(outfp, "dbg2       format:                      %d\n", format);
      fprintf(outfp, "dbg2       voxel_size_xy:               %f\n", voxel_size_xy);
      fprintf(outfp, "dbg2       voxel_size_z:                %f\n", voxel_size_z);
      fprintf(outfp, "dbg2       occupy_threshold:            %d\n", occupy_threshold);
      fprintf(outfp, "dbg2       empty_mode:                  %d\n", empty_mode);
      fprintf(outfp, "dbg2       occupied_mode:               %d\n", occupied_mode);
      fprintf(outfp, "dbg2       neighborhood:                %d\n", neighborhood);
      fprintf(outfp, "dbg2       apply_range_minimum:         %d\n", apply_range_minimum);
      fprintf(outfp, "dbg2       range_minimum:               %f\n", range_minimum);
      fprintf(outfp, "dbg2       apply_range_maximum:         %d\n", apply_range_maximum);
      fprintf(outfp, "dbg2       range_maximum:               %f\n", range_maximum);
      fprintf(outfp, "dbg2       apply_acrosstrack_minimum:   %d\n", apply_acrosstrack_minimum);
      fprintf(outfp, "dbg2       acrosstrack_minimum:         %f\n", acrosstrack_minimum);
      fprintf(outfp, "dbg2       apply_acrosstrack_maximum:   %d\n", apply_acrosstrack_maximum);
      fprintf(outfp, "dbg2       acrosstrack_maximum:         %f\n", acrosstrack_maximum);
      fprintf(outfp, "dbg2       apply_amplitude_minimum:     %d\n", apply_amplitude_minimum);
      fprintf(outfp, "dbg2       amplitude_minimum:           %f\n", amplitude_minimum);
      fprintf(outfp, "dbg2       apply_amplitude_maximum:     %d\n", apply_amplitude_maximum);
      fprintf(outfp, "dbg2       amplitude_maximum:           %f\n", amplitude_maximum);
    }

    if (help) {
      fprintf(outfp, "\n%s\n", help_message);
      fprintf(outfp, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  int error = MB_ERROR_NO_ERROR;

  bool uselockfiles = true;
  mb_uselockfiles(verbose, &uselockfiles);

  /* get format if required */
  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;
  void *datalist;
  char swathfile[MB_PATH_MAXLINE];
  char dfile[MB_PATH_MAXLINE];
  double file_weight;

  /* open file list */
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_NO;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS;
  } else {
    // else copy single filename to be read
    strcpy(swathfile, read_file);
    read_data = true;
  }

  int kind = MB_DATA_NONE;
  char swathfileread[MB_PATH_MAXLINE];
  int variable_beams;
  int traveltime;
  double btime_d;
  double etime_d;

  /* processing variables */
  int sensorhead = 0;
  int sensorhead_error = MB_ERROR_NO_ERROR;
  int pingsread = 0;
  int time_i[7];
  double time_d = 0.0;
  double navlon = 0.0;
  double navlat = 0.0;
  double speed = 0.0;
  double heading = 0.0;
  double distance = 0.0;
  double altitude = 0.0;
  double sensordepth = 0.0;
  int beams_bath = 0;
  int beams_amp = 0;
  int pixels_ss = 0;
  char *beamflag = nullptr;
  char *beamflagorg = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];

  /* swath data storage */
  struct mbvoxelclean_ping_struct *pings = nullptr;

  /* voxel storage */
  unsigned char *voxel_count = nullptr;  // TODO(schwehr): Make this a bool

  /* save file control variables */
  char esffile[MB_PATH_MAXLINE];
  struct mb_esf_struct esf;

  int n_files_tot = 0;
  int n_pings_tot = 0;
  int n_beams_tot = 0;
  int n_beamflag_null_tot = 0;
  int n_beamflag_good_tot = 0;
  int n_beamflag_flag_tot = 0;
  int n_esf_flag_tot = 0;
  int n_esf_unflag_tot = 0;
  int n_density_flag_tot = 0;
  int n_density_unflag_tot = 0;
  int n_minrange_flag_tot = 0;
  int n_maxrange_flag_tot = 0;
  int n_minacrosstrack_flag_tot = 0;
  int n_maxacrosstrack_flag_tot = 0;
  int n_minamplitude_flag_tot = 0;
  int n_maxamplitude_flag_tot = 0;

  bool esffile_open = false;
  bool locked = false;
  int n_voxel_alloc = 0;
  int npings_alloc = 0;

  /* loop over all files to be read */
  while (read_data) {
    bool oktoprocess = true;
    int beam_flagging;  // TODO(schwehr): Make mb_format_flags take a bool

    /* check format and get format flags */
    if ((status = mb_format_flags(verbose, &format, &variable_beams, &traveltime, &beam_flagging, &error)) != MB_SUCCESS) {
      char *message = nullptr;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_format_flags> regarding input format %d:\n%s\n", format,
        message);
      fprintf(stderr, "\nFile <%s> skipped by program <%s>\n", swathfile, program_name);
      oktoprocess = false;
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }

    /* warn if beam flagging not supported for the current data format */
    if (!beam_flagging) {
      fprintf(stderr, "\nWarning:\nMBIO format %d does not allow flagging of bad bathymetry data.\n", format);
      fprintf(stderr,
        "\nWhen mbprocess applies edits to file:\n\t%s\nthe soundings will be nulled (zeroed) rather than flagged.\n",
        swathfile);
    }

    char lock_date[25] = "";
    // int lock_status = MB_SUCCESS;
    /* try to lock file */
    if (uselockfiles) {
      status = mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);
    } else {
      /* lock_status = */
      int lock_purpose = MBP_LOCK_NONE;
      mb_path lock_program = "";
      mb_path lock_user = "";
      mb_path lock_cpu = "";
      mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);

      /* if locked get lock info */
      if (error == MB_ERROR_FILE_LOCKED) {
        fprintf(stderr, "\nFile %s locked but lock ignored\n", swathfile);
        fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
        fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
        error = MB_ERROR_NO_ERROR;
      }
    }

    /* if locked let the user know file can't be opened */
    if (status == MB_FAILURE) {
      /* if locked get lock info */
      if (error == MB_ERROR_FILE_LOCKED) {
        int lock_purpose = MBP_LOCK_NONE;
        mb_path lock_program = "";
        mb_path lock_user = "";
        mb_path lock_cpu = "";
        /* lock_status = */ mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu,
                   lock_date, &error);

        fprintf(stderr, "\nUnable to open input file:\n");
        fprintf(stderr, "  %s\n", swathfile);
        fprintf(stderr, "File locked by <%s> running <%s>\n", lock_user, lock_program);
        fprintf(stderr, "on cpu <%s> at <%s>\n", lock_cpu, lock_date);
      }

      /* else if unable to create lock file there is a permissions problem */
      else if (error == MB_ERROR_OPEN_FAIL) {
        fprintf(stderr, "Unable to create lock file\n");
        fprintf(stderr, "for intended input file:\n");
        fprintf(stderr, "  %s\n", swathfile);
        fprintf(stderr, "-Likely permissions issue\n");
      }

      /* reset error and status */
      oktoprocess = false;
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }

    /* proceed if file locked and format ok */
    if (oktoprocess) {
      /* check for *inf file, create if necessary, and load metadata */
      int formatread;
      struct mb_info_struct mb_info;
      status = mb_get_info_datalist(verbose, swathfile, &formatread, &mb_info, lonflip, &error);

      /* allocate space to store the bathymetry data */
      if (npings_alloc <= mb_info.nrecords) {
        status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nrecords * sizeof(struct mbvoxelclean_ping_struct),
          (void **)&pings, &error);
        if (error != MB_ERROR_NO_ERROR) {
          char *message = nullptr;
          mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
          fprintf(outfp, "\nMBIO Error allocating pings array:\n%s\n", message);
          fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
          mb_memory_clear(verbose, &error);
          exit(error);
        }
        memset((void *)&pings[npings_alloc], 0, (mb_info.nrecords - npings_alloc) * sizeof(struct mbvoxelclean_ping_struct));
        npings_alloc = mb_info.nrecords;
      }
      for (int i = 0; i<mb_info.nrecords; i++) {
        if (pings[i].beams_bath_alloc < mb_info.nbeams_bath) {
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(char),
                 (void **)&pings[i].beamflag, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(char),
               (void **)&pings[i].beamflagorg, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(double),
               (void **)&pings[i].bathacrosstrack, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(double),
               (void **)&pings[i].bathz, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(double),
               (void **)&pings[i].bathx, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(double),
               (void **)&pings[i].bathy, &error);
          if (error == MB_ERROR_NO_ERROR)
            status &= mb_reallocd(verbose, __FILE__, __LINE__, mb_info.nbeams_bath * sizeof(double),
               (void **)&pings[i].bathr, &error);
          if (error != MB_ERROR_NO_ERROR) {
            char *message = nullptr;
            mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
            fprintf(outfp, "\nMBIO Error allocating data arrays within the ping structure:\n%s\n", message);
            fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
            mb_memory_clear(verbose, &error);
            exit(error);
          }
          pings[i].beams_bath_alloc = mb_info.nbeams_bath;
        }
      }

      /* define local cartesian coordinate system based on first ping navigation and heading */
      double mtodeglon;
      double mtodeglat;
      mb_coor_scale(verbose, mb_info.lat_start, &mtodeglon, &mtodeglat);
      const double headingx = sin(mb_info.heading_start * DTR);
      const double headingy = cos(mb_info.heading_start * DTR);

      /* check for "fast bathymetry" or "fbt" file */
      strcpy(swathfileread, swathfile);
      formatread = format;
      mb_get_fbt(verbose, swathfileread, &formatread, &error);

      /* if verbose output status */
      if (verbose >= 0) {
        fprintf(stderr, "---------------------------------\n");
        fprintf(stderr, "Processing %s...\n\tActually reading %s...\n", swathfile, swathfileread);
      }

      /* initialize reading the input swath sonar file */
      void *mbio_ptr = nullptr;
      if (mb_read_init(verbose, swathfileread, formatread, defaultpings, lonflip, bounds, btime_i, etime_i, speedmin,
               timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) !=
          MB_SUCCESS) {
        char *message = nullptr;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", swathfile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* initialize per-file counting variables */
      int n_pings = 0;
      int n_beams = 0;
      int n_beamflag_null = 0;
      int n_beamflag_good = 0;
      int n_beamflag_flag = 0;
      int n_esf_flag = 0;
      int n_esf_unflag = 0;
      int n_density_flag = 0;
      int n_density_unflag = 0;
      int n_minrange_flag = 0;
      int n_maxrange_flag = 0;
      int n_minacrosstrack_flag = 0;
      int n_maxacrosstrack_flag = 0;
      int n_minamplitude_flag = 0;
      int n_maxamplitude_flag = 0;

      /* allocate memory for mb_get() data arrays */
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
             (void **)&beamflag, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
             (void **)&beamflagorg, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
             (void **)&bath, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
             (void **)&bathacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
             (void **)&bathalongtrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double),
             (void **)&amp, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
             (void **)&ss, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
              (void **)&ssacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double),
              (void **)&ssalongtrack, &error);

      /* if error initializing memory then quit */
      if (error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      void *store_ptr = nullptr;
      double x_min;
      double x_max;
      double y_min;
      double y_max;
      double z_min;
      double z_max;

      /* now deal with old edit save file */
      if (status == MB_SUCCESS) {
        /* reset message */
        fprintf(stderr, "\tOpening edit save file...\n");

        /* handle esf edits */
        status = mb_esf_load(verbose, program_name, swathfile, true, true, esffile, &esf, &error);
        if (status == MB_SUCCESS && esf.esffp != nullptr)
          esffile_open = true;
        if (status == MB_FAILURE && error == MB_ERROR_OPEN_FAIL) {
          esffile_open = false;
          fprintf(stderr, "\nUnable to open new edit save file %s\n", esf.esffile);
        }
        else if (status == MB_FAILURE && error == MB_ERROR_MEMORY_FAIL) {
          esffile_open = false;
          fprintf(stderr, "\nUnable to allocate memory for edits in esf file %s\n", esf.esffile);
        }
        /* reset message */
        if (esf.nedit > 0) {
          fprintf(stderr, "%d old edits sorted...\n", esf.nedit);
        }
      }

      /* read */
      bool done = false;
      bool first = true;
      while (!done) {
        if (verbose > 1)
          fprintf(stderr, "\n");

        /* read next record */
        error = MB_ERROR_NO_ERROR;
        status = mb_get(verbose, mbio_ptr, &kind, &pingsread, time_i, &time_d, &navlon,
            &navlat, &speed, &heading, &distance, &altitude, &sensordepth,
            &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
            bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
            &error);
        if (verbose >= 2) {
          fprintf(stderr, "\ndbg2  current data status:\n");
          fprintf(stderr, "dbg2    kind:     %d\n", kind);
          fprintf(stderr, "dbg2    status:   %d\n", status);
        }
        if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
          /* allocate space for data if needed */
          if (beams_bath > pings[n_pings].beams_bath_alloc) {
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
               (void **)&pings[n_pings].beamflag, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(char),
               (void **)&pings[n_pings].beamflagorg, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
               (void **)&pings[n_pings].bathacrosstrack, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
               (void **)&pings[n_pings].bathz, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
               (void **)&pings[n_pings].bathx, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
               (void **)&pings[n_pings].bathy, &error);
            if (error == MB_ERROR_NO_ERROR)
              status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_bath * sizeof(double),
               (void **)&pings[n_pings].bathr, &error);
            if (error != MB_ERROR_NO_ERROR) {
              char *message = nullptr;
              mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
              fprintf(outfp, "\nMBIO Error allocating data arrays within the ping structure:\n%s\n", message);
              fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
              mb_memory_clear(verbose, &error);
              exit(error);
            }
            pings[n_pings].beams_bath_alloc = beams_bath;
          }

          /* check for ping multiplicity */
          status = mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
          const int sensorhead_status = mb_sensorhead(verbose, mbio_ptr, store_ptr, &sensorhead, &sensorhead_error);
          if (sensorhead_status == MB_SUCCESS) {
            pings[n_pings].multiplicity = sensorhead;
          }
          else if (n_pings > 0 && fabs(pings[n_pings].time_d - pings[n_pings - 1].time_d) < MB_ESF_MAXTIMEDIFF) {
            pings[n_pings].multiplicity = pings[n_pings - 1].multiplicity + 1;
          }
          else {
            pings[n_pings].multiplicity = 0;
          }

          /* save relevant data */
          pings[n_pings].time_d = time_d;
          pings[n_pings].navlon = navlon;
          pings[n_pings].navlat = navlat;
          pings[n_pings].heading = heading;
          pings[n_pings].sensordepth = sensordepth;
          pings[n_pings].beams_bath = beams_bath;
          const double sensorx = (navlon - mb_info.lon_start) / mtodeglon;
          const double sensory = (navlat - mb_info.lat_start) / mtodeglat;
          const double sensorz = -sensordepth;
          for (int j = 0; j < beams_bath; j++) {
            pings[n_pings].beamflag[j] = beamflag[j];
            pings[n_pings].beamflagorg[j] = beamflag[j];
            if (!mb_beam_check_flag_null(beamflag[j])) {
              pings[n_pings].bathacrosstrack[j] = bathacrosstrack[j];
              pings[n_pings].bathx[j] = (navlon - mb_info.lon_start) / mtodeglon +
                       headingy * bathacrosstrack[j] + headingx * bathalongtrack[j];
              pings[n_pings].bathy[j] = (navlat - mb_info.lat_start) / mtodeglat -
                       headingx * bathacrosstrack[j] + headingy * bathalongtrack[j];
              pings[n_pings].bathz[j] = -bath[j];
              pings[n_pings].bathr[j] = sqrt((pings[n_pings].bathx[j] - sensorx)
                    * (pings[n_pings].bathx[j] - sensorx)
                         + (pings[n_pings].bathy[j] - sensory)
                    * (pings[n_pings].bathy[j] - sensory)
                         + (pings[n_pings].bathz[j] - sensorz)
                    * (pings[n_pings].bathz[j] - sensorz));
              if (first) {
                  x_min = pings[n_pings].bathx[j];
                  x_max = pings[n_pings].bathx[j];
                  y_min = pings[n_pings].bathy[j];
                  y_max = pings[n_pings].bathy[j];
                  z_min = pings[n_pings].bathz[j];
                  z_max = pings[n_pings].bathz[j];
                  first = false;
              } else {
                  x_min = std::min(x_min, pings[n_pings].bathx[j]);
                  x_max = std::max(x_max, pings[n_pings].bathx[j]);
                  y_min = std::min(y_min, pings[n_pings].bathy[j]);
                  y_max = std::max(y_max, pings[n_pings].bathy[j]);
                  z_min = std::min(z_min, pings[n_pings].bathz[j]);
                  z_max = std::max(z_max, pings[n_pings].bathz[j]);
              }

              // apply amplitude filter here where amplitude values are available
              // = note that a density unflag setting could undo flags defined here
              if (apply_amplitude_minimum || apply_amplitude_maximum) {
                if (mb_beam_ok(pings[n_pings].beamflag[j])) {
                  if (apply_amplitude_minimum && amp[j] < amplitude_minimum) {
                    pings[n_pings].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    const int action = MBP_EDIT_FILTER;
                    mb_esf_save(verbose, &esf, pings[n_pings].time_d,
                        j + pings[n_pings].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                        action, &error);
                    n_minamplitude_flag++;
                  }
                  if (apply_amplitude_maximum && amp[j] > amplitude_maximum) {
                    pings[n_pings].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    const int action = MBP_EDIT_FILTER;
                    mb_esf_save(verbose, &esf, pings[n_pings].time_d,
                        j + pings[n_pings].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                        action, &error);
                    n_maxamplitude_flag++;
                  }
                }
              }

            } else {
              pings[n_pings].bathacrosstrack[j] = 0.0;
              pings[n_pings].bathx[j] = 0.0;
              pings[n_pings].bathy[j] = 0.0;
              pings[n_pings].bathz[j] = 0.0;
              pings[n_pings].bathr[j] = 0.0;
            }
          }
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  beam locations (ping:beam xxx.xxx yyy.yyy zzz.zzz)\n");
            for (int j = 0; j < pings[n_pings].beams_bath; j++) {
              fprintf(stderr, "dbg2    %d:%3.3d %10.3f %10.3f %10.3f\n",
              n_pings, j, pings[n_pings].bathx[j],
              pings[n_pings].bathy[j], pings[n_pings].bathz[j]);
            }

            fprintf(stderr, "\ndbg2  current voxel bounds:\n");
            fprintf(stderr, "dbg2    x_min: %10.3f m\n", x_min);
            fprintf(stderr, "dbg2    x_max: %10.3f m\n", x_max);
            fprintf(stderr, "dbg2    y_min: %10.3f m\n", y_min);
            fprintf(stderr, "dbg2    y_max: %10.3f m\n", y_max);
            fprintf(stderr, "dbg2    z_min: %10.3f m\n", z_min);
            fprintf(stderr, "dbg2    z_max: %10.3f m\n", z_max);
          }

          /* update counters */
          for (int j = 0; j < pings[n_pings].beams_bath; j++) {
            if (mb_beam_ok(pings[n_pings].beamflag[j]))
                 n_beamflag_good++;
            else if (pings[n_pings].beamflag[j] == MB_FLAG_NULL)
                n_beamflag_null++;
            else
                n_beamflag_flag++;
          }

          /* apply saved edits */
          status &= mb_esf_apply(verbose, &esf, pings[n_pings].time_d, pings[n_pings].multiplicity, pings[n_pings].beams_bath,
                    pings[n_pings].beamflag, &error);

          /* update counters */
          for (int j = 0; j < pings[n_pings].beams_bath; j++) {
            if (pings[n_pings].beamflag[j] != pings[n_pings].beamflagorg[j]) {
              if (mb_beam_ok(pings[n_pings].beamflag[j]))
                n_esf_unflag++;
              else
                n_esf_flag++;
            }
          }
          n_beams += pings[n_pings].beams_bath;
          n_pings++;

        }
        else if (error > MB_ERROR_NO_ERROR) {
          done = true;
        }
      }

      /* close the swath file */
      status = mb_close(verbose, &mbio_ptr, &error);

      /* apply acrosstrack filter to the soundings */
      if (apply_acrosstrack_minimum || apply_acrosstrack_maximum) {
        for (int i = 0; i < n_pings; i++) {
          for (int j = 0; j< pings[i].beams_bath; j++) {
            if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
              if (apply_acrosstrack_minimum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathacrosstrack[j] < acrosstrack_minimum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_minacrosstrack_flag++;
              } else if (apply_acrosstrack_maximum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathacrosstrack[j] > acrosstrack_maximum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_maxacrosstrack_flag++;
              }
            }
          }
        }
      }

      /* apply range filter to the soundings */
      if (apply_range_minimum || apply_range_maximum) {
        for (int i = 0; i < n_pings; i++) {
          for (int j = 0; j< pings[i].beams_bath; j++) {
            if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
              if (apply_range_minimum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathr[j] < range_minimum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_minrange_flag++;
              } else if (apply_range_maximum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathr[j] > range_maximum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_maxrange_flag++;
              }
            }
          }
        }
      }

      // allocate arrays of voxel beam counts - use unsigned char so that beam
      // counts are capped at 255 - ergo the maximum occupied count threshold
      // is 254
      const int n_voxel_x = (x_max - x_min) / voxel_size_xy + 3;
      x_min = x_min - 0.5 * voxel_size_xy;
      x_max = x_min + n_voxel_x * voxel_size_xy;
      const int n_voxel_y = (x_max - y_min) / voxel_size_xy + 3;
      y_min = y_min - 0.5 * voxel_size_xy;
      y_max = y_min + n_voxel_y * voxel_size_xy;
      const int n_voxel_z = (z_max - z_min) / voxel_size_z + 3;
      z_min = z_min - 0.5 * voxel_size_z;
      z_max = z_min + n_voxel_z * voxel_size_z;
      int n_voxel = n_voxel_x * n_voxel_y * n_voxel_z;
      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  final voxel bounds:\n");
        fprintf(stderr, "dbg2    x_min:      %10.3f m\n", x_min);
        fprintf(stderr, "dbg2    x_max:      %10.3f m\n", x_max);
        fprintf(stderr, "dbg2    y_min:      %10.3f m\n", y_min);
        fprintf(stderr, "dbg2    y_max:      %10.3f m\n", y_max);
        fprintf(stderr, "dbg2    z_min:      %10.3f m\n", z_min);
        fprintf(stderr, "dbg2    z_max:      %10.3f m\n", z_max);
        fprintf(stderr, "dbg2    n_voxel_x:  %d\n", n_voxel_x);
        fprintf(stderr, "dbg2    n_voxel_y:  %d\n", n_voxel_y);
        fprintf(stderr, "dbg2    n_voxel_z:  %d\n", n_voxel_z);
        fprintf(stderr, "dbg2    n_voxel:    %d\n", n_voxel);
      }
      if (n_voxel_alloc < n_voxel) {
        status &= mb_reallocd(verbose, __FILE__, __LINE__, (size_t)n_voxel,
             (void **)&voxel_count, &error);
        if (error != MB_ERROR_NO_ERROR) {
          char *message = nullptr;
          mb_error(verbose, MB_ERROR_MEMORY_FAIL, &message);
          fprintf(outfp, "\nMBIO Error allocating voxel counting arrays:\n%s\n", message);
          fprintf(outfp, "\nProgram <%s> Terminated\n", program_name);
          mb_memory_clear(verbose, &error);
          exit(error);
        }
        memset((void *)voxel_count, 0, (size_t)n_voxel);
        n_voxel_alloc = n_voxel;
      }

      /* count the soundings in each voxel */
      for (int i = 0; i < n_pings; i++) {
        for (int j = 0; j< pings[i].beams_bath; j++) {
          if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
            const int ix = (pings[i].bathx[j] - x_min) / voxel_size_xy;
            const int iy = (pings[i].bathy[j] - y_min) / voxel_size_xy;
            const int iz = (pings[i].bathz[j] - z_min) / voxel_size_z;
            const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;
            if ((mb_beam_ok(pings[i].beamflag[j]) || count_flagged) && voxel_count[kk] < 254) {
              voxel_count[kk]++;
            }
          }
        }
      }

      // apply neighborhood to extend occupied region
      if (neighborhood > 0) {

        for (int ix=0; ix<n_voxel_x; ix++) {
          for (int iy=0; iy<n_voxel_y; iy++) {
            for (int iz=0; iz<n_voxel_z; iz++) {
              const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;

              if (voxel_count[kk] >= occupy_threshold && voxel_count[kk] < 255) {
                for (int iix=std::max(ix-neighborhood,0); iix<std::min(ix+neighborhood+1,n_voxel_x); iix++) {
                  for (int iiy=std::max(iy-neighborhood,0); iiy<std::min(iy+neighborhood+1,n_voxel_y); iiy++) {
                    for (int iiz=std::max(iz-neighborhood,0); iiz<std::min(iz+neighborhood+1,n_voxel_z); iiz++) {
                      const int kkk = (iix * n_voxel_y + iiy) * n_voxel_z + iiz;
                      if (voxel_count[kkk] < occupy_threshold) {
                        voxel_count[kkk] = 255;
                      }
                    }
                  }
                }
              }

            }
          }
        }
      }

      // apply threshold to generate binary mask of occupied voxels
      for (int kk = 0; kk < n_voxel; kk++) {
        if (voxel_count[kk] >= occupy_threshold) {
          voxel_count[kk] = 1;
        }
        else if (voxel_count[kk] < occupy_threshold) {
          voxel_count[kk] = 0;
        }
      }

      /* apply density filter to the soundings  */
      if (occupied_mode == MBVC_OCCUPIED_UNFLAG || empty_mode == MBVC_EMPTY_FLAG) {
        for (int i = 0; i < n_pings; i++) {
          for (int j = 0; j < pings[i].beams_bath; j++) {
            if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
              const int ix = (pings[i].bathx[j] - x_min) / voxel_size_xy;
              const int iy = (pings[i].bathy[j] - y_min) / voxel_size_xy;
              const int iz = (pings[i].bathz[j] - z_min) / voxel_size_z;
              const int kk = (ix * n_voxel_y + iy) * n_voxel_z + iz;
              if (occupied_mode == MBVC_OCCUPIED_UNFLAG
                && voxel_count[kk]
                && !mb_beam_ok(pings[i].beamflag[j])) {
                pings[i].beamflag[j] = MB_FLAG_NONE;
                const int action = MBP_EDIT_UNFLAG;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_density_unflag++;
              }
              if (empty_mode == MBVC_EMPTY_FLAG
                && !voxel_count[kk]
                && mb_beam_ok(pings[i].beamflag[j])) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                      j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                      action, &error);
                n_density_flag++;
              }
            }
          }
        }
      }

      /* apply acrosstrack filter to the soundings */
      if (apply_acrosstrack_minimum || apply_acrosstrack_maximum) {
        for (int i = 0; i < n_pings; i++) {
          for (int j = 0; j< pings[i].beams_bath; j++) {
            if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
              if (apply_acrosstrack_minimum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathacrosstrack[j] < acrosstrack_minimum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_minacrosstrack_flag++;
              } else if (apply_acrosstrack_maximum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathacrosstrack[j] > acrosstrack_maximum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_maxacrosstrack_flag++;
              }
            }
          }
        }
      }

      /* apply range filter to the soundings */
      if (apply_range_minimum || apply_range_maximum) {
        for (int i = 0; i < n_pings; i++) {
          for (int j = 0; j< pings[i].beams_bath; j++) {
            if (!mb_beam_check_flag_null(pings[i].beamflag[j])) {
              if (apply_range_minimum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathr[j] < range_minimum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_minrange_flag++;
              } else if (apply_range_maximum
                && mb_beam_ok(pings[i].beamflag[j])
                && pings[i].bathr[j] > range_maximum) {
                pings[i].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                const int action = MBP_EDIT_FILTER;
                mb_esf_save(verbose, &esf, pings[i].time_d,
                    j + pings[i].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                    action, &error);
                n_maxrange_flag++;
              }
            }
          }
        }
      }

      /* close edit save file */
      status = mb_esf_close(verbose, &esf, &error);

      /* update mbprocess parameter file */
      if (esffile_open) {
        /* update mbprocess parameter file */
        status = mb_pr_update_format(verbose, swathfile, true, format, &error);
        status = mb_pr_update_edit(verbose, swathfile, MBP_EDIT_ON, esffile, &error);
      }

      /* unlock the raw swath file */
      if (uselockfiles)
        status = mb_pr_unlockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);

      /* check memory */
      if (verbose >= 4)
        status = mb_memory_list(verbose, &error);

      /* increment the total counting variables */
      n_files_tot++;
      n_pings_tot += n_pings;
      n_beams_tot += n_beams;
      n_beamflag_null_tot += n_beamflag_null;
      n_beamflag_good_tot += n_beamflag_good;
      n_beamflag_flag_tot += n_beamflag_flag;
      n_esf_flag_tot += n_esf_flag;
      n_esf_unflag_tot += n_esf_unflag;
      n_density_flag_tot += n_density_flag;
      n_density_unflag_tot += n_density_unflag;
      n_minrange_flag_tot += n_minrange_flag;
      n_maxrange_flag_tot += n_maxrange_flag;
      n_minacrosstrack_flag_tot += n_minacrosstrack_flag;
      n_maxacrosstrack_flag_tot += n_maxacrosstrack_flag;
      n_minamplitude_flag_tot += n_minamplitude_flag;
      n_maxamplitude_flag_tot += n_maxamplitude_flag;

      /* give the statistics */
      if (verbose >= 1) {
        fprintf(stderr, "%7d survey data records processed\n", n_pings);
        fprintf(stderr, "%7d soundings processed\n", n_beams);
        fprintf(stderr, "%7d beams good originally\n", n_beamflag_good);
        fprintf(stderr, "%7d beams flagged originally\n", n_beamflag_flag);
        fprintf(stderr, "%7d beams null originally\n", n_beamflag_null);
        if (esf.nedit > 0) {
          fprintf(stderr, "%7d beams flagged in old esf file\n", n_esf_flag);
          fprintf(stderr, "%7d beams unflagged in old esf file\n", n_esf_unflag);
        }
        fprintf(stderr, "%7d beams flagged by density filter\n", n_density_flag);
        fprintf(stderr, "%7d beams unflagged by density filter\n", n_density_unflag);
        fprintf(stderr, "%7d beams flagged by minimum range filter\n", n_minrange_flag);
        fprintf(stderr, "%7d beams flagged by maximum range filter\n", n_maxrange_flag);
        fprintf(stderr, "%7d beams flagged by minimum acrosstrack filter\n", n_minacrosstrack_flag);
        fprintf(stderr, "%7d beams flagged by maximum acrosstrack filter\n", n_maxacrosstrack_flag);
        fprintf(stderr, "%7d beams flagged by minimum amplitude filter\n", n_minamplitude_flag);
        fprintf(stderr, "%7d beams flagged by maximum amplitude filter\n", n_maxamplitude_flag);
      }
    }

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = (mb_datalist_read(verbose, datalist, swathfile, dfile, &format, &file_weight, &error) == MB_SUCCESS);
    }
    else {
      read_data = false;
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* give the total statistics */
  if (verbose >= 0) {
    fprintf(stderr, "\n---------------------------------\n");
    fprintf(stderr, "MBvoxelclean Processing Totals:\n");
    fprintf(stderr, "---------------------------------\n");
    fprintf(stderr, "%d total swath data files processed\n", n_files_tot);
    fprintf(stderr, "%d total survey data records processed\n", n_pings_tot);
    fprintf(stderr, "%d total soundings processed\n", n_beams_tot);
    fprintf(stderr, "%d total beams good originally\n", n_beamflag_good_tot);
    fprintf(stderr, "%d total beams flagged originally\n", n_beamflag_flag_tot);
    fprintf(stderr, "%d total beams null originally\n", n_beamflag_null_tot);
    fprintf(stderr, "%d total beams flagged in old esf file\n", n_esf_flag_tot);
    fprintf(stderr, "%d total beams unflagged in old esf file\n", n_esf_unflag_tot);
    fprintf(stderr, "%d total beams flagged by density filter\n", n_density_flag_tot);
    fprintf(stderr, "%d total beams unflagged by density filter\n", n_density_unflag_tot);
    fprintf(stderr, "%d total beams flagged by minimum range filter\n", n_minrange_flag_tot);
    fprintf(stderr, "%d total beams flagged by maximum range filter\n", n_maxrange_flag_tot);
    fprintf(stderr, "%d total beams flagged by minimum acrosstrack filter\n", n_minacrosstrack_flag_tot);
    fprintf(stderr, "%d total beams flagged by maximum acrosstrack filter\n", n_maxacrosstrack_flag_tot);
    fprintf(stderr, "%d total beams flagged by minimum amplitude filter\n", n_minamplitude_flag_tot);
    fprintf(stderr, "%d total beams flagged by maximum amplitude filter\n", n_maxamplitude_flag_tot);
  }

  for (int i = 0; i<npings_alloc; i++) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].beamflag, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].beamflagorg, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathacrosstrack, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathz, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathx, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathy, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings[i].bathr, &error);
    pings[i].beams_bath_alloc = 0;
  }
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&pings, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&voxel_count, &error);

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
