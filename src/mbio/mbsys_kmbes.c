/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_kmbes.c  3.00  5/25/2018
 *
 *    Copyright (c) 2018-2019 by
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

#include <math.h>
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
#define BILLION (1E9)
#define CLOCK_REALTIME 0		// Not used in this clock_gettime() port (first arg)
#if defined(_MSC_VER) && (_MSC_VER <= 1800)
struct timespec { long tv_sec; long tv_nsec; };
#endif
static BOOL g_first_time = 1;
static LARGE_INTEGER g_counts_per_sec;

int clock_gettime(int dummy, struct timespec *ct) {
    LARGE_INTEGER count;

    if (g_first_time) {
        g_first_time = 0;

        if (0 == QueryPerformanceFrequency(&g_counts_per_sec)) {
            g_counts_per_sec.QuadPart = 0;
        }
    }

    if ((NULL == ct) || (g_counts_per_sec.QuadPart <= 0) || (0 == QueryPerformanceCounter(&count))) {
        return -1;
    }

    ct->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
    ct->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * BILLION) / g_counts_per_sec.QuadPart;

    return 0;
}
#endif

/*--------------------------------------------------------------------*/
int mbsys_kmbes_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  char *function_name = "mbsys_kmbes_alloc";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_kmbes_struct), (void **)store_ptr, error);

    /* initialize allocated structure to zero */
    if (status == MB_SUCCESS) {
        memset(*store_ptr, 0, sizeof(struct mbsys_kmbes_struct));
    }

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)*store_ptr;

  /* initialize data record kind */
  store->kind = MB_DATA_NONE;

  /* initialize data struct pointers to NULL */
    for (int i = 0; i < MBSYS_KMBES_MAX_NUM_MWC_DGMS; i++)
        store->mwc[i].beamData_p = NULL;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_deall";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)*store_ptr;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_dimensions";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_pingnumber";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)mb_io_ptr->store_data;

  /* extract data from structure */
  if (store->kind == MB_DATA_DATA) {
    *pingnumber = store->mrz[0].cmnPart.pingCnt;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_sonartype";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get sonar type */
  *sonartype = MB_TOPOGRAPHY_TYPE_ECHOSOUNDER;  // TODO: review this setting

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_sidescantype";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get sidescan type */
  *ss_type = MB_SIDESCAN_LINEAR; // TODO: review this setting

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
int mbsys_kmbes_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                                 double *navlon, double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss,
                                 char *beamflag, double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack,
                                 double *ss, double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
  char *function_name = "mbsys_kmbes_extract";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_spo *spo = NULL;
  struct mbsys_kmbes_skm *skm = NULL;
  struct mbsys_kmbes_cpo *cpo = NULL;
  struct mbsys_kmbes_xmc *xmc = NULL;
  struct mbsys_kmbes_xms *xms = NULL;
  double pixel_size;
  int numSoundings = 0;
  int imrz;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  spo = (struct mbsys_kmbes_spo *)&store->spo;
  skm = (struct mbsys_kmbes_skm *)&store->skm;
  cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  xms = (struct mbsys_kmbes_xms *)&store->xms;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* derive overall navigation from first sub-ping for this ping */
    mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

    /* get navigation */
    *navlon = mrz->pingInfo.longitude_deg;
    *navlat = mrz->pingInfo.latitude_deg;

    /* get speed */
    *speed = 0.0;

    /* get heading */
    *heading = mrz->pingInfo.headingVessel_deg;

    /* set beamwidths in mb_io structure */
    mb_io_ptr->beamwidth_xtrack = mrz->pingInfo.receiveArraySizeUsed_deg;
    mb_io_ptr->beamwidth_ltrack = mrz->pingInfo.transmitArraySizeUsed_deg;

    /* read distance and depth values from all sub-pings into storage arrays */
    *nbath = 0;
    *namp = 0;
    *nss = 0;
    numSoundings = 0;
    for (imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        bath[numSoundings] = mrz->sounding[i].z_reRefPoint_m + mrz->pingInfo.z_waterLevelReRefPoint_m;
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
    pixel_size = xms->pixel_size;
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

    /* print debug statements */
    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

    /* print debug statements */
    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

    /* print debug statements */
    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

    /* print debug statements */
    if (verbose >= 5) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

    /* print debug statements */
    if (verbose >= 4) {
      fprintf(stderr, "\ndbg4  Comment extracted by MBIO function <%s>\n", function_name);
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

    /* print debug statements */
    if (verbose >= 4) {
      fprintf(stderr, "\ndbg4  Data extracted by MBIO function <%s>\n", function_name);
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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_insert";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_spo *spo = NULL;
  struct mbsys_kmbes_skm *skm = NULL;
  struct mbsys_kmbes_cpo *cpo = NULL;
  struct mbsys_kmbes_xmc *xmc = NULL;
  struct mbsys_kmbes_xms *xms = NULL;
  int numSoundings;
  int imrz;
  struct timespec right_now_nsec;
  int numBytesComment;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
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
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  spo = (struct mbsys_kmbes_spo *)&store->spo;
  skm = (struct mbsys_kmbes_skm *)&store->skm;
  cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  xms = (struct mbsys_kmbes_xms *)&store->xms;

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
    numSoundings = 0;
    for(imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      mrz->pingInfo.longitude_deg = navlon;
      mrz->pingInfo.latitude_deg = navlat;
      mrz->pingInfo.headingVessel_deg = heading;
      // speed?
      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        mrz->sounding[i].z_reRefPoint_m = bath[numSoundings] - mrz->pingInfo.z_waterLevelReRefPoint_m;
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

  /* insert comment in structure */
  else if (store->kind == MB_DATA_COMMENT) {
    /* copy comment */
    strncpy(store->xmc.comment, comment, MB_COMMENT_MAXLINE-1);
    store->xmc.comment[MB_COMMENT_MAXLINE-1] = '\0';

    /* have to construct this record now */
    numBytesComment = strlen(store->xmc.comment) + (strlen(store->xmc.comment) % 2);
    store->xmc.header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + numBytesComment + 36;
    strncpy((char *)store->xmc.header.dgmType, "#XMC", 4);
    store->xmc.header.dgmVersion = 0;
    store->xmc.header.systemID = 0;
    store->xmc.header.echoSounderID = 0;

    /* insert current time as timestamp if needed (time_d close to zero) */
    if (fabs(time_d) < 1.0) {
      clock_gettime(CLOCK_REALTIME, &right_now_nsec);
      time_d = right_now_nsec.tv_sec + 0.000000001 * right_now_nsec.tv_nsec;
      mb_get_date(verbose, time_d, time_i);
    }
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;
    store->xmc.header.time_sec = (int)time_d;
    store->xmc.header.time_nanosec = (time_d - floor(time_d)) * 1.0e9;
  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_ttimes";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_mrz_tx_sector_info *sectorInfo;
  struct mbsys_kmbes_mrz_sounding *sounding;
  int numSoundings = 0;
  int imrz;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
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
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get depth offset (heave + sonar depth) */
    *ssv = mrz->pingInfo.soundSpeedAtTxDepth_mPerSec;

    /* get draft */
    *draft = mrz->pingInfo.z_waterLevelReRefPoint_m;

    /* read distance and depth values from all sub-pings into storage arrays */
    numSoundings = 0;
    for (imrz = 0; imrz < store->n_mrz_read; imrz++) {

      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        sounding = &mrz->sounding[i];
        sectorInfo = &mrz->sectorInfo[sounding->txSectorNumb];

        ttimes[numSoundings] = sounding->twoWayTravelTime_sec;
        angles[numSoundings] = sounding->beamAngleReRx_deg;
        angles_forward[numSoundings] = 0.0;
        angles_null[numSoundings] = 0.0;
        heave[numSoundings] = 0.0;
        alongtrack_offset[numSoundings] = sectorInfo->sectorTransmitDelay_sec;

        numSoundings++;
      }
      *nbeams = numSoundings;

      /* set status */
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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_detects";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  int numSoundings = 0;
  int imrz;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get detect type for each sounding - options include:
        MB_DETECT_UNKNOWN
        MB_DETECT_AMPLITUDE
        MB_DETECT_PHASE
        MB_DETECT_UNKNOWN */
    numSoundings = 0;
    for (imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_pulses";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  int numSoundings = 0;
  int imrz;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       pulses:     %p\n", (void *)pulses);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get transmit pulse type for each sounding - options include:
          MB_PULSE_TYPE_NUM 5
          MB_PULSE_UNKNOWN 0
          MB_PULSE_CW 1
          MB_PULSE_UPCHIRP 2
          MB_PULSE_DOWNCHIRP 3
          MB_PULSE_LIDAR 4 */
    numSoundings = 0;
    for (imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_gains";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  int numSoundings = 0;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_extract_altitude";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  double xtrackmin;
  int altitude_found = MB_NO;
  int imrz, i;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {

    /* get transducer depth and altitude */
    *transducer_depth = mrz->pingInfo.txTransducerDepth_m;

    /* get altitude using valid depth closest to nadir */
    *altitudev = 0.0;
    xtrackmin = 999999.9;
    for (imrz = 0; imrz < store->n_mrz_read; imrz++) {
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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_extract_nav";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_spo *spo = NULL;
  struct mbsys_kmbes_skm *skm = NULL;
  struct mbsys_kmbes_cpo *cpo = NULL;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  spo = (struct mbsys_kmbes_spo *)&store->spo;
  skm = (struct mbsys_kmbes_skm *)&store->skm;
  cpo = (struct mbsys_kmbes_cpo *)&store->cpo;

  /* get data kind */
  *kind = store->kind;

  /* extract data from survey record */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *navlon = mrz->pingInfo.longitude_deg;
    *navlat = mrz->pingInfo.latitude_deg;

    /* get speed */
    *speed = 0.0;

    /* get heading */
    *heading = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    *roll = 0.0;
    *pitch = 0.0;
    *heave = 0.0;

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
      *roll = 0.0;
      *pitch = 0.0;
      *heave = 0.0;

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
    *roll = 0.0;
    *pitch = 0.0;
    *heave = 0.0;

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
    *roll = 0.0;
    *pitch = 0.0;
    *heave = 0.0;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_extract_nnav";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_spo *spo = NULL;
  struct mbsys_kmbes_skm *skm = NULL;
  struct mbsys_kmbes_cpo *cpo = NULL;
  int i, inav;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       nmax:       %d\n", nmax);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  spo = (struct mbsys_kmbes_spo *)&store->spo;
  skm = (struct mbsys_kmbes_skm *)&store->skm;
  cpo = (struct mbsys_kmbes_cpo *)&store->cpo;

  /* get data kind */
  *kind = store->kind;

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
    speed[0] = 0.0;

    /* get heading */
    heading[0] = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    roll[0] = 0.0;
    pitch[0] = 0.0;
    heave[0] = 0.0;

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
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV1) {
    /* just one navigation value - in some formats there
        are multiple values in nav records to loop over */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = skm->sample[0].KMdefault.longitude_deg;
    navlat[0] = skm->sample[0].KMdefault.latitude_deg;

    /* get speed */
    speed[0] = 3.6 * sqrt(skm->sample[0].KMdefault.velNorth
                          * skm->sample[0].KMdefault.velNorth
                        + skm->sample[0].KMdefault.velEast
                          * skm->sample[0].KMdefault.velEast);

    /* get heading */
    heading[0] = skm->sample[0].KMdefault.heading_deg;

    /* get draft  */
    draft[0] = mrz->pingInfo.txTransducerDepth_m;

    /* get attitude  */
    roll[0] = 0.0;
    pitch[0] = 0.0;
    heave[0] = 0.0;

    /* done translating values */
  }

  /* extract data from nav record */
  else if (*kind == MB_DATA_NAV2) {
    /* just one navigation value - in some formats there
        are multiple values in nav records to loop over */
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
    roll[0] = 0.0;
    pitch[0] = 0.0;
    heave[0] = 0.0;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       n:          %d\n", *n);
    for (inav = 0; inav < *n; inav++) {
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
  char *function_name = "mbsys_kmbes_insert_nav";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_spo *spo = NULL;
  struct mbsys_kmbes_skm *skm = NULL;
  struct mbsys_kmbes_cpo *cpo = NULL;
  int imrz;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
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
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
  spo = (struct mbsys_kmbes_spo *)&store->spo;
  skm = (struct mbsys_kmbes_skm *)&store->skm;
  cpo = (struct mbsys_kmbes_cpo *)&store->cpo;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* loop over all sub-pings */
    for(imrz = 0; imrz < store->n_mrz_read; imrz++) {
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      mrz->pingInfo.longitude_deg = navlon;
      mrz->pingInfo.latitude_deg = navlat;
      mrz->pingInfo.headingVessel_deg = heading;
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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_extract_svp";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_svp *svp = NULL;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  svp = (struct mbsys_kmbes_svp *)&store->svp;

  /* get data kind */
  *kind = store->kind;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_insert_svp";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_svp *svp = NULL;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       nsvp:       %d\n", nsvp);
    for (int i = 0; i < nsvp; i++)
      fprintf(stderr, "dbg2       depth[%d]: %f   velocity[%d]: %f\n", i, depth[i], i, velocity[i]);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  svp = (struct mbsys_kmbes_svp *)&store->svp;

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

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
  char *function_name = "mbsys_kmbes_copy";
  int status = MB_SUCCESS;
  struct mb_io_struct *mb_io_ptr = NULL;
  struct mbsys_kmbes_struct *store = NULL;
  struct mbsys_kmbes_struct *copy = NULL;
  struct mbsys_kmbes_mwc *copy_mwc = NULL;
  struct mbsys_kmbes_mwc *store_mwc = NULL;
  struct mbsys_kmbes_mwc_rx_beam_data *copy_beamData_p;
  struct mbsys_kmbes_mwc_rx_beam_data *store_beamData_p;
  size_t alloc_size;
  int i, j;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  /* get mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointers */
  store = (struct mbsys_kmbes_struct *)store_ptr;
  copy = (struct mbsys_kmbes_struct *)copy_ptr;

  /* copy the data - for many formats memory must be allocated and
      sub-structures copied separately */
  //*copy = *store;
  copy->kind = store->kind;
  copy->time_d = store->time_d;
  for (i=0;i<7;i++) {
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
  for (i=0;i<MBSYS_KMBES_MAX_NUM_MRZ_DGMS;i++) {
    copy->mrz[i] = store->mrz[i];
  }
  copy->xms = store->xms;
  copy->n_mwc_read = store->n_mwc_read;
  copy->n_mwc_needed = store->n_mwc_needed;
  for (i=0;i<MBSYS_KMBES_MAX_NUM_MWC_DGMS;i++) {
    copy_mwc = &copy->mwc[i];
    store_mwc = &store->mwc[i];

    copy_mwc->header = store_mwc->header;
    copy_mwc->partition = store_mwc->partition;
    copy_mwc->cmnPart = store_mwc->cmnPart;
    copy_mwc->txInfo = store_mwc->txInfo;
    for (j=0;j<MBSYS_KMBES_MAX_NUM_TX_PULSES;j++) {
      copy_mwc->sectorData[j] = store_mwc->sectorData[j];
    }
    copy_mwc->rxInfo = store_mwc->rxInfo;

    alloc_size = (size_t)(store_mwc->rxInfo.numBeams * sizeof(struct mbsys_kmbes_mwc_rx_beam_data));
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
      for (j=0;j<store_mwc->rxInfo.numBeams;j++) {
        copy_beamData_p = &copy_mwc->beamData_p[j];
        store_beamData_p = &store_mwc->beamData_p[j];

        copy_beamData_p->beamPointAngReVertical_deg = store_beamData_p->beamPointAngReVertical_deg;
        copy_beamData_p->startRangeSampleNum = store_beamData_p->startRangeSampleNum;
        copy_beamData_p->detectedRangeInSamples = store_beamData_p->detectedRangeInSamples;
        copy_beamData_p->beamTxSectorNum = store_beamData_p->beamTxSectorNum;
        copy_beamData_p->numSampleData = store_beamData_p->numSampleData;

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
  copy->ib = store->ib;
  copy->xmb = store->xmb;
  copy->xmc = store->xmc;
  copy->unknown = store->unknown;

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_kmbes_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
             int swath_width_set, double *swath_width, int pixel_int, int *error) {
  char *function_name = "mbsys_kmbes_makess";
  int status = MB_SUCCESS;
  struct mbsys_kmbes_struct *store;
  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_xms *xms = NULL;
  double ss[MBSYS_KMBES_MAX_PIXELS];
  int ss_cnt[MBSYS_KMBES_MAX_PIXELS];
  double ssacrosstrack[MBSYS_KMBES_MAX_PIXELS];
  double ssalongtrack[MBSYS_KMBES_MAX_PIXELS];
  int nbathsort;
  double bathsort[MBSYS_KMBES_MAX_PIXELS];
  int nsoundings, nsamples;
  double median_altitude;
  double pixel_size_calc;
  double sample_size_m;
  int pixel_int_use;
  int first, last, k1, kc, k2;
  double dx1, dx2, dx, xx;
  int imrz;
  int i, k, kk;

  /* print input debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
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

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  store = (struct mbsys_kmbes_struct *)store_ptr;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {

    /* initialize the sidescan binning arrays */
    for (i=0;i<MBSYS_KMBES_MAX_PIXELS;i++) {
      ss[i] = 0.0;
      ss_cnt[i] = 0;
      ssacrosstrack[i] = 0.0;
      ssalongtrack[i] = 0.0;
    }

    /* if not set get swath width from sonar settings */
    if (swath_width_set == MB_NO) {
      *swath_width = MAX(fabs(store->mrz[0].pingInfo.portSectorEdge_deg),
                            fabs(store->mrz[0].pingInfo.starbSectorEdge_deg));
    }

    /* get median altitude if needed to calculate pixel size in meters */
    if (pixel_size_set == MB_NO) {

      /* loop over sub-ping datagrams */
      nbathsort = 0;
      for(imrz = 0; imrz < store->n_mrz_read; imrz++) {

        /* get the current MRZ datagram */
        mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

        /* loop over all soundings to get the median altitude for this ping */
        for (int i = 0; i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections); i++) {
          if (mb_beam_ok(mrz->sounding[i].beamflag)) {
            bathsort[nbathsort] = mrz->sounding[i].z_reRefPoint_m
                                    + mrz->pingInfo.z_waterLevelReRefPoint_m
                                    - mrz->pingInfo.txTransducerDepth_m;
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
    for(imrz = 0; imrz < store->n_mrz_read; imrz++) {

      /* get the current MRZ datagram */
      mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      /* get raw sample size */
      sample_size_m = 750.0 / mrz->rxInfo.seabedImageSampleRate;

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
          dx = sample_size_m * sin(DTR * mrz->sounding[i].beamAngleReRx_deg);

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
            for (k=k1;k<kc;k++) {
              xx = mrz->sounding[i].y_reRefPoint_m - dx2 * (k - kc);
              kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS) {
                ss[kk] += 10.0 * mrz->SIsample_desidB[k];
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
            for (k=kc;k<=k2;k++) {
              xx = mrz->sounding[i].y_reRefPoint_m - dx1 * (k - kc);
              kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS) {
                ss[kk] += 10.0 * mrz->SIsample_desidB[k];
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
            for (k=k1;k<kc;k++) {
              xx = mrz->sounding[i].y_reRefPoint_m + dx1 * (k - kc);
              kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS) {
                ss[kk] += 10.0 * mrz->SIsample_desidB[k];
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
            for (k=kc;k<=k2;k++) {
              xx = mrz->sounding[i].y_reRefPoint_m + dx2 * (k - kc);
              kk = MBSYS_KMBES_MAX_PIXELS / 2 + (int)(xx / (*pixel_size));
              if (kk > 0 && kk < MBSYS_KMBES_MAX_PIXELS) {
                ss[kk] += 10.0 * mrz->SIsample_desidB[k];
                ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
                ss_cnt[kk]++;
              }
            }
          }
          nsamples += mrz->sounding[i].SInumSamples;
//fprintf(stderr,"SS calculation: sounding %d angle:%f dx: %f %f %f N: %2d %4d %4d %4d\n",
//i, mrz->sounding[i].beamAngleReRx_deg, dx, dx1, dx2,
//mrz->sounding[i].SInumSamples, k1, kc, k2);
        }
      }
    }

		/* average the sidescan */
		first = MBSYS_KMBES_MAX_PIXELS;
		last = -1;
		for (k = 0; k < MBSYS_KMBES_MAX_PIXELS; k++) {
			if (ss_cnt[k] > 0) {
				ss[k] /= ss_cnt[k];
				ssalongtrack[k] /= ss_cnt[k];
				ssacrosstrack[k] = (k - MBSYS_KMBES_MAX_PIXELS / 2) * (*pixel_size);
				first = MIN(first, k);
				last = k;
			}
			else
				ss[k] = MB_SIDESCAN_NULL;
		}

		/* interpolate the sidescan */
		k1 = first;
		k2 = first;
		for (k = first + 1; k < last; k++) {
			if (ss_cnt[k] <= 0) {
				if (k2 <= k) {
					k2 = k + 1;
					while (ss_cnt[k2] <= 0 && k2 < last)
						k2++;
				}
				if (k2 - k1 <= pixel_int_use) {
					ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
					ssacrosstrack[k] = (k - MBSYS_KMBES_MAX_PIXELS / 2) * (*pixel_size);
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
    mrz = (struct mbsys_kmbes_mrz *)&store->mrz[0];
    xms = (struct mbsys_kmbes_xms *)&store->xms;
    xms->header = mrz->header;
    xms->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + 8 * MBSYS_KMBES_MAX_PIXELS + 48;
    strncpy((char *)xms->header.dgmType, "#XMS", 4);
    xms->pingCnt = mrz->cmnPart.pingCnt;
    xms->spare = 0;
    xms->pixel_size = *pixel_size;
    xms->pixels_ss = MBSYS_KMBES_MAX_PIXELS;
    memset((char *)xms->unused, 0, 32);
    for (k=0;k<xms->pixels_ss;k++) {
      xms->ss[k] = ss[k];
      xms->ss_alongtrack[k] = ssalongtrack[k];
    }

  }

  /* print output debug statements */
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
