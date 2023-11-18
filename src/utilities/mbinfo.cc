/*--------------------------------------------------------------------
 *    The MB-system:  mbinfo.c  2/1/93
 *
 *    Copyright (c) 1993-2023 by
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
 * MBINFO reads a swath sonar data file and outputs
 * some basic statistics.  If pings are averaged (pings > 2)
 * MBINFO estimates the variance for each of the swath
 * bathymetry beams by reading a set number of pings (>2) and then finding
 * the variance of the detrended values for each beam. The variances
 * for the amplitude beams and sidescan values are
 * calculated without detrending.
 * The results are dumped to stdout.
 *
 * Author:  D. W. Caress
 * Date:  February 1, 1993
 */

#include <cmath>
#include <algorithm>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_info.h"
#include "mb_io.h"
#include "mb_status.h"

constexpr int MBINFO_MAXPINGS = 50;

struct ping {
  char *beamflag;
  double *bath;
  double *bathlon;
  double *bathlat;
  double *amp;
  double *ss;
  double *sslon;
  double *sslat;
};

typedef enum {
  FREE_TEXT = 0,
  JSON = 1,
  XML = 2,
  MAX_OUTPUT_FORMAT = 2
} output_format_t ;

constexpr char program_name[] = "MBINFO";
constexpr char usage_message[] =
    "mbinfo [-Byr/mo/da/hr/mn/sc -C "
    "-Eyr/mo/da/hr/mn/sc -Fformat -G -Ifile -Llonflip -Mnx/ny "
    "-N -O -Ppings -Rw/e/s/n -Sspeed -W -V -H -XinfFormat]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int pings_get = 1;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings_get, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  char read_file[MB_PATH_MAXLINE] = "stdin";
  bool quick = false;
  bool comments = false;
  bool good_nav_only = false;
  bool lonflip_set = false;
  int lonflip_use = 0;
  bool coverage_mask = false;
  bool coverage_mask_bounds = false;
  int mask_nx = 0;
  int mask_ny = 0;
  double maskbounds[4];
  bool print_notices = false;
  bool output_usefile = false;
  int pings_read = 1;
  bool bathy_in_meters = true;
  output_format_t output_format = FREE_TEXT;

  /* process argument list */
  bool errflg = false;
  bool help = false;
  {
    int c;
    while ((c = getopt(argc, argv, "VvHhB:b:CcE:e:F:f:GgI:i:L:l:M:m:NnOoP:p:QqR:r:S:s:T:t:WwX:x:")) != -1) {
      switch (c) {
        case 'B':
        case 'b':
          sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
          btime_i[6] = 0;
          break;
        case 'C':
        case 'c':
          comments = true;
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
        case 'G':
        case 'g':
          good_nav_only = true;
          break;
        case 'H':
        case 'h':
          help = true;
          break;
        case 'I':
        case 'i':
          sscanf(optarg, "%1023s", read_file);
          break;
        case 'L':
        case 'l':
          sscanf(optarg, "%d", &lonflip);
          lonflip_set = true;
          lonflip_use = lonflip;
          break;
        case 'M':
        case 'm':
          {
          int n;
          n = sscanf(optarg, "%d/%d/%lf/%lf/%lf/%lf",
                          &mask_nx, &mask_ny, &maskbounds[0], &maskbounds[1],
                          &maskbounds[2], &maskbounds[3]);
          if (n >= 2 && mask_nx > 0 && mask_ny > 0)
              coverage_mask = true;
          if (n == 6 && maskbounds[1] > maskbounds[0] && maskbounds[3] > maskbounds[2])
              coverage_mask_bounds = true;
          }
          break;
        case 'N':
        case 'n':
          print_notices = true;
          break;
        case 'O':
        case 'o':
          output_usefile = true;
          break;
        case 'P':
        case 'p':
          sscanf(optarg, "%d", &pings_read);
          if (pings_read < 1)
            pings_read = 1;
          if (pings_read > MBINFO_MAXPINGS)
            pings_read = MBINFO_MAXPINGS;
          break;
        case 'Q':
        case 'q':
          quick = true;
          break;
        case 'R':
        case 'r':
          mb_get_bounds(optarg, bounds);
          break;
        case 'S':
        case 's':
          sscanf(optarg, "%lf", &speedmin);
          break;
        case 'T':
        case 't':
          sscanf(optarg, "%lf", &timegap);
          break;
        case 'V':
        case 'v':
          verbose++;
          break;
        case 'W':
        case 'w':
          bathy_in_meters = false;
          break;
        case 'X':
        case 'x':
        {
          int tmp;
          sscanf(optarg, "%d", &tmp);
          output_format = (output_format_t)tmp;
          if (output_format < 0 || output_format > MAX_OUTPUT_FORMAT) {
            errflg = true;
            fprintf(stderr, "Invalid output format for inf file");
          }
          break;
        }
        default:
          errflg = true;
      }
    }
  }

  FILE * const stream = verbose <= 1 ? stdout : stderr;

  if (errflg) {
    fprintf(stream, "usage: %s\n", usage_message);
    fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_ERROR_BAD_USAGE);
  }

  if (verbose == 1 || help) {
    fprintf(stream, "\nProgram %s\n", program_name);
    fprintf(stream, "MB-system Version %s\n", MB_VERSION);
  }

  if (verbose >= 2) {
    fprintf(stream, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stream, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stream, "dbg2  Control Parameters:\n");
    fprintf(stream, "dbg2       verbose:    %d\n", verbose);
    fprintf(stream, "dbg2       help:       %d\n", help);
    fprintf(stream, "dbg2       format:     %d\n", format);
    fprintf(stream, "dbg2       pings:      %d\n", pings_read);
    fprintf(stream, "dbg2       lonflip:    %d\n", lonflip);
    fprintf(stream, "dbg2       bounds[0]:  %f\n", bounds[0]);
    fprintf(stream, "dbg2       bounds[1]:  %f\n", bounds[1]);
    fprintf(stream, "dbg2       bounds[2]:  %f\n", bounds[2]);
    fprintf(stream, "dbg2       bounds[3]:  %f\n", bounds[3]);
    fprintf(stream, "dbg2       btime_i[0]: %d\n", btime_i[0]);
    fprintf(stream, "dbg2       btime_i[1]: %d\n", btime_i[1]);
    fprintf(stream, "dbg2       btime_i[2]: %d\n", btime_i[2]);
    fprintf(stream, "dbg2       btime_i[3]: %d\n", btime_i[3]);
    fprintf(stream, "dbg2       btime_i[4]: %d\n", btime_i[4]);
    fprintf(stream, "dbg2       btime_i[5]: %d\n", btime_i[5]);
    fprintf(stream, "dbg2       btime_i[6]: %d\n", btime_i[6]);
    fprintf(stream, "dbg2       etime_i[0]: %d\n", etime_i[0]);
    fprintf(stream, "dbg2       etime_i[1]: %d\n", etime_i[1]);
    fprintf(stream, "dbg2       etime_i[2]: %d\n", etime_i[2]);
    fprintf(stream, "dbg2       etime_i[3]: %d\n", etime_i[3]);
    fprintf(stream, "dbg2       etime_i[4]: %d\n", etime_i[4]);
    fprintf(stream, "dbg2       etime_i[5]: %d\n", etime_i[5]);
    fprintf(stream, "dbg2       etime_i[6]: %d\n", etime_i[6]);
    fprintf(stream, "dbg2       speedmin:   %f\n", speedmin);
    fprintf(stream, "dbg2       timegap:    %f\n", timegap);
    fprintf(stream, "dbg2       good_nav:   %d\n", good_nav_only);
    fprintf(stream, "dbg2       comments:   %d\n", comments);
    fprintf(stream, "dbg2       file:       %s\n", read_file);
    fprintf(stream, "dbg2       quick:      %d\n", quick);
    fprintf(stream, "dbg2       bathy meters:%d\n", bathy_in_meters);
    fprintf(stream, "dbg2       lonflip_set:%d\n", lonflip_set);
    fprintf(stream, "dbg2       coverage:   %d\n", coverage_mask);
    if (coverage_mask) {
      fprintf(stream, "dbg2       mask_nx:    %d\n", mask_nx);
      fprintf(stream, "dbg2       mask_ny:    %d\n", mask_ny);
    }
    if (coverage_mask_bounds) {
      fprintf(stream, "dbg2       maskbounds[0]: %f\n", maskbounds[0]);
      fprintf(stream, "dbg2       maskbounds[1]: %f\n", maskbounds[1]);
      fprintf(stream, "dbg2       maskbounds[2]: %f\n", maskbounds[2]);
      fprintf(stream, "dbg2       maskbounds[3]: %f\n", maskbounds[3]);
    }
  }

  if (help) {
    constexpr char help_message[] =
        "MBINFO reads a swath sonar data file and outputs\n"
              "some basic statistics.  If pings are averaged (pings > 2)\n"
              "MBINFO estimates the variance for each of the swath\n"
              "beams by reading a set number of pings (>2) and then finding\n"
              "the variance of the detrended values for each beam.\n"
              "The results are dumped to stdout.";
    fprintf(stream, "\n%s\n", help_message);
    fprintf(stream, "\nusage: %s\n", usage_message);
    exit(MB_ERROR_NO_ERROR);
  }

  int timbeg_i[7];
  int timend_i[7];
  for (int i = 0; i < 7; i++) {
    timbeg_i[i] = 0;
    timend_i[i] = 0;
  }
  int notice_list[MB_NOTICE_MAX];
  int notice_list_tot[MB_NOTICE_MAX];
  for (int i = 0; i < MB_NOTICE_MAX; i++) {
    notice_list[i] = 0;
    notice_list_tot[i] = 0;
  }

  int error = MB_ERROR_NO_ERROR;
  if (format == 0)
    mb_get_format(verbose, read_file, nullptr, &format, &error);

  double bathy_scale;
  if (bathy_in_meters)
    bathy_scale = 1.0;
  else
    bathy_scale = 1.0 / 0.3048;

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;

  /* if reading from datalist or reading in quick mode then variance 
      calculations are disabled */
  if (read_datalist || quick)
    pings_read = 1;

  /* output stream for basic stuff (stdout if verbose <= 1,
      output if verbose > 1) */
  FILE *output = nullptr;

  /* Open output file if requested */
  if (output_usefile) {
    char output_file[MB_PATH_MAXLINE];
    strcpy(output_file, read_file);
    switch (output_format) {
    case FREE_TEXT:
      strcat(output_file, ".inf");
      break;
    case JSON:
      strcat(output_file, "_inf.json");
      break;
    case XML:
      strcat(output_file, "_inf.xml");
      break;
    default:
      break;
    }
    if ((output = fopen(output_file, "w")) == nullptr)
      output = stream;
  }
  else {
    output = stream;
  }
  switch (output_format) {
  case FREE_TEXT:
    break;
  case JSON:
    fprintf(output, "{\n");
    break;
  case XML:
    fprintf(output, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(output, "<mbinfo>\n");
    break;
  default:
    break;
  }

  char format_description[MB_DESCRIPTION_LENGTH];

  int pings;
  double file_weight;
  double btime_d;
  double etime_d;
  char dpath[MB_PATH_MAXLINE];
  int beams_bath_alloc = 0;
  int beams_amp_alloc = 0;
  int pixels_ss_alloc = 0;
  int beams_bath_max = 0;
  int beams_amp_max = 0;
  int pixels_ss_max = 0;
  int beams_bath = 0;
  int beams_amp = 0;
  int pixels_ss = 0;

  /* MBIO read values */
  struct ping data[MBINFO_MAXPINGS];
  struct ping *datacur;
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
  double *bathlon = nullptr;
  double *bathlat = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *sslon = nullptr;
  double *sslat = nullptr;
  char comment[MB_COMMENT_MAXLINE];
  int icomment = 0;

  /* mbinfo control parameters */
  double speed_threshold = 50.0;

  /* limit variables */
  double lonmin = 0.0;
  double lonmax = 0.0;
  double latmin = 0.0;
  double latmax = 0.0;
  double sdpmin = 0.0;
  double sdpmax = 0.0;
  double altmin = 0.0;
  double altmax = 0.0;
  double bathmin = 0.0;
  double bathmax = 0.0;
  double ampmin = 0.0;
  double ampmax = 0.0;
  double ssmin = 0.0;
  double ssmax = 0.0;
  double bathbeg = 0.0;
  double bathend = 0.0;
  double lonbeg = 0.0;
  double latbeg = 0.0;
  double lonend = 0.0;
  double latend = 0.0;
  double spdbeg = 0.0;
  double hdgbeg = 0.0;
  double sdpbeg = 0.0;
  double altbeg = 0.0;
  double spdend = 0.0;
  double hdgend = 0.0;
  double sdpend = 0.0;
  double altend = 0.0;
  double timbeg = 0.0;
  double timend = 0.0;
  int timbeg_j[5];
  int timend_j[5];
  double timtot = 0.0;
  double distot = 0.0;
  double spdavg = 0.0;
  int irec = 0;
  int isbtmrec = 0;
  double timbegfile = 0.0;
  double timendpath = 0.0;
  double distotfile = 0.0;
  double timtotfile = 0.0;
  double spdavgfile = 0.0;
  int irecfile = 0;
  int ntdbeams = 0;
  int ngdbeams = 0;
  int nzdbeams = 0;
  int nfdbeams = 0;
  int ntabeams = 0;
  int ngabeams = 0;
  int nzabeams = 0;
  int nfabeams = 0;
  int ntsbeams = 0;
  int ngsbeams = 0;
  int nzsbeams = 0;
  int nfsbeams = 0;
  bool beginnav = false;
  bool beginsdp = false;
  bool beginalt = false;
  bool beginbath = false;
  bool beginamp = false;
  bool beginss = false;
  int nread = 0;

  /* variance finding variables */
  int namp;
  int nss;
  double delta;
  double a, b, dev, mean;
  double *bathmean = nullptr;
  double *bathvar = nullptr;
  int *nbathvar = nullptr;
  double *ampmean = nullptr;
  double *ampvar = nullptr;
  int *nampvar = nullptr;
  double *ssmean = nullptr;
  double *ssvar = nullptr;
  int *nssvar = nullptr;
  int nbathtot_alloc = 0;
  int namptot_alloc = 0;
  int nsstot_alloc = 0;
  double *bathmeantot = nullptr;
  double *bathvartot = nullptr;
  int *nbathvartot = nullptr;
  double *ampmeantot = nullptr;
  double *ampvartot = nullptr;
  int *nampvartot = nullptr;
  double *ssmeantot = nullptr;
  double *ssvartot = nullptr;
  int *nssvartot = nullptr;

  /* coverage mask variables */
  double mask_dx = 0.0;
  double mask_dy = 0.0;
  int *mask = nullptr;

  char *fileprint;
  char string[500];

  double speed_apparent;
  double time_d_last = 0.0;
  double val_double;

  /* read only once unless coverage mask requested */
  int pass = 0;

  /* metadata controls */
  int imetadata = 0;
  int meta_vessel = 0;
  int meta_institution = 0;
  int meta_platform = 0;
  int meta_sonar = 0;
  int meta_sonarversion = 0;
  int meta_cruiseid = 0;
  int meta_cruisename = 0;
  int meta_pi = 0;
  int meta_piinstitution = 0;
  int meta_client = 0;
  int meta_svcorrected = 0;
  int meta_tidecorrected = 0;
  int meta_batheditmanual = 0;
  int meta_batheditauto = 0;
  int meta_rollbias = 0;
  int meta_pitchbias = 0;
  int meta_headingbias = 0;
  int meta_draft = 0;

  void *datalist;

  /* if quick not set read data normally */
  if (!quick) {
    bool done = false;
    while (!done) {
      /* open file list */
      char path[MB_PATH_MAXLINE];
      char ppath[MB_PATH_MAXLINE] = "";
      char apath[MB_PATH_MAXLINE] = "";
      char dpath[MB_PATH_MAXLINE] = "";
      int pstatus;
      int astatus;
      if (read_datalist) {
        const int look_processed = MB_DATALIST_LOOK_UNSET;
        if (mb_datalist_open(verbose, &datalist, read_file, look_processed, &error) != MB_SUCCESS) {
          fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(MB_ERROR_OPEN_FAIL);
        }
        read_data = mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, 
                                  &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS;
      } else {
        // else copy single filename to be read
        strcpy(path, read_file);
        read_data = true;
      }

      /* loop over all files to be read */
      while (read_data) {

        void *mbio_ptr = nullptr;
        /* initialize reading the swath file */
        if (mb_read_init_altnav(verbose, path, format, pings_get, lonflip, bounds, btime_i, etime_i, speedmin, timegap,
                                   astatus, apath, &mbio_ptr, &btime_d, &etime_d, &beams_bath_alloc, &beams_amp_alloc, &pixels_ss_alloc,
                                   &error) != MB_SUCCESS) {
          char *message;
          mb_error(verbose, error, &message);
          fprintf(stream, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
          fprintf(stream, "\nSwath File <%s> not initialized for reading\n", path);
          fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }

        /* allocate memory for data arrays */
        memset(data, 0, MBINFO_MAXPINGS * sizeof(struct ping));
        for (int i = 0; i < pings_read; i++) {
          datacur = &data[i];
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                       (void **)&datacur->beamflag, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&datacur->bath,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&datacur->amp,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                       (void **)&datacur->bathlon, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                       (void **)&datacur->bathlat, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&datacur->ss, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&datacur->sslon,
                                       &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&datacur->sslat,
                                       &error);
        }
        if (pings_read > 1 && pass == 0) {
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathmean, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bathvar, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&nbathvar, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ampmean, &error);
          if (error == MB_ERROR_NO_ERROR)
            status =
                mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&ampvar, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(int), (void **)&nampvar, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssmean, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssvar, &error);
          if (error == MB_ERROR_NO_ERROR)
            status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(int), (void **)&nssvar, &error);
        }

        /* if coverage mask requested get cell sizes */
        if (coverage_mask && (coverage_mask_bounds || pass == 1)) {
          if (pass == 1) {
            maskbounds[0] = lonmin;
            maskbounds[1] = lonmax;
            maskbounds[2] = latmin;
            maskbounds[3] = latmax;
          }
          if (mask_nx > 1 && mask_ny <= 0) {
            if ((maskbounds[1] - maskbounds[0]) > (maskbounds[3] - maskbounds[2])) {
              mask_ny = mask_nx * (maskbounds[3] - maskbounds[2]) / (maskbounds[1] - maskbounds[0]);
            }
            else {
              mask_ny = mask_nx;
              mask_nx = mask_ny * (maskbounds[1] - maskbounds[0]) / (maskbounds[3] - maskbounds[2]);
              if (mask_ny < 2)
                mask_ny = 2;
            }
          }
          if (mask_nx < 2)
            mask_nx = 2;
          if (mask_ny < 2)
            mask_ny = 2;
          mask_dx = (maskbounds[1] - maskbounds[0]) / mask_nx;
          mask_dy = (maskbounds[3] - maskbounds[2]) / mask_ny;

          /* allocate mask */
          status = mb_mallocd(verbose, __FILE__, __LINE__, mask_nx * mask_ny * sizeof(int), (void **)&mask, &error);
        }

        /* if error initializing memory then quit */
        if (error != MB_ERROR_NO_ERROR) {
          char *message;
          mb_error(verbose, error, &message);
          fprintf(stream, "\nMBIO Error allocating data arrays:\n%s\n", message);
          fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }

        /* initialize data arrays */
        irecfile = 0;
        distotfile = 0.0;
        timtotfile = 0.0;
        spdavgfile = 0.0;
        if (pass == 0 && pings_read > 1) {
          for (int i = 0; i < beams_bath_alloc; i++) {
            bathmean[i] = 0.0;
            bathvar[i] = 0.0;
            nbathvar[i] = 0;
          }
          for (int i = 0; i < beams_amp_alloc; i++) {
            ampmean[i] = 0.0;
            ampvar[i] = 0.0;
            nampvar[i] = 0;
          }
          for (int i = 0; i < pixels_ss_alloc; i++) {
            ssmean[i] = 0.0;
            ssvar[i] = 0.0;
            nssvar[i] = 0;
          }
        }
        if (coverage_mask && (coverage_mask_bounds || pass == 1)) {
          for (int i = 0; i < mask_nx * mask_ny; i++)
            mask[i] = false;
        }

        /* initialize metadata counters */
        meta_vessel = 0;
        meta_institution = 0;
        meta_platform = 0;
        meta_sonar = 0;
        meta_sonarversion = 0;
        meta_cruiseid = 0;
        meta_cruisename = 0;
        meta_pi = 0;
        meta_piinstitution = 0;
        meta_client = 0;
        meta_svcorrected = 0;
        meta_tidecorrected = 0;
        meta_batheditmanual = 0;
        meta_batheditauto = 0;
        meta_rollbias = 0;
        meta_pitchbias = 0;
        meta_headingbias = 0;
        meta_draft = 0;

        /* printf out file and format */
        if (pass == 0) {
          if (strrchr(path, '/') == nullptr)
            fileprint = path;
          else
            fileprint = strrchr(path, '/') + 1;
          mb_format_description(verbose, &format, format_description, &error);
          switch (output_format) {
          case FREE_TEXT:
            fprintf(output, "\nSwath Data File:      %s\n", fileprint);
            fprintf(output, "MBIO Data Format ID:  %d\n", format);
            fprintf(output, "%s", format_description);
            break;
          case JSON:
          {
            fprintf(output, "\"file_info\": {\n");
            fprintf(output, "\"swath_data_file\": \"%s\",\n", fileprint);
            fprintf(output, "\"mbio_data_format_id\": \"%d\",\n", format);
            size_t len1 = strspn(format_description, "Formatname: ");
            size_t len2 = strcspn(&format_description[len1], "\n");
            strncpy(string, &format_description[len1], len2);
            string[len2] = '\0';
            fprintf(output, "\"format_name\": \"%s\",\n", string);
            len1 += len2 + 1;
            len1 += strspn(&format_description[len1], "InformalDescription: ");
            len2 = strcspn(&format_description[len1], "\n");
            strncpy(string, &format_description[len1], len2);
            string[len2] = '\0';
            fprintf(output, "\"informal_description\": \"%s\",\n", string);
            len1 += len2 + 1;
            len1 += strspn(&format_description[len1], "Attributes: ");
            // len2 = strlen(format_description);
            format_description[strlen(format_description) - 1] = '\0';
            for (len2 = len1; len2 <= strlen(format_description); len2++)
              if (format_description[len2] == 10)
                format_description[len2] = ';';
            fprintf(output, "\"attributes\": \"%s\"\n", &format_description[len1]);
            fprintf(output, "},\n");
            break;
          }
          case XML:
          {
            fprintf(output, "\t<file_info>\n");
            fprintf(output, "\t\t<swath_data_file>%s</swath_data_file>\n", fileprint);
            fprintf(output, "\t\t<mbio_data_format_id>%d</mbio_data_format_id>\n", format);
            size_t len1 = strspn(format_description, "Formatname: ");
            size_t len2 = strcspn(&format_description[len1], "\n");
            strncpy(string, &format_description[len1], len2);
            string[len2] = '\0';
            fprintf(output, "\t\t<format_name>%s</format_name>\n", string);
            len1 += len2 + 1;
            len1 += strspn(&format_description[len1], "InformalDescription: ");
            len2 = strcspn(&format_description[len1], "\n");
            strncpy(string, &format_description[len1], len2);
            string[len2] = '\0';
            fprintf(output, "\t\t<informal_description>%s</informal_description>\n", string);
            len1 += len2 + 1;
            len1 += strspn(&format_description[len1], "Attributes: ");
            // len2 = strlen(format_description);
            format_description[strlen(format_description) - 1] = '\0';
            for (len2 = len1; len2 <= strlen(format_description); len2++)
              if (format_description[len2] == 10)
                format_description[len2] = ' ';
            fprintf(output, "\t\t<attributes>%s</attributes>\n", &format_description[len1]);
            fprintf(output, "\t</file_info>\n");
            break;
          }
          default:
            errflg = true;
          }
        }

        /* read and process data */
        while (error <= MB_ERROR_NO_ERROR) {
          nread = 0;
          error = MB_ERROR_NO_ERROR;
          while (nread < pings_read && error == MB_ERROR_NO_ERROR) {

            int kind;
            /* read a ping of data */
            datacur = &data[nread];
            status = mb_read(verbose, mbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                             &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, datacur->beamflag,
                             datacur->bath, datacur->amp, datacur->bathlon, datacur->bathlat, datacur->ss, datacur->sslon,
                             datacur->sslat, comment, &error);

            /* use local pointers for convenience - do not set these before the
                mb_read call because registered arrays can be dynamically
                reallocated during mb_read, mb_get, and mb_get_all calls */
            beamflag = datacur->beamflag;
            bath = datacur->bath;
            amp = datacur->amp;
            bathlon = datacur->bathlon;
            bathlat = datacur->bathlat;
            ss = datacur->ss;
            sslon = datacur->sslon;
            sslat = datacur->sslat;

            /* increment counters */
            if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
              irec++;
              irecfile++;
              nread++;
            }

            /* print comment records */
            if (pass == 0 && error == MB_ERROR_COMMENT && comments) {
              if (strncmp(comment, "META", 4) != 0) {
                if (icomment == 0) {
                  switch (output_format) {
                  case FREE_TEXT:
                    fprintf(output, "\nComments in file %s:\n", path);
                    icomment++;
                    break;
                  case JSON:
                  case XML:
                  default:
                    break;
                  }
                }
                switch (output_format) {
                case FREE_TEXT:
                  fprintf(output, "  %s\n", comment);
                  break;
                case JSON:
                  fprintf(output, "\"comment\": \"%s\",\n", comment);
                  break;
                case XML:
                  fprintf(output, "\t<comment>%s</comment>\n", comment);
                  break;
                default:
                  break;
                }
              }
            }

            /* print metadata */
            if (pass == 0 && error == MB_ERROR_COMMENT && strncmp(comment, "META", 4) == 0) {
              switch (output_format) {
              case FREE_TEXT:
                if (imetadata == 0) {
                  fprintf(output, "\nMetadata:\n");
                  imetadata++;
                }
                if (strncmp(comment, "METAVESSEL:", 11) == 0) {
                  if (meta_vessel == 0)
                    fprintf(output, "Vessel:                 %s\n", &comment[11]);
                  meta_vessel++;
                }
                else if (strncmp(comment, "METAINSTITUTION:", 16) == 0) {
                  if (meta_institution == 0)
                    fprintf(output, "Institution:            %s\n", &comment[16]);
                  meta_institution++;
                }
                else if (strncmp(comment, "METAPLATFORM:", 13) == 0) {
                  if (meta_platform == 0)
                    fprintf(output, "Platform:               %s\n", &comment[13]);
                  meta_platform++;
                }
                else if (strncmp(comment, "METASONARVERSION:", 17) == 0) {
                  if (meta_sonarversion == 0)
                    fprintf(output, "Sonar Version:          %s\n", &comment[17]);
                  meta_sonarversion++;
                }
                else if (strncmp(comment, "METASONAR:", 10) == 0) {
                  if (meta_sonar == 0)
                    fprintf(output, "Sonar:                  %s\n", &comment[10]);
                  meta_sonar++;
                }
                else if (strncmp(comment, "METACRUISEID:", 13) == 0) {
                  if (meta_cruiseid == 0)
                    fprintf(output, "Cruise ID:              %s\n", &comment[13]);
                  meta_cruiseid++;
                }
                else if (strncmp(comment, "METACRUISENAME:", 15) == 0) {
                  if (meta_cruisename == 0)
                    fprintf(output, "Cruise Name:            %s\n", &comment[15]);
                  meta_cruisename++;
                }
                else if (strncmp(comment, "METAPI:", 7) == 0) {
                  if (meta_pi == 0)
                    fprintf(output, "PI:                     %s\n", &comment[7]);
                  meta_pi++;
                }
                else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0) {
                  if (meta_piinstitution == 0)
                    fprintf(output, "PI Institution:         %s\n", &comment[18]);
                  meta_piinstitution++;
                }
                else if (strncmp(comment, "METACLIENT:", 11) == 0) {
                  if (meta_client == 0)
                    fprintf(output, "Client:                 %s\n", &comment[11]);
                  meta_client++;
                }
                else if (strncmp(comment, "METASVCORRECTED:", 16) == 0) {
                  if (meta_svcorrected == 0) {
                    int val_int;
                    sscanf(comment, "METASVCORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "Corrected Depths:       YES\n");
                    else
                      fprintf(output, "Corrected Depths:       NO\n");
                  }
                  meta_svcorrected++;
                }
                else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) {
                  if (meta_tidecorrected == 0) {
                    int val_int;
                    sscanf(comment, "METATIDECORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "Tide Corrected:         YES\n");
                    else
                      fprintf(output, "Tide Corrected:         NO\n");
                  }
                  meta_tidecorrected++;
                }
                else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0) {
                  if (meta_batheditmanual == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITMANUAL:%d", &val_int);
                    if (val_int)
                      fprintf(output, "Depths Manually Edited: YES\n");
                    else
                      fprintf(output, "Depths Manually Edited: NO\n");
                  }
                  meta_batheditmanual++;
                }
                else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0) {
                  if (meta_batheditauto == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITAUTO:%d", &val_int);
                    if (val_int)
                      fprintf(output, "Depths Auto-Edited:     YES\n");
                    else
                      fprintf(output, "Depths Auto-Edited:     NO\n");
                  }
                  meta_batheditauto++;
                }
                else if (strncmp(comment, "METAROLLBIAS:", 13) == 0) {
                  if (meta_rollbias == 0) {
                    sscanf(comment, "METAROLLBIAS:%lf", &val_double);
                    fprintf(output, "Roll Bias:              %f degrees\n", val_double);
                  }
                  meta_rollbias++;
                }
                else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0) {
                  if (meta_pitchbias == 0) {
                    sscanf(comment, "METAPITCHBIAS:%lf", &val_double);
                    fprintf(output, "Pitch Bias:             %f degrees\n", val_double);
                  }
                  meta_pitchbias++;
                }
                else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0) {
                  if (meta_headingbias == 0) {
                    sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);
                    fprintf(output, "Heading Bias:           %f degrees\n", val_double);
                  }
                  meta_headingbias++;
                }
                else if (strncmp(comment, "METADRAFT:", 10) == 0) {
                  if (meta_draft == 0) {
                    sscanf(comment, "METADRAFT:%lf", &val_double);
                    fprintf(output, "Draft:                  %f m\n", val_double);
                  }
                  meta_draft++;
                }
                break;
              case JSON:
                if (strncmp(comment, "METAVESSEL:", 11) == 0) {
                  if (meta_vessel == 0)
                    fprintf(output, "\"vessel\":\"%s\",\n", &comment[11]);
                  meta_vessel++;
                }
                else if (strncmp(comment, "METAINSTITUTION:", 16) == 0) {
                  if (meta_institution == 0)
                    fprintf(output, "\"institution\":\"%s\",\n", &comment[16]);
                  meta_institution++;
                }
                else if (strncmp(comment, "METAPLATFORM:", 13) == 0) {
                  if (meta_platform == 0)
                    fprintf(output, "\"platform\": \"%s \",\n", &comment[13]);
                  meta_platform++;
                }
                else if (strncmp(comment, "METASONARVERSION:", 17) == 0) {
                  if (meta_sonarversion == 0)
                    fprintf(output, "\"sonar_version\": \"%s\",\n", &comment[17]);
                  meta_sonarversion++;
                }
                else if (strncmp(comment, "METASONAR:", 10) == 0) {
                  if (meta_sonar == 0)
                    fprintf(output, "\"sonar\": \"%s\",\n", &comment[10]);
                  meta_sonar++;
                }
                else if (strncmp(comment, "METACRUISEID:", 13) == 0) {
                  if (meta_cruiseid == 0)
                    fprintf(output, "\"cruise_id\": \"%s\",\n", &comment[13]);
                  meta_cruiseid++;
                }
                else if (strncmp(comment, "METACRUISENAME:", 15) == 0) {
                  if (meta_cruisename == 0)
                    fprintf(output, "\"cruise_name\": \"%s\",\n", &comment[15]);
                  meta_cruisename++;
                }
                else if (strncmp(comment, "METAPI:", 7) == 0) {
                  if (meta_pi == 0)
                    fprintf(output, "\"pi\": \"%s\",\n", &comment[7]);
                  meta_pi++;
                }
                else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0) {
                  if (meta_piinstitution == 0)
                    fprintf(output, "\"pi_institution\": \"%s\",\n", &comment[18]);
                  meta_piinstitution++;
                }
                else if (strncmp(comment, "METACLIENT:", 11) == 0) {
                  if (meta_client == 0)
                    fprintf(output, "\"client\": \"%s\",\n", &comment[11]);
                  meta_client++;
                }
                else if (strncmp(comment, "METASVCORRECTED:", 16) == 0) {
                  if (meta_svcorrected == 0) {
                    int val_int;
                    sscanf(comment, "METASVCORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\"corrected_depths\": \"YES\",\n");
                    else
                      fprintf(output, "\"corrected_depths\": \"NO\",\n");
                  }
                  meta_svcorrected++;
                }
                else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) {
                  if (meta_tidecorrected == 0) {
                    int val_int;
                    sscanf(comment, "METATIDECORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\"tide_corrected\": \"YES\",\n");
                    else
                      fprintf(output, "\"tide_corrected\": \"NO\",\n");
                  }
                  meta_tidecorrected++;
                }
                else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0) {
                  if (meta_batheditmanual == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITMANUAL:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\"depths_manually_edited\": \"YES\",\n");
                    else
                      fprintf(output, "\"depths_manually_edited\": \"NO\",\n");
                  }
                  meta_batheditmanual++;
                }
                else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0) {
                  if (meta_batheditauto == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITAUTO:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\"depths_auto-edited\": \"YES\",\n");
                    else
                      fprintf(output, "\"depths_auto-edited\": \"NO\",\n");
                  }
                  meta_batheditauto++;
                }
                else if (strncmp(comment, "METAROLLBIAS:", 13) == 0) {
                  if (meta_rollbias == 0) {
                    sscanf(comment, "METAROLLBIAS:%lf", &val_double);
                    fprintf(output, "\"roll_bias\": \"%f\",\n", val_double);
                  }
                  meta_rollbias++;
                }
                else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0) {
                  if (meta_pitchbias == 0) {
                    sscanf(comment, "METAPITCHBIAS:%lf", &val_double);
                    fprintf(output, "\"pitch_bias\": \"%f\",\n", val_double);
                  }
                  meta_pitchbias++;
                }
                else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0) {
                  if (meta_headingbias == 0) {
                    sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);
                    fprintf(output, "\"heading_bias\": \"%f\",\n", val_double);
                  }
                  meta_headingbias++;
                }
                else if (strncmp(comment, "METADRAFT:", 10) == 0) {
                  if (meta_draft == 0) {
                    sscanf(comment, "METADRAFT:%lf", &val_double);
                    fprintf(output, "\"draft\": \"%f\",\n", val_double);
                  }
                  meta_draft++;
                }
                break;
              case XML:
                if (imetadata == 0) {
                  fprintf(output, "\t<metadata>\n");
                  imetadata++;
                }
                if (strncmp(comment, "METAVESSEL:", 11) == 0) {
                  if (meta_vessel == 0)
                    fprintf(output, "\t\t<vessel>%s</vessel>\n", &comment[11]);
                  meta_vessel++;
                }
                else if (strncmp(comment, "METAINSTITUTION:", 16) == 0) {
                  if (meta_institution == 0)
                    fprintf(output, "\t\t<institution>%s</institution>\n", &comment[16]);
                  meta_institution++;
                }
                else if (strncmp(comment, "METAPLATFORM:", 13) == 0) {
                  if (meta_platform == 0)
                    fprintf(output, "\t\t<platform>%s</platform>\n", &comment[13]);
                  meta_platform++;
                }
                else if (strncmp(comment, "METASONARVERSION:", 17) == 0) {
                  if (meta_sonarversion == 0)
                    fprintf(output, "\t\t<sonar_version>%s</sonar_version>\n", &comment[17]);
                  meta_sonarversion++;
                }
                else if (strncmp(comment, "METASONAR:", 10) == 0) {
                  if (meta_sonar == 0)
                    fprintf(output, "\t\t<sonar>%s</sonar>\n", &comment[10]);
                  meta_sonar++;
                }
                else if (strncmp(comment, "METACRUISEID:", 13) == 0) {
                  if (meta_cruiseid == 0)
                    fprintf(output, "\t\t<cruise_id>%s</cruise_id>\n", &comment[13]);
                  meta_cruiseid++;
                }
                else if (strncmp(comment, "METACRUISENAME:", 15) == 0) {
                  if (meta_cruisename == 0)
                    fprintf(output, "\t\t<cruise_name>%s</cruise_name>\n", &comment[15]);
                  meta_cruisename++;
                }
                else if (strncmp(comment, "METAPI:", 7) == 0) {
                  if (meta_pi == 0)
                    fprintf(output, "\t\t<pi>%s</pi>\n", &comment[7]);
                  meta_pi++;
                }
                else if (strncmp(comment, "METAPIINSTITUTION:", 18) == 0) {
                  if (meta_piinstitution == 0)
                    fprintf(output, "\t\t<pi_institution>%s</pi_institution>\n", &comment[18]);
                  meta_piinstitution++;
                }
                else if (strncmp(comment, "METACLIENT:", 11) == 0) {
                  if (meta_client == 0)
                    fprintf(output, "\t\t<client>%s</client>\n", &comment[11]);
                  meta_client++;
                }
                else if (strncmp(comment, "METASVCORRECTED:", 16) == 0) {
                  if (meta_svcorrected == 0) {
                    int val_int;
                    sscanf(comment, "METASVCORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\t\t<corrected_depths>YES</corrected_depths>\n");
                    else
                      fprintf(output, "\t\t<corrected_depths>NO</corrected_depths>\n");
                  }
                  meta_svcorrected++;
                }
                else if (strncmp(comment, "METATIDECORRECTED:", 18) == 0) {
                  if (meta_tidecorrected == 0) {
                    int val_int;
                    sscanf(comment, "METATIDECORRECTED:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\t\t<tide_corrected>YES</tide_corrected>\n");
                    else
                      fprintf(output, "\t\t<tide_corrected>NO</tide_corrected>\n");
                  }
                  meta_tidecorrected++;
                }
                else if (strncmp(comment, "METABATHEDITMANUAL:", 19) == 0) {
                  if (meta_batheditmanual == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITMANUAL:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\t\t<depths_manually_edited>YES</depths_manually_edited>\n");
                    else
                      fprintf(output, "\t\t<depths_manually_edited>NO</depths_manually_edited>\n");
                  }
                  meta_batheditmanual++;
                }
                else if (strncmp(comment, "METABATHEDITAUTO:", 17) == 0) {
                  if (meta_batheditauto == 0) {
                    int val_int;
                    sscanf(comment, "METABATHEDITAUTO:%d", &val_int);
                    if (val_int)
                      fprintf(output, "\t\t<depths_auto_edited>YES</depths_auto_edited>\n");
                    else
                      fprintf(output, "\t\t<depths_auto_edited>NO</depths_auto_edited>\n");
                  }
                  meta_batheditauto++;
                }
                else if (strncmp(comment, "METAROLLBIAS:", 13) == 0) {
                  if (meta_rollbias == 0) {
                    sscanf(comment, "METAROLLBIAS:%lf\n", &val_double);
                    fprintf(output, "\t\t<roll_bias>%f</roll_bias>\n", val_double);
                  }
                  meta_rollbias++;
                }
                else if (strncmp(comment, "METAPITCHBIAS:", 14) == 0) {
                  if (meta_pitchbias == 0) {
                    sscanf(comment, "METAPITCHBIAS:%lf", &val_double);
                    fprintf(output, "\t\t<pitch_bias>%f</pitch_bias>\n", val_double);
                  }
                  meta_pitchbias++;
                }
                else if (strncmp(comment, "METAHEADINGBIAS:", 16) == 0) {
                  if (meta_headingbias == 0) {
                    sscanf(comment, "METAHEADINGBIAS:%lf", &val_double);
                    fprintf(output, "\t\t<heading_bias>%f</heading_bias>\n", val_double);
                  }
                  meta_headingbias++;
                }
                else if (strncmp(comment, "METADRAFT:", 10) == 0) {
                  if (meta_draft == 0) {
                    sscanf(comment, "METADRAFT:%lf", &val_double);
                    fprintf(output, "\t\t<draft>%fm</draft>\n\t</metadata>\n", val_double);
                  }
                  meta_draft++;
                }
                break;
              default:
                break;
              }
            }

            /* output error messages */
            if (pass != 0 || error == MB_ERROR_COMMENT) {
              /* do nothing */
            }
            else if (error == MB_ERROR_SUBBOTTOM) {
              /* do nothing */
            }
            else if (verbose >= 1 && error < MB_ERROR_NO_ERROR && error >= MB_ERROR_OTHER) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nNonfatal MBIO Error:\n%s\n", message);
              fprintf(stream, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
                      time_i[5], time_i[6]);
            }
            else if (verbose >= 1 && error < MB_ERROR_NO_ERROR) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nNonfatal MBIO Error:\n%s\n", message);
              fprintf(stream, "Number of good records so far: %d\n", irecfile);
            }
            else if (verbose >= 1 && error > MB_ERROR_NO_ERROR && error != MB_ERROR_EOF) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nFatal MBIO Error:\n%s\n", message);
              fprintf(stream, "Last Good Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3],
                      time_i[4], time_i[5], time_i[6]);
            }

            /* take note of min and maxes */
            beams_bath_max = std::max(beams_bath_max, beams_bath);
            beams_amp_max = std::max(beams_amp_max, beams_amp);
            pixels_ss_max = std::max(pixels_ss_max, pixels_ss);
            if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
              /* update data counts */
              ntdbeams += beams_bath;
              ntabeams += beams_amp;
              ntsbeams += pixels_ss;

              /* set lonflip if needed */
              if (!lonflip_set && (navlon != 0.0 || navlat != 0.0)) {
                lonflip_set = true;
                if (navlon < -270.0)
                  lonflip_use = 0;
                else if (navlon >= -270.0 && navlon < -90.0)
                  lonflip_use = -1;
                else if (navlon >= -90.0 && navlon < 90.0)
                  lonflip_use = 0;
                else if (navlon >= 90.0 && navlon < 270.0)
                  lonflip_use = 1;
                else if (navlon >= 270.0)
                  lonflip_use = 0;

                /* change and apply lonflip if needed */
                if (lonflip_use != lonflip) {
                  /* change lonflip used in reading */
                  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
                  mb_io_ptr->lonflip = lonflip_use;
                  lonflip = lonflip_use;

                  /* apply lonflip to data already read */
                  if (lonflip_use == -1) {
                    if (navlon > 0.0)
                      navlon -= 360.0;
                    for (int i = 0; i < beams_bath; i++) {
                      if (bathlon[i] > 0.0)
                        bathlon[i] -= 360.0;
                    }
                    for (int i = 0; i < pixels_ss; i++) {
                      if (sslon[i] > 0.0)
                        sslon[i] -= 360.0;
                    }
                  }
                  else if (lonflip_use == 1) {
                    if (navlon < 0.0)
                      navlon += 360.0;
                    for (int i = 0; i < beams_bath; i++) {
                      if (bathlon[i] < 0.0)
                        bathlon[i] += 360.0;
                    }
                    for (int i = 0; i < pixels_ss; i++) {
                      if (sslon[i] < 0.0)
                        sslon[i] += 360.0;
                    }
                  }
                  else if (lonflip_use == 0) {
                    if (navlon < -180.0)
                      navlon += 360.0;
                    if (navlon > 180.0)
                      navlon -= 360.0;
                    for (int i = 0; i < beams_bath; i++) {
                      if (bathlon[i] < -180.0)
                        bathlon[i] += 360.0;
                      if (bathlon[i] > 180.0)
                        bathlon[i] -= 360.0;
                    }
                    for (int i = 0; i < pixels_ss; i++) {
                      if (sslon[i] < -180.0)
                        sslon[i] += 360.0;
                      if (sslon[i] > 180.0)
                        sslon[i] -= 360.0;
                    }
                  }
                }
              }

              /* get beginning values */
              if (irec == 1) {
                if (beams_bath > 0) {
                  if (mb_beam_ok(beamflag[beams_bath / 2]))
                    bathbeg = bath[beams_bath / 2];
                  else
                    bathbeg = altitude + sensordepth;
                }
                lonbeg = navlon;
                latbeg = navlat;
                timbeg = time_d;
                timbegfile = time_d;
                for (int i = 0; i < 7; i++)
                  timbeg_i[i] = time_i[i];
                spdbeg = speed;
                hdgbeg = heading;
                sdpbeg = sensordepth;
                altbeg = altitude;
              }
              else if (good_nav_only) {
                if (lonbeg == 0.0 && latbeg == 0.0 && navlon != 0.0 && navlat != 0.0) {
                  lonbeg = navlon;
                  if (beams_bath > 0) {
                    if (mb_beam_ok(beamflag[beams_bath / 2]))
                      bathbeg = bath[beams_bath / 2];
                    else
                      bathbeg = altitude + sensordepth;
                  }
                  latbeg = navlat;
                  if (spdbeg == 0.0 && speed != 0.0)
                    spdbeg = speed;
                  if (hdgbeg == 0.0 && heading != 0.0)
                    hdgbeg = heading;
                  if (sdpbeg == 0.0 && sensordepth != 0.0)
                    sdpbeg = sensordepth;
                  if (altbeg == 0.0 && altitude != 0.0)
                    altbeg = altitude;
                }
              }


              /* reset ending values each time */
              if (beams_bath > 0) {
                if (mb_beam_ok(beamflag[beams_bath / 2]))
                  bathend = bath[beams_bath / 2];
                else
                  bathend = altitude + sensordepth;
              }
              lonend = navlon;
              latend = navlat;
              spdend = speed;
              hdgend = heading;
              sdpend = sensordepth;
              altend = altitude;
              timend = time_d;
              timendpath = time_d;
              for (int i = 0; i < 7; i++)
                timend_i[i] = time_i[i];

              /* check for good nav */
              speed_apparent = 3600.0 * distance / (time_d - time_d_last);
              bool good_nav = true;
              if (good_nav_only) {
                if ((navlon > -0.005 && navlon < 0.005) && (navlat > -0.005 && navlat < 0.005)) {
                  good_nav = false;
                }
                else if (beginnav && speed_apparent >= speed_threshold) {
                  good_nav = false;
                } // else { good_nav = true; }
              } // else { good_nav = true; }

              /* get total distance */
              if (!good_nav_only || (good_nav && speed_apparent < speed_threshold)) {
                distot += distance;
                distotfile += distance;
              }

              /* get starting mins and maxs */
              if (!beginnav && good_nav) {
                lonmin = navlon;
                lonmax = navlon;
                latmin = navlat;
                latmax = navlat;
                beginnav = true;
              }
              if (!beginsdp && sensordepth > 0.0) {
                sdpmin = sensordepth;
                sdpmax = sensordepth;
                beginsdp = true;
              }
              if (!beginalt && altitude > 0.0) {
                altmin = altitude;
                altmax = altitude;
                beginalt = true;
              }
              if (!beginbath && beams_bath > 0)
                for (int i = 0; i < beams_bath; i++)
                  if (mb_beam_ok(beamflag[i])) {
                    bathmin = bath[i];
                    bathmax = bath[i];
                    beginbath = true;
                  }
              if (!beginamp && beams_amp > 0)
                for (int i = 0; i < beams_amp; i++)
                  if (mb_beam_ok(beamflag[i])) {
                    ampmin = amp[i];
                    ampmax = amp[i];
                    beginamp = true;
                  }
              if (!beginss && pixels_ss > 0)
                for (int i = 0; i < pixels_ss; i++)
                  if (ss[i] > MB_SIDESCAN_NULL) {
                    ssmin = ss[i];
                    ssmax = ss[i];
                    beginss = true;
                  }

              /* get mins and maxs */
              if (good_nav && beginnav) {
                lonmin = std::min(lonmin, navlon);
                lonmax = std::max(lonmax, navlon);
                latmin = std::min(latmin, navlat);
                latmax = std::max(latmax, navlat);
              }
              if (beginsdp) {
                sdpmin = std::min(sdpmin, sensordepth);
                sdpmax = std::max(sdpmax, sensordepth);
              }
              if (beginalt) {
                altmin = std::min(altmin, altitude);
                altmax = std::max(altmax, altitude);
              }
              for (int i = 0; i < beams_bath; i++) {
                if (mb_beam_ok(beamflag[i])) {
                  if (good_nav && beginnav) {
                    lonmin = std::min(lonmin, bathlon[i]);
                    lonmax = std::max(lonmax, bathlon[i]);
                    latmin = std::min(latmin, bathlat[i]);
                    latmax = std::max(latmax, bathlat[i]);
                  }
                  bathmin = std::min(bathmin, bath[i]);
                  bathmax = std::max(bathmax, bath[i]);
                  ngdbeams++;
                }
                else if (beamflag[i] == MB_FLAG_NULL)
                  nzdbeams++;
                else
                  nfdbeams++;
              }
              for (int i = 0; i < beams_amp; i++) {
                if (mb_beam_ok(beamflag[i])) {
                  ampmin = std::min(ampmin, amp[i]);
                  ampmax = std::max(ampmax, amp[i]);
                  ngabeams++;
                }
                else if (beamflag[i] == MB_FLAG_NULL)
                  nzabeams++;
                else
                  nfabeams++;
              }
              for (int i = 0; i < pixels_ss; i++) {
                if (ss[i] > MB_SIDESCAN_NULL) {
                  if (good_nav && beginnav) {
                    lonmin = std::min(lonmin, sslon[i]);
                    lonmax = std::max(lonmax, sslon[i]);
                    latmin = std::min(latmin, sslat[i]);
                    latmax = std::max(latmax, sslat[i]);
                  }
                  ssmin = std::min(ssmin, ss[i]);
                  ssmax = std::max(ssmax, ss[i]);
                  ngsbeams++;
                }
                else if (ss[i] == 0.0)
                  nzsbeams++;
                else
                  nfsbeams++;
              }

              /* reset time of last ping */
              time_d_last = time_d;
            }

            /* update coverage mask */
            if ((coverage_mask && (coverage_mask_bounds || pass == 1))
                && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
              int ix = (int)((navlon - maskbounds[0]) / mask_dx);
              int iy = (int)((navlat - maskbounds[2]) / mask_dy);
              if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) {
                mask[ix + iy * mask_nx] = true;
              }
              for (int i = 0; i < beams_bath; i++) {
                if (mb_beam_ok(beamflag[i])) {
                  ix = (int)((bathlon[i] - maskbounds[0]) / mask_dx);
                  iy = (int)((bathlat[i] - maskbounds[2]) / mask_dy);
                  if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) {
                    mask[ix + iy * mask_nx] = true;
                  }
                }
              }
              for (int i = 0; i < pixels_ss; i++) {
                if (ss[i] > MB_SIDESCAN_NULL) {
                  ix = (int)((sslon[i] - maskbounds[0]) / mask_dx);
                  iy = (int)((sslat[i] - maskbounds[2]) / mask_dy);
                  if (ix >= 0 && ix < mask_nx && iy >= 0 && iy < mask_ny) {
                    mask[ix + iy * mask_nx] = true;
                  }
                }
              }
            }

            /* look for problems */
            if (pass == 0 && (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {
              if (navlon == 0.0 || navlat == 0.0)
                mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_ZERO_NAV);
              else if (beginnav && speed_apparent >= speed_threshold)
                mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_TOO_FAST);
              for (int i = 0; i < beams_bath; i++) {
                if (mb_beam_ok(beamflag[i])) {
                  if (bath[i] > 11000.0)
                    mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_TOO_DEEP);
                }
              }
            }
          }

          if (verbose >= 2) {
            fprintf(stream, "\ndbg2  Reading loop finished in program <%s>\n", program_name);
            fprintf(stream, "dbg2       status:     %d\n", status);
            fprintf(stream, "dbg2       error:      %d\n", error);
            fprintf(stream, "dbg2       nread:      %d\n", nread);
            fprintf(stream, "dbg2       pings_read: %d\n", pings_read);
          }

          /* process the pings */
          if (pass == 0 && pings_read > 2 && nread == pings_read &&
              (error == MB_ERROR_NO_ERROR || error == MB_ERROR_TIME_GAP)) {

            /* do the bathymetry */
            for (int i = 0; i < beams_bath; i++) {
              /* fit line to depths */
              int nbath = 0;
              double sumx = 0.0;
              double sumxx = 0.0;
              double sumy = 0.0;
              double sumxy = 0.0;
              for (int j = 0; j < nread; j++) {
                datacur = &data[j];
                bath = datacur->bath;
                beamflag = datacur->beamflag;
                if (mb_beam_ok(beamflag[i])) {
                  nbath++;
                  sumx = sumx + j;
                  sumxx = sumxx + j * j;
                  sumy = sumy + bath[i];
                  sumxy = sumxy + j * bath[i];
                }
              }
              if (nbath == pings_read) {
                double variance = 0.0;
                delta = nbath * sumxx - sumx * sumx;
                a = (sumxx * sumy - sumx * sumxy) / delta;
                b = (nbath * sumxy - sumx * sumy) / delta;
                for (int j = 0; j < nread; j++) {
                  datacur = &data[j];
                  bath = datacur->bath;
                  beamflag = datacur->beamflag;
                  if (mb_beam_ok(beamflag[i])) {
                    dev = bath[i] - a - b * j;
                    variance = variance + dev * dev;
                  }
                }
                bathmean[i] = bathmean[i] + sumy;
                bathvar[i] = bathvar[i] + variance;
                nbathvar[i] = nbathvar[i] + nbath;
              }
            }

            /* do the amplitude */
            for (int i = 0; i < beams_amp; i++) {

              /* get mean amplitude */
              namp = 0;
              mean = 0.0;
              for (int j = 0; j < nread; j++) {
                datacur = &data[j];
                amp = datacur->amp;
                beamflag = datacur->beamflag;
                if (mb_beam_ok(beamflag[i])) {
                  namp++;
                  mean = mean + amp[i];
                }
              }
              if (namp == pings_read) {
                double variance = 0.0;
                mean = mean / namp;
                for (int j = 0; j < nread; j++) {
                  datacur = &data[j];
                  amp = datacur->amp;
                  if (mb_beam_ok(beamflag[i])) {
                    dev = amp[i] - mean;
                    variance = variance + dev * dev;
                  }
                }
                ampmean[i] = ampmean[i] + namp * mean;
                ampvar[i] = ampvar[i] + variance;
                nampvar[i] = nampvar[i] + namp;
              }
            }

            /* do the sidescan */
            for (int i = 0; i < pixels_ss; i++) {

              /* get mean sidescan */
              nss = 0;
              mean = 0.0;
              for (int j = 0; j < nread; j++) {
                datacur = &data[j];
                ss = datacur->ss;
                if (ss[i] > MB_SIDESCAN_NULL) {
                  nss++;
                  mean = mean + ss[i];
                }
              }
              if (nss == pings_read) {
                double variance = 0.0;
                mean = mean / nss;
                for (int j = 0; j < nread; j++) {
                  datacur = &data[j];
                  ss = datacur->ss;
                  if (ss[i] > MB_SIDESCAN_NULL) {
                    dev = ss[i] - mean;
                    variance = variance + dev * dev;
                  }
                }
                ssmean[i] = ssmean[i] + nss * mean;
                ssvar[i] = ssvar[i] + variance;
                nssvar[i] = nssvar[i] + nss;
              }
            }
          }

          if (verbose >= 2) {
            fprintf(stream, "\ndbg2  Processing loop finished in program <%s>\n", program_name);
            fprintf(stream, "dbg2       status:     %d\n", status);
            fprintf(stream, "dbg2       error:      %d\n", error);
            fprintf(stream, "dbg2       nread:      %d\n", nread);
            fprintf(stream, "dbg2       pings_read: %d\n", pings_read);
          }
        }

        /* look for problems */
        timtotfile = (timendpath - timbegfile) / 3600.0;
        if (timtotfile > 0.0) {
          timtot += timtotfile;
          spdavgfile = distotfile / timtotfile;
        }
        if (irecfile <= 0)
          mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_NO_DATA);
        else if (timtotfile > 0.0 && spdavgfile >= speed_threshold)
          mb_notice_log_problem(verbose, mbio_ptr, MB_PROBLEM_AVG_TOO_FAST);

        /* get notices if desired */
        if (print_notices && pass == 0) {
          status = mb_notice_get_list(verbose, mbio_ptr, notice_list);
          for (int i = 0; i < MB_NOTICE_MAX; i++)
            notice_list_tot[i] += notice_list[i];
        }

        /* deal with statistics */
        if (pings_read > 2) {
          /* allocate total statistics arrays if needed */
          if (nbathtot_alloc < beams_bath_max) {
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(double), (void **)&bathmeantot, &error);
            status &=
                mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(double), (void **)&bathvartot, &error);
            status &=
                mb_reallocd(verbose, __FILE__, __LINE__, beams_bath_max * sizeof(int), (void **)&nbathvartot, &error);
            if (error != MB_ERROR_NO_ERROR) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nMBIO Error allocating data arrays:\n%s\n", message);
              fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
              exit(error);
            }
            else {
              for (int i = nbathtot_alloc; i < beams_bath_max; i++) {
                bathmeantot[i] = 0.0;
                bathvartot[i] = 0.0;
                nbathvartot[i] = 0;
              }
              nbathtot_alloc = beams_bath_max;
            }
          }
          if (namptot_alloc < beams_amp_max) {
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(double), (void **)&ampmeantot, &error);
            status &=
                mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(double), (void **)&ampvartot, &error);
            status &= mb_reallocd(verbose, __FILE__, __LINE__, beams_amp_max * sizeof(int), (void **)&nampvartot, &error);
            if (error != MB_ERROR_NO_ERROR) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nMBIO Error allocating data arrays:\n%s\n", message);
              fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
              exit(error);
            }
            else {
              for (int i = namptot_alloc; i < beams_amp_max; i++) {
                ampmeantot[i] = 0.0;
                ampvartot[i] = 0.0;
                nampvartot[i] = 0;
              }
              namptot_alloc = beams_amp_max;
            }
          }
          if (nsstot_alloc < pixels_ss_max) {
            status =
                mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(double), (void **)&ssmeantot, &error);
            status &= mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(double), (void **)&ssvartot, &error);
            status &= mb_reallocd(verbose, __FILE__, __LINE__, pixels_ss_max * sizeof(int), (void **)&nssvartot, &error);
            if (error != MB_ERROR_NO_ERROR) {
              char *message;
              mb_error(verbose, error, &message);
              fprintf(stream, "\nMBIO Error allocating data arrays:\n%s\n", message);
              fprintf(stream, "\nProgram <%s> Terminated\n", program_name);
              exit(error);
            }
            else {
              for (int i = nsstot_alloc; i < pixels_ss_max; i++) {
                ssmeantot[i] = 0.0;
                ssvartot[i] = 0.0;
                nssvartot[i] = 0;
              }
              nsstot_alloc = pixels_ss_max;
            }
          }

          /* copy statistics to total statistics */
          for (int i = 0; i < beams_bath; i++) {
            bathmeantot[i] += bathmean[i];
            bathvartot[i] += bathvar[i];
            nbathvartot[i] += nbathvar[i];
          }
          for (int i = 0; i < beams_amp; i++) {
            ampmeantot[i] += ampmean[i];
            ampvartot[i] += ampvar[i];
            nampvartot[i] += nampvar[i];
          }
          for (int i = 0; i < pixels_ss; i++) {
            ssmeantot[i] += ssmean[i];
            ssvartot[i] += ssvar[i];
            nssvartot[i] += nssvar[i];
          }
        }

        /* close the swath file */
        status &= mb_close(verbose, &mbio_ptr, &error);

        /* figure out whether and what to read next */
        if (read_datalist) {
          read_data = (mb_datalist_read(verbose, datalist, path, dpath, &format, &file_weight, &error) == MB_SUCCESS);
        } else {
          read_data = false;
        }

        /* end loop over files in list */
      }
      if (read_datalist)
        mb_datalist_close(verbose, &datalist, &error);

      if (pass > 0 || !coverage_mask || (coverage_mask && coverage_mask_bounds))
        done = true;

      pass++;
    }  /* end loop over reading passes */
  } /* end !quick */

  /* if quick then use mb_get_info_datalist() */
  else {
    struct mb_info_struct mb_info;
    status = mb_get_info_datalist(verbose, read_file, &format, &mb_info, lonflip_use, &error);

    if (status == MB_SUCCESS) {
      irec = mb_info.nrecords;

      notice_list_tot[MB_DATA_SIDESCAN2] = mb_info.nrecords_ss1;
      notice_list_tot[MB_DATA_SIDESCAN3] = mb_info.nrecords_ss2;
      notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM] = mb_info.nrecords_sbp;
      notice_list_tot[MB_DATA_SUBBOTTOM_MCS] = 0;
      notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] = 0;
      notice_list_tot[MB_DATA_WATER_COLUMN] = 0;
      beams_bath_max = mb_info.nbeams_bath;
      ntdbeams = mb_info.nbeams_bath_total;
      ngdbeams = mb_info.nbeams_bath_good;
      nzdbeams = mb_info.nbeams_bath_zero;
      nfdbeams = mb_info.nbeams_bath_flagged;
      beams_amp_max = mb_info.nbeams_amp;
      ntabeams = mb_info.nbeams_amp_total;
      ngabeams = mb_info.nbeams_amp_good;
      nzabeams = mb_info.nbeams_amp_zero;
      nfabeams = mb_info.nbeams_amp_flagged;
      pixels_ss_max = mb_info.npixels_ss;
      ntsbeams = mb_info.npixels_ss_total;
      ngsbeams = mb_info.npixels_ss_good;
      nzsbeams = mb_info.npixels_ss_zero;
      nfsbeams = mb_info.npixels_ss_flagged;

      timtot = mb_info.time_total;
      distot = mb_info.dist_total;
      spdavg = mb_info.speed_avg;

      timbeg = mb_info.time_start;
      mb_get_date(verbose, timbeg, timbeg_i);
      lonbeg = mb_info.lon_start;
      latbeg = mb_info.lat_start;
      bathbeg = mb_info.depth_start;
      hdgbeg = mb_info.heading_start;
      spdbeg = mb_info.speed_start;
      sdpbeg = mb_info.sensordepth_start;
      altbeg = mb_info.sonaraltitude_start;

      timend = mb_info.time_end;
      mb_get_date(verbose, timend, timend_i);
      lonend = mb_info.lon_end;
      latend = mb_info.lat_end;
      bathend = mb_info.depth_end;
      hdgend = mb_info.heading_end;
      spdend = mb_info.speed_end;
      sdpend = mb_info.sensordepth_end;
      altend = mb_info.sonaraltitude_end;

      lonmin = mb_info.lon_min;
      lonmax = mb_info.lon_max;
      latmin = mb_info.lat_min;
      latmax = mb_info.lat_max;
      sdpmin = mb_info.sensordepth_min;
      sdpmax = mb_info.sensordepth_max;
      altmin = mb_info.altitude_min;
      altmax = mb_info.altitude_max;
      bathmin = mb_info.depth_min;
      bathmax = mb_info.depth_max;
      ampmin = mb_info.amp_min;
      ampmax = mb_info.amp_max;
      ssmin = mb_info.ss_min;
      ssmax = mb_info.ss_max;

      notice_list_tot[MB_PROBLEM_NO_DATA + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_nodata;
      notice_list_tot[MB_PROBLEM_ZERO_NAV + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_zeronav;
      notice_list_tot[MB_PROBLEM_TOO_FAST + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_toofast;
      notice_list_tot[MB_PROBLEM_AVG_TOO_FAST + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_avgtoofast;
      notice_list_tot[MB_PROBLEM_TOO_DEEP + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_toodeep;
      notice_list_tot[MB_PROBLEM_BAD_DATAGRAM + MB_DATA_KINDS - (MB_ERROR_MIN)] = mb_info.problem_baddatagram;
    }
  }

  /* calculate final variances */
  if (pings_read > 2) {
    for (int i = 0; i < nbathtot_alloc; i++)
      if (nbathvartot[i] > 0) {
        bathmeantot[i] = bathmeantot[i] / nbathvartot[i];
        bathvartot[i] = bathvartot[i] / nbathvartot[i];
      }
    for (int i = 0; i < namptot_alloc; i++)
      if (nampvartot[i] > 0) {
        ampmeantot[i] = ampmeantot[i] / nampvartot[i];
        ampvartot[i] = ampvartot[i] / nampvartot[i];
      }
    for (int i = 0; i < nsstot_alloc; i++)
      if (nssvartot[i] > 0) {
        ssmeantot[i] = ssmeantot[i] / nssvartot[i];
        ssvartot[i] = ssvartot[i] / nssvartot[i];
      }
  }

  /* calculate percentages of data */
  double ngd_percent = 0.0;
  double nzd_percent = 0.0;
  double nfd_percent = 0.0;
  if (ntdbeams > 0) {
    ngd_percent = 100.0 * ngdbeams / ntdbeams;
    nzd_percent = 100.0 * nzdbeams / ntdbeams;
    nfd_percent = 100.0 * nfdbeams / ntdbeams;
  }

  double nga_percent = 0.0;
  double nza_percent = 0.0;
  double nfa_percent = 0.0;
  if (ntabeams > 0) {
    nga_percent = 100.0 * ngabeams / ntabeams;
    nza_percent = 100.0 * nzabeams / ntabeams;
    nfa_percent = 100.0 * nfabeams / ntabeams;
  }

  double ngs_percent = 0.0;
  double nzs_percent = 0.0;
  double nfs_percent = 0.0;
  if (ntsbeams > 0) {
    ngs_percent = 100.0 * ngsbeams / ntsbeams;
    nzs_percent = 100.0 * nzsbeams / ntsbeams;
    nfs_percent = 100.0 * nfsbeams / ntsbeams;
  }

  /* now print out the results */
  if (!quick)
    timtot = (timend - timbeg) / 3600.0;
  if (timtot > 0.0)
    spdavg = distot / timtot;
  mb_get_jtime(verbose, timbeg_i, timbeg_j);
  mb_get_jtime(verbose, timend_i, timend_j);

  switch (output_format) {
  case FREE_TEXT:
    fprintf(output, "\nData Totals:\n");
    fprintf(output, "Number of Records:                    %8d\n", irec);
    isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
               notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
    if (isbtmrec > 0)
      fprintf(output, "Number of Subbottom Records:          %8d\n", isbtmrec);
    if (notice_list_tot[MB_DATA_SIDESCAN2] > 0)
      fprintf(output, "Number of Secondary Sidescan Records: %8d\n", notice_list_tot[MB_DATA_SIDESCAN2]);
    if (notice_list_tot[MB_DATA_SIDESCAN3] > 0)
      fprintf(output, "Number of Tertiary Sidescan Records:  %8d\n", notice_list_tot[MB_DATA_SIDESCAN3]);
    if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0)
      fprintf(output, "Number of Water Column Records:       %8d\n", notice_list_tot[MB_DATA_WATER_COLUMN]);

    fprintf(output, "Bathymetry Data (%d beams):\n", beams_bath_max);
    fprintf(output, "  Number of Beams:         %8d\n", ntdbeams);
    fprintf(output, "  Number of Good Beams:    %8d     %5.2f%%\n", ngdbeams, ngd_percent);
    fprintf(output, "  Number of Zero Beams:    %8d     %5.2f%%\n", nzdbeams, nzd_percent);
    fprintf(output, "  Number of Flagged Beams: %8d     %5.2f%%\n", nfdbeams, nfd_percent);
    fprintf(output, "Amplitude Data (%d beams):\n", beams_amp_max);
    fprintf(output, "  Number of Beams:         %8d\n", ntabeams);
    fprintf(output, "  Number of Good Beams:    %8d     %5.2f%%\n", ngabeams, nga_percent);
    fprintf(output, "  Number of Zero Beams:    %8d     %5.2f%%\n", nzabeams, nza_percent);
    fprintf(output, "  Number of Flagged Beams: %8d     %5.2f%%\n", nfabeams, nfa_percent);
    fprintf(output, "Sidescan Data (%d pixels):\n", pixels_ss_max);
    fprintf(output, "  Number of Pixels:        %8d\n", ntsbeams);
    fprintf(output, "  Number of Good Pixels:   %8d     %5.2f%%\n", ngsbeams, ngs_percent);
    fprintf(output, "  Number of Zero Pixels:   %8d     %5.2f%%\n", nzsbeams, nzs_percent);
    fprintf(output, "  Number of Flagged Pixels:%8d     %5.2f%%\n", nfsbeams, nfs_percent);
    fprintf(output, "\nNavigation Totals:\n");
    fprintf(output, "Total Time:         %10.4f hours\n", timtot);
    fprintf(output, "Total Track Length: %10.4f km\n", distot);
    fprintf(output, "Average Speed:      %10.4f km/hr (%7.4f knots)\n", spdavg, spdavg / 1.85);
    fprintf(output, "\nStart of Data:\n");
    fprintf(output, "Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d (%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d)\n",
            timbeg_i[1], timbeg_i[2], timbeg_i[0], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1],
            timbeg_i[0], timbeg_i[1], timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
    if (bathy_in_meters)
      fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f meters\n", lonbeg, latbeg, bathbeg);
    else
      fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f feet\n", lonbeg, latbeg, bathy_scale * bathbeg);
    fprintf(output, "Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n", spdbeg, spdbeg / 1.85, hdgbeg);
    fprintf(output, "Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n", sdpbeg, altbeg);
    fprintf(output, "\nEnd of Data:\n");
    fprintf(output, "Time:  %2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d (%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d)\n",
            timend_i[1], timend_i[2], timend_i[0], timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1],
            timend_i[0], timend_i[1], timend_i[2], timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
    if (bathy_in_meters)
      fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f meters\n", lonend, latend, bathend);
    else
      fprintf(output, "Lon: %15.9f     Lat: %15.9f     Depth: %10.4f feet\n", lonend, latend, bathy_scale * bathend);
    fprintf(output, "Speed: %7.4f km/hr (%7.4f knots)  Heading:%9.4f degrees\n", spdend, spdend / 1.85, hdgend);
    fprintf(output, "Sonar Depth:%10.4f m  Sonar Altitude:%10.4f m\n", sdpend, altend);
    fprintf(output, "\nLimits:\n");
    fprintf(output, "Minimum Longitude:   %15.9f   Maximum Longitude:   %15.9f\n", lonmin, lonmax);
    fprintf(output, "Minimum Latitude:    %15.9f   Maximum Latitude:    %15.9f\n", latmin, latmax);
    fprintf(output, "Minimum Sonar Depth: %10.4f   Maximum Sonar Depth: %10.4f\n", sdpmin, sdpmax);
    fprintf(output, "Minimum Altitude:    %10.4f   Maximum Altitude:    %10.4f\n", altmin, altmax);
    if (ngdbeams > 0 || verbose >= 1)
      fprintf(output, "Minimum Depth:       %10.4f   Maximum Depth:       %10.4f\n", bathy_scale * bathmin,
              bathy_scale * bathmax);
    if (ngabeams > 0 || verbose >= 1)
      fprintf(output, "Minimum Amplitude:   %10.4f   Maximum Amplitude:   %10.4f\n", ampmin, ampmax);
    if (ngsbeams > 0 || verbose >= 1)
      fprintf(output, "Minimum Sidescan:    %10.4f   Maximum Sidescan:    %10.4f\n", ssmin, ssmax);
    break;
  case JSON:
    fprintf(output, "\"data_totals\": {\n");
    fprintf(output, "\"number_of_records\": \"%d\"", irec);
    isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
               notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
    if (isbtmrec > 0)
      fprintf(output, ",\n\"number_of_subbottom_records\":\"%d\"\n", isbtmrec);
    if (notice_list_tot[MB_DATA_SIDESCAN2] > 0)
      fprintf(output, ",\n\"number_of_secondary_sidescan_records\": \"%d\"", notice_list_tot[MB_DATA_SIDESCAN2]);
    if (notice_list_tot[MB_DATA_SIDESCAN3] > 0)
      fprintf(output, ",\n\"number_of_tertiary_sidescan_records\": \"%d\"", notice_list_tot[MB_DATA_SIDESCAN3]);
    if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0)
      fprintf(output, ",\n\"number_of_water_column_records\": \"%d\"", notice_list_tot[MB_DATA_WATER_COLUMN]);
    fprintf(output, "\n},\n");

    fprintf(output, "\"bathymetry_data\": {\n\"max_beams_per_ping\": \"%d\",\n", beams_bath_max);
    fprintf(output, "\"number_beams\": \"%d\",\n", ntdbeams);
    fprintf(output, "\"number_good_beams\": \"%d\",\n\"percent_good_beams\": \"%5.2f\",\n", ngdbeams, ngd_percent);
    fprintf(output, "\"number_zero_beams\": \"%d\",\n\"percent_zero_beams\": \"%5.2f\",\n", nzdbeams, nzd_percent);
    fprintf(output, "\"number_flagged_beams\": \"%d\",\n\"percent_flagged_beams\": \"%5.2f\"\n", nfdbeams, nfd_percent);
    fprintf(output, "},\n");

    fprintf(output, "\"amplitude_data\": {\n\"max_beams_per_ping\": \"%d\",\n", beams_amp_max);
    fprintf(output, "\"number_beams\": \"%d\",\n", ntabeams);
    fprintf(output, "\"number_good_beams\": \"%d\",\n\"percent_good_beams\": \" %5.2f\",\n", ngabeams, nga_percent);
    fprintf(output, "\"number_zero_beams\": \"%d\",\n\"percent_zero_beams\": \"%5.2f\",\n", nzabeams, nza_percent);
    fprintf(output, "\"number_flagged_beams\": \"%d\",\n\"percent_flagged_beams\": \"%5.2f\"\n", nfabeams, nfa_percent);
    fprintf(output, "},\n");

    fprintf(output, "\"sidescan_data\": {\n\"max_pixels_per_ping\": \"%d\",\n", pixels_ss_max);
    fprintf(output, "\"number_of_pixels\": \"%d\",\n", ntsbeams);
    fprintf(output, "\"number_good_pixels\": \"%d\",\n\"percent_good_pixels\": \"%5.2f\",\n", ngsbeams, ngs_percent);
    fprintf(output, "\"number_zero_pixels\": \"%d\",\n\"percent_zero_pixels\": \"%5.2f\",\n", nzsbeams, nzs_percent);
    fprintf(output, "\"number_flagged_pixels\": \"%d\",\n\"percent_flagged_pixels\": \"%5.2f\"\n", nfsbeams, nfs_percent);
    fprintf(output, "},\n");

    fprintf(output, "\"navigation_totals\": {\n");
    fprintf(output, "\"total_time_hours\": \"%.4f\",\n", timtot);
    fprintf(output, "\"total_track_length_km\": \"%.4f\",\n", distot);
    fprintf(output, "\"average_speed_km_per_hr\": \"%.4f\",\n\"average_speed_knots\": \"%.4f\"\n", spdavg, spdavg / 1.85);
    fprintf(output, "},\n");

    fprintf(output, "\"start_of_data\": {\n");
    fprintf(output, "\"time\": \"%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\",\n", timbeg_i[1], timbeg_i[2], timbeg_i[0],
            timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1]);
    fprintf(output, "\"time_iso\": \"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d\",\n", timbeg_i[0], timbeg_i[1], timbeg_i[2],
            timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
    if (bathy_in_meters)
      fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_meters\": \"%.4f\",\n", lonbeg, latbeg,
              bathbeg);
    else
      fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_feet\": \"%.4f\",\n", lonbeg, latbeg,
              bathy_scale * bathbeg);
    fprintf(output, "\"speed_km_per_hour\": \"%.4f\",\n\"speed_knots\": \"%.4f\",\n\"heading_degrees\": \"%.4f\",\n", spdbeg,
            spdbeg / 1.85, hdgbeg);
    fprintf(output, "\"sonar_depth_meters\": \"%.4f\",\n\"sonar_altitude_meters\": \"%.4f\"\n", sdpbeg, altbeg);
    fprintf(output, "},\n");

    fprintf(output, "\"end_of_data\": {\n");
    fprintf(output, "\"time\": \"%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d\",\n", timend_i[1], timend_i[2], timend_i[0],
            timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1]);
    fprintf(output, "\"time_iso\": \"%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d\",\n", timend_i[0], timend_i[1], timend_i[2],
            timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
    if (bathy_in_meters)
      fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_meters\": \"%.4f\",\n", lonend, latend,
              bathend);
    else
      fprintf(output, "\"longitude\": \"%.9f\",\n\"latitude\": \"%.9f\",\n\"depth_feet\": \"%.4f\",\n", lonend, latend,
              bathy_scale * bathend);
    fprintf(output, "\"speed_km_per_hour\": \"%.4f\",\n\"speed_knots\": \"%.4f\",\n\"heading_degrees\": \"%.4f\",\n", spdend,
            spdend / 1.85, hdgend);
    fprintf(output, "\"sonar_depth_meters\": \"%.4f\",\n\"sonar_altitude_meters\": \"%.4f\"\n", sdpend, altend);
    fprintf(output, "},\n");

    fprintf(output, "\"limits\": {\n");
    fprintf(output, "\"minimum_longitude\": \"%.9f\",\n\"maximum_longitude\": \"%.9f\",\n", lonmin, lonmax);
    fprintf(output, "\"minimum_latitude\": \"%.9f\",\n\"maximum_latitude\": \"%.9f\",\n", latmin, latmax);
    fprintf(output, "\"minimum_sonar_depth\": \"%.4f\",\n\"maximum_sonar_depth\": \"%.4f\",\n", sdpmin, sdpmax);
    fprintf(output, "\"minimum_altitude\": \"%.4f\",\n\"maximum_altitude\": \"%.4f\"", altmin, altmax);
    if (ngdbeams > 0 || verbose >= 1)
      fprintf(output, ",\n\"minimum_depth\": \"%.4f\",\n\"maximum_depth\": \"%.4f\"", bathy_scale * bathmin,
              bathy_scale * bathmax);
    if (ngabeams > 0 || verbose >= 1)
      fprintf(output, ",\n\"minimum_amplitude\": \"%.4f\",\n\"maximum_amplitude\": \"%.4f\"", ampmin, ampmax);
    if (ngsbeams > 0 || verbose >= 1)
      fprintf(output, ",\n\"minimum_sidescan\": \"%.4f\",\n\"maximum_sidescan\": \"%.4f\"", ssmin, ssmax);
    fprintf(output, "\n}");
    break;
  case XML:
    fprintf(output, "\t<data_totals>\n");
    fprintf(output, "\t\t<number_of_records>%d</number_of_records>\n", irec);
    isbtmrec = notice_list_tot[MB_DATA_SUBBOTTOM_MCS] + notice_list_tot[MB_DATA_SUBBOTTOM_CNTRBEAM] +
               notice_list_tot[MB_DATA_SUBBOTTOM_SUBBOTTOM];
    if (isbtmrec > 0)
      fprintf(output, "\t\t<number_of_subbottom_records>%d</number_of_subbottom_records>\n", isbtmrec);
    if (notice_list_tot[MB_DATA_SIDESCAN2] > 0)
      fprintf(output, "\t\t<number_of_secondary_sidescan_records>%d</number_of_secondary_sidescan_records>\n",
              notice_list_tot[MB_DATA_SIDESCAN2]);
    if (notice_list_tot[MB_DATA_SIDESCAN3] > 0)
      fprintf(output, "\t\t<number_of_tertiary_sidescan_records>%d</number_of_tertiary_sidescan_records>\n",
              notice_list_tot[MB_DATA_SIDESCAN3]);
    if (notice_list_tot[MB_DATA_WATER_COLUMN] > 0)
      fprintf(output, "\t\t<number_of_water_column_records>%d</number_of_water_column_records>\n",
              notice_list_tot[MB_DATA_WATER_COLUMN]);
    fprintf(output, "\t</data_totals>\n");

    fprintf(output, "\t<bathymetry_data>\n");
    fprintf(output, "\t\t<max_beams_per_ping>%d</max_beams_per_ping>\n", beams_bath_max);
    fprintf(output, "\t\t<number_beams>%d</number_beams>\n", ntdbeams);
    fprintf(output, "\t\t<number_good_beams>%d</number_good_beams>\n", ngdbeams);
    fprintf(output, "\t\t<percent_good_beams>%.2f</percent_good_beams>\n", ngd_percent);
    fprintf(output, "\t\t<number_zero_beams>%d</number_zero_beams>\n", nzdbeams);
    fprintf(output, "\t\t<percent_zero_beams>%.2f</percent_zero_beams>\n", nzd_percent);
    fprintf(output, "\t\t<number_flagged_beams>%d</number_flagged_beams>\n", nfdbeams);
    fprintf(output, "\t\t<percent_flagged_beams>%.2f</percent_flagged_beams>\n", nfd_percent);
    fprintf(output, "\t</bathymetry_data>\n");
    fprintf(output, "\t<amplitude_data>\n");
    fprintf(output, "\t\t<max_beams_per_ping>%d</max_beams_per_ping>\n", beams_bath_max);
    fprintf(output, "\t\t<number_beams>%d</number_beams>\n", ntabeams);
    fprintf(output, "\t\t<number_good_beams>%d</number_good_beams>\n", ngabeams);
    fprintf(output, "\t\t<percent_good_beams>%.2f</percent_good_beams>\n", nga_percent);
    fprintf(output, "\t\t<number_zero_beams>%d</number_zero_beams>\n", nzabeams);
    fprintf(output, "\t\t<percent_zero_beams>%.2f</percent_zero_beams>\n", nza_percent);
    fprintf(output, "\t\t<number_flagged_beams>%d</number_flagged_beams>\n", nfabeams);
    fprintf(output, "\t\t<percent_flagged_beams>%.2f</percent_flagged_beams>\n", nfa_percent);
    fprintf(output, "\t</amplitude_data>\n");
    fprintf(output, "\t<sidescan_data>\n");
    fprintf(output, "\t\t<max_pixels_per_ping>%d</max_pixels_per_ping>\n", pixels_ss_max);
    fprintf(output, "\t\t<number_pixels>%d</number_pixels>\n", ntsbeams);
    fprintf(output, "\t\t<number_good_pixels>%d</number_good_pixels>\n", ngsbeams);
    fprintf(output, "\t\t<percent_good_pixels>%.2f</percent_good_pixels>\n", ngs_percent);
    fprintf(output, "\t\t<number_zero_pixels>%d</number_zero_pixels>\n", nzsbeams);
    fprintf(output, "\t\t<percent_zero_pixels>%.2f</percent_zero_pixels>\n", nzs_percent);
    fprintf(output, "\t\t<number_flagged_pixels>%d</number_flagged_pixels>\n", nfsbeams);
    fprintf(output, "\t\t<percent_flagged_pixels>%.2f</percent_flagged_pixels>\n", nfs_percent);
    fprintf(output, "\t</sidescan_data>\n");

    fprintf(output, "\t<tnavigation_totals>\n");
    fprintf(output, "\t\t<total_time_hours>%.4f</total_time_hours>\n", timtot);
    fprintf(output, "\t\t<total_track_length_km>%.4f</total_track_length_km>\n", distot);
    fprintf(output, "\t\t<average_speed_km_per_hr>%.4f</average_speed_km_per_hr>\n", spdavg);
    fprintf(output, "\t\t<average_speed_knots>%.4f</average_speed_knots>\n", spdavg / 1.85);
    fprintf(output, "\t</tnavigation_totals>\n");

    fprintf(output, "\t<start_of_data>\n");
    fprintf(output, "\t\t<time>%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d</time>\n", timbeg_i[1], timbeg_i[2],
            timbeg_i[0], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6], timbeg_j[1]);
    fprintf(output, "\t\t<time_iso>%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d</time_iso>\n", timbeg_i[0], timbeg_i[1],
            timbeg_i[2], timbeg_i[3], timbeg_i[4], timbeg_i[5], timbeg_i[6]);
    if (bathy_in_meters) {
      fprintf(output, "\t\t<longitude>%.9f</longitude>\n", lonbeg);
      fprintf(output, "\t\t<latitude>%.9f</latitude>\n", latbeg);
      fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathbeg);
    }
    else {
      fprintf(output, "\t\t<longitude>%.9f</longitude>\n", lonbeg);
      fprintf(output, "\t\t<latitude>%.9f</latitude>\n", latbeg);
      fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathy_scale * bathbeg);
    }
    fprintf(output, "\t\t<speed_km_per_hour>%.4f</speed_km_per_hour>\n", spdbeg);
    fprintf(output, "\t\t<speed_knots>%.4f</speed_knots>\n", spdbeg / 1.85);
    fprintf(output, "\t\t<heading_degrees>%.4f</heading_degrees>\n", hdgbeg);
    fprintf(output, "\t\t<sonar_depth_meters>%.4f</sonar_depth_meters>\n", sdpbeg);
    fprintf(output, "\t\t<sonar_altitude_meters>%.4f</sonar_altitude_meters>\n", altbeg);
    fprintf(output, "\t</start_of_data>\n");

    fprintf(output, "\t<end_of_data>\n");
    fprintf(output, "\t\t<time>%2.2d %2.2d %4.4d %2.2d:%2.2d:%2.2d.%6.6d  JD%d</time>\n", timend_i[1], timend_i[2],
            timend_i[0], timend_i[3], timend_i[4], timend_i[5], timend_i[6], timend_j[1]);
    fprintf(output, "\t\t<time_iso>%4.4d-%2.2d-%2.2dT%2.2d:%2.2d:%2.2d.%6.6d</time_iso>\n", timend_i[0], timend_i[1],
            timend_i[2], timend_i[3], timend_i[4], timend_i[5], timend_i[6]);
    if (bathy_in_meters) {
      fprintf(output, "\t\t<longitude>%.9f</longitude>\n", lonend);
      fprintf(output, "\t\t<latitude>%.9f</latitude>\n", latend);
      fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathend);
    }
    else {
      fprintf(output, "\t\t<longitude>%.9f</longitude>\n", lonend);
      fprintf(output, "\t\t<latitude>%.9f</latitude>\n", latend);
      fprintf(output, "\t\t<depth_meters>%.4f</depth_meters>\n", bathy_scale * bathend);
    }
    fprintf(output, "\t\t<speed_km_per_hour>%.4f</speed_km_per_hour>\n", spdend);
    fprintf(output, "\t\t<speed_knots>%.4f</speed_knots>\n", spdend / 1.85);
    fprintf(output, "\t\t<heading_degrees>%.4f</heading_degrees>\n", hdgend);
    fprintf(output, "\t\t<sonar_depth_meters>%.4f</sonar_depth_meters>\n", sdpend);
    fprintf(output, "\t\t<sonar_altitude_meters>%.4f</sonar_altitude_meters>\n", altend);
    fprintf(output, "\t</end_of_data>\n");

    fprintf(output, "\t<limits>\n");
    fprintf(output, "\t\t<minimum_longitude>%.9f</minimum_longitude>\n", lonmin);
    fprintf(output, "\t\t<maximum_longitude>%.9f</maximum_longitude>\n", lonmax);
    fprintf(output, "\t\t<minimum_latitude>%.9f</minimum_latitude>\n", latmin);
    fprintf(output, "\t\t<maximum_latitude>%.9f</maximum_latitude>\n", latmax);
    fprintf(output, "\t\t<minimum_sonar_depth>%.4f</minimum_sonar_depth>\n", sdpmin);
    fprintf(output, "\t\t<maximum_sonar_depth>%.4f</maximum_sonar_depth>\n", sdpmax);
    fprintf(output, "\t\t<minimum_altitude>%.4f</minimum_altitude>\n", altmin);
    fprintf(output, "\t\t<maximum_altitude>%.4f</maximum_altitude>\n", altmax);
    if (ngdbeams > 0 || verbose >= 1) {
      fprintf(output, "\t\t<minimum_depth>%.4f</minimum_depth>\n", bathy_scale * bathmin);
      fprintf(output, "\t\t<maximum_depth>%.4f</maximum_depth>\n", bathy_scale * bathmax);
    }
    if (ngabeams > 0 || verbose >= 1) {
      fprintf(output, "\t\t<minimum_amplitude>%.4f</minimum_amplitude>\n", ampmin);
      fprintf(output, "\t\t<maximum_amplitude>%.4f</maximum_amplitude>\n", ampmax);
    }
    if (ngsbeams > 0 || verbose >= 1) {
      fprintf(output, "\t\t<minimum_sidescan>%.4f</minimum_sidescan>\n", ssmin);
      fprintf(output, "\t\t<maximum_sidescan>%.4f</maximum_sidescan>\n", ssmax);
    }
    fprintf(output, "\t</limits>\n");
    break;
  default:
    break;
  }
  if (pings_read > 2 && beams_bath_max > 0 && (ngdbeams > 0 || verbose >= 1)) {
    switch (output_format) {
    case FREE_TEXT:
      fprintf(output, "\nBeam Bathymetry Variances:\n");
      fprintf(output, "Pings Averaged: %d\n", pings_read);
      fprintf(output, " Beam     N      Mean     Variance    Sigma\n");
      fprintf(output, " ----     -      ----     --------    -----\n");
      for (int i = 0; i < beams_bath_max; i++)
        fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nbathvartot[i], bathy_scale * bathmeantot[i],
                bathy_scale * bathy_scale * bathvartot[i], bathy_scale * std::sqrt(bathvartot[i]));
      fprintf(output, "\n");
      break;
    case JSON:
      fprintf(output, ",\n\"beam_bathymetry_variances\":{\n");
      fprintf(output, "\"pings_averaged\": \"%d\",\n", pings_read);
      fprintf(output, "\"columns\" : \"#beam,N,mean,variance,sigma\",\n");
      fprintf(output, "\"values\": [\n");
      for (int i = 0; i < beams_bath_max; i++) {
        if (i > 0)
          fprintf(output, ",\n");
        double sigma = bathy_scale * std::sqrt(bathvartot[i]);
        if (std::isnan(sigma))
          sigma = 0.0;
        fprintf(output, "{\"row\":\"%d,%d,%.2f,%.2f,%.2f\"}", i, nbathvartot[i], bathy_scale * bathmeantot[i],
                bathy_scale * bathy_scale * bathvartot[i], sigma);
      }
      fprintf(output, "]}");
      break;
    case XML:
      fprintf(output, "\t<beam_bathymetry_variances>\n");
      fprintf(output, "\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
      fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n");
      fprintf(output, "\t\t<values>\n");
      for (int i = 0; i < beams_bath_max; i++) {
        double sigma = bathy_scale * std::sqrt(bathvartot[i]);
        if (std::isnan(sigma))
          sigma = 0.0;
        fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nbathvartot[i], bathy_scale * bathmeantot[i],
                bathy_scale * bathy_scale * bathvartot[i], sigma);
      }
      fprintf(output, "\t\t</values>\n");
      fprintf(output, "\t</beam_bathymetry_variances>\n");
      break;
    default:
      break;
    }
  }
  if (pings_read > 2 && beams_amp_max > 0 && (ngabeams > 0 || verbose >= 1)) {
    switch (output_format) {
    case FREE_TEXT:
      fprintf(output, "\nBeam Amplitude Variances:\n");
      fprintf(output, "Pings Averaged: %d\n", pings_read);
      fprintf(output, " Beam     N      Mean     Variance    Sigma\n");
      fprintf(output, " ----     -      ----     --------    -----\n");
      for (int i = 0; i < beams_amp_max; i++)
        fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nampvartot[i], ampmeantot[i], ampvartot[i],
                std::sqrt(ampvartot[i]));
      fprintf(output, "\n");
      break;
    case JSON:
      fprintf(output, ",\n\"beam_amplitude_variances\":{\n");
      fprintf(output, "\"pings_averaged\": \"%d\",\n", pings_read);
      fprintf(output, "\"columns\":\"beam,N,mean,variance,sigma\",\n");
      fprintf(output, "\"values\": [\n");
      for (int i = 0; i < beams_amp_max; i++) {
        if (i > 0)
          fprintf(output, ",\n");
        double sigma = std::sqrt(ampvartot[i]);
        if (std::isnan(sigma))
          sigma = 0;
        fprintf(output, "{\"row\" : \"%d,%d,%.2f,%.2f,%.2f\"}", i, nampvartot[i], ampmeantot[i], ampvartot[i], sigma);
      }
      fprintf(output, "\n]}");
      break;
    case XML:
      fprintf(output, "\t<beam_amplitude_variances>\n");
      fprintf(output, "\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
      fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n");
      fprintf(output, "\t\t<values>\n");
      for (int i = 0; i < beams_amp_max; i++) {
        double sigma = std::sqrt(ampvartot[i]);
        if (std::isnan(sigma))
          sigma = 0.0;
        fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nampvartot[i], ampmeantot[i], ampvartot[i], sigma);
      }
      fprintf(output, "\t\t</values>\n");
      fprintf(output, "\t</beam_amplitude_variances>\n");
      break;
    default:
      break;
    }
  }
  if (pings_read > 2 && pixels_ss_max > 0 && (ngsbeams > 0 || verbose >= 1)) {
    switch (output_format) {
    case FREE_TEXT:
      fprintf(output, "\nPixel Sidescan Variances:\n");
      fprintf(output, "Pings Averaged: %d\n", pings_read);
      fprintf(output, " Beam     N      Mean     Variance    Sigma\n");
      fprintf(output, " ----     -      ----     --------    -----\n");
      for (int i = 0; i < pixels_ss_max; i++)
        fprintf(output, "%4d  %5d   %8.2f   %8.2f  %8.2f\n", i, nssvartot[i], ssmeantot[i], ssvartot[i],
                std::sqrt(ssvartot[i]));
      fprintf(output, "\n");
      break;
    case JSON:
      fprintf(output, ",\n\"pixel_sidescan_variances\":{\n");
      fprintf(output, "\"pings_averaged\": \"%d\",\n", pings_read);
      fprintf(output, "\"columns\":\"pixel,N,mean,variance,sigma\",\n");
      fprintf(output, "\"values\": [\n");
      for (int i = 0; i < pixels_ss_max; i++) {
        if (i > 0)
          fprintf(output, ",\n");
        double sigma = std::sqrt(ssvartot[i]);
        if (std::isnan(sigma))
          sigma = 0.0;
        fprintf(output, "{\"row\":\"%d,%d,%.2f,%.2f,%.2f\"}", i, nssvartot[i], ssmeantot[i], ssvartot[i], sigma);
      }
      fprintf(output, "\n]\n}");
      break;
    case XML:
      fprintf(output, "\t<pixel_sidescan_variances>\n");
      fprintf(output, "\t\t<pings_averaged>%d</pings_averaged>\n", pings_read);
      fprintf(output, "\t\t<columns>pixel,N,mean,variance,sigma</columns>\n");
      fprintf(output, "\t\t<values>\n");
      for (int i = 0; i < pixels_ss_max; i++) {
        double sigma = std::sqrt(ssvartot[i]);
        if (std::isnan(sigma))
          sigma = 0.0;
        fprintf(output, "\t\t\t<row>%d,%d,%.2f,%.2f,%.2f</row>\n", i, nssvartot[i], ssmeantot[i], ssvartot[i], sigma);
      }
      fprintf(output, "\t\t</values>\n");
      fprintf(output, "\t</pixel_sidescan_variances>\n");
      break;
    default:
      break;
    }
  }
  if (print_notices) {
    switch (output_format) {
    case FREE_TEXT:
      fprintf(output, "\nData Record Type Notices:\n");
      for (int i = 0; i <= MB_DATA_KINDS; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "DN: %d %s\n", notice_list_tot[i], notice_message);
        }
      }
      fprintf(output, "\nNonfatal Error Notices:\n");
      for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "EN: %d %s\n", notice_list_tot[i], notice_message);
        }
      }
      fprintf(output, "\nProblem Notices:\n");
      for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "PN: %d %s\n", notice_list_tot[i], notice_message);
        }
      }
      break;
    case JSON: {
      fprintf(output, ",\n\"notices\": {\n");
      int notice_total = 0;
      fprintf(output, "\"data_record_type_notices\": [\n");
      for (int i = 0; i <= MB_DATA_KINDS; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          if (notice_total > 0)
            fprintf(output, ",\n");
          fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}",
                  notice_list_tot[i], notice_message);
          notice_total++;
        }
      }
      if (notice_total > 0)
        fprintf(output, "\n");
      fprintf(output, "]");
      notice_total = 0;
      fprintf(output, ",\n\"nonfatal_error_notices\": [\n");
      for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          if (notice_total > 0)
            fprintf(output, ",\n");
          fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}",
                  notice_list_tot[i], notice_message);
          notice_total++;
        }
      }
      if (notice_total > 0)
        fprintf(output, "\n");
      fprintf(output, "]");
      notice_total = 0;
      fprintf(output, ",\n\"problem_notices\": [\n");
      for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          if (notice_total > 0)
            fprintf(output, ",\n");
          fprintf(output, "{\"notice\": {\n\"notice_number\": \"%d\",\n\"notice_message\": \"%s\"\n}}",
                  notice_list_tot[i], notice_message);
          notice_total++;
        }
      }
      if (notice_total > 0)
        fprintf(output, "\n");
      fprintf(output, "]\n");
      fprintf(output, "}");
      break;
    }
    case XML:
      fprintf(output, "\t<data_record_type_notices>\n");
      for (int i = 0; i <= MB_DATA_KINDS; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "\t\t<notice_number>%d</notice_number>\n", notice_list_tot[i]);
          fprintf(output, "\t\t<notice_messsage>%s</notice_messsage>\n", notice_message);
        }
      }
      fprintf(output, "\t</data_record_type_notices>\n");
      fprintf(output, "\t<nonfatal_error_notices>\n");
      for (int i = MB_DATA_KINDS + 1; i <= MB_DATA_KINDS - (MB_ERROR_MIN); i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "\t\t<notice_number>%d</notice_number>\n", notice_list_tot[i]);
          fprintf(output, "\t\t<notice_messsage>%s</notice_messsage>\n", notice_message);
        }
      }
      fprintf(output, "\t</nonfatal_error_notices>\n");
      fprintf(output, "\t<problem_notices>\n");
      for (int i = MB_DATA_KINDS - (MB_ERROR_MIN) + 1; i < MB_NOTICE_MAX; i++) {
        if (notice_list_tot[i] > 0) {
          char *notice_message;
          mb_notice_message(verbose, i, &notice_message);
          fprintf(output, "\t\t<notice_number>%d</notice_number>\n", notice_list_tot[i]);
          fprintf(output, "\t\t<notice_messsage>%s</notice_messsage>\n", notice_message);
        }
      }
      fprintf(output, "\t</problem_notices>\n");
      break;
    default:
      break;
    }
  }
  if (coverage_mask) {
    switch (output_format) {
    case FREE_TEXT:
      fprintf(output, "\nCoverage Mask:\nCM dimensions: %d %d\n", mask_nx, mask_ny);
      for (int j = mask_ny - 1; j >= 0; j--) {
        fprintf(output, "CM:  ");
        for (int i = 0; i < mask_nx; i++) {
          const int k = i + j * mask_nx;
          fprintf(output, " %1d", mask[k]);
        }
        fprintf(output, "\n");
      }
      break;
    case JSON:
      fprintf(output, ",\n\"coverage_mask\": {\n");
      fprintf(output, "\"dimensions_nx\": \"%d\",\n\"dimensions_ny\": \"%d\",\n", mask_nx, mask_ny);
      fprintf(output, "\"mask\": \" ");
      for (int j = mask_ny - 1; j >= 0; j--) {
        for (int i = 0; i < mask_nx; i++) {
          const int k = i + j * mask_nx;
          if (i > 0)
            fprintf(output, ",");
          fprintf(output, "%1d", mask[k]);
        }
        fprintf(output, "\n");
      }
      fprintf(output, "\"}");
      break;
    case XML:
    default:
      break;
    }
  }

  /* close root element for XML export */
  switch (output_format) {
  case FREE_TEXT:
    break;
  case JSON:
    fprintf(output, "}\n");
    break;
  case XML:
    fprintf(output, "</mbinfo>\n");
    break;
  default:
    break;
  }

  if (output_usefile && output != nullptr) {
    fclose(output);
  }

  /* deallocate memory used for data arrays */
  mb_freed(verbose, __FILE__, __LINE__, (void **)&bathmeantot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&bathvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&nbathvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&ampmeantot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&ampvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&nampvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&ssmeantot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&ssvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&nssvartot, &error);
  mb_freed(verbose, __FILE__, __LINE__, (void **)&mask, &error);

  status = MB_SUCCESS;

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
