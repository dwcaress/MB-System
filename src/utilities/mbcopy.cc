/*--------------------------------------------------------------------
 *    The MB-system:  mbcopy.c  2/4/93
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
 * MBcopy copies an input swath sonar data file to an output
 * swath sonar data file with the specified conversions.  Options include
 * windowing in time and space and ping averaging.  The input and
 * output data formats may differ, though not all possible combinations
 * make sense.  The default input and output streams are stdin
 * and stdout.
 *
 * Author:  D. W. Caress
 * Date:  February 4, 1993
 */

#include <algorithm>
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
#include "mbsys_elacmk2.h"
#ifdef ENABLE_GSF
#include "mbsys_gsf.h"
#endif
#include "mbsys_hsds.h"
#include "mbsys_ldeoih.h"
#include "mbsys_reson8k.h"
#include "mbsys_simrad.h"
#include "mbsys_simrad2.h"
#include "mbsys_xse.h"

/* defines for special copying routines */
typedef enum {
    MBCOPY_PARTIAL = 0,
    MBCOPY_FULL = 1,
    MBCOPY_ELACMK2_TO_XSE = 2,
    MBCOPY_XSE_TO_ELACMK2 = 3,
    MBCOPY_SIMRAD_TO_SIMRAD2 = 4,
    MBCOPY_ANY_TO_MBLDEOIH = 5,
#ifdef ENABLE_GSF
    MBCOPY_RESON8K_TO_GSF = 6,
#endif
} copy_mode_t;

typedef enum {
    MBCOPY_STRIPMODE_NONE      = 0,
    MBCOPY_STRIPMODE_COMMENTS  = 1,
    MBCOPY_STRIPMODE_BATHYONLY = 2,
} strip_mode_t;

constexpr char program_name[] = "MBcopy";
constexpr char help_message[] =
    "MBcopy copies an input swath sonar data file to an output\n"
    "swath sonar data file with the specified conversions.  Options include\n"
    "windowing in time and space and ping averaging.  The input and\n"
    "output data formats may differ, though not all possible combinations\n"
    "make sense.  The default input and output streams are stdin and stdout.";
constexpr char usage_message[] =
    "mbcopy\n"
    "\t--bathymetry-only {-D}\n"
    "\t--begin-time=yr/mo/da/hr/mn/sc {-Byr/mo/da/hr/mn/sc}\n"
    "\t--bounds=west/east/south/north {-Rwest/east/south/north}\n"
    "\t--comment-file=commentfile {-Ccommentfile}\n"
    "\t--end-time=yr/mo/da/hr/mn/sc {-Eyr/mo/da/hr/mn/sc}\n"
    "\t--format=iformat/oformat/mformat {-Fiformat/oformat/mformat}\n"
    "\t--help {-H}\n"
    "\t--input=infile {-Iinfile}\n"
    "\t--lonflip=lonflip {-Llonflip}\n"
    "\t--merge-file=mergefile {-Mmergefile}\n"
    "\t--output=outfile {-Ooutfile}\n"
    "\t--ping-average=pings {-Ppings}\n"
    "\t--sleep-factor=sleep_factor {-Qsleep_factor}\n"
    "\t--speed-minimum=speed {-Sspeed}\n"
    "\t--strip-comments {-N}\n"
    "\t--time-gap=timegap {-Ttimegap}\n"
    "\t--verbose {-V}\n";

/*--------------------------------------------------------------------*/
int setup_transfer_rules(int verbose, int ibeams, int obeams, int *istart, int *iend, int *offset, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       ibeams:     %d\n", ibeams);
    fprintf(stderr, "dbg2       obeams:     %d\n", obeams);
  }

  /* set up transfer rules */
  if (ibeams == obeams) {
    *istart = 0;
    *iend = ibeams;
    *offset = 0;
  }
  else if (ibeams < obeams) {
    *istart = 0;
    *iend = ibeams;
    *offset = obeams / 2 - ibeams / 2;
  }
  else if (ibeams > obeams) {
    *istart = ibeams / 2 - obeams / 2;
    *iend = *istart + obeams;
    *offset = -*istart;
  }

  /* assume success */
  *error = MB_ERROR_NO_ERROR;
  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2       istart:     %d\n", *istart);
    fprintf(stderr, "dbg2       iend:       %d\n", *iend);
    fprintf(stderr, "dbg2       offset:     %d\n", *offset);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbcopy_elacmk2_to_xse(int verbose, struct mbsys_elacmk2_struct *istore, struct mbsys_xse_struct *ostore, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       istore:     %p\n", (void *)istore);
    fprintf(stderr, "dbg2       ostore:     %p\n", (void *)ostore);
  }

  /* copy the data  */
  if (istore != nullptr && ostore != nullptr && (void *)istore != (void *)ostore) {
    /* type of data record */
    ostore->kind = istore->kind; /* Survey, nav, Comment */

    /* parameter (ship frames) */
    ostore->par_source = 0; /* sensor id */
    int time_i[7];
    mb_fix_y2k(verbose, istore->par_year, &time_i[0]);
    time_i[1] = istore->par_month;
    time_i[2] = istore->par_day;
    time_i[3] = istore->par_hour;
    time_i[4] = istore->par_minute;
    time_i[5] = istore->par_second;
    time_i[6] = 10000 * istore->par_hundredth_sec + 100 * istore->par_thousandth_sec;
    double time_d;
    mb_get_time(verbose, time_i, &time_d);
    ostore->par_sec = ((unsigned int)time_d) + MBSYS_XSE_TIME_OFFSET;     /* sec since 1/1/1901 00:00 */
    ostore->par_usec = (time_d - ((int)time_d)) * 1000000;                /* microseconds */
    ostore->par_roll_bias = DTR * 0.01 * istore->roll_offset;             /* radians */
    ostore->par_pitch_bias = DTR * 0.01 * istore->pitch_offset;           /* radians */
    ostore->par_heading_bias = DTR * 0.01 * istore->heading_offset;       /* radians */
    ostore->par_time_delay = 0.01 * istore->time_delay;                   /* nav time lag, seconds */
    ostore->par_trans_x_port = 0.01 * istore->transducer_port_x;          /* port transducer x position, meters */
    ostore->par_trans_y_port = 0.01 * istore->transducer_port_y;          /* port transducer y position, meters */
    ostore->par_trans_z_port = 0.01 * istore->transducer_port_depth;      /* port transducer z position, meters */
    ostore->par_trans_x_stbd = 0.01 * istore->transducer_starboard_x;     /* starboard transducer x position, meters */
    ostore->par_trans_y_stbd = 0.01 * istore->transducer_starboard_y;     /* starboard transducer y position, meters */
    ostore->par_trans_z_stbd = 0.01 * istore->transducer_starboard_depth; /* starboard transducer z position, meters */
    ostore->par_trans_err_port =
        0.01 * istore->transducer_port_error; /* port transducer rotation in roll direction, radians */
    ostore->par_trans_err_stbd =
        0.01 * istore->transducer_starboard_error;     /* starboard transducer rotation in roll direction, radians */
    ostore->par_nav_x = 0.01 * istore->antenna_x;      /* navigation antenna x position, meters */
    ostore->par_nav_y = 0.01 * istore->antenna_y;      /* navigation antenna y position, meters */
    ostore->par_nav_z = 0.01 * istore->antenna_height; /* navigation antenna z position, meters */
    ostore->par_hrp_x = 0.01 * istore->vru_x;          /* motion sensor x position, meters */
    ostore->par_hrp_y = 0.01 * istore->vru_y;          /* motion sensor y position, meters */
    ostore->par_hrp_z = 0.01 * istore->vru_height;     /* motion sensor z position, meters */

    /* svp (sound velocity frames) */
    ostore->svp_source = 0; /* sensor id */
    mb_fix_y2k(verbose, istore->svp_year, &time_i[0]);
    time_i[1] = istore->svp_month;
    time_i[2] = istore->svp_day;
    time_i[3] = istore->svp_hour;
    time_i[4] = istore->svp_minute;
    time_i[5] = istore->svp_second;
    time_i[6] = 10000 * istore->svp_hundredth_sec + 100 * istore->svp_thousandth_sec;
    mb_get_time(verbose, time_i, &time_d);
    ostore->svp_sec = ((unsigned int)time_d) + MBSYS_XSE_TIME_OFFSET; /* sec since 1/1/1901 00:00 */
    ostore->svp_usec = (time_d - ((int)time_d)) * 1000000;            /* microseconds */
    ostore->svp_nsvp = istore->svp_num;                               /* number of depth values */
    ostore->svp_nctd = 0;                                             /* number of ctd values */
    ostore->svp_ssv = istore->sound_vel;                              /* m/s */
    for (int i = 0; i < ostore->svp_nsvp; i++) {
      ostore->svp_depth[i] = 0.1 * istore->svp_depth[i];  /* m */
      ostore->svp_velocity[i] = 0.1 * istore->svp_vel[i]; /* m/s */
      ostore->svp_conductivity[i] = 0.0;                  /* mmho/cm */
      ostore->svp_salinity[i] = 0.0;                      /* o/oo */
      ostore->svp_temperature[i] = 0.0;                   /* degrees Celsius */
      ostore->svp_pressure[i] = 0.0;                      /* bar */
    }

    /* position (navigation frames) */
    ostore->nav_source = 0; /* sensor id */
    mb_fix_y2k(verbose, istore->pos_year, &time_i[0]);
    time_i[1] = istore->pos_month;
    time_i[2] = istore->pos_day;
    time_i[3] = istore->pos_hour;
    time_i[4] = istore->pos_minute;
    time_i[5] = istore->pos_second;
    time_i[6] = 10000 * istore->pos_hundredth_sec + 100 * istore->pos_thousandth_sec;
    mb_get_time(verbose, time_i, &time_d);
    ostore->nav_sec = ((unsigned int)time_d) + MBSYS_XSE_TIME_OFFSET; /* sec since 1/1/1901 00:00 */
    ostore->nav_usec = (time_d - ((int)time_d)) * 1000000;            /* microseconds */
    ostore->nav_quality = 0;
    ostore->nav_status = 0;
    ostore->nav_description_len = 0;
    for (int i = 0; i < MBSYS_XSE_DESCRIPTION_LENGTH; i++)
      ostore->nav_description[i] = 0;
    ostore->nav_x = DTR * 0.00000009 * istore->pos_longitude; /* eastings (m) or
              longitude (radians) */
    ostore->nav_y = DTR * 0.00000009 * istore->pos_latitude;  /* northings (m) or
              latitude (radians) */
    ostore->nav_z = 0.0;                                      /* height (m) or
                                                  ellipsoidal height (m) */
    ostore->nav_speed_ground = 0.0;                           /* m/s */
    ostore->nav_course_ground = DTR * 0.01 * istore->heading; /* radians */
    ostore->nav_speed_water = 0.0;                            /* m/s */
    ostore->nav_course_water = 0.0;                           /* radians */

    /* survey depth (multibeam frames) */
    if (ostore->kind == MB_DATA_DATA) {
      ostore->mul_frame = true;             /* boolean flag - multibeam frame read */
      ostore->mul_group_beam = false;         /* boolean flag - beam group read */
      ostore->mul_group_tt = true;          /* boolean flag - tt group read */
      ostore->mul_group_quality = true;     /* boolean flag - quality group read */
      ostore->mul_group_amp = true;         /* boolean flag - amp group read */
      ostore->mul_group_delay = true;       /* boolean flag - delay group read */
      ostore->mul_group_lateral = true;     /* boolean flag - lateral group read */
      ostore->mul_group_along = true;       /* boolean flag - along group read */
      ostore->mul_group_depth = true;       /* boolean flag - depth group read */
      ostore->mul_group_angle = true;       /* boolean flag - angle group read */
      ostore->mul_group_heave = true;       /* boolean flag - heave group read */
      ostore->mul_group_roll = true;        /* boolean flag - roll group read */
      ostore->mul_group_pitch = true;       /* boolean flag - pitch group read */
      ostore->mul_group_gates = false;        /* boolean flag - gates group read */
      ostore->mul_group_noise = false;        /* boolean flag - noise group read */
      ostore->mul_group_length = false;       /* boolean flag - length group read */
      ostore->mul_group_hits = false;         /* boolean flag - hits group read */
      ostore->mul_group_heavereceive = false; /* boolean flag - heavereceive group read */
      ostore->mul_group_azimuth = false;      /* boolean flag - azimuth group read */
      ostore->mul_group_mbsystemnav = true; /* boolean flag - mbsystemnav group read */
    }
    else {
      ostore->mul_frame = false;              /* boolean flag - multibeam frame read */
      ostore->mul_group_beam = false;         /* boolean flag - beam group read */
      ostore->mul_group_tt = false;           /* boolean flag - tt group read */
      ostore->mul_group_quality = false;      /* boolean flag - quality group read */
      ostore->mul_group_amp = false;          /* boolean flag - amp group read */
      ostore->mul_group_delay = false;        /* boolean flag - delay group read */
      ostore->mul_group_lateral = false;      /* boolean flag - lateral group read */
      ostore->mul_group_along = false;        /* boolean flag - along group read */
      ostore->mul_group_depth = false;        /* boolean flag - depth group read */
      ostore->mul_group_angle = false;        /* boolean flag - angle group read */
      ostore->mul_group_heave = false;        /* boolean flag - heave group read */
      ostore->mul_group_roll = false;         /* boolean flag - roll group read */
      ostore->mul_group_pitch = false;        /* boolean flag - pitch group read */
      ostore->mul_group_gates = false;        /* boolean flag - gates group read */
      ostore->mul_group_noise = false;        /* boolean flag - noise group read */
      ostore->mul_group_length = false;       /* boolean flag - length group read */
      ostore->mul_group_hits = false;         /* boolean flag - hits group read */
      ostore->mul_group_heavereceive = false; /* boolean flag - heavereceive group read */
      ostore->mul_group_azimuth = false;      /* boolean flag - azimuth group read */
      ostore->mul_group_mbsystemnav = false;  /* boolean flag - mbsystemnav group read */
    }
    ostore->mul_source = 0; /* sensor id */
    mb_fix_y2k(verbose, istore->pos_year, &time_i[0]);
    time_i[1] = istore->month;
    time_i[2] = istore->day;
    time_i[3] = istore->hour;
    time_i[4] = istore->minute;
    time_i[5] = istore->second;
    time_i[6] = 10000 * istore->hundredth_sec + 100 * istore->thousandth_sec;
    mb_get_time(verbose, time_i, &time_d);
    ostore->mul_sec = ((unsigned int)time_d) + MBSYS_XSE_TIME_OFFSET; /* sec since 1/1/1901 00:00 */
    ostore->mul_usec = (time_d - ((int)time_d)) * 1000000;            /* microseconds */
    ostore->mul_lon = DTR * istore->longitude;                        /* longitude (radians) */
    ostore->mul_lat = DTR * istore->latitude;                         /* latitude (radians) */
    ostore->mul_heading = DTR * 0.01 * istore->heading;               /* heading (radians) */
    ostore->mul_speed = 0.0;                                          /* speed (m/s) */
    ostore->mul_ping = istore->ping_num;                              /* ping number */
    ostore->mul_frequency = 0.0;                                      /* transducer frequency (Hz) */
    ostore->mul_pulse = istore->pulse_length;                         /* transmit pulse length (sec) */
    ostore->mul_power = istore->source_power;                         /* transmit power (dB) */
    ostore->mul_bandwidth = 0.0;                                      /* receive bandwidth (Hz) */
    ostore->mul_sample = 0.0;                                         /* receive sample interval (sec) */
    ostore->mul_swath = 0.0;                                          /* swath width (radians) */
    ostore->mul_num_beams = istore->beams_bath;                       /* number of beams */
    for (int i = 0; i < ostore->mul_num_beams; i++) {
      const int j = istore->beams_bath - i - 1;
      ostore->beams[i].tt = 0.0001 * istore->beams[j].tt;
      ostore->beams[i].delay = 0.0005 * istore->beams[j].time_offset;
      ostore->beams[i].lateral = 0.01 * istore->beams[j].bath_acrosstrack;
      ostore->beams[i].along = 0.01 * istore->beams[j].bath_alongtrack;
      ostore->beams[i].depth = 0.01 * istore->beams[j].bath;
      ostore->beams[i].angle = DTR * 0.005 * istore->beams[j].angle;
      ostore->beams[i].heave = 0.001 * istore->beams[j].heave;
      ostore->beams[i].roll = DTR * 0.005 * istore->beams[j].roll;
      ostore->beams[i].pitch = DTR * 0.005 * istore->beams[j].pitch;
      ostore->beams[i].beam = i + 1;
      ostore->beams[i].quality = istore->beams[j].quality;
      ostore->beams[i].amplitude = istore->beams[j].amplitude;
    }

    /* survey sidescan (sidescan frames) */
    ostore->sid_frame = false;           /* boolean flag - sidescan frame read */
    ostore->sid_group_avt = false;       /* boolean flag - amp vs time group read */
    ostore->sid_group_pvt = false;       /* boolean flag - phase vs time group read */
    ostore->sid_group_avl = false;       /* boolean flag - amp vs lateral group read */
    ostore->sid_group_pvl = false;       /* boolean flag - phase vs lateral group read */
    ostore->sid_group_signal = false;    /* boolean flag - phase vs lateral group read */
    ostore->sid_group_ping = false;      /* boolean flag - phase vs lateral group read */
    ostore->sid_group_complex = false;   /* boolean flag - phase vs lateral group read */
    ostore->sid_group_weighting = false; /* boolean flag - phase vs lateral group read */
    ostore->sid_source = 0;              /* sensor id */
    ostore->sid_sec = 0;                 /* sec since 1/1/1901 00:00 */
    ostore->sid_usec = 0;                /* microseconds */
    ostore->sid_ping = 0;                /* ping number */
    ostore->sid_frequency = 0.0;         /* transducer frequency (Hz) */
    ostore->sid_pulse = 0.0;             /* transmit pulse length (sec) */
    ostore->sid_power = 0.0;             /* transmit power (dB) */
    ostore->sid_bandwidth = 0.0;         /* receive bandwidth (Hz) */
    ostore->sid_sample = 0.0;            /* receive sample interval (sec) */
    ostore->sid_avt_sampleus = 0;        /* sample interval (usec) */
    ostore->sid_avt_offset = 0;          /* time offset (usec) */
    ostore->sid_avt_num_samples = 0;     /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_avt_amp[i] = 0;  /* sidescan amplitude (dB) */
    ostore->sid_pvt_sampleus = 0;    /* sample interval (usec) */
    ostore->sid_pvt_offset = 0;      /* time offset (usec) */
    ostore->sid_pvt_num_samples = 0; /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_pvt_phase[i] = 0; /* sidescan phase (radians) */
    ostore->sid_avl_binsize = 0;      /* bin size (mm) */
    ostore->sid_avl_offset = 0;       /* lateral offset (mm) */
    ostore->sid_avl_num_samples = 0;  /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_avl_amp[i] = 0;  /* sidescan amplitude (dB) */
    ostore->sid_pvl_binsize = 0;     /* bin size (mm) */
    ostore->sid_pvl_offset = 0;      /* lateral offset (mm) */
    ostore->sid_pvl_num_samples = 0; /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_pvl_phase[i] = 0; /* sidescan phase (radians) */
    ostore->sid_sig_ping = 0;         /* ping number */
    ostore->sid_sig_channel = 0;      /* channel number */
    ostore->sid_sig_offset = 0.0;     /* start offset */
    ostore->sid_sig_sample = 0.0;     /* bin size / sample interval */
    ostore->sid_sig_num_samples = 0;  /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_sig_phase[i] = 0;     /* sidescan phase in radians */
    ostore->sid_png_pulse = 0;            /* pulse type (0=constant, 1=linear sweep) */
    ostore->sid_png_startfrequency = 0.0; /* start frequency (Hz) */
    ostore->sid_png_endfrequency = 0.0;   /* end frequency (Hz) */
    ostore->sid_png_duration = 0.0;       /* pulse duration (msec) */
    ostore->sid_png_mancode = 0;          /* manufacturer code (1=Edgetech, 2=Elac) */
    ostore->sid_png_pulseid = 0;          /* pulse identifier */
    for (int i = 0; i < MBSYS_XSE_DESCRIPTION_LENGTH; i++)
      ostore->sid_png_pulsename[i] = 0; /* pulse name */
    ostore->sid_cmp_ping = 0;             /* ping number */
    ostore->sid_cmp_channel = 0;          /* channel number */
    ostore->sid_cmp_offset = 0.0;         /* start offset (usec) */
    ostore->sid_cmp_sample = 0.0;         /* bin size / sample interval (usec) */
    ostore->sid_cmp_num_samples = 0;      /* number of samples */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_cmp_real[i] = 0; /* real sidescan signal */
    for (int i = 0; i < MBSYS_XSE_MAXPIXELS; i++)
      ostore->sid_cmp_imaginary[i] = 0; /* imaginary sidescan signal */
    ostore->sid_wgt_factorleft = 0;       /* weighting factor for block floating
                      point expansion  --
                      defined as 2^(-N) volts for lsb */
    ostore->sid_wgt_samplesleft = 0;      /* number of left samples */
    ostore->sid_wgt_factorright = 0; /* weighting factor for block floating
             point expansion  --
             defined as 2^(-N) volts for lsb */
    ostore->sid_wgt_samplesright = 0; /* number of right samples */

    /* comment */
    for (int i = 0; i < std::min(MBSYS_ELACMK2_COMMENT_LENGTH, MBSYS_XSE_COMMENT_LENGTH); i++)
      ostore->comment[i] = istore->comment[i];

    /* unsupported frame */
    ostore->rawsize = 0;
    for (int i = 0; i < MBSYS_XSE_BUFFER_SIZE; i++)
      ostore->raw[i] = 0;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbcopy_xse_to_elacmk2(int verbose, struct mbsys_xse_struct *istore, struct mbsys_elacmk2_struct *ostore, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       istore:     %p\n", (void *)istore);
    fprintf(stderr, "dbg2       ostore:     %p\n", (void *)ostore);
    fprintf(stderr, "dbg2       kind:       %d\n", istore->kind);
  }

  /* copy the data  */
  if (istore != nullptr && ostore != nullptr && (void *)istore != (void *)ostore) {
    /* type of data record */
    ostore->kind = istore->kind;
    ostore->sonar = MBSYS_ELACMK2_UNKNOWN;

    /* parameter telegram */
    double time_d = istore->par_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * istore->par_usec;
    int time_i[7];
    mb_get_date(verbose, time_d, time_i);
    mb_unfix_y2k(verbose, time_i[0], &ostore->par_year);
    ostore->par_month = time_i[1];
    ostore->par_day = time_i[2];
    ostore->par_hour = time_i[3];
    ostore->par_minute = time_i[4];
    ostore->par_second = time_i[5];
    ostore->par_hundredth_sec = time_i[6] / 10000;
    ostore->par_thousandth_sec = (time_i[6] - 10000 * ostore->par_hundredth_sec) / 100;
    ostore->roll_offset = RTD * 100 * istore->par_roll_bias;       /* roll offset (degrees) */
    ostore->pitch_offset = RTD * 100 * istore->par_pitch_bias;     /* pitch offset (degrees) */
    ostore->heading_offset = RTD * 100 * istore->par_heading_bias; /* heading offset (degrees) */
    ostore->time_delay = 100 * istore->par_time_delay;             /* positioning system delay (sec) */
    ostore->transducer_port_height = 0;
    ostore->transducer_starboard_height = 0;
    ostore->transducer_port_depth = 200 * istore->par_trans_z_port;
    ostore->transducer_starboard_depth = 200 * istore->par_trans_z_stbd;
    ostore->transducer_port_x = 200 * istore->par_trans_x_port;
    ostore->transducer_starboard_x = 200 * istore->par_trans_x_port;
    ostore->transducer_port_y = 200 * istore->par_trans_x_port;
    ostore->transducer_starboard_y = 200 * istore->par_trans_x_port;
    ostore->transducer_port_error = 200 * RTD * istore->par_trans_err_port;
    ostore->transducer_starboard_error = 200 * RTD * istore->par_trans_err_stbd;
    ostore->antenna_height = 200 * istore->par_nav_z;
    ostore->antenna_x = 200 * istore->par_nav_x;
    ostore->antenna_y = 200 * istore->par_nav_y;
    ostore->vru_height = 200 * istore->par_hrp_z;
    ostore->vru_x = 200 * istore->par_hrp_x;
    ostore->vru_y = 200 * istore->par_hrp_y;
    ostore->line_number = 0;
    ostore->start_or_stop = 0;
    ostore->transducer_serial_number = 0;
    for (int i = 0; i < std::min(MBSYS_ELACMK2_COMMENT_LENGTH, MBSYS_XSE_COMMENT_LENGTH); i++)
      ostore->comment[i] = istore->comment[i];

    /* position (position telegrams) */
    time_d = istore->nav_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * istore->nav_usec;
    mb_get_date(verbose, time_d, time_i);
    mb_unfix_y2k(verbose, time_i[0], &ostore->pos_year);
    ostore->pos_month = time_i[1];
    ostore->pos_day = time_i[2];
    ostore->pos_hour = time_i[3];
    ostore->pos_minute = time_i[4];
    ostore->pos_second = time_i[5];
    ostore->pos_hundredth_sec = time_i[6] / 10000;
    ostore->pos_thousandth_sec = (time_i[6] - 10000 * ostore->pos_hundredth_sec) / 100;
    ostore->pos_latitude = RTD * istore->nav_y / 0.00000009;
    ostore->pos_longitude = RTD * istore->nav_x / 0.00000009;
    ostore->utm_northing = 0;
    ostore->utm_easting = 0;
    ostore->utm_zone_lon = 0;
    ostore->utm_zone = 0;
    ostore->hemisphere = 0;
    ostore->ellipsoid = 0;
    ostore->pos_spare = 0;
    ostore->semi_major_axis = 0;
    ostore->other_quality = 0;

    /* sound velocity profile */
    time_d = istore->svp_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * istore->svp_usec;
    mb_get_date(verbose, time_d, time_i);
    mb_unfix_y2k(verbose, time_i[0], &ostore->svp_year);
    ostore->svp_month = time_i[1];
    ostore->svp_day = time_i[2];
    ostore->svp_hour = time_i[3];
    ostore->svp_minute = time_i[4];
    ostore->svp_second = time_i[5];
    ostore->svp_hundredth_sec = time_i[6] / 10000;
    ostore->svp_thousandth_sec = (time_i[6] - 10000 * ostore->svp_hundredth_sec) / 100;
    ostore->svp_num = istore->svp_nsvp;
    for (int i = 0; i < 500; i++) {
      ostore->svp_depth[i] = 10 * istore->svp_depth[i];  /* 0.1 meters */
      ostore->svp_vel[i] = 10 * istore->svp_velocity[i]; /* 0.1 meters/sec */
    }

    /* depth telegram */
    time_d = istore->mul_sec - MBSYS_XSE_TIME_OFFSET + 0.000001 * istore->mul_usec;
    mb_get_date(verbose, time_d, time_i);
    mb_unfix_y2k(verbose, time_i[0], &ostore->year);
    ostore->month = time_i[1];
    ostore->day = time_i[2];
    ostore->hour = time_i[3];
    ostore->minute = time_i[4];
    ostore->second = time_i[5];
    ostore->hundredth_sec = time_i[6] / 10000;
    ostore->thousandth_sec = (time_i[6] - 10000 * ostore->hundredth_sec) / 100;
    ostore->longitude = RTD * istore->mul_lon;
    ostore->latitude = RTD * istore->mul_lat;
    ostore->ping_num = istore->mul_ping;
    ostore->sound_vel = 10 * istore->svp_ssv;
    ostore->heading = 100 * RTD * istore->nav_course_ground;
    ostore->pulse_length = istore->mul_pulse;
    ostore->mode = 0;
    ostore->source_power = istore->mul_power;
    ostore->receiver_gain_stbd = 0;
    ostore->receiver_gain_port = 0;
    ostore->reserved = 0;
    ostore->beams_bath = 0;
    for (int i = 0; i < MBSYS_ELACMK2_MAXBEAMS; i++) {
      ostore->beams[i].bath = 0;
      ostore->beams[i].bath_acrosstrack = 0;
      ostore->beams[i].bath_alongtrack = 0;
      ostore->beams[i].tt = 0;
      ostore->beams[i].quality = 0;
      ostore->beams[i].amplitude = 0;
      ostore->beams[i].time_offset = 0;
      ostore->beams[i].heave = 0;
      ostore->beams[i].roll = 0;
      ostore->beams[i].pitch = 0;
      ostore->beams[i].angle = 0;
    }
    ostore->beams_bath = istore->beams[istore->mul_num_beams - 1].beam;
    if (ostore->beams_bath >= MBSYS_ELACMK2_MAXBEAMS)
      ostore->beams_bath = MBSYS_ELACMK2_MAXBEAMS - 1;
    for (int i = 0; i < istore->mul_num_beams; i++) {
      const int j = ostore->beams_bath - istore->beams[i].beam;
      if (j < 0 || j >= MBSYS_ELACMK2_MAXBEAMS)
        continue;
      ostore->beams[j].bath = 100 * istore->beams[i].depth;
      ostore->beams[j].bath_acrosstrack = -100 * istore->beams[i].lateral;
      ostore->beams[j].bath_alongtrack = 100 * istore->beams[i].along;
      ostore->beams[j].tt = 10000 * istore->beams[i].tt;
      ostore->beams[j].quality = istore->beams[i].quality;
      ostore->beams[j].amplitude = istore->beams[i].amplitude;
      ostore->beams[j].time_offset = 10000 * istore->beams[i].delay;
      ostore->beams[j].heave = 1000 * istore->beams[i].heave;
      ostore->beams[j].roll = 200 * RTD * istore->beams[i].roll;
      ostore->beams[j].pitch = 200 * RTD * istore->beams[i].pitch;
      ostore->beams[j].angle = 200 * istore->beams[i].angle;
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbcopy_any_to_mbldeoih(int verbose, int kind, int sensorhead, int sensortype,
                           int *time_i, double time_d, double navlon, double navlat, double speed,
                           double heading, double draft, double altitude, double roll, double pitch, double heave,
                           double beamwidth_xtrack, double beamwidth_ltrack, int nbath, int namp, int nss, char *beamflag,
                           double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                           double *ssacrosstrack, double *ssalongtrack, char *comment, void *ombio_ptr, void *ostore_ptr,
                           int *error) {
  /* get data structure pointer */
  struct mbsys_ldeoih_struct *ostore = (struct mbsys_ldeoih_struct *)ostore_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       ombio_ptr:  %p\n", (void *)ombio_ptr);
    fprintf(stderr, "dbg2       ostore_ptr: %p\n", (void *)ostore_ptr);
    fprintf(stderr, "dbg2       kind:       %d\n", kind);
  }
  if (verbose >= 2 && kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       sensorhead: %d\n", sensorhead);
    fprintf(stderr, "dbg2       sensortype: %d\n", sensortype);
  }
  if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
    fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
    fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
    fprintf(stderr, "dbg2       speed:      %f\n", speed);
    fprintf(stderr, "dbg2       heading:    %f\n", heading);
    fprintf(stderr, "dbg2       draft:      %f\n", draft);
    fprintf(stderr, "dbg2       altitude:   %f\n", altitude);
    fprintf(stderr, "dbg2       roll:       %f\n", roll);
    fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
    fprintf(stderr, "dbg2       heave:      %f\n", heave);
    fprintf(stderr, "dbg2       beamwidth_xtrack: %f\n", beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack: %f\n", beamwidth_ltrack);
  }
  if (verbose >= 2 && kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       nbath:      %d\n", nbath);
    if (verbose >= 3)
      for (int i = 0; i < nbath; i++)
        fprintf(stderr, "dbg3       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
                bathacrosstrack[i], bathalongtrack[i]);
    fprintf(stderr, "dbg2       namp:       %d\n", namp);
    if (verbose >= 3)
      for (int i = 0; i < namp; i++)
        fprintf(stderr, "dbg3        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
                bathalongtrack[i]);
    fprintf(stderr, "dbg2        nss:       %d\n", nss);
    if (verbose >= 3)
      for (int i = 0; i < nss; i++)
        fprintf(stderr, "dbg3        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
                ssalongtrack[i]);
  }
  if (verbose >= 2 && kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }

  int status = MB_SUCCESS;

  /* copy the data  */
  if (ostore != nullptr) {
    /* set sensorhead and beam widths */
        ostore->sensorhead = sensorhead;
        ostore->topo_type = sensortype;
    ostore->beam_xwidth = beamwidth_xtrack;
    ostore->beam_lwidth = beamwidth_ltrack;
    ostore->kind = kind;

    /* insert data */
    if (kind == MB_DATA_DATA) {
      mb_insert_nav(verbose, ombio_ptr, (void *)ostore, time_i, time_d, navlon, navlat, speed, heading, draft, roll, pitch,
                    heave, error);
      mb_insert_altitude(verbose, ombio_ptr, (void *)ostore, draft, altitude, error);
    }
    status =
        mb_insert(verbose, ombio_ptr, (void *)ostore, kind, time_i, time_d, navlon, navlat, speed, heading, nbath, namp, nss,
                  beamflag, bath, amp, bathacrosstrack, bathalongtrack, ss, ssacrosstrack, ssalongtrack, comment, error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
#ifdef ENABLE_GSF
int mbcopy_reson8k_to_gsf(int verbose, void *imbio_ptr, void *ombio_ptr, int *error) {
  struct mb_io_struct *imb_io_ptr = (struct mb_io_struct *)imbio_ptr;
  struct mb_io_struct *omb_io_ptr = (struct mb_io_struct *)ombio_ptr;

  struct mbsys_reson8k_struct *istore = static_cast<struct mbsys_reson8k_struct *>(imb_io_ptr->store_data);
  /* get gsf data structure pointer */
  struct mbsys_gsf_struct *ostore = static_cast<struct mbsys_gsf_struct *>(omb_io_ptr->store_data);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBcopy function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       imbio_ptr:  %p\n", (void *)imbio_ptr);
    fprintf(stderr, "dbg2       ombio_ptr:  %p\n", (void *)ombio_ptr);
    fprintf(stderr, "dbg2       istore:     %p\n", (void *)istore);
    fprintf(stderr, "dbg2       ostore:     %p\n", (void *)ostore);
    fprintf(stderr, "dbg2       kind:       %d\n", istore->kind);
  }

  gsfDataID *dataID; /* pointers withinin gsf data */
  gsfRecords *records;
  gsfSwathBathyPing *mb_ping;
  gsfMBParams params;
  double gain_correction;
  double angscale;
  double alpha;
  double beta;

  int status = MB_SUCCESS;

  /* copy the data  */
  if (istore != nullptr && ostore != nullptr) {
    /* output gsf data structure  */
    records = &(ostore->records);
    dataID = &(ostore->dataID);
    mb_ping = &(records->mb_ping);

    /* set data kind */
    ostore->kind = istore->kind;

    /* insert data in structure */
    if (istore->kind == MB_DATA_DATA) {
      /* on the first ping set the processing parameters up  */
      if (omb_io_ptr->ping_count == 0) {
        /* ostore->kind = MB_DATA_PROCESSING_PARAMETERS;
        dataID->recordID = GSF_RECORD_PROCESSING_PARAMETERS; */
        memset((void *)&params, 0, sizeof(gsfMBParams));

        params.roll_compensated = GSF_COMPENSATED;
        params.pitch_compensated = GSF_COMPENSATED;
        params.heave_compensated = GSF_COMPENSATED;
        params.tide_compensated = 0;
        params.ray_tracing = 0;
        params.depth_calculation = GSF_DEPTHS_RE_1500_MS;
        params.to_apply.draft[0] = 0;
        params.to_apply.roll_bias[0] = 0;
        ;
        params.to_apply.pitch_bias[0] = 0;
        ;
        params.to_apply.gyro_bias[0] = 0;
        ;
        /* note it appears the x and y axis are switched between
            reson and gsf reference systems  */
        params.to_apply.position_x_offset = istore->NavOffsetY;
        params.to_apply.position_y_offset = istore->NavOffsetX;
        params.to_apply.position_z_offset = istore->NavOffsetZ;
        params.to_apply.transducer_x_offset[0] = istore->MBOffsetY;
        params.to_apply.transducer_y_offset[0] = istore->MBOffsetX;
        params.to_apply.transducer_z_offset[0] = istore->MBOffsetZ;
        params.to_apply.mru_roll_bias = istore->MRUOffsetRoll;
        params.to_apply.mru_pitch_bias = istore->MRUOffsetPitch;
        params.to_apply.mru_heading_bias = 0;
        params.to_apply.mru_x_offset = istore->MRUOffsetY;
        params.to_apply.mru_y_offset = istore->MRUOffsetX;
        params.to_apply.mru_z_offset = istore->MRUOffsetZ;
        params.to_apply.center_of_rotation_x_offset = 0;
        params.to_apply.center_of_rotation_y_offset = 0;
        params.to_apply.center_of_rotation_z_offset = 0;
        /* ret = */ gsfPutMBParams(&params, records, omb_io_ptr->gsfid, 1);
      }

      /* set data id */
      dataID->recordID = GSF_RECORD_SWATH_BATHYMETRY_PING;
      mb_ping = &(records->mb_ping);

      /* get time */
      mb_ping->ping_time.tv_sec = (int)istore->png_time_d;
      mb_ping->ping_time.tv_nsec = (int)(1000000000 * (istore->png_time_d - mb_ping->ping_time.tv_sec));

      /* get navigation, applying inverse projection if defined */
      mb_ping->longitude = istore->png_longitude;
      mb_ping->latitude = istore->png_latitude;
      if (imb_io_ptr->projection_initialized) {
        mb_proj_inverse(verbose, imb_io_ptr->pjptr, mb_ping->longitude, mb_ping->latitude, &(mb_ping->longitude),
                        &(mb_ping->latitude), error);
      }

      /* get heading */
      mb_ping->heading = istore->png_heading;

      /* get speed */
      mb_ping->speed = istore->png_speed / 1.852;

      /* set sonar depth */
      mb_ping->depth_corrector = istore->MBOffsetZ;

      /* do roll pitch heave */
      mb_ping->roll = istore->png_roll;
      mb_ping->pitch = istore->png_pitch;
      mb_ping->heave = istore->png_heave;

      /* get numbers of beams */
      mb_ping->number_beams = istore->beams_bath;

      /* allocate memory in arrays if required */
      if (istore->beams_bath > 0) {
        mb_ping->beam_flags = (unsigned char *)realloc(mb_ping->beam_flags, istore->beams_bath * sizeof(unsigned char));
        mb_ping->depth = (double *)realloc(mb_ping->depth, istore->beams_bath * sizeof(double));
        mb_ping->across_track = (double *)realloc(mb_ping->across_track, istore->beams_bath * sizeof(double));
        mb_ping->along_track = (double *)realloc(mb_ping->along_track, istore->beams_bath * sizeof(double));
        mb_ping->travel_time = (double *)realloc(mb_ping->travel_time, istore->beams_bath * sizeof(double));
        mb_ping->beam_angle = (double *)realloc(mb_ping->beam_angle, istore->beams_bath * sizeof(double));
        mb_ping->beam_angle_forward = (double *)realloc(mb_ping->beam_angle_forward, istore->beams_bath * sizeof(double));

        if (mb_ping->beam_flags == nullptr || mb_ping->depth == nullptr || mb_ping->across_track == nullptr ||
            mb_ping->along_track == nullptr || mb_ping->travel_time == nullptr || mb_ping->beam_angle_forward == nullptr ||
            mb_ping->beam_angle == nullptr) {
          status = MB_FAILURE;
          *error = MB_ERROR_MEMORY_FAIL;
        }
      }
      if (istore->beams_amp > 0) {
        mb_ping->mr_amplitude = (double *)realloc(mb_ping->mr_amplitude, istore->beams_amp * sizeof(double));
        if (mb_ping->mr_amplitude == nullptr) {
          status = MB_FAILURE;
          *error = MB_ERROR_MEMORY_FAIL;
        }
      }

      /* if ping flag set check for any unset
          beam flags - unset ping flag if any
          good beams found */
      if (mb_ping->ping_flags != 0) {
        for (int i = 0; i < istore->beams_bath; i++) {
          if (mb_beam_ok(istore->beamflag[i]))
            mb_ping->ping_flags = 0;
        }
      }

      /* read depth and beam location values into storage arrays */
      const int icenter = istore->beams_bath / 2;
      angscale = ((double)istore->beam_width_num) / ((double)istore->beam_width_denom);
      for (int i = 0; i < istore->beams_bath; i++) {
        mb_ping->beam_flags[i] = istore->beamflag[i];
        if (istore->beamflag[i] != MB_FLAG_NULL) {
          mb_ping->depth[i] = istore->bath[i];
          mb_ping->across_track[i] = istore->bath_acrosstrack[i];
          mb_ping->along_track[i] = istore->bath_alongtrack[i];
          mb_ping->travel_time[i] = 0.25 * (double)istore->range[i] / (double)istore->sample_rate;
          alpha = istore->png_pitch;
          beta = 90.0 + (icenter - i) * angscale + istore->png_roll;
          double theta;
          double phi;
          mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
          mb_ping->beam_angle[i] = theta;
          if (phi < 0.0)
            phi += 360.0;
          if (phi > 360.0)
            phi -= 360.0;
          mb_ping->beam_angle_forward[i] = phi;
        }
        else {
          mb_ping->depth[i] = 0.0;
          mb_ping->across_track[i] = 0.0;
          mb_ping->along_track[i] = 0.0;
          mb_ping->travel_time[i] = 0.0;
          mb_ping->beam_angle[i] = 0.0;
          mb_ping->beam_angle_forward[i] = 0;
        }
      }
      for (int i = 0; i < istore->beams_amp; i++) {
        mb_ping->mr_amplitude[i] = istore->amp[i];
      }

      /* choose gain factor -it's a guess based on dataset
        regression analysis!! rcc */
      gain_correction = 2.2 * (istore->gain & 63) + 6 * istore->power;

      /* read amplitude values into storage arrays */
      if (mb_ping->mc_amplitude != nullptr) {
        for (int i = 0; i < istore->beams_amp; i++) {
          /* note - we are storing 1/2 db increments */
          mb_ping->mc_amplitude[i] = 40 * log10(istore->intensity[i]);
        }
      }
      else if (mb_ping->mr_amplitude != nullptr) {
        for (int i = 0; i < istore->beams_amp; i++) {
          mb_ping->mr_amplitude[i] = 40 * log10(istore->intensity[i]) - gain_correction;
        }
      }

      /* generate imagery from sidescan trace
          code here would have to be modified for snippets

              mb_ping->brb_inten = (gsfBRBIntensity *) realloc(mb_ping->brb_inten,sizeof(gsfBRBIntensity));

          gsfBRBIntensity *brb;
          brb = mb_ping->brb_inten;
          brb->bits_per_sample = (unsigned char) 16;
          brb->applied_corrections = 0;
          gsfTimeSeriesIntensity *brb_ts;
          *brb_ts = brb->time_series;
          *brb_ts = (gsfTimeSeriesIntensity*)realloc(*brb_ts,istore->beams_bath*sizeof(gsfTimeSeriesIntensity));
          */

      /* now fill in the reson 8100 specific fields */
      mb_ping->sensor_id = GSF_SWATH_BATHY_SUBRECORD_RESON_8101_SPECIFIC;
      mb_ping->sensor_data.gsfReson8100Specific.latency = istore->latency;
      mb_ping->sensor_data.gsfReson8100Specific.ping_number = istore->ping_number;
      mb_ping->sensor_data.gsfReson8100Specific.sonar_id = istore->sonar_id;
      mb_ping->sensor_data.gsfReson8100Specific.sonar_model = istore->sonar_model;
      mb_ping->sensor_data.gsfReson8100Specific.frequency = istore->frequency;
      mb_ping->sensor_data.gsfReson8100Specific.surface_velocity = istore->velocity;
      mb_ping->sensor_data.gsfReson8100Specific.sample_rate = istore->sample_rate;
      mb_ping->sensor_data.gsfReson8100Specific.ping_rate = istore->ping_rate;
      mb_ping->sensor_data.gsfReson8100Specific.mode = GSF_8100_AMPLITUDE;
      mb_ping->sensor_data.gsfReson8100Specific.range = istore->range_set;
      mb_ping->sensor_data.gsfReson8100Specific.power = istore->power;
      mb_ping->sensor_data.gsfReson8100Specific.gain = istore->gain;
      mb_ping->sensor_data.gsfReson8100Specific.pulse_width = istore->pulse_width;
      mb_ping->sensor_data.gsfReson8100Specific.tvg_spreading = istore->tvg_spread;
      mb_ping->sensor_data.gsfReson8100Specific.tvg_absorption = istore->tvg_absorp;
      mb_ping->sensor_data.gsfReson8100Specific.fore_aft_bw = istore->projector_beam_width / 10.;
      mb_ping->sensor_data.gsfReson8100Specific.athwart_bw =
          (double)istore->beam_width_num / (double)istore->beam_width_denom;
      mb_ping->sensor_data.gsfReson8100Specific.projector_type = istore->projector_type;
      mb_ping->sensor_data.gsfReson8100Specific.projector_angle = istore->projector_angle;
      mb_ping->sensor_data.gsfReson8100Specific.range_filt_min = istore->min_range;
      mb_ping->sensor_data.gsfReson8100Specific.range_filt_max = istore->max_range;
      mb_ping->sensor_data.gsfReson8100Specific.depth_filt_min = istore->min_depth;
      mb_ping->sensor_data.gsfReson8100Specific.depth_filt_max = istore->max_depth;
      mb_ping->sensor_data.gsfReson8100Specific.filters_active = istore->filters_active;
      mb_ping->sensor_data.gsfReson8100Specific.temperature = istore->temperature;
      mb_ping->sensor_data.gsfReson8100Specific.beam_spacing =
          (double)istore->beam_width_num / (double)istore->beam_width_denom;

      /* set the GSF scale factors for this ping */
      status = mbsys_gsf_setscalefactors(verbose, true, mb_ping, error);
    }

    /* insert comment in structure */
    else if (istore->kind == MB_DATA_COMMENT) {
      dataID->recordID = GSF_RECORD_COMMENT;
      if (records->comment.comment_length < strlen(istore->comment) + 1) {
        if ((records->comment.comment = (char *)realloc(records->comment.comment, strlen(istore->comment) + 1)) == nullptr) {
          status = MB_FAILURE;
          *error = MB_ERROR_MEMORY_FAIL;
          records->comment.comment_length = 0;
        }
      }
      if ((status = MB_SUCCESS) && (records->comment.comment != nullptr)) {
        strcpy(records->comment.comment, istore->comment);
        records->comment.comment_length = strlen(istore->comment) + 1;
        records->comment.comment_time.tv_sec = (int)istore->png_time_d;
        records->comment.comment_time.tv_nsec =
            (int)(1000000000 * (istore->png_time_d - records->comment.comment_time.tv_sec));
      }
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
#endif  // ENABLE_GSF

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

  int fbtversion;
  status &= mb_fbtversion(verbose, &fbtversion);

  char commentfile[MB_PATH_MAXLINE] = "";
  bool insertcomments = false;
  bool bathonly = false;
  int iformat = 0;
  int oformat = 0;
  int mformat = 0;
  char ifile[MB_PATH_MAXLINE] = "stdin";
  char mfile[MB_PATH_MAXLINE] = "";
  bool merge = false;
  strip_mode_t stripmode = MBCOPY_STRIPMODE_NONE;
  char ofile[MB_PATH_MAXLINE] = "stdout";
  double sleep_factor = 1.0;
  bool use_sleep = false;

  {
    static struct option options[] = {{"begin-time", required_argument, nullptr, 0},
                                      {"comment-file", required_argument, nullptr, 0},
                                      {"bathymetry-only", no_argument, nullptr, 0},
                                      {"end-time", required_argument, nullptr, 0},
                                      {"format", required_argument, nullptr, 0},
                                      {"help", no_argument, nullptr, 0},
                                      {"input", required_argument, nullptr, 0},
                                      {"lonflip", required_argument, nullptr, 0},
                                      {"merge-file", required_argument, nullptr, 0},
                                      {"strip-comments", no_argument, nullptr, 0},
                                      {"output", required_argument, nullptr, 0},
                                      {"ping-average", required_argument, nullptr, 0},
                                      {"sleep-factor", required_argument, nullptr, 0},
                                      {"bounds", required_argument, nullptr, 0},
                                      {"speed-minimum", required_argument, nullptr, 0},
                                      {"time-gap", required_argument, nullptr, 0},
                                      {"verbose", no_argument, nullptr, 0},
                                      {nullptr, 0, nullptr, 0}};

    int option_index;
    bool errflg = false;
    int c;
    bool help = false;
    while ((c = getopt_long(argc, argv, "B:b:C:c:DdE:e:F:f:HhI:i:L:l:M:m:NnO:o:P:p:Q:q:R:r:S:s:T:t:Vv", options, &option_index)) != -1)
      switch (c) {
      /* long options all return c=0 */
      case 0:
        if (strcmp("begin-time", options[option_index].name) == 0) {
          double seconds;
          sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &seconds);
          btime_i[5] = (int)floor(seconds);
          btime_i[6] = 1000000 * (seconds - btime_i[5]);
        }
        else if (strcmp("comment-file", options[option_index].name) == 0) {
          sscanf(optarg, "%1023s", commentfile);
          insertcomments = true;
        }
        else if (strcmp("bathymetry-only", options[option_index].name) == 0) {
          bathonly = true;
        }
        else if (strcmp("end-time", options[option_index].name) == 0) {
          double seconds;
          sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &seconds);
          etime_i[5] = (int)floor(seconds);
          etime_i[6] = 1000000 * (seconds - etime_i[5]);
        }
        else if (strcmp("format", options[option_index].name) == 0) {
          const int i = sscanf(optarg, "%d/%d/%d", &iformat, &oformat, &mformat);
          if (i == 1)
            oformat = iformat;
        }
        else if (strcmp("help", options[option_index].name) == 0) {
          help = true;
        }
        else if (strcmp("input", options[option_index].name) == 0) {
          sscanf(optarg, "%1023s", ifile);
        }
        else if (strcmp("lonflip", options[option_index].name) == 0) {
          sscanf(optarg, "%d", &lonflip);
        }
        else if (strcmp("merge-file", options[option_index].name) == 0) {
          const int i = sscanf(optarg, "%1023s", mfile);
          if (i == 1)
            merge = true;
        }
        else if (strcmp("strip-comments", options[option_index].name) == 0) {
          if (stripmode == MBCOPY_STRIPMODE_NONE) {
            stripmode = MBCOPY_STRIPMODE_COMMENTS;
          } else if (stripmode == MBCOPY_STRIPMODE_COMMENTS) {
            stripmode = MBCOPY_STRIPMODE_BATHYONLY;
          } else {
            fprintf(stderr, "Failure: Gave -n more than twice.\n");
            exit(MB_ERROR_BAD_USAGE);
          }
        }
        else if (strcmp("output", options[option_index].name) == 0) {
          sscanf(optarg, "%1023s", ofile);
        }
        else if (strcmp("ping-average", options[option_index].name) == 0) {
          sscanf(optarg, "%d", &pings);
        }
        else if (strcmp("sleep-factor", options[option_index].name) == 0) {
          sscanf(optarg, "%lf", &sleep_factor);
          use_sleep = true;
        }
        else if (strcmp("bounds", options[option_index].name) == 0) {
          mb_get_bounds(optarg, bounds);
        }
        else if (strcmp("speed-minimum", options[option_index].name) == 0) {
          sscanf(optarg, "%lf", &speedmin);
        }
        else if (strcmp("time-gap", options[option_index].name) == 0) {
          sscanf(optarg, "%lf", &timegap);
        }
        else if (strcmp("verbose", options[option_index].name) == 0) {
          verbose++;
        }
        break;

      case 'B':
      case 'b':
      {
        double seconds;
        sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &btime_i[0], &btime_i[1], &btime_i[2], &btime_i[3], &btime_i[4], &seconds);
        btime_i[5] = (int)floor(seconds);
        btime_i[6] = 1000000 * (seconds - btime_i[5]);
        break;
      }
      case 'C':
      case 'c':
        sscanf(optarg, "%1023s", commentfile);
        insertcomments = true;
        break;
      case 'D':
      case 'd':
        bathonly = true;
        break;
      case 'E':
      case 'e':
      {
        double seconds;
        sscanf(optarg, "%d/%d/%d/%d/%d/%lf", &etime_i[0], &etime_i[1], &etime_i[2], &etime_i[3], &etime_i[4], &seconds);
        etime_i[5] = (int)floor(seconds);
        etime_i[6] = 1000000 * (seconds - etime_i[5]);
        break;
      }
      case 'F':
      case 'f':
      {
        const int i = sscanf(optarg, "%d/%d/%d", &iformat, &oformat, &mformat);
        if (i == 1)
          oformat = iformat;
        break;
      }
      case 'H':
      case 'h':
        help = true;
        break;
      case 'I':
      case 'i':
        sscanf(optarg, "%1023s", ifile);
        break;
      case 'L':
      case 'l':
        sscanf(optarg, "%d", &lonflip);
        break;
      case 'M':
      case 'm':
      {
        const int i = sscanf(optarg, "%1023s", mfile);
        if (i == 1)
          merge = true;
        break;
      }
      case 'N':
      case 'n':
        if (stripmode == MBCOPY_STRIPMODE_NONE) {
          stripmode = MBCOPY_STRIPMODE_COMMENTS;
        } else if (stripmode == MBCOPY_STRIPMODE_COMMENTS) {
          stripmode = MBCOPY_STRIPMODE_BATHYONLY;
        } else {
          fprintf(stderr, "Failure: Gave -n more than twice.\n");
          exit(MB_ERROR_BAD_USAGE);
        }
        break;
      case 'O':
      case 'o':
        sscanf(optarg, "%1023s", ofile);
        break;
      case 'P':
      case 'p':
        sscanf(optarg, "%d", &pings);
        break;
      case 'Q':
      case 'q':
        sscanf(optarg, "%lf", &sleep_factor);
        use_sleep = true;
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
      fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
      fprintf(stderr, "dbg2       help:           %d\n", help);
      fprintf(stderr, "dbg2       pings:          %d\n", pings);
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
      fprintf(stderr, "dbg2       input format:   %d\n", iformat);
      fprintf(stderr, "dbg2       output format:  %d\n", oformat);
      fprintf(stderr, "dbg2       merge format:   %d\n", mformat);
      fprintf(stderr, "dbg2       input file:     %s\n", ifile);
      fprintf(stderr, "dbg2       output file:    %s\n", ofile);
      fprintf(stderr, "dbg2       merge file:     %s\n", mfile);
      fprintf(stderr, "dbg2       insert comments:%d\n", insertcomments);
      fprintf(stderr, "dbg2       comment file:   %s\n", commentfile);
      fprintf(stderr, "dbg2       stripmode:      %d\n", stripmode);
      fprintf(stderr, "dbg2       bath only:      %d\n", bathonly);
      fprintf(stderr, "dbg2       use sleep:      %d\n", use_sleep);
      fprintf(stderr, "dbg2       sleep factor:   %f\n", sleep_factor);
      fprintf(stderr, "dbg2       fbtversion:     %d\n", fbtversion);
    }

    if (help) {
      fprintf(stderr, "\n%s\n", help_message);
      fprintf(stderr, "\nusage: %s\n", usage_message);
      exit(MB_ERROR_NO_ERROR);
    }
  }

  int error = MB_ERROR_NO_ERROR;

  if (format == 0)
    mb_get_format(verbose, ifile, nullptr, &format, &error);

  /* MBIO read control parameters */
  double btime_d;
  double etime_d;
  int ibeams_bath;
  int ibeams_amp;
  int ipixels_ss;
  void *imbio_ptr = nullptr;

  /* MBIO write control parameters */
  int obeams_bath;
  int obeams_amp;
  int opixels_ss;
  void *ombio_ptr = nullptr;

  /* MBIO merge control parameters */
  int mbeams_bath;
  int mbeams_amp;
  int mpixels_ss;
  void *mmbio_ptr = nullptr;

  /* MBIO read and write values */
  struct mb_io_struct *omb_io_ptr;
  struct mb_io_struct *imb_io_ptr;
  void *istore_ptr;
  void *ostore_ptr;
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
  char *ibeamflag = nullptr;
  double *ibath = nullptr;
  double *ibathacrosstrack = nullptr;
  double *ibathalongtrack = nullptr;
  double *iamp = nullptr;
  double *iss = nullptr;
  double *issacrosstrack = nullptr;
  double *issalongtrack = nullptr;
  char *obeamflag = nullptr;
  double *obath = nullptr;
  double *obathacrosstrack = nullptr;
  double *obathalongtrack = nullptr;
  double *oamp = nullptr;
  double *oss = nullptr;
  double *ossacrosstrack = nullptr;
  double *ossalongtrack = nullptr;
  double draft;
  double roll;
  double pitch;
  double heave;

  int merror = MB_ERROR_NO_ERROR;
  int mkind = MB_DATA_NONE;
  int mpings = 0;
  int mtime_i[7];
  double mtime_d = 0.0;
  double mnavlon;
  double mnavlat;
  double mspeed;
  double mheading;
  double mdistance;
  double maltitude;
  double msensordepth;

  int sensorhead_error = MB_ERROR_NO_ERROR;
  int sensorhead = 0;
  int sensortype = 0;

  char mcomment[MB_COMMENT_MAXLINE];
  int mnbath, mnamp, mnss;
  char *mbeamflag = nullptr;
  double *mbath = nullptr;
  double *mbathacrosstrack = nullptr;
  double *mbathalongtrack = nullptr;
  double *mamp = nullptr;
  double *mss = nullptr;
  double *mssacrosstrack = nullptr;
  double *mssalongtrack = nullptr;
  int idata = 0;
  int icomment = 0;
  int odata = 0;
  int ocomment = 0;
  int nbath, namp, nss;
  int istart_bath, iend_bath, offset_bath;
  int istart_amp, iend_amp, offset_amp;
  int istart_ss, iend_ss, offset_ss;
  char comment[MB_COMMENT_MAXLINE];
  copy_mode_t copymode = MBCOPY_PARTIAL;

  /* sleep variable */
  double time_d_last;
  unsigned int sleep_time;

  FILE *fp;
  char *result;

  /* settle the input/output formats */
  if (iformat <= 0 && oformat <= 0) {
    iformat = format;
    oformat = format;
  }
  else if (iformat > 0 && oformat <= 0)
    oformat = iformat;

  if (merge && mformat <= 0)
    mb_get_format(verbose, mfile, nullptr, &mformat, &error);

  /* obtain format array locations - format ids will
      be aliased to current ids if old format ids given */
  if ((status = mb_format(verbose, &iformat, &error)) != MB_SUCCESS) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_format> regarding input format %d:\n%s\n", iformat, message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }
  if ((status = mb_format(verbose, &oformat, &error)) != MB_SUCCESS) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_format> regarding output format %d:\n%s\n", oformat, message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }
  if (merge && (status = mb_format(verbose, &mformat, &error)) != MB_SUCCESS) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_format> regarding merge format %d:\n%s\n", mformat, message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }

  /* initialize reading the input swath sonar file */
  if (mb_read_init(verbose, ifile, iformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &imbio_ptr,
                             &btime_d, &etime_d, &ibeams_bath, &ibeams_amp, &ipixels_ss, &error) != MB_SUCCESS) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
    fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", ifile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }
  imb_io_ptr = (struct mb_io_struct *)imbio_ptr;

  /* initialize reading the merge swath sonar file */
  if (merge &&
      (mb_read_init(verbose, mfile, mformat, pings, lonflip, bounds, btime_i, etime_i, speedmin, timegap, &mmbio_ptr,
                    &btime_d, &etime_d, &mbeams_bath, &mbeams_amp, &mpixels_ss, &error) != MB_SUCCESS)) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_read_init>:\n%s\n", message);
    fprintf(stderr, "\nMultibeam File <%s> not initialized for reading\n", mfile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }

  /* initialize writing the output swath sonar file */
  if ((status = mb_write_init(verbose, ofile, oformat, &ombio_ptr, &obeams_bath, &obeams_amp, &opixels_ss, &error)) !=
      MB_SUCCESS) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error returned from function <mb_write_init>:\n%s\n", message);
    fprintf(stderr, "\nMultibeam File <%s> not initialized for writing\n", ofile);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }
  omb_io_ptr = (struct mb_io_struct *)ombio_ptr;

  /* bathonly mode works only if output format is mbldeoih */
  if (bathonly && oformat != MBF_MBLDEOIH) {
    bathonly = false;
    if (verbose > 0) {
      fprintf(stderr, "\nThe -D option (strip amplitude and sidescan) is only valid for output format %d\n", MBF_MBLDEOIH);
      fprintf(stderr, "Program %s is ignoring the -D argument\n", program_name);
    }
  }

  /* if bathonly mode for mbldeoih format, assume we are making an fbt file
      - set the format to use - this allows user to set use of old format
      in .mbio_defaults file - purpose is to keep compatibility with
      Fledermaus */
  if (bathonly && oformat == MBF_MBLDEOIH) {
    omb_io_ptr->save1 = fbtversion;
  }

    /* if stripmode set to more than strip comments, then set flag in imb_io_ptr */
    if (stripmode > MBCOPY_STRIPMODE_COMMENTS) {
        omb_io_ptr->save15 = true;
    }

  /* determine if full or partial copies will be made */
  if (pings == 1 && imb_io_ptr->system != MB_SYS_NONE && imb_io_ptr->system == omb_io_ptr->system)
    copymode = MBCOPY_FULL;
  else if (pings == 1 && imb_io_ptr->system == MB_SYS_ELACMK2 && omb_io_ptr->system == MB_SYS_XSE)
    copymode = MBCOPY_ELACMK2_TO_XSE;
  else if (pings == 1 && imb_io_ptr->system == MB_SYS_XSE && omb_io_ptr->system == MB_SYS_ELACMK2)
    copymode = MBCOPY_XSE_TO_ELACMK2;
  else if (pings == 1 && imb_io_ptr->system == MB_SYS_SIMRAD && omb_io_ptr->format == MBF_EM300MBA)
    copymode = MBCOPY_SIMRAD_TO_SIMRAD2;
  else if (pings == 1 && omb_io_ptr->format == MBF_MBLDEOIH)
    copymode = MBCOPY_ANY_TO_MBLDEOIH;
#ifdef ENABLE_GSF
  else if (pings == 1 && imb_io_ptr->format == MBF_XTFR8101 && omb_io_ptr->format == MBF_GSFGENMB)
    copymode = MBCOPY_RESON8K_TO_GSF;
#endif
  else
    copymode = MBCOPY_PARTIAL;

#ifdef ENABLE_GSF
  /* quit if an unsupported copy to GSF is requested */
  if (omb_io_ptr->format == MBF_GSFGENMB && copymode == MBCOPY_PARTIAL) {
    fprintf(stderr, "Requested copy from format %d to GSF format %d is unsupported\n", imb_io_ptr->format,
            omb_io_ptr->format);
    fprintf(stderr, "Please consider writing the necessary translation code for mbcopy.c \n");
    fprintf(stderr, "\tand contributing it to the MB-System community\n");
    exit(error);
  }
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  Copy mode set in program <%s>\n", program_name);
    fprintf(stderr, "dbg2       pings:         %d\n", pings);
    fprintf(stderr, "dbg2       iformat:       %d\n", iformat);
    fprintf(stderr, "dbg2       oformat:       %d\n", oformat);
    fprintf(stderr, "dbg2       isystem:       %d\n", imb_io_ptr->system);
    fprintf(stderr, "dbg2       osystem:       %d\n", omb_io_ptr->system);
    fprintf(stderr, "dbg2       copymode:      %d\n", copymode);
  }

  /* allocate memory for data arrays */
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&ibeamflag, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ibath, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&iamp, &error);
  if (error == MB_ERROR_NO_ERROR)
    status =
        mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ibathacrosstrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&ibathalongtrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&iss, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&issacrosstrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, imbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&issalongtrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&obeamflag, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&obath, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&oamp, &error);
  if (error == MB_ERROR_NO_ERROR)
    status =
        mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&obathacrosstrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&obathalongtrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&oss, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ossacrosstrack, &error);
  if (error == MB_ERROR_NO_ERROR)
    status = mb_register_array(verbose, ombio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&ossalongtrack, &error);

  if (merge) {
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(char), (void **)&mbeamflag, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&mbath, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_AMPLITUDE, sizeof(double), (void **)&mamp, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&mbathacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_BATHYMETRY, sizeof(double), (void **)&mbathalongtrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&mss, &error);
    if (error == MB_ERROR_NO_ERROR)
      status =
          mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&mssacrosstrack, &error);
    if (error == MB_ERROR_NO_ERROR)
      status = mb_register_array(verbose, mmbio_ptr, MB_MEM_TYPE_SIDESCAN, sizeof(double), (void **)&mssalongtrack, &error);
  }

  /* if error initializing memory then quit */
  if (error != MB_ERROR_NO_ERROR) {
    char *message;
    mb_error(verbose, error, &message);
    fprintf(stderr, "\nMBIO Error allocating data arrays:\n%s\n", message);
    fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
    exit(error);
  }

  /* set up transfer rules */
  if (omb_io_ptr->variable_beams && obeams_bath != ibeams_bath)
    obeams_bath = ibeams_bath;
  if (omb_io_ptr->variable_beams && obeams_amp != ibeams_amp)
    obeams_amp = ibeams_amp;
  if (omb_io_ptr->variable_beams && opixels_ss != ipixels_ss)
    opixels_ss = ipixels_ss;
  setup_transfer_rules(verbose, ibeams_bath, obeams_bath, &istart_bath, &iend_bath, &offset_bath, &error);
  setup_transfer_rules(verbose, ibeams_amp, obeams_amp, &istart_amp, &iend_amp, &offset_amp, &error);
  setup_transfer_rules(verbose, ipixels_ss, opixels_ss, &istart_ss, &iend_ss, &offset_ss, &error);

  /* insert comments from file into output */
  if (insertcomments) {
    /* open file */
    if ((fp = fopen(commentfile, "r")) == nullptr) {
      fprintf(stderr, "\nUnable to Open Comment File <%s> for reading\n", commentfile);
      fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
      exit(MB_ERROR_OPEN_FAIL);
    }

    /* read and output comment lines */
    strncpy(comment, "", 256);
    while ((result = fgets(comment, 256, fp)) == comment) {
      kind = MB_DATA_COMMENT;
      comment[(int)strlen(comment) - 1] = '\0';
      status = mb_put_comment(verbose, ombio_ptr, comment, &error);
      if (error == MB_ERROR_NO_ERROR)
        ocomment++;
    }

    /* close the file */
    fclose(fp);
  }

  /* write comments to beginning of output file */
  if (stripmode == MBCOPY_STRIPMODE_NONE) {
    kind = MB_DATA_COMMENT;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "These data copied by program %s", program_name);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "MB-system Version %s", MB_VERSION);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    char user[256], host[256], date[32];
    status = mb_user_host_date(verbose, user, host, date, &error);
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "Run by user <%s> on cpu <%s> at <%s>", user, host, date);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "Control Parameters:");
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Input file:         %s", ifile);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Input MBIO format:  %d", iformat);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    if (merge) {
      strncpy(comment, "", 256);
      snprintf(comment, sizeof(comment), "  Merge file:         %s", mfile);
      status = mb_put_comment(verbose, ombio_ptr, comment, &error);
      if (error == MB_ERROR_NO_ERROR)
        ocomment++;
      strncpy(comment, "", 256);
      snprintf(comment, sizeof(comment), "  Merge MBIO format:  %d", mformat);
      status = mb_put_comment(verbose, ombio_ptr, comment, &error);
      if (error == MB_ERROR_NO_ERROR)
        ocomment++;
    }
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Output file:        %s", ofile);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Output MBIO format: %d", oformat);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Ping averaging:     %d", pings);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Longitude flip:     %d", lonflip);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Longitude bounds:   %f %f", bounds[0], bounds[1]);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Latitude bounds:    %f %f", bounds[2], bounds[3]);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Begin time:         %d %d %d %d %d %d %d", btime_i[0], btime_i[1], btime_i[2], btime_i[3], btime_i[4],
            btime_i[5], btime_i[6]);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  End time:           %d %d %d %d %d %d %d", etime_i[0], etime_i[1], etime_i[2], etime_i[3], etime_i[4],
            etime_i[5], etime_i[6]);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Minimum speed:      %f", speedmin);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), "  Time gap:           %f", timegap);
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
    strncpy(comment, "", 256);
    snprintf(comment, sizeof(comment), " ");
    status = mb_put_comment(verbose, ombio_ptr, comment, &error);
    if (error == MB_ERROR_NO_ERROR)
      ocomment++;
  }

  /* start expecting data to be in time and space bounds */
  bool inbounds = true;

  /* read and write */
  while (error <= MB_ERROR_NO_ERROR) {

    /* read some data */
    error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;
    inbounds = true;
    if (copymode != MBCOPY_PARTIAL) {
      status = mb_get_all(verbose, imbio_ptr, &istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading,
                          &distance, &altitude, &sensordepth, &nbath, &namp, &nss, ibeamflag, ibath, iamp, ibathacrosstrack,
                          ibathalongtrack, iss, issacrosstrack, issalongtrack, comment, &error);
    }
    else {
      status = mb_get(verbose, imbio_ptr, &kind, &pings, time_i, &time_d, &navlon, &navlat, &speed, &heading, &distance,
                      &altitude, &sensordepth, &nbath, &namp, &nss, ibeamflag, ibath, iamp, ibathacrosstrack,
                      ibathalongtrack, iss, issacrosstrack, issalongtrack, comment, &error);
    }

    /* increment counters and clear errors associated */
    if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_DATA)
      idata = idata + pings;
    else if (error <= MB_ERROR_NO_ERROR && kind == MB_DATA_COMMENT)
      icomment++;

    /* time gaps do not matter to mbcopy */
    if (error == MB_ERROR_TIME_GAP) {
      status = MB_SUCCESS;
      error = MB_ERROR_NO_ERROR;
    }

    /* check for survey data in or out of bounds */
    if (kind == MB_DATA_DATA) {
      if (error == MB_ERROR_NO_ERROR)
        inbounds = true;
      else if (error == MB_ERROR_OUT_BOUNDS || error == MB_ERROR_OUT_TIME)
        inbounds = false;
    }

    if (merge && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && inbounds) {
      while (merror <= MB_ERROR_NO_ERROR && (mkind != MB_DATA_DATA || time_d - .001 > mtime_d)) {
        /* find merge record */

        /* int mstatus = */
        mb_get(verbose, mmbio_ptr, &mkind, &mpings, mtime_i, &mtime_d, &mnavlon, &mnavlat, &mspeed, &mheading,
               &mdistance, &maltitude, &msensordepth, &mnbath, &mnamp, &mnss, mbeamflag, mbath, mamp,
               mbathacrosstrack, mbathalongtrack, mss, mssacrosstrack, mssalongtrack, mcomment, &merror);
      }

      if (time_d + .001 < mtime_d || merror > 0) {
        inbounds = false;
      }
    }

    /* check numbers of input and output beams */
    if (copymode == MBCOPY_PARTIAL && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && nbath != ibeams_bath) {
      ibeams_bath = nbath;
      if (omb_io_ptr->variable_beams)
        obeams_bath = ibeams_bath;
      setup_transfer_rules(verbose, ibeams_bath, obeams_bath, &istart_bath, &iend_bath, &offset_bath, &error);
    }
    if (copymode == MBCOPY_PARTIAL && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && namp != ibeams_amp) {
      ibeams_amp = namp;
      if (omb_io_ptr->variable_beams)
        obeams_amp = ibeams_amp;
      setup_transfer_rules(verbose, ibeams_amp, obeams_amp, &istart_amp, &iend_amp, &offset_amp, &error);
    }
    if (copymode == MBCOPY_PARTIAL && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR && nss != ipixels_ss) {
      ipixels_ss = nss;
      if (omb_io_ptr->variable_beams)
        opixels_ss = ipixels_ss;
      setup_transfer_rules(verbose, ipixels_ss, opixels_ss, &istart_ss, &iend_ss, &offset_ss, &error);
    }

    /* output error messages */
    if (verbose >= 1) {
      if (error == MB_ERROR_COMMENT) {
        if (icomment == 1)
          fprintf(stderr, "\nComments:\n");
        fprintf(stderr, "%s\n", comment);
      }
      else if (kind == MB_DATA_DATA && error < MB_ERROR_NO_ERROR && error >= MB_ERROR_OTHER) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
        fprintf(stderr, "Input Record: %d\n", idata);
        fprintf(stderr, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
                time_i[6]);
      }
      else if (kind == MB_DATA_DATA && error < MB_ERROR_NO_ERROR) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nNonfatal MBIO Error:\n%s\n", message);
        fprintf(stderr, "Number of good records so far: %d\n", idata);
      }
      else if (error > MB_ERROR_NO_ERROR && error != MB_ERROR_EOF) {
        char *message;
        mb_error(verbose, error, &message);
        fprintf(stderr, "\nFatal MBIO Error:\n%s\n", message);
        fprintf(stderr, "Last Good Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4],
                time_i[5], time_i[6]);
      }
    }

    /* do sleep if required */
    if (use_sleep && kind == MB_DATA_DATA && error <= MB_ERROR_NO_ERROR && idata == 1) {
      time_d_last = time_d;
    }
    else if (use_sleep && kind == MB_DATA_DATA && error <= MB_ERROR_NO_ERROR && idata > 1) {
      sleep_time = (unsigned int)(sleep_factor * (time_d - time_d_last));
      sleep(sleep_time);
      time_d_last = time_d;
    }

    /* process some data */
    if (copymode == MBCOPY_PARTIAL && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR) {
      /* zero bathymetry */
      for (int j = 0; j < offset_bath; j++) {
        obeamflag[j] = MB_FLAG_NULL;
        obath[j] = 0.0;
        obathacrosstrack[j] = 0.0;
        obathalongtrack[j] = 0.0;
      }

      /* do bathymetry */
      if (merge) {
        /* merge data */
        for (int i = istart_bath; i < iend_bath; i++) {
          const int j = i + offset_bath;
          obeamflag[j] = mbeamflag[i];
          obath[j] = mbath[i];
          obathacrosstrack[j] = mbathacrosstrack[i];
          obathalongtrack[j] = mbathalongtrack[i];
        }
      }
      else {
        for (int i = istart_bath; i < iend_bath; i++) {
          const int j = i + offset_bath;
          obeamflag[j] = ibeamflag[i];
          obath[j] = ibath[i];
          obathacrosstrack[j] = ibathacrosstrack[i];
          obathalongtrack[j] = ibathalongtrack[i];
        }
      }
      for (int j = iend_bath + offset_bath; j < obeams_bath; j++) {
        obeamflag[j] = MB_FLAG_NULL;
        obath[j] = 0.0;
        obathacrosstrack[j] = 0.0;
        obathalongtrack[j] = 0.0;
      }

      /* do amplitudes */
      for (int j = 0; j < offset_amp; j++) {
        oamp[j] = 0.0;
      }
      for (int i = istart_amp; i < iend_amp; i++) {
        const int j = i + offset_amp;
        oamp[j] = iamp[i];
      }
      for (int j = iend_amp + offset_amp; j < obeams_amp; j++) {
        oamp[j] = 0.0;
      }

      /* do sidescan */
      for (int j = 0; j < offset_ss; j++) {
        oss[j] = 0.0;
        ossacrosstrack[j] = 0.0;
        ossalongtrack[j] = 0.0;
      }
      for (int i = istart_ss; i < iend_ss; i++) {
        const int j = i + offset_ss;
        oss[j] = iss[i];
        ossacrosstrack[j] = issacrosstrack[i];
        ossalongtrack[j] = issalongtrack[i];
      }
      for (int j = iend_ss + offset_ss; j < opixels_ss; j++) {
        oss[j] = 0.0;
        ossacrosstrack[j] = 0.0;
        ossalongtrack[j] = 0.0;
      }
    }

    /* handle special full translation cases */
    if (copymode == MBCOPY_FULL && error == MB_ERROR_NO_ERROR) {
      ostore_ptr = istore_ptr;
    }
    else if (copymode == MBCOPY_ELACMK2_TO_XSE && error == MB_ERROR_NO_ERROR) {
      ostore_ptr = omb_io_ptr->store_data;
      status = mbcopy_elacmk2_to_xse(verbose, static_cast<mbsys_elacmk2_struct *>(istore_ptr),
                                     static_cast<mbsys_xse_struct *>(ostore_ptr), &error);
    }
    else if (copymode == MBCOPY_XSE_TO_ELACMK2 && error == MB_ERROR_NO_ERROR) {
      ostore_ptr = omb_io_ptr->store_data;
      status = mbcopy_xse_to_elacmk2(verbose, static_cast<mbsys_xse_struct *>(istore_ptr), static_cast<mbsys_elacmk2_struct *>(ostore_ptr), &error);
    }
    else if (copymode == MBCOPY_SIMRAD_TO_SIMRAD2 && error == MB_ERROR_NO_ERROR) {
      ostore_ptr = omb_io_ptr->store_data;
      status = mbsys_simrad2_from_simrad(verbose, static_cast<mbsys_simrad_struct *>(istore_ptr), static_cast<mbsys_simrad2_struct *>(ostore_ptr), &error);
    }
#ifdef ENABLE_GSF
    else if (copymode == MBCOPY_RESON8K_TO_GSF && error == MB_ERROR_NO_ERROR) {

      ostore_ptr = omb_io_ptr->store_data;
      status = mbcopy_reson8k_to_gsf(verbose, imbio_ptr, ombio_ptr, &error);
    }
#endif
    else if (copymode == MBCOPY_ANY_TO_MBLDEOIH && error == MB_ERROR_NO_ERROR) {
      if (kind == MB_DATA_DATA) {
        mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &draft,
                       &roll, &pitch, &heave, &error);
        /* int sensorhead_status = */ mb_sensorhead(verbose, imbio_ptr, istore_ptr, &sensorhead, &sensorhead_error);
        /* sensorhead_status = */ mb_sonartype(verbose, imbio_ptr, istore_ptr, &sensortype, &sensorhead_error);
      }
      ostore_ptr = omb_io_ptr->store_data;
      if (kind == MB_DATA_DATA || kind == MB_DATA_COMMENT) {
        /* strip amplitude and sidescan if requested */
        if (bathonly) {
          namp = 0;
          nss = 0;
        }

        /* copy the data to mbldeoih */
        if (merge) {
          status =
              mbcopy_any_to_mbldeoih(verbose, kind, sensorhead, sensortype,
                                               time_i, time_d, navlon, navlat, speed, heading, draft, altitude,
                                     roll, pitch, heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack,
                                     nbath, namp, nss, mbeamflag, mbath, iamp, mbathacrosstrack, mbathalongtrack, iss,
                                     issacrosstrack, issalongtrack, comment, ombio_ptr, ostore_ptr, &error);
        }
        else {
          status =
              mbcopy_any_to_mbldeoih(verbose, kind, sensorhead, sensortype,
                                               time_i, time_d, navlon, navlat, speed, heading, draft, altitude,
                                     roll, pitch, heave, imb_io_ptr->beamwidth_xtrack, imb_io_ptr->beamwidth_ltrack,
                                     nbath, namp, nss, ibeamflag, ibath, iamp, ibathacrosstrack, ibathalongtrack, iss,
                                     issacrosstrack, issalongtrack, comment, ombio_ptr, ostore_ptr, &error);
        }
      }
      else
        error = MB_ERROR_OTHER;
    }
    else if (copymode == MBCOPY_PARTIAL && error == MB_ERROR_NO_ERROR) {
      istore_ptr = imb_io_ptr->store_data;
      ostore_ptr = omb_io_ptr->store_data;
      if (pings == 1 && (kind == MB_DATA_DATA
                          || kind == MB_DATA_NAV ||kind == MB_DATA_NAV1
                          || kind == MB_DATA_NAV2 ||kind == MB_DATA_NAV3)) {
        mb_extract_nav(verbose, imbio_ptr, istore_ptr, &kind, time_i, &time_d, &navlon, &navlat, &speed, &heading, &draft,
                       &roll, &pitch, &heave, &error);
        mb_insert_nav(verbose, ombio_ptr, ostore_ptr, time_i, time_d, navlon, navlat, speed, heading, draft, roll, pitch,
                      heave, &error);
      }
      if (kind == MB_DATA_DATA) {
        status = mb_insert(verbose, ombio_ptr, ostore_ptr, kind, time_i, time_d, navlon, navlat, speed, heading, obeams_bath,
                         obeams_amp, opixels_ss, obeamflag, obath, oamp, obathacrosstrack, obathalongtrack, oss,
                         ossacrosstrack, ossalongtrack, comment, &error);
      }
    }

    if (merge && kind == MB_DATA_DATA && error == MB_ERROR_NO_ERROR) {
      switch (copymode) {
      case MBCOPY_PARTIAL:
      case MBCOPY_ANY_TO_MBLDEOIH:
        /* Already looked after */
        break;
      case MBCOPY_FULL:
      case MBCOPY_SIMRAD_TO_SIMRAD2:
      case MBCOPY_ELACMK2_TO_XSE:
      case MBCOPY_XSE_TO_ELACMK2:
#ifdef ENABLE_GSF
      case MBCOPY_RESON8K_TO_GSF:
#endif
        status = mb_insert(verbose, ombio_ptr, ostore_ptr, kind, time_i, time_d, navlon, navlat, speed, heading,
                           mbeams_bath, ibeams_amp, ipixels_ss, mbeamflag, mbath, iamp, mbathacrosstrack, mbathalongtrack,
                           iss, issacrosstrack, issalongtrack, comment, &error);
        break;
      }
    }

    /* write some data */
    if ((kind != MB_DATA_COMMENT && error == MB_ERROR_NO_ERROR && inbounds) ||
        (kind == MB_DATA_COMMENT && stripmode == MBCOPY_STRIPMODE_NONE)) {
      error = MB_ERROR_NO_ERROR;
      status = mb_put_all(verbose, ombio_ptr, ostore_ptr, false, kind, time_i, time_d, navlon, navlat, speed, heading,
                          obeams_bath, obeams_amp, opixels_ss, obeamflag, obath, oamp, obathacrosstrack, obathalongtrack,
                          oss, ossacrosstrack, ossalongtrack, comment, &error);
      if (status == MB_SUCCESS) {
        if (kind == MB_DATA_DATA)
          odata++;
        else if (kind == MB_DATA_COMMENT)
          ocomment++;
      }
      else {
        char *message;
        mb_error(verbose, error, &message);
        if (copymode != MBCOPY_PARTIAL)
          fprintf(stderr, "\nMBIO Error returned from function <mb_put_all>:\n%s\n", message);
        else
          fprintf(stderr, "\nMBIO Error returned from function <mb_put>:\n%s\n", message);
        fprintf(stderr, "\nMultibeam Data Not Written To File <%s>\n", ofile);
        fprintf(stderr, "Output Record: %d\n", odata + 1);
        fprintf(stderr, "Time: %d %d %d %d %d %d %d\n", time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5],
                time_i[6]);
        fprintf(stderr, "\nProgram <%s> Terminated\n", program_name);
        exit(error);
      }
    }
  }

  status &= mb_close(verbose, &imbio_ptr, &error);
  status &= mb_close(verbose, &ombio_ptr, &error);

  if (verbose >= 4)
    status &= mb_memory_list(verbose, &error);

  /* give the statistics */
  if (verbose >= 1) {
    fprintf(stderr, "\n%d input data records\n", idata);
    fprintf(stderr, "%d input comment records\n", icomment);
    fprintf(stderr, "%d output data records\n", odata);
    fprintf(stderr, "%d output comment records\n", ocomment);
  }

  exit(error);
}
/*--------------------------------------------------------------------*/
