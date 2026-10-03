 /*--------------------------------------------------------------------
 *    The MB-system:  mbsys_kmbes.c  3.00  5/25/2018
 *
 *    Copyright (c) 2018-2025 by
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
 * mbsys_kmbes.c contains the MBIO functions for handling data from
 * the following data formats:
 *    MBSYS_KMBES formats (code in mbsys_kmbes.c and mbsys_kmbes.h):
 *      MBF_KEMKMALL : MBIO ID 261 (code in mbr_kemkmall.c)
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

/* MinGW-w64 already supplies clock_gettime() via pthread_time.h (included
 * from time.h), so this replacement is only needed for MSVC. */
#if defined(_MSC_VER)

/* Based on https://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows */
/* - modified to not use static variables for thread safety */
#include <Windows.h>
#define CLOCK_REALTIME 0
#if _MSC_VER <= 1800
struct timespec { long tv_sec; long tv_nsec; };
#endif

int clock_gettime(int dummy, struct timespec *ct) {
    LARGE_INTEGER g_counts_per_sec, count;
    if (NULL == ct) {
      return -1;
    }
    if (0 == QueryPerformanceFrequency(&g_counts_per_sec) || g_counts_per_sec.QuadPart == 0) {
      return -1;
    }
    if (0 == QueryPerformanceCounter(&count)) {
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

  /* -1 means "not yet parsed from iip.install_txt" - see mbsys_kmbes_active_attitude_system() */
  store->active_attitude_system = -1;

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
    if (store->msc.rawbytes != NULL && store->msc.num_rawbytes_alloc > 0) {
      store->msc.num_rawbytes_alloc = 0;
      store->msc.num_rawbytes = 0;
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&store->msc.rawbytes), error);
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
/* A Kongsberg installation can log #SKM (attitude/heading) samples from more
    than one physical sensor system - mbr_kemkmall_rd_skm() always assigns
    sensorSystem 0 to kind MB_DATA_NAV1 and sensorSystem 1 to kind
    MB_DATA_NAV2, with no guarantee that NAV1 is the one actually used by the
    sonar's own real-time processing. The #IIP installation datagram's
    free-text install_txt separately documents, per sensor, an ATTI_<n> line
    (1-based n) ending in "U=ACTIVE" or "U=PASSIVE" (or "U=NOT_SET"); ATTI_1
    corresponds to MB_DATA_NAV1 (sensorSystem 0) and ATTI_2 to MB_DATA_NAV2
    (sensorSystem 1). This parses install_txt once (caching the result in
    store->active_attitude_system) and returns MB_DATA_NAV1 or MB_DATA_NAV2
    according to whichever ATTI_<n> line is marked active, or -1 if
    install_txt has not been read yet or no line is unambiguously marked
    active. It is purely informational - mbpreprocess's --nav-async,
    --heading-async, and --attitude-async options are what actually select
    which of MB_DATA_NAV1/MB_DATA_NAV2 is merged, since only the user knows
    (e.g. from this value, or from reading install_txt directly) whether a
    given installation's #IIP tagging can be trusted. */
int mbsys_kmbes_active_attitude_system(void *store_ptr) {
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  if (store->active_attitude_system != -1 || store->iip.install_txt[0] == '\0') {
    return store->active_attitude_system;
  }

  const char *line = (const char *)store->iip.install_txt;
  const char *const txt_end = line + sizeof(store->iip.install_txt);
  int active_kind = -1;
  while (line < txt_end && *line != '\0') {
    const char *comma = memchr(line, ',', (size_t)(txt_end - line));
    const char *lineend = comma != NULL ? comma : (const char *)memchr(line, '\0', (size_t)(txt_end - line));
    if (lineend == NULL)
      lineend = txt_end;
    int atti_num = 0;
    if (lineend - line > 5 && strncmp(line, "ATTI_", 5) == 0 && sscanf(line + 5, "%d", &atti_num) == 1) {
      for (const char *p = line; p < lineend - 2; p++) {
        if (strncmp(p, ";U=", 3) == 0) {
          if (strncmp(p + 3, "ACTIVE", 6) == 0) {
            if (atti_num == 1)
              active_kind = MB_DATA_NAV1;
            else if (atti_num == 2)
              active_kind = MB_DATA_NAV2;
          }
          break;
        }
      }
    }
    line = comma != NULL ? comma + 1 : txt_end;
  }

  store->active_attitude_system = active_kind;
  return active_kind;
}

/*--------------------------------------------------------------------*/
/* Helper functions for parsing the #IIP datagram's free-text install_txt,
    used by mbsys_kmbes_extract_platform() below. install_txt is a series of
    comma-delimited segments; most have the form "PREFIX:key1=val1;key2=
    val2;..." (e.g. "TRAI_TX1:N=0;X=4.221;Y=0.914;Z=6.225;R=0.060;P=-0.070;
    H=0.120;S=1.0") but a few are bare "KEY=value" (e.g. "SN=10055") or
    "PREFIX:freetext" with no key=value structure (e.g. "SYSTEM:EM 124"). A
    single '\n' may follow any comma as cosmetic formatting and is skipped
    when looking for the next segment's prefix. */

/* Finds the first install_txt segment beginning with the exact string
    prefix (e.g. "TRAI_TX1:" or "SN=") and returns the segment's remaining
    text (after prefix) in [*value_start, *value_end). Returns false if no
    such segment is found or install_txt has not been read yet. */
static bool mbsys_kmbes_iip_segment(void *store_ptr, const char *prefix, const char **value_start, const char **value_end) {
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  const size_t prefixlen = strlen(prefix);
  const char *p = (const char *)store->iip.install_txt;
  const char *const txt_end = p + sizeof(store->iip.install_txt);
  while (p < txt_end && *p != '\0') {
    const char *comma = memchr(p, ',', (size_t)(txt_end - p));
    const char *segend = comma != NULL ? comma : (const char *)memchr(p, '\0', (size_t)(txt_end - p));
    if (segend == NULL)
      segend = txt_end;
    if ((size_t)(segend - p) >= prefixlen && strncmp(p, prefix, prefixlen) == 0) {
      *value_start = p + prefixlen;
      *value_end = segend;
      return true;
    }
    p = comma != NULL ? comma + 1 : txt_end;
    if (p < txt_end && *p == '\n')
      p++;
  }
  return false;
}

/* Within a single install_txt segment's value range [start, end) (as
    returned by mbsys_kmbes_iip_segment()), locates the value of a ';'
    delimited "key=value" field. Returns false if the field is not present. */
static bool mbsys_kmbes_iip_field(const char *start, const char *end, const char *key, const char **value_start,
                                   size_t *value_len) {
  const size_t keylen = strlen(key);
  const char *p = start;
  while (p < end) {
    const char *semi = memchr(p, ';', (size_t)(end - p));
    const char *fieldend = semi != NULL ? semi : end;
    if ((size_t)(fieldend - p) > keylen && strncmp(p, key, keylen) == 0 && p[keylen] == '=') {
      *value_start = p + keylen + 1;
      *value_len = (size_t)(fieldend - *value_start);
      return true;
    }
    p = semi != NULL ? semi + 1 : end;
  }
  return false;
}

/* Parses a ';'-delimited "key=value" field within [start, end) as a double.
    Returns false if the field is absent or not parseable as a number. */
static bool mbsys_kmbes_iip_double(const char *start, const char *end, const char *key, double *value) {
  const char *value_start;
  size_t value_len;
  if (!mbsys_kmbes_iip_field(start, end, key, &value_start, &value_len) || value_len == 0)
    return false;
  char buffer[32];
  if (value_len >= sizeof(buffer))
    value_len = sizeof(buffer) - 1;
  memcpy(buffer, value_start, value_len);
  buffer[value_len] = '\0';
  return sscanf(buffer, "%lf", value) == 1;
}

/* Returns true if the ';'-delimited "U=" field within [start, end) is
    present and set to "ACTIVE" (as opposed to "PASSIVE" or "NOT_SET"). */
static bool mbsys_kmbes_iip_active(const char *start, const char *end) {
  const char *value_start;
  size_t value_len;
  return mbsys_kmbes_iip_field(start, end, "U", &value_start, &value_len) && value_len == 6 &&
         strncmp(value_start, "ACTIVE", 6) == 0;
}

/*--------------------------------------------------------------------*/
/* Constructs a platform model from the #IIP installation datagram's
    install_txt free text, following the pattern of
    mbsys_simrad3_extract_platform(). The multibeam sensor's two offsets come
    from the TRAI_TX1/TRAI_RX1 lines (transmit/receive transducer; model name
    from the SYSTEM line); up to three position sensors come from the
    POSI_1/POSI_2/POSI_3 lines; up to two motion (attitude) sensors come from
    the ATTI_1/ATTI_2 lines. Whichever POSI_n/ATTI_n line is marked
    "U=ACTIVE" selects the platform's primary source of
    position/heading/rollpitch/heave. install_txt reports offsets in
    Kongsberg's own SCS convention (X=forward, Y=starboard, Z=down); as in
    mbsys_simrad3_extract_platform(), these are swapped/negated to match
    MB-System's platform convention. */
int mbsys_kmbes_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:       %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
  }

  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* if needed allocate a new platform structure */
  if (*platform_ptr == NULL) {
    status = mb_platform_init(verbose, (void **)platform_ptr, error);
  }

  if (*platform_ptr != NULL && store->iip.install_txt[0] != '\0') {
    struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

    const char *value_start;
    const char *value_end;

    /* look for multibeam sensor, add it if necessary */
    int sensor_multibeam = -1;
    for (int isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
      if (platform->sensors[isensor].type == MB_SENSOR_TYPE_SONAR_MULTIBEAM) {
        sensor_multibeam = isensor;
      }
    }
    if (sensor_multibeam < 0) {
      mb_longname multibeam_model;
      strcpy(multibeam_model, "Unknown");
      if (mbsys_kmbes_iip_segment(store, "SYSTEM:", &value_start, &value_end)) {
        size_t len = (size_t)(value_end - value_start);
        if (len >= sizeof(multibeam_model))
          len = sizeof(multibeam_model) - 1;
        memcpy(multibeam_model, value_start, len);
        multibeam_model[len] = '\0';
      }
      const int capability1 = MB_SENSOR_CAPABILITY1_NONE;
      const int capability2 = MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM + MB_SENSOR_CAPABILITY2_BACKSCATTER_MULTIBEAM;
      const int num_offsets = 2;
      const int num_time_latency = 0;
      status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_SONAR_MULTIBEAM, multibeam_model, "Kongsberg",
                                      NULL, capability1, capability2, num_offsets, num_time_latency, error);
      if (status == MB_SUCCESS) {
        sensor_multibeam = platform->num_sensors - 1;
      }
    }
    if (sensor_multibeam >= 0 && status == MB_SUCCESS) {
      platform->source_bathymetry = sensor_multibeam;
      platform->source_backscatter = sensor_multibeam;

      if (platform->sensors[sensor_multibeam].num_offsets > 0 &&
          mbsys_kmbes_iip_segment(store, "TRAI_TX1:", &value_start, &value_end)) {
        double x = 0.0, y = 0.0, z = 0.0, r = 0.0, p = 0.0, h = 0.0;
        mbsys_kmbes_iip_double(value_start, value_end, "X", &x);
        mbsys_kmbes_iip_double(value_start, value_end, "Y", &y);
        mbsys_kmbes_iip_double(value_start, value_end, "Z", &z);
        mbsys_kmbes_iip_double(value_start, value_end, "R", &r);
        mbsys_kmbes_iip_double(value_start, value_end, "P", &p);
        mbsys_kmbes_iip_double(value_start, value_end, "H", &h);
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, sensor_multibeam, 0, y, x, -z, h, r, p, error);
      }
      if (status == MB_SUCCESS && platform->sensors[sensor_multibeam].num_offsets > 1 &&
          mbsys_kmbes_iip_segment(store, "TRAI_RX1:", &value_start, &value_end)) {
        double x = 0.0, y = 0.0, z = 0.0, r = 0.0, p = 0.0, h = 0.0;
        mbsys_kmbes_iip_double(value_start, value_end, "X", &x);
        mbsys_kmbes_iip_double(value_start, value_end, "Y", &y);
        mbsys_kmbes_iip_double(value_start, value_end, "Z", &z);
        mbsys_kmbes_iip_double(value_start, value_end, "R", &r);
        mbsys_kmbes_iip_double(value_start, value_end, "P", &p);
        mbsys_kmbes_iip_double(value_start, value_end, "H", &h);
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, sensor_multibeam, 1, y, x, -z, h, r, p, error);
      }
    }

    /* position sensors POSI_1/POSI_2/POSI_3 -> source_position1/2/3 */
    const char *const posi_prefix[3] = {"POSI_1:", "POSI_2:", "POSI_3:"};
    int *const source_position_n[3] = {&platform->source_position1, &platform->source_position2,
                                        &platform->source_position3};
    for (int i = 0; i < 3 && status == MB_SUCCESS; i++) {
      if (!mbsys_kmbes_iip_segment(store, posi_prefix[i], &value_start, &value_end))
        continue;
      if (*source_position_n[i] < 0) {
        const int capability1 = MB_SENSOR_CAPABILITY1_POSITION;
        const int capability2 = MB_SENSOR_CAPABILITY2_NONE;
        status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, capability1,
                                        capability2, 1, 0, error);
        if (status == MB_SUCCESS)
          *source_position_n[i] = platform->num_sensors - 1;
      }
      if (status == MB_SUCCESS && *source_position_n[i] >= 0 &&
          platform->sensors[*source_position_n[i]].num_offsets == 1) {
        double x = 0.0, y = 0.0, z = 0.0;
        mbsys_kmbes_iip_double(value_start, value_end, "X", &x);
        mbsys_kmbes_iip_double(value_start, value_end, "Y", &y);
        mbsys_kmbes_iip_double(value_start, value_end, "Z", &z);
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, *source_position_n[i], 0, y, x, -z, 0.0, 0.0, 0.0,
                                               error);
      }
      if (status == MB_SUCCESS && mbsys_kmbes_iip_active(value_start, value_end)) {
        platform->source_position = *source_position_n[i];
      }
    }

    /* motion/attitude sensors ATTI_1/ATTI_2 -> source_rollpitch1/2 */
    const char *const atti_prefix[2] = {"ATTI_1:", "ATTI_2:"};
    int *const source_rollpitch_n[2] = {&platform->source_rollpitch1, &platform->source_rollpitch2};
    for (int i = 0; i < 2 && status == MB_SUCCESS; i++) {
      if (!mbsys_kmbes_iip_segment(store, atti_prefix[i], &value_start, &value_end))
        continue;
      if (*source_rollpitch_n[i] < 0) {
        const int capability1 = MB_SENSOR_CAPABILITY1_ROLLPITCH + MB_SENSOR_CAPABILITY1_HEADING + MB_SENSOR_CAPABILITY1_HEAVE;
        const int capability2 = MB_SENSOR_CAPABILITY2_NONE;
        status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, capability1,
                                        capability2, 1, 0, error);
        if (status == MB_SUCCESS)
          *source_rollpitch_n[i] = platform->num_sensors - 1;
      }
      if (status == MB_SUCCESS && *source_rollpitch_n[i] >= 0 &&
          platform->sensors[*source_rollpitch_n[i]].num_offsets == 1) {
        double x = 0.0, y = 0.0, z = 0.0, r = 0.0, p = 0.0, h = 0.0;
        mbsys_kmbes_iip_double(value_start, value_end, "X", &x);
        mbsys_kmbes_iip_double(value_start, value_end, "Y", &y);
        mbsys_kmbes_iip_double(value_start, value_end, "Z", &z);
        mbsys_kmbes_iip_double(value_start, value_end, "R", &r);
        mbsys_kmbes_iip_double(value_start, value_end, "P", &p);
        mbsys_kmbes_iip_double(value_start, value_end, "H", &h);
        /* The R, H and P of an ATTI_n line are corrections that Kongsberg adds to the angles the
            sensor measures to get vessel angles (checked against #SKM data and GSF roll, pitch
            and heading). The platform model instead gives the orientation of the sensor
            relative to the platform, which is subtracted, so the angles are negated here. */
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, *source_rollpitch_n[i], 0, y, x, -z, -h, -r, -p, error);
      }
      if (status == MB_SUCCESS && mbsys_kmbes_iip_active(value_start, value_end)) {
        platform->source_rollpitch = *source_rollpitch_n[i];
        platform->source_heading = *source_rollpitch_n[i];
        platform->source_heave = *source_rollpitch_n[i];
      }
    }

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }
  else if (*platform_ptr == NULL) {
    *error = MB_ERROR_OPEN_FAIL;
    status = MB_FAILURE;
    fprintf(stderr, "\nUnable to initialize platform offset structure\n");
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:           %d\n", *kind);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
    fprintf(stderr, "dbg2       error:          %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

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
/* Convert a heading, roll and pitch measured by the platform's heading and
    attitude sensors into the platform (vessel) frame using the platform model.
    The roll and pitch are always converted. The heading is only converted if
    convert_heading is true, since a heading taken from the MRZ datagram is already
    the vessel heading. If platform is NULL, or the conversion fails, the values
    are returned unchanged. */
static void mbsys_kmbes_platform_orientation(int verbose, struct mb_platform_struct *platform, bool convert_heading,
                                              double heading, double roll, double pitch, double *platform_heading,
                                              double *platform_roll, double *platform_pitch) {
  *platform_heading = heading;
  *platform_roll = roll;
  *platform_pitch = pitch;
  if (platform != NULL) {
    double h;
    double r;
    double p;
    int orientation_error = MB_ERROR_NO_ERROR;
    if (mb_platform_orientation(verbose, (void *)platform, heading, roll, pitch, &h, &r, &p, &orientation_error)
        == MB_SUCCESS) {
      *platform_roll = r;
      *platform_pitch = p;
      if (convert_heading)
        *platform_heading = h;
    }
  }
}

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

  *error = MB_ERROR_NO_ERROR;

  /* check for non-null data */
  assert(mbio_ptr != NULL);
  assert(preprocess_pars_ptr != NULL);

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get preprocessing parameters */
  struct mb_preprocess_struct *pars = (struct mb_preprocess_struct *)preprocess_pars_ptr;

  /* This function calculates MB-System extensions to the original data including
  		beam angles and travel times set for raytracing and pseudosidescan. The bathymetry
  		(seafloor depths, acrosstrack and alongtrack distances) are not recalculated.
  		Function mbr_rt_kemkmall() calls this function automatically on the first read of
      any not-yet-preprocessed ping (e.g. including XMT and XMS datagrams), passing 
      &mb_io_ptr->preprocess_pars - a struct it fills in from its own per-file-handle 
      nav/attitude buffer, which only holds whatever asynchronous data this same read 
      pass has encountered so far in the current file. That is necessarily incomplete for 
      the first several pings of a file (and, when a program such as mbpreprocess reads 
      the same raw file more than once - once per its own pass, once per file in a 
      multi-file datalist - transiently incomplete every time), even though a later, 
      explicitly-driven call (e.g. from mbpreprocess's own second pass, using its own 
      file-spanning tables) may have complete data and will overwrite this preliminary 
      result before anything is written out. Silently missing attitude here is expected 
      and, for that automatic call, non-final, so the "no attitude data" warning below is 
      limited to calls using a caller-supplied preprocess_pars struct, where a lack of 
      data is not superseded by any later call and so is worth reporting. */
  const bool pars_is_mbio_internal = (pars == &mb_io_ptr->preprocess_pars);

  /* data structure pointers */
  struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
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
  if (verbose >= 5) {
    int time_i[7];
    fprintf(stderr, "dbg5       n_nav:                         %d\n", pars->n_nav);
    for (int i = 0; i < pars->n_nav; i++) {
      mb_get_date(0, pars->nav_time_d[i], time_i);
      fprintf(stderr, "dbg5         %d %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %15.6f %14.10f %14.10f %6.3f\n",
    				i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
    				pars->nav_time_d[i], pars->nav_lon[i], pars->nav_lat[i], pars->nav_speed != NULL ? pars->nav_speed[i] : 0.0);
    }
    fprintf(stderr, "dbg2       n_sensordepth:                 %d\n", pars->n_sensordepth);
    for (int i = 0; i < pars->n_sensordepth; i++) {
      mb_get_date(0, pars->sensordepth_time_d[i], time_i);
      fprintf(stderr, "dbg5         %d %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %15.6f %8.3f\n", 
    				i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
    				pars->sensordepth_time_d[i], pars->sensordepth_sensordepth[i]);
    }
    fprintf(stderr, "dbg2       n_heading:                     %d\n", pars->n_heading);
    for (int i = 0; i < pars->n_heading; i++) {
      mb_get_date(0, pars->heading_time_d[i], time_i);
      fprintf(stderr, "dbg5         %d %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %15.6f %6.3f\n", 
    				i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
    				pars->heading_time_d[i], pars->heading_heading[i]);
    }
    fprintf(stderr, "dbg2       n_altitude:                    %d\n", pars->n_altitude);
    for (int i = 0; i < pars->n_altitude; i++) {
      mb_get_date(0, pars->altitude_time_d[i], time_i);
      fprintf(stderr, "dbg5         %d %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %15.6f %6.3f\n", 
    				i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
    				pars->altitude_time_d[i], pars->altitude_altitude[i]);
    }
    fprintf(stderr, "dbg2       n_attitude:                    %d\n", pars->n_attitude);
    for (int i = 0; i < pars->n_attitude; i++) {
      mb_get_date(0, pars->attitude_time_d[i], time_i);
      fprintf(stderr, "dbg5         %d %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d %15.6f %6.3f %6.3f %6.3f\n", 
    				i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
    				pars->attitude_time_d[i], pars->attitude_roll[i], pars->attitude_pitch[i], pars->attitude_heave[i]);
    }
  }

  int status = MB_SUCCESS;

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

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

  /* deal with a survey record */
  else if (store->kind == MB_DATA_DATA) {

    /*--------------------------------------------------------------*/
    /* change timestamp if indicated */
    /*--------------------------------------------------------------*/
    if (pars->timestamp_changed) {
      /* set time */
      double time_d_old = store->time_d;
      int time_i_old[7];
      mb_get_date(verbose, time_d_old, time_i_old);
      mb_get_date(verbose, pars->time_d, time_i);
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = pars->time_d;
      fprintf(stderr,
              "Timestamp changed in function %s: "
              "from %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
              "to %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d "
              "| ping_number:%d\n",
              __func__,
              time_i_old[0], time_i_old[1], time_i_old[2], time_i_old[3], time_i_old[4], time_i_old[5], time_i_old[6],
              time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
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
    double soundspeedsnellfactor = 1.0;

    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];

      // get time
      if (pars->timestamp_changed) {
        mrz->header.time_sec = (unsigned int)floor(pars->time_d);
        mrz->header.time_nanosec = (unsigned int)((pars->time_d - (double)mrz->header.time_sec) * 1.0e9);
      }
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

      /* Establish baseline navigation and heading for this ping from the ping's own
          logged values, so that navlon/navlat/heading are always defined even before
          any external navigation/heading data has been received (e.g. for the first
          ping(s) of a file whose timestamps can precede the first asynchronous
          navigation/heading datagram). These baselines are overridden below by the
          file-wide interpolated values whenever those are available. */
      navlon = mrz->pingInfo.longitude_deg;
      navlat = mrz->pingInfo.latitude_deg;
      heading = mrz->pingInfo.headingVessel_deg;

      xmt->xmtPingInfo.lever_acrosstrack = 0.0;
      xmt->xmtPingInfo.lever_alongtrack = 0.0;
      xmt->xmtPingInfo.speed = 0.0;
      if (spo->sensorData.speedOverGround_mPerSec > 0.0)
        xmt->xmtPingInfo.speed = spo->sensorData.speedOverGround_mPerSec;
      else if (cpo->sensorData.speedOverGround_mPerSec > 0.0)
        xmt->xmtPingInfo.speed = cpo->sensorData.speedOverGround_mPerSec;

      xmt->xmtPingInfo.sensordepth = mrz->pingInfo.txTransducerDepth_m;

      /* Establish a baseline roll/pitch/heave for this ping from the ping's own embedded
          attitude sample, so that these are always defined even before any external
          navigation/attitude data has been received (e.g. for the first ping(s) of a file
          whose timestamps can precede the first asynchronous navigation/attitude datagram).
          This baseline is overridden below by the file-wide interpolated attitude whenever
          that is available. */
      if (skm->infoPart.numSamplesArray > 0) {
        roll = skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.roll_deg;
        pitch = skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.pitch_deg;
        heave = kluge_auvsentrysensordepth ? 0.0 : skm->sample[skm->infoPart.numSamplesArray-1].KMdefault.heave_m;
      }
      else {
        roll = 0.0;
        pitch = 0.0;
        heave = 0.0;
      }
      xmt->xmtPingInfo.roll = roll;
      xmt->xmtPingInfo.pitch = pitch;
      xmt->xmtPingInfo.heave = heave;

      /* Interpolate heading. The heading logged in the MRZ is only replaced when
          a heading array is defined. */
      bool heading_from_array = false;
      if (pars->n_heading > 0) {
        /* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                 pars->n_heading, time_d, &heading, &jheading, &interp_error);
        heading_from_array = true;
      }

      /* interpolate altitude */
      //if (pars->n_altitude > 0) {
      //  interp_status = mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
      //                                    time_d, &altitude, &jaltitude, &interp_error);
      //}

      /* interpolate attitude - the MRZ has no roll, pitch or heave, so these always
          come from the attitude arrays when they are defined */
      if (pars->n_attitude > 0) {
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                         time_d, &roll, &jattitude, &interp_error);
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                         time_d, &pitch, &jattitude, &interp_error);
        /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                         time_d, &heave, &jattitude, &interp_error);
      }
      else if (skm->infoPart.numSamplesArray == 0 && !pars_is_mbio_internal && verbose >= 1) {
        fprintf(stderr, "Warning: mbsys_kmbes_preprocess: no attitude data (external or embedded "
                "in this ping) available for ping at time %.6f - roll/pitch/heave defaulted to 0.0.\n",
                time_d);
      }

      /* Heading, roll and pitch interpolated from sensor arrays are in the frame of
          the sensors that measured them. Keep those values for the per-sounding
          interpolation below, and convert the ping values to the platform frame
          when a platform model is defined. A heading taken from the MRZ is already
          the vessel heading and is not converted. */
      const double sensor_heading = heading;
      const double sensor_roll = roll;
      const double sensor_pitch = pitch;
      mbsys_kmbes_platform_orientation(verbose, platform, heading_from_array, sensor_heading, sensor_roll, sensor_pitch,
                                       &heading, &roll, &pitch);
      if (heading_from_array) {
        mrz->pingInfo.headingVessel_deg = heading;
        xmt->xmtPingInfo.heading = heading;
      }
      xmt->xmtPingInfo.roll = roll;
      xmt->xmtPingInfo.pitch = pitch;
      xmt->xmtPingInfo.heave = kluge_auvsentrysensordepth ? 0.0 : heave;

      /* interpolate nav */
      if (pars->n_nav > 0) {
        int interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1,
                                                    pars->nav_lon - 1, pars->n_nav,
                                                    time_d, &navlon, &jnav, &interp_error);
        interp_status &= mb_linear_interp_latitude(verbose, pars->nav_time_d - 1,
                                                    pars->nav_lat - 1, pars->n_nav,
                                                    time_d, &navlat, &jnav, &interp_error);

        /* The MRZ navigation is the position of the vessel reference point, which is
            the origin of the platform model. The interpolated navigation is the position
            of the navigation sensor, so move it to the vessel reference point when a
            platform model is defined. The navigation arrays that mbr_rt_kemkmall()
            passes (pars_is_mbio_internal) are only used for the ping speed, and leave the
            logged MRZ navigation unchanged. */
        if (!pars_is_mbio_internal) {
          double origin_lon = navlon;
          double origin_lat = navlat;
          double origin_depth;
          int lever_error = MB_ERROR_NO_ERROR;
          if (platform == NULL
              || mb_platform_position_platform(verbose, (void *)platform, -1, 0, navlon, navlat, 0.0, heading, roll,
                                               pitch, &origin_lon, &origin_lat, &origin_depth, &lever_error) != MB_SUCCESS) {
            origin_lon = navlon;
            origin_lat = navlat;
          }
          mrz->pingInfo.longitude_deg = origin_lon;
          mrz->pingInfo.latitude_deg = origin_lat;
          xmt->xmtPingInfo.longitude = origin_lon;
          xmt->xmtPingInfo.latitude = origin_lat;
        }

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

      /* Interpolate sensordepth. The sensordepth array takes precedence over the
          Sentry kluge, which takes precedence over the transducer depth logged in
          the MRZ. A sensordepth taken from an array is the depth at the depth sensor,
          so move it to the transducer when a platform model is defined. */
      double sensordepth_lever = 0.0;
      if (pars->n_sensordepth > 0) {
        /* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                        pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);
        if (platform != NULL) {
          double target_lon;
          double target_lat;
          double target_depth;
          int lever_error = MB_ERROR_NO_ERROR;
          if (mb_platform_position_platform(verbose, (void *)platform, pars->target_sensor, 0,
                                            mrz->pingInfo.longitude_deg, mrz->pingInfo.latitude_deg, sensordepth,
                                            heading, roll, pitch, &target_lon, &target_lat, &target_depth,
                                            &lever_error) == MB_SUCCESS) {
            sensordepth_lever = target_depth - sensordepth;
            sensordepth = target_depth;
          }
        }
        mrz->pingInfo.txTransducerDepth_m = sensordepth;
        xmt->xmtPingInfo.sensordepth = sensordepth;
      }
      else if (kluge_auvsentrysensordepth) {
        sensordepth = -mrz->pingInfo.ellipsoidHeightReRefPoint_m;
        mrz->pingInfo.txTransducerDepth_m = sensordepth;
        xmt->xmtPingInfo.sensordepth = sensordepth;
      }
      else {
        sensordepth = mrz->pingInfo.txTransducerDepth_m;
        xmt->xmtPingInfo.sensordepth = sensordepth;
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

      /* Get the transducer array mounting angles. The orientation of the platform is
          known at this point, so these are the mounting offsets of the target sensor
          relative to the platform, with the TX array first and the RX array second. */
      mb_3D_orientation tx_align = {0.0, 0.0, 0.0};
      mb_3D_orientation tx_orientation;
      double tx_steer;
      mb_3D_orientation rx_align = {0.0, 0.0, 0.0};
      mb_3D_orientation rx_orientation;
      double rx_steer;
      int tx_sign = 1.0;
      int rx_sign = 1.0;
      if (platform != NULL && pars->target_sensor >= 0 && pars->target_sensor < platform->num_sensors
          && platform->sensors[pars->target_sensor].num_offsets > 0) {
        const struct mb_sensor_struct *sensor_target = &platform->sensors[pars->target_sensor];
        tx_align.heading = sensor_target->offsets[0].attitude_offset_heading;
        tx_align.roll = sensor_target->offsets[0].attitude_offset_roll;
        tx_align.pitch = sensor_target->offsets[0].attitude_offset_pitch;
        const int rx_offset = (sensor_target->num_offsets > 1) ? 1 : 0;
        rx_align.heading = sensor_target->offsets[rx_offset].attitude_offset_heading;
        rx_align.roll = sensor_target->offsets[rx_offset].attitude_offset_roll;
        rx_align.pitch = sensor_target->offsets[rx_offset].attitude_offset_pitch;

        // handle reverse mounting of transmit array */
        if (tx_align.heading > 100.0 || tx_align.heading < -100.0) {
          tx_align.heading -= 180.0;
          if (tx_align.heading < 0.0)
            tx_align.heading += 360.0;
          tx_sign = -1.0;
        }

        // handle reverse mounting of receive array */
        if (rx_align.heading > 100.0 || rx_align.heading < -100.0) {
          rx_align.heading -= 180.0;
          if (rx_align.heading < 0.0)
            rx_align.heading += 360.0;
          rx_sign = -1.0;
        }

        /* The MRZ navigation and the soundings x/y_reRefPoint_m are relative to the vessel
            reference point (the platform origin), but the navigation reported by MB-System is
            that of the sonar, and so are bathymetry raytraced from the XMT travel times and
            angles. The alongtrack position of a sounding is set by the transmit array and the
            acrosstrack position by the receive array, so the sonar reference point is taken to
            be at the alongtrack position of the transmit array and the acrosstrack position of
            the receive array. Save its horizontal displacement from the vessel reference point,
            in the heading-aligned surface coordinate system, as the lever arm. */
        double lever_x;
        double lever_y;
        double lever_z;
        int lever_error = MB_ERROR_NO_ERROR;
        if (mb_platform_lever_origin(verbose, (void *)platform, pars->target_sensor, 0, 0.0, roll, pitch,
                                     &lever_x, &lever_y, &lever_z, &lever_error) == MB_SUCCESS)
          xmt->xmtPingInfo.lever_alongtrack = lever_y;
        if (mb_platform_lever_origin(verbose, (void *)platform, pars->target_sensor,
                                     (sensor_target->num_offsets > 1) ? 1 : 0, 0.0, roll, pitch,
                                     &lever_x, &lever_y, &lever_z, &lever_error) == MB_SUCCESS)
          xmt->xmtPingInfo.lever_acrosstrack = lever_x;

        /* The #XMT navigation is the position of the sonar reference point, which is what
            MB-System reports as the navigation: the vessel reference point position of the
            #MRZ datagram displaced by the lever arm */
        double dlon;
        double dlat;
        mb_platform_displacement_to_lonlat(mrz->pingInfo.latitude_deg, heading, xmt->xmtPingInfo.lever_acrosstrack,
                                           xmt->xmtPingInfo.lever_alongtrack, &dlon, &dlat);
        xmt->xmtPingInfo.longitude = mrz->pingInfo.longitude_deg + dlon;
        xmt->xmtPingInfo.latitude = mrz->pingInfo.latitude_deg + dlat;
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
        double reference_heading;
        double beamAzimuth;
        double beamDepression;
        double txroll, txpitch, txheading;
        double beamroll, beampitch, beamheading;
        // double theta, phi;
        // double mtodeglon, mtodeglat;
        // double headingx, headingy;

        double ttime = mrz->sounding[i].twoWayTravelTime_sec
                                  + mrz->sounding[i].twoWayTravelTimeCorrection_sec;

        /* Each sounding's transmit sector fires at its own time relative to the ping
            header time (sectorTransmitDelay_sec), and its bottom return arrives
            ttime later still. Both the transmit and receive orientations used
            below must be interpolated at these sounding-specific times, not at
            the ping header time time_d, so that multi-sector (including dual
            swath) pings get the correct attitude per sector/beam. */
        const double sector_transmit_delay =
            mrz->sectorInfo[mrz->sounding[i].txSectorNumb].sectorTransmitDelay_sec;
        const double transmit_time_d = time_d + sector_transmit_delay;
        const double receive_time_delay = sector_transmit_delay + ttime;
        const double receive_time_d = time_d + receive_time_delay;

        /* Get the roll, pitch and heading at the time this sounding's sector actually
            transmitted and at the time its bottom return was received. The interpolated
            sensor values are converted to the platform frame when a platform model is
            defined; the mounting offsets of the transducer arrays are then applied by
            mb_beaudoin() as tx_align and rx_align. */
        double s_roll = sensor_roll;
        double s_pitch = sensor_pitch;
        double s_heading = sensor_heading;
        if (pars->n_attitude > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1,
                                pars->attitude_roll - 1, pars->n_attitude,
                                transmit_time_d, &s_roll, &jattitude, &interp_error);
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1,
                                pars->attitude_pitch - 1, pars->n_attitude,
                                transmit_time_d, &s_pitch, &jattitude, &interp_error);
        }
        if (pars->n_heading > 0) {
          /* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                   pars->n_heading, transmit_time_d, &s_heading,
                                                   &jheading, &interp_error);
        }
        mbsys_kmbes_platform_orientation(verbose, platform, heading_from_array, s_heading, s_roll, s_pitch,
                                         &txheading, &txroll, &txpitch);

        s_roll = sensor_roll;
        s_pitch = sensor_pitch;
        s_heading = sensor_heading;
        if (pars->n_attitude > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1,
                                pars->attitude_roll - 1, pars->n_attitude,
                                receive_time_d, &s_roll, &jattitude, &interp_error);
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1,
                                pars->attitude_pitch - 1, pars->n_attitude,
                                receive_time_d, &s_pitch, &jattitude, &interp_error);
        }
        if (pars->n_heading > 0) {
          /* interp_status = */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                   pars->n_heading, receive_time_d, &s_heading,
                                                   &jheading, &interp_error);
        }
        mbsys_kmbes_platform_orientation(verbose, platform, heading_from_array, s_heading, s_roll, s_pitch,
                                         &beamheading, &beamroll, &beampitch);

        /* change the sound speed recorded for the current ping and
         * then use it to alter the beam angles and recalculate the Bathymetry */
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
        tx_steer = tx_sign * mrz->sectorInfo[mrz->sounding[i].txSectorNumb].tiltAngleReTx_deg;
        tx_orientation.roll = txroll;
        tx_orientation.pitch = txpitch;
        tx_orientation.heading = txheading;
        rx_steer = rx_sign * (mrz->sounding[i].beamAngleReRx_deg - mrz->sounding[i].beamAngleCorrection_deg);
        rx_orientation.roll = beamroll;
        rx_orientation.pitch = beampitch;
        rx_orientation.heading = beamheading;
        reference_heading = txheading;
//fprintf(stderr, "%s:%d:%s: beam %d: beamAngleReRx_deg:%f beamAngleCorrection_deg:%f roll:%f %f rx_steer:%f  tx_steer:%f tx att: %f %f %f\n",
//__FILE__, __LINE__, __func__, i, mrz->sounding[i].beamAngleReRx_deg, mrz->sounding[i].beamAngleCorrection_deg,
//roll, beamroll, rx_steer, tx_steer, roll, pitch, heading);

        status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                             reference_heading, &beamAzimuth, &beamDepression, error);
        const double theta = 90.0 - beamDepression;
        double phi = 90.0 - beamAzimuth;
        if (phi < 0.0)
          phi += 360.0;

        /* calculate Bathymetry */
        // const double rr = 0.5 * soundspeed * ttime;
        // const double xx = rr * sin(DTR * theta);
        // const double zz = rr * cos(DTR * theta);
        // mrz->sounding[i].y_reRefPoint_m = xx * cos(DTR * phi);
        // mrz->sounding[i].x_reRefPoint_m = xx * sin(DTR * phi);
        // mrz->sounding[i].z_reRefPoint_m = zz;
        double receive_sensordepth = sensordepth;
        double receive_heave = heave;
        if (pars->n_sensordepth > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                          pars->n_sensordepth, receive_time_d, &receive_sensordepth, &jsensordepth, &interp_error);
          receive_sensordepth += sensordepth_lever;
        }
        else if (kluge_auvsentrysensordepth) {
          receive_sensordepth = -mrz->pingInfo.ellipsoidHeightReRefPoint_m;
        }
        else {
          receive_sensordepth = mrz->pingInfo.txTransducerDepth_m;
        }
        if (pars->n_attitude > 0) {
          /* interp_status = */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                           receive_time_d, &receive_heave, &jattitude, &interp_error);
        }

        xmt->xmtSounding[i].soundingIndex = mrz->sounding[i].soundingIndex;
        xmt->xmtSounding[i].padding0 = 0;
        xmt->xmtSounding[i].twtt = ttime;
        xmt->xmtSounding[i].angle_vertical = theta;
        xmt->xmtSounding[i].angle_azimuthal = phi;
        xmt->xmtSounding[i].beam_heave = (receive_sensordepth - sensordepth) + (receive_heave - heave);
        /* The alongtrack position of a sounding is fixed by its transmit sector, so the only
            vessel advance to include is that between the ping reference time and the time this
            sector transmitted - not the advance during the two-way travel time, which is
            already contained in Kongsberg's solution and would add speed * ttime (tens of
            meters at survey speeds) of spurious alongtrack offset */
        xmt->xmtSounding[i].alongtrack_offset = sector_transmit_delay * xmt->xmtPingInfo.speed;
//fprintf(stderr, "receive_time_delay:%f xmt->xmtPingInfo.speed:%f\n", receive_time_delay, xmt->xmtPingInfo.speed);
//fprintf(stderr, "preprocess: imrz:%d beam %d %d: tt:%f  angle_xtrk:%f  angle_ltrk:%f  angle_null:%f  depth_off:%f  ltrk_off:%f\n",
//imrz, i, xmt->xmtSounding[i].soundingIndex, xmt->xmtSounding[i].twtt, xmt->xmtSounding[i].angle_vertical, xmt->xmtSounding[i].angle_azimuthal, 
//0.0, xmt->xmtSounding[i].beam_heave, xmt->xmtSounding[i].alongtrack_offset);

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
/* Kongsberg's per-sounding x_reRefPoint_m/y_reRefPoint_m (ship-frame along/across-track
    offsets) are relative to the navigation and heading logged for that sounding's own
    MRZ sub-ping (pingInfo.longitude_deg/latitude_deg/headingVessel_deg), which in
    dual-swath (or any other multi-sub-ping-per-cycle) kmall data can differ measurably
    from the single reference sub-ping (imrz==0) whose navigation/heading is the only
    one reported for the whole merged MB-System ping. This reprojects a beam's
    along/across-track offsets from the frame of its own sub-ping's navigation/heading
    into the equivalent offsets in the reference sub-ping's frame - via absolute lon/lat
    - using the same rotation and meters-to-degrees convention as mb_extract_lonlat()
    and mb_read() (src/mbio/mb_access.c, src/mbio/mb_read.c) - so that every beam ends
    up correctly georeferenced once the caller applies the single reported reference
    navigation/heading to bathacrosstrack/bathalongtrack. */
static void mbsys_kmbes_reproject_beam_to_reference(double navlon_sounding, double navlat_sounding, double heading_sounding,
                                                     double navlon_ref, double navlat_ref, double heading_ref,
                                                     double acrosstrack_sounding, double alongtrack_sounding,
                                                     double *acrosstrack_ref, double *alongtrack_ref) {
  double mtodeglon_sounding, mtodeglat_sounding;
  mb_coor_scale(0, navlat_sounding, &mtodeglon_sounding, &mtodeglat_sounding);
  const double headingx_sounding = sin(DTR * heading_sounding);
  const double headingy_sounding = cos(DTR * heading_sounding);
  const double lon = navlon_sounding + headingy_sounding * mtodeglon_sounding * acrosstrack_sounding
                                      + headingx_sounding * mtodeglon_sounding * alongtrack_sounding;
  const double lat = navlat_sounding - headingx_sounding * mtodeglat_sounding * acrosstrack_sounding
                                      + headingy_sounding * mtodeglat_sounding * alongtrack_sounding;

  double mtodeglon_ref, mtodeglat_ref;
  mb_coor_scale(0, navlat_ref, &mtodeglon_ref, &mtodeglat_ref);
  const double dE = (lon - navlon_ref) / mtodeglon_ref;
  const double dN = (lat - navlat_ref) / mtodeglat_ref;
  const double headingx_ref = sin(DTR * heading_ref);
  const double headingy_ref = cos(DTR * heading_ref);
  *acrosstrack_ref = headingy_ref * dE - headingx_ref * dN;
  *alongtrack_ref = headingx_ref * dE + headingy_ref * dN;
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
  // struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  // struct mbsys_kmbes_shi *shi = (struct mbsys_kmbes_shi *)&store->shi;
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

      /* Beams from any sub-ping other than the reference (imrz==0) were logged with
          their own navigation/heading (dual-swath sub-pings can be a fraction of a
          second, and a measurable vessel motion, apart) - reproject them into the
          reference sub-ping's frame so they end up correctly positioned once *navlon,
          *navlat, *heading (always the reference sub-ping's values) are applied. */
      /* The soundings of each sub-ping are relative to the vessel reference point, whereas the
          navigation reported is that of the sonar reference point, which is displaced from it by
          the sub-ping's lever arm. Remove the lever arm to make the soundings relative to the
          sub-ping's sonar reference point, which is the sub-ping's #XMT navigation. */
      const struct mbsys_kmbes_xmt *xmt_i = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];
      const double lever_across = xmt_i->xmtPingInfo.lever_acrosstrack;
      const double lever_along = xmt_i->xmtPingInfo.lever_alongtrack;
      const bool reproject_beams = (imrz != 0)
              && (xmt_i->xmtPingInfo.longitude != *navlon || xmt_i->xmtPingInfo.latitude != *navlat
                  || mrz->pingInfo.headingVessel_deg != *heading);

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        beamflag[numSoundings] = mrz->sounding[i].beamflag;
        if (!mb_beam_check_flag_null(mrz->sounding[i].beamflag) && mrz->sounding[i].twoWayTravelTime_sec > 0.0) {
          bath[numSoundings] = mrz->sounding[i].z_reRefPoint_m
                                - mrz->pingInfo.z_waterLevelReRefPoint_m;
          const double across_sonar = mrz->sounding[i].y_reRefPoint_m - lever_across;
          const double along_sonar = mrz->sounding[i].x_reRefPoint_m - lever_along;
          if (reproject_beams) {
            mbsys_kmbes_reproject_beam_to_reference(xmt_i->xmtPingInfo.longitude, xmt_i->xmtPingInfo.latitude,
                                                     mrz->pingInfo.headingVessel_deg, *navlon, *navlat, *heading,
                                                     across_sonar, along_sonar,
                                                     &bathacrosstrack[numSoundings], &bathalongtrack[numSoundings]);
          } else {
            bathacrosstrack[numSoundings] = across_sonar;
            bathalongtrack[numSoundings] = along_sonar;
          }
          amp[numSoundings] = mrz->sounding[i].reflectivity1_dB;
        } else {
          bath[numSoundings] = 0.0;
          bathacrosstrack[numSoundings] = 0.0;
          bathalongtrack[numSoundings] = 0.0;
          amp[numSoundings] = 0.0;
        }
//fprintf(stderr,"\n%s:%d:%s: %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d txd:%f wlr:%f ehr:%f zreref:%f bath:%f flag:%d %d amp:%f\n",
//__FILE__, __LINE__, __func__,
//time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
//mrz->pingInfo.txTransducerDepth_m, mrz->pingInfo.z_waterLevelReRefPoint_m,
//mrz->pingInfo.ellipsoidHeightReRefPoint_m,
//mrz->sounding[i].z_reRefPoint_m, bath[numSoundings],
//mrz->sounding[i].beamflag_enabled, mrz->sounding[i].beamflag, mrz->sounding[i].reflectivity1_dB);
        numSoundings++;
      }
    }
    *nbath = numSoundings;
    *namp = numSoundings;
		*nss = MIN(xms->pixels_ss, MBSYS_KMBES_MAX_PIXELS);
		store->num_pixels = *nss;
		double pixel_size = xms->pixel_size;
		int pixels_per_swath = MBSYS_KMBES_MAX_PIXELS / xms->num_swaths;

		for (int iswath = 0; iswath < xms->num_swaths; iswath++) {
				int pixel_offset = iswath * pixels_per_swath;
				int center_pixel = pixel_offset + pixels_per_swath / 2;

				/* the pseudosidescan pixel positions are relative to the vessel reference point, so
				    make them relative to the sonar reference point by removing the lever arm */
				const struct mbsys_kmbes_xmt *xmt_swath =
						(struct mbsys_kmbes_xmt *)&store->xmt[MIN(iswath, MAX(store->n_mrz_read - 1, 0))];
				const double ss_lever_across = xmt_swath->xmtPingInfo.lever_acrosstrack;
				const double ss_lever_along = xmt_swath->xmtPingInfo.lever_alongtrack;
				for (int i = pixel_offset; i < pixel_offset + pixels_per_swath; i++) {
						ssacrosstrack[i] = pixel_size * (i - center_pixel) - ss_lever_across;
						if (xms->ss[i] == MBSYS_KMBES_INVALID_SS
								|| (xms->ss[i] == MBSYS_KMBES_INVALID_AMP && xms->ss_alongtrack[i] == 0.0)) {
								ss[i] = MB_SIDESCAN_NULL;
								ssalongtrack[i] = 0.0;
						} else {
								ss[i] = xms->ss[i];
								ssalongtrack[i] = xms->ss_alongtrack[i] - ss_lever_along;
						}
				}
		}
    if (verbose >= 2) {
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
  else if (*kind == MB_DATA_NAV3) {
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
  else if (*kind == MB_DATA_SENSORDEPTH) {
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
    if (strlen(xmc->comment) > 0) {
      memset((void *)comment, 0, MB_COMMENT_MAXLINE);
		  strncpy(comment, xmc->comment, MB_COMMENT_MAXLINE - 1);
    }
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
  if (verbose >= 2 && (*kind == MB_DATA_DATA || *kind == MB_DATA_NAV 
  		|| *kind == MB_DATA_NAV1 || *kind == MB_DATA_NAV2 || *kind == MB_DATA_NAV3)) {
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
  if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV || kind == MB_DATA_NAV1 
  		|| kind == MB_DATA_NAV2 || kind == MB_DATA_NAV3)) {
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
  // struct mbsys_kmbes_sde *sde = (struct mbsys_kmbes_sde *)&store->sde;
  struct mbsys_kmbes_sha *sha = (struct mbsys_kmbes_sha *)&store->sha;
  struct mbsys_kmbes_cpo *cpo = (struct mbsys_kmbes_cpo *)&store->cpo;
  struct mbsys_kmbes_xmc *xmc = (struct mbsys_kmbes_xmc *)&store->xmc;
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[0];
  struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;

  /* set data kind */
  store->kind = kind;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    bool change_timestamp = false;
    unsigned int time_sec = (unsigned int)floor(time_d);
    unsigned int time_nanosec = (unsigned int)((time_d - (double)time_sec) * 1.0e9);
    if (store->time_d != time_d) {
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = time_d;
      change_timestamp = true;
      xmt->header.time_sec = time_sec;
      xmt->header.time_nanosec = time_nanosec;
      xms->header.time_sec = time_sec;
      xms->header.time_nanosec = time_nanosec;
    }

    /* loop over all sub-pings */
    /* read distance and depth values into storage arrays */
    int numSoundings = 0;
    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

      if (change_timestamp) {
        mrz->header.time_sec = time_sec;
        mrz->header.time_nanosec = time_nanosec;
      }

      /* The navigation passed in is that of the sonar reference point, and the distances are
          relative to it, as returned by mbsys_kmbes_extract(). The #MRZ navigation and soundings
          are relative to the vessel reference point, which is displaced from the sonar reference
          point by the lever arm held in the #XMT datagram. The #XMT datagram holds the sonar
          navigation. */
      const struct mbsys_kmbes_xmt *xmt_i = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];
      const double lever_across = xmt_i->xmtPingInfo.lever_acrosstrack;
      const double lever_along = xmt_i->xmtPingInfo.lever_alongtrack;

      /* Only the reference sub-ping (imrz==0) is represented by the single
          navlon/navlat/heading passed into this function - other sub-pings
          (dual swath etc.) keep their own distinct logged navigation/heading,
          the same way mbsys_kmbes_extract() treats them (see
          mbsys_kmbes_reproject_beam_to_reference() above). So bathacrosstrack/
          bathalongtrack, which extract() reprojected out of each sub-ping's own
          frame into the reference frame, must be reprojected back into that
          sub-ping's own (preserved) frame here - the same function run with the
          reference and sub-ping frames swapped is its own exact inverse -
          rather than overwriting the sub-ping's pingInfo with the reference
          ping's values and losing the distinction entirely. */
      const bool reproject_beams = (imrz != 0)
              && (xmt_i->xmtPingInfo.longitude != navlon || xmt_i->xmtPingInfo.latitude != navlat
                  || mrz->pingInfo.headingVessel_deg != heading);
      if (imrz == 0) {
        /* leave the vessel reference point navigation as logged if the navigation and heading
            are unchanged, so that data passed through unmodified are not altered at all */
        if (xmt->xmtPingInfo.longitude != navlon || xmt->xmtPingInfo.latitude != navlat
            || mrz->pingInfo.headingVessel_deg != heading) {
          double dlon;
          double dlat;
          mb_platform_displacement_to_lonlat(navlat, heading, lever_across, lever_along, &dlon, &dlat);
          mrz->pingInfo.longitude_deg = navlon - dlon;
          mrz->pingInfo.latitude_deg = navlat - dlat;
        }
        mrz->pingInfo.headingVessel_deg = heading;
        xmt->xmtPingInfo.longitude = navlon;
        xmt->xmtPingInfo.latitude = navlat;
        xmt->xmtPingInfo.heading = heading;
        xmt->xmtPingInfo.speed = speed /  3.6;
      }

      for (int i = 0;
            i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections);
            i++) {
        mrz->sounding[i].z_reRefPoint_m = bath[numSoundings]
                                + mrz->pingInfo.z_waterLevelReRefPoint_m;
//        mrz->sounding[i].z_reRefPoint_m = bath[numSoundings]
//                                - mrz->pingInfo.txTransducerDepth_m;
        mrz->sounding[i].beamflag = beamflag[numSoundings];
        if (reproject_beams) {
          double acrosstrack_subping, alongtrack_subping;
          mbsys_kmbes_reproject_beam_to_reference(navlon, navlat, heading,
                                                   xmt_i->xmtPingInfo.longitude, xmt_i->xmtPingInfo.latitude,
                                                   mrz->pingInfo.headingVessel_deg,
                                                   bathacrosstrack[numSoundings], bathalongtrack[numSoundings],
                                                   &acrosstrack_subping, &alongtrack_subping);
          mrz->sounding[i].y_reRefPoint_m = acrosstrack_subping + lever_across;
          mrz->sounding[i].x_reRefPoint_m = alongtrack_subping + lever_along;
        } else {
          mrz->sounding[i].x_reRefPoint_m = bathalongtrack[numSoundings] + lever_along;
          mrz->sounding[i].y_reRefPoint_m = bathacrosstrack[numSoundings] + lever_across;
        }
        mrz->sounding[i].reflectivity1_dB = amp[numSoundings];

        numSoundings++;
      }
    }

    /* insert the sidescan */
    xms->pixels_ss = nss;
    const int pixels_per_swath = MBSYS_KMBES_MAX_PIXELS / MAX(xms->num_swaths, 1);
    for (int i = 0; i < MBSYS_KMBES_MAX_PIXELS; i++) {
      if (ss[i] == MB_SIDESCAN_NULL) {
        xms->ss[i] = MBSYS_KMBES_INVALID_SS;
        xms->ss_alongtrack[i] = 0.0;
      } else {
        /* the stored pseudosidescan alongtrack distances are relative to the vessel reference point */
        const struct mbsys_kmbes_xmt *xmt_swath =
            (struct mbsys_kmbes_xmt *)&store->xmt[MIN(i / pixels_per_swath, MAX(store->n_mrz_read - 1, 0))];
        xms->ss[i] = ss[i];
        xms->ss_alongtrack[i] = ssalongtrack[i] + xmt_swath->xmtPingInfo.lever_alongtrack;
      }
    }

  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV) {
    if (store->time_d != time_d) {
      spo->header.time_sec = (unsigned int)floor(time_d);
      spo->header.time_nanosec = (unsigned int)((time_d - (double)spo->header.time_sec) * 1.0e9);
    }
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
    if (store->time_d != time_d) {
      skm->header.time_sec = (unsigned int)floor(time_d);
      skm->header.time_nanosec = (unsigned int)((time_d - (double)skm->header.time_sec) * 1.0e9);
    }
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
    if (store->time_d != time_d) {
      skm->header.time_sec = (unsigned int)floor(time_d);
      skm->header.time_nanosec = (unsigned int)((time_d - (double)skm->header.time_sec) * 1.0e9);
    }
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
  else if (store->kind == MB_DATA_NAV3) {
    if (store->time_d != time_d) {
      cpo->header.time_sec = (unsigned int)floor(time_d);
      cpo->header.time_nanosec = (unsigned int)((time_d - (double)cpo->header.time_sec) * 1.0e9);
    }
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
  else if (store->kind == MB_DATA_SENSORDEPTH) {

  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_HEADING) {
    if (store->time_d != time_d) {
      sha->header.time_sec = (unsigned int)floor(time_d);
      sha->header.time_nanosec = (unsigned int)((time_d - (double)sha->header.time_sec) * 1.0e9);
    }
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
    memset((void *)xmc->comment, 0, MB_COMMENT_MAXLINE);
    strncpy(xmc->comment, comment, MB_COMMENT_MAXLINE - 1);

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

    /* get altitude using valid depth closest to nadir - the sounding z_reRefPoint_m
        is measured down from the vessel reference point, so the altitude of the
        transducer above the seafloor is the depth below the waterline
        (z_reRefPoint_m - z_waterLevelReRefPoint_m) minus the transducer depth */
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
            *altitudev = mrz->sounding[i].z_reRefPoint_m - mrz->pingInfo.z_waterLevelReRefPoint_m
                          - mrz->pingInfo.txTransducerDepth_m;
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
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

  /* extract data from SPO record */
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
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);
    else
      *heading = spo->sensorData.courseOverGround_deg;
  
    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);
      *heave = 0.0;
    } else {
      *draft = mrz->pingInfo.txTransducerDepth_m;
    }

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    } else {
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
}

  /* extract data from SKM record */
  else if (*kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    if ((skm->infoPart.sensorDataContents & 0x00000001) && skm->infoPart.numSamplesArray > 0) {
	  *navlon = skm->sample[0].KMdefault.longitude_deg;
	  *navlat = skm->sample[0].KMdefault.latitude_deg;
	  *speed = 3.6 * sqrt(skm->sample[0].KMdefault.velNorth
							* skm->sample[0].KMdefault.velNorth
						  + skm->sample[0].KMdefault.velEast
							* skm->sample[0].KMdefault.velEast);
    }
    else  if (mb_io_ptr->nfix > 0) {
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    }
    else {
      *navlon = xmt->xmtPingInfo.longitude;
      *navlat = xmt->xmtPingInfo.latitude;
      *speed = 3.6 * xmt->xmtPingInfo.speed;
    }

    /* get heading */
    if ((skm->infoPart.sensorDataContents & 0x00000004) && skm->infoPart.numSamplesArray > 0) {
      *heading = skm->sample[0].KMdefault.heading_deg;
    }
    else  if (mb_io_ptr->nheading > 0) {
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    }
    else {
      *heading = xmt->xmtPingInfo.heading;
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);
      *heave = 0.0;
    } 
    else {
      *draft = xmt->xmtPingInfo.sensordepth;
    }

    /* get attitude  */
    if ((skm->infoPart.sensorDataContents & 0x00000002) && skm->infoPart.numSamplesArray > 0) {
      *roll = skm->sample[0].KMdefault.roll_deg;
      *pitch = skm->sample[0].KMdefault.pitch_deg;
      *heave = skm->sample[0].KMdefault.heave_m;
    }
    else if (mb_io_ptr->nattitude > 0) {
	  mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
    }
    else {
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
  }

  /* extract data from SKM record */
  else if (*kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    if ((skm->infoPart.sensorDataContents & 0x00000001) && skm->infoPart.numSamplesArray > 0) {
	  *navlon = skm->sample[0].KMdefault.longitude_deg;
	  *navlat = skm->sample[0].KMdefault.latitude_deg;
	  *speed = 3.6 * sqrt(skm->sample[0].KMdefault.velNorth
							* skm->sample[0].KMdefault.velNorth
						  + skm->sample[0].KMdefault.velEast
							* skm->sample[0].KMdefault.velEast);
    }
    else  if (mb_io_ptr->nfix > 0) {
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    }
    else {
      *navlon = xmt->xmtPingInfo.longitude;
      *navlat = xmt->xmtPingInfo.latitude;
      *speed = 3.6 * xmt->xmtPingInfo.speed;
    }

    /* get heading */
    if ((skm->infoPart.sensorDataContents & 0x00000004) && skm->infoPart.numSamplesArray > 0) {
      *heading = skm->sample[0].KMdefault.heading_deg;
    }
    else  if (mb_io_ptr->nheading > 0) {
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    }
    else {
      *heading = xmt->xmtPingInfo.heading;
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);
      *heave = 0.0;
    } 
    else {
      *draft = xmt->xmtPingInfo.sensordepth;
    }

    /* get attitude  */
    if ((skm->infoPart.sensorDataContents & 0x00000002) && skm->infoPart.numSamplesArray > 0) {
      *roll = skm->sample[0].KMdefault.roll_deg;
      *pitch = skm->sample[0].KMdefault.pitch_deg;
      *heave = skm->sample[0].KMdefault.heave_m;
    }
    else if (mb_io_ptr->nattitude > 0) {
	  mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
    }
    else {
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
  }

  /* extract data from CPO record */
  else if (*kind == MB_DATA_NAV3) {
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
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);
    else
      *heading = cpo->sensorData.courseOverGround_deg;

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);
      *heave = 0.0;
    } else {
      *draft = mrz->pingInfo.txTransducerDepth_m;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }
    else {
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
  }

  /* extract data from SDE record */
  else if (*kind == MB_DATA_SENSORDEPTH) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 3.6 * xmt->xmtPingInfo.speed;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    else {
      *navlon = xmt->xmtPingInfo.longitude;
      *navlat = xmt->xmtPingInfo.latitude;
    }

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, *time_d, heading, error);
    else
      *heading = mrz->pingInfo.headingVessel_deg;

    /* get draft  */
    *draft = sde->sensorData.depthUsed_m;

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }
    else {
      *roll = xmt->xmtPingInfo.roll;
      *pitch = xmt->xmtPingInfo.pitch;
      *heave = xmt->xmtPingInfo.heave;
    }

    /* done translating values */
  }

  /* extract data from heading record */
  else if (*kind == MB_DATA_HEADING) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 3.6 * xmt->xmtPingInfo.speed;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    else {
      *navlon = xmt->xmtPingInfo.longitude;
      *navlat = xmt->xmtPingInfo.latitude;
    }

    /* get heading */
    *heading = sha->sensorData[0].headingCorrected_deg;

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, *time_d, draft, error);
      *heave = 0.0;
    } else {
      *draft = mrz->pingInfo.txTransducerDepth_m;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }
    else {
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
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

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

  /* extract all nav and attitude data from record SKM (KM Binary) */
  if (*kind == MB_DATA_NAV1) {
    *n = MIN(skm->infoPart.numSamplesArray, MB_NAV_MAX);

    for (int i = 0; i < *n; i++) {
      /* get time */
      time_d[i] = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
      mb_get_date(verbose, time_d[i], &time_i[7*i]);
  
	  /* get heading */
	  if ((skm->infoPart.sensorDataContents & 0x00000004) && skm->infoPart.numSamplesArray > 0) {
		heading[i] = skm->sample[i].KMdefault.heading_deg;
	  }
	  else  if (mb_io_ptr->nheading > 0) {
		mb_hedint_interp(verbose, mbio_ptr, time_d[i], &heading[i], error);
	  }
	  else {
		heading[i] = xmt->xmtPingInfo.heading;
	  }

	  /* get navigation */
	  if ((skm->infoPart.sensorDataContents & 0x00000001) && skm->infoPart.numSamplesArray > 0) {
		navlon[i] = skm->sample[i].KMdefault.longitude_deg;
		navlat[i] = skm->sample[i].KMdefault.latitude_deg;
		speed[i] = 3.6 * sqrt(skm->sample[i].KMdefault.velNorth
							  * skm->sample[i].KMdefault.velNorth
							+ skm->sample[i].KMdefault.velEast
							  * skm->sample[i].KMdefault.velEast);
	  }
	  else  if (mb_io_ptr->nfix > 0) {
		mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], speed[i], navlon, navlat, speed, error);
	  }
	  else {
		navlon[i] = xmt->xmtPingInfo.longitude;
		navlat[i] = xmt->xmtPingInfo.latitude;
		speed[i] = 3.6 * xmt->xmtPingInfo.speed;
	  }

	  /* get draft  */
	  if (mb_io_ptr->nsensordepth > 0) {
		mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], error);
		heave[i] = 0.0;
	  }
	  else {
		draft[i] = xmt->xmtPingInfo.sensordepth;
	  }

	  /* get attitude  */
	  if ((skm->infoPart.sensorDataContents & 0x00000002) && skm->infoPart.numSamplesArray > 0) {
		roll[i] = skm->sample[i].KMdefault.roll_deg;
		pitch[i] = skm->sample[i].KMdefault.pitch_deg;
		heave[i] = skm->sample[i].KMdefault.heave_m;
	  }
	  else if (mb_io_ptr->nattitude > 0) {
		mb_attint_interp(verbose, mbio_ptr, time_d[i], &heave[i], &roll[i], &pitch[i], error);
	  }
	  else {
		roll[i] = xmt->xmtPingInfo.roll;
		pitch[i] = xmt->xmtPingInfo.pitch;
		heave[i] = xmt->xmtPingInfo.heave;
	  }
    }

    /* done translating values */
  }

  else if (*kind == MB_DATA_NAV2) {
    *n = MIN(skm->infoPart.numSamplesArray, MB_NAV_MAX);

    for (int i = 0; i < *n; i++) {
      /* get time */
      time_d[i] = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
      mb_get_date(verbose, time_d[i], &time_i[7*i]);
  
	  /* get heading */
	  if ((skm->infoPart.sensorDataContents & 0x00000004) && skm->infoPart.numSamplesArray > 0) {
		heading[i] = skm->sample[i].KMdefault.heading_deg;
	  }
	  else  if (mb_io_ptr->nheading > 0) {
		mb_hedint_interp(verbose, mbio_ptr, time_d[i], &heading[i], error);
	  }
	  else {
		heading[i] = xmt->xmtPingInfo.heading;
	  }

	  /* get navigation */
	  if ((skm->infoPart.sensorDataContents & 0x00000001) && skm->infoPart.numSamplesArray > 0) {
		navlon[i] = skm->sample[i].KMdefault.longitude_deg;
		navlat[i] = skm->sample[i].KMdefault.latitude_deg;
		speed[i] = 3.6 * sqrt(skm->sample[i].KMdefault.velNorth
							  * skm->sample[i].KMdefault.velNorth
							+ skm->sample[i].KMdefault.velEast
							  * skm->sample[i].KMdefault.velEast);
	  }
	  else  if (mb_io_ptr->nfix > 0) {
		mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], speed[i], navlon, navlat, speed, error);
	  }
	  else {
		navlon[i] = xmt->xmtPingInfo.longitude;
		navlat[i] = xmt->xmtPingInfo.latitude;
		speed[i] = 3.6 * xmt->xmtPingInfo.speed;
	  }

	  /* get draft  */
	  if (mb_io_ptr->nsensordepth > 0) {
		mb_depint_interp(verbose, mbio_ptr, time_d[i], &draft[i], error);
		heave[i] = 0.0;
	  }
	  else {
		draft[i] = xmt->xmtPingInfo.sensordepth;
	  }

	  /* get attitude  */
	  if ((skm->infoPart.sensorDataContents & 0x00000002) && skm->infoPart.numSamplesArray > 0) {
		roll[i] = skm->sample[i].KMdefault.roll_deg;
		pitch[i] = skm->sample[i].KMdefault.pitch_deg;
		heave[i] = skm->sample[i].KMdefault.heave_m;
	  }
	  else if (mb_io_ptr->nattitude > 0) {
		mb_attint_interp(verbose, mbio_ptr, time_d[i], &heave[i], &roll[i], &pitch[i], error);
	  }
	  else {
		roll[i] = xmt->xmtPingInfo.roll;
		pitch[i] = xmt->xmtPingInfo.pitch;
		heave[i] = xmt->xmtPingInfo.heave;
	  }
    }

    /* done translating values */
  }

  /* extract data from heading record */
  else if (*kind == MB_DATA_HEADING) {
    *n = sha->dataInfo.numSamplesArray;

    const double sha_time_d = sha->header.time_sec + 0.000000001 * sha->header.time_nanosec;
    for (int i=0; i<*n; i++) {

      /* get time */
      time_d[i] = sha_time_d + 0.000000001 * sha->sensorData[i].timeSinceRecStart_nanosec;
      mb_get_date(verbose, time_d[i], &time_i[7*i]);

      /* get speed */
      speed[i] = 3.6 * xmt->xmtPingInfo.speed;

      /* get heading */
      heading[i] = sha->sensorData[i].headingCorrected_deg;

     /* get navigation from buffered time series */
      mb_navint_interp(verbose, mbio_ptr, time_d[i], heading[i], speed[i],
                          &(navlon[i]), &(navlat[i]), &(speed[i]), error);

      // get draft from buffered time series
      if (mb_io_ptr->nsensordepth > 0)
        mb_depint_interp(verbose, mbio_ptr, time_d[i], &(draft[i]), error);
      else
        draft[i] = 0.0;

      /* get roll pitch and heave */
      if (mb_io_ptr->nattitude > 0) {
        mb_attint_interp(verbose, mbio_ptr, time_d[i], &(heave[i]), &(roll[i]), &(pitch[i]), error);
      }
    }

    /* done translating values */
  }

  // All other records have single values so set *n=1 and call the mbsys_kmbes_extract_nav() function
  else {
    *n = 1;
    status = mbsys_kmbes_extract_nav(verbose, mbio_ptr, store_ptr, kind, time_i, time_d,
                                  navlon, navlat, speed, heading, draft, roll,
                                  pitch, heave, error);
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
  struct mbsys_kmbes_xms *xms = (struct mbsys_kmbes_xms *)&store->xms;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    bool change_timestamp = false;
    unsigned int time_sec = (unsigned int)floor(time_d);
    unsigned int time_nanosec = (unsigned int)((time_d - (double)time_sec) * 1.0e9);
    if (store->time_d != time_d) {
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = time_d;
      change_timestamp = true;
      xms->header.time_sec = time_sec;
      xms->header.time_nanosec = time_nanosec;
    }

    /* loop over all sub-pings */
    for(int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[imrz];

      if (change_timestamp) {
        mrz->header.time_sec = time_sec;
        mrz->header.time_nanosec = time_nanosec;
        xmt->header.time_sec = time_sec;
        xmt->header.time_nanosec = time_nanosec;
      }
      /* the navigation passed in is that of the sonar reference point, so the vessel reference
          point navigation of the #MRZ datagram is displaced from it by the lever arm */
      if (xmt->xmtPingInfo.longitude != navlon || xmt->xmtPingInfo.latitude != navlat
          || mrz->pingInfo.headingVessel_deg != heading) {
        double dlon;
        double dlat;
        mb_platform_displacement_to_lonlat(navlat, heading, xmt->xmtPingInfo.lever_acrosstrack,
                                           xmt->xmtPingInfo.lever_alongtrack, &dlon, &dlat);
        mrz->pingInfo.longitude_deg = navlon - dlon;
        mrz->pingInfo.latitude_deg = navlat - dlat;
      }
      mrz->pingInfo.headingVessel_deg = heading;
      xmt->xmtPingInfo.longitude = navlon;
      xmt->xmtPingInfo.latitude = navlat;
      xmt->xmtPingInfo.heading = heading;
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
    if (store->time_d != time_d) {
      spo->header.time_sec = (unsigned int)floor(time_d);
      spo->header.time_nanosec = (unsigned int)((time_d - (double)spo->header.time_sec) * 1.0e9);
    }
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
    if (store->time_d != time_d) {
      skm->header.time_sec = (unsigned int)floor(time_d);
      skm->header.time_nanosec = (unsigned int)((time_d - (double)skm->header.time_sec) * 1.0e9);
    }
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
    if (store->time_d != time_d) {
      skm->header.time_sec = (unsigned int)floor(time_d);
      skm->header.time_nanosec = (unsigned int)((time_d - (double)skm->header.time_sec) * 1.0e9);
    }
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
  else if (store->kind == MB_DATA_NAV3) {
    if (store->time_d != time_d) {
      cpo->header.time_sec = (unsigned int)floor(time_d);
      cpo->header.time_nanosec = (unsigned int)((time_d - (double)cpo->header.time_sec) * 1.0e9);
    }
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
		/* get number of swaths */
		const int num_swaths = (store->n_mrz_read > 0)
													 ? store->mrz[0].cmnPart.swathsPerPing
													 : 1;
		const int pixels_per_swath = MBSYS_KMBES_MAX_PIXELS / num_swaths;
		
		/* initialize per-swath sidescan binning arrays */
		double ss[MBSYS_KMBES_MAX_PIXELS];
		int ss_cnt[MBSYS_KMBES_MAX_PIXELS];
		double ssalongtrack[MBSYS_KMBES_MAX_PIXELS];
		for (int i = 0; i < MBSYS_KMBES_MAX_PIXELS; i++) {
				ss[i] = 0.0;
				ss_cnt[i] = 0;
				ssalongtrack[i] = 0.0;
		}
		
		/* if not set get swath width from sonar settings */
		if (!swath_width_set) {
				*swath_width = MAX(fabs(store->mrz[0].pingInfo.portSectorEdge_deg),
													fabs(store->mrz[0].pingInfo.starbSectorEdge_deg));
		}
		
		/* get median altitude across all sub-pings for pixel size calculation */
		if (!pixel_size_set) {
				int nbathsort = 0;
				double bathsort[MBSYS_KMBES_MAX_PIXELS];
				for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
						struct mbsys_kmbes_mrz *mrz = &store->mrz[imrz];
						for (int i = 0; i < (mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections); i++) {
								if (mb_beam_ok(mrz->sounding[i].beamflag)) {
										bathsort[nbathsort++] = mrz->sounding[i].z_reRefPoint_m;
								}
						}
				}
				qsort((char *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
				double median_altitude = bathsort[nbathsort / 2];
				double pixel_size_calc = 2 * tan(DTR * (*swath_width)) * median_altitude / pixels_per_swath;
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
		
		/* loop over all sub-ping MRZ datagrams, binning into per-swath pixel strips */
		for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
				struct mbsys_kmbes_mrz *mrz = &store->mrz[imrz];
		
				/* determine which strip this swath maps to:
				 * swathAlongPosition 0 = aft = first strip (pixel offset 0)
				 * swathAlongPosition 1 = forward = second strip (pixel offset pixels_per_swath) */
				const int swath_index = mrz->cmnPart.swathAlongPosition;
				const int pixel_offset = swath_index * pixels_per_swath;
				const int center_pixel = pixel_offset + pixels_per_swath / 2;
		
				int nsoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
				int nsamples = 0;
				for (int i = 0; i < nsoundings; i++) {
						if (mb_beam_ok(mrz->sounding[i].beamflag)) {
								int k1 = nsamples;
								int kc = k1 + mrz->sounding[i].SIcentreSample - 1;
								int k2 = k1 + mrz->sounding[i].SInumSamples - 1;
		
								/* dx1, dx2 geometry - same logic as original, unchanged */
								double dx1, dx2;
								if (mrz->sounding[i].y_reRefPoint_m < 0.0) {
										/* port side */
										dx1 = (i > 0)
													? (mrz->sounding[i].y_reRefPoint_m - mrz->sounding[i-1].y_reRefPoint_m)
														/ (mrz->sounding[i].SInumSamples - mrz->sounding[i].SIcentreSample)
													: 0.0;
										dx2 = (i < nsoundings - 1)
													? (mrz->sounding[i+1].y_reRefPoint_m - mrz->sounding[i].y_reRefPoint_m)
														/ mrz->sounding[i].SIcentreSample
													: dx1;
										if (i == 0) dx1 = dx2;
		
										for (int k = k1; k <= k2; k++) {
												double xx = mrz->sounding[i].y_reRefPoint_m
																		- (k < kc ? dx2 : dx1) * (k - kc);
												const int kk = center_pixel + (int)(xx / (*pixel_size));
												if (kk >= pixel_offset && kk < pixel_offset + pixels_per_swath
																&& mrz->SIsample_desidB[k] > -32767) {
														ss[kk] += 0.1 * (float)(mrz->SIsample_desidB[k]);
														ssalongtrack[kk] += mrz->sounding[i].x_reRefPoint_m;
														ss_cnt[kk]++;
												}
										}
								} else {
										/* starboard side */
										dx1 = (i > 0)
													? (mrz->sounding[i].y_reRefPoint_m - mrz->sounding[i-1].y_reRefPoint_m)
														/ mrz->sounding[i].SIcentreSample
													: 0.0;
										dx2 = (i < nsoundings - 1)
													? (mrz->sounding[i+1].y_reRefPoint_m - mrz->sounding[i].y_reRefPoint_m)
														/ (mrz->sounding[i].SInumSamples - mrz->sounding[i].SIcentreSample)
													: dx1;
										if (i == 0) dx1 = dx2;
		
										for (int k = k1; k <= k2; k++) {
												double xx = mrz->sounding[i].y_reRefPoint_m
																		+ (k < kc ? dx1 : dx2) * (k - kc);
												const int kk = center_pixel + (int)(xx / (*pixel_size));
												if (kk >= pixel_offset && kk < pixel_offset + pixels_per_swath
																&& mrz->SIsample_desidB[k] > -32767) {
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
		
		/* average, find bounds, and interpolate — do this per swath strip */
		int first = MBSYS_KMBES_MAX_PIXELS;
		int last = -1;
		for (int k = 0; k < MBSYS_KMBES_MAX_PIXELS; k++) {
				if (ss_cnt[k] > 0) {
						ss[k] /= ss_cnt[k];
						ssalongtrack[k] /= ss_cnt[k];
						first = MIN(first, k);
						last = k;
				} else {
						ss[k] = MB_SIDESCAN_NULL;
				}
		}
		
		/* interpolate within each swath strip only — do not interpolate across the boundary */
		for (int iswath = 0; iswath < num_swaths; iswath++) {
				int strip_start = iswath * pixels_per_swath;
				int strip_end   = strip_start + pixels_per_swath - 1;
		
				/* find first and last valid pixel in this strip */
				int sfirst = strip_end + 1, slast = strip_start - 1;
				for (int k = strip_start; k <= strip_end; k++) {
						if (ss_cnt[k] > 0) { sfirst = MIN(sfirst, k); slast = k; }
				}
		
				int k1 = sfirst, k2 = sfirst;
				for (int k = sfirst + 1; k < slast; k++) {
						if (ss_cnt[k] <= 0) {
								if (k2 <= k) {
										k2 = k + 1;
										while (ss_cnt[k2] <= 0 && k2 < slast) k2++;
								}
								if (k2 - k1 <= pixel_int_use) {
										ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
										ssalongtrack[k] = ssalongtrack[k1]
												+ (ssalongtrack[k2] - ssalongtrack[k1])
												* ((double)(k - k1)) / ((double)(k2 - k1));
								}
						} else {
								k1 = k;
						}
				}
		}
		
		/* insert pseudosidescan into XMS datagram */
		store->num_pixels = MBSYS_KMBES_MAX_PIXELS;
		struct mbsys_kmbes_mrz *mrz0 = &store->mrz[0];
		struct mbsys_kmbes_xms *xms_out = &store->xms;
		xms_out->header = mrz0->header;
		xms_out->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + 8 * MBSYS_KMBES_MAX_PIXELS + 48;
		strncpy((char *)xms_out->header.dgmType, "#XMS", 4);
		xms_out->pingCnt = mrz0->cmnPart.pingCnt;
		xms_out->spare = 0;
		xms_out->pixel_size = *pixel_size;
		xms_out->pixels_ss = MBSYS_KMBES_MAX_PIXELS;
		xms_out->num_swaths = num_swaths;   /* NEW field */
		memset((char *)xms_out->unused, 0, 28);
		for (int k = 0; k < MBSYS_KMBES_MAX_PIXELS; k++) {
			xms_out->ss[k] = ss[k];
			xms_out->ss_alongtrack[k] = ssalongtrack[k];
		}

		if (verbose >= 4) {
			fprintf(stderr, "\ndbg5  Values calculated in MBIO function <%s>\n", __func__);
			fprintf(stderr, "dbg4  xms->header.numBytesDgm:          %d\n", xms_out->header.numBytesDgm);
			fprintf(stderr, "dbg4  xms->header.dgmType:              %c\n", *xms_out->header.dgmType);
			fprintf(stderr, "dbg4  xms->pingCnt:                     %d\n", xms_out->pingCnt);
			fprintf(stderr, "dbg4  xms->spare:                       %d\n", xms_out->spare);
			fprintf(stderr, "dbg4  xms->pixel_size:                  %f\n", xms_out->pixel_size);
			fprintf(stderr, "dbg4  xms->pixels_ss:                   %d\n", xms_out->pixels_ss);
			fprintf(stderr, "dbg4  xms->num_swaths:                  %d\n", xms_out->num_swaths);
			fprintf(stderr, "dbg4  xms->unused:                      ");
			for (int i=0; i< 28; i++) {
				fprintf(stderr, "%x ", xms_out->unused[i]);
			}
			fprintf(stderr, "\n");
			fprintf(stderr, "dbg4  pixel swath xtrack ltrack ss\n");
			for (int k = 0; k < MBSYS_KMBES_MAX_PIXELS; k++) {
				int swath = k / pixels_per_swath;
				int center_pixel = pixels_per_swath * swath + pixels_per_swath / 2;
				double ssacrosstrack = xms_out->pixel_size * (k - center_pixel);
				fprintf(stderr, "dbg4    %5d %d   %.3f %.3f %.6f\n", k, swath, ssacrosstrack, ssalongtrack[k], ss[k]);
			}
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
