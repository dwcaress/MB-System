/*--------------------------------------------------------------------
 *    The MB-system:  mblist.c  2/1/93
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
 * MBlist prints the specified contents of a swath sonar data
 * file to stdout. The form of the output is quite flexible;
 * MBlist is tailored to produce ascii files in spreadsheet
 * style with data columns separated by tabs.
 *
 * Author:  D. W. Caress
 * Date:  February 1, 1993
 *
 * Note:  This program was originally based on the identically named program
 *    mblist created by Alberto Malinverno (then at L-DEO, later at Schlumberger,
 *    even later returned to L-DEO) in August 1991.  It also included elements
 *    derived from the program mbdump created by D. Caress in 1990.
 *    Gordon Keith of CSIRO (since retired) greatly augmented the mblist output
 *    capabilities during the early 2000's.
 *
 */

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <getopt.h>
#include <unistd.h>
#include <limits>

#include <algorithm>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_simrad2.h"
#include "mbsys_simrad3.h"

constexpr int MAX_OPTIONS = 25;

typedef enum {
    DUMP_MODE_LIST = 1,
    DUMP_MODE_BATH = 2,
    DUMP_MODE_TOPO = 3,
    DUMP_MODE_AMP = 4,
    DUMP_MODE_SS = 5,
} dump_mode_t;
typedef enum {
    MBLIST_CHECK_ON = 0,
    MBLIST_CHECK_ON_NULL = 1,
    MBLIST_CHECK_OFF_RAW = 2,
    MBLIST_CHECK_OFF_NAN = 3,
    MBLIST_CHECK_OFF_FLAGNAN = 4,
} check_t;
typedef enum {
    MBLIST_SET_OFF = 0,
    MBLIST_SET_ON = 1,
    MBLIST_SET_ALL = 2,
    MBLIST_SET_EXCLUDE_OUTER = 3,
} beam_set_t;
typedef enum {
    MBLIST_SEGMENT_MODE_NONE = 0,
    MBLIST_SEGMENT_MODE_TAG = 1,
    MBLIST_SEGMENT_MODE_SWATHFILE = 2,
    MBLIST_SEGMENT_MODE_DATALIST = 3,
} segment_mode_t;

#define SECONDARY_FILE_COLUMNS_MAX 20
int num_secondary = 0;
int num_secondary_columns = 0;
int num_secondary_alloc = 0;
int j_secondary_interp = 0;
double *secondary_time_d = NULL;
double *secondary_data = NULL;

constexpr char program_name[] = "MBLIST";
constexpr char help_message[] =
    "MBLIST prints the specified contents of a swath data\n"
    "file to stdout. The form of the output is quite flexible;\n"
    "MBLIST is tailored to produce ascii files in spreadsheet\n"
    "style with data columns separated by tabs.";
constexpr char usage_message[] =
    "mblist [-Byr/mo/da/hr/mn/sc -C -Ddump_mode -Eyr/mo/da/hr/mn/sc\n"
    "    -Fformat -Gdelimiter -H -Ifile -Jprojection -Kdecimate -Llonflip\n"
    "    -M[beam_start/beam_end | A | X%] -Npixel_start/pixel_end\n"
    "    -Ooptions -Ppings -Rw/e/s/n -Sspeed -Ttimegap -Ucheck -V -W -Xoutfile -Zsegment]";

/*--------------------------------------------------------------------*/
int set_output(int verbose, int beams_bath, int beams_amp, int pixels_ss, bool use_bath, bool use_amp, bool use_ss, dump_mode_t dump_mode,
               beam_set_t beam_set, int pixel_set, int beam_vertical, int pixel_vertical, int *beam_start, int *beam_end,
               int *beam_exclude_percent, int *pixel_start, int *pixel_end, int *n_list, char *list, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBLIST function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       beams_bath:      %d\n", beams_bath);
    fprintf(stderr, "dbg2       beams_amp:       %d\n", beams_amp);
    fprintf(stderr, "dbg2       pixels_ss:       %d\n", pixels_ss);
    fprintf(stderr, "dbg2       use_bath:        %d\n", use_bath);
    fprintf(stderr, "dbg2       use_amp:         %d\n", use_amp);
    fprintf(stderr, "dbg2       use_ss:          %d\n", use_ss);
    fprintf(stderr, "dbg2       dump_mode:       %d\n", dump_mode);
    fprintf(stderr, "dbg2       beam_set:        %d\n", beam_set);
    fprintf(stderr, "dbg2       pixel_set:       %d\n", pixel_set);
    fprintf(stderr, "dbg2       beam_vertical:   %d\n", beam_vertical);
    fprintf(stderr, "dbg2       pixel_vertical:  %d\n", pixel_vertical);
    fprintf(stderr, "dbg2       beam_start:      %d\n", *beam_start);
    fprintf(stderr, "dbg2       beam_end:        %d\n", *beam_end);
    fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", *beam_exclude_percent);
    fprintf(stderr, "dbg2       pixel_start:     %d\n", *pixel_start);
    fprintf(stderr, "dbg2       pixel_end:       %d\n", *pixel_end);
    fprintf(stderr, "dbg2       n_list:          %d\n", *n_list);
    for (int i = 0; i < *n_list; i++)
      fprintf(stderr, "dbg2       list[%2d]:        %c\n", i, list[i]);
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  int status = MB_SUCCESS;

  if (beam_set == MBLIST_SET_OFF && pixel_set == MBLIST_SET_OFF && beams_bath <= 0 && pixels_ss <= 0) {
    *beam_start = 0;
    *beam_end = 1;
    *pixel_start = 0;
    *pixel_end = -1;
  }
  else if (beam_set == MBLIST_SET_OFF && pixel_set != MBLIST_SET_OFF) {
    *beam_start = 0;
    *beam_end = -1;
  }
  else if (beam_set == MBLIST_SET_OFF && beams_bath <= 0) {
    *beam_start = 0;
    *beam_end = -1;
    *pixel_start = pixel_vertical;
    *pixel_end = pixel_vertical;
  }
  else if (beam_set == MBLIST_SET_OFF) {
    *beam_start = beam_vertical;
    *beam_end = beam_vertical;
  }
  else if (beam_set == MBLIST_SET_ALL) {
    *beam_start = 0;
    *beam_end = beams_bath - 1;
  }
  else if (beam_set == MBLIST_SET_EXCLUDE_OUTER) {
    *beam_start = (beams_bath * *beam_exclude_percent) / 100;
    *beam_end = beams_bath - (*beam_start + 1);
  }
  if (pixel_set == MBLIST_SET_OFF && beams_bath > 0) {
    *pixel_start = 0;
    *pixel_end = -1;
  }
  else if (pixel_set == MBLIST_SET_ALL) {
    *pixel_start = 0;
    *pixel_end = pixels_ss - 1;
  }

  /* deal with dump_mode if set */
  if (dump_mode == DUMP_MODE_BATH) {
    *beam_start = 0;
    *beam_end = beams_bath - 1;
    *pixel_start = 0;
    *pixel_end = -1;
    strcpy(list, "XYz");
    *n_list = 3;
  }
  else if (dump_mode == DUMP_MODE_TOPO) {
    *beam_start = 0;
    *beam_end = beams_bath - 1;
    *pixel_start = 0;
    *pixel_end = -1;
    strcpy(list, "XYZ");
    *n_list = 3;
  }
  else if (dump_mode == DUMP_MODE_AMP) {
    *beam_start = 0;
    *beam_end = beams_bath - 1;
    *pixel_start = 0;
    *pixel_end = -1;
    strcpy(list, "XYB");
    *n_list = 3;
  }
  else if (dump_mode == DUMP_MODE_SS) {
    *beam_start = 0;
    *beam_end = -1;
    *pixel_start = 0;
    *pixel_end = pixels_ss - 1;
    strcpy(list, "XYb");
    *n_list = 3;
  }

  /* check if beam and pixel range is ok */
  if ((use_bath && *beam_end >= *beam_start) && beams_bath <= 0) {
    fprintf(stderr, "\nBathymetry data not available\n");
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }
  else if (use_bath && *beam_end >= *beam_start && (*beam_start < 0 || *beam_end >= beams_bath)) {
    fprintf(stderr, "\nBeam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end, beams_bath - 1);
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }
  if (*error == MB_ERROR_NO_ERROR && use_amp && beams_amp <= 0) {
    fprintf(stderr, "\nAmplitude data not available\n");
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }
  else if (*error == MB_ERROR_NO_ERROR && *beam_end >= *beam_start && use_amp &&
           (*beam_start < 0 || *beam_end >= beams_amp)) {
    fprintf(stderr, "\nAmplitude beam range %d to %d exceeds available beams 0 to %d\n", *beam_start, *beam_end,
            beams_amp - 1);
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }
  if (*error == MB_ERROR_NO_ERROR && (use_ss || *pixel_end >= *pixel_start) && pixels_ss <= 0) {
    fprintf(stderr, "\nSidescan data not available\n");
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }
  else if (*error == MB_ERROR_NO_ERROR && *pixel_end >= *pixel_start && (*pixel_start < 0 || *pixel_end >= pixels_ss)) {
    fprintf(stderr, "\nPixels range %d to %d exceeds available pixels 0 to %d\n", *pixel_start, *pixel_end, pixels_ss - 1);
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_USAGE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBCOPY function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2       beam_start:    %d\n", *beam_start);
    fprintf(stderr, "dbg2       beam_end:      %d\n", *beam_end);
    fprintf(stderr, "dbg2       pixel_start:   %d\n", *pixel_start);
    fprintf(stderr, "dbg2       pixel_end:     %d\n", *pixel_end);
    fprintf(stderr, "dbg2       n_list:        %d\n", *n_list);
    for (int i = 0; i < *n_list; i++)
      fprintf(stderr, "dbg2       list[%2d]:      %c\n", i, list[i]);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int set_bathyslope(int verbose, int nbath, char *beamflag, double *bath, double *bathacrosstrack, int *ndepths, double *depths,
                   double *depthacrosstrack, int *nslopes, double *slopes, double *slopeacrosstrack, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       nbath:           %d\n", nbath);
    fprintf(stderr, "dbg2       bath:            %p\n", (void *)bath);
    fprintf(stderr, "dbg2       bathacrosstrack: %p\n", (void *)bathacrosstrack);
    fprintf(stderr, "dbg2       bath:\n");
    for (int i = 0; i < nbath; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, bath[i], bathacrosstrack[i]);
  }

  /* first find all depths */
  *ndepths = 0;
  for (int i = 0; i < nbath; i++) {
    if (mb_beam_ok(beamflag[i])) {
      depths[*ndepths] = bath[i];
      depthacrosstrack[*ndepths] = bathacrosstrack[i];
      (*ndepths)++;
    }
  }

  /* now calculate slopes */
  *nslopes = *ndepths + 1;
  for (int i = 0; i < *ndepths - 1; i++) {
    slopes[i + 1] = (depths[i + 1] - depths[i]) / (depthacrosstrack[i + 1] - depthacrosstrack[i]);
    slopeacrosstrack[i + 1] = 0.5 * (depthacrosstrack[i + 1] + depthacrosstrack[i]);
  }
  if (*ndepths > 1) {
    slopes[0] = 0.0;
    slopeacrosstrack[0] = depthacrosstrack[0];
    slopes[*ndepths] = 0.0;
    slopeacrosstrack[*ndepths] = depthacrosstrack[*ndepths - 1];
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ndepths:         %d\n", *ndepths);
    fprintf(stderr, "dbg2       depths:\n");
    for (int i = 0; i < *ndepths; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
    fprintf(stderr, "dbg2       nslopes:         %d\n", *nslopes);
    fprintf(stderr, "dbg2       slopes:\n");
    for (int i = 0; i < *nslopes; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int get_bathyslope(int verbose, int ndepths, double *depths, double *depthacrosstrack, int nslopes, double *slopes,
                   double *slopeacrosstrack, double acrosstrack, double *depth, double *slope, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       ndepths:         %d\n", ndepths);
    fprintf(stderr, "dbg2       depths:\n");
    for (int i = 0; i < ndepths; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, depths[i], depthacrosstrack[i]);
    fprintf(stderr, "dbg2       nslopes:         %d\n", nslopes);
    fprintf(stderr, "dbg2       slopes:\n");
    for (int i = 0; i < nslopes; i++)
      fprintf(stderr, "dbg2         %d %f %f\n", i, slopes[i], slopeacrosstrack[i]);
    fprintf(stderr, "dbg2       acrosstrack:     %f\n", acrosstrack);
  }

  /* check if acrosstrack is in defined interval */
  bool found_depth = false;
  bool found_slope = false;
  if (ndepths > 1)
    if (acrosstrack >= depthacrosstrack[0] && acrosstrack <= depthacrosstrack[ndepths - 1]) {

      /* look for depth */
      int idepth = -1;
      while (found_depth && idepth < ndepths - 2) {
        idepth++;
        if (acrosstrack >= depthacrosstrack[idepth] && acrosstrack <= depthacrosstrack[idepth + 1]) {
          *depth = depths[idepth] + (acrosstrack - depthacrosstrack[idepth]) /
                                        (depthacrosstrack[idepth + 1] - depthacrosstrack[idepth]) *
                                        (depths[idepth + 1] - depths[idepth]);
          found_depth = true;
          *error = MB_ERROR_NO_ERROR;
        }
      }

      /* look for slope */
      int islope = -1;
      while (!found_slope && islope < nslopes - 2) {
        islope++;
        if (acrosstrack >= slopeacrosstrack[islope] && acrosstrack <= slopeacrosstrack[islope + 1]) {
          *slope = slopes[islope] + (acrosstrack - slopeacrosstrack[islope]) /
                                        (slopeacrosstrack[islope + 1] - slopeacrosstrack[islope]) *
                                        (slopes[islope + 1] - slopes[islope]);
          found_slope = true;
          *error = MB_ERROR_NO_ERROR;
        }
      }
    }

  /* translate slope to degrees */
  if (found_slope)
    *slope = RTD * atan(*slope);

  int status = MB_SUCCESS;

  /* check for failure */
  if (!found_depth || !found_slope) {
    status = MB_FAILURE;
    *error = MB_ERROR_OTHER;
    *depth = 0.0;
    *slope = 0.0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       depth:           %f\n", *depth);
    fprintf(stderr, "dbg2       slope:           %f\n", *slope);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int printsimplevalue(int verbose, FILE *output, double value, int width, int precision, bool ascii, bool *invert, bool *flipsign,
                     int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       value:           %f\n", value);
    fprintf(stderr, "dbg2       width:           %d\n", width);
    fprintf(stderr, "dbg2       precision:       %d\n", precision);
    fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
    fprintf(stderr, "dbg2       invert:          %d\n", *invert);
    fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
  }

  /* make print format */
  char format[24];
  format[0] = '%';
  if (*invert)
    strcpy(format, "%g");
  else if (width > 0)
    snprintf(&format[1], 23, "%d.%df", width, precision);
  else
    snprintf(&format[1], 23, ".%df", precision);

  /* invert value if desired */
  if (*invert) {
    *invert = false;
    if (value != 0.0)
      value = 1.0 / value;
  }

  /* flip sign value if desired */
  if (*flipsign) {
    *flipsign = false;
    value = -value;
  }

  /* print value */
  if (ascii)
    fprintf(output, format, value);
  else
    fwrite(&value, sizeof(double), 1, output);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       invert:          %d\n", *invert);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int printNaN(int verbose, FILE *output, bool ascii, bool *invert, bool *flipsign, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       ascii:           %d\n", ascii);
    fprintf(stderr, "dbg2       invert:          %d\n", *invert);
    fprintf(stderr, "dbg2       flipsign:        %d\n", *flipsign);
  }

  /* reset invert flag */
  if (*invert)
    *invert = false;

  /* reset flipsign flag */
  if (*flipsign)
    *flipsign = false;

  /* print value */
  if (ascii) {
    fprintf(output, "NaN");
  } else {
    const double NaN = std::numeric_limits<double>::quiet_NaN();
    fwrite(&NaN, sizeof(double), 1, output);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       invert:          %d\n", *invert);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
/*
Method to get fields from simrad2 raw data.
*/

int mb_get_raw_simrad2(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate,
                       double *absorption, int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                       double *bsn, double *bso, int *tx, int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples,
                       int *start_sample, int *range, double *depression, double *bs, double *ss_pixels, int *error) {
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr;
  struct mbsys_simrad2_struct *store_ptr;
  struct mbsys_simrad2_ping_struct *ping_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
  }

  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  store_ptr = (struct mbsys_simrad2_struct *)mb_io_ptr->store_data;
  ping_ptr = store_ptr->ping;

  if (store_ptr->kind == MB_DATA_DATA) {
    *mode = store_ptr->run_mode;
    *ipulse_length = store_ptr->run_tran_pulse;
    *png_count = ping_ptr->png_count;
    *sample_rate = ping_ptr->png_sample_rate;
    *absorption = ping_ptr->png_max_range * 0.01;
    *max_range = ping_ptr->png_max_range;
    *r_zero = ping_ptr->png_r_zero;
    *r_zero_corr = ping_ptr->png_r_zero_corr;
    *tvg_start = ping_ptr->png_tvg_start;
    *tvg_stop = ping_ptr->png_tvg_stop;
    *bsn = ping_ptr->png_bsn * 0.5;
    *bso = ping_ptr->png_bso * 0.5;
    *tx = ping_ptr->png_tx;
    *tvg_crossover = ping_ptr->png_tvg_crossover;
    *nbeams_ss = ping_ptr->png_nbeams_ss;
    *npixels = ping_ptr->png_npixels;

    for (int i = 0; i < ping_ptr->png_nbeams; i++) {
      range[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_range[i];
      depression[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_depression[i] * .01;
      bs[ping_ptr->png_beam_num[i] - 1] = ping_ptr->png_amp[i] * 0.5;
    }
    for (int i = 0; i < ping_ptr->png_nbeams_ss; i++) {
      beam_samples[ping_ptr->png_beam_index[i]] = ping_ptr->png_beam_samples[i];
      start_sample[ping_ptr->png_beam_index[i]] = ping_ptr->png_start_sample[i];
    }
    for (int i = 0; i < ping_ptr->png_npixels; i++)
      ss_pixels[i] = ping_ptr->png_ssraw[i] * 0.5;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       mode:            %d\n", *mode);
    fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
    fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
    fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
    fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
    fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
    fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
    fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
    fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
    fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
    fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
    fprintf(stderr, "dbg2       bso:             %f\n", *bso);
    fprintf(stderr, "dbg2       tx:              %d\n", *tx);
    fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
    fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
    fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
    }
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
    }
    for (int i = 0; i < *npixels; i++) {
      fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return status;
}
/*--------------------------------------------------------------------*/
/*
Method to get fields from simrad3 raw data.
*/

int mb_get_raw_simrad3(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate,
                       double *absorption, int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop,
                       double *bsn, double *bso, int *tx, int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples,
                       int *start_sample, int *range, double *depression, double *bs, double *ss_pixels, int *error) {
  struct mb_io_struct *mb_io_ptr;
  struct mbsys_simrad3_struct *store_ptr;
  struct mbsys_simrad3_ping_struct *ping_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
  }

  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  store_ptr = (struct mbsys_simrad3_struct *)mb_io_ptr->store_data;
  ping_ptr = (struct mbsys_simrad3_ping_struct *)&(store_ptr->pings[store_ptr->ping_index]);

  if (store_ptr->kind == MB_DATA_DATA) {
    *mode = store_ptr->run_mode;
    *ipulse_length = store_ptr->run_tran_pulse;
    *png_count = ping_ptr->png_count;
    *sample_rate = ping_ptr->png_sample_rate;
    *absorption = store_ptr->run_absorption * 0.01;
    *max_range = 0;
    *r_zero = ping_ptr->png_r_zero;
    *r_zero_corr = 0;
    *tvg_start = 0;
    *tvg_stop = 0;
    *bsn = ping_ptr->png_bsn * 0.1;
    *bso = ping_ptr->png_bso * 0.1;
    *tx = ping_ptr->png_tx * 0.1;
    *tvg_crossover = ping_ptr->png_tvg_crossover;
    *nbeams_ss = ping_ptr->png_nbeams_ss;
    *npixels = ping_ptr->png_npixels;

    for (int i = 0; i < ping_ptr->png_nbeams; i++) {
      range[i] = ping_ptr->png_range[i];
      depression[i] = ping_ptr->png_depression[i] * .01;
      bs[i] = ping_ptr->png_amp[i] * 0.5;
    }
    for (int i = 0; i < ping_ptr->png_nbeams_ss; i++) {
      beam_samples[i] = ping_ptr->png_beam_samples[i];
      start_sample[i] = ping_ptr->png_start_sample[i];
    }
    for (int i = 0; i < ping_ptr->png_npixels; i++)
      ss_pixels[i] = ping_ptr->png_ssraw[i] * 0.5;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       mode:            %d\n", *mode);
    fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
    fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
    fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
    fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
    fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
    fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
    fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
    fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
    fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
    fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
    fprintf(stderr, "dbg2       bso:             %f\n", *bso);
    fprintf(stderr, "dbg2       tx:              %d\n", *tx);
    fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
    fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
    fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
    }
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
    }
    for (int i = 0; i < *npixels; i++) {
      fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return status;
}

/*--------------------------------------------------------------------*/
/*
Method to get fields from raw data, similar to mb_get_all.
*/
int mb_get_raw(int verbose, void *mbio_ptr, int *mode, int *ipulse_length, int *png_count, int *sample_rate, double *absorption,
               int *max_range, int *r_zero, int *r_zero_corr, int *tvg_start, int *tvg_stop, double *bsn, double *bso, int *tx,
               int *tvg_crossover, int *nbeams_ss, int *npixels, int *beam_samples, int *start_sample, int *range,
               double *depression, double *bs, double *ss_pixels, int *error) {
  struct mb_io_struct *mb_io_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
  }

  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  *mode = -1;
  *ipulse_length = 0;
  *png_count = 0;
  *sample_rate = 0;
  *absorption = 0;
  *max_range = 0;
  *r_zero = 0;
  *r_zero_corr = 0;
  *tvg_start = 0;
  *tvg_stop = 0;
  *bsn = 0;
  *bso = 0;
  *tx = 0;
  *tvg_crossover = 0;
  *nbeams_ss = 0;
  *npixels = 0;

  for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
    beam_samples[i] = 0;
    start_sample[i] = 0;
    range[i] = 0;
    depression[i] = 0.0;
    bs[i] = 0.0;
  }

  switch (mb_io_ptr->format) {
  case MBF_EM300MBA:
  case MBF_EM300RAW:
    mb_get_raw_simrad2(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption, max_range, r_zero,
                       r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover, nbeams_ss, npixels, beam_samples,
                       start_sample, range, depression, bs, ss_pixels, error);

    break;
  case MBF_EM710MBA:
  case MBF_EM710RAW:
    mb_get_raw_simrad3(verbose, mbio_ptr, mode, ipulse_length, png_count, sample_rate, absorption, max_range, r_zero,
                       r_zero_corr, tvg_start, tvg_stop, bsn, bso, tx, tvg_crossover, nbeams_ss, npixels, beam_samples,
                       start_sample, range, depression, bs, ss_pixels, error);

    break;
  }

  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBlist function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       mode:            %d\n", *mode);
    fprintf(stderr, "dbg2       ipulse_length:   %d\n", *ipulse_length);
    fprintf(stderr, "dbg2       png_count:       %d\n", *png_count);
    fprintf(stderr, "dbg2       sample_rate:     %d\n", *sample_rate);
    fprintf(stderr, "dbg2       absorption:      %f\n", *absorption);
    fprintf(stderr, "dbg2       max_range:       %d\n", *max_range);
    fprintf(stderr, "dbg2       r_zero:          %d\n", *r_zero);
    fprintf(stderr, "dbg2       r_zero_corr:     %d\n", *r_zero_corr);
    fprintf(stderr, "dbg2       tvg_start:       %d\n", *tvg_start);
    fprintf(stderr, "dbg2       tvg_stop:        %d\n", *tvg_stop);
    fprintf(stderr, "dbg2       bsn:             %f\n", *bsn);
    fprintf(stderr, "dbg2       bso:             %f\n", *bso);
    fprintf(stderr, "dbg2       tx:              %d\n", *tx);
    fprintf(stderr, "dbg2       tvg_crossover:   %d\n", *tvg_crossover);
    fprintf(stderr, "dbg2       nbeams_ss:       %d\n", *nbeams_ss);
    fprintf(stderr, "dbg2       npixels:         %d\n", *npixels);
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d range:%d depression:%f bs:%f\n", i, range[i], depression[i], bs[i]);
    }
    for (int i = 0; i < mb_io_ptr->beams_bath_max; i++) {
      fprintf(stderr, "dbg2       beam:%d samples:%d start:%d\n", i, beam_samples[i], start_sample[i]);
    }
    for (int i = 0; i < *npixels; i++) {
      fprintf(stderr, "dbg2       pixel:%d ss:%f\n", i, ss_pixels[i]);
    }
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return status;
}

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

  mb_path read_file = "datalist.mb-1";
  bool bathy_in_feet = false;

  bool ascii = true;
  bool netcdf = false;
  bool netcdf_cdl = true;
  dump_mode_t dump_mode = DUMP_MODE_LIST;
  beam_set_t beam_set = MBLIST_SET_OFF;
  int pixel_set = MBLIST_SET_OFF;  // TODO(schwehr): Is this really beam_set_t?
  char delimiter[MB_PATH_MAXLINE] = "\t";
  char projection_pars[MB_PATH_MAXLINE] = "";
  bool use_projection = false;
  int decimate = 1;
  int beam_exclude_percent;
  int beam_start;
  int beam_end;
  int pixel_start;
  int pixel_end;
  check_t check_values = MBLIST_CHECK_ON;
  bool check_nav = false;
  char output_file[MB_PATH_MAXLINE] = "-";
  bool segment = false;
  segment_mode_t segment_mode = MBLIST_SEGMENT_MODE_NONE;
  char segment_tag[MB_PATH_MAXLINE] = "";
  mb_path secondary_file = "";
  bool secondary_file_set = false;

  // set up the default list controls
  //   (Time, lon, lat, heading, speed, along-track distance, center beam depth)
  char list[MAX_OPTIONS] = "TXYHSLZ";
  int n_list = 7;

  /* process argument list */
  {
    bool errflg = false;
    bool help = false;
    int c;
    while ((c = getopt(argc, argv, "AaB:b:CcD:d:E:e:F:f:G:g:I:i:J:j:K:k:L:l:M:m:N:n:O:o:P:p:QqR:r:S:s:T:t:U:u:X:x:Y:y:Z:z:VvWwHh")) !=
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
      case 'A':
      case 'a':
        ascii = false;
        netcdf_cdl = false;
        break;
      case 'B':
      case 'b':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &btime_i[5]);
        btime_i[6] = 0;
        break;
      case 'C':
      case 'c':
        netcdf = true;
        break;
      case 'D':
      case 'd':
        {
          int tmp;
          sscanf(optarg, "%d", &tmp);
          // TODO(schwehr): Range check tmp.
          dump_mode = (dump_mode_t)tmp;
          if (dump_mode == DUMP_MODE_BATH)
            beam_set = MBLIST_SET_ALL;
          else if (dump_mode == DUMP_MODE_TOPO)
            beam_set = MBLIST_SET_ALL;
          else if (dump_mode == DUMP_MODE_AMP)
            beam_set = MBLIST_SET_ALL;
          else if (dump_mode == DUMP_MODE_SS)
            pixel_set = MBLIST_SET_ALL;
          break;
        }
      case 'E':
      case 'e':
        sscanf(optarg, "%d/%d/%d/%d/%d/%d", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &etime_i[5]);
        etime_i[6] = 0;
        break;
      case 'G':
      case 'g':
        sscanf(optarg, "%1023s", delimiter);
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
      case 'K':
      case 'k':
        sscanf(optarg, "%d", &decimate);
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
        break;
      case 'M':
      case 'm':
        if (optarg[0] == 'a' || optarg[0] == 'A') {
          beam_set = MBLIST_SET_ALL;
        }
        else if (optarg[0] == 'x' || optarg[0] == 'X') {
          beam_set = MBLIST_SET_EXCLUDE_OUTER;
          sscanf(optarg, "%*c%d", &beam_exclude_percent);
        }
        else {
          sscanf(optarg, "%d/%d", &beam_start, &beam_end);
          beam_set = MBLIST_SET_ON;
        }
        break;
      case 'N':
      case 'n':
        if (optarg[0] == 'a' || optarg[0] == 'A') {
          pixel_set = MBLIST_SET_ALL;
        }
        else {
          sscanf(optarg, "%d/%d", &pixel_start, &pixel_end);
          pixel_set = MBLIST_SET_ON;
        }
        break;
      case 'O':
      case 'o':
        if (strcmp(optarg, "%fnv") == 0 || strcmp(optarg, "%FNV") == 0) {
          strncpy(list, "tMXYHScRPr=X=Y+X+Y", sizeof(list));
          n_list = strlen(list);
        } else if (strlen(optarg) > 0) {
          n_list = MIN(strlen(optarg), MAX_OPTIONS);
          for (int j = 0; j < n_list; j++){
            if (j < MAX_OPTIONS) {
              list[j] = optarg[j];
              if (list[j] == '^')
                use_projection = true;
            }
          }
        }
        break;
      case 'P':
      case 'p':
        sscanf(optarg, "%d", &pings);
        break;
      case 'Q':
      case 'q':
        check_values = MBLIST_CHECK_OFF_RAW;
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
      case 'U':
      case 'u':
        if (optarg[0] == 'N')
          check_nav = true;
        else {
          int tmp;
          sscanf(optarg, "%d", &tmp);
          check_values = (check_t)tmp;
          if (check_values < MBLIST_CHECK_ON || check_values > MBLIST_CHECK_OFF_FLAGNAN) {
            fprintf(stderr, "WARNING: -u/-U: check_values out of range.\n");
            check_values = MBLIST_CHECK_ON;
          }
        }
        break;
      case 'W':
      case 'w':
        bathy_in_feet = true;
        break;
      case 'X':
      case 'x':
        sscanf(optarg, "%1023s", output_file);
        break;
      case 'Y':
      case 'y':
        sscanf(optarg, "%1023s", secondary_file);
        secondary_file_set = true;
        break;
      case 'Z':
      case 'z':
        segment = true;
        sscanf(optarg, "%1023s", segment_tag);
        if (strcmp(segment_tag, "swathfile") == 0)
          segment_mode = MBLIST_SEGMENT_MODE_SWATHFILE;
        else if (strcmp(segment_tag, "datalist") == 0)
          segment_mode = MBLIST_SEGMENT_MODE_DATALIST;
        else
          segment_mode = MBLIST_SEGMENT_MODE_TAG;
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
      fprintf(stderr, "dbg2       pings:          %d\n", pings);
      fprintf(stderr, "dbg2       decimate:       %d\n", decimate);
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
      fprintf(stderr, "dbg2       output_file:    %s\n", output_file);
      fprintf(stderr, "dbg2       ascii:          %d\n", ascii);
      fprintf(stderr, "dbg2       netcdf:         %d\n", netcdf);
      fprintf(stderr, "dbg2       netcdf_cdl:     %d\n", netcdf_cdl);
      fprintf(stderr, "dbg2       segment:        %d\n", segment);
      fprintf(stderr, "dbg2       segment_mode:   %d\n", segment_mode);
      fprintf(stderr, "dbg2       segment_tag:    %s\n", segment_tag);
      fprintf(stderr, "dbg2       delimiter:      %s\n", delimiter);
      fprintf(stderr, "dbg2       beam_set:       %d\n", beam_set);
      fprintf(stderr, "dbg2       beam_start:     %d\n", beam_start);
      fprintf(stderr, "dbg2       beam_end:       %d\n", beam_end);
      fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", beam_exclude_percent);
      fprintf(stderr, "dbg2       pixel_set:      %d\n", pixel_set);
      fprintf(stderr, "dbg2       pixel_start:    %d\n", pixel_start);
      fprintf(stderr, "dbg2       pixel_end:      %d\n", pixel_end);
      fprintf(stderr, "dbg2       dump_mode:      %d\n", dump_mode);
      fprintf(stderr, "dbg2       check_values:   %d\n", check_values);
      fprintf(stderr, "dbg2       check_nav:      %d\n", check_nav);
      fprintf(stderr, "dbg2       use_projection: %d\n", use_projection);
      fprintf(stderr, "dbg2       projection_pars:%s\n", projection_pars);
      fprintf(stderr, "dbg2       secondary_file: %s\n", secondary_file);
      fprintf(stderr, "dbg2       secondary_file_set:%d\n", secondary_file_set);
      fprintf(stderr, "dbg2       n_list:         %d\n", n_list);
      for (int i = 0; i < n_list; i++)
        fprintf(stderr, "dbg2         list[%d]:      %c\n", i, list[i]);
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

  double bathy_scale;
  /* set bathymetry scaling */
  if (bathy_in_feet)
    bathy_scale = 1.0 / 0.3048;
  else
    bathy_scale = 1.0;

  /* if secondary file with time series table specified read up to NUM_SECONDARY_MAX columns */
  if (secondary_file_set) {
    FILE *sfp = fopen(secondary_file, "r");
    if (sfp != nullptr) {
      double dd[SECONDARY_FILE_COLUMNS_MAX];
      mb_path buffer;
      while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
        if (buffer[0] != '#') {
          int num_read = sscanf(buffer, 
            "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            &dd[0], &dd[1], &dd[2], &dd[3], &dd[4], &dd[5], &dd[6], &dd[7], &dd[8], &dd[9], 
            &dd[10], &dd[11], &dd[12], &dd[13], &dd[14], &dd[15], &dd[16], &dd[17], &dd[18], &dd[19]);
          num_secondary_columns = MAX(num_secondary_columns, num_read);
        }
      }
      rewind(sfp);
      while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
        if (buffer[0] != '#') {
          int num_read = sscanf(buffer, 
            "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            &dd[0], &dd[1], &dd[2], &dd[3], &dd[4], &dd[5], &dd[6], &dd[7], &dd[8], &dd[9], 
            &dd[10], &dd[11], &dd[12], &dd[13], &dd[14], &dd[15], &dd[16], &dd[17], &dd[18], &dd[19]);
          if (num_read == num_secondary_columns)
            num_secondary_alloc++;
        }
      }
      rewind(sfp);
      status = mb_mallocd(verbose, __FILE__, __LINE__, 
                          num_secondary_alloc * sizeof(double), 
                          (void **)&secondary_time_d, &error);
      status = mb_mallocd(verbose, __FILE__, __LINE__, 
                          num_secondary_alloc * (num_secondary_columns - 1) * sizeof(double), 
                          (void **)&secondary_data, &error);
      while (fgets(buffer, sizeof(mb_path), sfp) != NULL) {
        if (buffer[0] != '#') {
          int num_read = sscanf(buffer, 
            "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
            &dd[0], &dd[1], &dd[2], &dd[3], &dd[4], &dd[5], &dd[6], &dd[7], &dd[8], &dd[9], 
            &dd[10], &dd[11], &dd[12], &dd[13], &dd[14], &dd[15], &dd[16], &dd[17], &dd[18], &dd[19]);
          if (num_read == num_secondary_columns) {
            secondary_time_d[num_secondary] = dd[0];
            for (int i = 1; i < num_secondary_columns; i++) {
              int j = (i - 1) * num_secondary_alloc + num_secondary;
              secondary_data[j] = dd[i];
            }
            num_secondary++;
          }
        }
      }
      fclose(sfp);
    }
    else {
      fprintf(stderr, "\nUnable to open data secondary file: %s\n", secondary_file);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }
  }

  /* determine whether to read one file or a list of files */
  const bool read_datalist = format < 0;
  bool read_data;
  void *datalist;
  char path[MB_PATH_MAXLINE] = "";
  char ppath[MB_PATH_MAXLINE] = "";
  char apath[MB_PATH_MAXLINE] = "";
  char dpath[MB_PATH_MAXLINE] = "";
  double file_weight;
  int pstatus;
  int astatus;

  /* open file list */
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

  double btime_d;
  double etime_d;
  int beams_bath;
  int beams_amp;
  int pixels_ss;

  /* output format list controls */
  int beam_vertical = 0;
  int pixel_vertical = 0;
  int beam_status = MB_SUCCESS;
  int pixel_status = MB_SUCCESS;
  int time_j[5];
  bool use_bath = false;
  bool use_amp = false;
  bool use_ss = false;
  bool use_slope = false;
  bool use_attitude = false;
  bool use_gains = false;
  bool use_detects = true;
  bool use_pingnumber = false;
  bool use_linenumber = false;
  bool check_bath = false;
  bool check_amp = false;
  bool check_ss = false;
  bool signflip_next_value = false;
  bool raw_next_value = false;
  bool port_next_value = false;
  bool stbd_next_value = false;
  bool sensornav_next_value = false;
  bool sensorrelative_next_value = false;
  bool projectednav_next_value = false;
  bool use_raw = false;
  bool special_character = false;

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
  unsigned int pingnumber;
  unsigned int linenumber;

  /* additional time variables */
  bool first_m = true;
  bool first_u = true;
  time_t time_u;
  time_t time_u_ref;
  double seconds;

  /* crosstrack slope values */
  double avgslope;
  double sx, sy, sxx, sxy;
  int ns;
  double angle, depth, slope;
  int ndepths;
  double *depths = nullptr;
  double *depthacrosstrack = nullptr;
  int nslopes;
  double *slopes = nullptr;
  double *slopeacrosstrack = nullptr;

  /* course calculation variables */
  double course, course_old;
  double time_d_old, dt;
  double time_interval;
  double speed_made_good, speed_made_good_old;
  double navlon_old, navlat_old;
  double dx, dy, dist;
  double delta, b;
  double dlon, dlat, minutes;
  int degrees;
  char hemi;
  double headingx, headingy, mtodeglon, mtodeglat;

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

  /* raw data values */
  int count = 0;
  int invert;
  int flip;
  int mode;
  int ipulse_length;
  int png_count;
  int sample_rate;
  double absorption;
  int max_range;
  int r_zero;
  int r_zero_corr;
  int tvg_start;
  int tvg_stop;
  double bsn;
  double bso;
  double mback;
  int nback;
  int tx;
  int tvg_crossover;
  int nbeams_ss;
  int npixels;
  int *beam_samples = nullptr;
  int *range = nullptr;
  int *start_sample = nullptr;
  double *depression = nullptr;
  double *bs = nullptr;
  double *ss_pixels = nullptr;
  double transmit_gain;
  double pulse_length;
  double receive_gain;
  double dsecondary = 0.0;

  int nbeams;

  char output_file_temp[2*MB_PATH_MAXLINE+20] = "";

  /* netcdf variables */
  int lcount = 0;

  /* set the initial along track distance here so */
  /* it is cumulative over multiple files */
  double distance_total = 0.0;

  /* initialize output files */
  FILE **output;
  status = mb_mallocd(verbose, __FILE__, __LINE__, n_list * sizeof(FILE *), (void **)&output, &error);

  bool invert_next_value = false;

  FILE *outfile;
  if (!netcdf) {
    if (0 == strncmp("-", output_file, 2))
      outfile = stdout;
    else
      outfile = fopen(output_file, "w");
    if (nullptr == outfile) {
      fprintf(stderr, "Could not open file: %s\n", output_file);
      exit(1);
    }

    /* for non netcdf all output goes to the same file */
    for (int i = 0; i < n_list; i++)
      output[i] = outfile;
  }
  else {
    /* netcdf must be ascii and must not be segmented */
    ascii = true;
    segment = false;

    /* open CDL file */
    if (0 == strncmp("-", output_file, 2) && !netcdf_cdl)
      strcpy(output_file, "mblist.nc");
    if (0 == strncmp("-", output_file, 2)) {
      outfile = stdout;
    }
    else {
      strncpy(output_file_temp, output_file, MB_PATH_MAXLINE - 5);
      if (!netcdf_cdl)
        strcat(output_file_temp, ".cdl");
      outfile = fopen(output_file_temp, "w+");
      if (outfile == nullptr) {
        fprintf(stderr, "Unable to open file: %s\n", output_file_temp);
        exit(1);
      }
    }

    /* output CDL headers */
    fprintf(outfile, "netcdf mlist {\n\n\t// ");
    for (int i = 0; i < argc; i++)
      fprintf(outfile, "%s ", argv[i]);
    fprintf(outfile, "\n");
    fprintf(outfile, "dimensions:\n\ttimestring = 26, timestring_J = 24, timestring_j = 23, \n\t");
    fprintf(outfile, "timefields_J = 6,  timefields_j = 5, timefields_t = 7, latm = 13, \n\t");

    /* find dimensions in format list */
    raw_next_value = false;
    for (int i = 0; i < n_list; i++)
      if (list[i] == '/' || list[i] == '-' || list[i] == '=' || list[i] == '+') {
        // ignore
      }
      else if (!raw_next_value) {
        if (list[i] == '.')
          raw_next_value = true;
      }
      else if (list[i] >= '0' && list[i] <= '9')
        count = count * 10 + list[i] - '0';
      else {
        raw_next_value = false;
        if (count > 0) {
          fprintf(outfile, "%c = %d,  ", list[i], count);
          count = 0;
        }
      }

    fprintf(outfile, "\n\tdata = unlimited ;\n\n");
    fprintf(outfile, "variables:\n\t");
    fprintf(outfile, ":command_line = \"");
    for (int i = 0; i < argc; i++)
      fprintf(outfile, "%s ", argv[i]);
    fprintf(outfile, "\";\n");
    fprintf(outfile, "\t:mbsystem_version = \"%s\";\n", MB_VERSION);

    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
    fprintf(outfile, "\t:run = \"by <%s> on cpu <%s> at <%s>\";\n\n", user, host, date);

    /* get temporary output file for each variable */
    for (int i = 0; i < n_list; i++) {
      output[i] = tmpfile();
      if (output[i] == nullptr) {
        fprintf(stderr, "Unable to open temp files\n");
        exit(1);
      }

      char variable[MB_PATH_MAXLINE] = "";  // TODO(schwehr): Localize to all the use sites.
      if (!raw_next_value) {
        switch (list[i]) {
        case '/': /* Inverts next simple value */
          invert_next_value = true;
          break;

        case '-': /* Flip sign on next simple value */
          signflip_next_value = true;
          break;

        case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
          sensornav_next_value = true;
          break;

        case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
          sensorrelative_next_value = true;
          break;

        case '^': /* Print position values in projected coordinates
                   * - easting northing rather than lon lat
                   * - applies to XY */
          projectednav_next_value = true;
          break;

        case '.': /* Raw value next field */
          raw_next_value = true;
          break;

        case '=': /* Port-most value next field -ignored here */
          break;

        case '+': /* Starboard-most value next field - ignored here*/
          break;

        case 'A': /* Average seafloor crosstrack slope */
          strcpy(variable, "aslope");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Average seafloor crosstrack slope\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "tangent of angle from seafloor to vertical\";\n");
          else
            fprintf(outfile, "tangent of angle from seafloor to horizontal\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'a': /* Per-beam seafloor crosstrack slope */
          strcpy(variable, "bslope");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Per-beam seafloor crosstrack slope\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "tangent of angle from seafloor to vertical\";\n");
          else
            fprintf(outfile, "tangent of angle from seafloor to horizontal\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'B': /* amplitude */
          strcpy(variable, "amplitude");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Amplitude\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (format == MBF_EM300RAW || format == MBF_EM300MBA)
            fprintf(outfile, "dB + 64\";\n");
          else
            fprintf(outfile, "backscatter\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'b': /* sidescan */
          strcpy(variable, "sidescan");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"sidescan\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (format == MBF_EM300RAW || format == MBF_EM300MBA)
            fprintf(outfile, "dB + 64\";\n");
          else
            fprintf(outfile, "backscatter\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'C': /* Sonar altitude (m) */
          strcpy(variable, "altitude");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sonar altitude\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'c': /* Sonar transducer depth (m) */
          strcpy(variable, "transducer");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sonar transducer depth\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'D': /* acrosstrack dist. */
        case 'd':
          strcpy(variable, "acrosstrack");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Acrosstrack distance\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (bathy_in_feet)
            fprintf(outfile, "f\";\n");
          else
            fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'E': /* alongtrack dist. */
        case 'e':
          strcpy(variable, "alongtrack");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (bathy_in_feet)
            fprintf(outfile, "f\";\n");
          else
            fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'F': /* beamflag (numeric only for netcdf) */
        case 'f':
          strcpy(variable, "beamflag");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Beamflag\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (bathy_in_feet)
            fprintf(outfile, "f\";\n");
          else
            fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'G': /* flat bottom grazing angle */
          strcpy(variable, "flatgrazing");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Flat bottom grazing angle\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "tangent of angle from beam to vertical\";\n");
          else
            fprintf(outfile, "tangent of angle from beam to horizontal\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'g': /* grazing angle using slope */
          strcpy(variable, "grazing");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Grazing angle using slope\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "tangent of angle from beam to perpendicular to seafloor\";\n");
          else
            fprintf(outfile, "tangent of angle from beam to seafloor\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'H': /* heading */
          strcpy(variable, "heading");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Heading\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees true\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'h': /* course */
          strcpy(variable, "course");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Course\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees true\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'J': /* time string */
          strcpy(variable, "time_J");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data,timefields_J);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day hour minute seconds\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "year, julian day, hour, minute, second, nanosecond\";\n");
          break;

        case 'j': /* time string */
          strcpy(variable, "time_j");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data,timefields_j);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Time - year julian_day minute seconds\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "year, julian day, minute, second, nanosecond\";\n");
          break;

        case 'K': /* proportion of non-null beams that are unflagged */
          strcpy(variable, "goodbeamfraction");
          fprintf(output[i], "\t%s = ", variable);
          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Good beam fraction of non-null beams\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "number of good beams divided by number of non-null beams\";\n");
          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'k': /* proportion of all possible beams that are unflagged */
          strcpy(variable, "goodbeamfractionall");
          fprintf(output[i], "\t%s = ", variable);
          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Good beam fraction of all possible beams\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "number of good beams divided by number of possible beams\";\n");
          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'L': /* along-track distance (km) */
          strcpy(variable, "along_track");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "km\";\n");

          signflip_next_value = false;
          invert_next_value = false;

          break;

        case 'l': /* along-track distance (m) */
          strcpy(variable, "along_track_m");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Alongtrack distance\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'M': /* Decimal unix seconds since
                      1/1/70 00:00:00 */
          strcpy(variable, "unix_time");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tdouble %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Seconds since 1/1/70 00:00:00\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "s\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'm': /* time in decimal seconds since
                      first record */

          strcpy(variable, "survey_time");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tdouble %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Seconds since first record\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "s\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'N': /* ping counter */
          strcpy(variable, "ping");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Ping counter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "pings\";\n");
          break;

        case 'P': /* pitch */
          strcpy(variable, "pitch");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Pitch\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees from horizontal\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'p': /* draft */
          strcpy(variable, "draft");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Draft\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'q': /* bottom detect type */
        case 'Q': /* bottom detect type */
          strcpy(variable, "bottom_detect_type");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Bottom detect type\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "0=unknown,1=amplitude,2=phase\";\n");
          break;

        case 'R': /* roll */
          strcpy(variable, "roll");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Roll\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees from horizontal\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'r': /* heave */
          strcpy(variable, "heave");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Heave\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'S': /* speed */
          strcpy(variable, "speed");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Speed\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "km/hr\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 's': /* speed made good */
          strcpy(variable, "speed_made_good");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Speed made good\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "km/hr\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
          strcpy(variable, "time_T");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tchar %s(data,timestring);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Time string - year/month/day/hour/minute/seconds\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "yyyy/MM/dd/hh/mm/ss.ssssss\";\n");
          break;

        case 't': /* yyyy mm dd hh mm ss time string */
          strcpy(variable, "time_t");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data,timefields_t);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Time - year month day hour minute seconds\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "year, month, day, hour, minute, second, nanosecond\";\n");
          break;

        case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
          strcpy(variable, "unix_time_s");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since 1/1/70 00:00:00\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "s\";\n");
          break;

        case 'u': /* time in seconds since first record */
          strcpy(variable, "survey_time_s");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Integer seconds since first record\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "s\";\n");
          break;

        case 'V': /* time in seconds since last ping */
        case 'v':
          strcpy(variable, "ping_time");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Seconds since last ping\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "s\";\n");
          break;

        case 'X': /* longitude decimal degrees */
          strcpy(variable, "longitude");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tdouble %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Longitude\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'x': /* longitude degrees + decimal minutes */
          strcpy(variable, "longitude_minutes");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tchar %s(data,latm);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Longitude - decimal minutes\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "ddd mm.mmmmmH\";\n");
          break;

        case 'Y': /* latitude decimal degrees */
          strcpy(variable, "latitude");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tdouble %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Latitude\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'y': /* latitude degrees + decimal minutes */
          strcpy(variable, "latitude_minutes");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tchar %s(data,latm);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Latitude - decimal minutes\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "ddd mm.mmmmmH\";\n");
          break;

        case 'Z': /* topography */
          strcpy(variable, "topography");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Topography\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (bathy_in_feet)
            fprintf(outfile, "f\";\n");
          else
            fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case 'z': /* depth */
          strcpy(variable, "depth");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Depth\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (bathy_in_feet)
            fprintf(outfile, "f\";\n");
          else
            fprintf(outfile, "m\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          break;

        case '#': /* beam number */
          strcpy(variable, "beam");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Beam number\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "number\";\n");
          break;
        }
      }
      else {
        switch (list[i]) {
        case '/': /* Inverts next simple value */
          invert_next_value = true;
          break;

        case '-': /* Flip sign on next simple value */
          signflip_next_value = true;
          break;

        case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
          sensornav_next_value = true;
          break;

        case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
          sensorrelative_next_value = true;
          break;

        case '^': /* Print position values in projected coordinates
                   * - easting northing rather than lon lat
                   * - applies to XY */
          projectednav_next_value = true;
          break;

        case '.': /* Raw value next field */
          raw_next_value = true;
          count = 0;
          break;

        case '=': /* Port-most value next field -ignored here */
          break;

        case '+': /* Starboard-most value next field - ignored here*/
          break;

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
          count = count * 10 + list[i] - '0';
          break;

        case 'A': /* backscatter */
          strcpy(variable, "backscatter");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Backscatter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'a': /* absorption */
          strcpy(variable, "absorption");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Mean absorption\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB/km\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'B': /* BSN - Normal incidence backscatter */
          strcpy(variable, "bsn");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Normal incidence backscatter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'b': /* BSO - Oblique backscatter */
          strcpy(variable, "bso");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Oblique backscatter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'c': /* mean backscatter */
          strcpy(variable, "mback");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Mean backscatter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          if (format == MBF_EM300RAW || format == MBF_EM300MBA)
            fprintf(outfile, "dB + 64\";\n");
          else
            fprintf(outfile, "backscatter\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'd': /* beam depression angle */
          strcpy(variable, "depression");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Beam depression angle\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "degrees\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'F': /* filename */
          strcpy(variable, "filename");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tchar %s(data,pathsize);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Name of swath data file\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "file name\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'f': /* format */
          strcpy(variable, "format");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tshort %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"MBsystem file format number\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);

          fprintf(outfile, "see mbformat\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'G': /* TVG start */
          strcpy(variable, "tvg_start");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Start range of TVG ramp\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "samples\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'g': /* TVG stop */
          strcpy(variable, "tvg_stop");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Stop range of TVG ramp\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "samples\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'L': /* Pulse length */
          strcpy(variable, "pulse_length");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Pulse Length\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "us");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'l': /* Transmit pulse length */
          strcpy(variable, "pulse_length");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Pulse length\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "seconds\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'M': /* Mode */
          strcpy(variable, "mode");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sounder mode\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "0=very shallow,1=shallow,2=medium,3=deep,4=very deep,5=extra deep\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'N': /* Ping number */
          strcpy(variable, "ping_no");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tlong %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sounder ping counter\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          fprintf(outfile, "pings\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'p': /* sidescan pixel */
          strcpy(variable, "sidescan");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          if (count == 0)
            fprintf(outfile, "\tfloat %s(data);\n", variable);
          else
            fprintf(outfile, "\tfloat %s(data, %c);\n", variable, list[i]);

          fprintf(outfile, "\t\t%s:long_name = \"Raw sidescan pixels\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'R': /* range */
          strcpy(variable, "range");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Range \";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "samples\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'r': /* Sample rate */
          strcpy(variable, "sample_rate");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sample Rate\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "Hertz\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'S': /* Sidescan pixels */
          strcpy(variable, "pixels");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Total sidescan pixels \";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "pixels\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 's': /* Sidescan pixels per beam */
          strcpy(variable, "beam_pixels");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Sidescan pixels per beam\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "pixels\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 'T': /* Transmit gain */
          strcpy(variable, "transmit_gain");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Transmit gain\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        case 't': /* Receive gain */
          strcpy(variable, "receive_gain");
          if (signflip_next_value)
            strcat(variable, "-");
          if (invert_next_value)
            strcat(variable, "_");

          fprintf(output[i], "\t%s = ", variable);

          fprintf(outfile, "\tfloat %s(data);\n", variable);
          fprintf(outfile, "\t\t%s:long_name = \"Receive gain\";\n", variable);
          fprintf(outfile, "\t\t%s:units = \"", variable);
          if (signflip_next_value)
            fprintf(outfile, "-");
          if (invert_next_value)
            fprintf(outfile, "1/");
          fprintf(outfile, "dB\";\n");

          signflip_next_value = false;
          invert_next_value = false;
          raw_next_value = false;
          break;

        default:
          raw_next_value = false;
          break;
        }
      }
    }
    fprintf(outfile, "\n\ndata:\n");
  }

  bool use_course = false;
  bool use_time_interval = false;
  bool use_swathbounds = false;
  double time_d_ref = 0;

  /* loop over all files to be read */
  while (read_data) {

    /* initialize reading the swath file */
    if (mb_read_init_altnav(verbose, path, format, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, astatus, apath, &mbio_ptr,
                               &btime_d, &etime_d, &beams_bath, &beams_amp, &pixels_ss, &error) != MB_SUCCESS) {
      char *message;
      mb_error(verbose, error, &message);
      fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
      fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", path);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(error);
    }

    /* figure out whether bath, amp, or ss will be used */
    if (dump_mode == DUMP_MODE_BATH || dump_mode == DUMP_MODE_TOPO)
      use_bath = true;
    else if (dump_mode == DUMP_MODE_AMP)
      use_amp = true;
    else if (dump_mode == DUMP_MODE_SS)
      use_ss = true;
    else
      for (int i = 0; i < n_list; i++) {
        if (!raw_next_value) {
          // TODO(schwehr): Why not a switch?
          if (list[i] == 'Z' || list[i] == 'z' || list[i] == 'A' || list[i] == 'a' || list[i] == 'Q' || list[i] == 'q')
            use_bath = true;
          if (list[i] == 'B')
            use_amp = true;
          if (list[i] == 'b')
            use_ss = true;
          if (list[i] == 'h')
            use_course = true;
          if (list[i] == 's')
            use_course = true;
          if (list[i] == 'V' || list[i] == 'v')
            use_time_interval = true;
          if (list[i] == 'A' || list[i] == 'a' || list[i] == 'G' || list[i] == 'g')
            use_slope = true;
          if (list[i] == 'P' || list[i] == 'p' || list[i] == 'R' || list[i] == 'r')
            use_attitude = true;
          if (list[i] == 'Q' || list[i] == 'q')
            use_detects = true;
          if (list[i] == 'N')
            use_pingnumber = true;
          if (list[i] == 'n')
            use_linenumber = true;
          if (list[i] == '.')
            raw_next_value = true;
          if (list[i] == '=')
            use_swathbounds = true;
          if (list[i] == '+')
            use_swathbounds = true;
        }
        else {
          if (list[i] == 'T' || list[i] == 't' || list[i] == 'U' || list[i] == 'l')
            use_gains = true;
          else if (list[i] == 'F' || list[i] == 'f')
            ; // ignore
          else {
            use_raw = true;
            if (list[i] == 'R' || list[i] == 'd')
              use_bath = true;
            if (list[i] == 'B' || list[i] == 'b' || list[i] == 'c')
              use_amp = true;
          }
          if (list[i] != '/' && list[i] != '-' && list[i] != '.')
            raw_next_value = false;
        }
      }
    if (check_values == MBLIST_CHECK_ON || check_values == MBLIST_CHECK_ON_NULL) {
      if (use_bath)
        check_bath = true;
      if (use_amp)
        check_amp = true;
      if (use_ss)
        check_ss = true;
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
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depths, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */
          mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depthacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopes, &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(double), (void **)&slopeacrosstrack,
                                 &error);
    if (error == MB_ERROR_NO_ERROR)
      /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, 2 * sizeof(int), (void **)&detect, &error);
    if (use_raw) {
      if (error == MB_ERROR_NO_ERROR)
        /* status &= */
            mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&beam_samples, &error);
      if (error == MB_ERROR_NO_ERROR)
        /* status &= */
            mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&start_sample, &error);
      if (error == MB_ERROR_NO_ERROR)
        /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(int), (void **)&range, &error);
      if (error == MB_ERROR_NO_ERROR)
        /* status &= */
            mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&depression, &error);
      if (error == MB_ERROR_NO_ERROR)
        /* status &= */ mb_register_array(verbose, mbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&bs, &error);
      /* status &= */ mb_mallocd(verbose, __FILE__, __LINE__, (MBSYS_SIMRAD2_MAXRAWPIXELS) * sizeof(double), (void **)&ss_pixels,
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

    /* output separator for GMT style segment file output */
    if (segment && ascii && !netcdf) {
      if (segment_mode == MBLIST_SEGMENT_MODE_TAG)
        fprintf(output[0], "%s\n", segment_tag);
      else if (segment_mode == MBLIST_SEGMENT_MODE_SWATHFILE)
        fprintf(output[0], "# %s\n", path);
      else if (segment_mode == MBLIST_SEGMENT_MODE_DATALIST)
        fprintf(output[0], "# %s\n", dpath);
    }

    /* read and print data */
    int nread = 0;
    bool first = true;
    while (error <= MB_ERROR_NO_ERROR) {
      /* reset error */
      error = MB_ERROR_NO_ERROR;

      /* read a ping of data */
      if (pings == 1 || use_attitude || use_detects || use_pingnumber || use_linenumber) {
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
        if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
           /* get attitude using mb_extract_nav(), but do not overwrite the navigation that 
            may derive from an alternative navigation source */
          double tnavlon, tnavlat, tspeed, theading;
          status = mb_extract_nav(verbose, mbio_ptr, store_ptr, &kind, time_i, &time_d, &tnavlon, &tnavlat,
                                  &tspeed, &theading, &draft, &roll, &pitch, &heave, &error);
        }

        /* if survey data extract detects */
        if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_detects) {
          nbeams = beams_bath;
          status = mb_detects(verbose, mbio_ptr, store_ptr, &kind, &nbeams, detect, &error);
        }

        /* if survey data extract pingnumber */
        if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_pingnumber)
          status = mb_pingnumber(verbose, mbio_ptr, &pingnumber, &error);
        if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && use_linenumber) {
          unsigned int cdpnumber;
          status = mb_segynumber(verbose, mbio_ptr, &linenumber, &pingnumber, &cdpnumber, &error);
        }
      }
      else {
        status = mb_get(verbose, mbio_ptr, &kind, &pings_read, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                        &distance, &altitude, &sensordepth, &beams_bath, &beams_amp, &pixels_ss, beamflag, bath, amp,
                        bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, &error);

        /* time gaps are not a problem here */
        if (error == MB_ERROR_TIME_GAP) {
          error = MB_ERROR_NO_ERROR;
          status = MB_SUCCESS;
        }
      }

      /* make sure non survey data records are ignored */
      if (error == MB_ERROR_NO_ERROR && kind != MB_DATA_DATA)
        error = MB_ERROR_OTHER;

      /* increment counter and set cumulative distance */
      if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
        nread++;
        if (!use_pingnumber)
          pingnumber = nread;
        distance_total += distance;
      }

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

      /* count beams */
      int beams_null = 0;
      int beams_flagged = 0;
      int beams_unflagged = 0;
      double goodbeamfraction = 0.0;
      for (int k = 0; k < beams_bath; k++) {
        if (mb_beam_ok(beamflag[k])) {
          beams_unflagged++;
        } else if (beamflag[k] == MB_FLAG_NULL) {
          beams_null++;
        } else {
          beams_flagged++;
        }
      }

      /* set output beams and pixels */
      if (error == MB_ERROR_NO_ERROR) {
        /* find vertical-most non-null beam (the nadir beam)
            and port and starboard-most good beams */
        status = mb_swathbounds(verbose, true, beams_bath, pixels_ss,
                                beamflag, bathacrosstrack,
                                ss, ssacrosstrack,
                                &beam_port, &beam_vertical, &beam_stbd,
                                &pixel_port, &pixel_vertical, &pixel_stbd, &error);

        /* set and/or check beams and pixels to be output */
        status &= set_output(verbose, beams_bath, beams_amp, pixels_ss, use_bath, use_amp, use_ss, dump_mode, beam_set,
                            pixel_set, beam_vertical, pixel_vertical, &beam_start, &beam_end, &beam_exclude_percent,
                            &pixel_start, &pixel_end, &n_list, list, &error);

        if (status == MB_FAILURE) {
          fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
          exit(error);
        }

        if (verbose >= 2) {
          fprintf(stderr, "\ndbg2  Beams set for output in <%s>\n", program_name);
          fprintf(stderr, "dbg2       status:       %d\n", status);
          fprintf(stderr, "dbg2       error:        %d\n", error);
          fprintf(stderr, "dbg2       use_bath:     %d\n", use_bath);
          fprintf(stderr, "dbg2       use_amp:      %d\n", use_amp);
          fprintf(stderr, "dbg2       use_ss:       %d\n", use_ss);
          fprintf(stderr, "dbg2       beam_start:   %d\n", beam_start);
          fprintf(stderr, "dbg2       beam_end:     %d\n", beam_end);
          fprintf(stderr, "dbg2       beam_exclude_percent: %d\n", beam_exclude_percent);
          fprintf(stderr, "dbg2       pixel_start:  %d\n", pixel_start);
          fprintf(stderr, "dbg2       pixel_end:    %d\n", pixel_end);
          fprintf(stderr, "dbg2       check_values: %d\n", check_values);
          fprintf(stderr, "dbg2       check_bath:   %d\n", check_bath);
          fprintf(stderr, "dbg2       check_amp:    %d\n", check_amp);
          fprintf(stderr, "dbg2       check_ss:     %d\n", check_ss);
          fprintf(stderr, "dbg2       n_list:       %d\n", n_list);
          for (int i = 0; i < n_list; i++)
            fprintf(stderr, "dbg2       list[%d]:      %c\n", i, list[i]);
        }
      }

      /* get factors for lon lat calculations */
      if (error == MB_ERROR_NO_ERROR) {
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        headingx = sin(DTR * heading);
        headingy = cos(DTR * heading);
      }

      /* get time interval since last ping */
      if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA && first) {
        time_interval = 0.0;
      }
      else if (error == MB_ERROR_NO_ERROR && kind == MB_DATA_DATA) {
        time_interval = time_d - time_d_old;
      }

      /* calculate course made good */
      if (error == MB_ERROR_NO_ERROR && use_course) {
        if (first) {
          course = heading;
          speed_made_good = speed;
          course_old = heading;
          speed_made_good_old = speed;
        }
        else {
          dx = (navlon - navlon_old) / mtodeglon;
          dy = (navlat - navlat_old) / mtodeglat;
          dist = sqrt(dx * dx + dy * dy);
          if (dist > 0.0)
            course = RTD * atan2(dx / dist, dy / dist);
          else
            course = course_old;
          if (course < 0.0)
            course = course + 360.0;
          dt = (time_d - time_d_old);
          if (dt > 0.0)
            speed_made_good = 3.6 * dist / dt;
          else
            speed_made_good = speed_made_good_old;
        }
      }

      /* calculate slopes if required */
      if (error == MB_ERROR_NO_ERROR && use_slope) {
        /* get average slope */
        ns = 0;
        sx = 0.0;
        sy = 0.0;
        sxx = 0.0;
        sxy = 0.0;
        for (int k = 0; k < beams_bath; k++)
          if (mb_beam_ok(beamflag[k])) {
            sx += bathacrosstrack[k];
            sy += bath[k];
            sxx += bathacrosstrack[k] * bathacrosstrack[k];
            sxy += bathacrosstrack[k] * bath[k];
            ns++;
          }
        if (ns > 0) {
          delta = ns * sxx - sx * sx;
          /* a = (sxx*sy - sx*sxy)/delta; */
          b = (ns * sxy - sx * sy) / delta;
          avgslope = RTD * atan(b);
        }
        else
          avgslope = 0.0;

        /* get per beam slope */
        set_bathyslope(verbose, beams_bath, beamflag, bath, bathacrosstrack, &ndepths, depths, depthacrosstrack, &nslopes,
                       slopes, slopeacrosstrack, &error);
      }

      /* reset old values */
      if (error == MB_ERROR_NO_ERROR) {
        navlon_old = navlon;
        navlat_old = navlat;
        course_old = course;
        speed_made_good_old = speed_made_good;
        time_d_old = time_d;
      }

      /* get raw values if required */
      if (error == MB_ERROR_NO_ERROR && use_raw) {
        status = mb_get_raw(verbose, mbio_ptr, &mode, &ipulse_length, &png_count, &sample_rate, &absorption, &max_range,
                            &r_zero, &r_zero_corr, &tvg_start, &tvg_stop, &bsn, &bso, &tx, &tvg_crossover, &nbeams_ss,
                            &npixels, beam_samples, start_sample, range, depression, bs, ss_pixels, &error);
      }

      /* get gains values if required */
      if (error == MB_ERROR_NO_ERROR && use_gains) {
        status = mb_gains(verbose, mbio_ptr, store_ptr, &kind, &transmit_gain, &pulse_length, &receive_gain, &error);
      }

      /* now loop over beams */
      if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0)
        for (int j = beam_start; j <= beam_end; j++) {
          /* check beam status */
          beam_status = MB_SUCCESS;
          if (check_bath && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j]))
            beam_status = MB_FAILURE;
          else if (check_bath && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL)
            beam_status = MB_FAILURE;
          if (check_amp && check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[j]))
            beam_status = MB_FAILURE;
          else if (check_amp && check_values == MBLIST_CHECK_ON_NULL && beamflag[j] == MB_FLAG_NULL)
            beam_status = MB_FAILURE;
          if (check_ss && j != beam_vertical)
            beam_status = MB_FAILURE;
          else if (check_ss && j == beam_vertical)
            if (ss[pixel_vertical] <= MB_SIDESCAN_NULL)
              beam_status = MB_FAILURE;
          if (use_time_interval && first)
            beam_status = MB_FAILURE;
          if (check_nav && (navlon == 0.0 || navlat == 0.0))
            beam_status = MB_FAILURE;

          /* print out good beams */
          if (beam_status == MB_SUCCESS) {
            signflip_next_value = false;
            invert_next_value = false;
            raw_next_value = false;
            sensornav_next_value = false;
            sensorrelative_next_value = false;
            projectednav_next_value = false;
            special_character = false;
            for (int i = 0; i < n_list; i++) {
              if (netcdf && lcount > 0)
                fprintf(output[i], ", ");
              int k;
              if (port_next_value) {
                k = beam_port;
                port_next_value = false;
              }
              else if (stbd_next_value) {
                k = beam_stbd;
                stbd_next_value = false;
              }
              else
                k = j;

              if (!raw_next_value) {
                switch (list[i]) {
                case '/': /* Inverts next simple value */
                  invert_next_value = true;
                  special_character = true;
                  break;
                case '-': /* Flip sign on next simple value */
                  signflip_next_value = true;
                  special_character = true;
                  break;
                case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
                  sensornav_next_value = true;
                  special_character = true;
                  break;
                case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
                  sensorrelative_next_value = true;
                  special_character = true;
                  break;
                case '^': /* Print position values in projected coordinates
                           * - easting northing rather than lon lat
                           * - applies to XY */
                  projectednav_next_value = true;
                  special_character = true;
                  break;
                case '.': /* Raw value next field */
                  raw_next_value = true;
                  special_character = true;
                  count = 0;
                  break;
                case '=': /* Port-most value next field -ignored here */
                  port_next_value = true;
                  special_character = true;
                  break;
                case '+': /* Starboard-most value next field - ignored here*/
                  stbd_next_value = true;
                  special_character = true;
                  break;
                case 'A': /* Average seafloor crosstrack slope */
                  printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'a': /* Per-beam seafloor crosstrack slope */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                            slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
                    printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'B': /* amplitude */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    printsimplevalue(verbose, output[i], amp[k], 0, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'b': /* sidescan */
                  printsimplevalue(verbose, output[i], ss[pixel_vertical], 0, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'C': /* Sonar altitude (m) */
                  printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'c': /* Sonar transducer depth (m) */
                  printsimplevalue(verbose, output[i], sensordepth, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'D': /* acrosstrack dist. */
                case 'd':
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = bathy_scale * bathacrosstrack[k];
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'E': /* alongtrack dist. */
                case 'e':
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = bathy_scale * bathalongtrack[k];
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'F': /* Beamflag numeric value */
                  if (ascii) {
                    // TODO(schwehr): Bug?
                    // if (netcdf)
                      fprintf(output[i], "%d", beamflag[k]);
                    // else
                    //  fprintf(output[i], "%u", beamflag[k]);
                  }
                  else {
                    b = beamflag[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'f': /* Beamflag character value (ascii only) */
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d", beamflag[k]);
                    else {
                      if (mb_beam_check_flag_unusable(beamflag[k]))
                        fprintf(output[i], "-");
                      else if (mb_beam_ok(beamflag[k]))
                        fprintf(output[i], "G");
                      else if (mb_beam_check_flag_manual(beamflag[k]))
                        fprintf(output[i], "M");
                      else if (mb_beam_check_flag_filter(beamflag[k]))
                        fprintf(output[i], "F");
                      else if (mb_beam_check_flag_filter2(beamflag[k]))
                        fprintf(output[i], "F");
                      else if (mb_beam_check_flag_multipick(beamflag[k]))
                        fprintf(output[i], "N");
                      else if (mb_beam_check_flag_interpolate(beamflag[k]))
                        fprintf(output[i], "I");
                      else if (mb_beam_check_flag_sonar(beamflag[k]))
                        fprintf(output[i], "S");
                    }
                  }
                  else {
                    b = beamflag[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'G': /* flat bottom grazing angle */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    angle = RTD * (atan(bathacrosstrack[k] / (bath[k] - sensordepth)));
                    printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'g': /* grazing angle using slope */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                            slopeacrosstrack, bathacrosstrack[k], &depth, &slope, &error);
                    angle = RTD * (atan(bathacrosstrack[k] / (bath[k] - sensordepth))) + slope;
                    printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  break;
                case 'H': /* heading */
                  printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'h': /* course */
                  printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'J': /* time string */
                  mb_get_jtime(verbose, time_i, time_j);
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3],
                              time_i[4], time_i[5], time_i[6]);
                    else
                      fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3],
                              time_i[4], seconds);
                  }
                  else {
                    b = time_j[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[5];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[6];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'j': /* time string */
                  mb_get_jtime(verbose, time_i, time_j);
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3],
                              time_j[4]);
                    else
                      fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
                  }
                  else {
                    b = time_j[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'K': /* proportion of good beams over non-null beams */
                  if (beams_bath - beams_null > 0)
                    goodbeamfraction = ((double)beams_unflagged) / ((double)(beams_bath - beams_null));
                  else
                    goodbeamfraction = 0.0;
                  printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'k': /* proportion of good beams over all possible beams */
                  if (beams_bath > 0)
                    goodbeamfraction = ((double)beams_unflagged) / ((double)beams_bath);
                  else
                    goodbeamfraction = 0.0;
                  printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'L': /* along-track distance (km) */
                  printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'l': /* along-track distance (m) */
                  printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'M': /* Decimal unix seconds since
                        1/1/70 00:00:00 */
                  printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'm': /* time in decimal seconds since
                        first record */
                  if (first_m) {
                    time_d_ref = time_d;
                    first_m = false;
                  }
                  b = time_d - time_d_ref;
                  printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value,
                                   &error);
                  break;
                case 'N': /* ping counter | ping number | shot number */
                  if (ascii)
                    fprintf(output[i], "%6u", pingnumber);
                  else {
                    b = pingnumber;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'n': /* line number */
                  if (ascii)
                    fprintf(output[i], "%6u", linenumber);
                  else {
                    b = linenumber;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'P': /* pitch */
                  printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'p': /* draft */
                  printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'q': /* bottom detection type */
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");

                    fprintf(output[i], "%d", detect[k]);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = detect[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'Q': /* bottom detection type */
                  if (ascii) {
                    if (netcdf) {
                      fprintf(output[i], "\"");
                      fprintf(output[i], "%d", detect[k]);
                      fprintf(output[i], "\"");
                    }
                    else {
                      if (detect[k] == MB_DETECT_AMPLITUDE)
                        fprintf(output[i], "A");
                      else if (detect[k] == MB_DETECT_PHASE)
                        fprintf(output[i], "P");
                      else
                        fprintf(output[i], "U");
                    }
                  }
                  else {
                    b = detect[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'R': /* roll */
                  printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'r': /* heave */
                  printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'S': /* speed */
                  printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 's': /* speed made good */
                  printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");

                    fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2],
                            time_i[3], time_i[4], seconds);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = time_i[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = seconds;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 't': /* yyyy mm dd hh mm ss time string */
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2],
                              time_i[3], time_i[4], time_i[5], time_i[6]);
                    else
                      fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2],
                              time_i[3], time_i[4], seconds);
                  }
                  else {
                    b = time_i[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = seconds;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
                  time_u = (int)time_d;
                  if (ascii)
                    fprintf(output[i], "%ld", time_u);
                  else {
                    b = time_u;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'u': /* time in seconds since first record */
                  time_u = (int)time_d;
                  if (first_u) {
                    time_u_ref = time_u;
                    first_u = false;
                  }
                  if (ascii)
                    fprintf(output[i], "%ld", time_u - time_u_ref);
                  else {
                    b = time_u - time_u_ref;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'V': /* time in seconds since last ping */
                case 'v':
                  if (ascii) {
                    if (fabs(time_interval) > 100.)
                      fprintf(output[i], "%g", time_interval);
                    else
                      fprintf(output[i], "%10.6f", time_interval);
                  }
                  else {
                    fwrite(&time_interval, sizeof(double), 1, outfile);
                  }
                  break;
                case 'X': /* longitude decimal degrees */
                  if (!projectednav_next_value) {
                    if (sensorrelative_next_value)
                      dlon = 0.0;
                    else
                      dlon = navlon;
                    if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                      dlon += headingy * mtodeglon * bathacrosstrack[k] +
                              headingx * mtodeglon * bathalongtrack[k];
                    printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  else {
                    if (sensorrelative_next_value)
                      deasting = 0.0;
                    else
                      deasting = naveasting;
                    if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                      deasting += headingy * bathacrosstrack[k] + headingx * bathalongtrack[k];
                    printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  projectednav_next_value = false;
                  break;
                case 'x': /* longitude degrees + decimal minutes */
                  dlon = navlon;
                  if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                    dlon +=
                        headingy * mtodeglon * bathacrosstrack[k] + headingx * mtodeglon * bathalongtrack[k];
                  if (dlon < 0.0) {
                    hemi = 'W';
                    dlon = -dlon;
                  }
                  else
                    hemi = 'E';
                  degrees = (int)dlon;
                  minutes = 60.0 * (dlon - degrees);
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");
                    fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = degrees;
                    if (hemi == 'W')
                      b = -b;
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = minutes;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  sensornav_next_value = false;
                  break;
                case 'Y': /* latitude decimal degrees */
                  if (!projectednav_next_value) {
                    if (sensorrelative_next_value)
                      dlat = 0.0;
                    else
                      dlat = navlat;
                    if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                      dlat += -headingx * mtodeglat * bathacrosstrack[k] +
                              headingy * mtodeglat * bathalongtrack[k];
                    printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  else {
                    if (sensorrelative_next_value)
                      dnorthing = 0.0;
                    else
                      dnorthing = navnorthing;
                    if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                      dnorthing += -headingx * bathacrosstrack[k] + headingy * bathalongtrack[k];
                    printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  projectednav_next_value = false;
                  break;
                case 'y': /* latitude degrees + decimal minutes */
                  dlat = navlat;
                  if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                    dlat +=
                        -headingx * mtodeglat * bathacrosstrack[k] + headingy * mtodeglat * bathalongtrack[k];
                  if (dlat < 0.0) {
                    hemi = 'S';
                    dlat = -dlat;
                  }
                  else
                    hemi = 'N';
                  degrees = (int)dlat;
                  minutes = 60.0 * (dlat - degrees);
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");
                    fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = degrees;
                    if (hemi == 'S')
                      b = -b;
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = minutes;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  sensornav_next_value = false;
                  break;
                case 'Z': /* topography */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = -bathy_scale * bath[k];
                    if (sensorrelative_next_value)
                      b -= -bathy_scale * sensordepth;
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  break;
                case 'z': /* depth */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = bathy_scale * bath[k];
                    if (sensorrelative_next_value)
                      b -= bathy_scale * sensordepth;
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  break;
                case '#': /* beam number */
                  if (ascii)
                    fprintf(output[i], "%6d", k);
                  else {
                    b = k;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                default:
                  if (ascii)
                    fprintf(output[i], "<Invalid Option: %c>", list[i]);
                  break;
                }
              }
              else /* raw_next_value */
              {
                switch (list[i]) {
                case '/': /* Inverts next simple value */
                  invert_next_value = true;
                  special_character = true;
                  break;
                case '-': /* Flip sign on next simple value */
                  signflip_next_value = true;
                  special_character = true;
                  break;
                case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
                  sensornav_next_value = true;
                  special_character = true;
                  break;
                case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
                  sensorrelative_next_value = true;
                  special_character = true;
                  break;
                case '^': /* Print position values in projected coordinates
                           * - easting northing rather than lon lat
                           * - applies to XY */
                  projectednav_next_value = true;
                  special_character = true;
                  break;
                case '.': /* Raw value next field */
                  raw_next_value = true;
                  special_character = true;
                  count = 0;
                  break;
                case '=': /* Port-most value next field -ignored here */
                  port_next_value = true;
                  special_character = true;
                  break;
                case '+': /* Starboard-most value next field - ignored here*/
                  stbd_next_value = true;
                  special_character = true;
                  break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                  count = count * 10 + list[i] - '0';
                  break;

                case 'A': /* backscatter */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    printsimplevalue(verbose, output[i], bs[k], 5, 1, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  raw_next_value = false;
                  break;

                case 'a': /* absorption */
                  printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'B': /* BSN - Normal incidence backscatter */
                  printsimplevalue(verbose, output[i], bsn, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;
                case 'b': /* BSO - Oblique backscatter */
                  printsimplevalue(verbose, output[i], bso, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'c': /* Mean backscatter */
                  mback = 0;
                  nback = 0;
                  for (int m = 0; m < beams_amp; m++) {
                    if (mb_beam_ok(beamflag[m])) {
                      mback += amp[m];
                      nback++;
                    }
                  }
                  printsimplevalue(verbose, output[i], mback / nback, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'C': /* Value from specified column of secondary file */
                  mb_linear_interp(verbose, secondary_time_d-1, (&secondary_data[num_secondary * (count-1)])-1, num_secondary, time_d, 
                                    &dsecondary, &j_secondary_interp, &error);
                  printsimplevalue(verbose, output[i], dsecondary, 16, 8, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'd': /* beam depression angle */
                  if (beamflag[k] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[k]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    printsimplevalue(verbose, output[i], depression[k], 5, 2, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  raw_next_value = false;
                  break;

                case 'F': /* filename */
                  if (netcdf)
                    fprintf(output[i], "\"");
                  fprintf(output[i], "%s", path);

                  if (netcdf)
                    fprintf(output[i], "\"");

                  if (!ascii)
                    for (k = strlen(path); k < MB_PATH_MAXLINE; k++)
                      fwrite(&path[strlen(path)], sizeof(char), 1, outfile);

                  raw_next_value = false;
                  break;

                case 'f': /* format */
                  if (ascii)
                    fprintf(output[i], "%6d", format);
                  else {
                    b = format;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'G': /* TVG start */
                  if (ascii)
                    fprintf(output[i], "%6d", tvg_start);
                  else {
                    b = tvg_start;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'g': /* TVG stop */
                  if (ascii)
                    fprintf(output[i], "%6d", tvg_stop);
                  else {
                    b = tvg_stop;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'L': /* Pulse length */
                  if (ascii)
                    fprintf(output[i], "%6d", ipulse_length);
                  else {
                    b = ipulse_length;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'l': /* Transmit pulse length (sec) */
                  printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'M': /* mode */
                  if (ascii)
                    fprintf(output[i], "%4d", mode);
                  else {
                    b = mode;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'N': /* ping counter */
                  if (ascii)
                    fprintf(output[i], "%6d", png_count);
                  else {
                    b = png_count;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'p': /* sidescan */
                  invert = invert_next_value;
                  flip = signflip_next_value;
                  printsimplevalue(verbose, output[i], ss_pixels[start_sample[k]], 5, 1, ascii,
                                   &invert_next_value, &signflip_next_value, &error);
                  if (count > 0) {
                    int m = 1;
                    for (; m < count && m < beam_samples[k]; m++) {
                      if (netcdf)
                        fprintf(output[i], ", ");
                      if (ascii)
                        fprintf(output[i], "%s", delimiter);
                      invert_next_value = invert;
                      signflip_next_value = flip;

                      printsimplevalue(verbose, output[i], ss_pixels[start_sample[k] + m], 5, 1, ascii,
                                       &invert_next_value, &signflip_next_value, &error);
                    }
                    for (; m < count; m++) {
                      if (netcdf)
                        fprintf(output[i], ", ");
                      if (ascii)
                        fprintf(output[i], "%s", delimiter);
                      printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                    }
                  }
                  raw_next_value = false;
                  break;

                case 'R': /* range */
                  if (ascii)
                    fprintf(output[i], "%6d", range[k]);
                  else {
                    b = range[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 'r': /* Sample rate */
                  if (ascii)
                    fprintf(output[i], "%6d", sample_rate);
                  else {
                    b = sample_rate;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 'S': /* Sidescan pixels */
                  if (ascii)
                    fprintf(output[i], "%6d", npixels);
                  else {
                    b = npixels;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 's': /* Sidescan pixels per beam */
                  if (ascii)
                    fprintf(output[i], "%6d", beam_samples[k]);
                  else {
                    b = beam_samples[k];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 'T': /* Transmit gain (dB) */
                  printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;
                case 't': /* Receive gain (dB) */
                  printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                default:
                  if (ascii)
                    fprintf(output[i], "<Invalid Option: %c>", list[i]);
                  raw_next_value = false;
                  break;
                }
              }
              if (ascii) {
                if (i < (n_list - 1)) {
                  if (!special_character) {
                    fprintf(output[i], "%s", delimiter);
                  }
                  else {
                    special_character = false;
                  }
                }
                else
                  fprintf(output[lcount++ % n_list], "\n");
              }
            }
          }
        }

      /* now loop over pixels */
      if (error == MB_ERROR_NO_ERROR && (nread - 1) % decimate == 0)
        for (int j = pixel_start; j <= pixel_end; j++) {
          /* check pixel status */
          pixel_status = MB_SUCCESS;
          if (check_bath && j != pixel_vertical)
            pixel_status = MB_FAILURE;
          else if (check_bath && j == pixel_vertical) {
            if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical]))
              pixel_status = MB_FAILURE;
            else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL)
              pixel_status = MB_FAILURE;
          }
          if (check_amp && j != pixel_vertical)
            pixel_status = MB_FAILURE;
          else if (check_amp && j == pixel_vertical) {
            if (check_values == MBLIST_CHECK_ON && !mb_beam_ok(beamflag[beam_vertical]))
              pixel_status = MB_FAILURE;
            else if (check_values == MBLIST_CHECK_ON_NULL && beamflag[beam_vertical] == MB_FLAG_NULL)
              pixel_status = MB_FAILURE;
          }
          if (check_ss && ss[j] <= MB_SIDESCAN_NULL)
            pixel_status = MB_FAILURE;
          if (use_time_interval && first)
            pixel_status = MB_FAILURE;
          // TODO(schwehr): Should the last check be mavlat?
          if (check_nav && (navlon == 0.0 /* || navlon == 0.0 */ ))
            pixel_status = MB_FAILURE;

          /* print out good pixels */
          if (pixel_status == MB_SUCCESS) {
            signflip_next_value = false;
            invert_next_value = false;
            raw_next_value = false;
            sensornav_next_value = false;
            projectednav_next_value = false;
            special_character = false;
            for (int i = 0; i < n_list; i++) {
              if (netcdf && lcount > 0)
                fprintf(output[i], ", ");
              int k;
              if (port_next_value) {
                k = pixel_port;
                port_next_value = false;
              }
              else if (stbd_next_value) {
                k = pixel_stbd;
                stbd_next_value = false;
              }
              else
                k = j;

              if (!raw_next_value) {
                switch (list[i]) {
                case '/': /* Inverts next simple value */
                  invert_next_value = true;
                  special_character = true;
                  break;
                case '-': /* Flip sign on next simple value */
                  signflip_next_value = true;
                  special_character = true;
                  break;
                case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
                  sensornav_next_value = true;
                  special_character = true;
                  break;
                case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
                  sensorrelative_next_value = true;
                  special_character = true;
                  break;
                case '^': /* Print position values in projected coordinates
                           * - easting northing rather than lon lat
                           * - applies to XY */
                  projectednav_next_value = true;
                  special_character = true;
                  break;
                case '.': /* Raw value next field */
                  raw_next_value = true;
                  count = 0;
                  special_character = true;
                  break;
                case '=': /* Port-most value next field -ignored here */
                  port_next_value = true;
                  special_character = true;
                  break;
                case '+': /* Starboard-most value next field - ignored here*/
                  stbd_next_value = true;
                  special_character = true;
                  break;
                case 'A': /* Average seafloor crosstrack slope */
                  printsimplevalue(verbose, output[i], avgslope, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'a': /* Per-pixel seafloor crosstrack slope */
                  status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                          slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
                  printsimplevalue(verbose, output[i], slope, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'B': /* amplitude */
                  printsimplevalue(verbose, output[i], amp[beam_vertical], 0, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'b': /* sidescan */
                  printsimplevalue(verbose, output[i], ss[k], 0, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'C': /* Sonar altitude (m) */
                  printsimplevalue(verbose, output[i], altitude, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'c': /* Sonar transducer depth (m) */
                  printsimplevalue(verbose, output[i], sensordepth, 0, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'D': /* acrosstrack dist. */
                case 'd':
                  b = bathy_scale * ssacrosstrack[k];
                  printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value,
                                   &error);
                  break;
                case 'E': /* alongtrack dist. */
                case 'e':
                  b = bathy_scale * ssalongtrack[k];
                  printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value, &signflip_next_value,
                                   &error);
                  break;
                case 'G': /* flat bottom grazing angle */
                  status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                          slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
                  angle = RTD * (atan(ssacrosstrack[k] / (depth - sensordepth)));
                  printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'g': /* grazing angle using slope */
                  status = get_bathyslope(verbose, ndepths, depths, depthacrosstrack, nslopes, slopes,
                                          slopeacrosstrack, ssacrosstrack[k], &depth, &slope, &error);
                  angle = RTD * (atan(bathacrosstrack[k] / (depth - sensordepth))) + slope;
                  printsimplevalue(verbose, output[i], angle, 0, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'H': /* heading */
                  printsimplevalue(verbose, output[i], heading, 7, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'h': /* course */
                  printsimplevalue(verbose, output[i], course, 7, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'J': /* time string */
                  mb_get_jtime(verbose, time_i, time_j);
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d, %d", time_j[0], time_j[1], time_i[3],
                              time_i[4], time_i[5], time_i[6]);
                    else
                      fprintf(output[i], "%.4d %.3d %.2d %.2d %9.6f", time_j[0], time_j[1], time_i[3],
                              time_i[4], seconds);
                  }
                  else {
                    b = time_j[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[5];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[6];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'j': /* time string */
                  mb_get_jtime(verbose, time_i, time_j);
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d", time_j[0], time_j[1], time_j[2], time_j[3],
                              time_j[4]);
                    else
                      fprintf(output[i], "%.4d %.3d %.4d %9.6f", time_j[0], time_j[1], time_j[2], seconds);
                  }
                  else {
                    b = time_j[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_j[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'K': /* proportion of non-null beams that are unflagged */
                  if (beams_bath - beams_null > 0)
                    goodbeamfraction = ((double)beams_unflagged) / ((double)(beams_bath - beams_null));
                  else
                    goodbeamfraction = 0.0;
                  printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'k': /* proportion of all possible beams that are unflagged */
                  if (beams_bath > 0)
                    goodbeamfraction = ((double)beams_unflagged) / ((double)beams_bath);
                  else
                    goodbeamfraction = 0.0;
                  printsimplevalue(verbose, output[i], goodbeamfraction, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'L': /* along-track distance (km) */
                  printsimplevalue(verbose, output[i], distance_total, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'l': /* along-track distance (m) */
                  printsimplevalue(verbose, output[i], 1000.0 * distance_total, 8, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'M': /* Decimal unix seconds since
                        1/1/70 00:00:00 */
                  printsimplevalue(verbose, output[i], time_d, 0, 6, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'm': /* time in decimal seconds since
                        first record */
                  if (first_m) {
                    time_d_ref = time_d;
                    first_m = false;
                  }
                  b = time_d - time_d_ref;
                  printsimplevalue(verbose, output[i], b, 0, 6, ascii, &invert_next_value, &signflip_next_value,
                                   &error);
                  break;
                case 'N': /* ping counter */
                  if (ascii)
                    fprintf(output[i], "%6u", pingnumber);
                  else {
                    b = pingnumber;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'n': /* line number */
                  if (ascii)
                    fprintf(output[i], "%6u", linenumber);
                  else {
                    b = linenumber;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'P': /* pitch */
                  printsimplevalue(verbose, output[i], pitch, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'p': /* draft */
                  printsimplevalue(verbose, output[i], draft, 7, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'Q': /* bottom detection type */
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");

                    fprintf(output[i], "%d", MB_DETECT_UNKNOWN);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = MB_DETECT_UNKNOWN;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'R': /* roll */
                  printsimplevalue(verbose, output[i], roll, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'r': /* heave */
                  printsimplevalue(verbose, output[i], heave, 7, 4, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'S': /* speed */
                  printsimplevalue(verbose, output[i], speed, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 's': /* speed made good */
                  printsimplevalue(verbose, output[i], speed_made_good, 6, 3, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  break;
                case 'T': /* yyyy/mm/dd/hh/mm/ss time string */
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");
                    fprintf(output[i], "%.4d/%.2d/%.2d/%.2d/%.2d/%09.6f", time_i[0], time_i[1], time_i[2],
                            time_i[3], time_i[4], seconds);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = time_i[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = seconds;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 't': /* yyyy mm dd hh mm ss time string */
                  seconds = time_i[5] + 1e-6 * time_i[6];
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "%d, %d, %d, %d, %d, %d, %d", time_i[0], time_i[1], time_i[2],
                              time_i[3], time_i[4], time_i[5], time_i[6]);
                    else
                      fprintf(output[i], "%.4d %.2d %.2d %.2d %.2d %09.6f", time_i[0], time_i[1], time_i[2],
                              time_i[3], time_i[4], seconds);
                  }
                  else {
                    b = time_i[0];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[1];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[2];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[3];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = time_i[4];
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = seconds;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'U': /* unix time in seconds since 1/1/70 00:00:00 */
                  time_u = (int)time_d;
                  if (ascii)
                    fprintf(output[i], "%ld", time_u);
                  else {
                    b = time_u;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'u': /* time in seconds since first record */
                  time_u = (int)time_d;
                  if (first_u) {
                    time_u_ref = time_u;
                    first_u = false;
                  }
                  if (ascii)
                    fprintf(output[i], "%ld", time_u - time_u_ref);
                  else {
                    b = time_u - time_u_ref;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                case 'V': /* time in seconds since last ping */
                case 'v':
                  if (ascii) {
                    if (fabs(time_interval) > 100.)
                      fprintf(output[i], "%g", time_interval);
                    else
                      fprintf(output[i], "%10.6f", time_interval);
                  }
                  else {
                    fwrite(&time_interval, sizeof(double), 1, outfile);
                  }
                  break;
                case 'X': /* longitude decimal degrees */
                  if (!projectednav_next_value) {
                    if (sensorrelative_next_value)
                      dlon = 0.0;
                    else
                      dlon = navlon;
                    if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
                      dlon +=
                          headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
                    printsimplevalue(verbose, output[i], dlon, 15, 10, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  else {
                    if (sensorrelative_next_value)
                      deasting = 0.0;
                    else
                      deasting = naveasting;
                    if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
                      deasting += headingy * ssacrosstrack[k] + headingx * ssalongtrack[k];
                    printsimplevalue(verbose, output[i], deasting, 15, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  projectednav_next_value = false;
                  break;
                case 'x': /* longitude degrees + decimal minutes */
                  dlon = navlon;
                  if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
                    dlon += headingy * mtodeglon * ssacrosstrack[k] + headingx * mtodeglon * ssalongtrack[k];
                  if (dlon < 0.0) {
                    hemi = 'W';
                    dlon = -dlon;
                  }
                  else
                    hemi = 'E';
                  degrees = (int)dlon;
                  minutes = 60.0 * (dlon - degrees);
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");
                    fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = degrees;
                    if (hemi == 'W')
                      b = -b;
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = minutes;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  sensornav_next_value = false;
                  break;
                case 'Y': /* latitude decimal degrees */
                  if (!projectednav_next_value) {
                    if (sensorrelative_next_value)
                      dlat = 0.0;
                    else
                      dlat = navlat;
                    if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
                      dlat +=
                          -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
                    printsimplevalue(verbose, output[i], dlat, 15, 10, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  else {
                    if (sensorrelative_next_value)
                      dnorthing = 0.0;
                    else
                      dnorthing = navnorthing;
                    if (!sensornav_next_value && (beam_set != MBLIST_SET_OFF || k != j))
                      dnorthing += -headingx * ssacrosstrack[k] + headingy * ssalongtrack[k];
                    printsimplevalue(verbose, output[i], dnorthing, 15, 3, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  projectednav_next_value = false;
                  break;
                case 'y': /* latitude degrees + decimal minutes */
                  dlat = navlat;
                  if (!sensornav_next_value && (pixel_set != MBLIST_SET_OFF || k != j))
                    dlat += -headingx * mtodeglat * ssacrosstrack[k] + headingy * mtodeglat * ssalongtrack[k];
                  if (dlat < 0.0) {
                    hemi = 'S';
                    dlat = -dlat;
                  }
                  else
                    hemi = 'N';
                  degrees = (int)dlat;
                  minutes = 60.0 * (dlat - degrees);
                  if (ascii) {
                    if (netcdf)
                      fprintf(output[i], "\"");
                    fprintf(output[i], "%3d %11.8f%c", degrees, minutes, hemi);
                    if (netcdf)
                      fprintf(output[i], "\"");
                  }
                  else {
                    b = degrees;
                    if (hemi == 'S')
                      b = -b;
                    fwrite(&b, sizeof(double), 1, outfile);
                    b = minutes;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  sensornav_next_value = false;
                  break;
                case 'Z': /* topography */
                  if (beamflag[beam_vertical] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = -bathy_scale * bath[beam_vertical];
                    if (sensorrelative_next_value)
                      b -= -bathy_scale * sensordepth;
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  break;
                case 'z': /* depth */
                  if (beamflag[beam_vertical] == MB_FLAG_NULL &&
                      (check_values == MBLIST_CHECK_OFF_NAN || check_values == MBLIST_CHECK_OFF_FLAGNAN)) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else if (!mb_beam_ok(beamflag[beam_vertical]) && check_values == MBLIST_CHECK_OFF_FLAGNAN) {
                    printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                  }
                  else {
                    b = bathy_scale * bath[beam_vertical];
                    if (sensorrelative_next_value)
                      b -= bathy_scale * sensordepth;
                    printsimplevalue(verbose, output[i], b, 0, 4, ascii, &invert_next_value,
                                     &signflip_next_value, &error);
                  }
                  sensornav_next_value = false;
                  sensorrelative_next_value = false;
                  break;
                case '#': /* pixel number */
                  if (ascii)
                    fprintf(output[i], "%6d", k);
                  else {
                    b = k;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  break;
                default:
                  fprintf(output[i], "<Invalid Option: %c>", list[i]);
                  break;
                }
              }
              else /* raw_next_value */
              {
                switch (list[i]) {
                case '/': /* Inverts next simple value */
                  invert_next_value = true;
                  special_character = true;
                  break;
                case '-': /* Flip sign on next simple value */
                  signflip_next_value = true;
                  special_character = true;
                  break;
                case '_': /* Print sensor position rather than beam or pixel position - applies to XxYy */
                  sensornav_next_value = true;
                  special_character = true;
                  break;
                case '@': /* Print beam or pixel position and depth values relative to sensor - applies to XYZz */
                  sensorrelative_next_value = true;
                  special_character = true;
                  break;
                case '^': /* Print position values in projected coordinates
                           * - easting northing rather than lon lat
                           * - applies to XY */
                  projectednav_next_value = true;
                  special_character = true;
                  break;
                case '.': /* Raw value next field */
                  raw_next_value = true;
                  count = 0;
                  special_character = true;
                  break;
                case '=': /* Port-most value next field -ignored here */
                  port_next_value = true;
                  special_character = true;
                  break;
                case '+': /* Starboard-most value next field - ignored here*/
                  stbd_next_value = true;
                  special_character = true;
                  break;

                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                  count = count * 10 + list[i] - '0';
                  break;

                case 'A': /* backscatter */
                  printsimplevalue(verbose, output[i], bs[beam_vertical], 5, 1, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'a': /* absorption */
                  printsimplevalue(verbose, output[i], absorption, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'B': /* BSN - Normal incidence backscatter */
                  printsimplevalue(verbose, output[i], bsn, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'b': /* BSO - Oblique backscatter */
                  printsimplevalue(verbose, output[i], bso, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'c': /* Mean backscatter */
                  mback = 0;
                  nback = 0;
                  for (int m = 0; m < beams_amp; m++) {
                    if (mb_beam_ok(beamflag[m])) {
                      mback += amp[m];
                      nback++;
                    }
                  }
                  printsimplevalue(verbose, output[i], mback / nback, 5, 2, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'C': /* Value from specified column of secondary file */
                  mb_linear_interp(verbose, secondary_time_d-1, (&secondary_data[num_secondary * (count-1)])-1, num_secondary, time_d, 
                                    &dsecondary, &j_secondary_interp, &error);
                  printsimplevalue(verbose, output[i], dsecondary, 16, 8, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'd': /* beam depression angle */
                  printsimplevalue(verbose, output[i], depression[beam_vertical], 5, 2, ascii,
                                   &invert_next_value, &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'F': /* filename */
                  if (netcdf)
                    fprintf(output[i], "\"");
                  fprintf(output[i], "%s", path);
                  if (netcdf)
                    fprintf(output[i], "\"");

                  if (!ascii)
                    for (k = strlen(path); k < MB_PATH_MAXLINE; k++)
                      fwrite(&path[strlen(path)], sizeof(char), 1, outfile);

                  raw_next_value = false;
                  break;

                case 'f': /* format */
                  if (ascii)
                    fprintf(output[i], "%6d", format);
                  else {
                    b = format;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'G': /* TVG start */
                  if (ascii)
                    fprintf(output[i], "%6d", tvg_start);
                  else {
                    b = tvg_start;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'g': /* TVG stop */
                  if (ascii)
                    fprintf(output[i], "%6d", tvg_stop);
                  else {
                    b = tvg_stop;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'L': /* Pulse length */
                  if (ascii)
                    fprintf(output[i], "%6d", ipulse_length);
                  else {
                    b = ipulse_length;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'l': /* Transmit pulse length (sec) */
                  printsimplevalue(verbose, output[i], pulse_length, 9, 6, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                case 'M': /* mode */
                  if (ascii)
                    fprintf(output[i], "%4d", mode);
                  else {
                    b = mode;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'N': /* ping counter */
                  if (ascii)
                    fprintf(output[i], "%6d", png_count);
                  else {
                    b = png_count;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'p': /* sidescan */
                  invert = invert_next_value;
                  flip = signflip_next_value;
                  printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical]], 5, 1, ascii,
                                   &invert_next_value, &signflip_next_value, &error);
                  if (count > 0) {
                    int m = 1;
                    for (; m < count && m < beam_samples[beam_vertical]; m++) {
                      if (netcdf)
                        fprintf(output[i], ", ");
                      if (ascii)
                        fprintf(output[i], "%s", delimiter);
                      invert_next_value = invert;
                      signflip_next_value = flip;

                      printsimplevalue(verbose, output[i], ss_pixels[start_sample[beam_vertical] + m], 5, 1,
                                       ascii, &invert_next_value, &signflip_next_value, &error);
                    }
                    for (; m < count; m++) {
                      if (netcdf)
                        fprintf(output[i], ", ");
                      if (ascii)
                        fprintf(output[i], "%s", delimiter);
                      printNaN(verbose, output[i], ascii, &invert_next_value, &signflip_next_value, &error);
                    }
                  }

                  raw_next_value = false;
                  break;

                case 'R': /* range */
                  if (ascii)
                    fprintf(output[i], "%6d", range[beam_vertical]);
                  else {
                    b = range[beam_vertical];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'r': /* Sample rate */
                  if (ascii)
                    fprintf(output[i], "%6d", sample_rate);
                  else {
                    b = sample_rate;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;

                case 'S': /* Sidescan pixels */
                  if (ascii)
                    fprintf(output[i], "%6d", npixels);
                  else {
                    b = npixels;
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 's': /* Sidescan pixels per beam */
                  if (ascii)
                    fprintf(output[i], "%6d", beam_samples[beam_vertical]);
                  else {
                    b = beam_samples[beam_vertical];
                    fwrite(&b, sizeof(double), 1, outfile);
                  }
                  raw_next_value = false;
                  break;
                case 'T': /* Transmit gain (dB) */
                  printsimplevalue(verbose, output[i], transmit_gain, 5, 1, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;
                case 't': /* Receive gain (dB) */
                  printsimplevalue(verbose, output[i], receive_gain, 5, 1, ascii, &invert_next_value,
                                   &signflip_next_value, &error);
                  raw_next_value = false;
                  break;

                default:
                  if (ascii)
                    fprintf(output[i], "<Invalid Option: %c>", list[i]);
                  raw_next_value = false;
                  break;
                }
              }
              if (ascii) {
                if (i < (n_list - 1)) {
                  if (!special_character) {
                    fprintf(output[i], "%s", delimiter);
                  }
                  else {
                    special_character = false;
                  }
                }
                else
                  fprintf(output[lcount++ % n_list], "\n");
              }
            }
          }
        }

      /* reset first flag */
      if (error == MB_ERROR_NO_ERROR && first) {
        first = false;
      }
    }

    /* close the swath file */
    status &= mb_close(verbose, &mbio_ptr, &error);

    /* deallocate memory used for data arrays */
    if (use_raw) {
      mb_freed(verbose, __FILE__, __LINE__, (void **)&ss_pixels, &error);
    }

    /* figure out whether and what to read next */
    if (read_datalist) {
      read_data = mb_datalist_read3(verbose, datalist, &pstatus, path, ppath, 
                                    &astatus, apath, dpath, &format, &file_weight, &error) == MB_SUCCESS;
    } else {
      read_data = false;
    }

    /* end loop over files in list */
  }
  if (read_datalist)
    mb_datalist_close(verbose, &datalist, &error);

  /* compile CDL file */
  if (netcdf) {
    for (int i = 0; i < n_list; i++) {
      if (list[i] != '/' && list[i] != '-' && list[i] != '.' && !(list[i] >= '0' && list[i] <= '9')) {
        fprintf(output[i], " ;\n\n");
        rewind(output[i]);

        /* copy data to CDL file */
	      char buffer[MB_BUFFER_MAX];
        size_t read_len = 0;
        while ((read_len = fread(buffer, sizeof(char), MB_BUFFER_MAX, output[i])) > 0) {
          size_t write_len = fwrite(buffer, sizeof(char), read_len, outfile);
          if (write_len != read_len) {
            fprintf(stderr, "Error writing to CDL file");
          }
        }
      }
      fclose(output[i]);
    }

    fprintf(outfile, "}\n");
    fclose(outfile);

    /* convert cdl to netcdf */
    if (!netcdf_cdl) {
      snprintf(output_file_temp, sizeof(output_file_temp), "ncgen -o %s %s.cdl", output_file, output_file);
      const int shellstatus = system(output_file_temp);
      if (shellstatus == 0) {
        snprintf(output_file_temp, sizeof(output_file_temp), "rm %s.cdl", output_file);
        // TODO(schwehr): Check return of system.
        /* shellstatus = */ system(output_file_temp);
      }
    }
  } else {
    fclose(outfile);
  }

  /* free secondary file data */
  if (num_secondary_alloc > 0) {
    mb_freed(verbose, __FILE__, __LINE__, (void **)&secondary_time_d, &error);
    mb_freed(verbose, __FILE__, __LINE__, (void **)&secondary_data, &error);
    num_secondary_alloc = 0;
  }

  /* free projection */
  if (use_projection && pjptr != NULL) {
    mb_proj_free(verbose, &(pjptr), &error);
  }

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
