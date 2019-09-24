/*--------------------------------------------------------------------
 *    The MB-system:  mbminirovnav.c  9/7/2017
 *
 *    Copyright (c) 2017-2019 by
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
 * MBminirov reads USBL tracking and CTD day files from the MBARI MiniROV
 * and produces a single ROV navigation file in one of the standard MBARI
 * formats.
 *
 * This program replaces the several format-specific preprocessing programs
 * found in MB-System version 5 releases with a single program for version 6.
 *
 * Author:  D. W. Caress
 * Date:  7 September, 2017
 */

#include <getopt.h>
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
#include "mb_process.h"
#include "mb_status.h"

static const char program_name[] = "mbminirovnav";
static const char help_message[] =
    " MBminirov reads USBL tracking and CTD day files from the MBARI MiniROV\n"
    "\tand produces a single ROV navigation file in one of the standard MBARI\n"
    "\tformats handles preprocessing of swath sonar data as part of setting up\n"
    "\tan MB-System processing structure for a dataset.\n";
static const char usage_message[] =
    "mbminirovnav\n"
    "\t--help\n\n"
    "\t--input=fileroot\n"
    "\t--input-ctd-file=file\n"
    "\t--input-dvl-file=file\n"
    "\t--input-nav-file=file\n"
    "\t--input-rov-file=file\n"
    "\t--interpolate-position\n"
    "\t--interval=seconds\n"
    "\t--output=file\n"
    "\t--rov-dive-start=yyyymmddhhmmss\n"
    "\t--rov-dive-end=yyyymmddhhmmss\n"
    "\t--utm-zone=zone_id/NorS\n"
    "\t--verbose\n\n";

/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int option_index;
  int errflg = 0;
  int c;
  int help = MB_NO;

  /* ROV dive time start and end */
  int rov_dive_start_time_set = MB_NO;
  double rov_dive_start_time_d;
  int rov_dive_start_time_i[7];
  int rov_dive_end_time_set = MB_NO;
  double rov_dive_end_time_d;
  int rov_dive_end_time_i[7];
  int interpolate_position = MB_NO;

  /* MBIO status variables */
  int status = MB_SUCCESS;
  int verbose = 0;
  int error = MB_ERROR_NO_ERROR;
  char *message;

  int num_nav_alloc = 0;
  int num_nav = 0;
  double *nav_time_d = NULL;
  double *nav_lon = NULL;
  double *nav_lat = NULL;

  int num_ctd_alloc = 0;
  int num_ctd = 0;
  double *ctd_time_d = NULL;
  double *ctd_depth = NULL;

  int num_rov_alloc = 0;
  int num_rov = 0;
  double *rov_time_d = NULL;
  double *rov_heading = NULL;
  double *rov_roll = NULL;
  double *rov_pitch = NULL;

  int num_dvl_alloc = 0;
  int num_dvl = 0;
  double *dvl_time_d = NULL;
  double *dvl_altitude = NULL;
  double *dvl_stime = NULL;
  double *dvl_vx = NULL;
  double *dvl_vy = NULL;
  double *dvl_vz = NULL;
  double *dvl_status = NULL;

  double time_d;
  double rawlat, rawlon, dummydouble, ldegrees, lminutes;
  double reference_lon = 0.0;
  double reference_lat = 0.0;
  int utm_zone_set = MB_NO;
  int  utm_zone = 0;
  mb_path projection_id;
  void *pjptr = NULL;
  char NorS, EorW;
  mb_path dummystring;
  double ctd_C, ctd_T, ctd_D, ctd_S;
  double ctd_O2uM, ctd_O2raw, ctd_DGH_T, ctd_C2_T, ctd_C2_C;
  double rov_x, rov_y, rov_z, rov_yaw, rov_magna_amps;
  double rov_F1, rov_F2, rov_F3, rov_F4, rov_F5;
  double rov_Heading, rov_Pitch, rov_Roll;
  double dvl_Altitude, dvl_Stime, dvl_Vx, dvl_Vy, dvl_Vz, dvl_Status;

  double start_time_d = 0.0;
  double end_time_d = 0.0;
  double interval = 1.0;
  int onav_time_i[7], onav_time_j[5];
  int onav_year, onav_jday, onav_timetag;
  int num_output;
  double onav_time_d;
  double onav_lon;
  double onav_lat;
  double onav_easting;
  double onav_northing;
  double onav_depth;
  double onav_pressure;
  double onav_heading;
  double onav_altitude;
  double onav_pitch;
  double onav_roll;
  int onav_position_flag;
  int onav_pressure_flag;
  int onav_heading_flag;
  int onav_altitude_flag;
  int onav_attitude_flag;

  int nrecord;
  int nget, nscan;
  int ioutput;
  size_t size;
  FILE *fp;
  int jnav = 0;
  int jctd = 0;
  int jdvl = 0;
  int jrov = 0;
  int interp_status = MB_SUCCESS;
  int interp_error = MB_ERROR_NO_ERROR;
  int proj_status = 0;

  /* command line option definitions */
  /* mbminirovnav
   *     --verbose
   *     --help
   *
   *     --input-nav=file
   *     --input-ctd=file
   *     --output=file
   */
  static struct option options[] = {{"help", no_argument, NULL, 0},
                                    {"input", required_argument, NULL, 0},
                                    {"input-nav-file", required_argument, NULL, 0},
                                    {"input-ctd-file", required_argument, NULL, 0},
                                    {"input-dvl-file", required_argument, NULL, 0},
                                    {"input-rov-file", required_argument, NULL, 0},
                                    {"interpolate-position", no_argument, NULL, 0},
                                    {"interval", required_argument, NULL, 0},
                                    {"output", required_argument, NULL, 0},
                                    {"rov-dive-start", required_argument, NULL, 0},
                                    {"rov-dive-end", required_argument, NULL, 0},
                                    {"utm-zone", required_argument, NULL, 0},
                    {"verbose", no_argument, NULL, 0},
                                    {NULL, 0, NULL, 0}};

    /* files */
    mb_path input_nav_file = "";
    mb_path input_ctd_file = "";
    mb_path input_dvl_file = "";
    mb_path input_rov_file = "";
    mb_path output_file = "";

  /* process argument list */
  while ((c = getopt_long(argc, argv, "", options, &option_index)) != -1)
    switch (c) {
    /* long options all return c=0 */
    case 0:
      /* verbose */
      if (strcmp("verbose", options[option_index].name) == 0) {
        verbose++;
      }

      /* help */
      else if (strcmp("help", options[option_index].name) == 0) {
        help = MB_YES;
      }

      /*-------------------------------------------------------
       * Define input and output files */

      /* input-nav=file (required) */
      else if (strcmp("input-nav-file", options[option_index].name) == 0) {
        strcpy(input_nav_file, optarg);
      }

      /* input-rov=file (required) */
      else if (strcmp("input-rov-file", options[option_index].name) == 0) {
        strcpy(input_rov_file, optarg);
      }

      /* input-ctd=file (required) */
      else if (strcmp("input-ctd-file", options[option_index].name) == 0) {
        strcpy(input_ctd_file, optarg);
      }

      /* input-dvl=file (optional) */
      else if (strcmp("input-dvl-file", options[option_index].name) == 0) {
        strcpy(input_dvl_file, optarg);
      }

      /* output=file */
      else if (strcmp("output", options[option_index].name) == 0) {
        strcpy(output_file, optarg);
      }

      /* interval */
      else if (strcmp("interval", options[option_index].name) == 0) {
        nscan = sscanf(optarg, "%lf", &interval);
        if (interval <= 0.0) {
          fprintf(stderr,"Program %s command error: %s %s\n\toutput interval reset to 1.0 seconds\n",
              program_name, options[option_index].name, optarg);

        }
      }

      /* start rov dive time */
      else if (strcmp("rov-dive-start", options[option_index].name) == 0) {
        nscan = sscanf(optarg, "%d/%d/%d/%d/%d/%d", &rov_dive_start_time_i[0],
             &rov_dive_start_time_i[1], &rov_dive_start_time_i[2],
             &rov_dive_start_time_i[3], &rov_dive_start_time_i[4],
             &rov_dive_start_time_i[5]);
        if (nscan == 6) {
          rov_dive_start_time_i[6] = 0;
          mb_get_time(verbose, rov_dive_start_time_i, &rov_dive_start_time_d);
          rov_dive_start_time_set = MB_YES;
        }
        else {
          fprintf(stderr,"Program %s command error: %s %s\n",
              program_name, options[option_index].name, optarg);
        }
      }

      /* end rov dive time */
      else if (strcmp("rov-dive-end", options[option_index].name) == 0) {
        nscan = sscanf(optarg, "%d/%d/%d/%d/%d/%d", &rov_dive_end_time_i[0],
             &rov_dive_end_time_i[1], &rov_dive_end_time_i[2],
             &rov_dive_end_time_i[3], &rov_dive_end_time_i[4],
             &rov_dive_end_time_i[5]);
        if (nscan == 6) {
          rov_dive_end_time_i[6] = 0;
          mb_get_time(verbose, rov_dive_end_time_i, &rov_dive_end_time_d);
          rov_dive_end_time_set = MB_YES;
        }
        else {
          fprintf(stderr,"Program %s command error: %s %s\n",
              program_name, options[option_index].name, optarg);
        }
      }

      /* utm zone */
      else if (strcmp("utm-zone", options[option_index].name) == 0) {
        nscan = sscanf(optarg, "%d/%c", &utm_zone, &NorS);
        if (nscan < 2)
          nscan = sscanf(optarg, "%d%c", &utm_zone, &NorS);
        if (nscan == 2) {
          utm_zone_set = MB_YES;
          if (NorS == 'N' || NorS == 'n')
            sprintf(projection_id, "UTM%2.2dN", utm_zone);
          else if (NorS == 'S' || NorS == 's')
            sprintf(projection_id, "UTM%2.2dS", utm_zone);
          else
            sprintf(projection_id, "UTM%2.2dN", utm_zone);
        }
        else {
          fprintf(stderr,"Program %s command error: %s %s\n",
              program_name, options[option_index].name, optarg);
        }
      }

      /* interpolate position over gaps in USBL fixes (rather than repeat position values) */
      else if (strcmp("interpolate-position", options[option_index].name) == 0) {
        interpolate_position = MB_YES;
      }

      /*----------------------------------------------------------------*/

      break;
    case '?':
      errflg++;
      break;
    }

  /* if error flagged then print it and exit */
  if (errflg) {
    fprintf(stderr, "usage: %s\n", usage_message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    error = MB_ERROR_BAD_USAGE;
    exit(error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:                      %d\n", verbose);
    fprintf(stderr, "dbg2       help:                         %d\n", help);
    fprintf(stderr, "dbg2       input_nav_file:               %s\n", input_nav_file);
    fprintf(stderr, "dbg2       input_ctd_file:               %s\n", input_ctd_file);
    fprintf(stderr, "dbg2       input_dvl_file:               %s\n", input_dvl_file);
    fprintf(stderr, "dbg2       input_rov_file:               %s\n", input_rov_file);
    fprintf(stderr, "dbg2       output_file:                  %s\n", output_file);
    fprintf(stderr, "dbg2       output time interval:         %f\n", interval);
    fprintf(stderr, "dbg2       rov_dive_start_time_set:      %d\n", rov_dive_start_time_set);
    if (rov_dive_start_time_set == MB_YES)
      fprintf(stderr, "dbg2       rov_dive_start_time_i:        %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
          rov_dive_start_time_i[0], rov_dive_start_time_i[1], rov_dive_start_time_i[2],
          rov_dive_start_time_i[3], rov_dive_start_time_i[4], rov_dive_start_time_i[5],
          rov_dive_start_time_i[6]);
    fprintf(stderr, "dbg2       rov_dive_end_time_set:        %d\n", rov_dive_end_time_set);
    if (rov_dive_end_time_set == MB_YES)
      fprintf(stderr, "dbg2       rov_dive_end_time_i:          %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
          rov_dive_end_time_i[0], rov_dive_end_time_i[1], rov_dive_end_time_i[2],
          rov_dive_end_time_i[3], rov_dive_end_time_i[4], rov_dive_end_time_i[5],
          rov_dive_end_time_i[6]);
    fprintf(stderr, "dbg2       utm_zone_set:                 %d\n", utm_zone_set);
    if (utm_zone_set == MB_YES) {
      fprintf(stderr, "dbg2       utm_zone:                     %d\n", utm_zone);
      fprintf(stderr, "dbg2       projection_id:                %s\n", projection_id);
    }
    fprintf(stderr, "dbg2       interpolate_position:         %d\n", interpolate_position);
  }

  else if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  /* print starting verbose */
  if (verbose == 1) {
    time_t right_now;
    char date[32], user[128], *user_ptr, host[128];
    right_now = time((time_t *)0);
    strcpy(date, ctime(&right_now));
    date[strlen(date) - 1] = '\0';
    if ((user_ptr = getenv("USER")) == NULL)
      user_ptr = getenv("LOGNAME");
    if (user_ptr != NULL)
      strcpy(user, user_ptr);
    else
      strcpy(user, "unknown");
    gethostname(host, 128);
    fprintf(stdout, "Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
    fprintf(stdout, "Control Parameters:\n");
    fprintf(stdout, "\tverbose:                      %d\n", verbose);
    fprintf(stdout, "\thelp:                         %d\n", help);
    fprintf(stdout, "\tinput_nav_file:               %s\n", input_nav_file);
    fprintf(stdout, "\tinput_ctd_file:               %s\n", input_ctd_file);
    fprintf(stdout, "\tinput_dvl_file:               %s\n", input_dvl_file);
    fprintf(stdout, "\tinput_rov_file:               %s\n", input_rov_file);
    fprintf(stdout, "\toutput_file:                  %s\n", output_file);
    fprintf(stdout, "\toutput time interval:         %f\n", interval);
    fprintf(stdout, "\trov_dive_start_time_set:      %d\n", rov_dive_start_time_set);
    if (rov_dive_start_time_set == MB_YES)
      fprintf(stdout, "\trov_dive_start_time_i:        %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
          rov_dive_start_time_i[0], rov_dive_start_time_i[1], rov_dive_start_time_i[2],
          rov_dive_start_time_i[3], rov_dive_start_time_i[4], rov_dive_start_time_i[5],
          rov_dive_start_time_i[6]);
    fprintf(stdout, "\trov_dive_end_time_set:        %d\n", rov_dive_end_time_set);
    if (rov_dive_end_time_set == MB_YES)
      fprintf(stdout, "\trov_dive_end_time_i:          %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
          rov_dive_end_time_i[0], rov_dive_end_time_i[1], rov_dive_end_time_i[2],
          rov_dive_end_time_i[3], rov_dive_end_time_i[4], rov_dive_end_time_i[5],
          rov_dive_end_time_i[6]);
    fprintf(stdout, "\tutm_zone_set:                 %d\n", utm_zone_set);
    if (utm_zone_set == MB_YES) {
      fprintf(stdout, "\tutm_zone:                     %d\n", utm_zone);
      fprintf(stdout, "\tprojection_id:                %s\n", projection_id);
    }
    fprintf(stdout, "\tinterpolate_position:         %d\n", interpolate_position);
  }

  /* if help desired then print it and exit */
  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
    exit(error);
  }

  /*-------------------------------------------------------------------*/
  /* load input nav data */

  /* count the records */
  error = MB_ERROR_NO_ERROR;
  if ((fp = fopen(input_nav_file, "r")) != NULL) {
    /* loop over reading the records */
    int nrecord = 0;
    char buffer[MB_PATH_MAXLINE];
    char *result = NULL;
    while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer)
      if (buffer[0] != '#' && strlen(buffer) > 5)
        nrecord++;

    /* rewind the file */
    rewind(fp);

    /* allocate memory */
    if (num_nav_alloc < nrecord) {
      size = nrecord * sizeof(double);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&nav_time_d, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&nav_lon, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&nav_lat, &error);
      if (status == MB_SUCCESS)
        num_nav_alloc = nrecord;
    }

    /* read the records */
    if (status == MB_SUCCESS) {
      nrecord = 0;
      /* loop over reading the records */
      char buffer[MB_PATH_MAXLINE];
      char *result = NULL;
      while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer) {
        nget = sscanf(buffer, "%lf,$GPGLL,%lf,%c,%lf,%c,%lf,%s",
                &time_d, &rawlat, &NorS, &rawlon, &EorW, &dummydouble, dummystring);
        if (nget >= 1) {
          if (start_time_d <= 0.0)
            start_time_d = time_d;
          if (time_d > 0.0 && time_d < start_time_d)
            start_time_d = time_d;
          if (time_d > end_time_d)
            end_time_d = time_d;
          rawlat = 0.0;
          NorS = 'N';
          rawlon = 0.0;
          EorW = 'E';
        }
        if (nget >= 5) {
          nav_time_d[num_nav] = time_d;
          ldegrees = floor(rawlat / 100.0);
          lminutes = rawlat - ldegrees * 100;
          nav_lat[num_nav] = ldegrees + (lminutes / 60.0);
          if (NorS == 'S' || NorS == 's')
            nav_lat[num_nav] *= -1;
          ldegrees = floor(rawlon / 100.0);
          lminutes = rawlon - ldegrees * 100;
          nav_lon[num_nav] = ldegrees + (lminutes / 60.0);
          if (EorW == 'W' || EorW == 'w')
            nav_lon[num_nav] *= -1;
        }

        if (interpolate_position == MB_NO
          || num_nav <= 1
          || nav_lon[num_nav] != nav_lon[num_nav-1]
          || nav_lat[num_nav] != nav_lat[num_nav-1]) {
          reference_lon += nav_lon[num_nav];
          reference_lat += nav_lat[num_nav];
          if (num_nav < num_nav_alloc)
            num_nav++;
        }
      }

      /* calculate average longitude for UTM zone calcuation */
      if (num_nav > 0) {
        reference_lon /= num_nav;
        reference_lat /= num_nav;
      }
      if (reference_lon < 180.0)
        reference_lon += 360.0;
      if (reference_lon >= 180.0)
        reference_lon -= 360.0;
    }

    /* close the file */
    fclose(fp);
  }

  else {
    if (verbose) {
      fprintf(stderr, "\nUnable to open NAV file: %s\n", input_nav_file);
    }
  }

  /*-------------------------------------------------------------------*/
  /* load input ctd data */

  /* count the records */
  error = MB_ERROR_NO_ERROR;
  if ((fp = fopen(input_ctd_file, "r")) != NULL) {
    /* loop over reading the records */
    int nrecord = 0;
    char buffer[MB_PATH_MAXLINE];
    char *result = NULL;
    while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer)
      if (buffer[0] != '#' && strlen(buffer) > 5)
        nrecord++;

    /* rewind the file */
    rewind(fp);

    /* allocate memory if necessary */
    if (num_ctd_alloc < nrecord) {
      size = nrecord * sizeof(double);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&ctd_time_d, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&ctd_depth, &error);
      if (status == MB_SUCCESS)
        num_ctd_alloc = nrecord;
    }

    /* loop over reading the records */
    if (status == MB_SUCCESS) {
      nrecord = 0;
      char buffer[MB_PATH_MAXLINE];
      char *result = NULL;
      while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer) {
        nget = sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                &time_d, &ctd_C, &ctd_T, &ctd_D, &ctd_S,
                &ctd_O2uM, &ctd_O2raw, &ctd_DGH_T, &ctd_C2_T, &ctd_C2_C);
        if (nget == 10) {
          if (start_time_d <= 0.0)
            start_time_d = time_d;
          if (time_d > 0.0 && time_d < start_time_d)
            start_time_d = time_d;
          if (time_d > end_time_d)
            end_time_d = time_d;

          ctd_time_d[num_ctd] = time_d;
          ctd_depth[num_ctd] = ctd_D;
          if (num_ctd < num_ctd_alloc)
            num_ctd++;
          }
        }
    }

    /* close the file */
    fclose(fp);
  }

  else {
    if (verbose) {
      fprintf(stderr, "\nUnable to open CTD file: %s\n", input_ctd_file);
    }
  }

  /*-------------------------------------------------------------------*/
  /* load input rov data */

  /* count the records */
  error = MB_ERROR_NO_ERROR;
  nrecord = 0;
  if ((fp = fopen(input_rov_file, "r")) != NULL) {
    /* loop over reading the records */
    char buffer[MB_PATH_MAXLINE];
    char *result = NULL;
    while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer)
      if (buffer[0] != '#' && strlen(buffer) > 5)
        nrecord++;

    /* rewind the file */
    rewind(fp);

    /* allocate memory */
    if (num_rov_alloc < nrecord) {
      size = nrecord * sizeof(double);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&rov_time_d, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&rov_heading, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&rov_roll, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&rov_pitch, &error);
      if (status == MB_SUCCESS)
        num_rov_alloc = nrecord;
    }

    /* read the records */
    if (status == MB_SUCCESS) {
      nrecord = 0;
      /* loop over reading the records */
      char buffer[MB_PATH_MAXLINE];
      char *result = NULL;
      while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer) {
        nget = sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                &time_d, &rov_x, &rov_y, &rov_z, &rov_yaw, &rov_magna_amps,
                &rov_F1, &rov_F2, &rov_F3, &rov_F4, &rov_F5,
                &rov_Heading, &rov_Pitch, &rov_Roll);
        if (nget == 14) {
          if (start_time_d <= 0.0)
            start_time_d = time_d;
          if (time_d > 0.0 && time_d < start_time_d)
            start_time_d = time_d;
          if (time_d > end_time_d)
            end_time_d = time_d;

          rov_time_d[num_rov] = time_d;
          rov_heading[num_rov] = rov_Heading;
          rov_roll[num_rov] = rov_Roll;
          rov_pitch[num_rov] = rov_Pitch;
          if (num_rov < num_rov_alloc)
            num_rov++;
        }
      }
    }

    /* close the file */
    fclose(fp);
  }

  else {
    if (verbose) {
      fprintf(stderr, "\nUnable to open ROV file: %s\n", input_rov_file);
    }
  }

  /*-------------------------------------------------------------------*/
  /* load input dvl data */

  /* count the records */
  error = MB_ERROR_NO_ERROR;
  if ((fp = fopen(input_dvl_file, "r")) != NULL) {
    /* loop over reading the records */
    int nrecord = 0;
    char buffer[MB_PATH_MAXLINE];
    char *result = NULL;
    while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer)
      if (buffer[0] != '#' && strlen(buffer) > 5)
        nrecord++;

    /* rewind the file */
    rewind(fp);

    /* allocate memory if necessary */
    if (status == MB_SUCCESS && num_dvl_alloc < nrecord) {
      size = nrecord * sizeof(double);
      status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_time_d, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_altitude, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_stime, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_vx, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_vy, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_vz, &error);
      if (status == MB_SUCCESS)
        status &= mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&dvl_status, &error);
      if (status == MB_SUCCESS)
        num_dvl_alloc = nrecord;
    }

    /* read the records */
    if (status == MB_SUCCESS) {
      nrecord = 0;
      /* loop over reading the records - handle the different formats */
      char buffer[MB_PATH_MAXLINE];
      char *result = NULL;
      while ((result = fgets(buffer, (MB_PATH_MAXLINE - 1), fp)) == buffer) {
        nget = sscanf(buffer, "%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                &time_d, &dvl_Altitude, &dvl_Stime, &dvl_Vx, &dvl_Vy, &dvl_Vz, &dvl_Status);
        if (nget == 7) {
          if (start_time_d <= 0.0)
            start_time_d = time_d;
          if (time_d > 0.0 && time_d < start_time_d)
            start_time_d = time_d;
          if (time_d > end_time_d)
            end_time_d = time_d;

          dvl_time_d[num_dvl] = time_d;
          dvl_altitude[num_dvl] = dvl_Altitude;
          dvl_stime[num_dvl] = dvl_Stime;
          dvl_vx[num_dvl] = dvl_Vx;
          dvl_vy[num_dvl] = dvl_Vy;
          dvl_vz[num_dvl] = dvl_Vz;
          dvl_status[num_dvl] = dvl_Status;
          if (num_dvl < num_dvl_alloc)
            num_dvl++;
        }
      }

      /* close the file */
      fclose(fp);
    }
  }

  else {
    if (verbose) {
      fprintf(stderr, "\nUnable to open DVL file: %s\n", input_dvl_file);
    }
  }

  /*-------------------------------------------------------------------*/

  /* get time range of output based on max bounds of any input data
    or use the specified time interval */
  if (rov_dive_start_time_set == MB_YES) {
    start_time_d = rov_dive_start_time_d;
  }
  if (rov_dive_end_time_set == MB_YES) {
    end_time_d = rov_dive_end_time_d;
  }
  start_time_d = floor(start_time_d);
  num_output = (int)(ceil((end_time_d - start_time_d) / interval));
  end_time_d = start_time_d + num_output * interval;

  /* get UTM projection for easting and northing fields */
  if (utm_zone_set == MB_YES) {
    if (utm_zone < 0)
      sprintf(projection_id, "UTM%2.2dS", abs(utm_zone));
    else
      sprintf(projection_id, "UTM%2.2dN", utm_zone);
  }
  else {
    utm_zone = (int)(((reference_lon + 183.0) / 6.0) + 0.5);
    if (reference_lat >= 0.0)
      sprintf(projection_id, "UTM%2.2dN", utm_zone);
    else
      sprintf(projection_id, "UTM%2.2dS", utm_zone);
  }
  proj_status = mb_proj_init(verbose, projection_id, &(pjptr), &error);

  /* write the MiniROV navigation data */
  int num_position_valid = 0;
  int num_depth_valid = 0;
  int num_heading_valid = 0;
  int num_attitude_valid = 0;
  int num_altitude_valid = 0;
  if (status == MB_SUCCESS && num_nav > 0 && num_rov > 0) {
    if ((fp = fopen(output_file, "w")) == NULL) {
      error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
    }
    else {

      /* loop over defined intervals (1 second by default) from start time to end time */
      for (ioutput=0;ioutput<num_output;ioutput++) {

        /* set the output time */
        onav_time_d = start_time_d + ioutput * interval;
        mb_get_date(verbose, onav_time_d, onav_time_i);
        onav_year = onav_time_i[0];
        onav_timetag = 10000 * onav_time_i[3] + 100 * onav_time_i[4] + onav_time_i[5];
        mb_get_jtime(verbose, onav_time_i, onav_time_j);
        onav_jday = onav_time_j[1];

        /* interpolate values onto the target time */
        onav_position_flag = MB_NO;
        onav_pressure_flag = MB_NO;
        onav_heading_flag = MB_NO;
        onav_altitude_flag = MB_NO;
        onav_attitude_flag = MB_NO;
        onav_lon = 0.0;
        onav_lat = 0.0;
        onav_easting = 0.0;
        onav_northing = 0.0;
        onav_depth = 0.0;
        onav_altitude = 0.0;
        onav_roll = 0.0;
        onav_pitch = 0.0;
        if (num_nav > 0) {
          interp_status &= mb_linear_interp_longitude(verbose, nav_time_d - 1, nav_lon - 1, num_nav, onav_time_d, &onav_lon, &jnav, &interp_error);
          interp_status &= mb_linear_interp_latitude(verbose, nav_time_d - 1, nav_lat - 1, num_nav, onav_time_d, &onav_lat, &jnav, &interp_error);

          // if not interpolating navigation, then actually use the most recent
          // navigation values even if those are repeated, as identified by the
          // jnav value returned by the interpolation function
          if (interpolate_position == MB_YES) {
            onav_lon = nav_lon[jnav-1];
            onav_lat = nav_lat[jnav-1];
          }

          if (onav_lon != 0.0 && onav_lat != 0.0) {
            onav_position_flag = MB_YES;
            mb_proj_forward(verbose, pjptr, onav_lon, onav_lat, &onav_easting, &onav_northing, &error);
          }
        }
        if (num_ctd > 0) {
          interp_status = mb_linear_interp(verbose, ctd_time_d - 1, ctd_depth - 1, num_ctd, onav_time_d, &onav_depth, &jctd, &interp_error);
          if (onav_depth != 0.0) {
            onav_pressure_flag = MB_YES;
            onav_pressure = onav_depth * (1.0052405 * (1 + 5.28E-3 * sin(DTR * onav_lat) * sin(DTR * onav_lat)));
          }
        }
        if (num_dvl > 0) {
          interp_status = mb_linear_interp(verbose, dvl_time_d - 1, dvl_altitude - 1, num_dvl, onav_time_d, &onav_altitude, &jdvl, &interp_error);
          if (onav_altitude != 0.0) {
            onav_altitude_flag = MB_YES;
          }
        }
        if (num_rov > 0) {
          interp_status = mb_linear_interp_heading(verbose, rov_time_d - 1, rov_heading - 1, num_rov, onav_time_d, &onav_heading, &jrov, &interp_error);
          if (onav_heading != 0.0) {
            onav_heading_flag = MB_YES;
          }
          interp_status = mb_linear_interp(verbose, rov_time_d - 1, rov_roll - 1, num_rov, onav_time_d, &onav_roll, &jrov, &interp_error);
          interp_status = mb_linear_interp(verbose, rov_time_d - 1, rov_pitch - 1, num_rov, onav_time_d, &onav_pitch, &jrov, &interp_error);
          if (onav_roll != 0.0 && onav_pitch != 0.0) {
            onav_attitude_flag = MB_YES;
          }
        }

        if (verbose >= 4) {
          fprintf(stderr, "\ndbg4  Data to be written in MBIO function <%s>\n", program_name);
          fprintf(stderr, "dbg4  Values,read:\n");
          fprintf(stderr, "dbg4       onav_time_d:         %f\n", onav_time_d);
          fprintf(stderr, "dbg4       onav_lat:            %f\n", onav_lat);
          fprintf(stderr, "dbg4       onav_lon:            %f\n", onav_lon);
          fprintf(stderr, "dbg4       onav_easting:        %f\n", onav_easting);
          fprintf(stderr, "dbg4       onav_northing:       %f\n", onav_northing);
          fprintf(stderr, "dbg4       onav_depth:          %f\n", onav_depth);
          fprintf(stderr, "dbg4       onav_pressure:       %f\n", onav_pressure);
          fprintf(stderr, "dbg4       onav_heading:        %f\n", onav_heading);
          fprintf(stderr, "dbg4       onav_altitude:       %f\n", onav_altitude);
          fprintf(stderr, "dbg4       onav_pitch:          %f\n", onav_pitch);
          fprintf(stderr, "dbg4       onav_roll:           %f\n", onav_roll);
          fprintf(stderr, "dbg4       onav_position_flag:  %d\n", onav_position_flag);
          fprintf(stderr, "dbg4       onav_pressure_flag:  %d\n", onav_pressure_flag);
          fprintf(stderr, "dbg4       onav_heading_flag:   %d\n", onav_heading_flag);
          fprintf(stderr, "dbg4       onav_altitude_flag:  %d\n", onav_altitude_flag);
          fprintf(stderr, "dbg4       onav_attitude_flag:  %d\n", onav_attitude_flag);
          fprintf(stderr, "dbg4       error:               %d\n", error);
          fprintf(stderr, "dbg4       status:              %d\n", status);
        }

        /* write the record */
        fprintf(fp, "%4.4d,%3.3d,%6.6d,%9.0f,%10.6f,%11.6f,%7.0f,%7.0f,%7.2f,%5.1f,%6.2f,%4.1f,%4.1f,%d,%d,%d,%d,%d\n",
            onav_year, onav_jday, onav_timetag, onav_time_d,
            onav_lat, onav_lon, onav_easting, onav_northing,
            onav_pressure, onav_heading, onav_altitude, onav_pitch, onav_roll,
            onav_position_flag, onav_pressure_flag, onav_heading_flag,
            onav_altitude_flag, onav_attitude_flag);
        num_position_valid += onav_position_flag;
        num_depth_valid += onav_pressure_flag;
        num_heading_valid += onav_heading_flag;
        num_attitude_valid += onav_attitude_flag;
        num_altitude_valid += onav_altitude_flag;

      }

      /* close the file */
      fclose(fp);
    }
  }

  if (verbose) {
    int time_i[7];
    fprintf(stdout,"Input data:\n\tNavigation:     %5d\n\tCTD:            %5d\n\tAttitude:       %5d\n\tDVL:            %5d\n",
        num_nav, num_ctd, num_rov, num_dvl);
    fprintf(stdout, "Output file: %s\n", output_file);
    fprintf(stdout, "\tOutput records: %d\n", num_output);
    mb_get_date(verbose, start_time_d, time_i);
    fprintf(stdout, "\tStart time:     %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
    mb_get_date(verbose, end_time_d, time_i);
    fprintf(stdout, "\tEnd time:       %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6]);
    fprintf(stdout,"Valid output data:\n\tPosition:       %5d\n\tDepth:          %5d\n\tHeading:        %5d\n\tAttitude:       %5d\n\tAltitude:       %5d\n\n",
        num_position_valid, num_depth_valid, num_heading_valid, num_attitude_valid, num_altitude_valid);
  }

  /*-------------------------------------------------------------------*/

  proj_status = mb_proj_free(verbose, &(pjptr), &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_time_d, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_lon, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&nav_lat, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ctd_time_d, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&ctd_depth, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&rov_time_d, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&rov_heading, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&rov_roll, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&rov_pitch, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_time_d, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_altitude, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_stime, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_vx, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_vy, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_vz, &error);
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&dvl_status, &error);

  exit(status);

  /*-------------------------------------------------------------------*/

}
