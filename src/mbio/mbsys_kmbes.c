/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_kmbes.c  3.00  5/25/2018
 *
 *    Copyright (c) 2018-2020 by
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
 * mbsys_kmbes.c contains the MBIO functions for handling data from
 * the following data formats:
 *    MBSYS_KMBES formats (code in mbsys_kmbes.c and mbsys_kmbes.h):
 *      MBF_TEMPFORM : MBIO ID ??? (code in mbr_kmbes.c)
 *
 * Author:  D. W. Caress
 * Date:  May 25, 2018
 *
 *
 */

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_kmbes.h"

#ifdef _WIN32

/* Based on https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows */
#include <Windows.h>
#define CLOCK_REALTIME 0
#if defined(_MSC_VER) && (_MSC_VER <= 1800)
struct timespec { long tv_sec; long tv_nsec; };
#endif
static bool g_first_time = 1;
static LARGE_INTEGER g_counts_per_sec;

int clock_gettime(int dummy, struct timespec *ct) {
    if (g_first_time) {
        g_first_time = 0;

        if (0 == QueryPerformanceFrequency(&g_counts_per_sec)) {
            g_counts_per_sec.QuadPart = 0;
        }
    }

    LARGE_INTEGER count;
    if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) || (0 == QueryPerformanceCounter(&count))) {
        return -1;
    }

    ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
    ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * 1e9) / g_counts_per_sec.QuadPart;

    return 0;
}
#endif

/*--------------------------------------------------------------------*/
int mbsys_kmbes_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_kmbes_struct), (void **)store_ptr, error);

  /* initialize allocated structure to zero */
  if (status == MB_SUCCESS) {
    memset(*store_ptr, 0, sizeof(struct mbsys_kmbes_struct));
  }

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)*store_ptr;

  /* initialize data record kind */
  store->kind = MB_DATA_NONE;

  /* initialize data struct pointers to NULL */
  for (int i = 0; i < MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++)
    store->mwc[i].beamData_p = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)*store_ptr;
  int status = MB_SUCCESS;

  /* deallocate any arrays or structures contained within the store data structure */
  for (int i = 0; i < MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++) {
    if (store->mwc[i].beamData_p != NULL && store->mwc[i].beamData_p_alloc_size > 0) {
      for (int k = 0; k < store->mwc[i].rxInfo.numBeams; k++) {
        if (store->mwc[i].beamData_p[k].sampleAmplitude05dB_p != NULL
          && store->mwc[i].beamData_p[k].sampleAmplitude05dB_p_alloc_size > 0) {
          status = mb_freed(verbose, __FILE__, __LINE__,
                    (void **)(&store->mwc[i].beamData_p[k].sampleAmplitude05dB_p), error);
        store->mwc[i].beamData_p[k].sampleAmplitude05dB_p_alloc_size = 0;
        }
        if (store->mwc[i].beamData_p[k].samplePhase8bit != NULL
          && store->mwc[i].beamData_p[k].samplePhase8bit_alloc_size > 0) {
          status = mb_freed(verbose, __FILE__, __LINE__,
                    (void **)(&store->mwc[i].beamData_p[k].samplePhase8bit), error);
        store->mwc[i].beamData_p[k].samplePhase8bit_alloc_size = 0;
        }
        if (store->mwc[i].beamData_p[k].samplePhase16bit != NULL
          && store->mwc[i].beamData_p[k].samplePhase16bit_alloc_size > 0) {
          status = mb_freed(verbose, __FILE__, __LINE__,
                    (void **)(&store->mwc[i].beamData_p[k].samplePhase16bit), error);
        store->mwc[i].beamData_p[k].samplePhase16bit_alloc_size = 0;
        }
      }

      status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&store->mwc[i].beamData_p), error);
      store->mwc[i].beamData_p_alloc_size = 0;
    }
  }

  /* deallocate memory for data structure */
  status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_kmbes_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                           int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get beam and pixel numbers */
    *nbath = store->num_soundings;
    *namp = store->num_soundings;
    *nss = store->num_pixels;
  }
  else {
    /* get beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
    fprintf(stderr, "dbg2       namp:       %d\n", *namp);
    fprintf(stderr, "dbg2       nss:        %d\n", *nss);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)mb_io_ptr->store_data;

  /* extract data from structure */
  if (store->kind == MB_DATA_DATA) {
    *pingnumber = store->mrz[0].cmnPart.pingCnt;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pingnumber: %d\n", *pingnumber);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  // struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get sonar type */
  *sonartype = MB_TOPOGRAPHY_TYPE_ECHOSOUNDER;  // TODO: review this setting

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  // struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get sidescan type */
  *ss_type = MB_SIDESCAN_LINEAR; // TODO: review this setting

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_preprocess(int verbose, void *mbio_ptr, void *store_ptr,
                             void *platform_ptr, void *preprocess_pars_ptr,
                             int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:                   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:                  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:               %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       preprocess_pars_ptr:        %p\n", (void *)preprocess_pars_ptr);
  }

  int time_i[7];
  double time_d;
  double navlon;
  double navlat;
  double sensordepth;
  double heading;
  // double altitude;
  double speed;
  double roll;
  double pitch;
  double heave;
  double soundspeed;
  double soundspeednew;

  *error = MB_ERROR_NO_ERROR;

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(preprocess_pars_ptr != NULL);

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* data structure pointers */
  // struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  // struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  // struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];
  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  // struct mbsys_kmbes_xmc *xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;

  /* kluge parameters */
  // int kluge_beampatternsnell = false;
  // double kluge_beampatternsnellfactor = 1.0;
  bool kluge_soundspeedsnell = false;
  double kluge_soundspeedsnellfactor = 1.0;
  int kluge_auvsentrysensordepth = false;

  /* get saved values */
  // double *pixel_size = (double *)&mb_io_ptr->saved1;
  // double *swath_width = (double *)&mb_io_ptr->saved2;
  int *kluge_auvsentrysensordepth_set = (int *)&mb_io_ptr->save10;
  if (*kluge_auvsentrysensordepth_set) {
    kluge_auvsentrysensordepth = true; // this allows mbtrnpp to enable Sentry sensordepth kluge
  }

  /* get kluges */
  for (int i = 0; i < pars->n_kluge; i++) {
    if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
      // kluge_beampatternsnell = true;
      // kluge_beampatternsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
      kluge_soundspeedsnell = true;
      kluge_soundspeedsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    if (pars->kluge_id[i] == MB_PR_KLUGE_AUVSENTRYSENSORDEPTH) {
      kluge_auvsentrysensordepth = true;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       target_sensor:                 %d\n", pars->target_sensor);
    fprintf(stderr, "dbg2       timestamp_changed:             %d\n", pars->timestamp_changed);
    fprintf(stderr, "dbg2       time_d:                        %f\n", pars->time_d);
    fprintf(stderr, "dbg2       n_nav:                         %d\n", pars->n_nav);
    fprintf(stderr, "dbg2       nav_time_d:                    %p\n", pars->nav_time_d);
    fprintf(stderr, "dbg2       nav_lon:                       %p\n", pars->nav_lon);
    fprintf(stderr, "dbg2       nav_lat:                       %p\n", pars->nav_lat);
    fprintf(stderr, "dbg2       nav_speed:                     %p\n", pars->nav_speed);
    fprintf(stderr, "dbg2       n_sensordepth:                 %d\n", pars->n_sensordepth);
    fprintf(stderr, "dbg2       sensordepth_time_d:            %p\n", pars->sensordepth_time_d);
    fprintf(stderr, "dbg2       sensordepth_sensordepth:       %p\n", pars->sensordepth_sensordepth);
    fprintf(stderr, "dbg2       n_heading:                     %d\n", pars->n_heading);
    fprintf(stderr, "dbg2       heading_time_d:                %p\n", pars->heading_time_d);
    fprintf(stderr, "dbg2       heading_heading:               %p\n", pars->heading_heading);
    fprintf(stderr, "dbg2       n_altitude:                    %d\n", pars->n_altitude);
    fprintf(stderr, "dbg2       altitude_time_d:               %p\n", pars->altitude_time_d);
    fprintf(stderr, "dbg2       altitude_altitude:             %p\n", pars->altitude_altitude);
    fprintf(stderr, "dbg2       n_attitude:                    %d\n", pars->n_attitude);
    fprintf(stderr, "dbg2       attitude_time_d:               %p\n", pars->attitude_time_d);
    fprintf(stderr, "dbg2       attitude_roll:                 %p\n", pars->attitude_roll);
    fprintf(stderr, "dbg2       attitude_pitch:                %p\n", pars->attitude_pitch);
    fprintf(stderr, "dbg2       attitude_heave:                %p\n", pars->attitude_heave);
    fprintf(stderr, "dbg2       no_change_survey:              %d\n", pars->no_change_survey);
    fprintf(stderr, "dbg2       multibeam_sidescan_source:     %d\n", pars->multibeam_sidescan_source);
    fprintf(stderr, "dbg2       modify_soundspeed:             %d\n", pars->modify_soundspeed);
    fprintf(stderr, "dbg2       recalculate_bathymetry:        %d\n", pars->recalculate_bathymetry);
    fprintf(stderr, "dbg2       sounding_amplitude_filter:     %d\n", pars->sounding_amplitude_filter);
    fprintf(stderr, "dbg2       sounding_amplitude_threshold:  %f\n", pars->sounding_amplitude_threshold);
    fprintf(stderr, "dbg2       ignore_water_column:           %d\n", pars->ignore_water_column);
    for (int i = 0; i < pars->n_kluge; i++) {
      fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
      if (pars->kluge_id[i] == MB_PR_KLUGE_AUVSENTRYSENSORDEPTH) {
        fprintf(stderr, "dbg2       kluge_auvsentrysensordepth:        %d\n", kluge_auvsentrysensordepth);
      }
    }
  }

  int status = MB_SUCCESS;

  /* deal with a survey record */
  if (store->kind == MB_DATA_DATA) {

    /*--------------------------------------------------------------*/
    /* change timestamp if indicated */
    /*--------------------------------------------------------------*/
    if (pars->timestamp_changed) {
      /* set time */
      mb_get_date(verbose, pars->time_d, time_i);
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = pars->time_d;
      fprintf(stderr,
              "Timestamp changed in function %s: "
              "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
              "| ping_number:%d\n",
              __func__, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
              store->mrz[0].cmnPart.pingCnt);
    }

    /*--------------------------------------------------------------*/
    /* interpolate ancillary values  */
    /*--------------------------------------------------------------*/

    /* loop over all sub-pings */
    // int numSoundings = 0;
    // int interp_status = MB_SUCCESS;
    int interp_error = MB_ERROR_NO_ERROR;
    int jnav = 0;
    int jsensordepth = 0;
    int jheading = 0;
    int jattitude = 0;
    int jsoundspeed = 0;
    double soundspeedsnellfactor;

    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];

      // get time
      time_d = ((double)mrz->header.time_sec) + MBSYS_KMBES_NANO * mrz->header.time_nanosec;

      // construct xmt basics
      xmt->header = mrz->header;
      memcpy(xmt->header.dgmType, "#XMT", 4);
      xmt->partition = mrz->partition;
      xmt->cmnPart = mrz->cmnPart;
      xmt->xmtPingInfo.numBytesInfoData = MBSYS_KMBES_XMT_PINGINFO_DATALENGTH;
      xmt->xmtPingInfo.numBytesPerSounding = MBSYS_KMBES_XMT_SOUNDING_DATALENGTH;
      xmt->xmtPingInfo.numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
      xmt->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE
                                + MBSYS_KMBES_PARITION_SIZE
                                + MBSYS_KMBES_XMT_PINGINFO_DATALENGTH
                                + xmt->xmtPingInfo.numSoundings
                                  * MBSYS_KMBES_XMT_SOUNDING_DATALENGTH
                                  + MBSYS_KMBES_END_SIZE;

      xmt->xmtPingInfo.longitude = mrz->pingInfo.longitude_deg;
      xmt->xmtPingInfo.latitude = mrz->pingInfo.latitude_deg;
      xmt->xmtPingInfo.heading = mrz->pingInfo.headingVessel_deg;

      xmt->xmtPingInfo.speed = 0.0;
      if (spo->sensorData.speedOverGround_mPerSec > 0.0)
        xmt->xmtPingInfo.speed = spo->sensorData.speedOverGround_mPerSec;
      else if (cpo->sensorData.speedOverGround_mPerSec > 0.0)
        xmt->xmtPingInfo.speed = cpo->sensorData.speedOverGround_mPerSec;

      xmt->xmtPingInfo.sensordepth = mrz->pingInfo.txTransducerDepth_m;

      if (skm->infoPart.numSamplesArray > 0) {
        xmt->xmtPingInfo.roll = skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.roll_deg;
        xmt->xmtPingInfo.pitch = skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.pitch_deg;
        if (kluge_auvsentrysensordepth)
          xmt->xmtPingInfo.heave = 0.0;
        else
          xmt->xmtPingInfo.heave = skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.heave_m;
      }

      /* interpolate nav */
      if (pars->n_nav > 0) {
        int interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1,
                                                    pars->nav_lon - 1, pars->n_nav,
                                                    time_d, &navlon, &jnav, &interp_error);
        interp_status &= mb_linear_interp_latitude(verbose, pars->nav_time_d - 1,
                                                    pars->nav_lat - 1, pars->n_nav,
                                                    time_d, &navlat, &jnav, &interp_error);
        mrz->pingInfo.longitude_deg = navlon;
        mrz->pingInfo.latitude_deg = navlat;
        xmt->xmtPingInfo.longitude = navlon;
        xmt->xmtPingInfo.latitude = navlat;

        /* calculate speed from position */
        double mtodeglon, mtodeglat;
        double dx, dy, dt;
        mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
        speed = 0.0;
        if (interp_status == MB_SUCCESS && jnav > 0) {
          int j1;
          int j2;
          if (jnav > 1) {
            j1 = jnav - 2;
            j2 = jnav - 1;
          }
          else /* if (jnav == 1) */ {
            j1 = jnav - 1;
            j2 = jnav;
          }
          dx = (pars->nav_lon[j2] - pars->nav_lon[j1]) / mtodeglon;
          dy = (pars->nav_lat[j2] - pars->nav_lat[j1]) / mtodeglat;
          dt = (pars->nav_time_d[j2] - pars->nav_time_d[j1]);
          if (dt > 0.0)
            speed = sqrt(dx * dx + dy * dy) / dt;
        }
        if (speed > 0.0)
          xmt->xmtPingInfo.speed = speed;
      }
      if (pars->nav_speed != NULL) {
        /* interp_status = */ mb_linear_interp(verbose, pars->nav_time_d - 1,
                                                  pars->nav_speed - 1, pars->n_nav,
                                                  time_d, &speed, &jnav, &interp_error);
        xmt->xmtPingInfo.speed = speed;
      }

      /* interpolate sensordepth */
      if (kluge_auvsentrysensordepth) {
        if (pars->n_sensordepth > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                          pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);
          mrz->pingInfo.txTransducerDepth_m = sensordepth;
          xmt->xmtPingInfo.sensordepth = sensordepth;
        }
        else {
          sensordepth = -mrz->pingInfo.ellipsoidHeightReRefPoint_m;
          mrz->pingInfo.txTransducerDepth_m = sensordepth;
          xmt->xmtPingInfo.sensordepth = sensordepth;
        }
      }
      else {
        if (pars->n_sensordepth > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                          pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);
          mrz->pingInfo.txTransducerDepth_m = sensordepth;
          xmt->xmtPingInfo.sensordepth = sensordepth;
        }
        else {
          sensordepth = mrz->pingInfo.txTransducerDepth_m;
          xmt->xmtPingInfo.sensordepth = sensordepth;
        }
      }

      /* interpolate heading */
      if (pars->n_heading > 0) {
        /* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                 pars->n_heading, time_d, &heading, &jheading, &interp_error);
        mrz->pingInfo.headingVessel_deg = heading;
        xmt->xmtPingInfo.heading = heading;
      }

      /* interpolate altitude */
      //if (pars->n_altitude > 0) {
      //  interp_status = mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
      //                                    time_d, &altitude, &jaltitude, &interp_error);
      //}

      /* interpolate Attitude */
      if (pars->n_attitude > 0) {
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                         time_d, &roll, &jattitude, &interp_error);
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                         time_d, &pitch, &jattitude, &interp_error);
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                         time_d, &heave, &jattitude, &interp_error);
        xmt->xmtPingInfo.roll = roll;
        xmt->xmtPingInfo.pitch = pitch;
        if (kluge_auvsentrysensordepth)
          xmt->xmtPingInfo.heave = 0.0;
        else
          xmt->xmtPingInfo.heave = heave;
      }

      /* interpolate soundspeed */
      soundspeed = mrz->pingInfo.soundSpeedAtTxDepth_mPerSec;
      if (pars->modify_soundspeed) {
        /* interp_status = */ mb_linear_interp(verbose, pars->soundspeed_time_d - 1, pars->soundspeed_soundspeed - 1, pars->n_soundspeed,
                                       time_d, &soundspeednew, &jsoundspeed, &interp_error);
        soundspeedsnellfactor = soundspeednew / soundspeed;
        soundspeed = soundspeednew;
        mrz->pingInfo.soundSpeedAtTxDepth_mPerSec = soundspeednew;
      }

      /* if requested apply kluge scaling of sound speed - which means
          changing beam angles by Snell's law and changing the sound
          speed used to calculate Bathymetry */
      if (kluge_soundspeedsnell) {
        /*
         * sound speed
         */
        soundspeedsnellfactor *= kluge_soundspeedsnellfactor;
        soundspeed *= kluge_soundspeedsnellfactor;
      }

      // loop over all soundings
      for (int i = 0; i < xmt->xmtPingInfo.numSoundings; i++) {
        /* variables for beam angle calculation */
        mb_3D_orientation tx_align;
        mb_3D_orientation tx_orientation;
        double tx_steer;
        mb_3D_orientation rx_orientation;
        double rx_steer;
        double reference_heading;
        double beamAzimuth;
        double beamDepression;
        double ttime = 0.0;  // TODO(schwehr): Bug?
        double beamroll, beampitch, beamheading;
        // double theta, phi;
        // double mtodeglon, mtodeglat;
        // double headingx, headingy;

        /* get roll at bottom return time for this beam */
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1,
                              pars->attitude_roll - 1, pars->n_attitude,
                              time_d + ttime, &beamroll, &jattitude, error);

        /* get pitch at bottom return time for this beam */
        /* interp_status = */
            mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                             time_d + ttime, &beampitch, &jattitude, error);

        /* get heading at bottom return time for this beam */
        /* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                 pars->n_heading, time_d + ttime, &beamheading,
                                                 &jheading, error);

        /* change the sound speed recorded for the current ping and
         * then use it to alter the beam angles and recalculate the Bathymetry */
        const double soundspeedsnellfactor = 0.0;  // TODO(schwehr): Likely a bug
        if (pars->modify_soundspeed || kluge_soundspeedsnell) {
          mrz->sounding[i].beamAngleReRx_deg =
                RTD * asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                         * sin(DTR * mrz->sounding[i].beamAngleReRx_deg))));
        }


        /* calculate beam angles for raytracing using Jon Beaudoin's code based on:
            Beaudoin, J., Hughes Clarke, J., and Bartlett, J. Application of
            Surface Sound Speed Measurements in Post-Processing for Multi-Sector
            Multibeam Echosounders : International Hydrographic Review, v.5, no.3,
            p.26-31.
            (http://www.omg.unb.ca/omg/papers/beaudoin_IHR_nov2004.pdf).
           note complexity if transducer arrays are reverse mounted, as determined
           by a mount heading angle of about 180 degrees rather than about 0 degrees.
           If a receive array or a transmit array are reverse mounted then:
            1) subtract 180 from the heading mount angle of the array
            2) flip the sign of the pitch and roll mount offsets of the array
            3) flip the sign of the beam steering angle from that array
                (reverse TX means flip sign of TX steer, reverse RX
                means flip sign of RX steer) */
        tx_steer = mrz->sectorInfo[mrz->sounding[i].txSectorNumb].tiltAngleReTx_deg;
        tx_orientation.roll = roll;
        tx_orientation.pitch = pitch;
        tx_orientation.heading = heading;
        rx_steer = mrz->sounding[i].beamAngleReRx_deg - mrz->sounding[i].beamAngleCorrection_deg;
        rx_orientation.roll = beamroll;
        rx_orientation.pitch = beampitch;
        rx_orientation.heading = beamheading;
        reference_heading = heading;
//fprintf(stderr, "%s:%d:%s: beam %d: beamAngleReRx_deg:%f beamAngleCorrection_deg:%f roll:%f %f rx_steer:%f\n",
//__FILE__, __LINE__, __func__, i, mrz->sounding[i].beamAngleReRx_deg, mrz->sounding[i].beamAngleCorrection_deg,
//roll, beamroll, rx_steer);

        mb_3D_orientation rx_align = {0.0, 0.0, 0.0};  // TODO(schwehr): Bug?
        status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                             reference_heading, &beamAzimuth, &beamDepression, error);
        const double theta = 90.0 - beamDepression;
        double phi = 90.0 - beamAzimuth;
        if (phi < 0.0)
          phi += 360.0;

        ttime = mrz->sounding[i].twoWayTravelTime_sec
                                  + mrz->sounding[i].twoWayTravelTimeCorrection_sec;

        /* calculate Bathymetry */
        // const double rr = 0.5 * soundspeed * ttime;
        // const double xx = rr * sin(DTR * theta);
        // const double zz = rr * cos(DTR * theta);
        // mrz->sounding[i].y_reRefPoint_m = xx * cos(DTR * phi);
        // mrz->sounding[i].x_reRefPoint_m = xx * sin(DTR * phi);
        // mrz->sounding[i].z_reRefPoint_m = zz;
        double receive_time_delay = ttime +
            mrz->sectorInfo[mrz->sounding[i].txSectorNumb].sectorTransmitDelay_sec;
        double receive_time_d = time_d + receive_time_delay;
        double receive_sensordepth = sensordepth;
        double receive_heave = heave;
        if (pars->n_sensordepth > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                          pars->n_sensordepth, receive_time_d, &receive_sensordepth, &jsensordepth, &interp_error);
        }
        else if (kluge_auvsentrysensordepth) {
          receive_sensordepth = -mrz->pingInfo.ellipsoidHeightReRefPoint_m;
        }
        else {
          sensordepth = mrz->pingInfo.txTransducerDepth_m;
        }
        if (pars->n_attitude > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                           time_d, &receive_heave, &jattitude, &interp_error);
        }

        xmt->xmtSounding[i].soundingIndex = mrz->sounding[i].soundingIndex;
        xmt->xmtSounding[i].padding0 = 0;
        xmt->xmtSounding[i].twtt = ttime;
        xmt->xmtSounding[i].angle_vertical = theta;
        xmt->xmtSounding[i].angle_azimuthal = phi;
        xmt->xmtSounding[i].beam_heave = (receive_sensordepth - sensordepth) + (receive_heave - heave);
        xmt->xmtSounding[i].alongtrack_offset = receive_time_delay * xmt->xmtPingInfo.speed;

        if (kluge_auvsentrysensordepth) {
          mrz->pingInfo.z_waterLevelReRefPoint_m = -sensordepth;
        }
      }
    }

    // generate pseudosidescan
    struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
    if (xms->pingCnt != mrz->cmnPart.pingCnt) {
      double *pixel_size = (double *)&mb_io_ptr->saved1;
      double *swath_width = (double *)&mb_io_ptr->saved2;
      status = mbsys_kmbes_makess(verbose, mbio_ptr, store_ptr, false,
                                  pixel_size, false, swath_width, 0, error);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:         %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  return (status);

}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                                 char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                 double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  // struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  struct mbsys_kmbes_shi *shi = (struct mbsys_kmbes_shi *)&store->shi;
  struct mbsys_kmbes_sha *sha = (struct mbsys_kmbes_sha *)&store->sha;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  struct mbsys_kmbes_xmc *xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];
  struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = xmt->xmtPingInfo.heading;

    /* set beamwidths in mb_io structure */
    mb_io_ptr->beamwidth_xtrack = mrz->pingInfo.receiveArraySizeUsed_deg;
    mb_io_ptr->beamwidth_ltrack = mrz->pingInfo.transmitArraySizeUsed_deg;

    /* read distance and depth values from all sub-pings into storage arrays */
    *nbath = 0;
    *namp = 0;
    *nss = 0;
    int numSoundings = 0;
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        bath[numSoundings] = mrz->sounding[i].z_reRefPoint_m
                              - mrz->pingInfo.z_waterLevelReRefPoint_m;
//fprintf(stderr,"%s:%d:%s: %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d txd:%f wlr:%f ehr:%f zreref:%f bath:%f\n",
//__FILE__, __LINE__, __func__,
//time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
//mrz->pingInfo.txTransducerDepth_m, mrz->pingInfo.z_waterLevelReRefPoint_m,
//mrz->pingInfo.ellipsoidHeightReRefPoint_m,
//mrz->sounding[i].z_reRefPoint_m, bath[numSoundings]);
        beamflag[numSoundings] = mrz->sounding[i].beamflag;
        bathacrosstrack[numSoundings] = mrz->sounding[i].y_reRefPoint_m;
        bathalongtrack[numSoundings] = mrz->sounding[i].x_reRefPoint_m;
        amp[numSoundings] = mrz->sounding[i].reflectivity1_dB;

        numSoundings++;
      }
    }
    *nbath = numSoundings;
    *namp = numSoundings;
    *nss = MIN(xms->pixels_ss, MBSYS_KMBES_MAX_PIXELS);
    store->num_pixels = *nss;
    double pixel_size = xms->pixel_size;
    for (int i = 0; i < MBSYS_KMBES_MAX_PIXELS; i++) {
      if (xms->ss[i] == MBSYS_KMBES_INVALID_SS
        || (xms->ss[i] == MBSYS_KMBES_INVALID_AMP && xms->ss_alongtrack[i] == 0.0)) {
        ss[i] = MB_SIDESCAN_NULL;
        ssacrosstrack[i] = pixel_size * (i - MBSYS_KMBES_MAX_PIXELS / 2);
        ssalongtrack[i] = 0.0;
      } else {
        ss[i] = xms->ss[i];
        ssacrosstrack[i] = pixel_size * (i - MBSYS_KMBES_MAX_PIXELS / 2);
        ssalongtrack[i] = xms->ss_alongtrack[i];
      }
    }

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
      fprintf(stderr, "dbg4       nbath:      %d\n", *nbath);
      for (int i = 0; i < *nbath; i++)
        fprintf(stderr, "dbg4       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
                bathacrosstrack[i], bathalongtrack[i]);
      fprintf(stderr, "dbg4        namp:     %d\n", *namp);
      for (int i = 0; i < *namp; i++)
        fprintf(stderr, "dbg4        beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
                bathalongtrack[i]);
      fprintf(stderr, "dbg4        nss:      %d\n", *nss);
      for (int i = 0; i < *nss; i++)
        fprintf(stderr, "dbg4        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
                ssalongtrack[i]);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = spo->sensorData.correctedLong_deg;
    *navlat = spo->sensorData.correctedLat_deg;

    /* get speed */
    *speed = 3.6 * spo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    *heading = spo->sensorData.courseOverGround_deg;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = skm->sample[0].KMdefault.longitude_deg;
    *navlat = skm->sample[0].KMdefault.latitude_deg;

    /* get speed */
    *speed = 3.6 * sqrt(skm->sample[0].KMdefault.velNorth
                          * skm->sample[0].KMdefault.velNorth
                        + skm->sample[0].KMdefault.velEast
                          * skm->sample[0].KMdefault.velEast);

    /* get heading */
    *heading = skm->sample[0].KMdefault.heading_deg;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = cpo->sensorData.correctedLong_deg;
    *navlat = cpo->sensorData.correctedLat_deg;

    /* get speed */
    *speed = 3.6 * cpo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    *heading = cpo->sensorData.courseOverGround_deg;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_SONARDEPTH) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = xmt->xmtPingInfo.heading;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_HEADING) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = sha->sensorData[0].headingCorrected_deg;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract comment from structure */
  else if (*kind == MB_DATA_COMMENT) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* copy comment */
    if (strlen(xmc->comment) > 0)
      strncpy(comment, xmc->comment, MB_COMMENT_MAXLINE);
    else
      comment[0] = '\0';

    if (verbose >= 4) {
      fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  New ping values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       comment:    %s\n", comment);
    }
  }

  /* set time for other data records */
  else {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    if (verbose >= 4) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg4  Extracted values:\n");
      fprintf(stderr, "dbg4       kind:       %d\n", *kind);
      fprintf(stderr, "dbg4       error:      %d\n", *error);
      fprintf(stderr, "dbg4       time_i[0]:  %d\n", time_i[0]);
      fprintf(stderr, "dbg4       time_i[1]:  %d\n", time_i[1]);
      fprintf(stderr, "dbg4       time_i[2]:  %d\n", time_i[2]);
      fprintf(stderr, "dbg4       time_i[3]:  %d\n", time_i[3]);
      fprintf(stderr, "dbg4       time_i[4]:  %d\n", time_i[4]);
      fprintf(stderr, "dbg4       time_i[5]:  %d\n", time_i[5]);
      fprintf(stderr, "dbg4       time_i[6]:  %d\n", time_i[6]);
      fprintf(stderr, "dbg4       time_d:     %f\n", *time_d);
      fprintf(stderr, "dbg4       comment:    %s\n", comment);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }
  else if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind != MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
  }
  if (verbose >= 2 && (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV)) {
    fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", *speed);
    fprintf(stderr, "dbg2       heading:       %f\n", *heading);
  }
  if (verbose >= 2 && *error <= MB_ERROR_NO_ERROR && *kind == MB_DATA_DATA) {
    fprintf(stderr, "dbg2       nbath:      %d\n", *nbath);
    for (int i = 0; i < *nbath; i++)
      fprintf(stderr, "dbg2       beam:%d  flag:%3d  bath:%f  acrosstrack:%f  alongtrack:%f\n", i, beamflag[i], bath[i],
              bathacrosstrack[i], bathalongtrack[i]);
    fprintf(stderr, "dbg2        namp:     %d\n", *namp);
    for (int i = 0; i < *namp; i++)
      fprintf(stderr, "dbg2       beam:%d   amp:%f  acrosstrack:%f  alongtrack:%f\n", i, amp[i], bathacrosstrack[i],
              bathalongtrack[i]);
    fprintf(stderr, "dbg2        nss:      %d\n", *nss);
    for (int i = 0; i < *nss; i++)
      fprintf(stderr, "dbg2        pixel:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
              ssalongtrack[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d,
                                double navlon, double navlat, double speed, double heading, int nbath, int namp, int nss,
                                char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:       %d\n", kind);
  }
  if (verbose >= 2 && kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }
  if (verbose >= 2 && (kind != MB_DATA_COMMENT)) {
    fprintf(stderr, "dbg2       time_i[0]:  %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:  %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:  %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:  %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:  %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:  %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:  %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:     %f\n", time_d);
  }
  if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV)) {
    fprintf(stderr, "dbg2       navlon:     %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:     %f\n", navlat);
    fprintf(stderr, "dbg2       speed:      %f\n", speed);
    fprintf(stderr, "dbg2       heading:    %f\n", heading);
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
        fprintf(stderr, "dbg3        beam:%d   ss:%f  acrosstrack:%f  alongtrack:%f\n", i, ss[i], ssacrosstrack[i],
                ssalongtrack[i]);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  struct mbsys_kmbes_sha *sha = (struct mbsys_kmbes_sha *)&store->sha;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  struct mbsys_kmbes_xmc *xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];
  struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;

  /* set data kind */
  store->kind = kind;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* loop over all sub-pings */
    /* read distance and depth values into storage arrays */
    int numSoundings = 0;
    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      xmt->xmtPingInfo.longitude = navlon;
      xmt->xmtPingInfo.latitude = navlat;
      xmt->xmtPingInfo.heading = heading;
      xmt->xmtPingInfo.speed = speed /  3.6;
      mrz->pingInfo.longitude_deg = navlon;
      mrz->pingInfo.latitude_deg = navlat;
      mrz->pingInfo.headingVessel_deg = heading;

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        mrz->sounding[i].z_reRefPoint_m = bath[numSoundings]
                                + mrz->pingInfo.z_waterLevelReRefPoint_m;
//        mrz->sounding[i].z_reRefPoint_m = bath[numSoundings]
//                                - mrz->pingInfo.txTransducerDepth_m;
        mrz->sounding[i].beamflag = beamflag[numSoundings];
        mrz->sounding[i].x_reRefPoint_m = bathalongtrack[numSoundings];
        mrz->sounding[i].y_reRefPoint_m = bathacrosstrack[numSoundings];
        mrz->sounding[i].reflectivity1_dB = amp[numSoundings];

        numSoundings++;
      }
    }

    /* insert the sidescan */
    xms->pixels_ss = nss;
    for (int i = 0; i < MBSYS_KMBES_MAX_PIXELS; i++) {
      if (ss[i] == MB_SIDESCAN_NULL) {
        xms->ss[i] = MBSYS_KMBES_INVALID_SS;
        xms->ss_alongtrack[i] = 0.0;
      } else {
        xms->ss[i] = ss[i];
        xms->ss_alongtrack[i] = ssalongtrack[i];
      }
    }

  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    spo->sensorData.correctedLong_deg = navlon;
    spo->sensorData.correctedLat_deg = navlat;

    /* get heading */
    spo->sensorData.courseOverGround_deg = heading;

    /* get speed  */
    spo->sensorData.speedOverGround_mPerSec = speed / 3.6;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    skm->sample[0].KMdefault.longitude_deg = navlon;
    skm->sample[0].KMdefault.latitude_deg = navlat;

    /* get heading */
    skm->sample[0].KMdefault.heading_deg = heading;

    /* get speed  */
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    cpo->sensorData.correctedLong_deg = navlon;
    cpo->sensorData.correctedLat_deg = navlat;

    /* get heading */
    cpo->sensorData.courseOverGround_deg = heading;

    /* get speed  */
    cpo->sensorData.speedOverGround_mPerSec = speed / 3.6;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_SONARDEPTH) {

  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_HEADING) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */

    /* get heading */
    sha->sensorData[0].headingCorrected_deg = heading;

    /* get speed  */
  }

  /* insert comment in structure */
  else if (store->kind == MB_DATA_COMMENT) {
    /* copy comment */
    strncpy(xmc->comment, comment, MB_COMMENT_MAXLINE-1);
    xmc->comment[MB_COMMENT_MAXLINE-1] = '\0';

    /* have to construct this record now */
    const int numBytesComment = strlen(xmc->comment) + (strlen(xmc->comment) % 2);
    xmc->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + numBytesComment + 36;
    strncpy((char *)xmc->header.dgmType, "#XMC", 4);
    xmc->header.dgmVersion = 0;
    xmc->header.systemID = 0;
    xmc->header.echoSounderID = 0;

    /* insert current time as timestamp if needed (time_d close to zero) */
    if (fabs(time_d) < 1.0) {
      struct timespec right_now_nsec;
      clock_gettime(CLOCK_REALTIME, &right_now_nsec);
      time_d = right_now_nsec.tv_sec + 0.000000001 * right_now_nsec.tv_nsec;
      mb_get_date(verbose, time_d, time_i);
    }
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;
    xmc->header.time_sec = (int)time_d;
    xmc->header.time_nanosec = (time_d - floor(time_d)) * 1.0e9;
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes,
                                double *angles, double *angles_forward, double *angles_null, double *heave,
                                double *alongtrack_offset, double *draft, double *ssv, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       ttimes:     %p\n", (void *)ttimes);
    fprintf(stderr, "dbg2       angles_xtrk:%p\n", (void *)angles);
    fprintf(stderr, "dbg2       angles_ltrk:%p\n", (void *)angles_forward);
    fprintf(stderr, "dbg2       angles_null:%p\n", (void *)angles_null);
    fprintf(stderr, "dbg2       heave:      %p\n", (void *)heave);
    fprintf(stderr, "dbg2       ltrk_off:   %p\n", (void *)alongtrack_offset);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

    /* get depth offset (heave + sonar depth) */
    *ssv = mrz->pingInfo.soundSpeedAtTxDepth_mPerSec;

    /* get draft */
    *draft = mrz->pingInfo.txTransducerDepth_m;

    /* read distance and depth values from all sub-pings into storage arrays */
    int numSoundings = 0;
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {

      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];

      for (int i = 0;
           i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
           i++) {
        // struct mbsys_kmbes_mrz_sounding *sounding = &mrz->sounding[i];
        // struct mbsys_kmbes_mrz_tx_sector_info *sectorInfo = &mrz->sectorInfo[sounding->txSectorNumb];

        ttimes[numSoundings] = xmt->xmtSounding[i].twtt;
        angles[numSoundings] = xmt->xmtSounding[i].angle_vertical;
        angles_forward[numSoundings] = xmt->xmtSounding[i].angle_azimuthal;
        angles_null[numSoundings] = 0.0;
        heave[numSoundings] = xmt->xmtSounding[i].beam_heave;
        alongtrack_offset[numSoundings] = xmt->xmtSounding[i].alongtrack_offset;
        numSoundings++;
      }
      *nbeams = numSoundings;

      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;

    /* done translating values */
    }
  }
  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       draft:      %f\n", *draft);
    fprintf(stderr, "dbg2       ssv:        %f\n", *ssv);
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
              i, ttimes[i], angles[i], angles_forward[i], angles_null[i], heave[i], alongtrack_offset[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get detect type for each sounding - options include:
        MB_DETECT_UNKNOWN
        MB_DETECT_AMPLITUDE
        MB_DETECT_PHASE
        MB_DETECT_UNKNOWN */
    int numSoundings = 0;
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        if (mrz->sounding[i].detectionMethod == 1) {
          detects[numSoundings] = MB_DETECT_AMPLITUDE;
        } else if (mrz->sounding[i].detectionMethod == 2) {
          detects[numSoundings] = MB_DETECT_PHASE;
        } else {
          detects[numSoundings] = MB_DETECT_UNKNOWN;
        }
        numSoundings++;
      }
    }
    *nbeams = numSoundings;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: detects:%d\n", i, detects[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_pulses(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *pulses, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       pulses:     %p\n", (void *)pulses);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get transmit pulse type for each sounding - options include:
          MB_PULSE_TYPE_NUM 5
          MB_PULSE_UNKNOWN 0
          MB_PULSE_CW 1
          MB_PULSE_UPCHIRP 2
          MB_PULSE_DOWNCHIRP 3
          MB_PULSE_LIDAR 4 */
    int numSoundings = 0;
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        if (mrz->sectorInfo[mrz->sounding[i].txSectorNumb].signalWaveForm == 0) {
          pulses[numSoundings] = MB_PULSE_CW;
        } else if (mrz->sounding[i].detectionMethod == 1) {
          pulses[numSoundings] = MB_PULSE_UPCHIRP;
        } else if (mrz->sounding[i].detectionMethod == 1) {
          pulses[numSoundings] = MB_PULSE_DOWNCHIRP;
        } else {
          pulses[numSoundings] = MB_PULSE_UNKNOWN;
        }
        numSoundings++;
      }
    }
    *nbeams = numSoundings;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nbeams:     %d\n", *nbeams);
    for (int i = 0; i < *nbeams; i++)
      fprintf(stderr, "dbg2       beam %d: pulses: %d\n", i, pulses[i]);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain,
                               double *pulse_length, double *receive_gain, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

    /* get transmit_gain (dB) */
    *transmit_gain = mrz->pingInfo.transmitPower_dB;

    /* get pulse_length (usec) */
    *transmit_gain = mrz->pingInfo.maxEffTxPulseLength_sec;

    /* get receive_gain (dB) */
    *receive_gain = 0.0;

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       transmit_gain: %f\n", *transmit_gain);
    fprintf(stderr, "dbg2       pulse_length:  %f\n", *pulse_length);
    fprintf(stderr, "dbg2       receive_gain:  %f\n", *receive_gain);
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                          double *altitudev, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

  int status = MB_SUCCESS;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {

    /* get transducer depth and altitude */
    *transducer_depth = mrz->pingInfo.txTransducerDepth_m;

    /* get altitude using valid depth closest to nadir */
    *altitudev = 0.0;
    double xtrackmin = 999999.9;
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        if (mb_beam_ok(mrz->sounding[i].beamflag)) {
          if (fabs(mrz->sounding[i].y_reRefPoint_m) < xtrackmin) {
            xtrackmin = fabs(mrz->sounding[i].y_reRefPoint_m);
            *altitudev = mrz->sounding[i].z_reRefPoint_m;
          }
        }
      }
    }

    /* set status */
    *error = MB_ERROR_NO_ERROR;
    status = MB_SUCCESS;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       transducer_depth:  %f\n", *transducer_depth);
    fprintf(stderr, "dbg2       altitude:          %f\n", *altitudev);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                            double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                            double *pitch, double *heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];
  // struct mbsys_kmbes_xmb *xmb = (struct mbsys_kmbes_xmb *)&store->xmb;
  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  struct mbsys_kmbes_sha *sha = (struct mbsys_kmbes_sha *)&store->sha;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from survey record */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = spo->sensorData.correctedLong_deg;
    *navlat = spo->sensorData.correctedLat_deg;

    /* get speed */
    *speed = 3.6 * spo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    *heading = spo->sensorData.courseOverGround_deg;

    /* get draft  */
    *draft = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
}

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = skm->sample[0].KMdefault.longitude_deg;
    *navlat = skm->sample[0].KMdefault.latitude_deg;

    /* get speed */
    *speed = 3.6 * sqrt(skm->sample[0].KMdefault.velNorth
                          * skm->sample[0].KMdefault.velNorth
                        + skm->sample[0].KMdefault.velEast
                          * skm->sample[0].KMdefault.velEast);

    /* get heading */
    *heading = skm->sample[0].KMdefault.heading_deg;

    /* get draft  */
    *draft = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = skm->sample[0].KMdefault.roll_deg;
    *pitch = skm->sample[0].KMdefault.pitch_deg;
    *heave = skm->sample[0].KMdefault.heave_m;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = cpo->sensorData.correctedLong_deg;
    *navlat = cpo->sensorData.correctedLat_deg;

    /* get speed */
    *speed = 3.6 * cpo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    *heading = cpo->sensorData.courseOverGround_deg;

    /* get draft  */
    *draft = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_SONARDEPTH) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    *draft = sde->sensorData.depthUsed_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_HEADING) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = sha->sensorData[0].headingCorrected_deg;

    /* get draft  */
    *draft = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:          %d\n", *kind);
    fprintf(stderr, "dbg2       time_i[0]:     %d\n", time_i[0]);
    fprintf(stderr, "dbg2       time_i[1]:     %d\n", time_i[1]);
    fprintf(stderr, "dbg2       time_i[2]:     %d\n", time_i[2]);
    fprintf(stderr, "dbg2       time_i[3]:     %d\n", time_i[3]);
    fprintf(stderr, "dbg2       time_i[4]:     %d\n", time_i[4]);
    fprintf(stderr, "dbg2       time_i[5]:     %d\n", time_i[5]);
    fprintf(stderr, "dbg2       time_i[6]:     %d\n", time_i[6]);
    fprintf(stderr, "dbg2       time_d:        %f\n", *time_d);
    fprintf(stderr, "dbg2       longitude:     %f\n", *navlon);
    fprintf(stderr, "dbg2       latitude:      %f\n", *navlat);
    fprintf(stderr, "dbg2       speed:         %f\n", *speed);
    fprintf(stderr, "dbg2       heading:       %f\n", *heading);
    fprintf(stderr, "dbg2       draft:         %f\n", *draft);
    fprintf(stderr, "dbg2       roll:          %f\n", *roll);
    fprintf(stderr, "dbg2       pitch:         %f\n", *pitch);
    fprintf(stderr, "dbg2       heave:         %f\n", *heave);
    fprintf(stderr, "dbg2       error:         %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                                      double *time_d, double *navlon, double *navlat, double *speed, double *heading,
                                      double *draft, double *roll, double *pitch, double *heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  struct mbsys_kmbes_sha *sha = (struct mbsys_kmbes_sha *)&store->sha;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from survey record */
  if (*kind == MB_DATA_DATA) {
    /* just one navigation value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = mrz->pingInfo.longitude_deg;
    navlat[0] = mrz->pingInfo.latitude_deg;

    /* get speed */
    speed[0] = 3.6 * xmt->xmtPingInfo.speed;;

    /* get heading */
    heading[0] = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    roll[0] = xmt->xmtPingInfo.roll;
    pitch[0] = xmt->xmtPingInfo.pitch;
    heave[0] = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV) {
    /* just one navigation value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = spo->sensorData.correctedLong_deg;
    navlat[0] = spo->sensorData.correctedLat_deg;

    /* get speed */
    speed[0] = 3.6 * spo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    heading[0] = spo->sensorData.courseOverGround_deg;

    /* get attitude  */
    roll[0] = xmt->xmtPingInfo.roll;
    pitch[0] = xmt->xmtPingInfo.pitch;
    heave[0] = xmt->xmtPingInfo.heave;
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV1) {
    *n = MIN(skm->infoPart.numSamplesArray, MB_NAV_MAX);

    for (int i = 0; i < *n; i++) {
      /* get time */
      time_d[i] = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
      mb_get_date(verbose, time_d[i], &time_i[7*i]);

      /* get navigation */
      navlon[i] = skm->sample[i].KMdefault.longitude_deg;
      navlat[i] = skm->sample[i].KMdefault.latitude_deg;

      /* get speed */
      speed[i] = 3.6 * sqrt(skm->sample[i].KMdefault.velNorth
                            * skm->sample[i].KMdefault.velNorth
                          + skm->sample[i].KMdefault.velEast
                            * skm->sample[i].KMdefault.velEast);

      /* get heading */
      heading[i] = skm->sample[i].KMdefault.heading_deg;

      /* get draft  */
      draft[i] = mrz->pingInfo.txTransducerDepth_m;

      /* get attitude  */
      roll[i] = skm->sample[i].KMdefault.roll_deg;
      pitch[i] = skm->sample[i].KMdefault.pitch_deg;
      heave[i] = skm->sample[i].KMdefault.heave_m;
    }

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV2) {
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = cpo->sensorData.correctedLong_deg;
    navlat[0] = cpo->sensorData.correctedLat_deg;

    /* get speed */
    speed[0] = 3.6 * cpo->sensorData.speedOverGround_mPerSec;

    /* get heading */
    heading[0] = cpo->sensorData.courseOverGround_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    roll[0] = xmt->xmtPingInfo.roll;
    pitch[0] = xmt->xmtPingInfo.pitch;
    heave[0] = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_SONARDEPTH) {
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    *navlon = xmt->xmtPingInfo.longitude;
    *navlat = xmt->xmtPingInfo.latitude;

    /* get speed */
    *speed = 3.6 * xmt->xmtPingInfo.speed;

    /* get heading */
    *heading = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    *draft = sde->sensorData.depthUsed_m;

    /* get attitude  */
    *roll = xmt->xmtPingInfo.roll;
    *pitch = xmt->xmtPingInfo.pitch;
    *heave = xmt->xmtPingInfo.heave;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_HEADING) {
    *n = sha->dataInfo.numSamplesArray;

    const double sha_time_d = sha->header.time_sec + 0.000000001 * sha->header.time_nanosec;
    double sha_sample_time_d;
    for (int i=0; i<*n; i++) {

      /* get time */
      time_d[i] = sha_time_d + 0.000000001 * sha->sensorData[i].timeSinceRecStart_nanosec;
      mb_get_date(verbose, time_d[i], &time_i[7*i]);

      /* get navigation */
      *navlon = xmt->xmtPingInfo.longitude;
      *navlat = xmt->xmtPingInfo.latitude;

      /* get speed */
      *speed = 3.6 * xmt->xmtPingInfo.speed;

      /* get heading */
      *heading = sha->sensorData[i].headingCorrected_deg;

      /* get draft  */
      *draft = mrz->pingInfo.txTransducerDepth_m;

      /* get attitude  */
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       n:          %d\n", *n);
    for (int inav = 0; inav < *n; inav++) {
      for (int i = 0; i < 7; i++)
        fprintf(stderr, "dbg2       %d time_i[%d]:     %d\n", inav, i, time_i[inav * 7 + i]);
      fprintf(stderr, "dbg2       %d time_d:        %f\n", inav, time_d[inav]);
      fprintf(stderr, "dbg2       %d longitude:     %f\n", inav, navlon[inav]);
      fprintf(stderr, "dbg2       %d latitude:      %f\n", inav, navlat[inav]);
      fprintf(stderr, "dbg2       %d speed:         %f\n", inav, speed[inav]);
      fprintf(stderr, "dbg2       %d heading:       %f\n", inav, heading[inav]);
      fprintf(stderr, "dbg2       %d draft:         %f\n", inav, draft[inav]);
      fprintf(stderr, "dbg2       %d roll:          %f\n", inav, roll[inav]);
      fprintf(stderr, "dbg2       %d pitch:         %f\n", inav, pitch[inav]);
      fprintf(stderr, "dbg2       %d heave:         %f\n", inav, heave[inav]);
    }
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                                    double navlat, double speed, double heading, double draft, double roll, double pitch,
                                    double heave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
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
    fprintf(stderr, "dbg2       roll:       %f\n", roll);
    fprintf(stderr, "dbg2       pitch:      %f\n", pitch);
    fprintf(stderr, "dbg2       heave:      %f\n", heave);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  // struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  struct mbsys_kmbes_spo *spo = (struct mbsys_kmbes_spo *)&store->spo;
  struct mbsys_kmbes_skm *skm = (struct mbsys_kmbes_skm *)&store->skm;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  // struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* loop over all sub-pings */
    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];
      mrz->pingInfo.longitude_deg = navlon;
      mrz->pingInfo.latitude_deg = navlat;
      mrz->pingInfo.headingVessel_deg = heading;
      xmt->xmtPingInfo.speed = speed /  3.6;
      mrz->pingInfo.txTransducerDepth_m = draft - heave;
      xmt->xmtPingInfo.sensordepth = draft - heave;
      xmt->xmtPingInfo.roll = roll;
      xmt->xmtPingInfo.pitch = pitch;
      xmt->xmtPingInfo.heave = heave;
    }
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    spo->sensorData.correctedLong_deg = navlon;
    spo->sensorData.correctedLat_deg = navlat;

    /* get heading */
    spo->sensorData.courseOverGround_deg = heading;

    /* get speed  */
    spo->sensorData.speedOverGround_mPerSec = speed / 3.6;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    skm->sample[0].KMdefault.longitude_deg = navlon;
    skm->sample[0].KMdefault.latitude_deg = navlat;

    /* get heading */
    skm->sample[0].KMdefault.heading_deg = heading;

    /* get speed  */
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    cpo->sensorData.correctedLong_deg = navlon;
    cpo->sensorData.correctedLat_deg = navlat;

    /* get heading */
    cpo->sensorData.courseOverGround_deg = heading;

    /* get speed  */
    cpo->sensorData.speedOverGround_mPerSec = speed / 3.6;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth,
                                     double *velocity, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_svp *svp = (struct mbsys_kmbes_svp *)&store->svp;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_VELOCITY_PROFILE) {
      /* get number of depth-velocity pairs */
      *nsvp = svp->numSamples;

      /* get sound velocity profile data */
      for (int i = 0; i < *nsvp; i++) {
          depth[i] = svp->sensorData[i].depth_m;
          velocity[i] = svp->sensorData[i].soundVelocity_mPerSec;
      }

      /* done translating values */
  }

  /* deal with comment */
  else if (*kind == MB_DATA_COMMENT) {
    /* set status */
    *error = MB_ERROR_COMMENT;
    status = MB_FAILURE;
  }

  /* deal with other record type */
  else {
    /* set status */
    *error = MB_ERROR_OTHER;
    status = MB_FAILURE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       nsvp:              %d\n", *nsvp);
    for (int i = 0; i < *nsvp; i++)
      fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
                                    int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
    for (int i = 0; i < nsvp; i++)
      fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_svp *svp = (struct mbsys_kmbes_svp *)&store->svp;

  /* insert data in structure */
  if (store->kind == MB_DATA_VELOCITY_PROFILE) {

      /* get number of depth-velocity pairs */
      svp->numSamples = MIN(nsvp, MBSYS_KMBES_MAX_SVP_POINTS);

      /* get sound velocity profile data */
      for (int i = 0; i < nsvp; i++) {
          svp->sensorData[i].depth_m = depth[i];
          svp->sensorData[i].soundVelocity_mPerSec = velocity[i];
      }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointers */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_struct *copy = (struct mbsys_kmbes_struct *)copy_ptr;

  /* copy the data - for many formats memory must be allocated and
      sub-structures copied separately */
  //*copy = *store;
  copy->kind = store->kind;
  copy->time_d = store->time_d;
  for (int i = 0; i < 7; i++) {
    copy->time_i[i] = store->time_i[i];
  }
  copy->num_soundings = store->num_soundings;
  copy->num_backscatter_samples = store->num_backscatter_samples;
  copy->num_pixels = store->num_pixels;
  copy->spo = store->spo;
  copy->skm = store->skm;
  copy->svp = store->svp;
  copy->svt = store->svt;
  copy->scl = store->scl;
  copy->sde = store->sde;
  copy->shi = store->shi;
  copy->sha = store->sha;
  copy->n_mrz_read = store->n_mrz_read;
  copy->n_mrz_needed = store->n_mrz_needed;
  for (int i = 0; i < MBSYS_KMBES_MAX_NUM_MRZ_DGMS; i++) {
    copy->mrz[i] = store->mrz[i];
  }
  copy->xms = store->xms;
  copy->n_mwc_read = store->n_mwc_read;
  copy->n_mwc_needed = store->n_mwc_needed;

  int status = MB_SUCCESS;

  for (int i = 0; i < MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++) {
    struct mbsys_kmbes_mwc *copy_mwc = &copy->mwc[i];
    struct mbsys_kmbes_mwc *store_mwc = &store->mwc[i];

    copy_mwc->header = store_mwc->header;
    copy_mwc->partition = store_mwc->partition;
    copy_mwc->cmnPart = store_mwc->cmnPart;
    copy_mwc->txInfo = store_mwc->txInfo;
    for (int j = 0; j < MBSYS_KMBES_MAX_NUM_TX_PULSES; j++) {
      copy_mwc->sectorData[j] = store_mwc->sectorData[j];
    }
    copy_mwc->rxInfo = store_mwc->rxInfo;

    size_t alloc_size = (size_t)(store_mwc->rxInfo.numBeams * sizeof(struct mbsys_kmbes_mwc_rx_beam_data));
    if (copy_mwc->beamData_p_alloc_size < alloc_size || copy_mwc->beamData_p == NULL) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                            (void **)&(copy_mwc->beamData_p), error);
      if (status == MB_SUCCESS) {
        memset(&copy_mwc->beamData_p[copy_mwc->beamData_p_alloc_size],
                0, alloc_size - copy_mwc->beamData_p_alloc_size);
        copy_mwc->beamData_p_alloc_size = alloc_size;
      } else {
        copy_mwc->beamData_p_alloc_size = 0;
      }
    }
    if (status == MB_SUCCESS) {
      for (int j = 0; j < store_mwc->rxInfo.numBeams; j++) {
        struct mbsys_kmbes_mwc_rx_beam_data *copy_beamData_p = &copy_mwc->beamData_p[j];
        struct mbsys_kmbes_mwc_rx_beam_data *store_beamData_p = &store_mwc->beamData_p[j];

        copy_beamData_p->beamPointAngReVertical_deg = store_beamData_p->beamPointAngReVertical_deg;
        copy_beamData_p->startRangeSampleNum = store_beamData_p->startRangeSampleNum;
        copy_beamData_p->detectedRangeInSamples = store_beamData_p->detectedRangeInSamples;
        copy_beamData_p->beamTxSectorNum = store_beamData_p->beamTxSectorNum;
        copy_beamData_p->numSampleData = store_beamData_p->numSampleData;
        copy_beamData_p->detectedRangeInSamplesHighResolution = store_beamData_p->detectedRangeInSamplesHighResolution;

        alloc_size = (size_t)(store_beamData_p->numSampleData);
        if (copy_beamData_p->sampleAmplitude05dB_p_alloc_size < alloc_size) {
          alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
          status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                              (void **)&(copy_beamData_p->sampleAmplitude05dB_p), error);
          if (status == MB_SUCCESS) {
            copy_beamData_p->sampleAmplitude05dB_p_alloc_size = alloc_size;
          } else {
            copy_beamData_p->sampleAmplitude05dB_p_alloc_size = 0;
          }
        }
        if (status == MB_SUCCESS) {
          memcpy(copy_beamData_p->sampleAmplitude05dB_p, store_beamData_p->sampleAmplitude05dB_p,
                  (size_t)(store_beamData_p->numSampleData));
          memset(&copy_beamData_p->sampleAmplitude05dB_p[(size_t)(store_beamData_p->numSampleData)],
                0, copy_beamData_p->sampleAmplitude05dB_p_alloc_size - (size_t)(store_beamData_p->numSampleData));
        }

        if (status == MB_SUCCESS && store_mwc->rxInfo.phaseFlag == 1) {
          alloc_size = (size_t)(store_beamData_p->numSampleData);
          if (copy_beamData_p->samplePhase8bit_alloc_size < alloc_size) {
            alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
            status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                                (void **)&(copy_beamData_p->samplePhase8bit), error);
            if (status == MB_SUCCESS) {
              copy_beamData_p->samplePhase8bit_alloc_size = alloc_size;
            } else {
              copy_beamData_p->samplePhase8bit_alloc_size = 0;
            }
          }
          if (status == MB_SUCCESS) {
            memcpy(copy_beamData_p->samplePhase8bit, store_beamData_p->samplePhase8bit,
                    copy_beamData_p->numSampleData);
            memset(&copy_beamData_p->samplePhase8bit[(size_t)(store_beamData_p->numSampleData)],
                0, copy_beamData_p->samplePhase8bit_alloc_size - (size_t)(store_beamData_p->numSampleData));
          }
        }

        if (status == MB_SUCCESS && store_mwc->rxInfo.phaseFlag == 2) {
          alloc_size = (size_t)(2 * store_beamData_p->numSampleData);
          if (copy_beamData_p->samplePhase16bit_alloc_size < alloc_size) {
            alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
            status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                                (void **)&(copy_beamData_p->samplePhase16bit), error);
            if (status == MB_SUCCESS) {
              copy_beamData_p->samplePhase16bit_alloc_size = alloc_size;
            } else {
              copy_beamData_p->samplePhase16bit_alloc_size = 0;
            }
          }
          if (status == MB_SUCCESS) {
            memcpy(copy_beamData_p->samplePhase16bit, store_beamData_p->samplePhase16bit,
                    (size_t)(2 * store_beamData_p->numSampleData));
            memset(&copy_beamData_p->samplePhase16bit[(size_t)(store_beamData_p->numSampleData)],
                0, copy_beamData_p->samplePhase16bit_alloc_size - (size_t)(2 * store_beamData_p->numSampleData));
          }
        }
      }
    }
  }

  copy->cpo = store->cpo;
  copy->che = store->che;
  copy->iip = store->iip;
  copy->iop = store->iop;
  copy->ibe = store->ibe;
  copy->ibr = store->ibr;
  copy->ibs = store->ibs;
  copy->fcf = store->fcf;
  copy->xmb = store->xmb;
  copy->xmc = store->xmc;
  copy->unknown = store->unknown;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_makess(
    int verbose, void *mbio_ptr, void *store_ptr,
    int pixel_size_set,  // TODO(schwehr): bool
    double *pixel_size,
    int swath_width_set, // TODO(schwehr): bool
    double *swath_width, int pixel_int, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
  }

  double ss[MBSYS_KMBES_MAX_PIXELS];
  int ss_cnt[MBSYS_KMBES_MAX_PIXELS];
  double ssalongtrack[MBSYS_KMBES_MAX_PIXELS];
  int nbathsort;
  double bathsort[MBSYS_KMBES_MAX_PIXELS];
  int nsoundings, nsamples;
  double median_altitude;
  double pixel_size_calc;
  const int pixel_int_use = 0;  // TODO(schwehr): Likely a bug
  int first, last, k1, kc, k2;
  double dx1, dx2, xx;

  /* get mbio descriptor */
  // struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {

    /* initialize the sidescan binning arrays */
    // double ssacrosstrack[MBSYS_KMBES_MAX_PIXELS];
    for (int i = 0; i < MBSYS_KMBES_MAX_PIXELS; i++) {
      ss[i] = 0.0;
      ss_cnt[i] = 0;
      // ssacrosstrack[i] = 0.0;
      ssalongtrack[i] = 0.0;
    }

    /* if not set get swath width from sonar settings */
    if (!swath_width_set) {
      *swath_width = MAX(fabs(store->mrz[0].pingInfo.portSectorEdge_deg),
                            fabs(store->mrz[0].pingInfo.starbSectorEdge_deg));
    }

    /* get median altitude if needed to calculate pixel size in meters */
    if (!pixel_size_set) {

      /* loop over sub-ping datagrams */
      nbathsort = 0;
      for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {

        /* get the current MRZ datagram */
        struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

        /* loop over all soundings to get the median altitude for this ping */
        for (int i = 0; i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections); i++) {
          if (mb_beam_ok(mrz->sounding[i].beamflag)) {
            bathsort[nbathsort] = mrz->sounding[i].z_reRefPoint_m;
            nbathsort++;
          }
        }
      }
      qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
      median_altitude = bathsort[nbathsort/2];

      /* calculate pixel size from swath width of median altitude */
      pixel_size_calc = 2 * tan(DTR * (*swath_width)) * median_altitude / MBSYS_KMBES_MAX_PIXELS;
      pixel_size_calc = MAX(pixel_size_calc, median_altitude * tan(DTR * 0.1));
      if ((*pixel_size) <= 0.0)
        (*pixel_size) = pixel_size_calc;
      else if (0.95 * (*pixel_size) > pixel_size_calc)
        (*pixel_size) = 0.95 * (*pixel_size);
      else if (1.05 * (*pixel_size) < pixel_size_calc)
        (*pixel_size) = 1.05 * (*pixel_size);
      else
        (*pixel_size) = pixel_size_calc;
    }

    /* loop over all valid soundings, binning the raw backscatter samples into the sidescan */
    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {

      /* get the current MRZ datagram */
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      /* get raw sample size */
      // const double sample_size_m = 750.0 / mrz->rxInfo.seabedImageSampleRate;

      /* use only valid, unflagged soundings */
      nsoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
      nsamples = 0;
      for (int i = 0; i < nsoundings; i++) {
        if (mb_beam_ok(mrz->sounding[i].beamflag)) {

          /* deal with backscatter samples */
          k1 = nsamples;
          kc = k1 + mrz->sounding[i].SIcentreSample - 1;
          k2 = k1 + mrz->sounding[i].SInumSamples - 1;

          /* calculate nominal pixel size */
          // const double dx = sample_size_m * sin(DTR * mrz->sounding[i].beamAngleReRx_deg);

          /* deal with sounding to port of nadir - samples ordered right-to-left */
          if (mrz->sounding[i].y_reRefPoint_m < 0.0) {
            if (i > 0) {
              dx1 = (mrz->sounding[i].y_reRefPoint_m - mrz->sounding[i-1].y_reRefPoint_m)
                      / (mrz->sounding[i].SInumSamples - mrz->sounding[i].SIcentreSample);
            }
            if (i < nsoundings - 1) {
              dx2 = (mrz->sounding[i+1].y_reRefPoint_m - mrz->sounding[i].y_reRefPoint_m)
                      / (mrz->sounding[i].SIcentreSample);
            }
            if (i== 0) {
              dx1 = dx2;
            }
            if (i == nsoundings - 1) {
              dx2 = dx1;
            }
            for (int k = k1; k < kc; k++) {
              xx = mrz->sounding[i].y_reRefPoint_m - dx2 * (k - kc);
              const int kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS && mrz->SIsample_desidB[k] > -32767) {
                ss[kk] += 0.1 * (float)(mrz->SIsample_desidB[k]);
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
            for (int k = kc; k <= k2; k++) {
              xx = mrz->sounding[i].y_reRefPoint_m - dx1 * (k - kc);
              const int kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS && mrz->SIsample_desidB[k] > -32767) {
                ss[kk] += 0.1 * (float)(mrz->SIsample_desidB[k]);
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
          }

          /* else deal with sounding to starboard of nadir - samples ordered left-to-right */
          else {
            if (i > 0) {
              dx1 = (mrz->sounding[i].y_reRefPoint_m - mrz->sounding[i-1].y_reRefPoint_m)
                      / (mrz->sounding[i].SIcentreSample);
            }
            if (i < nsoundings - 1) {
              dx2 = (mrz->sounding[i+1].y_reRefPoint_m - mrz->sounding[i].y_reRefPoint_m)
                      / (mrz->sounding[i].SInumSamples - mrz->sounding[i].SIcentreSample);
            }
            if (i== 0) {
              dx1 = dx2;
            }
            if (i == nsoundings - 1) {
              dx2 = dx1;
            }
            for (int k = k1; k < kc; k++) {
              xx = mrz->sounding[i].y_reRefPoint_m + dx1 * (k - kc);
              const int kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS && mrz->SIsample_desidB[k] > -32767) {
                ss[kk] += 0.1 * (float)(mrz->SIsample_desidB[k]);
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
            for (int k = kc; k <= k2; k++) {
              xx = mrz->sounding[i].y_reRefPoint_m + dx2 * (k - kc);
              const int kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS && mrz->SIsample_desidB[k] > -32767) {
                ss[kk] += 0.1 * (float)(mrz->SIsample_desidB[k]);
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
          }
          nsamples += mrz->sounding[i].SInumSamples;
        }
      }
    }

    /* average the sidescan */
    first = MBSYS_KMBES_MAX_PIXELS;
    last = -1;
    // double ssacrosstrack[MBSYS_KMBES_MAX_PIXELS];
    for (int k = 0; k < MBSYS_KMBES_MAX_PIXELS; k++) {
      if (ss_cnt[k] > 0) {
        ss[k] /= ss_cnt[k];
        ssalongtrack[k] /= ss_cnt[k];
        // ssacrosstrack[k] = (k - MBSYS_KMBES_MAX_PIXELS / 2) * (*pixel_size);
        first = MIN(first, k);
        last = k;
      }
      else
        ss[k] = MB_SIDESCAN_NULL;
    }

    /* interpolate the sidescan */
    k1 = first;
    k2 = first;
    for (int k = first + 1; k < last; k++) {
      if (ss_cnt[k] <= 0) {
        if (k2 <= k) {
          k2 = k + 1;
          while (ss_cnt[k2] <= 0 && k2 < last)
            k2++;
        }
        if (k2 - k1 <= pixel_int_use) {
          ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
          // ssacrosstrack[k] = (k - MBSYS_KMBES_MAX_PIXELS / 2) * (*pixel_size);
          ssalongtrack[k] =
              ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
        }
      }
      else {
        k1 = k;
      }
    }

    /* insert the pseudosidescan into the data structure for an XMS datagram */
    store->num_pixels = MBSYS_KMBES_MAX_PIXELS;
    struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
    struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;
    xms->header = mrz->header;
    xms->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + 8 * MBSYS_KMBES_MAX_PIXELS + 48;
    strncpy((char *)xms->header.dgmType, "#XMS", 4);
    xms->pingCnt = mrz->cmnPart.pingCnt;
    xms->spare = 0;
    xms->pixel_size = *pixel_size;
    xms->pixels_ss = MBSYS_KMBES_MAX_PIXELS;
    memset((char *)xms->unused, 0, 32);
    for (int k = 0; k < xms->pixels_ss; k++) {
      xms->ss[k] = ss[k];
      xms->ss_alongtrack[k] = ssalongtrack[k];
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
