/*--------------------------------------------------------------------
 *    The MB-system:  mbswath2las.c  11/26/20
 *
 *    Copyright (c) 2020-2023 by
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
 * MBswath2las exports swath bathymetry data from swath files to LAS format files.
 *
 * Author:  D. W. Caress
 * Date:  November 26, 2020
 *
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"

constexpr char program_name[] = "mbswath2las";
constexpr char help_message[] =
    "MBswath2las exports swath bathymetry data from swath files to LAS format files.";
constexpr char usage_message[] =
    "mbswath2las [--input=input --output=outputfile --verbose --help]";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int verbose = 0;
  int format;
  int pings;
  int pings_read;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  int status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);

  bool input_datalist = true;
  char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";
  char projection_pars[MB_PATH_MAXLINE] = "";
  bool use_projection = false;

  /* swathbounds variables */
  int beam_port = 0;
  int beam_stbd = 0;
  int pixel_port = 0;
  int pixel_stbd = 0;

  /* projected coordinate system */
  char projection_id[MB_PATH_MAXLINE] = "";
  int proj_status;
  void *pjptr = nullptr;
  double reference_lon, reference_lat;
  int utm_zone;
  double naveasting, navnorthing, deasting, dnorthing;
  double headingx, headingy, mtodeglon, mtodeglat;

  /* process argument list */
  {
    bool errflg = false;
    bool help = false;
    int c;
    while ((c = getopt(argc, argv, "B:b:E:e:F:f:I:i:J:j:L:l:R:r:S:s:T:t:VvHh")) !=
           -1)
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
        sscanf(optarg, "%1023s", projection_pars);
        use_projection = true;
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
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
      fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
      fprintf(stderr, "dbg2       help:           %d\n", help);
      fprintf(stderr, "dbg2       format:         %d\n", format);
      fprintf(stderr, "dbg2       lonflip:        %d\n", lonflip);
      fprintf(stderr, "dbg2       bounds[0]:      %f\n", bounds[0]);
      fprintf(stderr, "dbg2       bounds[1]:      %f\n", bounds[1]);
      fprintf(stderr, "dbg2       bounds[2]:      %f\n", bounds[2]);
      fprintf(stderr, "dbg2       bounds[3]:      %f\n", bounds[3]);
      fprintf(stderr, "dbg2       btime_i[0]:     %d\n", btime_i[0]);
      fprintf(stderr, "dbg2       btime_i[1]:     %d\n", btime_i[1]);
      fprintf(stderr, "dbg2       btime_i[2]:     %d\n", btime_i[2]);
      fprintf(stderr, "dbg2       btime_i[3]:     %d\n", btime_i[3]);
      fprintf(stderr, "dbg2       btime_i[4]:     %d\n", btime_i[4]);
      fprintf(stderr, "dbg2       btime_i[5]:     %d\n", btime_i[5]);
      fprintf(stderr, "dbg2       btime_i[6]:     %d\n", btime_i[6]);
      fprintf(stderr, "dbg2       etime_i[0]:     %d\n", etime_i[0]);
      fprintf(stderr, "dbg2       etime_i[1]:     %d\n", etime_i[1]);
      fprintf(stderr, "dbg2       etime_i[2]:     %d\n", etime_i[2]);
      fprintf(stderr, "dbg2       etime_i[3]:     %d\n", etime_i[3]);
      fprintf(stderr, "dbg2       etime_i[4]:     %d\n", etime_i[4]);
      fprintf(stderr, "dbg2       etime_i[5]:     %d\n", etime_i[5]);
      fprintf(stderr, "dbg2       etime_i[6]:     %d\n", etime_i[6]);
      fprintf(stderr, "dbg2       speedmin:       %f\n", speedmin);
      fprintf(stderr, "dbg2       timegap:        %f\n", timegap);
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
  char file[MB_PATH_MAXLINE] = "";
  char dfile[MB_PATH_MAXLINE] = "";
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
    // else copy single filename to be read
    strcpy(file, read_file);
    read_data = true;
  }

  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* MBIO read values */
  void *mbio_ptr = nullptr;
  void *store_ptr = nullptr;
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
  double draft;
  double roll;
  double pitch;
  double heave;
  char *beamflag = nullptr;
  double *bath = nullptr;
  double *bathacrosstrack = nullptr;
  double *bathalongtrack = nullptr;
  int *detect = nullptr;
  double *amp = nullptr;
  double *ss = nullptr;
  double *ssacrosstrack = nullptr;
  double *ssalongtrack = nullptr;
  char comment[MB_COMMENT_MAXLINE];
  int icomment = 0;

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
    bool first = true;
    while (error <= MB_ERROR_NO_ERROR) {
      /* reset error */
      error = MB_ERROR_NO_ERROR;

      /* read next data record */
      status = mb_get_all(verbose, mbio_ptr, &store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                          bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

      /* time gaps are not a problem here */
      if (error == MB_ERROR_TIME_GAP) {
        error = MB_ERROR_NO_ERROR;
        status = MB_SUCCESS;
      }

      /* if survey data extract nav */
      if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
        status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed,
                                &heading, &draft, &roll, &pitch, &heave, &error);

      /* make sure non survey data records are ignored */
      if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA)
        error = MB_ERROR_OTHER;

      /* get projected navigation if needed */
      if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_projection) {
        /* set up projection if this is the first data */
        if (pjptr == nullptr) {
          /* Default projection is UTM */
          if (strlen(projection_pars) == 0)
            strcpy(projection_pars, "U");

          /* check for UTM with undefined zone */
          if (strcmp(projection_pars, "UTM") == 0 || strcmp(projection_pars, "U") == 0 ||
              strcmp(projection_pars, "utm") == 0 || strcmp(projection_pars, "u") == 0) {
            reference_lon = navlon;
            if (reference_lon < 180.0)
              reference_lon += 360.0;
            if (reference_lon >= 180.0)
              reference_lon -= 360.0;
            utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
            reference_lat = navlat;
            if (reference_lat >= 0.0)
              snprintf(projection_id, sizeof(projection_id), "UTM%2.2dN", utm_zone);
            else
              snprintf(projection_id, sizeof(projection_id), "UTM%2.2dS", utm_zone);
          }
          else
            strcpy(projection_id, projection_pars);

          /* set projection flag */
          proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

          /* if projection not successfully initialized then quit */
          if (proj_status != MB_SUCCESS) {
            fprintf(stderr, "\nOutput projection %s not found in database\n", projection_id);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            error = MB_ERROR_BAD_PARAMETER;
            mb_memory_clear(verbose, &error);
            exit(MB_ERROR_BAD_PARAMETER);
          }
        }

        /* get projected navigation */
        mb_proj_forward(verbose, pjptr, navlon, navlat, &naveasting, &navnorthing, &error);
      }

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
        fprintf(stderr, "dbg2       kind:           %d\n", kind);
        fprintf(stderr, "dbg2       error:          %d\n", error);
        fprintf(stderr, "dbg2       status:         %d\n", status);
      }

      if (verbose >= 1 && kind == MB_DATA_COMMENT) {
        if (icomment == 0) {
          fprintf(stderr, "\nComments:\n");
          icomment++;
        }
        fprintf(stderr, "%s\n", comment);
      }

      /* get factors for lon lat calculations */
      if (error == MB_ERROR_NO_ERROR) {
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        headingx = sin(DTR * heading);
        headingy = cos(DTR * heading);
      }

      /* now loop over beams */
      if (error == MB_ERROR_NO_ERROR)
        for (int j = 0; j <= beams_bath; j++) {
        }
    }

    /* close the swath file */
    status &= mb_close(verbose, &mbio_ptr, &error);

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
