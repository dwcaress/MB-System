/*--------------------------------------------------------------------
 *    The MB-system:  mbclean.c  2/26/93
 *
 *    Copyright (c) 1993-2025 by
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
 * mbclean identifies and flags artifacts in swath sonar bathymetry data.
 * The edit events are output to an edit save file which can be applied
 * to the data by the program mbprocess.
 * Several algorithms are available for identifying artifacts; multiple
 * algorithmscan be applied in a single pass.  The most commonly used
 * approach is to identify artifacts  based  on  excessive  bathymetric
 * slopes.  If desired, mbclean will also flag beams associated with
 * "rails" where outer beams have  smaller  acrosstrack distances than
 * more inner beams (-Q option).  Low and high bounds on acceptable depth
 * values can be set; depth values outside  the  acceptable  range  will  be
 * flagged.  The acceptable depth ranges can either be absolute (-B option), relative
 * to the local median depth (-A option) or defined by low and high fractions
 * of the local median depth (-G option).  A set number of outer beams can also be
 * flagged.

 * The order in which the flagging algorithms are applied is as follows:
 *      1. Flag specified number of outer beams (-X option).
 *      2. Flag soundings outside specified acceptable
 *         depth range (-B option).
 *      3. Flag soundings outside acceptable depth range using
 *         fractions of local median depth (-G option).
 *      4. Flag soundings outside acceptable depth range using
 *         deviation from local median depth (-A option).
 *      5. Flag soundings associated with excessive slopes
 *         (-C option or default).
 *      6. Zap "rails" (-Q option).
 *      7. Flag all soundings in pings with too few
 *         good soundings (-U option).
 *
 *
 * Author:  D. W. Caress
 * Date:  February 26, 1993 (buffered i/o version)
 * Date:  January 19, 2001 (edit save file version)
 *
 * Acknowledgments:
 * This program is based to a large extent on the program mbcleanx
 * by Alberto Malinverno (formerly at L-DEO, then at Schlumberger,
 * then back at L-DEO), which was in turn based on the original
 * program mbclean (v. 1.0) by David Caress.
 */

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"

typedef enum {
    MBCLEAN_FLAG_ONE = 1,
    MBCLEAN_FLAG_BOTH = 2,
} clean_mode_t;

typedef enum {
    MBCLEAN_Y_MODE_DISTANCE_FLAG = 1,
    MBCLEAN_Y_MODE_DISTANCE_UNFLAG = 2,
    MBCLEAN_Y_MODE_ANGLE_FLAG = 3,
    MBCLEAN_Y_MODE_ANGLE_UNFLAG = 4,
} y_mode_t;

/* MBIO buffer size default */
constexpr int MBCLEAN_BUFFER_DEFAULT = 500;

/* edit action defines */
constexpr int MBCLEAN_NOACTION = 0;

/* ping structure definition */
struct mbclean_ping_struct {
  int time_i[7];
  double time_d;
  int multiplicity;
  double navlon;
  double navlat;
  double speed;
  double heading;
  int beams_bath;
  char *beamflag;
  char *beamflagorg;
  double *bath;
  double *bathacrosstrack;
  double *bathalongtrack;
  double *bathx;
  double *bathy;
};

/* bad beam identifier structure definition */
struct bad_struct {
  bool flag;
  int ping;
  int beam;
  double bath;
};

constexpr char program_name[] = "mbclean";
constexpr char help_message[] =
    "Mbclean identifies and flags artifacts in swath sonar bathymetry data.\n"
    "Several algorithms are available for identifying artifacts;\n"
    "multiple algorithms can be applied in a single pass.\n";
constexpr char usage_message[] =
    "mbclean [-Amax -Blow/high -Cslope/unit -Dmin/max\n"
    "\t-Fformat -Gfraction_low/fraction_high -Iinfile -Krange_min\n"
    "\t-Llonflip -Mmode Ntolerance -Ooutfile -Pmin_speed/max_speed -Q -Rmaxheadingrate\n"
    "\t-Sspike_slope/mode/format -Ttolerance -Wwest/east/south/north \n"
    "\t-Xbeamsleft/beamsright -Ydistanceleft/distanceright[/mode] -Z\n\t-V -H]\n\n";

/*--------------------------------------------------------------------*/
/* edit output function */
int mbclean_save_edit(int verbose, FILE *sofp, double time_d, int beam, int action, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       sofp:            %p\n", (void *)sofp);
    fprintf(stderr, "dbg2       time_d:          %f\n", time_d);
    fprintf(stderr, "dbg2       beam:            %d\n", beam);
    fprintf(stderr, "dbg2       action:          %d\n", action);
  }

  fprintf(stderr, "OUTPUT EDIT: %f %d %d\n", time_d, beam, action);

  int status = MB_SUCCESS;
  if (sofp != nullptr) {
#ifdef BYTESWAPPED
    mb_swap_double(&time_d);
    beam = mb_swap_int(beam);
    action = mb_swap_int(action);
#endif
    if (fwrite(&time_d, sizeof(double), 1, sofp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
    if (status == MB_SUCCESS && fwrite(&beam, sizeof(int), 1, sofp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
    if (status == MB_SUCCESS && fwrite(&action, sizeof(int), 1, sofp) != 1) {
      status = MB_FAILURE;
      *error = MB_ERROR_WRITE_FAIL;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:       %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int main(int argc, char **argv) {
  int status;
  int verbose = 0;
  int format;
  int pings;
  int lonflip;
  double bounds[4];
  int btime_i[7];
  int etime_i[7];
  double speedmin;
  double timegap;
  status = mb_defaults(verbose, &format, &pings, &lonflip, bounds, btime_i, etime_i, &speedmin, &timegap);
  bool uselockfiles;
  mb_uselockfiles(verbose, &uselockfiles);

  /* reset all defaults but the format and lonflip */
  pings = 1;
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

  y_mode_t optiony_mode;

  double deviation_max;
  double depth_low;
  double depth_high;
  int slope_form = 0;
  double slopemax = 1.0;
  double distancemin = 0.01;
  double distancemax = 0.25;
  double max_acrosstrack = 120.0;
  bool check_deviation = false;
  bool check_range = false;
  bool check_slope = false;
  bool zap_long_across = false;
  double fraction_low;
  double fraction_high;
  bool check_fraction = false;
  double range_min;
  bool check_range_min = false;
  clean_mode_t mode = MBCLEAN_FLAG_ONE;
  double ping_deviation_tolerance = 1.0;
  bool check_ping_deviation = false;
  double speed_low;
  double speed_high;
  bool check_speed_good = false;
  bool zap_rails = false;
  double backup_dist = 0;
  bool zap_max_heading_rate = false;
  double max_heading_rate;
  double spikemax = 1.0;
  int spike_mode = 1;
  bool check_spike = false;
  /* fix_edit_timestamps variables */
  bool fix_edit_timestamps = false;
  double tolerance = 0.0;
  bool check_num_good_min = false;
  int num_good_min;
  bool check_position_bounds = false;
  double west;
  double east;
  double south;
  double north;
  bool zap_beams = false;
  int zap_beams_right = 0;
  int zap_beams_left = 0;
  double left;
  double right;
  bool flag_distance = false;
  double flag_distance_right = 0.0;
  double flag_distance_left = 0.0;
  bool unflag_distance = false;
  double unflag_distance_right = 0.0;
  double unflag_distance_left = 0.0;
  double flag_angle_right = 0.0;
  double flag_angle_left = 0.0;
  bool flag_angle = false;
  bool unflag_angle = false;
  double unflag_angle_right = 0.0;
  double unflag_angle_left = 0.0;
  bool check_zero_position = false;
  char read_file[MB_PATH_MAXLINE] = "datalist.mb-1";

  {
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt(argc, argv, "VvHhA:a:B:b:C:c:D:d:E:e:F:f:G:g:K:k:L:l:I:i:M:m:N:n:Q:q:P:p:R:r:S:s:T:t:U:u:W:w:X:x:Y:y:Zz")) !=
           -1) {
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
        sscanf(optarg, "%lf", &deviation_max);
        check_deviation = true;
        break;
      case 'B':
      case 'b':
        sscanf(optarg, "%lf/%lf", &depth_low, &depth_high);
        check_range = true;
        break;
      case 'C':
      case 'c':
        slope_form = 0;
        sscanf(optarg, "%lf/%d", &slopemax, &slope_form);
        check_slope = true;
        if (slope_form == 1)
          slopemax = tan(slopemax);
        else if (slope_form == 2)
          slopemax = tan(DTR * slopemax);
        break;
      case 'D':
      case 'd':
        sscanf(optarg, "%lf/%lf", &distancemin, &distancemax);
        break;
      case 'E':
      case 'e':
        sscanf(optarg, "%lf", &max_acrosstrack);
        zap_long_across = true;
        break;
      case 'F':
      case 'f':
        sscanf(optarg, "%d", &format);
        break;
      case 'G':
      case 'g':
        sscanf(optarg, "%lf/%lf", &fraction_low, &fraction_high);
        check_fraction = true;
        break;
      case 'K':
      case 'k':
        sscanf(optarg, "%lf", &range_min);
        check_range_min = true;
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", read_file);
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
        break;
      case 'M':
      case 'm':
      {
        int tmp;
        sscanf(optarg, "%d", &tmp);
        mode = (clean_mode_t)tmp;  // TODO(schwehr): Range check.
        break;
      }
      case 'N':
      case 'n':
        sscanf(optarg, "%lf", &ping_deviation_tolerance);
        check_ping_deviation = true;
        break;
      case 'P':
      case 'p':
        sscanf(optarg, "%lf/%lf", &speed_low, &speed_high);
        check_speed_good = true;
        break;
      case 'Q':
      case 'q':
        zap_rails = true;
        backup_dist = 0.0;
        sscanf(optarg, "%lf", &backup_dist);
        break;
      case 'R':
      case 'r':
        zap_max_heading_rate = true;
        sscanf(optarg, "%lf", &max_heading_rate);
        break;
      case 'S':
      case 's':
        slope_form = 0;
        sscanf(optarg, "%lf/%d/%d", &spikemax, &spike_mode, &slope_form);
        check_spike = true;
        if (2 == slope_form)
          spikemax = tan(DTR * spikemax);
        if (1 == slope_form)
          spikemax = tan(spikemax);
        break;
      case 'T':
      case 't':
        fix_edit_timestamps = true;
        sscanf(optarg, "%lf", &tolerance);
        break;
      case 'U':
      case 'u':
        sscanf(optarg, "%d", &num_good_min);
        check_num_good_min = true;
        break;
      case 'W':
      case 'w':
        check_position_bounds = true;
        sscanf(optarg, "%lf/%lf/%lf/%lf", &west, &east, &south, &north);
        break;
      case 'X':
      case 'x':
      {
        const int n = sscanf(optarg, "%d/%d", &zap_beams_left, &zap_beams_right);
        if (n == 1)
          zap_beams_right = zap_beams_left;
        zap_beams = true;
        break;
      }
      case 'Y':
      case 'y':
      {
        int optiony_mode_tmp;
        const int n = sscanf(optarg, "%lf/%lf/%d", &left, &right, &optiony_mode_tmp);
        if (n == 1) {
          flag_distance_left = -fabs(left);
          flag_distance_right = fabs(left);
          flag_distance = true;
        }
        else if (n == 2) {
          flag_distance_left = left;
          flag_distance_right = right;
          flag_distance = true;
        }
        else if (n == 3) {
          if (optiony_mode_tmp >= 1 && optiony_mode_tmp <= 4)
            optiony_mode = (y_mode_t)optiony_mode_tmp;
          else
            optiony_mode = MBCLEAN_Y_MODE_DISTANCE_FLAG;
          if (optiony_mode == MBCLEAN_Y_MODE_DISTANCE_FLAG) {
            flag_distance_left = left;
            flag_distance_right = right;
            flag_distance = true;
          }
          else if (optiony_mode == MBCLEAN_Y_MODE_DISTANCE_UNFLAG) {
            unflag_distance_left = left;
            unflag_distance_right = right;
            unflag_distance = true;
          }
          else if (optiony_mode == MBCLEAN_Y_MODE_ANGLE_FLAG) {
            flag_angle_left = left;
            flag_angle_right = right;
            flag_angle = true;
          }
          else if (optiony_mode == MBCLEAN_Y_MODE_ANGLE_UNFLAG) {
            unflag_angle_left = left;
            unflag_angle_right = right;
            unflag_angle = true;
          }
        }
        break;
      }
      case 'Z':
      case 'z':
        check_zero_position = true;
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

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  /* turn on slope checking if nothing else is to be used */
  if (!check_slope && !zap_beams && !flag_distance && !unflag_distance && !zap_rails &&
      !check_spike && !check_range && !check_fraction && !check_speed_good &&
      !check_deviation && !check_num_good_min && !check_position_bounds &&
      !check_zero_position && !fix_edit_timestamps &&
      !zap_max_heading_rate)
    check_slope = true;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Program <%s>\n", program_name);
    fprintf(stderr, "dbg2  MB-system Version %s\n", MB_VERSION);
    fprintf(stderr, "dbg2  Control Parameters:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       pings:                %d\n", pings);
    fprintf(stderr, "dbg2       lonflip:              %d\n", lonflip);
    fprintf(stderr, "dbg2       bounds[0]:            %f\n", bounds[0]);
    fprintf(stderr, "dbg2       bounds[1]:            %f\n", bounds[1]);
    fprintf(stderr, "dbg2       bounds[2]:            %f\n", bounds[2]);
    fprintf(stderr, "dbg2       bounds[3]:            %f\n", bounds[3]);
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
    fprintf(stderr, "dbg2       speedmin:             %f\n", speedmin);
    fprintf(stderr, "dbg2       timegap:              %f\n", timegap);
    fprintf(stderr, "dbg2       data format:          %d\n", format);
    fprintf(stderr, "dbg2       input file:           %s\n", read_file);
    fprintf(stderr, "dbg2       mode:                 %d\n", mode);
    fprintf(stderr, "dbg2       zap_beams:            %d\n", zap_beams);
    fprintf(stderr, "dbg2       zap_beams_left:       %d\n", zap_beams_left);
    fprintf(stderr, "dbg2       zap_beams_right:      %d\n", zap_beams_right);
    fprintf(stderr, "dbg2       flag_distance:        %d\n", flag_distance);
    fprintf(stderr, "dbg2       flag_distance_left:   %f\n", flag_distance_left);
    fprintf(stderr, "dbg2       flag_distance_right:  %f\n", flag_distance_right);
    fprintf(stderr, "dbg2       unflag_distance:      %d\n", unflag_distance);
    fprintf(stderr, "dbg2       unflag_distance_left: %f\n", unflag_distance_left);
    fprintf(stderr, "dbg2       unflag_distance_right:%f\n", unflag_distance_right);
    fprintf(stderr, "dbg2       flag_angle:           %d\n", flag_angle);
    fprintf(stderr, "dbg2       flag_angle_left:      %f\n", flag_angle_left);
    fprintf(stderr, "dbg2       flag_angle_right:     %f\n", flag_angle_right);
    fprintf(stderr, "dbg2       unflag_angle:         %d\n", unflag_angle);
    fprintf(stderr, "dbg2       unflag_angle_left:    %f\n", unflag_angle_left);
    fprintf(stderr, "dbg2       unflag_angle_right:   %f\n", unflag_angle_right);
    fprintf(stderr, "dbg2       zap_rails:            %d\n", zap_rails);
    fprintf(stderr, "dbg2       backup_dist:          %f\n", backup_dist);
    fprintf(stderr, "dbg2       zap_max_heading_rate: %d\n", zap_max_heading_rate);
    fprintf(stderr, "dbg2       max_heading_rate:     %f\n", max_heading_rate);
    fprintf(stderr, "dbg2       check_slope:          %d\n", check_slope);
    fprintf(stderr, "dbg2       maximum slope:        %f\n", slopemax);
    fprintf(stderr, "dbg2       check_spike:          %d\n", check_spike);
    fprintf(stderr, "dbg2       maximum spike:        %f\n", spikemax);
    fprintf(stderr, "dbg2       spike mode:           %d\n", spike_mode);
    fprintf(stderr, "dbg2       minimum dist:         %f\n", distancemin);
    fprintf(stderr, "dbg2       minimum dist:         %f\n", distancemax);
    fprintf(stderr, "dbg2       check_range:          %d\n", check_range);
    fprintf(stderr, "dbg2       depth_low:            %f\n", depth_low);
    fprintf(stderr, "dbg2       depth_high:           %f\n", depth_high);
    fprintf(stderr, "dbg2       check_fraction:       %d\n", check_fraction);
    fprintf(stderr, "dbg2       fraction_low:         %f\n", fraction_low);
    fprintf(stderr, "dbg2       fraction_high:        %f\n", fraction_high);
    fprintf(stderr, "dbg2       check_deviation:      %d\n", check_deviation);
    fprintf(stderr, "dbg2       check_num_good_min:   %d\n", check_num_good_min);
    fprintf(stderr, "dbg2       num_good_min:         %d\n", num_good_min);
    fprintf(stderr, "dbg2       zap_long_across:      %d\n", zap_long_across);
    fprintf(stderr, "dbg2       max_acrosstrack:      %f\n", max_acrosstrack);
    fprintf(stderr, "dbg2       fix_edit_timestamps:  %d\n", fix_edit_timestamps);
    fprintf(stderr, "dbg2       tolerance:            %f\n", tolerance);
    fprintf(stderr, "dbg2       check_speed_good:     %d\n", check_speed_good);
    fprintf(stderr, "dbg2       speed_low:            %f\n", speed_low);
    fprintf(stderr, "dbg2       speed_high:           %f\n", speed_high);
    fprintf(stderr, "dbg2       check_position_bounds:%d\n", check_position_bounds);
    fprintf(stderr, "dbg2       check_zero_position:  %d\n", check_zero_position);
    fprintf(stderr, "dbg2       check_ping_deviation: %d\n", check_ping_deviation);
    fprintf(stderr, "dbg2       ping_deviation_tolerance:  %f\n", ping_deviation_tolerance);
  } else if (verbose == 1) {
    fprintf(stderr, "\nProgram %s\n", program_name);
    fprintf(stderr, "MB-system Version %s\n", MB_VERSION);
  }

  int error = MB_ERROR_NO_ERROR;

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
    const int look_processed = MB_DATALIST_LOOK_UNSET;
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

  double btime_d;
  double etime_d;

  /* swath file locking variables */
  bool locked = false;
  int lock_purpose = MBP_LOCK_NONE;
  mb_path lock_program;
  mb_path lock_cpu;
  mb_path lock_user;
  char lock_date[25];

  /* MBIO read control parameters */
  char swathfileread[MB_PATH_MAXLINE];
  int formatread;
  int variable_beams;
  int traveltime;
  double distance;
  double altitude;
  double sensordepth;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* mbio read and write values */
  void *mbio_ptr = nullptr;
  void *store_ptr = nullptr;
  int kind;
  struct mbclean_ping_struct ping[3];
  int pingsread;
  int nfiletot = 0;
  int ndatatot = 0;
  int ndepthrangetot = 0;
  int nminrangetot = 0;
  int nfractiontot = 0;
  int ndeviationtot = 0;
  int nouterbeamstot = 0;
  int nouterdistancetot = 0;
  int ninnerdistancetot = 0;
  int nouterangletot = 0;
  int ninnerangletot = 0;
  int nrailtot = 0;
  int nlong_acrosstot = 0;
  int nmintot = 0;
  int nbadtot = 0;
  int nspiketot = 0;
  int npingdeviationtot = 0;
  int nflagtot = 0;
  int nunflagtot = 0;
  int nflagesftot = 0;
  int nunflagesftot = 0;
  int nzeroesftot = 0;
  int ninnerdistance = 0;
  int nouterangle = 0;
  int ninnerangle = 0;
  int nmax_heading_rate = 0;
  int nmax_heading_ratetot = 0;
  int npingdeviation = 0;
  char comment[MB_COMMENT_MAXLINE];
  int num_good;
  int action;
  double dev;
  double ping_deviation;

  /* rail processing variables */
  int center;
  double lowdist;
  double highdist;

  /* slope processing variables */
  double mtodeglon;
  double mtodeglat;
  int nlist;
  double median = 0.0;

  /* save file control variables */
  char esffile[MB_PATH_MAXLINE];
  struct mb_esf_struct esf;

  /* processing variables */
  int sensorhead = 0;
  int sensorhead_error = MB_ERROR_NO_ERROR;

  int beam_flagging;  // TODO(schwehr): make mb_format_flags take a bool.

  /* loop over all files to be read */
  while (read_data) {
    bool oktoprocess = true;

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

    /* try to lock file */
    if (uselockfiles)
      status = mb_pr_lockswathfile(verbose, swathfile, MBP_LOCK_EDITBATHY, program_name, &error);
    else {
      // lock_status =
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
        // lock_status =
        mb_pr_lockinfo(verbose, swathfile, &locked, &lock_purpose, lock_program, lock_user, lock_cpu, lock_date, &error);

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
      /* check for "fast bathymetry" or "fbt" file */
      strcpy(swathfileread, swathfile);
      formatread = format;
      mb_get_fbt(verbose, swathfileread, &formatread, &error);

      /* initialize reading the input swath sonar file */
      if (mb_read_init(verbose, swathfileread, formatread, pings, lonflip, bounds, btime_i, etime_i, speedmin,
                                 timegap, &mbio_ptr, &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) !=
          MB_SUCCESS) {
        char *message = nullptr;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", swathfile);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* initialize and increment counting variables */
      int ndata = 0;
      int ndepthrange = 0;
      int nminrange = 0;
      int nfraction = 0;
      int nspeed = 0;
      int nzeropos = 0;
      int nrangepos = 0;
      int ndeviation = 0;
      int nouterbeams = 0;
      int nouterdistance = 0;
      int nrail = 0;
      int nlong_across = 0;
      int nmin = 0;
      int nbad = 0;
      int nspike = 0;
      int nflag = 0;
      int nunflag = 0;
      int nflagesf = 0;
      int nunflagesf = 0;
      int nzeroesf = 0;

      /* give the statistics */
      if (verbose >= 0) {
        fprintf(stderr, "\nProcessing %s\n", swathfileread);
      }

      /* allocate memory for data arrays */
      for (int i = 0; i < 3; i++) {
        ping[i].beamflag = nullptr;
        ping[i].beamflagorg = nullptr;
        ping[i].bath = nullptr;
        ping[i].bathacrosstrack = nullptr;
        ping[i].bathalongtrack = nullptr;
        ping[i].bathx = nullptr;
        ping[i].bathy = nullptr;
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                     (void **)&ping[i].beamflag, &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char),
                                     (void **)&ping[i].beamflagorg, &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bath,
                                     &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                     (void **)&ping[i].bathacrosstrack, &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double),
                                     (void **)&ping[i].bathalongtrack, &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bathx,
                                     &error);
        if (error == MB_ERROR_NO_ERROR)
          status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ping[i].bathy,
                                     &error);
      }
      double *amp = nullptr;
      double *ss = nullptr;
      double *ssacrosstrack = nullptr;
      double *ssalongtrack = nullptr;
      double *list = nullptr;
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&amp, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ss, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssacrosstrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status =
            mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ssalongtrack, &error);
      if (error == MB_ERROR_NO_ERROR)
        status = mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 4 * sizeof(double), (void **)&list, &error);

      /* if error initializing memory then quit */
      if (error != MB_ERROR_NO_ERROR) {
        char *message = nullptr;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }

      /* now deal with old edit save file */
      bool esffile_open = false;
      if (status == MB_SUCCESS) {
        /* reset message */
        fprintf(stderr, "Sorting old edits...\n");

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
        fprintf(stderr, "%d old edits sorted...\n", esf.nedit);
      }

      /* read */
      int nrec = 0;
      fprintf(stderr, "Processing data...\n");
      bool done = false;
      while (!done) {
        if (verbose > 1)
          fprintf(stderr, "\n");

        /* read next record */
        error = MB_ERROR_NO_ERROR;
        status = mb_get(verbose, mbio_ptr, &kind, &pingsread, ping[nrec].time_i, &ping[nrec].time_d, &ping[nrec].navlon,
                        &ping[nrec].navlat, &ping[nrec].speed, &ping[nrec].heading, &distance, &altitude, &sensordepth,
                        &ping[nrec].beams_bath, &beams_amp, &pixels_ss, ping[nrec].beamflag, ping[nrec].bath, amp,
                        ping[nrec].bathacrosstrack, ping[nrec].bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment,
                        &error);
        if (verbose >= 2) {
          fprintf(stderr, "\ndbg2  current data status:\n");
          fprintf(stderr, "dbg2    kind:           %d\n", kind);
          fprintf(stderr, "dbg2    status:         %d\n", status);
          fprintf(stderr, "dbg2    ndata:          %d\n", ndata);
          fprintf(stderr, "dbg2    nrec:           %d\n", nrec);
          fprintf(stderr, "dbg2    nflagesf:       %d\n", nflagesf);
          fprintf(stderr, "dbg2    nunflagesf:     %d\n", nunflagesf);
          fprintf(stderr, "dbg2    nzeroesf:       %d\n", nzeroesf);
          fprintf(stderr, "dbg2    nouterbeams:    %d\n", nouterbeams);
          fprintf(stderr, "dbg2    nouterdistance: %d\n", nouterdistance);
          fprintf(stderr, "dbg2    nmin:           %d\n", nmin);
          fprintf(stderr, "dbg2    ndepthrange:    %d\n", ndepthrange);
          fprintf(stderr, "dbg2    nminrange:      %d\n", nminrange);
          fprintf(stderr, "dbg2    nfraction:      %d\n", nfraction);
          fprintf(stderr, "dbg2    nspeed:         %d\n", nspeed);
          fprintf(stderr, "dbg2    nzeropos:       %d\n", nzeropos);
          fprintf(stderr, "dbg2    nrangepos:      %d\n", nrangepos);
          fprintf(stderr, "dbg2    ndeviation:     %d\n", ndeviation);
          fprintf(stderr, "dbg2    nrail:          %d\n", nrail);
          fprintf(stderr, "dbg2    nlong_across:   %d\n", nlong_across);
          fprintf(stderr, "dbg2    nbad:           %d\n", nbad);
          fprintf(stderr, "dbg2    npike;          %d\n", nspike);
          fprintf(stderr, "dbg2    nflag:          %d\n", nflag);
          fprintf(stderr, "dbg2    nunflag:        %d\n", nunflag);
        }
        if (status == MB_SUCCESS && kind == MB_DATA_DATA) {
          /* check for ping multiplicity */
          status = mb_get_store(verbose, mbio_ptr, &store_ptr, &error);
          const int sensorhead_status = mb_sensorhead(verbose, mbio_ptr, store_ptr, &sensorhead, &sensorhead_error);
          if (sensorhead_status == MB_SUCCESS) {
            ping[nrec].multiplicity = sensorhead;
          }
          else if (nrec > 0 && fabs(ping[nrec].time_d - ping[nrec - 1].time_d) < MB_ESF_MAXTIMEDIFF) {
            ping[nrec].multiplicity = ping[nrec - 1].multiplicity + 1;
          }
          else {
            ping[nrec].multiplicity = 0;
          }

          /* save original beamflags */
          for (int i = 0; i < ping[nrec].beams_bath; i++) {
            ping[nrec].beamflagorg[i] = ping[nrec].beamflag[i];
          }

          /* get locations of data points in local coordinates */
          mb_coor_scale(verbose, ping[nrec].navlat, &mtodeglon, &mtodeglat);
          const double headingx = sin(ping[nrec].heading * DTR);
          const double headingy = cos(ping[nrec].heading * DTR);
          for (int j = 0; j <= nrec; j++) {
            for (int i = 0; i < ping[j].beams_bath; i++) {
              ping[j].bathx[i] = (ping[j].navlon - ping[0].navlon) / mtodeglon +
                                 headingy * ping[j].bathacrosstrack[i] + headingx * ping[j].bathalongtrack[i];
              ping[j].bathy[i] = (ping[j].navlat - ping[0].navlat) / mtodeglat -
                                 headingx * ping[j].bathacrosstrack[i] + headingy * ping[j].bathalongtrack[i];
            }
          }
          if (verbose >= 2) {
            fprintf(stderr, "\ndbg2  beam locations (ping:beam xxx.xxx yyy.yyy)\n");
            for (int j = 0; j <= nrec; j++)
              for (int i = 0; i < ping[j].beams_bath; i++) {
                fprintf(stderr, "dbg2    %d:%3.3d %10.3f %10.3f\n", j, i, ping[j].bathx[i], ping[j].bathy[i]);
              }
          }

          /* if requested set all edit timestamps within tolerance of
              ping[nrec].time_d to ping[nrec].time_d */
          if (fix_edit_timestamps)
            mb_esf_fixtimestamps(verbose, &esf, ping[nrec].time_d, tolerance, &error);

          /* apply saved edits */
          mb_esf_apply(verbose, &esf, ping[nrec].time_d, ping[nrec].multiplicity, ping[nrec].beams_bath,
                                ping[nrec].beamflag, &error);

          /* update counters */
          for (int i = 0; i < ping[nrec].beams_bath; i++) {
            if (ping[nrec].beamflag[i] != ping[nrec].beamflagorg[i]) {
              if (mb_beam_ok(ping[nrec].beamflag[i]))
                nunflagesf++;
              else
                nflagesf++;
            }
          }
          ndata++;
          nrec++;
        }
        else if (error > MB_ERROR_NO_ERROR) {
          done = true;
        }

        /* process a record */
        if (nrec > 0) {
          /* get record to process */
          // int irec;  // TODO(schwehr): Probable bug setting irec.
          // if (nrec >= 2)
          //  irec = 1;
          // else if (nrec == 1)
          //   irec = 0;
          const int irec = nrec >= 2 ? 1 : 0;

          /* get center beam */
          center = ping[irec].beams_bath / 2;

          /* zap outer beams by number if requested */
          if (zap_beams) {
            for (int i = 0; i < std::min(zap_beams_left, center); i++) {
              if (mb_beam_ok(ping[irec].beamflag[i])) {
                if (verbose >= 1)
                  fprintf(stderr, "x: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nouterbeams++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
            for (int i = 0; i < std::min(zap_beams_right, center); i++) {
              int j = ping[irec].beams_bath - i - 1;
              if (mb_beam_ok(ping[irec].beamflag[j])) {
                if (verbose >= 1)
                  fprintf(stderr, "x: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], j,
                          ping[irec].bath[j]);
                ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nouterbeams++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            j + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* flag outer beams by distance if requested */
          if (flag_distance) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i]) && (ping[irec].bathacrosstrack[i] <= flag_distance_left ||
                                                         ping[irec].bathacrosstrack[i] >= flag_distance_right)) {
                if (verbose >= 1)
                  fprintf(stderr, "y: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nouterdistance++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* unflag inner beams by distance if requested */
          if (unflag_distance) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (ping[irec].beamflag[i] != MB_FLAG_NULL && !mb_beam_ok(ping[irec].beamflag[i]) &&
                  (ping[irec].bathacrosstrack[i] >= unflag_distance_left &&
                   ping[irec].bathacrosstrack[i] <= unflag_distance_right)) {
                if (verbose >= 1)
                  fprintf(stderr, "y: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_NONE;
                ninnerdistance++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_UNFLAG, &error);
              }
            }
          }

          /* flag outer beams by acrosstrack angle if requested */
          if (flag_angle) {;
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              double theta;
              double phi;
              double pitch;
              double roll;
              if (mb_beam_ok(ping[irec].beamflag[i])) {
                const double xx = ping[irec].bathacrosstrack[i];
                const double yy = ping[irec].bathalongtrack[i];
                const double zz = ping[irec].bath[i] - sensordepth;
                mb_xyz_to_takeoff(verbose, xx, yy, zz, &theta, &phi, &error);
                mb_takeoff_to_rollpitch(verbose, theta, phi, &pitch, &roll, &error);
                roll = 90.0 - roll;
                if (roll <= flag_angle_left || roll >= flag_angle_right) {
                  if (verbose >= 1)
                    fprintf(stderr, "a: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                            ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                            ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                            ping[irec].bath[i]);
                  ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                  nouterangle++;
                  nflag++;
                  mb_ess_save(verbose, &esf, ping[irec].time_d,
                              i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
                }
              }
            }
          }

          /* unflag inner beams by acrosstrack angle if requested */
          if (unflag_angle) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              double theta;
              double phi;
              double pitch;
              double roll;
              if (ping[irec].beamflag[i] != MB_FLAG_NULL && !mb_beam_ok(ping[irec].beamflag[i])) {
                const double xx = ping[irec].bathacrosstrack[i];
                const double yy = ping[irec].bathalongtrack[i];
                const double zz = ping[irec].bath[i] - sensordepth;
                mb_xyz_to_takeoff(verbose, xx, yy, zz, &theta, &phi, &error);
                mb_takeoff_to_rollpitch(verbose, theta, phi, &pitch, &roll, &error);
                roll = 90.0 - roll;
                if (roll >= unflag_angle_left && roll <= unflag_angle_right) {
                  if (verbose >= 1)
                    fprintf(stderr, "a: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                            ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                            ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                            ping[irec].bath[i]);
                  ping[irec].beamflag[i] = MB_FLAG_NONE;
                  ninnerangle++;
                  nflag++;
                  mb_ess_save(verbose, &esf, ping[irec].time_d,
                              i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_UNFLAG, &error);
                }
              }
            }
          }

          /* check for speed range if requested */
          if (check_speed_good) {
            if (ping[irec].speed > speed_high || ping[irec].speed < speed_low) {
              for (int i = 0; i < ping[irec].beams_bath; i++) {
                if (verbose >= 1)
                  fprintf(stderr, "p: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3], ping[irec].time_i[4],
                          ping[irec].time_i[5], ping[irec].time_i[6], i, ping[irec].speed);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nspeed++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* check for range latitude and longitude if requested */
          if (check_position_bounds) {
            if (ping[irec].navlon < west || ping[irec].navlon > east || ping[irec].navlat < south ||
                ping[irec].navlat > north) {
              for (int i = 0; i < ping[irec].beams_bath; i++) {
                if (verbose >= 1)
                  fprintf(stderr, "w: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %f %f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3], ping[irec].time_i[4],
                          ping[irec].time_i[5], ping[irec].time_i[6], i, mtodeglon, mtodeglat);
                ping[irec].beamflag[i] = MB_FLAG_NULL;
                nrangepos++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_ZERO, &error);
              }
            }
          }

          /* check for zero latitude and longitude if requested */
          if (check_zero_position) {
            if (ping[irec].navlon == 0.0 && ping[irec].navlat == 0.0) {
              for (int i = 0; i < ping[irec].beams_bath; i++) {
                if (verbose >= 1)
                  fprintf(stderr, "z: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %f %f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3], ping[irec].time_i[4],
                          ping[irec].time_i[5], ping[irec].time_i[6], i, mtodeglon, mtodeglat);
                ping[irec].beamflag[i] = MB_FLAG_NULL;
                nzeropos++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_ZERO, &error);
              }
            }
          }

          /* check depths for acceptable range if requested */
          if (check_range) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i]) &&
                  (ping[irec].bath[i] < depth_low || ping[irec].bath[i] > depth_high)) {
                if (verbose >= 1)
                  fprintf(stderr, "b: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                ndepthrange++;
                nflag++;

                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* check depths for minimum range if requested (replacement for Dana Yoerger test) */
          if (check_range_min) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i]) &&
                  sqrt(ping[irec].bathacrosstrack[i] * ping[irec].bathacrosstrack[i] +
                       ping[irec].bathalongtrack[i] * ping[irec].bathalongtrack[i] +
                       (ping[irec].bath[i] - sensordepth) * (ping[irec].bath[i] - sensordepth)) < range_min) {
                if (verbose >= 1)
                  fprintf(stderr, "k: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nminrange++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* check for max heading rate if requested */
          if (zap_max_heading_rate) {
            double heading_rate;
            if (nrec > 1) {
              double dh = ping[nrec-1].heading - ping[0].heading;
              if (dh > 180)
                dh -= 360;
              if (dh < -180)
                dh += 360;
              heading_rate = dh / (ping[nrec-1].time_d - ping[0].time_d);
            } else {
              heading_rate = 0.0;
            }
            //printf("heading rate: %.3f deg/s",heading_rate);
            //if (fabs(heading_rate) > max_heading_rate) printf(" ********");
            //printf("\n");

            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (fabs(heading_rate) > max_heading_rate) {
                if (verbose >= 1)
                  fprintf(stderr, "r: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], i,
                          ping[irec].bath[i]);
                ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nmax_heading_rate++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* zap rails if requested */
          if (zap_rails) {
            /* flag all beams with acrosstrack distance less than the maximum out to that beam */
            lowdist = 0.0;
            highdist = 0.0;

            for (int j = center; j < ping[irec].beams_bath; j++) {
              if (mb_beam_ok(ping[irec].beamflag[j]) && ping[irec].bathacrosstrack[j] <= highdist - backup_dist) {
                if (verbose >= 1)
                  fprintf(stderr, "q: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], j,
                          ping[irec].bath[j]);
                ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nrail++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            j + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
              else
                highdist = ping[irec].bathacrosstrack[j];

              int k = center - (j - center) - 1;
              if (mb_beam_ok(ping[irec].beamflag[k]) && ping[irec].bathacrosstrack[k] >= lowdist + backup_dist) {
                if (verbose >= 1)
                  fprintf(stderr, "q: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], k,
                          ping[irec].bath[k]);
                ping[irec].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nrail++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            k + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
              else
                lowdist = ping[irec].bathacrosstrack[k];
            }
          } // if zap_rails

          /* zap long acrosstrack if requested */
          if (zap_long_across) {
            for (int j = 0; j < ping[irec].beams_bath; j++) {
              if (mb_beam_ok(ping[irec].beamflag[j]) && fabs(ping[irec].bathacrosstrack[j]) > max_acrosstrack) {
                if (verbose >= 1)
                  fprintf(stderr, "e: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f\n", ping[irec].time_i[0],
                          ping[irec].time_i[1], ping[irec].time_i[2], ping[irec].time_i[3],
                          ping[irec].time_i[4], ping[irec].time_i[5], ping[irec].time_i[6], j,
                          ping[irec].bath[j]);
                ping[irec].beamflag[j] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                nlong_across++;
                nflag++;
                mb_ess_save(verbose, &esf, ping[irec].time_d,
                            j + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER, &error);
              }
            }
          }

          /* do tests that require looping over all available beams */
          if (check_fraction || check_deviation || check_spike || check_slope) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i])) {
                /* get local median value from all available records */
                if (median <= 0.0)
                  median = ping[irec].bath[i];
                nlist = 0;
                for (int j = 0; j < nrec; j++) {
                  for (int k = 0; k < ping[j].beams_bath; k++) {
                    if (mb_beam_ok(ping[j].beamflag[k])) {
                      const double dd = sqrt((ping[j].bathx[k] - ping[irec].bathx[i]) *
                                    (ping[j].bathx[k] - ping[irec].bathx[i]) +
                                (ping[j].bathy[k] - ping[irec].bathy[i]) *
                                    (ping[j].bathy[k] - ping[irec].bathy[i]));
                      if (dd <= distancemax * median) {
                        list[nlist] = ping[j].bath[k];
                        nlist++;
                      }
                    }
                  }
                }
                qsort((char *)list, nlist, sizeof(double), mb_double_compare);
                median = list[nlist / 2];
                if (verbose >= 2) {
                  fprintf(stderr, "\ndbg2  depth statistics:\n");
                  fprintf(stderr, "dbg2    number:        %d\n", nlist);
                  fprintf(stderr, "dbg2    minimum depth: %f\n", list[0]);
                  fprintf(stderr, "dbg2    median depth:  %f\n", median);
                  fprintf(stderr, "dbg2    maximum depth: %f\n", list[nlist - 1]);
                }

                /* check fractional deviation from median if desired */
                if (check_fraction && median > 0.0) {
                  if (ping[irec].bath[i] / median < fraction_low ||
                      ping[irec].bath[i] / median > fraction_high) {
                    if (verbose >= 1)
                      fprintf(stderr, "f: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
                              ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                              ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                              ping[irec].time_i[6], i, ping[irec].bath[i], median);
                    ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    nfraction++;
                    nflag++;
                    mb_ess_save(verbose, &esf, ping[irec].time_d,
                                i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
                                &error);
                  }
                }

                /* check absolute deviation from median if desired */
                if (check_deviation && median > 0.0) {
                  if (fabs(ping[irec].bath[i] - median) > deviation_max) {
                    if (verbose >= 1)
                      fprintf(stderr, "a: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f\n",
                              ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                              ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                              ping[irec].time_i[6], i, ping[irec].bath[i], median);
                    ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    ndeviation++;
                    nflag++;
                    mb_ess_save(verbose, &esf, ping[irec].time_d,
                                i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
                                &error);
                  }
                }

                /* check spikes - acrosstrack */
                if (check_spike && 0 != (spike_mode & 1) && median > 0.0 && i > 0 &&
                    i < ping[irec].beams_bath - 1 && mb_beam_ok(ping[irec].beamflag[i - 1]) &&
                    mb_beam_ok(ping[irec].beamflag[i + 1])) {
                  const double dd = sqrt((ping[irec].bathx[i - 1] - ping[irec].bathx[i]) *
                                (ping[irec].bathx[i - 1] - ping[irec].bathx[i]) +
                            (ping[irec].bathy[i - 1] - ping[irec].bathy[i]) *
                                (ping[irec].bathy[i - 1] - ping[irec].bathy[i]));
                  if (dd > distancemin * median && dd <= distancemax * median) {
                    const double slope = (ping[irec].bath[i - 1] - ping[irec].bath[i]) / dd;
                    const double dd2 = sqrt((ping[irec].bathx[i + 1] - ping[irec].bathx[i]) *
                                   (ping[irec].bathx[i + 1] - ping[irec].bathx[i]) +
                               (ping[irec].bathy[i + 1] - ping[irec].bathy[i]) *
                                   (ping[irec].bathy[i + 1] - ping[irec].bathy[i]));
                    if (dd2 > distancemin * median && dd2 <= distancemax * median) {
                      const double slope2 = (ping[irec].bath[i] - ping[irec].bath[i + 1]) / dd2;
                      if ((slope > spikemax && slope2 < -spikemax) ||
                          (slope2 > spikemax && slope < -spikemax)) {
                        nspike++;
                        nflag++;
                        ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                        mb_ess_save(verbose, &esf, ping[irec].time_d,
                                    i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                    MBP_EDIT_FILTER, &error);
                        if (verbose >= 1) {
                          if (verbose >= 2)
                            fprintf(stderr, "\n");
                          fprintf(stderr,
                                  "s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f "
                                  "%6.2f %6.2f\n",
                                  ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                                  ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                                  ping[irec].time_i[6], i, ping[irec].bath[i], median, slope, slope2,
                                  dd, dd2);
                        }
                      }
                    }
                  }
                }

                /* check spikes - alongtrack */
                if (check_spike && nrec == 3 && 0 != (spike_mode & 2) &&
                    mb_beam_ok(ping[0].beamflag[i]) && mb_beam_ok(ping[2].beamflag[i])) {
                  const double dd = sqrt((ping[0].bathx[i] - ping[1].bathx[i]) * (ping[0].bathx[i] - ping[1].bathx[i]) +
                            (ping[0].bathy[i] - ping[1].bathy[i]) * (ping[0].bathy[i] - ping[1].bathy[i]));
                  if (dd > distancemin * median && dd <= distancemax * median) {
                    const double slope = (ping[0].bath[i] - ping[1].bath[i]) / dd;
                    const double dd2 = sqrt((ping[2].bathx[i] - ping[1].bathx[i]) * (ping[2].bathx[i] - ping[1].bathx[i]) +
                               (ping[2].bathy[i] - ping[1].bathy[i]) * (ping[2].bathy[i] - ping[1].bathy[i]));
                    if (dd2 > distancemin * median && dd2 <= distancemax * median) {
                      const double slope2 = (ping[1].bath[i] - ping[2].bath[i]) / dd2;
                      if ((slope > spikemax && slope2 < -spikemax) ||
                          (slope2 > spikemax && slope < -spikemax)) {
                        nspike++;
                        nflag++;
                        ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                        mb_ess_save(verbose, &esf, ping[1].time_d,
                                    i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                    MBP_EDIT_FILTER, &error);
                        if (verbose >= 1) {
                          if (verbose >= 2)
                            fprintf(stderr, "\n");
                          fprintf(stderr,
                                  "s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f "
                                  "%6.2f %6.2f\n",
                                  ping[1].time_i[0], ping[1].time_i[1], ping[1].time_i[2],
                                  ping[1].time_i[3], ping[1].time_i[4], ping[1].time_i[5],
                                  ping[1].time_i[6], i, ping[1].bath[i], median, slope, slope2, dd,
                                  dd2);
                        }
                      }
                    }
                  }
                }

                /* check slopes - loop over each of the beams in the current ping */
                if (check_slope && nrec == 3 && median > 0.0)
                  for (int j = 0; j < nrec; j++) {
                    for (int k = 0; k < ping[j].beams_bath; k++) {
                      if (mb_beam_ok(ping[j].beamflag[k])) {
                        const double dd = sqrt((ping[j].bathx[k] - ping[1].bathx[i]) *
                                      (ping[j].bathx[k] - ping[1].bathx[i]) +
                                  (ping[j].bathy[k] - ping[1].bathy[i]) *
                                      (ping[j].bathy[k] - ping[1].bathy[i]));
                        const double slope =
                            dd > 0.0 && dd <= distancemax * median
                            ? fabs((ping[j].bath[k] - ping[1].bath[i]) / dd)
                            : 0.0;
                        // TODO(schwehr): Make sure bad is being set correctly.
                        struct bad_struct bad[2] = {
                          {false, 0, 0, 0.0},
                          {false, 0, 0, 0.0},
                        };
                        if (slope > slopemax && dd > distancemin * median) {
                          if (mode == MBCLEAN_FLAG_BOTH) {
                            bad[0].flag = true;
                            bad[0].ping = j;
                            bad[0].beam = k;
                            bad[0].bath = ping[j].bath[k];
                            bad[1].flag = true;
                            bad[1].ping = 1;
                            bad[1].beam = i;
                            bad[1].bath = ping[1].bath[i];
                            ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                            ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                            nbad++;
                            nflag = nflag + 2;
                            mb_ess_save(verbose, &esf, ping[j].time_d,
                                        k + ping[j].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                        MBP_EDIT_FILTER, &error);
                            mb_ess_save(verbose, &esf, ping[1].time_d,
                                        i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                        MBP_EDIT_FILTER, &error);
                          }
                          else {
                            if (fabs((double)ping[j].bath[k] - median) >
                                fabs((double)ping[1].bath[i] - median)) {
                              bad[0].flag = true;
                              bad[0].ping = j;
                              bad[0].beam = k;
                              bad[0].bath = ping[j].bath[k];
                              bad[1].flag = false;
                              ping[j].beamflag[k] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                              mb_ess_save(verbose, &esf, ping[j].time_d,
                                          k + ping[j].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                          MBP_EDIT_FILTER, &error);
                            }
                            else {
                              bad[0].flag = true;
                              bad[0].ping = 1;
                              bad[0].beam = i;
                              bad[0].bath = ping[1].bath[i];
                              bad[1].flag = false;
                              ping[1].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                              mb_ess_save(verbose, &esf, ping[1].time_d,
                                          i + ping[1].multiplicity * MB_ESF_MULTIPLICITY_FACTOR,
                                          MBP_EDIT_FILTER, &error);
                            }
                            nbad++;
                            nflag++;
                          }
                        }
                        if (verbose >= 1 && slope > slopemax && dd > distancemin * median &&
                            bad[0].flag) {
                          const int p = bad[0].ping;
                          const int b = bad[0].beam;
                          if (verbose >= 2)
                            fprintf(stderr, "\n");
                          fprintf(
                              stderr,
                              "s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f\n",
                              ping[p].time_i[0], ping[p].time_i[1], ping[p].time_i[2],
                              ping[p].time_i[3], ping[p].time_i[4], ping[p].time_i[5],
                              ping[p].time_i[6], b, bad[0].bath, median, slope, dd);
                        }
                        if (verbose >= 1 && slope > slopemax && dd > distancemin * median &&
                            bad[1].flag) {
                          const int p = bad[1].ping;
                          const int b = bad[1].beam;
                          if (verbose >= 2)
                            fprintf(stderr, "\n");
                          fprintf(
                              stderr,
                              "s: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %8.2f %6.2f %6.2f\n",
                              ping[p].time_i[0], ping[p].time_i[1], ping[p].time_i[2],
                              ping[p].time_i[3], ping[p].time_i[4], ping[p].time_i[5],
                              ping[p].time_i[6], b, bad[1].bath, median, slope, dd);
                        }
                      }
                    }
                  }
              }
            }
          }

          /* check for minimum number of good depths
              on each side of swath */
          if (check_num_good_min && num_good_min > 0) {
            /* do port */
            num_good = 0;
            for (int i = 0; i < center; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i]))
                num_good++;
            }
            if (num_good < num_good_min) {
              for (int i = 0; i < center; i++) {
                if (mb_beam_ok(ping[irec].beamflag[i])) {
                  if (verbose >= 1)
                    fprintf(stderr, "n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
                            ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                            ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                            ping[irec].time_i[6], i, ping[irec].bath[i], num_good, num_good_min);
                  ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                  nmin++;
                  nflag++;
                  mb_ess_save(verbose, &esf, ping[irec].time_d,
                              i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
                              &error);
                }
              }
            }

            /* do starboard */
            num_good = 0;
            for (int i = center + 1; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec].beamflag[i]))
                num_good++;
            }
            if (num_good < num_good_min) {
              for (int i = center + 1; i < ping[irec].beams_bath; i++) {
                if (mb_beam_ok(ping[irec].beamflag[i])) {
                  if (verbose >= 1)
                    fprintf(stderr, "n: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %3d\n",
                            ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                            ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                            ping[irec].time_i[6], i, ping[irec].bath[i], num_good, num_good_min);
                  ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                  nmin++;
                  nflag++;
                  mb_ess_save(verbose, &esf, ping[irec].time_d,
                              i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
                              &error);
                }
              }
            }
          }

          /* check ping deviation */
          if (check_ping_deviation && nrec >= 3) {
            double devsqsum = 0.0;
            int ndevsqsum = 0;
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (mb_beam_ok(ping[irec-1].beamflag[i])
                && mb_beam_ok(ping[irec].beamflag[i])
                && mb_beam_ok(ping[irec+1].beamflag[i])) {
                dev = (ping[irec].bath[i] - ping[irec+1].bath[i])
                      + (ping[irec].bath[i] - ping[irec-1].bath[i]);
                devsqsum += dev * dev;
                ndevsqsum++;
              }
            }
            if (ndevsqsum > (ping[irec].beams_bath / 4)) {
              ping_deviation = sqrt(devsqsum / ndevsqsum);
              if (ping_deviation > ping_deviation_tolerance) {
                for (int i = 0; i < ping[irec].beams_bath; i++) {
                  if (mb_beam_ok(ping[irec].beamflag[i])) {
                    if (verbose >= 1)
                      fprintf(stderr, "p: %4d %2d %2d %2.2d:%2.2d:%2.2d.%6.6d  %4d %8.2f %3d %f %f\n",
                              ping[irec].time_i[0], ping[irec].time_i[1], ping[irec].time_i[2],
                              ping[irec].time_i[3], ping[irec].time_i[4], ping[irec].time_i[5],
                              ping[irec].time_i[6], i, ping[irec].bath[i],
                              ndevsqsum, ping_deviation, ping_deviation_tolerance);
                    ping[irec].beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
                    npingdeviation++;
                    nflag++;
                    mb_ess_save(verbose, &esf, ping[irec].time_d,
                                i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, MBP_EDIT_FILTER,
                                &error);
                  }
                }
              }
            }
          }
        }

        /* write out edits from completed pings */
        if ((status == MB_SUCCESS && nrec == 3) || done) {
          int k;
          if (done)
            k = nrec;
          else
            k = 1;
          for (int irec = 0; irec < k; irec++) {
            for (int i = 0; i < ping[irec].beams_bath; i++) {
              if (ping[irec].beamflag[i] != ping[irec].beamflagorg[i]) {
                if (mb_beam_ok(ping[irec].beamflag[i])) {
                  action = MBP_EDIT_UNFLAG;
                }
                else if (mb_beam_check_flag_filter2(ping[irec].beamflag[i])) {
                  action = MBP_EDIT_FILTER;
                }
                else if (mb_beam_check_flag_filter(ping[irec].beamflag[i])) {
                  action = MBP_EDIT_FILTER;
                }
                else if (ping[irec].beamflag[i] != MB_FLAG_NULL) {
                  action = MBP_EDIT_FLAG;
                }
                else {
                  action = MBP_EDIT_ZERO;
                }
                mb_esf_save(verbose, &esf, ping[irec].time_d,
                            i + ping[irec].multiplicity * MB_ESF_MULTIPLICITY_FACTOR, action, &error);
              }
            }
          }
        }

        /* reset counters and data */
        if (status == MB_SUCCESS && nrec == 3) {
          /* copy data back one */
          nrec = 2;
          for (int j = 0; j < 2; j++) {
            for (int i = 0; i < 7; i++)
              ping[j].time_i[i] = ping[j + 1].time_i[i];
            ping[j].time_d = ping[j + 1].time_d;
            ping[j].multiplicity = ping[j + 1].multiplicity;
            ping[j].navlon = ping[j + 1].navlon;
            ping[j].navlat = ping[j + 1].navlat;
            ping[j].speed = ping[j + 1].speed;
            ping[j].heading = ping[j + 1].heading;
            ping[j].beams_bath = ping[j + 1].beams_bath;
            for (int i = 0; i < ping[j].beams_bath; i++) {
              ping[j].beamflag[i] = ping[j + 1].beamflag[i];
              ping[j].beamflagorg[i] = ping[j + 1].beamflagorg[i];
              ping[j].bath[i] = ping[j + 1].bath[i];
              ping[j].bathacrosstrack[i] = ping[j + 1].bathacrosstrack[i];
              ping[j].bathalongtrack[i] = ping[j + 1].bathalongtrack[i];
              ping[j].bathx[i] = ping[j + 1].bathx[i];
              ping[j].bathy[i] = ping[j + 1].bathy[i];
            }
          }
        }
      }

      status = mb_close(verbose, &mbio_ptr, &error);
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
      nfiletot++;
      ndatatot += ndata;
      nflagesftot += nflagesf;
      nunflagesftot += nunflagesf;
      nzeroesftot += nzeroesf;
      ndepthrangetot += ndepthrange;
      nminrangetot += nminrange;
      nfractiontot += nfraction;
      ndeviationtot += ndeviation;
      nouterbeamstot += nouterbeams;
      nouterdistancetot += nouterdistance;
      ninnerdistancetot += ninnerdistance;
      nouterangletot += nouterangle;
      ninnerangletot += ninnerangle;
      nrailtot += nrail;
      nlong_acrosstot += nlong_across;
      nmax_heading_ratetot += nmax_heading_rate;
      nmintot += nmin;
      nbadtot += nbad;
      nspiketot += nspike;
      npingdeviationtot += npingdeviation;
      nflagtot += nflag;
      nunflagtot += nunflag;

      /* give the statistics */
      if (verbose >= 0) {
        fprintf(stderr, "%d bathymetry data records processed\n", ndata);
        if (esf.nedit > 0) {
          fprintf(stderr, "%d beams flagged in old esf file\n", nflagesf);
          fprintf(stderr, "%d beams unflagged in old esf file\n", nunflagesf);
          fprintf(stderr, "%d beams zeroed in old esf file\n", nzeroesf);
        }
        fprintf(stderr, "%d beams zapped by beam number\n", nouterbeams);
        fprintf(stderr, "%d beams zapped by distance\n", nouterdistance);
        fprintf(stderr, "%d beams unzapped by distance\n", ninnerdistance);
        fprintf(stderr, "%d beams zapped by angle\n", nouterangle);
        fprintf(stderr, "%d beams unzapped by angle\n", ninnerangle);
        fprintf(stderr, "%d beams zapped for too few good beams in ping\n", nmin);
        fprintf(stderr, "%d beams out of acceptable depth range\n", ndepthrange);
        fprintf(stderr, "%d beams less than minimum range\n", nminrange);
        fprintf(stderr, "%d beams out of acceptable fractional depth range\n", nfraction);
        fprintf(stderr, "%d beams out of acceptable speed range\n", nspeed);
        fprintf(stderr, "%d beams have zero position (lat/lon)\n", nzeropos);
        fprintf(stderr, "%d beams exceed acceptable deviation from median depth\n", ndeviation);
        fprintf(stderr, "%d bad rail beams identified\n", nrail);
        fprintf(stderr, "%d long acrosstrack beams identified\n", nlong_across);
        fprintf(stderr, "%d max heading rate pings identified\n", nmax_heading_rate);
        fprintf(stderr, "%d excessive slopes identified\n", nbad);
        fprintf(stderr, "%d excessive spikes identified\n", nspike);
        fprintf(stderr, "%d ping deviations identified\n", npingdeviation);
        fprintf(stderr, "%d beams flagged\n", nflag);
        fprintf(stderr, "%d beams unflagged\n", nunflag);
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
    fprintf(stderr, "\nMBclean Processing Totals:\n");
    fprintf(stderr, "-------------------------\n");
    fprintf(stderr, "%d total swath data files processed\n", nfiletot);
    fprintf(stderr, "%d total bathymetry data records processed\n", ndatatot);
    fprintf(stderr, "%d total beams flagged in old esf files\n", nflagesftot);
    fprintf(stderr, "%d total beams unflagged in old esf files\n", nunflagesftot);
    fprintf(stderr, "%d total beams zeroed in old esf files\n", nzeroesftot);
    fprintf(stderr, "%d total beams zapped by beam number\n", nouterbeamstot);
    fprintf(stderr, "%d total beams zapped by distance\n", nouterdistancetot);
    fprintf(stderr, "%d total beams unzapped by distance\n", ninnerdistancetot);
    fprintf(stderr, "%d total beams zapped by angle\n", nouterangletot);
    fprintf(stderr, "%d total beams unzapped by angle\n", ninnerangletot);
    fprintf(stderr, "%d total beams zapped for too few good beams in ping\n", nmintot);
    fprintf(stderr, "%d total beams out of acceptable depth range\n", ndepthrangetot);
    fprintf(stderr, "%d total beams less than minimum range\n", nminrangetot);
    fprintf(stderr, "%d total beams out of acceptable fractional depth range\n", nfractiontot);
    // int nspeedtot = 0;
    // fprintf(stderr, "%d total beams out of acceptable speed range\n", nspeedtot);
    // int nzeropostot = 0;
    // fprintf(stderr, "%d total beams zero position (lat/lon)\n", nzeropostot);
    fprintf(stderr, "%d total beams exceed acceptable deviation from median depth\n", ndeviationtot);
    fprintf(stderr, "%d total bad rail beams identified\n", nrailtot);
    fprintf(stderr, "%d total long acrosstrack beams identified\n", nlong_acrosstot);
    fprintf(stderr, "%d total max heading rate beams identified\n", nmax_heading_ratetot);
    fprintf(stderr, "%d total excessive spikes identified\n", nspiketot);
    fprintf(stderr, "%d total excessive slopes identified\n", nbadtot);
    fprintf(stderr, "%d ping deviations identified\n", npingdeviationtot);
    fprintf(stderr, "%d total beams flagged\n", nflagtot);
    fprintf(stderr, "%d total beams unflagged\n", nunflagtot);
  }

  /* check memory */
  if ((status = mb_memory_list(verbose, &error)) == MB_FAILURE) {
    fprintf(stderr, "Program %s completed but failed to deallocate all allocated memory - the code has a memory leak somewhere!\n", program_name);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
