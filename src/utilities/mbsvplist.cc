/*--------------------------------------------------------------------
 *    The MB-system:  mbsvplist.c  1/3/2001
 *
 *    Copyright (c) 2001-2023 by
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
 * This program, mbsvplist, lists all water sound velocity
 * profiles (SVPs) within swath data files. Swath bathymetry is
 * calculated from raw angles and travel times by raytracing
 * through a model of the speed of sound in water. Many swath
 * data formats allow SVPs to be embedded in the data, and
 * often the SVPs used to calculate the data will be included.
 * By default, all unique SVPs encountered are listed to
 * stdout. The SVPs may instead be written to individual files
 * with names FILE_XXX.svp, where FILE is the swath data
 * filename and XXX is the SVP count within the file.  The -D
 * option causes duplicate SVPs to be output. The -P option
 * implies -O, and also causes the parameter file to be modified
 * so that the first svp output for each file becomes the
 * svp used for recalculating bathymetry for that swath file.
 *
 * Author:  D. W. Caress
 * Date:  January 3,  2001
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_status.h"
#include "mb_format.h"
#include "mb_define.h"
#include "mb_process.h"

constexpr int MBSVPLIST_SVP_NUM_ALLOC = 24;
typedef enum {
    MBSVPLIST_PRINTMODE_CHANGE = 0,
    MBSVPLIST_PRINTMODE_UNIQUE = 1,
    MBSVPLIST_PRINTMODE_ALL = 2,
} printmode_t;

struct mbsvplist_svp_struct {
  bool time_set;        /* time stamp known */
  bool position_set;    /* position known */
  bool repeat_in_file;  /* repeats a previous svp in the same file */
  bool match_last;      /* repeats the last svp in the same file or the previous file */
  bool depthzero_reset; /* uppermost SVP value set to zero depth */
  double time_d;
  double longitude;
  double latitude;
  double depthzero;
  int n;
  double depth[MB_SVP_MAX];
  double velocity[MB_SVP_MAX];
};

constexpr char program_name[] = "mbsvplist";
constexpr char help_message[] =
    "mbsvplist lists all water sound velocity\n"
    "profiles (SVPs) within swath data files. Swath bathymetry is\n"
    "calculated from raw angles and travel times by raytracing\n"
    "through a model of the speed of sound in water. Many swath\n"
    "data formats allow SVPs to be embedded in the data, and\n"
    "often the SVPs used to calculate the data will be included.\n"
    "By default, all unique SVPs encountered are listed to\n"
    "stdout. The SVPs may instead be written to individual files\n"
    "with names FILE_XXX.svp, where FILE is the swath data\n"
    "filename and XXX is the SVP count within the file. The -D\n"
    "option causes duplicate SVPs to be output.\n"
    "The -T option will output a CSV table of svp#, time, longitude, latitude and number of points for SVPs.\n"
    "When the -Nmin_num_pairs option is used, only svps that have at least min_num_pairs svp values will "
    "be output.(This is particularly useful for .xse data where the svp is entered as a single values svp.)";
constexpr char usage_message[] =
    "mbsvplist [-Asource -C -D -Fformat -H -Ifile -Mmode -O -Nmin_num_pairs -P -T -V -Z]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
  pings = 1;
  bounds[0] = -360.0;
  bounds[1] = 360.0;
  bounds[2] = -90.0;
  bounds[3] = 90.0;

  printmode_t svp_printmode = MBSVPLIST_PRINTMODE_CHANGE;
  bool output_counts = false;
  bool ssv_output = false;
  char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
  int min_num_pairs = 0;
  bool svp_file_output = false;
  bool svp_setprocess = false;
  double ssv_bounds[4] = {-360.0, 360.0, -90.0, 90.0};
  bool ssv_bounds_set = false;
  bool output_as_table = false;
  bool svp_force_zero = false;
  int svp_source_use = -1;

  {
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt(argc, argv, "A:a:CcDdF:f:I:i:M:m:N:n:OoPpR:r:SsTtZzVvHh")) != -1)
      switch (c) {
      case 'H':
      case 'h':
        help = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        break;
      case 'A':
      case 'a':
        if (optarg[0] == 'C' || optarg[0] == 'c') {
          svp_source_use = MB_DATA_CTD;
        } else if (optarg[0] == 'S' || optarg[0] == 's') {
          svp_source_use = MB_DATA_VELOCITY_PROFILE;
        } else {
          sscanf(optarg, "%d", &svp_source_use);
        }
        break;
      case 'D':
      case 'd':
        svp_printmode = MBSVPLIST_PRINTMODE_ALL;
        break;
      case 'C':
      case 'c':
        output_counts = true;
        ssv_output = false;
        break;
      case 'F':
      case 'f':
        sscanf(optarg, "%d", &format);
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", read_file);
        break;
      case 'M':
      case 'm':
      {
        int tmp;
        sscanf(optarg, "%d", &tmp);
        svp_printmode = (printmode_t)tmp;
        break;
      }
      case 'N':
      case 'n':
        sscanf(optarg, "%d", &min_num_pairs);
        break;
      case 'O':
      case 'o':
        svp_file_output = true;
        ssv_output = false;
        break;
      case 'P':
      case 'p':
        svp_file_output = true;
        svp_setprocess = true;
        ssv_output = false;
        break;
      case 'R':
      case 'r':
        mb_get_bounds(optarg, ssv_bounds);
        ssv_bounds_set = true;
        break;
      case 'S':
      case 's':
        ssv_output = true;
        svp_file_output = false;
        svp_setprocess = false;
        break;
      case 'T':
      case 't':
        output_as_table = true;
        ssv_output = false;
        break;
      case 'Z':
      case 'z':
        svp_force_zero = true;
        break;
      case '?':
        errflg = true;
      }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }

    if (verbose == 1 || help) {
      fprintf(stderr, "\nProgram %s\n", program_name);
      fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
    }

    if (verbose >= 2) {
      fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
      fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
      fprintf(stderr, "dbg2  Control Parameters:\n");
      fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
      fprintf(stderr, "dbg2       help:              %d\n", help);
      fprintf(stderr, "dbg2       format:            %d\n", format);
      fprintf(stderr, "dbg2       pings:             %d\n", pings);
      fprintf(stderr, "dbg2       lonflip:           %d\n", lonflip);
      fprintf(stderr, "dbg2       bounds[0]:         %f\n", bounds[0]);
      fprintf(stderr, "dbg2       bounds[1]:         %f\n", bounds[1]);
      fprintf(stderr, "dbg2       bounds[2]:         %f\n", bounds[2]);
      fprintf(stderr, "dbg2       bounds[3]:         %f\n", bounds[3]);
      fprintf(stderr, "dbg2       btime_i[0]:        %d\n", btime_i[0]);
      fprintf(stderr, "dbg2       btime_i[1]:        %d\n", btime_i[1]);
      fprintf(stderr, "dbg2       btime_i[2]:        %d\n", btime_i[2]);
      fprintf(stderr, "dbg2       btime_i[3]:        %d\n", btime_i[3]);
      fprintf(stderr, "dbg2       btime_i[4]:        %d\n", btime_i[4]);
      fprintf(stderr, "dbg2       btime_i[5]:        %d\n", btime_i[5]);
      fprintf(stderr, "dbg2       btime_i[6]:        %d\n", btime_i[6]);
      fprintf(stderr, "dbg2       etime_i[0]:        %d\n", etime_i[0]);
      fprintf(stderr, "dbg2       etime_i[1]:        %d\n", etime_i[1]);
      fprintf(stderr, "dbg2       etime_i[2]:        %d\n", etime_i[2]);
      fprintf(stderr, "dbg2       etime_i[3]:        %d\n", etime_i[3]);
      fprintf(stderr, "dbg2       etime_i[4]:        %d\n", etime_i[4]);
      fprintf(stderr, "dbg2       etime_i[5]:        %d\n", etime_i[5]);
      fprintf(stderr, "dbg2       etime_i[6]:        %d\n", etime_i[6]);
      fprintf(stderr, "dbg2       speedmin:          %f\n", speedmin);
      fprintf(stderr, "dbg2       timegap:           %f\n", timegap);
      fprintf(stderr, "dbg2       read_file:         %s\n", read_file);
      fprintf(stderr, "dbg2       svp_source_use:    %d\n", svp_source_use);
      fprintf(stderr, "dbg2       svp_printmode:     %d\n", svp_printmode);
      fprintf(stderr, "dbg2       svp_file_output:   %d\n", svp_file_output);
      fprintf(stderr, "dbg2       svp_setprocess:    %d\n", svp_setprocess);
      fprintf(stderr, "dbg2       svp_force_zero:    %d\n", svp_force_zero);
      fprintf(stderr, "dbg2       ssv_output:        %d\n", ssv_output);
      fprintf(stderr, "dbg2       ssv_bounds_set:    %d\n", ssv_bounds_set);
      fprintf(stderr, "dbg2       ssv_bounds[0]:     %f\n", ssv_bounds[0]);
      fprintf(stderr, "dbg2       ssv_bounds[1]:     %f\n", ssv_bounds[1]);
      fprintf(stderr, "dbg2       ssv_bounds[2]:     %f\n", ssv_bounds[2]);
      fprintf(stderr, "dbg2       ssv_bounds[3]:     %f\n", ssv_bounds[3]);
    }

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  int error = MB_ERROR_NO_ERROR;

  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;
  void *datalist;
  char file[MB_PATH_MAXLINE];
  char dfile[MB_PATH_MAXLINE];
  double file_weight;

  /* open file list */
  if (read_datalist) {
    const int look_processed = MB_DATALIST_LOOK_UNSET;
    if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
      fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
    read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
  } else {
    /* else copy single filename to be read */
    strcpy(file, read_file);
    read_data = true;
  }

  /* MBIO read control parameters */
  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* MBIO read values */
  void *mbio_ptr = nullptr;
  void *store_ptr;
  int kind;
  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];

  /* save time stamp and position of last survey data */
  double last_time_d = 0.0;
  double last_navlon = 0.0;
  double last_navlat = 0.0;

  /* data record source types */
  int platform_source;
  int nav_source;
  int sensordepth_source;
  int heading_source;
  int attitude_source;
  int svp_source;

  /* output mode settings */

  /* SVP values */
  struct mbsvplist_svp_struct svp;
  struct mbsvplist_svp_struct svp_last;
  svp_last.n = 0;
  int svp_save_alloc = 0;
  struct mbsvplist_svp_struct *svp_save = nullptr;
  mb_pathplus svp_file;
  int svp_read_tot = 0;
  int svp_written_tot = 0;
  int svp_repeat_in_file;
  int out_cnt = 0;
  int svp_time_i[7];

  /* ttimes values */
  int nbeams;
  double *ttimes = nullptr;
  double *angles = nullptr;
  double *angles_forward = nullptr;
  double *angles_null = nullptr;
  double *heave = nullptr;
  double *alongtrack_offset = nullptr;
  double ssv;

  bool svp_match_last = false;
  int svp_unique_tot = 0;

  /* loop over all files to be read */
  while (read_data) {
    /* check format and get data sources */
    if ((status = mb_format_source(verbose, &format, &platform_source, &nav_source, &sensordepth_source, &heading_source,
                                   &attitude_source, &svp_source, &error)) == MB_FAILURE) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_format_source>:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* if svp source record type has been specified, override the default svp_source for this format */
    if (svp_source_use >= 0) {
      svp_source = svp_source_use;
    }

    /* initialize reading the swath file */
    if (mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mbio_ptr,
                               &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* allocate memory for data arrays */
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ttimes, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_forward, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&angles_null, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&heave, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&alongtrack_offset, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* output info */
    if (verbose >= 1) {
      if (ssv_output)
        fprintf(stderr, "\nSearching %s for SSV records\n", file);
      else
        fprintf(stderr, "\nSearching %s for SVP records\n", file);
    }

    /* read and print data */
    bool svp_loaded = false;
    svp.n = 0;
    int svp_save_count = 0;
    int svp_read = 0;
    int svp_written = 0;
    int svp_unique = 0;
    while (error <= MB_ERROR_NO_ERROR) {
      /* read a data record */
      status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }

      /* if svp then extract data */
      if (error <= MB_ERROR_NO_ERROR && kind == svp_source && svp_source != MB_DATA_NONE) {
        /* extract svp */
        status = mb_extract_svp(verbose, mbio_ptr, store_ptr, &kind, &svp.n, svp.depth, svp.velocity, &error);
        if (status == MB_SUCCESS) {
          svp_read++;
          svp_loaded = true;
          svp.match_last = false;
          svp.repeat_in_file = false;
          if (last_time_d != 0.0) {
            svp.time_set = true;
            svp.time_d = last_time_d;
          }
          else {
            svp.time_set = false;
            svp.time_d = 0.0;
          }
          if (navlon != 0.0 || navlat != 0.0) {
            svp.position_set = true;
            svp.longitude = navlon;
            svp.latitude = navlat;
          }
          else if (last_navlon != 0.0 || last_navlat != 0.0) {
            svp.position_set = true;
            svp.longitude = last_navlon;
            svp.latitude = last_navlat;
          }
          else {
            svp.position_set = false;
            svp.longitude = 0.0;
            svp.latitude = 0.0;
          }
          svp.depthzero_reset = false;
          svp.depthzero = 0.0;
        }
        else {
          svp_loaded = false;
        }

        /* force zero depth if requested */
        if (svp_loaded && svp.n > 0 && svp_force_zero && svp.depth[0] != 0.0) {
          svp.depthzero = svp.depth[0];
          svp.depth[0] = 0.0;
          svp.depthzero_reset = true;
        }

        /* check if the svp is a duplicate to a previous svp
            in the same file */
        if (svp_loaded) {
          svp_match_last = false;
          for (int j = 0; j < svp_save_count && svp_match_last; j++) {
            if (svp.n == svp_save[j].n && memcmp(svp.depth, svp_save[j].depth, svp.n) == 0 &&
                memcmp(svp.velocity, svp_save[j].velocity, svp.n) == 0) {
              svp_match_last = true;
            }
          }
          svp.match_last = svp_match_last;
        // }

        /* check if the svp is a duplicate to the previous svp
            whether from the same file or a previous file */
        // if (svp_loaded) {
          /* check if svp is the same as the previous */
          if (svp.n == svp_last.n && memcmp(svp.depth, svp_last.depth, svp.n) == 0 &&
              memcmp(svp.velocity, svp_last.velocity, svp.n) == 0) {
            svp_repeat_in_file = true;
          }
          else {
            svp_repeat_in_file = false;
          }
          svp.repeat_in_file = svp_repeat_in_file;

          /* save the svp */
          svp_last.time_set = false;
          svp_last.position_set = false;
          svp_last.n = svp.n;
          for (int i = 0; i < svp.n; i++) {
            svp_last.depth[i] = svp.depth[i];
            svp_last.velocity[i] = svp.velocity[i];
          }
        }

        /* if the svp is unique so far, save it in memory */
        if (svp_loaded && !svp_match_last && svp.n >= min_num_pairs) {
          /* allocate memory as needed */
          if (svp_save_count >= svp_save_alloc) {
            svp_save_alloc += MBSVPLIST_SVP_NUM_ALLOC;
            status = mb_reallocd(verbose, __FILE__, __LINE__, svp_save_alloc * sizeof(struct mbsvplist_svp_struct),
                                 (void **)&svp_save, &error);
          }

          /* save the svp */
          svp_save[svp_save_count].time_set = svp.time_set;
          svp_save[svp_save_count].position_set = svp.position_set;
          svp_save[svp_save_count].repeat_in_file = svp.repeat_in_file;
          svp_save[svp_save_count].match_last = svp.match_last;
          svp_save[svp_save_count].time_d = svp.time_d;
          svp_save[svp_save_count].longitude = svp.longitude;
          svp_save[svp_save_count].latitude = svp.latitude;
          svp_save[svp_save_count].n = svp.n;
          for (int i = 0; i < svp.n; i++) {
            svp_save[svp_save_count].depth[i] = svp.depth[i];
            svp_save[svp_save_count].velocity[i] = svp.velocity[i];
          }
          svp_save_count++;
          svp_unique++;
        }
      }

      /* else if survey data save most recent ping time
          and if ssv output desired call mb_ttimes() and output ssv */
      else if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
        /* save most recent survey time stamp and position */
        last_time_d = time_d;
        last_navlon = navlon;
        last_navlat = navlat;

        /* check if any saved svps need time tags and position */
        if (time_d != 0.0 && (navlon != 0.0 || navlat != 0.0)) {
          for (int isvp = 0; isvp < svp_save_count; isvp++) {
            if (!svp_save[isvp].time_set) {
              svp_save[isvp].time_set = true;
              svp_save[isvp].time_d = time_d;
            }
            if (!svp_save[isvp].position_set) {
              svp_save[isvp].position_set = true;
              svp_save[isvp].longitude = navlon;
              svp_save[isvp].latitude = navlat;
            }
          }
        }

        /* if desired output ssv_output */
        if (ssv_output) {
          /* extract ttimes */
          status = mb_ttimes(verbose, mbio_ptr, store_ptr, &kind, &nbeams, ttimes, angles, angles_forward, angles_null,
                             heave, alongtrack_offset, &sensordepth, &ssv, &error);

          /* output ssv */
          if (status == MB_SUCCESS) {
            if (!ssv_bounds_set || (navlon >= ssv_bounds[0] && navlon <= ssv_bounds[1] &&
                                            navlat >= ssv_bounds[2] && navlat <= ssv_bounds[3]))
              fprintf(stdout, "%f %f\n", sensordepth, ssv);
          }
        }
      }
    }

    status &= mb_close(verbose, &mbio_ptr, &error);

    /* output svps from this file if there are any and ssv_output and output_counts are false */
    if (svp_save_count > 0 && !ssv_output && !output_counts) {
      for (int isvp = 0; isvp < svp_save_count; isvp++) {
        if (svp_save[isvp].n >= min_num_pairs &&
            ((svp_printmode == MBSVPLIST_PRINTMODE_CHANGE &&
              (svp_written == 0 || !svp_save[isvp].repeat_in_file)) ||
             (svp_printmode == MBSVPLIST_PRINTMODE_UNIQUE && !svp_save[isvp].match_last) ||
             (svp_printmode == MBSVPLIST_PRINTMODE_ALL))) {
          /* set the output */
          FILE *svp_fp = stdout;
          if (svp_file_output) {
            snprintf(svp_file, sizeof(svp_file), "%s_%3.3d.svp", file, isvp);
            svp_fp = fopen(svp_file, "w");
          }

          /* get time as date */
          mb_get_date(verbose, svp_save[isvp].time_d, svp_time_i);

          /* print out the svp */
          if (output_as_table) /* output csv table to stdout */
          {
            if (out_cnt == 0) /* output header records */
            {
              printf("#mbsvplist CSV table output\n#navigation information is "
                     "approximate\n#SVP_cnt,date_time,longitude,latitude,num_data_points\n");
            }
            out_cnt++;
            printf("%d,%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d,%.6f,%.6f,%d\n", out_cnt, svp_time_i[0],
                   svp_time_i[1], svp_time_i[2], svp_time_i[3], svp_time_i[4], svp_time_i[5], svp_time_i[6],
                   svp_save[isvp].longitude, svp_save[isvp].latitude, svp_save[isvp].n);
          }
          else if (svp_fp != nullptr) {
            /* output info */
            if (verbose >= 1) {
              fprintf(stderr, "Outputting SVP to file: %s (# svp pairs=%d)\n", svp_file, svp_save[isvp].n);
            }

            /* write it out */
            fprintf(svp_fp, "## MB-SVP %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %.9f %.9f\n", svp_time_i[0],
                    svp_time_i[1], svp_time_i[2], svp_time_i[3], svp_time_i[4], svp_time_i[5], svp_time_i[6],
                    svp_save[isvp].longitude, svp_save[isvp].latitude);
            fprintf(svp_fp, "## Water Sound Velocity Profile (SVP)\n");
            fprintf(svp_fp, "## Output by Program %s\n", program_name);
            fprintf(svp_fp, "## MB-System Version %s\n", MB_VERSION);
            char user[256], host[256], date[32];
            status = mb_user_host_date(verbose, user, host, date, &error);
            fprintf(svp_fp, "## Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
            fprintf(svp_fp, "## Swath File: %s\n", file);
            fprintf(svp_fp, "## Start Time: %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", svp_time_i[0],
                    svp_time_i[1], svp_time_i[2], svp_time_i[3], svp_time_i[4], svp_time_i[5], svp_time_i[6]);
            fprintf(svp_fp, "## SVP Longitude: %f\n", svp_save[isvp].longitude);
            fprintf(svp_fp, "## SVP Latitude:  %f\n", svp_save[isvp].latitude);
            fprintf(svp_fp, "## SVP Count: %d\n", svp_save_count);
            if (svp_save[isvp].depthzero_reset) {
              fprintf(svp_fp, "## Initial depth reset from %f to 0.0 meters\n", svp_save[isvp].depthzero);
            }
            if (verbose >= 1 && svp_save[isvp].depthzero_reset) {
              fprintf(stderr, "Initial depth reset from %f to 0.0 meters\n", svp_save[isvp].depthzero);
            }
            fprintf(svp_fp, "## Number of SVP Points: %d\n", svp_save[isvp].n);
            for (int i = 0; i < svp_save[isvp].n; i++)
              fprintf(svp_fp, "%8.2f\t%7.2f\n", svp_save[isvp].depth[i], svp_save[isvp].velocity[i]);
            if (!svp_file_output) {
              fprintf(svp_fp, "## \n");
              fprintf(svp_fp, "## \n");
            }
            svp_written++;
          }

          /* close the svp file */
          if (svp_file_output && svp_fp != nullptr) {
            fclose(svp_fp);

            /* if desired, set first svp output to be used for recalculating
                bathymetry */
            if (svp_setprocess && svp_save_count == 1) {
              status = mb_pr_update_svp(verbose, file, true, svp_file, MBP_ANGLES_OK, true, &error);
            }
          }
        }
      }
    }

    /* update total counts */
    svp_read_tot += svp_read;
    svp_unique_tot += svp_unique;
    svp_written_tot += svp_written;

    /* output info */
    if (verbose >= 1) {
      fprintf(stderr, "%d SVP records read\n", svp_read);
      fprintf(stderr, "%d SVP unique records read\n", svp_unique);
      fprintf(stderr, "%d SVP records written\n", svp_written);
    }

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
    } else {
      read_data = false;
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* output info */
  if (verbose >= 1) {
    fprintf(stderr, "\nTotal %d SVP records read\n", svp_read_tot);
    fprintf(stderr, "Total %d SVP unique records found\n", svp_unique_tot);
    fprintf(stderr, "Total %d SVP records written\n", svp_written_tot);
  }
  if (output_counts)
    fprintf(stdout, "%d\n", svp_unique_tot);

  /* deallocate memory */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&svp_save, &error);

  /* check memory */
  if (verbose >= 4)
    status &= mb_memory_list(verbose, &error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2  Ending status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
