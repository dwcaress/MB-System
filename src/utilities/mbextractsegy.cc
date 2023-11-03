/*--------------------------------------------------------------------
 *    The MB-system:  mbextractsegy.c  4/18/2004
 *
 *    Copyright (c) 2004-2023 by
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
 * mbextractsegy extracts subbottom profiler, center beam reflection,
 * or seismic reflection data from data supported by MB-System and
 * rewrites it as a SEGY file in the form used by SIOSEIS. .
 *
 * Author:  D. W. Caress
 * Date:  April 18, 2004
 */

#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_segy.h"
#include "mb_status.h"

constexpr int MBES_ALLOC_NUM = 128;
typedef enum {
    MBES_ROUTE_WAYPOINT_NONE = 0,
    MBES_ROUTE_WAYPOINT_SIMPLE = 1,
    MBES_ROUTE_WAYPOINT_TRANSIT = 2,
    MBES_ROUTE_WAYPOINT_STARTLINE = 3,
    MBES_ROUTE_WAYPOINT_ENDLINE = 4,
} waypoint_t;
constexpr double MBES_ONLINE_THRESHOLD = 15.0;
constexpr int MBES_ONLINE_COUNT = 30;
constexpr int MBES_NUM_PLOT_MAX = 50;
constexpr double MBES_MAX_SWEEP = 1.0;

constexpr char program_name[] = "MBextractsegy";
constexpr char help_message[] =
    "MBextractsegy extracts subbottom profiler, center beam reflection,\n"
    "or seismic reflection data from data supported by MB-System and\n"
    "rewrites it as a SEGY file in the form used by SIOSEIS.";
constexpr char usage_message[] =
    "mbextractsegy [-Byr/mo/dy/hr/mn/sc/us -Eyr/mo/dy/hr/mn/sc/us -Fformat\n"
    "    -Ifile -Jxscale/yscale -Lstartline/lineroot\n"
    "    -Osegyfile -Qtimelistfile -Rroutefile\n"
    "    -Ssampleformat -Zplotmax -H -V]";

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

  mb_path read_file = "datalist.mb-1";

  mb_path lineroot = "sbp";
  int startline = 1;

  int sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;

  mb_path route_file = "";
  bool route_file_set = false;
  double timeshift = 0.0;
  double maxwidth = 30.0;
  double xscale = 0.01;
  double yscale = 50.0;
  double zmax = 50;
  bool checkroutebearing = false;
  double rangethreshold = 25.0;
  mb_path timelist_file = {0};
  bool timelist_file_set = false;
  mb_pathplus output_file = {0};
  bool output_file_set = false;
  waypoint_t waypoint;

  {
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt(argc, argv, "B:b:D:d:E:e:F:f:I:i:J:j:L:l:MmO:o:Q:q:R:r:S:s:T:t:U:u:Z:z:VvHh")) != -1)
    {
      switch (c) {
      case 'H':
      case 'h':
        help = true;
        break;
      case 'V':
      case 'v':
        verbose++;
        break;
      case 'B':
      case 'b':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
        btime_i[6] = 0;
        break;
      case 'E':
      case 'e':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
        etime_i[6] = 0;
        break;
      case 'F':
      case 'f':
        sscanf(optarg, "%d", &format);
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", read_file);
        break;
      case 'J':
      case 'j':
        sscanf(optarg, "%lf/%lf/%lf", &xscale, &yscale, &maxwidth);
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d/%1023s", &startline, lineroot);
        break;
      case 'M':
      case 'm':
        checkroutebearing = true;
        break;
      case 'O':
      case 'o':
        sscanf(optarg, "%1023s", output_file);
        output_file_set = true;
        break;
      case 'Q':
      case 'q':
        sscanf(optarg, "%1023s", timelist_file);
        timelist_file_set = true;
        break;
      case 'R':
      case 'r':
        sscanf(optarg, "%1023s", route_file);
        route_file_set = true;
        break;
      case 'S':
      case 's':
        sscanf(optarg, "%d", &sampleformat);
        break;
      case 'T':
      case 't':
        sscanf(optarg, "%lf", &timeshift);
        break;
      case 'U':
      case 'u':
        sscanf(optarg, "%lf", &rangethreshold);
        break;
      case 'Z':
      case 'z':
        sscanf(optarg, "%lf", &zmax);
        break;
      case '?':
        errflg = true;
      }
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
      fprintf(stderr, "dbg2       sampleformat:      %d\n", sampleformat);
      fprintf(stderr, "dbg2       timeshift:         %f\n", timeshift);
      fprintf(stderr, "dbg2       timelist_file_set: %d\n", timelist_file_set);
      fprintf(stderr, "dbg2       timelist_file:     %s\n", timelist_file);
      fprintf(stderr, "dbg2       route_file_set:    %d\n", route_file_set);
      fprintf(stderr, "dbg2       route_file:        %s\n", route_file);
      fprintf(stderr, "dbg2       checkroutebearing: %d\n", checkroutebearing);
      fprintf(stderr, "dbg2       output_file_set:   %d\n", output_file_set);
      fprintf(stderr, "dbg2       output_file:       %s\n", output_file);
      fprintf(stderr, "dbg2       lineroot:          %s\n", lineroot);
      fprintf(stderr, "dbg2       xscale:            %f\n", xscale);
      fprintf(stderr, "dbg2       yscale:            %f\n", yscale);
      fprintf(stderr, "dbg2       maxwidth:          %f\n", maxwidth);
      fprintf(stderr, "dbg2       rangethreshold:    %f\n", rangethreshold);
    }

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  void *datalist = nullptr;
  double file_weight = 1.0;
  double btime_d;
  double etime_d;
  mb_path file = "";
  mb_path dfile = "";
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* MBIO read values */
  void *mbio_ptr = nullptr;
  void *store_ptr = nullptr;
  int kind;
  int time_i[7];
  int time_j[5];
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
  int icomment = 0;

  /* segy data */
  int samplesize = 0;
  struct mb_segytraceheader_struct segytraceheader;
  int segydata_alloc = 0;
  float *segydata = nullptr;
  int buffer_alloc = 0;
  char *buffer = nullptr;

  /* route and auto-line data */
  double *routetime_d = nullptr;
  int nroutepoint = 0;
  double lon;
  double lat;
  double topo;
  double *routelon = nullptr;
  double *routelat = nullptr;
  double *routeheading = nullptr;
  int *routewaypoint = nullptr;
  double range = 0.0;
  double rangelast = 0.0;
  int activewaypoint = 0;

  /* auto plotting */
  FILE *sfp = nullptr;
  mb_pathplus scriptfile = "";
  double seafloordepthmin = -1.0;
  double seafloordepthmax = -1.0;
  double seafloordepthminplot[MBES_NUM_PLOT_MAX];
  double seafloordepthmaxplot[MBES_NUM_PLOT_MAX];
  double sweep;
  double delay;
  double startlon;
  double startlat;
  int startshot;
  double endlon;
  double endlat;
  int endshot;
  double linedistance;
  double linebearing;
  int nplot = 0;
  mb_path zbounds = {0};

  mb_command command = {0};
  mb_path scale = {0};
  double mtodeglon, mtodeglat;
  double headingdiff;
  int oktowrite;
  FILE *fp = nullptr;
  char *result;
  int nwrite;
  double tracemin, tracemax, tracerms, tracelength;
  double linetracemin, linetracemax, linetracelength, endofdata;
  double draft, roll, pitch, heave;

  /* set starting line number */
  int linenumber = startline;

  /* set maximum number of shots per plot */
  int nshotmax = (int)(maxwidth / xscale);

  // bool rawroutefile = false;
  bool rangeok;
  int error = MB_ERROR_NO_ERROR;

  /* if specified read route time list file */
  int nroutepointalloc = 0;
  if (timelist_file_set) {
    /* open the input file */
    if ((fp = fopen(timelist_file, "r")) == nullptr) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open time list file <%s> for reading\n", timelist_file);
      exit(status);
    }
    // rawroutefile = false;
    while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
      if (comment[0] != '#') {
        {
          int cnt;
          int tmp;
          /* const int nget = */
          sscanf(comment, "%d %d %lf %lf %lf %lf", &cnt, &tmp, &lon, &lat, &heading, &time_d);
          if (tmp >= 0 && tmp <= 4)
            waypoint = (waypoint_t)tmp;
          else
            waypoint = MBES_ROUTE_WAYPOINT_NONE;
        }

        /* if good data check for need to allocate more space */
        if (nroutepoint + 1 > nroutepointalloc) {
          nroutepointalloc += MBES_ALLOC_NUM;
          int reallocd_status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routetime_d, &error);
          if (reallocd_status != MB_SUCCESS) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* add good point to route */
        if (nroutepointalloc > nroutepoint) {
          routewaypoint[nroutepoint] = waypoint;
          routelon[nroutepoint] = lon;
          routelat[nroutepoint] = lat;
          routeheading[nroutepoint] = heading;
          routetime_d[nroutepoint] = time_d;
          nroutepoint++;
        }
      }
    }

    /* close the file */
    fclose(fp);
    fp = nullptr;

    /* set starting values */
    activewaypoint = 0;
    mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
    rangelast = 1000 * rangethreshold;
    seafloordepthmin = -1.0;
    seafloordepthmax = -1.0;
    nplot = 0;
    for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
      seafloordepthminplot[i] = -1;
      seafloordepthmaxplot[i] = -1;
    }
    oktowrite = 0;
    rangeok = false;

    /* output status */
    if (verbose > 0) {
      /* output info on file output */
      fprintf(stderr, "Read %d waypoints from time list file: %s\n", nroutepoint, timelist_file);
    }
  }

  /* if specified read route file */
  else if (route_file_set) {
    /* open the input file */
    if ((fp = fopen(route_file, "r")) == nullptr) {
      error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
      fprintf(stderr, "\nUnable to open route file <%s> for reading\n", route_file);
      exit(status);
    }
    bool rawroutefile = false;  // TODO(schwehr): Explain how this flag works.  Suspicious
    while ((result = fgets(comment, MB_PATH_MAXLINE, fp)) == comment) {
      if (comment[0] == '#') {
        if (strncmp(comment, "## Route File Version", 21) == 0) {
          rawroutefile = false;
        }
      }
      else {
        int waypoint_tmp;
        const int nget = sscanf(comment, "%lf %lf %lf %d %lf", &lon, &lat, &topo, &waypoint_tmp, &heading);
        waypoint = (waypoint_t)waypoint_tmp;  // TODO(schwehr): Range check
        if (comment[0] == '#') {
          fprintf(stderr, "buffer:%s", comment);
          if (strncmp(comment, "## Route File Version", 21) == 0) {
            rawroutefile = false;
          }
        }
        bool point_ok;
        if ((rawroutefile && nget >= 2) ||
            (!rawroutefile && nget >= 3 && waypoint > MBES_ROUTE_WAYPOINT_NONE))
          point_ok = true;
        else
          point_ok = false;

        /* if good data check for need to allocate more space */
        if (point_ok && nroutepoint + 1 > nroutepointalloc) {
          nroutepointalloc += MBES_ALLOC_NUM;
          int reallocd_status = mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelon, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routelat, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(double), (void **)&routeheading, &error);
          reallocd_status &= mb_reallocd(verbose, __FILE__, __LINE__, nroutepointalloc * sizeof(int), (void **)&routewaypoint, &error);
          if (reallocd_status != MB_SUCCESS) {
            char *message;
            mb_error(verbose, error, &message);
            fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(error);
          }
        }

        /* add good point to route */
        if (point_ok && nroutepointalloc > nroutepoint + 1) {
          routelon[nroutepoint] = lon;
          routelat[nroutepoint] = lat;
          routeheading[nroutepoint] = heading;
          routewaypoint[nroutepoint] = waypoint;
          nroutepoint++;
        }
      }
    }

    /* close the file */
    fclose(fp);
    fp = nullptr;

    /* set starting values */
    activewaypoint = 1;
    mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
    rangelast = 1000 * rangethreshold;
    seafloordepthmin = -1.0;
    seafloordepthmax = -1.0;
    nplot = 0;
    for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
      seafloordepthminplot[i] = -1;
      seafloordepthmaxplot[i] = -1;
    }
    oktowrite = 0;
    rangeok = false;

    /* output status */
    if (verbose > 0) {
      /* output info on file output */
      fprintf(stderr, "Read %d waypoints from route file: %s\n", nroutepoint, route_file);
    }
  }

  /* get format if required */
  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  /* get sample size from sampleformat */
  if (sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC)
    samplesize = 2 * sizeof(float);
  else
    samplesize = sizeof(float);

  /* get plot zbounds from sampleformat */
  if (sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE)
    snprintf(zbounds, sizeof(zbounds), "0/%f/1", zmax);
  else
    snprintf(zbounds, sizeof(zbounds),  "-%f/%f", zmax, zmax);

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;

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

  /* set up plotting script file */
  if ((route_file_set && nroutepoint > 1) || (timelist_file_set && nroutepoint > 1)) {
    snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", lineroot);
  }
  else if (!output_file_set || read_datalist) {
    snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", read_file);
  }
  else {
    snprintf(scriptfile, sizeof(scriptfile), "%s_section.cmd", file);
  }
  if ((sfp = fopen(scriptfile, "w")) == nullptr) {
    error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
    fprintf(stderr, "\nUnable to open plotting script file <%s> \n", scriptfile);
    exit(status);
  }

  struct mb_segyasciiheader_struct segyasciiheader;
  for (int j = 0; j < 40; j++)
    for (int i = 0; i < 80; i++)
      segyasciiheader.line[j][i] = 0;

  struct mb_segyfileheader_struct segyfileheader;
  segyfileheader.jobid = 0;
  segyfileheader.line = 0;
  segyfileheader.reel = 0;
  segyfileheader.channels = 0;
  segyfileheader.aux_channels = 0;
  segyfileheader.sample_interval = 0;
  segyfileheader.sample_interval_org = 0;
  segyfileheader.number_samples = 0;
  segyfileheader.number_samples_org = 0;
  segyfileheader.format = 5;
  segyfileheader.cdp_fold = 0;
  segyfileheader.trace_sort = 0;
  segyfileheader.vertical_sum = 0;
  segyfileheader.sweep_start = 0;
  segyfileheader.sweep_end = 0;
  segyfileheader.sweep_length = 0;
  segyfileheader.sweep_type = 0;
  segyfileheader.sweep_trace = 0;
  segyfileheader.sweep_taper_start = 0;
  segyfileheader.sweep_taper_end = 0;
  segyfileheader.sweep_taper = 0;
  segyfileheader.correlated = 0;
  segyfileheader.binary_gain = 0;
  segyfileheader.amplitude = 0;
  segyfileheader.units = 0;
  segyfileheader.impulse_polarity = 0;
  segyfileheader.domain = 0;
  for (int i = 0; i < 338; i++)
    segyfileheader.extra[i] = 0;

  bool linechange = false;

  /* loop over all files to be read */
  while (read_data) {

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
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&beamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bath, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);

    /* if error initializing memory then quit */
    if (error != MB_ERROR_NO_ERROR) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* read and print data */
    int nread = 0;
    // bool first = true;
    double lastlon = 0.0;
    double lastlat = 0.0;
    double lastheading = 0.0;
    double lastdistance = 0.0;
    while (error <= MB_ERROR_NO_ERROR) {
      /* reset error */
      error = MB_ERROR_NO_ERROR;

      /* read next data record */
      status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      /* ignore nonfatal errors */
      if (error < 0) {
        error = MB_ERROR_NO_ERROR;
        status = MB_SUCCESS;
      }

      /* deal with nav and time from survey data only - not nav, sidescan, or subbottom */
      if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
        /* reset output flag */
        linechange = false;

        /* get nav data */
        status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                                &draft, &roll, &pitch, &heave, &error);

        /* save last nav and heading */
        if (navlon != 0.0)
          lastlon = navlon;
        if (navlat != 0.0)
          lastlat = navlat;
        if (heading != 0.0)
          lastheading = heading;
        if (distance != 0.0)
          lastdistance = distance;

        /* to set lines check survey data time against time list */
        if (nroutepoint > 1) {
          const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
          const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
          range = sqrt(dx * dx + dy * dy);
          if (activewaypoint < nroutepoint && time_d >= routetime_d[activewaypoint]) {
            linechange = true;
          }
        }

        /* else to set lines check survey data position against waypoints */
        else if (nroutepoint > 1 && navlon != 0.0 && navlat != 0.0) {
          const double dx = (navlon - routelon[activewaypoint]) / mtodeglon;
          const double dy = (navlat - routelat[activewaypoint]) / mtodeglat;
          range = sqrt(dx * dx + dy * dy);
          if (range < rangethreshold)
            rangeok = true;
          if (rangeok && (activewaypoint == 0 || range > rangelast) && activewaypoint < nroutepoint - 1) {
            linechange = true;
          }
        }

        /* apply line change */
        if (linechange) {
          /* close current output file */
          if (fp != nullptr) {
            fclose(fp);
            fp = nullptr;

            /* output count of segy records */
            fprintf(stderr, "%d records output to segy file %s\n", nwrite, output_file);
            if (verbose > 0)
              fprintf(stderr, "\n");

            /* use mbsegyinfo to generate a sinf file */
            snprintf(command, sizeof(command),  "mbsegyinfo -I %s -O", output_file);
            fprintf(stderr, "Executing: %s\n", command);
            /* const int shellstatus = */ system(command);
            // TODO(schwehr): Check the shellstatus

            /* get bearing and plot scale */
            const double dx = (endlon - startlon) / mtodeglon;
            const double dy = (endlat - startlat) / mtodeglat;
            linedistance = sqrt(dx * dx + dy * dy);
            linebearing = RTD * atan2(dx, dy);
            if (linebearing < 0.0)
              linebearing += 360.0;
            if (linebearing >= 45.0 && linebearing <= 225.0)
              snprintf(scale, sizeof(scale), "-Jx%f/%f", xscale, yscale);
            else
              snprintf(scale, sizeof(scale), "-Jx-%f/%f", xscale, yscale);

            /* output commands to first cut plotting script file */
            /* The maximum useful plot length is about nshotmax shots, so
                we break longer files up into multiple plots */
            // int nshot = endshot - startshot + 1;
            nplot = nwrite / nshotmax;
            if (nwrite % nshotmax > 0)
              nplot++;
            /* calculate sweep needed for all of the data in the line - if this is more than 1.0 seconds,
              then make section plots using only the sweep needed for each section alone */
            delay = seafloordepthmin / 750.0;
            delay = ((int)(delay / 0.05)) * 0.05;
            endofdata = seafloordepthmax / 750.0 + linetracelength;
            endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
            sweep = endofdata - delay;

            const bool recalculatesweep = sweep > MBES_MAX_SWEEP;

            fprintf(sfp, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
            fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
            fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
            fprintf(sfp, "#   Section length: %f km\n", linedistance);
            fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
            fprintf(stderr, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
            fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
            fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
            fprintf(stderr, "#   Section length: %f km\n", linedistance);
            fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
            for (int i = 0; i < nplot; i++) {
              if (recalculatesweep) {
                seafloordepthmin = seafloordepthminplot[i];
                seafloordepthmax = seafloordepthmaxplot[i];
                delay = seafloordepthmin / 750.0;
                delay = ((int)(delay / 0.05)) * 0.05;
                endofdata = seafloordepthmax / 750.0 + linetracelength;
                endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
                sweep = endofdata - delay;
              }

              snprintf(command, sizeof(command), "#   Section plot %d of %d\n", i + 1, nplot);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);
              snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n", seafloordepthmin,
                      seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);
              snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);
              snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);
              snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);

              snprintf(command, sizeof(command), "mbsegygrid -I %s \\\n\t-S0/%d/%d -T%.2f/%.2f \\\n\t-O %s_%4.4d_%2.2d_section\n",
                      output_file, (startshot + i * nshotmax), std::min((startshot + (i + 1) * nshotmax - 1), endshot),
                      sweep, delay, lineroot, linenumber, i + 1);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);

              snprintf(command, sizeof(command),
                      "mbm_grdplot -I %s_%4.4d_%2.2d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
                      "-W1/4 -D -V \\\n\t-O %s_%4.4d_%2.2d_sectionplot \\\n\t-L\"%s Line %d Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
                      lineroot, linenumber, i + 1, scale, zbounds, lineroot, linenumber, i + 1, lineroot,
                      linenumber, i + 1, nplot);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);

              snprintf(command, sizeof(command), "%s_%4.4d_%2.2d_sectionplot.cmd $1\n\n", lineroot, linenumber, i + 1);
              fprintf(stderr, "%s", command);
              fprintf(sfp, "%s", command);

              fflush(sfp);
            }
          }

          /* increment line number */
          if (activewaypoint > 0)
            linenumber++;

          /* increment active waypoint */
          activewaypoint++;

          mb_coor_scale(verbose, routelat[activewaypoint], &mtodeglon, &mtodeglat);
          rangelast = 1000 * rangethreshold;
          seafloordepthmin = -1.0;
          seafloordepthmax = -1.0;
          nplot = 0;
          for (int i = 0; i < MBES_NUM_PLOT_MAX; i++) {
            seafloordepthminplot[i] = -1;
            seafloordepthmaxplot[i] = -1;
          }
          oktowrite = 0;
          rangeok = false;
        }
        else
          rangelast = range;
        if (verbose > 0 && nroutepoint > 0)
          fprintf(stderr,
                  "> activewaypoint:%d linenumber:%d time_d:%f range:%f   lon: %f %f   lat: %f %f oktowrite:%d "
                  "rangeok:%d kind:%d\n",
                  activewaypoint, linenumber, time_d, range, navlon, routelon[activewaypoint], navlat,
                  routelat[activewaypoint], oktowrite, rangeok, kind);
      }

      /* if desired extract subbottom data */
      if (error == MB_ERROR_NO_ERROR &&
          (kind == MB_DATA_SUBBOTTOM_MCS || kind == MB_DATA_SUBBOTTOM_CNTRBEAM || kind == MB_DATA_SUBBOTTOM_SUBBOTTOM)) {
        /* extract the header */
        status = mb_extract_segytraceheader(verbose, mbio_ptr, store_ptr, &kind, (void *)&segytraceheader, &error);

        /* allocate the required memory */
        if (status == MB_SUCCESS && segytraceheader.nsamps > segydata_alloc) {
          status =
              mb_mallocd(verbose, __FILE__, __LINE__, segytraceheader.nsamps * samplesize, (void **)&segydata, &error);
          if (status == MB_SUCCESS)
            segydata_alloc = segytraceheader.nsamps;
          else
            segydata_alloc = 0;
        }
        if (status == MB_SUCCESS &&
            (buffer_alloc < MB_SEGY_TRACEHEADER_LENGTH || buffer_alloc < segytraceheader.nsamps * samplesize)) {
          buffer_alloc = std::max(MB_SEGY_TRACEHEADER_LENGTH, segytraceheader.nsamps * samplesize);
          status = mb_mallocd(verbose, __FILE__, __LINE__, buffer_alloc, (void **)&buffer, &error);
          if (status != MB_SUCCESS)
            buffer_alloc = 0;
        }

        /* extract the data */
        if (status == MB_SUCCESS)
          status = mb_extract_segy(verbose, mbio_ptr, store_ptr, &sampleformat, &kind, (void *)&segytraceheader,
                                   segydata, &error);

        /* apply time shift if needed */
        if (status == MB_SUCCESS && timeshift != 0.0) {
          time_j[0] = segytraceheader.year;
          time_j[1] = segytraceheader.day_of_yr;
          time_j[2] = 60 * segytraceheader.hour + segytraceheader.min;
          time_j[3] = segytraceheader.sec;
          time_j[4] = 1000 * segytraceheader.mils;
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          time_d += timeshift;
          mb_get_date(verbose, time_d, time_i);
          mb_get_jtime(verbose, time_i, time_j);
          segytraceheader.year = time_i[0];
          segytraceheader.day_of_yr = time_j[1];
          segytraceheader.hour = time_i[3];
          segytraceheader.min = time_i[4];
          segytraceheader.sec = time_i[5];
          segytraceheader.mils = time_i[6] / 1000;
        }

        /* set nav and heading using most recent survey data */
        segytraceheader.src_long = (int)(lastlon * 360000.0);
        segytraceheader.src_lat = (int)(lastlat * 360000.0);
        segytraceheader.grp_long = (int)(lastlon * 360000.0);
        segytraceheader.grp_lat = (int)(lastlat * 360000.0);
        segytraceheader.heading = lastheading;
        segytraceheader.roll = roll;
        segytraceheader.pitch = pitch;
        segytraceheader.distance = 1000.0 * lastdistance;

        /* if following a route check that the vehicle has come on line
            (within MBES_ONLINE_THRESHOLD degrees)
            before writing any data */
        if (activewaypoint > 0 && checkroutebearing && nroutepoint > 1) {
          headingdiff = fabs(routeheading[activewaypoint - 1] - segytraceheader.heading);
          if (headingdiff > 180.0)
            headingdiff = 360.0 - headingdiff;
          if (headingdiff < MBES_ONLINE_THRESHOLD)
            oktowrite++;
          else
            oktowrite = 0;
        }
        else if (activewaypoint > 0)
          oktowrite = MBES_ONLINE_COUNT;
        else if (nroutepoint == 0 && nroutepoint == 0)
          oktowrite = MBES_ONLINE_COUNT;

        /* open output segy file if needed */
        if (fp == nullptr && oktowrite > 0) {
          /* set up output filename */
          if (!output_file_set) {
            memset(output_file, 0, MB_PATH_MAXLINE);
            if (nroutepoint > 1) {
              assert(strlen(lineroot) < MB_PATH_MAXLINE - 10);
              snprintf(output_file, sizeof(output_file), "%s_%4.4d.segy", lineroot, linenumber);
            }
            else {
              assert(strlen(file) < MB_PATH_MAXLINE - 5);
              strcat(output_file, "%s.segy");
            }
          }

          /* open the new file */
          nwrite = 0;
          if ((fp = fopen(output_file, "w")) == nullptr) {
            fprintf(stderr, "\nError opening output segy file:\n%s\n", output_file);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(MB_ERROR_WRITE_FAIL);
          }
          else if (verbose > 0) {
            /* output info on file output */
            fprintf(stderr, "Outputting subbottom data to segy file %s\n", output_file);
          }
        }

        /* note good status */
        if (status == MB_SUCCESS) {
          /* get trace min and max */
          tracemin = segydata[0];
          tracemax = segydata[0];
          tracerms = 0.0;
          for (int i = 0; i < segytraceheader.nsamps; i++) {
            tracemin = std::min(tracemin, static_cast<double>(segydata[i]));
            tracemax = std::max(tracemax, static_cast<double>(segydata[i]));
            tracerms += segydata[i] * segydata[i];
          }
          tracerms = sqrt(tracerms / segytraceheader.nsamps);
          tracelength = 0.000001 * segytraceheader.si_micros * segytraceheader.nsamps;

          /* get starting and ending positions */
          if (nwrite == 0) {
            startlon = ((double)segytraceheader.src_long) / 360000.0;
            startlat = ((double)segytraceheader.src_lat) / 360000.0;
            startshot = segytraceheader.shot_num;
            linetracemin = tracemin;
            linetracemax = tracemax;
            linetracelength = tracelength;
          }
          else {
            endlon = ((double)segytraceheader.src_long) / 360000.0;
            endlat = ((double)segytraceheader.src_lat) / 360000.0;
            endshot = segytraceheader.shot_num;
            linetracemin = std::min(tracemin, linetracemin);
            linetracemax = std::max(tracemax, linetracemax);
            linetracelength = std::max(tracelength, linetracelength);
          }

          /* check for new section plot */
          if (nwrite > 0 && (nwrite % nshotmax) == 0)
            nplot++;

          /* get seafloor depth min and max */
          if (segytraceheader.src_wbd > 0) {
            if (seafloordepthmin < 0.0) {
              seafloordepthmin = 0.01 * ((double)segytraceheader.src_wbd);
              seafloordepthmax = 0.01 * ((double)segytraceheader.src_wbd);
            }
            else {
              seafloordepthmin = std::min(seafloordepthmin, 0.01 * ((double)segytraceheader.src_wbd));
              seafloordepthmax = std::max(seafloordepthmax, 0.01 * ((double)segytraceheader.src_wbd));
            }
            if (seafloordepthminplot[nplot] < 0.0) {
              seafloordepthminplot[nplot] = 0.01 * ((double)segytraceheader.src_wbd);
              seafloordepthmaxplot[nplot] = 0.01 * ((double)segytraceheader.src_wbd);
            }
            else {
              seafloordepthminplot[nplot] =
                  std::min(seafloordepthminplot[nplot], 0.01 * ((double)segytraceheader.src_wbd));
              seafloordepthmaxplot[nplot] =
                  std::max(seafloordepthmaxplot[nplot], 0.01 * ((double)segytraceheader.src_wbd));
            }
          }

          /* output info */
          nread++;
          if (nread % 10 == 0 && verbose > 0)
            fprintf(stderr,
                    "file:%s record:%d shot:%d  %4.4d/%3.3d %2.2d:%2.2d:%2.2d.%3.3d samples:%d interval:%d usec  "
                    "minmax: %f %f\n",
                    file, nread, segytraceheader.shot_num, segytraceheader.year, segytraceheader.day_of_yr,
                    segytraceheader.hour, segytraceheader.min, segytraceheader.sec, segytraceheader.mils,
                    segytraceheader.nsamps, segytraceheader.si_micros, tracemin, tracemax);

          /* only write data if ok */
          if (oktowrite >= MBES_ONLINE_COUNT) {
            /* write characteristics file */
            fprintf(stderr, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d  %d %d %d   %f %f %f  %f %f %f %f\n",
                    time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
                    segytraceheader.shot_num, segytraceheader.nsamps, segytraceheader.si_micros, tracemin, tracemax,
                    tracerms, sensordepth, altitude, roll, pitch);

            /* write fileheader if needed */
            if (status == MB_SUCCESS && nwrite == 0) {
              segyfileheader.line = linenumber;
              segyfileheader.format = 5;
              segyfileheader.channels = 1;
              segyfileheader.aux_channels = 0;
              segyfileheader.sample_interval = segytraceheader.si_micros;
              segyfileheader.sample_interval_org = segytraceheader.si_micros;
              segyfileheader.number_samples = segytraceheader.nsamps;

              /* insert file header data into output buffer */
              int index = 0;
              mb_put_binary_int(false, segyfileheader.jobid, (void *)&(buffer[index]));
              index += 4;
              mb_put_binary_int(false, segyfileheader.line, (void *)&(buffer[index]));
              index += 4;
              mb_put_binary_int(false, segyfileheader.reel, (void *)&(buffer[index]));
              index += 4;
              mb_put_binary_short(false, segyfileheader.channels, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.aux_channels, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sample_interval, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sample_interval_org, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.number_samples, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.number_samples_org, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.format, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.cdp_fold, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.trace_sort, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.vertical_sum, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_start, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_end, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_length, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_type, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_trace, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_taper_start, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_taper_end, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.sweep_taper, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.correlated, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.binary_gain, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.amplitude, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.units, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.impulse_polarity, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.vibrate_polarity, (void *)&(buffer[index]));
              index += 2;
              mb_put_binary_short(false, segyfileheader.domain, (void *)&(buffer[index]));
              index += 2;
              for (int i = 0; i < 338; i++) {
                buffer[index] = segyfileheader.extra[i];
                index++;
              }

              segyfileheader.number_samples_org = segytraceheader.nsamps;
              if (fwrite(&segyasciiheader, 1, MB_SEGY_ASCIIHEADER_LENGTH, fp) != MB_SEGY_ASCIIHEADER_LENGTH) {
                status = MB_FAILURE;
                error = MB_ERROR_WRITE_FAIL;
              }
              else if (fwrite(buffer, 1, MB_SEGY_FILEHEADER_LENGTH, fp) != MB_SEGY_FILEHEADER_LENGTH) {
                status = MB_FAILURE;
                error = MB_ERROR_WRITE_FAIL;
              }
            }

            /* insert segy header data into output buffer */
            int index = 0;
            mb_put_binary_int(false, segytraceheader.seq_num, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.seq_reel, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.shot_num, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.shot_tr, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.espn, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.rp_num, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.rp_tr, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_short(false, segytraceheader.trc_id, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.num_vstk, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.cdp_fold, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.use, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_int(false, segytraceheader.range, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.grp_elev, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.src_elev, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.src_depth, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.grp_datum, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.src_datum, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.src_wbd, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.grp_wbd, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_short(false, segytraceheader.elev_scalar, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.coord_scalar, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_int(false, segytraceheader.src_long, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.src_lat, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.grp_long, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.grp_lat, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_short(false, segytraceheader.coord_units, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.wvel, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.sbvel, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.src_up_vel, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.grp_up_vel, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.src_static, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.grp_static, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.tot_static, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.laga, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_int(false, segytraceheader.delay_mils, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_short(false, segytraceheader.smute_mils, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.emute_mils, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.nsamps, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.si_micros, (void *)&buffer[index]);
            index += 2;
            for (int i = 0; i < 19; i++) {
              mb_put_binary_short(false, segytraceheader.other_1[i], (void *)&buffer[index]);
              index += 2;
            }
            mb_put_binary_short(false, segytraceheader.year, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.day_of_yr, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.hour, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.min, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.sec, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.mils, (void *)&buffer[index]);
            index += 2;
            mb_put_binary_short(false, segytraceheader.tr_weight, (void *)&buffer[index]);
            index += 2;
            for (int i = 0; i < 5; i++) {
              mb_put_binary_short(false, segytraceheader.other_2[i], (void *)&buffer[index]);
              index += 2;
            }
            mb_put_binary_float(false, segytraceheader.delay, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.smute_sec, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.emute_sec, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.si_secs, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.wbt_secs, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_int(false, segytraceheader.end_of_rp, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.dummy1, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.dummy2, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.dummy3, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.dummy4, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.soundspeed, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.distance, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.roll, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.pitch, (void *)&buffer[index]);
            index += 4;
            mb_put_binary_float(false, segytraceheader.heading, (void *)&buffer[index]);
            // index += 4;

            /* write out segy header */
            if (fwrite(buffer, 1, MB_SEGY_TRACEHEADER_LENGTH, fp) != MB_SEGY_TRACEHEADER_LENGTH) {
              status = MB_FAILURE;
              error = MB_ERROR_WRITE_FAIL;
            }

            /* insert segy data into output buffer */
            index = 0;
            for (int i = 0; i < segytraceheader.nsamps; i++) {
              mb_put_binary_float(false, segydata[i], (void *)&buffer[index]);
              index += 4;
            }

            /* write out data */
            nwrite++;
            if (status == MB_SUCCESS &&
                fwrite(buffer, 1, segytraceheader.nsamps * samplesize, fp) != segytraceheader.nsamps * samplesize) {
              status = MB_FAILURE;
              error = MB_ERROR_WRITE_FAIL;
            }
          }
        }
      }

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }

      /* print comments */
      if (verbose >= 1 && kind == MB_DATA_COMMENT) {
        if (icomment == 0) {
          fprintf(stderr, "\nComments:\n");
          icomment++;
        }
        fprintf(stderr, "%s\n", comment);
      }
    }

    /* close the swath file */
    status = mb_close(verbose, &mbio_ptr, &error);

    /* output read statistics */
    fprintf(stderr, "%d records read from %s\n", nread, file);

    /* deallocate memory used for segy data arrays */
    mb_freed(verbose, __FILE__, __LINE__, (void **)&segydata, &error);
    segydata_alloc = 0;
    mb_freed(verbose, __FILE__, __LINE__, (void **)&buffer, &error);
    buffer_alloc = 0;

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight, &error) == MB_SUCCESS;
    } else {
      read_data = false;
    }

    /* close output file if conditions warrant */
    if (!read_data || (!output_file_set && nroutepoint < 2)) {
      /* close current output file */
      if (fp != nullptr) {
        fclose(fp);
        fp = nullptr;

        /* output count of segy records */
        fprintf(stderr, "\n%d records output to segy file %s\n", nwrite, output_file);
        if (verbose > 0)
          fprintf(stderr, "\n");

        /* use mbsegyinfo to generate a sinf file */
        snprintf(command, sizeof(command), "mbsegyinfo -I %s -O", output_file);
        fprintf(stderr, "Executing: %s\n", command);
        /* const int shellstatus = */ system(command);
        // TODO(schwehr): Check the shellstatus

        /* get bearing and plot scale */
        const double dx = (endlon - startlon) / mtodeglon;
        const double dy = (endlat - startlat) / mtodeglat;
        linedistance = sqrt(dx * dx + dy * dy);
        if (linebearing < 0.0)
          linebearing += 360.0;
        if (linebearing >= 45.0 && linebearing <= 225.0)
          snprintf(scale, sizeof(scale), "-Jx%f/%f", xscale, yscale);
        else
          snprintf(scale, sizeof(scale), "-Jx-%f/%f", xscale, yscale);

        /* output commands to first cut plotting script file */
        /* The maximum useful plot length is about nshotmax shots, so
            we break longer files up into multiple plots */
        // const int nshot = endshot - startshot + 1;
        nplot = nwrite / nshotmax;
        if (nwrite % nshotmax > 0)
          nplot++;

        /* calculate sweep needed for all of the data in the line - if this is more than 1.0 seconds,
          then make section plots using only the sweep needed for each section alone */
        delay = seafloordepthmin / 750.0;
        delay = ((int)(delay / 0.05)) * 0.05;
        endofdata = seafloordepthmax / 750.0 + linetracelength;
        endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
        sweep = endofdata - delay;
        const bool recalculatesweep = sweep > MBES_MAX_SWEEP;

        fprintf(sfp, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
        fprintf(sfp, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
        fprintf(sfp, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
        fprintf(sfp, "#   Section length: %f km\n", linedistance);
        fprintf(sfp, "#   Section bearing: %f degrees\n", linebearing);
        fprintf(stderr, "# Generate %d section plot(s) of segy file: %s\n", nplot, output_file);
        fprintf(stderr, "#   Section Start Position: %.6f %.6f\n", startlon, startlat);
        fprintf(stderr, "#   Section End Position:   %.6f %.6f\n", endlon, endlat);
        fprintf(stderr, "#   Section length: %f km\n", linedistance);
        fprintf(stderr, "#   Section bearing: %f degrees\n", linebearing);
        for (int i = 0; i < nplot; i++) {
          if (recalculatesweep) {
            seafloordepthmin = seafloordepthminplot[i];
            seafloordepthmax = seafloordepthmaxplot[i];
            delay = seafloordepthmin / 750.0;
            delay = ((int)(delay / 0.05)) * 0.05;
            endofdata = seafloordepthmax / 750.0 + linetracelength;
            endofdata = (1 + (int)(endofdata / 0.05)) * 0.05;
            sweep = endofdata - delay;
          }

          snprintf(command, sizeof(command), "#   Section plot %d of %d\n", i + 1, nplot);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);
          snprintf(command, sizeof(command), "#     Seafloor depth min: %f (m) %f (s)  max: %f (m) %f (s)\n", seafloordepthmin,
                  seafloordepthmin / 750.0, seafloordepthmax, seafloordepthmax / 750.0);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);
          snprintf(command, sizeof(command), "#     Trace length: %f (s)\n", linetracelength);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);
          snprintf(command, sizeof(command), "#     Trace delay: %f (s)\n", delay);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);
          snprintf(command, sizeof(command), "#     Trace sweep: %f (s)\n", sweep);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);

          snprintf(command, sizeof(command), "mbsegygrid -I %s \\\n\t-S0/%d/%d -T%.2f/%.2f \\\n\t-O %s_%4.4d_%2.2d_section\n",
                  output_file, (startshot + i * nshotmax), std::min((startshot + (i + 1) * nshotmax - 1), endshot), sweep,
                  delay, lineroot, linenumber, i + 1);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);

          snprintf(command, sizeof(command),
                  "mbm_grdplot -I %s_%4.4d_%2.2d_section.grd \\\n\t%s -MGO2/2 -Z%s \\\n\t-Ba250/a0.05g0.05 -G1 "
                  "-W1/4 -D -V \\\n\t-O %s_%4.4d_%2.2d_sectionplot \\\n\t-L\"%s Line %d Plot %d of %d\"\\\n\t-MIE300 -MITg\n",
                  lineroot, linenumber, i + 1, scale, zbounds, lineroot, linenumber, i + 1, lineroot,
                  linenumber, i + 1, nplot);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);

          snprintf(command, sizeof(command), "%s_%4.4d_%2.2d_sectionplot.cmd $1\n\n", lineroot, linenumber, i + 1);
          fprintf(stderr, "%s", command);
          fprintf(sfp, "%s", command);

          fflush(sfp);
        }

        /* increment line number */
        linenumber++;
      }
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* close plotting script file */
  fclose(sfp);
  snprintf(command, sizeof(command), "chmod +x %s", scriptfile);
  /* const int shellstatus = */ system(command);
  // TODO(schwehr): Check the shellstatus

  /* deallocate route arrays */
  if (nroutepointalloc > 0) {
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelon, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routelat, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routeheading, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routewaypoint, &error);
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&routetime_d, &error);
  }

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
