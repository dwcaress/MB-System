/*--------------------------------------------------------------------
 *    The MB-system:  mbotps.c  7/30/2009
 *
 *    Copyright (c) 2009-2023 by
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
 * MBotps predicts tides using methods and data derived
 * from the OSU Tidal Prediction Software (OTPS) distributions
 * distributed from:
 *      http://www.coas.oregonstate.edu/research/po/research/tide/
 * The OTPS distributions include programs written in Fortran 90 that
 * operate in batch mode with specified control files. This program is
 * intended to provide the same tidal prediction capability through a
 * command line interface more consistent with MB-System.
 *
 * The mbotps usage is:
 *       mbotps [-Atideformat -Byear/month/day/hour/minute/second
 *              -Ctidestationformat -Dinterval
 *              -Eyear/month/day/hour/minute/second -Fformat -Idatalist
 *              -Lopts_path -Ntidestationfile -Ooutput -Potps_location
 *             -Rlon/lat -S -Tmodel -Utidestationlon/tidestationlat -V]
 *
 * This program can be used in two modes. In the first, the user
 * specifies a location (-Rlon/lat), start and end times (-B and -E),
 * and a tidal sampling interval (-D). The program then writes a two
 * column tide time series consisting of epoch time values in seconds followed
 * by tide values in meters for the specified location and times. The
 * output is to a file specified with -Otide_file.
 *
 * In the second mode, the user specifies one or more swath data files using
 * -Idatalist.mb-1. A tide file is generated for each swath file by
 * outputing the time and tide value for the sonar navigation sampled
 * according to -Dinterval. MBotps also sets the parameter file for each
 * swath file so that mbprocess applies the tide model during processing.
 *
 * The -Ctidestationformat, -Ntidestationfile, and  -Utidestationlon/tidestationlat
 * commands together allow users to input observations from a tide station;
 * these observations can be used to calculate corrections to tidal model values
 * in the vicinity of the tide station. If tide station data are specified,
 * then MBotps calculates the difference between the observed and modeled tide
 * at that station for each data point in the input tide station data. This
 * difference time series is then used as a correction to the output tide models,
 * whether at a location specified with the -Rlon/lat option or for swath data
 * specified with the -Idatalist option.

 * Author:  D. W. Caress
 * Date:  July 30,  2009
 * Date:  April 5,  2018
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_process.h"
#include "mb_status.h"

/* OTPS installation location */
#include "otps.h"

#define MBOTPS_MODE_POSITION            0x00
#define MBOTPS_MODE_NAVIGATION          0x01
#define MBOTPS_MODE_TIDESTATION         0x02
#define MBOTPS_MODE_NAV_WRT_STATION     0x03
#define MBOTPS_DEFAULT_MODEL "tpxo9_atlas"

static char program_name[] = "mbotps";
static char help_message[] =
    "MBotps predicts tides using methods and data derived from the "
    "OSU Tidal Prediction Software (OTPS) distributions.";
static char usage_message[] =
    "mbotps [-Atideformat -Byear/month/day/hour/minute/second -Ctidestationformat\n"
    "\t-Dinterval -Eyear/month/day/hour/minute/second -Fformat\n"
    "\t-Idatalist -Lopts_path -Ntidestationfile -Ooutput -Potps_location\n"
    "\t-Rlon/lat -S -Tmodel -Utidestationlon/tidestationlat -V]";
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

  int status = mb_defaults(
      verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i,
      &speedmin, &timegap);
  btime_i[0] = 2009;
  btime_i[1] = 7;
  btime_i[2] = 31;
  btime_i[3] = 0;
  btime_i[4] = 0;
  btime_i[5] = 0;
  btime_i[6] = 0;
  etime_i[0] = 2009;
  etime_i[1] = 8;
  etime_i[2] = 2;
  etime_i[3] = 1;
  etime_i[4] = 0;
  etime_i[5] = 0;
  etime_i[6] = 0;

  /* set default location of the OTPS package */
  mb_path otps_location_use;
  strcpy(otps_location_use, otps_location);

  // Set defaults for the AUV survey we were running on Coaxial Segment, Juan de Fuca Ridge
  bool otps_model_set = false;
  mb_path otps_model = MBOTPS_DEFAULT_MODEL;
  mb_path tide_file = "tide_model.txt";
  double tidelon = -129.588618;
  double tidelat = 46.50459;
  double interval = 60.0;  // TODO(schwehr): Why not the 300.0?
  int tideformat = 2;
  int tidestation_format = 2;
  mb_path read_file = "datalist.mb-1";
  int mbotps_mode = MBOTPS_MODE_POSITION;
  bool mbprocess_update = false;
  mb_path tidestation_file;
  bool skip_existing = false;
  double tidestation_lon = 0.0;
  double tidestation_lat = 0.0;

  /* process argument list */
  bool help = false;
  {
    bool errflg = false;
    int c;
    while ((c =
            getopt(argc, argv, "A:a:B:b:C:c:D:d:E:e:F:f:I:i:MmN:n:O:o:P:p:R:r:SST:t:U:u:VvHh")) != -1)
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
          sscanf(optarg, "%d", &tideformat);
          if (tideformat != 2)
            tideformat = 1;
          break;
        case 'B':
        case 'b':
          sscanf(optarg, "%d/%d/%d/%d/%d/%d",
                 &btime_i[0], &btime_i[1], &btime_i[2],
                 &btime_i[3], &btime_i[4], &btime_i[5]);
          btime_i[6] = 0;
          break;
        case 'C':
        case 'c':
          sscanf(optarg, "%d", &tidestation_format);
          if (tidestation_format < 1 || tidestation_format > 4)
            tidestation_format = 2;
          break;
        case 'D':
        case 'd':
          sscanf(optarg, "%lf", &interval);
          break;
        case 'E':
        case 'e':
          sscanf(optarg, "%d/%d/%d/%d/%d/%d",
                 &etime_i[0], &etime_i[1], &etime_i[2],
                 &etime_i[3], &etime_i[4], &etime_i[5]);
          etime_i[6] = 0;
          break;
        case 'F':
        case 'f':
          sscanf(optarg, "%d", &format);
      break;
        case 'I':
        case 'i':
          sscanf(optarg, "%s", read_file);
          mbotps_mode = mbotps_mode | MBOTPS_MODE_NAVIGATION;
          break;
        case 'M':
        case 'm':
          mbprocess_update = true;
          break;
        case 'N':
        case 'n':
          sscanf(optarg, "%s", tidestation_file);
          mbotps_mode = mbotps_mode | MBOTPS_MODE_TIDESTATION;
          break;
        case 'O':
        case 'o':
          sscanf(optarg, "%s", tide_file);
          break;
        case 'P':
        case 'p':
          sscanf(optarg, "%s", otps_location_use);
          break;
        case 'R':
        case 'r':
          sscanf(optarg, "%lf/%lf", &tidelon, &tidelat);
          break;
        case 'S':
        case 's':
          skip_existing = true;
          break;
        case 'T':
        case 't':
          sscanf(optarg, "%s", otps_model);
          otps_model_set = true;
          break;
        case 'U':
        case 'u':
          sscanf(optarg, "%lf/%lf", &tidestation_lon, &tidestation_lat);
          break;
        case '?':
          errflg = true;
      }

    if (errflg) {
      fprintf(stderr, "usage: %s\n", usage_message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_BAD_USAGE);
    }
  }

  if (verbose == 1 || help) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  if (help) {
    fprintf(stderr, "\n%s\n", help_message);
    fprintf(stderr, "\nusage: %s\n", usage_message);
  }

  /* Check for available tide models */
  if (help || verbose > 0) {
    fprintf(stderr, "\nChecking for available OTPS tide models\n");
    fprintf(stderr, "  OTPS location: %s\n  Default OTPS model name: %s\n  Possible OTPS tidal models:\n",
            otps_location_use, MBOTPS_DEFAULT_MODEL);
  }

  int notpsmodels = 0;

  {
    mb_command command = "";
    snprintf(command, sizeof(command), "/bin/ls -1 %s/DATA | grep Model_ | sed \"s/^Model_//\"", otps_location_use);

    FILE *tfp = popen(command, "r");
    if (tfp == NULL) {
      // error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open ls using popen():\n%s\n", command);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
    }

    /* send relevant input to predict_tide through its stdin stream */
    mb_path line = "";
    while (fgets(line, sizeof(line), tfp)) {
      mb_path modelname = "";
      sscanf(line, "%s", modelname);
      mb_command modelfile = "";
      snprintf(modelfile, sizeof(modelfile), "%s/DATA/Model_%s", otps_location_use, modelname);
      fprintf(stderr, "    %s", modelname);

      /* check the files referenced in the model file */
      int nmodeldatafiles = 0;
      FILE *mfp = fopen(modelfile, "r");
      if (mfp != NULL) {
        /* stat the file referenced in each line */
        while (fgets(line, sizeof(line), mfp) != NULL) {
          char modeldatafile[2*MB_PATHPLUS_MAXLINE] = "";
          sscanf(line, "%s", modeldatafile);
          if (line[0] == '/') {
            strncpy(modeldatafile, line, sizeof(line));
          }
          else {
            snprintf(modeldatafile, sizeof(modeldatafile), "%s/%s", otps_location_use, line);
          }

          snprintf(command, sizeof(command), "/bin/ls -1 %s 2>/dev/null", modeldatafile);
          FILE *lfp = popen(command, "r");
          if (fgets(line, sizeof(line), lfp) != NULL
              && strlen(line) > 0) {
            nmodeldatafiles++;
          }
        }
        fclose(mfp);
      }
      if (help || verbose > 0) {
        if (nmodeldatafiles >= 3) {
          fprintf(stderr, " <installed>\n");
        } else {
          fprintf(stderr, " <not installed>\n");
        }
      }
      if (nmodeldatafiles >= 3) {
        if (!otps_model_set &&
            (notpsmodels == 0 || strcmp(modelname, MBOTPS_DEFAULT_MODEL) == 0)) {
            strcpy(otps_model, modelname);
        }
        notpsmodels++;
      }
    }

    /* close the process */
    pclose(tfp);
  }
  if (help || verbose > 0) {
    fprintf(stderr, "  Number of available OTPS tide models: %d\n", notpsmodels);
    fprintf(stderr, "Using OTPS tide model:                %s\n", otps_model);
  }

  /* exit if no valid OTPS models can be found */
  if (notpsmodels <= 0) {
    // error = MB_ERROR_OPEN_FAIL;
    fprintf(stderr, "\nUnable to find a valid OTPS tidal model\n");
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(MB_FAILURE);
  }

  if (help)
    exit(MB_ERROR_NO_ERROR);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       help:                 %d\n", help);
    fprintf(stderr, "dbg2       otps_location:        %s\n", otps_location);
    fprintf(stderr, "dbg2       otps_location_use:    %s\n", otps_location_use);
    fprintf(stderr, "dbg2       otps_model_set:       %d\n", otps_model_set);
    fprintf(stderr, "dbg2       otps_model:           %s\n", otps_model);
    fprintf(stderr, "dbg2       mbotps_mode:          %d\n", mbotps_mode);
    fprintf(stderr, "dbg2       tidelon:              %f\n", tidelon);
    fprintf(stderr, "dbg2       tidelat:              %f\n", tidelat);
    fprintf(stderr, "dbg2       tidestation_file:     %s\n", tidestation_file);
    fprintf(stderr, "dbg2       tidestation_lon:       %f\n", tidestation_lon);
    fprintf(stderr, "dbg2       tidestation_lat:       %f\n", tidestation_lat);
    fprintf(stderr, "dbg2       tidestation_format:    %d\n", tidestation_format);
    fprintf(stderr, "dbg2       btime_i[0]:           %d\n", btime_i[0]);
    fprintf(stderr, "dbg2       btime_i[1]:           %d\n", btime_i[1]);
    fprintf(stderr, "dbg2       btime_i[2]:           %d\n", btime_i[2]);
    fprintf(stderr, "dbg2       btime_i[3]:           %d\n", btime_i[3]);
    fprintf(stderr, "dbg2       btime_i[4]:           %d\n", btime_i[4]);
    fprintf(stderr, "dbg2       btime_i[5]:           %d\n", btime_i[5]);
    fprintf(stderr, "dbg2       btime_i[6]:           %d\n", btime_i[6]);
    fprintf(stderr, "dbg2       etime_i[0]:           %d\n", etime_i[0]);
    fprintf(stderr, "dbg2       etime_i[1]:           %d\n", etime_i[1]);
    fprintf(stderr, "dbg2       etime_i[2]:           %d\n", etime_i[2]);
    fprintf(stderr, "dbg2       etime_i[3]:           %d\n", etime_i[3]);
    fprintf(stderr, "dbg2       etime_i[4]:           %d\n", etime_i[4]);
    fprintf(stderr, "dbg2       etime_i[5]:           %d\n", etime_i[5]);
    fprintf(stderr, "dbg2       etime_i[6]:           %d\n", etime_i[6]);
    fprintf(stderr, "dbg2       interval:             %f\n", interval);
    fprintf(stderr, "dbg2       tide_file:            %s\n", tide_file);
    fprintf(stderr, "dbg2       mbprocess_update:     %d\n", mbprocess_update);
    fprintf(stderr, "dbg2       skip_existing:        %d\n", skip_existing);
    fprintf(stderr, "dbg2       tideformat:           %d\n", tideformat);
    fprintf(stderr, "dbg2       format:               %d\n", format);
    fprintf(stderr, "dbg2       read_file:            %s\n", read_file);
  }

  int error = MB_ERROR_NO_ERROR;
  int ntidestation = 0;
  mb_path line = "";  // TODO(schwehr): Localize
  double *tidestation_time_d = NULL;
  double *tidestation_tide = NULL;
  double *tidestation_model = NULL;
  double *tidestation_correction = NULL;
  int time_i[7];
  int time_j[5];
  double sec;
  double time_d;
  int ihr;
  mb_pathplus predict_tide = "";
  int ngood;
  double lon;
  double lat;
  double depth;
  double tide;

  /* -------------------------------------------------------------------------
   * if specified read in tide station data and calculate model values for the
   * same location and times- the difference is applied as a correction to the
   * model values calculated at the desired locations and times
   * -----------------------------------------------------------------------*/
  if (mbotps_mode & MBOTPS_MODE_TIDESTATION) {
    /* make sure longitude is positive */
    if (tidestation_lon < 0.0)
      tidestation_lon += 360.0;

    /* open the tide station data file */
    FILE *tfp = fopen(tidestation_file, "r");
    if (tfp == NULL) {
      // error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr,
        "\nUnable to open tide station file <%s> for writing\n",
        tidestation_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
    }

    /* count the lines in the tide station data */
    ntidestation = 0;
    {
      char *result;
      while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line)
        ntidestation++;
    }
    rewind(tfp);

    /* allocate memory for tide station arrays */
    const int size = ntidestation * sizeof(double);
    status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_time_d, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_tide, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_model, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&tidestation_correction, &error);
    if (error != MB_ERROR_NO_ERROR) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* read the tide station data in the specified format */
    ntidestation = 0;
    char *result;
    while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
      bool tidestation_ok = false;

      /* ignore comments */
      if (line[0] != '#') {
        /* deal with tide station data in form: time_d tide */
        if (tidestation_format == 1) {
          const int nget = sscanf(line,
            "%lf %lf",
            &tidestation_time_d[ntidestation],
            &tidestation_tide[ntidestation]);
          if (nget == 2)
            tidestation_ok = true;
        } else if (tidestation_format == 2) {
          // deal with tide station data in form: yr mon day hour min sec tide
          const int nget = sscanf(line,
            "%d %d %d %d %d %lf %lf",
            &time_i[0],
            &time_i[1],
            &time_i[2],
            &time_i[3],
            &time_i[4],
            &sec,
            &tidestation_tide[ntidestation]);
          time_i[5] = (int)sec;
          time_i[6] = 1000000 * (sec - time_i[5]);
          mb_get_time(verbose, time_i, &time_d);
          tidestation_time_d[ntidestation] = time_d;
          if (nget == 7)
            tidestation_ok = true;
        } else if (tidestation_format == 3) {
          /* deal with tide station data in form: yr jday hour min sec tide */
          const int nget = sscanf(line,
            "%d %d %d %d %lf %lf",
            &time_j[0],
            &time_j[1],
            &ihr,
            &time_j[2],
            &sec,
            &tidestation_tide[ntidestation]);
          time_j[2] = time_j[2] + 60 * ihr;
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          tidestation_time_d[ntidestation] = time_d;
          if (nget == 6)
            tidestation_ok = true;
        } else if (tidestation_format == 4) {
          /* deal with tide station data in form: yr jday daymin sec tide */
          const int nget = sscanf(line,
            "%d %d %d %lf %lf",
            &time_j[0],
            &time_j[1],
            &time_j[2],
            &sec,
            &tidestation_tide[ntidestation]);
          time_j[3] = (int)sec;
          time_j[4] = 1000000 * (sec - time_j[3]);
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          tidestation_time_d[ntidestation] = time_d;
          if (nget == 5)
            tidestation_ok = true;
          }
        }

      /* output some debug values */
      if (verbose >= 5 && tidestation_ok)
        {
        fprintf(stderr, "\ndbg5  New tide point read in program <%s>\n", program_name);
        fprintf(stderr, "dbg5       tide[%d]: %f %f\n", ntidestation,
          tidestation_time_d[ntidestation], tidestation_tide[ntidestation]);
        }
      else if (verbose >= 5)
        {
        fprintf(stderr,
          "\ndbg5  Error parsing line in tide file in program <%s>\n",
          program_name);
        fprintf(stderr, "dbg5       line: %s\n", line);
        }

      /* check for reverses or repeats in time */
      if (tidestation_ok) {
        if (ntidestation == 0) {
          ntidestation++;
        } else if (tidestation_time_d[ntidestation] > tidestation_time_d[ntidestation - 1]) {
          ntidestation++;
        } else if (ntidestation > 0 &&
                   tidestation_time_d[ntidestation] <= tidestation_time_d[ntidestation - 1] &&
                   verbose >= 5) {
          fprintf(stderr, "\ndbg5  Tide time error in program <%s>\n", program_name);
          fprintf(stderr,
                  "dbg5       tide[%d]: %f %f\n",
                  ntidestation - 1,
                  tidestation_time_d[ntidestation - 1],
                  tidestation_tide[ntidestation - 1]);
          fprintf(stderr,
                  "dbg5       tide[%d]: %f %f\n",
                  ntidestation,
                  tidestation_time_d[ntidestation],
                  tidestation_tide[ntidestation]);
        }
      }
      strncpy(line, "", sizeof(line));
    }
    fclose(tfp);

    /* now get time and tide model values at the tide station location */

    /* Note: because predict_tide is a 1970's style Fortran batch program
          that limits filenames to 80 (!) characters, we put the temporary
          files in the user's home directory, which hopefully leads to
          adequately short pathnames. */

    /* first open temporary file of lat lon time */
    int pid = getpid();
    mb_path wd = "";
    getcwd(wd, sizeof(wd));
    mb_pathplus lltfile = "";
    mb_pathplus otpsfile = "";
    snprintf(lltfile, sizeof(lltfile), "%s/t%d.txt", getenv("HOME"), pid);
    snprintf(otpsfile, sizeof(otpsfile), "%s/u%d.txt", getenv("HOME"), pid);
    if ((tfp = fopen(lltfile, "w")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr,
        "\nUnable to open temporary lat-lon-time file <%s> for writing\n",
        lltfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }
    else {
      for (int i = 0; i < ntidestation; i++) {
        mb_get_date(verbose, tidestation_time_d[i], time_i);
        fprintf(tfp, "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d\n",
          tidestation_lat, tidestation_lon,
          time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5]);
      }
      fclose(tfp);
    }

    /* call predict_tide with popen */
    snprintf(predict_tide, sizeof(predict_tide), "cd %s; ./predict_tide", otps_location_use);
    if ((tfp = popen(predict_tide, "w")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    /* send relevant input to predict_tide through its stdin stream */
    fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
    fprintf(tfp, "%s\n", lltfile);
    fprintf(tfp, "z\n\nAP\noce\n1\n");
    //fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");
    fprintf(tfp, "%s\n", otpsfile);

    /* close the process */
    pclose(tfp);

    /* now read results from predict_tide and rewrite them in a useful form */
    if ((tfp = fopen(otpsfile, "r")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open predict_time results temporary file <%s>\n",
        otpsfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    int nline = 0;
    ngood = 0;
    while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
      nline++;
      if (nline > 6) {
        const int nget = sscanf(line,
          "%lf %lf %d.%d.%d %d:%d:%d %lf %lf",
          &lat, &lon,
          &time_i[1], &time_i[2], &time_i[0], &time_i[3], &time_i[4], &time_i[5],
          &tide, &depth);
        if (nget == 10) {
          tidestation_model[ngood] = tide;
          tidestation_correction[ngood] = tidestation_tide[ngood] - tidestation_model[ngood];
          ngood++;
        }
      }
    }
    fclose(tfp);
    if (ngood != ntidestation)
      {
      error = MB_ERROR_BAD_FORMAT;
      fprintf(stderr,
        "\nNumber of tide station values does not match number of model values <%d != %d>\n",
        ntidestation,
        ngood);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    /* get start end min max of tide station data */
    double tidestation_d_min = 0.0;
    double tidestation_d_max = 0.0;
    double tidestation_m_min = 0.0;
    double tidestation_m_max = 0.0;
    double tidestation_c_min = 0.0;
    double tidestation_c_max = 0.0;
    double tidestation_stime_d;
    double tidestation_etime_d;
    for (int i = 0; i < ntidestation; i++) {
      if (i == 0) {
        tidestation_d_min = tidestation_tide[i];
        tidestation_d_max = tidestation_tide[i];
        tidestation_m_min = tidestation_model[i];
        tidestation_m_max = tidestation_model[i];
        tidestation_c_min = tidestation_correction[i];
        tidestation_c_max = tidestation_correction[i];
        tidestation_stime_d = tidestation_time_d[i];
      } else {
        tidestation_d_min = MIN(tidestation_tide[i], tidestation_d_min);
        tidestation_d_max = MAX(tidestation_tide[i], tidestation_d_max);
        tidestation_m_min = MIN(tidestation_model[i], tidestation_m_min);
        tidestation_m_max = MAX(tidestation_model[i], tidestation_m_max);
        tidestation_c_min = MIN(tidestation_correction[i], tidestation_c_min);
        tidestation_c_max = MAX(tidestation_correction[i], tidestation_c_max);
        tidestation_etime_d = tidestation_time_d[i];
      }
    }
    int tidestation_stime_i[7];
    mb_get_date(verbose, tidestation_stime_d, tidestation_stime_i);
    int tidestation_etime_i[7];
    mb_get_date(verbose, tidestation_etime_d, tidestation_etime_i);

    /* output info on tide station data */
    if (verbose > 0 && mbotps_mode & MBOTPS_MODE_TIDESTATION) {
      fprintf(stderr, "\nTide station data file:             %s\n", tidestation_file);
      fprintf(stderr, "  Tide station longitude:           %f\n", tidestation_lon);
      fprintf(stderr, "  Tide station latitude:            %f\n", tidestation_lat);
      fprintf(stderr, "  Tide station format:              %d\n", tidestation_format);
      fprintf(stderr, "  Tide station data summary:\n");
      fprintf(stderr, "    Number of samples:              %d\n", ntidestation);
      fprintf(stderr,
              "    Start time:                     %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
              tidestation_stime_i[0],
              tidestation_stime_i[1],
              tidestation_stime_i[2],
              tidestation_stime_i[3],
              tidestation_stime_i[4],
              tidestation_stime_i[5],
              tidestation_stime_i[6]);
      fprintf(stderr,
              "    End time:                       %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
              tidestation_etime_i[0],
              tidestation_etime_i[1],
              tidestation_etime_i[2],
              tidestation_etime_i[3],
              tidestation_etime_i[4],
              tidestation_etime_i[5],
              tidestation_etime_i[6]);
      fprintf(stderr, "    Minimum values:     %7.3f %7.3f %7.3f\n",
              tidestation_d_min, tidestation_m_min, tidestation_c_min);
      fprintf(stderr, "    Maximum values:     %7.3f %7.3f %7.3f\n",
              tidestation_d_max, tidestation_m_max, tidestation_c_max);
    }

    /* remove the temporary files */
    unlink(lltfile);
    unlink(otpsfile);
  }

  double file_weight;
  mb_path swath_file;
  mb_path file;
  mb_path dfile;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  void *mbio_ptr = NULL;
  void *store_ptr = NULL;
  int kind;
  double navlon;
  double navlat;
  double speed;
  double heading;
  double distance;
  double altitude;
  double sensordepth;
  char *beamflag = NULL;
  double *bath = NULL;
  double *bathacrosstrack = NULL;
  double *bathalongtrack = NULL;
  double *amp = NULL;
  double *ss = NULL;
  double *ssacrosstrack = NULL;
  double *ssalongtrack = NULL;
  char comment[MB_COMMENT_MAXLINE];

  /* mbotps control parameters */
  double btime_d;
  double etime_d;

  /* tide station data */
  int intstat;
  int itime;
  double correction;

  struct stat file_status;
  bool proceed = true;
  int input_size, input_modtime, output_size, output_modtime;

  /* -------------------------------------------------------------------------
   * calculate tide model  for a single position and time range
   * -----------------------------------------------------------------------*/
  if (!(mbotps_mode & MBOTPS_MODE_NAVIGATION))
    {
    /* Note: because predict_tide is a 1970's style Fortran batch program
          that limits filenames to 80 (!) characters, we put the temporary
          files in the user's home directory, which hopefully leads to
          adequately short pathnames. */

    /* first open temporary file of lat lon time */
    int pid = getpid();
    mb_path wd = "";
    getcwd(wd, sizeof(wd));
    assert(strlen(wd) > 0);
    mb_pathplus lltfile = "";
    mb_pathplus otpsfile = "";
    snprintf(lltfile, sizeof(lltfile), "%s/t%d.txt", getenv("HOME"), pid);
    snprintf(otpsfile, sizeof(otpsfile), "%s/u%d.txt", getenv("HOME"), pid);
    FILE *tfp = NULL;
    if ((tfp = fopen(lltfile, "w")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr,
        "\nUnable to open temporary lat-lon-time file <%s> for writing\n",
        lltfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    /* make sure longitude is positive */
    if (tidelon < 0.0)
      tidelon += 360.0;

    /* loop over the time of interest generating the lat-lon-time values */
    mb_get_time(verbose, btime_i, &btime_d);
    mb_get_time(verbose, etime_i, &etime_d);
    const int ntime = 1 + (int)floor((etime_d - btime_d) / interval);
    for (int i = 0; i < ntime; i++)
      {
      time_d = btime_d + i * interval;
      mb_get_date(verbose, time_d, time_i);
      fprintf(tfp,
        "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d\n",
        tidelat,
        tidelon,
        time_i[0],
        time_i[1],
        time_i[2],
        time_i[3],
        time_i[4],
        time_i[5]);
      }

    /* close the llt file */
    fclose(tfp);

    /* call predict_tide with popen */
    snprintf(predict_tide, sizeof(predict_tide), "cd %s; ./predict_tide", otps_location_use);
    fprintf(stderr, "Running: %s\n", predict_tide);
    if ((tfp = popen(predict_tide, "w")) == NULL) {
      // error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    /* send relevant input to predict_tide through its stdin stream */
    fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
    fprintf(tfp, "%s\n", lltfile);
    fprintf(tfp, "z\n\nAP\noce\n1\n");
    //fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");
    fprintf(tfp, "%s\n", otpsfile);

    /* close the process */
    pclose(tfp);

    /* now read results from predict_tide and rewrite them in a useful form */
    if ((tfp = fopen(otpsfile, "r")) == NULL) {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open predict_time results temporary file <%s>\n",
        otpsfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
    }
    FILE *ofp = NULL;
    if ((ofp = fopen(tide_file, "w")) == NULL) {
      // error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open tide output file <%s>\n", tide_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
    }
    fprintf(ofp, "# Tide model generated by program %s\n", program_name);
    fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
    fprintf(ofp, "# Tide model generated by program %s\n", program_name);
    fprintf(ofp, "# which in turn calls OTPS program predict_tide obtained from:\n");
    fprintf(ofp, "#     http://www.coas.oregonstate.edu/research/po/research/tide/\n");
    fprintf(ofp, "#\n");
    fprintf(ofp, "# OTPSnc tide model: \n");
    fprintf(ofp, "#      %s\n", otps_model);
    if (tideformat == 2) {
      fprintf(ofp, "# Output format:\n");
      fprintf(ofp, "#      year month day hour minute second tide\n");
      fprintf(ofp, "# where tide is in meters\n");
    } else {
      fprintf(ofp, "# Output format:\n");
      fprintf(ofp, "#      time_d tide\n");
      fprintf(ofp, "# where time_d is in seconds since January 1, 1970\n");
      fprintf(ofp, "# and tide is in meters\n");
    }
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
    fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);

    /* loop over tide model values, writing them out in the specified format */
    int nline = 0;
    ngood = 0;
    char *result;
    while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
      nline++;
      if (nline == 2 || nline == 3) {
        fprintf(ofp, "#%s", line);
      } else if (nline > 6) {
        const int nget = sscanf(line,
          "%lf %lf %d.%d.%d %d:%d:%d %lf %lf",
          &lat,
          &lon,
          &time_i[1],
          &time_i[2],
          &time_i[0],
          &time_i[3],
          &time_i[4],
          &time_i[5],
          &tide,
          &depth);
        if (nget == 10) {
          ngood++;

          /* if tide station data have been loaded, interpolate the
           * correction value to apply to the tide model */
          if (mbotps_mode & MBOTPS_MODE_TIDESTATION && (ntidestation > 0))
            {
            intstat = mb_linear_interp(verbose,
              tidestation_time_d - 1,
              tidestation_correction - 1,
              ntidestation,
              time_d,
              &correction,
              &itime,
              &error);
            if (intstat == MB_SUCCESS)
              tide += correction;
            }

          /* write out the tide model */
          if (tideformat == 2)
            {
            fprintf(ofp,
              "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n",
              time_i[0],
              time_i[1],
              time_i[2],
              time_i[3],
              time_i[4],
              time_i[5],
              tide);
            }
          else
            {
            mb_get_time(verbose, time_i, &time_d);
            fprintf(ofp, "%.3f %9.4f\n", time_d, tide);
            }
        }
      }
    }
    fclose(tfp);
    fclose(ofp);

    /* remove the temporary files */
    unlink(lltfile);
    unlink(otpsfile);

    /* some helpful output */
    fprintf(stderr, "\nResults are really in %s\n", tide_file);
    }  /* end single position mode */

  /* -------------------------------------------------------------------------
   * else get tides along the navigation contained in a set of swath files
   * - accumulate all of the desired nav and time points for all files and
   *   call predict_tide just once, then break the results up into individual
   *   *.tde tide files for each swath file.
   * -----------------------------------------------------------------------*/
  else if (mbotps_mode & MBOTPS_MODE_NAVIGATION)
    {
    fprintf(stderr, "\nModel tide for swath data referenced by %s\n", read_file);
    if (mbotps_mode & MBOTPS_MODE_TIDESTATION && (ntidestation > 0)) {
      fprintf(stderr, " - Also apply tide station correction\n");
    }
    if (mbprocess_update) {
      fprintf(stderr, " - Set mbprocess parameter files to apply tide correction\n");
      }
    fprintf(stderr, "\n");

    /*fprintf(stderr,"Doing tide correction for swath navigation\n"); */
    /* get format if required */
    if (format == 0)
      mb_get_format(verbose, read_file, NULL, &format, &error);

    /* determine whether to read one file or a list of files */
    const bool read_datalist = format < 0;
    void *datalist;
    bool read_data;

    /* open file list */
    if (read_datalist) {
      const int look_processed = MB_DATALIST_LOOK_UNSET;
      status = mb_datalist_open(verbose, &datalist, read_file, look_processed, &error);
      if (status != MB_SUCCESS) {
        fprintf(stderr, "\nUnable to open data list file: %s\n", read_file);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(MB_ERROR_OPEN_FAIL);
      }
      if ((status =
        mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight,
          &error)) == MB_SUCCESS)
        read_data = true;
      else
        read_data = false;
      }
    /* else copy single filename to be read */
    else
      {
      strcpy(file, read_file);
      read_data = true;
      }

    /* Note: because predict_tide is a 1970's style Fortran batch program
          that limits filenames to 80 (!) characters, we put the temporary
          files in the user's home directory, which hopefully leads to
          adequately short pathnames. */

    /* first open temporary file of lat lon time */
    int pid = getpid();
    mb_path wd = "";
    getcwd(wd, sizeof(wd));
    mb_pathplus lltfile = "";
    mb_pathplus otpsfile = "";
    snprintf(lltfile, sizeof(lltfile), "%s/t%d.txt", getenv("HOME"), pid);
    snprintf(otpsfile, sizeof(otpsfile), "%s/u%d.txt", getenv("HOME"), pid);
    FILE *tfp = NULL;
    if ((tfp = fopen(lltfile, "w")) == NULL)
      {
      fprintf(stderr,
        "\nUnable to open temporary lat-lon-time file <%s> for writing\n",
        lltfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    double savetime_d;
    double lasttime_d;
    double lastlon;
    double lastlat;
    /* loop over all files to be read */
    while (read_data) {
      /* Figure out if the file needs a tide model - don't generate a new tide
         model if one was made previously and is up to date AND the
         appropriate request has been made */
      proceed = true;
      mb_pathplus tides_file = "";
      snprintf(tides_file, sizeof(tides_file), "%s.tde", file);
      if (skip_existing) {
        const int fstat1 = stat(file, &file_status);
        if (fstat1 == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          input_modtime = file_status.st_mtime;
          input_size = file_status.st_size;
        } else {
          input_modtime = 0;
          input_size = 0;
        }
        const int fstat2 = stat(tide_file, &file_status);
        if (fstat2 == 0 && (file_status.st_mode & S_IFMT) != S_IFDIR) {
          output_modtime = file_status.st_mtime;
          output_size = file_status.st_size;
        } else {
          output_modtime = 0;
          output_size = 0;
        }
        if (output_modtime > input_modtime && input_size > 0 && output_size > 0)
          proceed = false;
        }

      /* skip the file */
      if (!proceed) {
        /* some helpful output */
        fprintf(stderr, "%s : skipped - tide model file is up to date\n", swath_file);
      }

      /* add nav points to tidal model list */
      else {

        /* read fnv file if possible */
        strcpy(swath_file, file);
        mb_get_fnv(verbose, file, &format, &error);

        /* initialize reading the swath file */
        if ((status =
          mb_read_init(verbose, file, format, pings, lonflip, bounds, btime_i, etime_i,
            speedmin, timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp,
            &pixels_ss, &error)) !=MB_SUCCESS) {
          char *message;
          mb_error(verbose, error, &message);
          fprintf(stderr,
            "\nMBIO Error returned from function <mb_read_init>:\n%s\n",
            message);
          fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", file);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }

        /* allocate memory for data arrays */
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_BATHYMETRY,
            sizeof(char),
            (void **)&beamflag,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
            mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_BATHYMETRY,
            sizeof(double),
            (void **)&bath,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_AMPLITUDE,
            sizeof(double),
            (void **)&amp,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_BATHYMETRY,
            sizeof(double),
            (void **)&bathacrosstrack,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_BATHYMETRY,
            sizeof(double),
            (void **)&bathalongtrack,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_SIDESCAN,
            sizeof(double),
            (void **)&ss,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_SIDESCAN,
            sizeof(double),
            (void **)&ssacrosstrack,
            &error);
        }
        if (error == MB_ERROR_NO_ERROR) {
          // status =
          mb_register_array(verbose,
            mbio_ptr,
            MB_MEM_TYPE_SIDESCAN,
            sizeof(double),
            (void **)&ssalongtrack,
            &error);
        }

        /* if error initializing memory then quit */
        if (error != MB_ERROR_NO_ERROR) {
          char *message;
          mb_error(verbose, error, &message);
          fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }

        /* read and use data */
        int nread = 0;
        int nuse = 0;
        while (error <= MB_ERROR_NO_ERROR) {
          /* reset error */
          error = MB_ERROR_NO_ERROR;
          bool output = false;

          /* read next data record */
          status = mb_get_all(verbose,
            mbio_ptr,
            &store_ptr,
            &kind,
            time_i,
            &time_d,
            &navlon,
            &navlat,
            &speed,
            &heading,
            &distance,
            &altitude,
            &sensordepth,
            &beams_bath,
            &beams_amp,
            &pixels_ss,
            beamflag,
            bath,
            amp,
            bathacrosstrack,
            bathalongtrack,
            ss,
            ssacrosstrack,
            ssalongtrack,
            comment,
            &error);

          /* print debug statements */
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  Ping read in program <%s>\n", program_name);
            fprintf(stderr, "dbg2       kind:           %d\n", kind);
            fprintf(stderr, "dbg2       error:          %d\n", error);
            fprintf(stderr, "dbg2       status:         %d\n", status);
            }

          /* deal with nav and time from survey data only - not nav, sidescan, or
             subbottom */
          if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
            /* flag positions and times for output at specified interval */
            if (nread == 0 || time_d - savetime_d >= interval) {
              savetime_d = time_d;
              output = true;
            }
            lasttime_d = time_d;
            lastlon = navlon;
            lastlat = navlat;

            /* increment counter */
            nread++;
          }

          /* output position and time if flagged or end of file */
          if (output || error == MB_ERROR_EOF) {
            if (lastlon < 0.0)
              lastlon += 360.0;
            mb_get_date(verbose, lasttime_d, time_i);
            fprintf(tfp,
              "%.6f %.6f %4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %s\n",
              lastlat,
              lastlon,
              time_i[0],
              time_i[1],
              time_i[2],
              time_i[3],
              time_i[4],
              time_i[5],
              swath_file);
            nuse++;
            }
          }

        /* close the swath file */
        status = mb_close(verbose, &mbio_ptr, &error);

        /* output read statistics */
        fprintf(stderr, "%s : model tide at %d of %d records\n", file, nuse, nread);
        }

      /* figure out whether and what to read next */
      if (read_datalist) {
        if ((status =
          mb_datalist_read(verbose, datalist, file, dfile, &format, &file_weight,
            &error)) == MB_SUCCESS)
          read_data = true;
        else
          read_data = false;
      } else {
        read_data = false;
      }

      /* end loop over files in list */
      }
    if (read_datalist)
      mb_datalist_close(verbose, &datalist, &error);

    /* close the llt file */
    fclose(tfp);

    /* call predict_tide with popen */
    fprintf(stderr, "\nCalling OTPS predict_tide:\n");
    fprintf(stderr, "  %s/predict_tide\n", otps_location_use);
    fprintf(stderr, "  %s/DATA/Model_%s\n", otps_location_use, otps_model);
    fprintf(stderr, "  Input llt file:   %s\n", lltfile);
    fprintf(stderr, "  Output otps file: %s\n", otpsfile);
    fprintf(stderr, "---------------------------------------\n");
    snprintf(predict_tide, sizeof(predict_tide), "cd %s; ./predict_tide", otps_location_use);
    tfp =  popen(predict_tide, "w");
    if (tfp == NULL) {
      // error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr, "\nUnable to open predict_time program using popen()\n");
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
    }

    /* send relevant input to predict_tide through its stdin stream */
    fprintf(tfp, "%s/DATA/Model_%s\n", otps_location_use, otps_model);
    fprintf(tfp, "%s\n", lltfile);
    fprintf(tfp, "z\n\nAP\noce\n1\n");
    //fprintf(tfp, "z\nm2,s2,n2,k2,k1,o1,p1,q1\nAP\noce\n1\n");
    fprintf(tfp, "%s\n", otpsfile);

    /* close the process */
    pclose(tfp);

    fprintf(stderr, "---------------------------------------\n\n");

    /* now read results from predict_tide, correlate with the llt file,
       and output a tide file for each swath file */
    if ((tfp = fopen(otpsfile, "r")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr,
        "\nUnable to open predict_time results temporary file <%s>\n",
        otpsfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }
    FILE *lfp = NULL;
    if ((lfp = fopen(lltfile, "r")) == NULL)
      {
      error = MB_ERROR_OPEN_FAIL;
      fprintf(stderr,
        "\nUnable to reopen llt temporary file <%s>\n",
        lltfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_FAILURE);
      }

    /* read through predict_tide output header */
    mb_path tline2 = "";
    mb_path tline3 = "";

    {
    int nline = 0;
      char *result;
      while (nline < 7 && (result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
        nline++;
        if (nline == 2)
          strncpy(tline2, line, sizeof(tline2));
        if (nline == 3)
          strncpy(tline3, line, sizeof(tline3));
      }
    }

    /* loop over tide model values, writing them out in the specified format
       to tide files associated with each swath file */
    // nline = 0;
    ngood = 0;
    FILE *ofp= NULL;
    mb_path current_swath_file = "";
    char *result;
    while ((result = fgets(line, MB_PATH_MAXLINE, tfp)) == line) {
      bool output_ok = true;

      /* get the next tide value */
      int nget = sscanf(line, "%lf %lf %d.%d.%d %d:%d:%d %lf %lf",
          &lat, &lon,
          &time_i[1], &time_i[2], &time_i[0], &time_i[3], &time_i[4], &time_i[5],
          &tide, &depth);
      if (nget == 10) {
        ngood++;
      } else {
        output_ok = false;
        if (strstr(line, "***** Site is out of model grid OR land *****") != NULL) {
          fprintf(stderr, "Skipping data: position %f %f is outside the model grid or located on land\n",
                  lon, lat);
        }
      }

      /* if tide station data have been loaded, interpolate the
       * correction value to apply to the tide model */
      if (output_ok && mbotps_mode & MBOTPS_MODE_TIDESTATION && (ntidestation > 0)) {
        intstat = mb_linear_interp(verbose,
                                    tidestation_time_d - 1,
                                    tidestation_correction - 1,
                                    ntidestation,
                                    time_d,
                                    &correction,
                                    &itime,
                                    &error);
        if (intstat == MB_SUCCESS)
          tide += correction;
      }

      /* get next entry from the llt file and check the associated swath_file
          if needed open new output tide file */
      if ((result = fgets(line, MB_PATH_MAXLINE, lfp)) == line) {
        /* get the corresponding swath file */
        nget = sscanf(line, "%lf %lf %d %d %d %d %d %d %s",
                      &lat, &lon,
                      &time_i[0], &time_i[1], &time_i[2],
                      &time_i[3], &time_i[4], &time_i[5],
                      swath_file);
        if (nget != 9) {
          output_ok = false;
        }
      }

      /* make sure an output tide file is open */
      if (output_ok) {
        if (strcmp(current_swath_file, swath_file) != 0) {
          strncpy(current_swath_file, swath_file, sizeof(current_swath_file));
          if (ofp != NULL) {
            fclose(ofp);
          }

          mb_pathplus tides_file = "";
          snprintf(tides_file, sizeof(tides_file), "%s.tde", swath_file);
          fprintf(stderr, "Generating tide file %s\n", tides_file);
          if ((ofp = fopen(tides_file, "w")) == NULL) {
            //error = MB_ERROR_OPEN_FAIL;
            fprintf(stderr, "\nUnable to open tide output file <%s>\n", tides_file);
            fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
            exit(MB_FAILURE);
          }
          fprintf(ofp, "# Tide model generated by program %s\n", program_name);
          fprintf(ofp, "# MB-System Version: %s\n", MB_VERSION);
          fprintf(ofp, "# Tide model generated by program %s\n", program_name);
          fprintf(ofp, "# which in turn calls OTPS program predict_tide obtained from:\n");
          fprintf(ofp, "#     http://www.coas.oregonstate.edu/research/po/research/tide/\n");
          char user[256], host[256], date[32];
          status = mb_user_host_date(verbose, user, host, date, &error);
          fprintf(ofp, "# Run by user <%s> on cpu <%s> at <%s>\n", user, host, date);
          fprintf(ofp, "#%s", tline2);
          fprintf(ofp, "#%s", tline3);

          /* set mbprocess usage of tide file */
          if (mbprocess_update) {
            status = mb_pr_update_tide(verbose,
              swath_file,
              MBP_TIDE_ON,
              tides_file,
              tideformat,
              &error);
          }
        }
      }

      /* write out the tide model to swath file tide file */
      if (output_ok) {
        if (tideformat == 2) {
          fprintf(ofp,
            "%4.4d %2.2d %2.2d %2.2d %2.2d %2.2d %9.4f\n",
            time_i[0],
            time_i[1],
            time_i[2],
            time_i[3],
            time_i[4],
            time_i[5],
            tide);
        } else {
          mb_get_time(verbose, time_i, &time_d);
          fprintf(ofp, "%.3f %9.4f\n", time_d, tide);
        }
      }
    } // end loop reading tide values output by predict_tide

    fclose(ofp);
    fclose(lfp);
    fclose(tfp);

    /* remove the temporary files */
    unlink(lltfile);
    unlink(otpsfile);
  }

  /* check memory */
  if (verbose >= 4)
    status = mb_memory_list(verbose, &error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s> completed\n", program_name);
    fprintf(stderr, "dbg2  Ending status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return(error);
}
/*--------------------------------------------------------------------*/
