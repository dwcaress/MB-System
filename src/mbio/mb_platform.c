/*--------------------------------------------------------------------
 *    The MB-system:  mb_platform.c  11/1/00
 *
 *    Copyright (c) 2015-2023 by
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
 * This source file includes the functions used to read and write
 * platform definition files.
 * The structures used to store platform, sensor, and offset information
 * are defined in mb_io.h
 *
 * Author:  D. W. Caress
 * Date:  June 26, 2015
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "mb_define.h"
#include "mb_format.h"
/* Set define so that mb_sensor_type_id[] amd mb_sensor_type_string[] arrays are
   defined in mb_platform.c (this source file) when included from mb_io.h
   - these will be declared extern elsewhere */
#define MB_NEED_SENSOR_TYPE 1
#include "mb_io.h"
#include "mb_segy.h"
#include "mb_status.h"


const char *mb_platform_type(mb_platform_enum platform) {
    const char *platform_string[] = {
      "Unknown platform type", "Surface vessel", "Tow body", "ROV", "AUV",
      "Aircraft", "Satellite", "Mooring", "Fixed"};
    return platform_string[platform];
}

/*--------------------------------------------------------------------*/
int mb_platform_init(int verbose, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:             %p\n", platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:            %p\n", *platform_ptr);
  }

  /* allocate memory for platform descriptor structure if needed */
  int status = MB_SUCCESS;
  if (*platform_ptr == NULL) {
    status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mb_platform_struct), (void **)platform_ptr, error);
    if (status == MB_SUCCESS) {
      memset(*platform_ptr, 0, sizeof(struct mb_platform_struct));
    }
  }

  /* initialize structure if platform structure is allocated */
  if (*platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)*platform_ptr;

    /* set values */
    platform->type = MB_PLATFORM_NONE;
    memset(platform->name, 0, sizeof(mb_longname));
    memset(platform->organization, 0, sizeof(mb_longname));
    memset(platform->documentation_url, 0, sizeof(mb_longname));
    memset(platform->start_time_i, 0, 7 * sizeof(int));
    memset(platform->end_time_i, 0, 7 * sizeof(int));
    platform->source_bathymetry = -1;
    platform->source_bathymetry1 = -1;
    platform->source_bathymetry2 = -1;
    platform->source_bathymetry3 = -1;
    platform->source_backscatter = -1;
    platform->source_backscatter1 = -1;
    platform->source_backscatter2 = -1;
    platform->source_backscatter3 = -1;
    platform->source_subbottom = -1;
    platform->source_subbottom1 = -1;
    platform->source_subbottom2 = -1;
    platform->source_subbottom3 = -1;
    platform->source_camera = -1;
    platform->source_camera1 = -1;
    platform->source_camera2 = -1;
    platform->source_camera3 = -1;
    platform->source_position = -1;
    platform->source_position1 = -1;
    platform->source_position2 = -1;
    platform->source_position3 = -1;
    platform->source_depth = -1;
    platform->source_depth1 = -1;
    platform->source_depth2 = -1;
    platform->source_depth3 = -1;
    platform->source_heading = -1;
    platform->source_heading1 = -1;
    platform->source_heading2 = -1;
    platform->source_heading3 = -1;
    platform->source_rollpitch = -1;
    platform->source_rollpitch1 = -1;
    platform->source_rollpitch2 = -1;
    platform->source_rollpitch3 = -1;
    platform->source_heave = -1;
    platform->source_heave1 = -1;
    platform->source_heave2 = -1;
    platform->source_heave3 = -1;
    platform->num_sensors = 0;
    platform->num_sensors_alloc = 0;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       *platform_ptr:             %p\n", *platform_ptr);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_setinfo(int verbose, void *platform_ptr, int type, char *name, char *organization, char *documentation_url,
                        double start_time_d, double end_time_d, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:             %p\n", platform_ptr);
    fprintf(stderr, "dbg2       type:                 %d\n", type);
    fprintf(stderr, "dbg2       name:                 %s\n", name);
    fprintf(stderr, "dbg2       organization:           %s\n", organization);
    fprintf(stderr, "dbg2       documentation_url:       %s\n", documentation_url);
    fprintf(stderr, "dbg2       start_time_d:           %f\n", start_time_d);
    fprintf(stderr, "dbg2       end_time_d:               %f\n", end_time_d);
  }

  int status = MB_SUCCESS;

  /* proceed if platform structure is allocated */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* set values */
    platform->type = type;
    if (name != NULL)
      strcpy(platform->name, name);
    else
      memset(platform->name, 0, sizeof(mb_longname));
    if (organization != NULL)
      strcpy(platform->organization, organization);
    else
      memset(platform->name, 0, sizeof(mb_longname));
    if (documentation_url != NULL)
      strcpy(platform->documentation_url, documentation_url);
    else
      memset(platform->name, 0, sizeof(mb_longname));
    platform->start_time_d = start_time_d;
    if (platform->start_time_d > 100.0)
      mb_get_date(verbose, platform->start_time_d, platform->start_time_i);
    else
      memset(platform->start_time_i, 0, 7 * sizeof(int));
    platform->end_time_d = end_time_d;
    if (platform->end_time_d > 100.0)
      mb_get_date(verbose, platform->end_time_d, platform->end_time_i);
    else
      memset(platform->end_time_i, 0, 7 * sizeof(int));

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:           %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mb_platform_add_sensor(int verbose, void *platform_ptr, int type, mb_longname model, mb_longname manufacturer,
                           mb_longname serialnumber, int capability1, int capability2, int num_offsets, int num_time_latency,
                           int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:         %p\n", platform_ptr);
    fprintf(stderr, "dbg2       type:                 %d\n", type);
    fprintf(stderr, "dbg2       model:                %s\n", model);
    fprintf(stderr, "dbg2       manufacturer:         %s\n", manufacturer);
    fprintf(stderr, "dbg2       serialnumber:         %s\n", serialnumber);
    fprintf(stderr, "dbg2       capability1:          %d\n", capability1);
    fprintf(stderr, "dbg2       capability2:          %d\n", capability2);
    fprintf(stderr, "dbg2       num_offsets:          %d\n", num_offsets);
    fprintf(stderr, "dbg2       num_time_latency:     %d\n", num_time_latency);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* allocate memory for sensor if needed */
    platform->num_sensors++;
    if (platform->num_sensors > platform->num_sensors_alloc) {
      size_t size = platform->num_sensors * sizeof(struct mb_sensor_struct);
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&platform->sensors, error);
      if (status == MB_SUCCESS) {
        memset(&platform->sensors[platform->num_sensors_alloc], 0,
               (platform->num_sensors - platform->num_sensors_alloc) * sizeof(struct mb_sensor_struct));
        platform->num_sensors_alloc = platform->num_sensors;
      }
      else {
        char *message = NULL;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating sensor structures:\n%s\n", message);
        fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
        exit(*error);
      }
    }

    /* insert values */
    int isensor = platform->num_sensors - 1;
    struct mb_sensor_struct *sensor = &platform->sensors[isensor];
    sensor->type = type;
    if (model != NULL)
      strcpy(sensor->model, model);
    else
      memset(sensor->model, 0, sizeof(mb_longname));
    if (manufacturer != NULL)
      strcpy(sensor->manufacturer, manufacturer);
    else
      memset(sensor->manufacturer, 0, sizeof(mb_longname));
    if (serialnumber != NULL)
      strcpy(sensor->serialnumber, serialnumber);
    else
      memset(sensor->serialnumber, 0, sizeof(mb_longname));
    sensor->capability1 = capability1;
    sensor->capability2 = capability2;
    sensor->num_offsets = num_offsets;

    /* allocate memory for offsets if needed */
    isensor = platform->num_sensors - 1;
    sensor = &platform->sensors[isensor];
    sensor->num_offsets = num_offsets;
    if (sensor->num_offsets > sensor->num_offsets_alloc) {
      size_t size = sensor->num_offsets * sizeof(struct mb_sensor_offset_struct);
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->offsets, error);
      if (status == MB_SUCCESS) {
        memset(sensor->offsets, 0, size);
        sensor->num_offsets_alloc = sensor->num_offsets;
      }
      else {
        char *message = NULL;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
        fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
        exit(*error);
      }
    }

    /* allocate memory for time latency model if needed */
    if (num_time_latency > 0) {
      size_t size = num_time_latency * sizeof(double);
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->time_latency_time_d, error);
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->time_latency_value, error);
      if (status == MB_SUCCESS) {
        memset(sensor->time_latency_time_d, 0, size);
        memset(sensor->time_latency_value, 0, size);
        sensor->num_time_latency_alloc = num_time_latency;
      }
      else {
        char *message = NULL;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
        fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
        exit(*error);
      }
    }

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:  %p\n", platform_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_set_sensor_offset(int verbose, void *platform_ptr, int isensor, int ioffset, int position_offset_mode,
                                  double position_offset_x, double position_offset_y, double position_offset_z,
                                  int attitude_offset_mode, double attitude_offset_heading, double attitude_offset_roll,
                                  double attitude_offset_pitch, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                     %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:                %p\n", platform_ptr);
    fprintf(stderr, "dbg2       isensor:                     %d\n", isensor);
    fprintf(stderr, "dbg2       ioffset:                     %d\n", ioffset);
    fprintf(stderr, "dbg2       position_offset_mode:        %d\n", position_offset_mode);
    fprintf(stderr, "dbg2       position_offset_x:           %f\n", position_offset_x);
    fprintf(stderr, "dbg2       position_offset_y:          %f\n", position_offset_y);
    fprintf(stderr, "dbg2       position_offset_z:          %f\n", position_offset_z);
    fprintf(stderr, "dbg2       attitude_offset_mode:        %d\n", attitude_offset_mode);
    fprintf(stderr, "dbg2       attitude_offset_heading:     %f\n", attitude_offset_heading);
    fprintf(stderr, "dbg2       attitude_offset_roll:        %f\n", attitude_offset_roll);
    fprintf(stderr, "dbg2       attitude_offset_pitch:       %f\n", attitude_offset_pitch);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform and sensor structures */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
    struct mb_sensor_struct *sensor = &platform->sensors[isensor];

    /* allocate memory for offsets if needed */
    if (ioffset > sensor->num_offsets - 1)
      sensor->num_offsets = ioffset + 1;
    if (sensor->num_offsets > sensor->num_offsets_alloc) {
      size_t size = sensor->num_offsets * sizeof(struct mb_sensor_offset_struct);
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->offsets, error);
      if (status == MB_SUCCESS) {
        memset(sensor->offsets, 0, size);
        sensor->num_offsets_alloc = sensor->num_offsets;
      }
      else {
        char *message = NULL;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
        fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
        exit(*error);
      }
    }

    /* get offset structure */
    struct mb_sensor_offset_struct *offset = &sensor->offsets[ioffset];

    /* set offset values */
    offset->position_offset_mode = position_offset_mode;
    offset->position_offset_x = position_offset_x;
    offset->position_offset_y = position_offset_y;
    offset->position_offset_z = position_offset_z;
    offset->attitude_offset_mode = attitude_offset_mode;
    offset->attitude_offset_heading = attitude_offset_heading;
    offset->attitude_offset_roll = attitude_offset_roll;
    offset->attitude_offset_pitch = attitude_offset_pitch;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:  %p\n", platform_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_set_sensor_timelatency(int verbose, void *platform_ptr, int isensor, int time_latency_mode,
                                       double time_latency_static, int num_time_latency, double *time_latency_time_d,
                                       double *time_latency_value, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                 %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:            %p\n", platform_ptr);
    fprintf(stderr, "dbg2       isensor:                 %d\n", isensor);
    fprintf(stderr, "dbg2       time_latency_mode:      %d\n", time_latency_mode);
    fprintf(stderr, "dbg2       time_latency_static:      %f\n", time_latency_static);
    fprintf(stderr, "dbg2       num_time_latency:        %d\n", num_time_latency);
    for (int k = 0; k < num_time_latency; k++) {
      fprintf(stderr, "dbg2       time_latency[%2d]:       %16.6f %8.6f\n", k, time_latency_time_d[k],
              time_latency_value[k]);
    }
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform and sensor structures */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
    struct mb_sensor_struct *sensor = &platform->sensors[isensor];

    /* allocate memory for time latency model if needed */
    if (num_time_latency > 0) {
      const size_t size = num_time_latency * sizeof(double);
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->time_latency_time_d, error);
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&sensor->time_latency_value, error);
      if (status == MB_SUCCESS) {
        memset(sensor->time_latency_time_d, 0, size);
        memset(sensor->time_latency_value, 0, size);
        sensor->num_time_latency_alloc = num_time_latency;
      }
      else {
        char *message = NULL;
        mb_error(verbose, *error, &message);
        fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
        fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
        exit(*error);
      }
    }

    /* set time latency values */
    sensor->time_latency_mode = time_latency_mode;
    sensor->time_latency_static = time_latency_static;
    sensor->num_time_latency = num_time_latency;
    for (int k = 0; k < sensor->num_time_latency; k++) {
      sensor->time_latency_time_d[k] = time_latency_time_d[k];
      sensor->time_latency_value[k] = time_latency_value[k];
    }

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_set_source_sensor(int verbose, void *platform_ptr, int source_type, int sensor, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:             %p\n", platform_ptr);
    fprintf(stderr, "dbg2       source_type:             %d\n", source_type);
    fprintf(stderr, "dbg2       sensor:                 %d\n", sensor);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* set source sensor for bathymetry1 data */
    if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY)
      platform->source_bathymetry = sensor;
    if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY1)
      platform->source_bathymetry1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY2)
      platform->source_bathymetry2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY3)
      platform->source_bathymetry3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER)
      platform->source_backscatter = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER1)
      platform->source_backscatter1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER2)
      platform->source_backscatter2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER3)
      platform->source_backscatter3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM)
      platform->source_subbottom = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM1)
      platform->source_subbottom1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM2)
      platform->source_subbottom2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM3)
      platform->source_subbottom3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_CAMERA)
      platform->source_camera = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_CAMERA1)
      platform->source_camera1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_CAMERA2)
      platform->source_camera2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_CAMERA3)
      platform->source_camera3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_POSITION)
      platform->source_position = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_POSITION1)
      platform->source_position1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_POSITION2)
      platform->source_position2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_POSITION3)
      platform->source_position3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_DEPTH)
      platform->source_depth = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_DEPTH1)
      platform->source_depth1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_DEPTH2)
      platform->source_depth2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_DEPTH3)
      platform->source_depth3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEADING)
      platform->source_heading = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEADING1)
      platform->source_heading1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEADING2)
      platform->source_heading2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEADING3)
      platform->source_heading3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH)
      platform->source_rollpitch = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH1)
      platform->source_rollpitch1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH2)
      platform->source_rollpitch2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH3)
      platform->source_rollpitch3 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEAVE)
      platform->source_heave = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEAVE1)
      platform->source_heave1 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEAVE2)
      platform->source_heave2 = sensor;
    else if (source_type == MB_PLATFORM_SOURCE_HEAVE3)
      platform->source_heave3 = sensor;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:                  %p\n", platform_ptr);
    if (platform_ptr != NULL) {
      struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;
      if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY)
        fprintf(stderr, "dbg2       value set: platform->source_bathymetry:    %d\n", platform->source_bathymetry);
      if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY1)
        fprintf(stderr, "dbg2       value set: platform->source_bathymetry1:    %d\n", platform->source_bathymetry1);
      else if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY2)
        fprintf(stderr, "dbg2       value set: platform->source_bathymetry2:    %d\n", platform->source_bathymetry2);
      else if (source_type == MB_PLATFORM_SOURCE_BATHYMETRY3)
        fprintf(stderr, "dbg2       value set: platform->source_bathymetry3:    %d\n", platform->source_bathymetry3);
      else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER)
        fprintf(stderr, "dbg2       value set: platform->source_backscatter:    %d\n", platform->source_backscatter);
      else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER1)
        fprintf(stderr, "dbg2       value set: platform->source_backscatter1:    %d\n", platform->source_backscatter1);
      else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER2)
        fprintf(stderr, "dbg2       value set: platform->source_backscatter2:    %d\n", platform->source_backscatter2);
      else if (source_type == MB_PLATFORM_SOURCE_BACKSCATTER3)
        fprintf(stderr, "dbg2       value set: platform->source_backscatter3:    %d\n", platform->source_backscatter3);
      else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM)
        fprintf(stderr, "dbg2       value set: platform->source_subbottom:        %d\n", platform->source_subbottom);
      else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM1)
        fprintf(stderr, "dbg2       value set: platform->source_subbottom1:    %d\n", platform->source_subbottom1);
      else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM2)
        fprintf(stderr, "dbg2       value set: platform->source_subbottom2:    %d\n", platform->source_subbottom2);
      else if (source_type == MB_PLATFORM_SOURCE_SUBBOTTOM3)
        fprintf(stderr, "dbg2       value set: platform->source_subbottom3:    %d\n", platform->source_subbottom3);
      else if (source_type == MB_PLATFORM_SOURCE_CAMERA)
        fprintf(stderr, "dbg2       value set: platform->source_camera:        %d\n", platform->source_camera);
      else if (source_type == MB_PLATFORM_SOURCE_CAMERA1)
        fprintf(stderr, "dbg2       value set: platform->source_camera1:       %d\n", platform->source_camera1);
      else if (source_type == MB_PLATFORM_SOURCE_CAMERA2)
        fprintf(stderr, "dbg2       value set: platform->source_camera2:       %d\n", platform->source_camera2);
      else if (source_type == MB_PLATFORM_SOURCE_CAMERA3)
        fprintf(stderr, "dbg2       value set: platform->source_camera3:       %d\n", platform->source_camera3);
      else if (source_type == MB_PLATFORM_SOURCE_POSITION)
        fprintf(stderr, "dbg2       value set: platform->source_position:      %d\n", platform->source_position);
      else if (source_type == MB_PLATFORM_SOURCE_POSITION1)
        fprintf(stderr, "dbg2       value set: platform->source_position1:      %d\n", platform->source_position1);
      else if (source_type == MB_PLATFORM_SOURCE_POSITION2)
        fprintf(stderr, "dbg2       value set: platform->source_position2:      %d\n", platform->source_position2);
      else if (source_type == MB_PLATFORM_SOURCE_POSITION3)
        fprintf(stderr, "dbg2       value set: platform->source_position3:      %d\n", platform->source_position3);
      else if (source_type == MB_PLATFORM_SOURCE_DEPTH)
        fprintf(stderr, "dbg2       value set: platform->source_depth:        %d\n", platform->source_depth);
      else if (source_type == MB_PLATFORM_SOURCE_DEPTH1)
        fprintf(stderr, "dbg2       value set: platform->source_depth1:      %d\n", platform->source_depth1);
      else if (source_type == MB_PLATFORM_SOURCE_DEPTH2)
        fprintf(stderr, "dbg2       value set: platform->source_depth2:      %d\n", platform->source_depth2);
      else if (source_type == MB_PLATFORM_SOURCE_DEPTH3)
        fprintf(stderr, "dbg2       value set: platform->source_depth3:      %d\n", platform->source_depth3);
      else if (source_type == MB_PLATFORM_SOURCE_HEADING)
        fprintf(stderr, "dbg2       value set: platform->source_heading:      %d\n", platform->source_heading);
      else if (source_type == MB_PLATFORM_SOURCE_HEADING1)
        fprintf(stderr, "dbg2       value set: platform->source_heading1:      %d\n", platform->source_heading1);
      else if (source_type == MB_PLATFORM_SOURCE_HEADING2)
        fprintf(stderr, "dbg2       value set: platform->source_heading2:      %d\n", platform->source_heading2);
      else if (source_type == MB_PLATFORM_SOURCE_HEADING3)
        fprintf(stderr, "dbg2       value set: platform->source_heading3:      %d\n", platform->source_heading3);
      else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH)
        fprintf(stderr, "dbg2       value set: platform->source_rollpitch:      %d\n", platform->source_rollpitch);
      else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH1)
        fprintf(stderr, "dbg2       value set: platform->source_rollpitch1:    %d\n", platform->source_rollpitch1);
      else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH2)
        fprintf(stderr, "dbg2       value set: platform->source_rollpitch2:    %d\n", platform->source_rollpitch2);
      else if (source_type == MB_PLATFORM_SOURCE_ROLLPITCH3)
        fprintf(stderr, "dbg2       value set: platform->source_rollpitch3:    %d\n", platform->source_rollpitch3);
      else if (source_type == MB_PLATFORM_SOURCE_HEAVE)
        fprintf(stderr, "dbg2       value set: platform->source_heave:        %d\n", platform->source_heave);
      else if (source_type == MB_PLATFORM_SOURCE_HEAVE1)
        fprintf(stderr, "dbg2       value set: platform->source_heave1:      %d\n", platform->source_heave1);
      else if (source_type == MB_PLATFORM_SOURCE_HEAVE2)
        fprintf(stderr, "dbg2       value set: platform->source_heave2:      %d\n", platform->source_heave2);
      else if (source_type == MB_PLATFORM_SOURCE_HEAVE3)
        fprintf(stderr, "dbg2       value set: platform->source_heave3:      %d\n", platform->source_heave3);
    }
    fprintf(stderr, "dbg2       error:                        %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:                        %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/

int mb_platform_deall(int verbose, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:      %p\n", platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:     %p\n", *platform_ptr);
  }

  int status = MB_SUCCESS;

  /* free memory for platform descriptor structure if needed */
  if (*platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)*platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* loop over all sensors */
    for (int isensor = 0; isensor < platform->num_sensors_alloc; isensor++) {
      struct mb_sensor_struct *sensor = (struct mb_sensor_struct *)&platform->sensors[isensor];

      /* free all offsets */
      if (sensor->num_offsets_alloc > 0 && sensor->offsets != NULL) {
        /* free any time latency model */
        for (int ioffset = 0; ioffset < sensor->num_offsets; ioffset++) {
          if (sensor->num_time_latency_alloc > 0) {
            status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensor->time_latency_time_d, error);
            status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&sensor->time_latency_value, error);
            sensor->num_time_latency_alloc = 0;
          }
        }

        status = mb_freed(verbose, __FILE__, __LINE__, (void **)&sensor->offsets, error);
        sensor->num_offsets_alloc = 0;
      }
    }

    /* free all offsets */
    if (platform->num_sensors_alloc > 0 && platform->sensors != NULL) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&platform->sensors, error);
    }

    /* free platform structure */
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)platform_ptr, error);
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:     %p\n", *platform_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/

int mb_platform_read(int verbose, char *platform_file, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_file:     %s\n", platform_file);
    fprintf(stderr, "dbg2       platform_ptr:      %p\n", platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:     %p\n", *platform_ptr);
  }

  int status = MB_SUCCESS;

  /* allocate memory for platform descriptor structure if needed */
  if (*platform_ptr == NULL) {
    status = mb_platform_init(verbose, platform_ptr, error);
  }

  /* proceed if platform structure is allocated */
  if (*platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)*platform_ptr;

    /* open and read platform file */
    FILE *fp = fopen(platform_file, "r");
    if (fp != NULL) {
      char buffer[MB_PATH_MAXLINE];
      char *result;
      while ((result = fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer) {
        if (buffer[0] != '#') {
          const int len = strlen(buffer);
          if (len > 0) {
            if (buffer[len - 1] == '\n')
              buffer[len - 1] = '\0';
            if (buffer[len - 2] == '\r')
              buffer[len - 2] = '\0';
          }

          char dummy[MB_PATH_MAXLINE];

          /* general parameters */
          if (strncmp(buffer, "PLATFORM_TYPE", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->type);
          }
          else if (strncmp(buffer, "PLATFORM_NAME", 13) == 0) {
            sscanf(buffer, "%s %s", dummy, platform->name);
          }
          else if (strncmp(buffer, "PLATFORM_ORGANIZATION", 21) == 0) {
            sscanf(buffer, "%s %s", dummy, platform->organization);
          }
          else if (strncmp(buffer, "DOCUMENTATION_URL", 17) == 0) {
            sscanf(buffer, "%s %s", dummy, platform->documentation_url);
          }
          else if (strncmp(buffer, "START_TIME_D", 12) == 0) {
            sscanf(buffer, "%s %lf", dummy, &platform->start_time_d);
            if (platform->start_time_d > 100.0)
              mb_get_date(verbose, platform->start_time_d, platform->start_time_i);
            else
              memset(platform->start_time_i, 0, 7 * sizeof(int));
          }
          else if (strncmp(buffer, "END_TIME_D", 10) == 0) {
            sscanf(buffer, "%s %lf", dummy, &platform->end_time_d);
            if (platform->end_time_d > 100.0)
              mb_get_date(verbose, platform->end_time_d, platform->end_time_i);
            else
              memset(platform->end_time_i, 0, 7 * sizeof(int));
          }

          else if (strncmp(buffer, "SOURCE_BATHYMETRY1", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_bathymetry1);
          }
          else if (strncmp(buffer, "SOURCE_BATHYMETRY2", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_bathymetry2);
          }
          else if (strncmp(buffer, "SOURCE_BATHYMETRY3", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_bathymetry3);
          }
          else if (strncmp(buffer, "SOURCE_BATHYMETRY", 17) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_bathymetry);
          }
          else if (strncmp(buffer, "SOURCE_BACKSCATTER1", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_backscatter1);
          }
          else if (strncmp(buffer, "SOURCE_BACKSCATTER2", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_backscatter2);
          }
          else if (strncmp(buffer, "SOURCE_BACKSCATTER3", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_backscatter3);
          }
          else if (strncmp(buffer, "SOURCE_BACKSCATTER", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_backscatter);
          }
          else if (strncmp(buffer, "SOURCE_SUBBOTTOM1", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_subbottom1);
          }
          else if (strncmp(buffer, "SOURCE_SUBBOTTOM2", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_subbottom2);
          }
          else if (strncmp(buffer, "SOURCE_SUBBOTTOM3", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_subbottom3);
          }
          else if (strncmp(buffer, "SOURCE_SUBBOTTOM", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_subbottom);
          }
          else if (strncmp(buffer, "SOURCE_SUBCAMERA1", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_camera1);
          }
          else if (strncmp(buffer, "SOURCE_SUBCAMERA2", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_camera2);
          }
          else if (strncmp(buffer, "SOURCE_SUBCAMERA3", 19) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_camera3);
          }
          else if (strncmp(buffer, "SOURCE_SUBCAMERA", 18) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_camera);
          }
          else if (strncmp(buffer, "SOURCE_POSITION1", 16) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_position1);
          }
          else if (strncmp(buffer, "SOURCE_POSITION2", 16) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_position2);
          }
          else if (strncmp(buffer, "SOURCE_POSITION3", 16) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_position3);
          }
          else if (strncmp(buffer, "SOURCE_POSITION", 15) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_position);
          }
          else if (strncmp(buffer, "SOURCE_DEPTH1", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_depth1);
          }
          else if (strncmp(buffer, "SOURCE_DEPTH2", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_depth2);
          }
          else if (strncmp(buffer, "SOURCE_DEPTH3", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_depth3);
          }
          else if (strncmp(buffer, "SOURCE_DEPTH", 12) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_depth);
          }
          else if (strncmp(buffer, "SOURCE_HEADING1", 15) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heading1);
          }
          else if (strncmp(buffer, "SOURCE_HEADING2", 15) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heading2);
          }
          else if (strncmp(buffer, "SOURCE_HEADING3", 15) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heading3);
          }
          else if (strncmp(buffer, "SOURCE_HEADING", 14) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heading);
          }
          else if (strncmp(buffer, "SOURCE_ROLLPITCH1", 17) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_rollpitch1);
          }
          else if (strncmp(buffer, "SOURCE_ROLLPITCH2", 17) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_rollpitch2);
          }
          else if (strncmp(buffer, "SOURCE_ROLLPITCH3", 17) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_rollpitch3);
          }
          else if (strncmp(buffer, "SOURCE_ROLLPITCH", 16) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_rollpitch);
          }
          else if (strncmp(buffer, "SOURCE_HEAVE1", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heave1);
          }
          else if (strncmp(buffer, "SOURCE_HEAVE2", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heave2);
          }
          else if (strncmp(buffer, "SOURCE_HEAVE3", 13) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heave3);
          }
          else if (strncmp(buffer, "SOURCE_HEAVE", 12) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->source_heave);
          }

          else if (strncmp(buffer, "PLATFORM_NUM_SENSORS", 20) == 0) {
            sscanf(buffer, "%s %d", dummy, &platform->num_sensors);
            if (platform->num_sensors > platform->num_sensors_alloc) {
              const size_t size = platform->num_sensors * sizeof(struct mb_sensor_struct);
              status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&platform->sensors, error);
              if (status == MB_SUCCESS) {
                memset(platform->sensors, 0, size);
                platform->num_sensors_alloc = platform->num_sensors;
              }
              else {
                char *message;
                mb_error(verbose, *error, &message);
                fprintf(stderr, "\nMBIO Error allocating sensor structures:\n%s\n", message);
                fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
                exit(*error);
              }
            }
          }

          else if (strncmp(buffer, "SENSOR_TYPE", 11) == 0) {
            int isensor;
            int ivalue;
            sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              platform->sensors[isensor].type = ivalue;
          }
          else if (strncmp(buffer, "SENSOR_MODEL", 12) == 0) {
            int isensor;
            char svalue[MB_PATH_MAXLINE];
            sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              strcpy(platform->sensors[isensor].model, svalue);
          }
          else if (strncmp(buffer, "SENSOR_MANUFACTURER", 19) == 0) {
            int isensor;
            char svalue[MB_PATH_MAXLINE];
            sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              strcpy(platform->sensors[isensor].manufacturer, svalue);
          }
          else if (strncmp(buffer, "SENSOR_SERIALNUMBER", 19) == 0) {
            int isensor;
            char svalue[MB_PATH_MAXLINE];
            sscanf(buffer, "%s %d %s", dummy, &isensor, svalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              strcpy(platform->sensors[isensor].serialnumber, svalue);
          }
          else if (strncmp(buffer, "SENSOR_CAPABILITY1", 18) == 0) {
            int isensor;
            int ivalue;
            sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              platform->sensors[isensor].capability1 = ivalue;
          }
          else if (strncmp(buffer, "SENSOR_CAPABILITY2", 18) == 0) {
            int isensor;
            int ivalue;
            sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              platform->sensors[isensor].capability2 = ivalue;
          }
          else if (strncmp(buffer, "SENSOR_NUM_OFFSETS", 17) == 0) {
            int isensor;
            int ivalue;
            sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
            if (isensor >= 0 && isensor < platform->num_sensors)
              platform->sensors[isensor].num_offsets = ivalue;
            if (platform->sensors[isensor].num_offsets > platform->sensors[isensor].num_offsets_alloc) {
              const size_t size = platform->sensors[isensor].num_offsets * sizeof(struct mb_sensor_offset_struct);
              status = mb_mallocd(verbose, __FILE__, __LINE__, size, (void **)&platform->sensors[isensor].offsets,
                                  error);
              if (status == MB_SUCCESS) {
                memset(platform->sensors[isensor].offsets, 0, size);
                platform->sensors[isensor].num_offsets_alloc = platform->sensors[isensor].num_offsets;
              }
              else {
                char *message;
                mb_error(verbose, *error, &message);
                fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
                fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
                exit(*error);
              }
            }
          }

          else if (strncmp(buffer, "OFFSET_POSITION", 15) == 0) {
            int isensor;
            int ioffset;
            double dvalue;
            double dvalue2;
            double dvalue3;
            sscanf(buffer, "%s %d %d %lf %lf %lf", dummy, &isensor, &ioffset, &dvalue, &dvalue2, &dvalue3);
            platform->sensors[isensor].offsets[ioffset].position_offset_x = dvalue;
            platform->sensors[isensor].offsets[ioffset].position_offset_y = dvalue2;
            platform->sensors[isensor].offsets[ioffset].position_offset_z = dvalue3;
            platform->sensors[isensor].offsets[ioffset].position_offset_mode = MB_SENSOR_POSITION_OFFSET_STATIC;
          }
          else if (strncmp(buffer, "OFFSET_ATTITUDE", 15) == 0) {
            int isensor;
            int ioffset;
            double dvalue;
            double dvalue2;
            double dvalue3;
            sscanf(buffer, "%s %d %d %lf %lf %lf", dummy, &isensor, &ioffset, &dvalue, &dvalue2, &dvalue3);
            platform->sensors[isensor].offsets[ioffset].attitude_offset_heading = dvalue;
            platform->sensors[isensor].offsets[ioffset].attitude_offset_roll = dvalue2;
            platform->sensors[isensor].offsets[ioffset].attitude_offset_pitch = dvalue3;
            platform->sensors[isensor].offsets[ioffset].attitude_offset_mode = MB_SENSOR_ATTITUDE_OFFSET_STATIC;
          }

          else if (strncmp(buffer, "SENSOR_TIME_LATENCY_STATIC", 26) == 0) {
            int isensor;
            double dvalue;
            sscanf(buffer, "%s %d %lf", dummy, &isensor, &dvalue);
            if (isensor >= 0 && isensor < platform->num_sensors) {
              platform->sensors[isensor].time_latency_static = dvalue;
              platform->sensors[isensor].time_latency_mode = MB_SENSOR_TIME_LATENCY_STATIC;
            }
          }
          else if (strncmp(buffer, "SENSOR_TIME_LATENCY_MODEL", 26) == 0) {
            int isensor;
            int ivalue;
            sscanf(buffer, "%s %d %d", dummy, &isensor, &ivalue);
            if (isensor >= 0 && isensor < platform->num_sensors) {
              platform->sensors[isensor].num_time_latency = ivalue;
              platform->sensors[isensor].time_latency_mode = MB_SENSOR_TIME_LATENCY_MODEL;
              if (platform->sensors[isensor].num_time_latency < platform->sensors[isensor].num_time_latency_alloc) {
                const size_t size = platform->sensors[isensor].num_time_latency * sizeof(double);
                status &= mb_mallocd(verbose, __FILE__, __LINE__, size,
                                    (void **)&platform->sensors[isensor].time_latency_time_d, error);
                status &= mb_mallocd(verbose, __FILE__, __LINE__, size,
                                    (void **)&platform->sensors[isensor].time_latency_value, error);
                if (status == MB_SUCCESS) {
                  memset(platform->sensors[isensor].time_latency_time_d, 0, size);
                  memset(platform->sensors[isensor].time_latency_value, 0, size);
                  platform->sensors[isensor].num_time_latency_alloc =
                      platform->sensors[isensor].num_time_latency;
                }
                else {
                  char *message;
                  mb_error(verbose, *error, &message);
                  fprintf(stderr, "\nMBIO Error allocating sensor offsets structures:\n%s\n", message);
                  fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
                  exit(*error);
                }
              }

              /* read the time latency model */
              for (int i = 0; i < platform->sensors[isensor].num_time_latency; i++) {
                if ((result = fgets(buffer, MB_PATH_MAXLINE, fp)) == buffer) {
                  sscanf(buffer, "%lf %lf", &platform->sensors[isensor].time_latency_time_d[i],
                         &platform->sensors[isensor].time_latency_value[i]);
                }
                else {
                  char *message;
                  status = MB_FAILURE;
                  *error = MB_ERROR_EOF;
                  mb_error(verbose, *error, &message);
                  fprintf(stderr, "\nMBIO Error parsing sensor offset time latency model:\n%s\n", message);
                  fprintf(stderr, "\nProgram terminated in function <%s>\n", __func__);
                  exit(*error);
                }
              }
            }
          }
        }
      }

      /* close the file */
      fclose(fp);

      /* only successful if at least one sensor is defined */
      if (platform->num_sensors <= 0) {
        *error = MB_ERROR_BAD_PARAMETER;
        status = MB_FAILURE;
      }

      /* print platform */
      if (verbose >= 2) {
        status = mb_platform_print(verbose, (void *)platform, error);
      }
    }

    /* failure opening */
    else {
      *error = MB_ERROR_OPEN_FAIL;
      status = MB_FAILURE;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_write(int verbose, char *platform_file, void *platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
  }

  int status = MB_SUCCESS;

  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* open and write platform file */
    FILE *fp = fopen(platform_file, "w");
    if (fp != NULL) {
      char user[256], host[256], date[32];
      status = mb_user_host_date(verbose, user, host, date, error);
      fprintf(fp, "## MB-System Platform Definition File\n");
      fprintf(fp, "MB-SYSTEM_VERSION        %s\n", MB_VERSION);
      fprintf(fp, "FILE_VERSION             1.00\n");
      fprintf(fp, "ORIGIN                   Generated by user <%s> on cpu <%s> at <%s>\n", user, host, date);
      fprintf(fp, "##\n");
      mb_path type_string;
      strcpy(type_string, mb_platform_type(platform->type));
      fprintf(fp, "PLATFORM_TYPE            %d  ## %s\n", platform->type, type_string);
      fprintf(fp, "PLATFORM_NAME            %s\n", platform->name);
      fprintf(fp, "PLATFORM_ORGANIZATION    %s\n", platform->organization);
      fprintf(fp, "DOCUMENTATION_URL        %s\n", platform->documentation_url);
      fprintf(fp, "##\n");
      fprintf(fp, "START_TIME_D             %f  ## %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", platform->start_time_d,
              platform->start_time_i[0], platform->start_time_i[1], platform->start_time_i[2], platform->start_time_i[3],
              platform->start_time_i[4], platform->start_time_i[5], platform->start_time_i[6]);
      fprintf(fp, "END_TIME_D             %f  ## %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n", platform->end_time_d,
              platform->end_time_i[0], platform->end_time_i[1], platform->end_time_i[2], platform->end_time_i[3],
              platform->end_time_i[4], platform->end_time_i[5], platform->end_time_i[6]);
      fprintf(fp, "##\n");
      fprintf(fp, "PLATFORM_NUM_SENSORS     %d\n", platform->num_sensors);
      fprintf(fp, "##\n");
      fprintf(fp, "## Defined data source sensors:\n");
      if (platform->source_bathymetry >= 0)
        fprintf(fp, "SOURCE_BATHYMETRY        %d\n", platform->source_bathymetry);
      if (platform->source_bathymetry1 >= 0)
        fprintf(fp, "SOURCE_BATHYMETRY1       %d\n", platform->source_bathymetry1);
      if (platform->source_bathymetry2 >= 0)
        fprintf(fp, "SOURCE_BATHYMETRY2       %d\n", platform->source_bathymetry2);
      if (platform->source_bathymetry3 >= 0)
        fprintf(fp, "SOURCE_BATHYMETRY3       %d\n", platform->source_bathymetry3);
      if (platform->source_backscatter >= 0)
        fprintf(fp, "SOURCE_BACKSCATTER       %d\n", platform->source_backscatter);
      if (platform->source_backscatter1 >= 0)
        fprintf(fp, "SOURCE_BACKSCATTER1      %d\n", platform->source_backscatter1);
      if (platform->source_backscatter2 >= 0)
        fprintf(fp, "SOURCE_BACKSCATTER2      %d\n", platform->source_backscatter2);
      if (platform->source_backscatter3 >= 0)
        fprintf(fp, "SOURCE_BACKSCATTER3      %d\n", platform->source_backscatter3);
      if (platform->source_subbottom >= 0)
        fprintf(fp, "SOURCE_SUBBOTTOM         %d\n", platform->source_subbottom);
      if (platform->source_subbottom1 >= 0)
        fprintf(fp, "SOURCE_SUBBOTTOM1        %d\n", platform->source_subbottom1);
      if (platform->source_subbottom2 >= 0)
        fprintf(fp, "SOURCE_SUBBOTTOM2        %d\n", platform->source_subbottom2);
      if (platform->source_subbottom3 >= 0)
        fprintf(fp, "SOURCE_SUBBOTTOM3        %d\n", platform->source_subbottom3);
      if (platform->source_camera >= 0)
        fprintf(fp, "SOURCE_CAMERA            %d\n", platform->source_camera);
      if (platform->source_camera1 >= 0)
        fprintf(fp, "SOURCE_CAMERA1           %d\n", platform->source_camera1);
      if (platform->source_camera2 >= 0)
        fprintf(fp, "SOURCE_CAMERA2           %d\n", platform->source_camera2);
      if (platform->source_camera3 >= 0)
        fprintf(fp, "SOURCE_CAMERA3           %d\n", platform->source_camera3);
      if (platform->source_position >= 0)
        fprintf(fp, "SOURCE_POSITION          %d\n", platform->source_position);
      if (platform->source_position1 >= 0)
        fprintf(fp, "SOURCE_POSITION1         %d\n", platform->source_position1);
      if (platform->source_position2 >= 0)
        fprintf(fp, "SOURCE_POSITION2         %d\n", platform->source_position2);
      if (platform->source_position3 >= 0)
        fprintf(fp, "SOURCE_POSITION3         %d\n", platform->source_position3);
      if (platform->source_depth >= 0)
        fprintf(fp, "SOURCE_DEPTH             %d\n", platform->source_depth);
      if (platform->source_depth1 >= 0)
        fprintf(fp, "SOURCE_DEPTH1            %d\n", platform->source_depth1);
      if (platform->source_depth2 >= 0)
        fprintf(fp, "SOURCE_DEPTH2            %d\n", platform->source_depth2);
      if (platform->source_depth3 >= 0)
        fprintf(fp, "SOURCE_DEPTH3            %d\n", platform->source_depth3);
      if (platform->source_heading >= 0)
        fprintf(fp, "SOURCE_HEADING           %d\n", platform->source_heading);
      if (platform->source_heading1 >= 0)
        fprintf(fp, "SOURCE_HEADING1          %d\n", platform->source_heading1);
      if (platform->source_heading2 >= 0)
        fprintf(fp, "SOURCE_HEADING2          %d\n", platform->source_heading2);
      if (platform->source_heading3 >= 0)
        fprintf(fp, "SOURCE_HEADING3          %d\n", platform->source_heading3);
      if (platform->source_rollpitch >= 0)
        fprintf(fp, "SOURCE_ROLLPITCH         %d\n", platform->source_rollpitch);
      if (platform->source_rollpitch1 >= 0)
        fprintf(fp, "SOURCE_ROLLPITCH1        %d\n", platform->source_rollpitch1);
      if (platform->source_rollpitch2 >= 0)
        fprintf(fp, "SOURCE_ROLLPITCH2        %d\n", platform->source_rollpitch2);
      if (platform->source_rollpitch3 >= 0)
        fprintf(fp, "SOURCE_ROLLPITCH3        %d\n", platform->source_rollpitch3);
      if (platform->source_heave >= 0)
        fprintf(fp, "SOURCE_HEAVE             %d\n", platform->source_heave);
      if (platform->source_heave1 >= 0)
        fprintf(fp, "SOURCE_HEAVE1            %d\n", platform->source_heave1);
      if (platform->source_heave2 >= 0)
        fprintf(fp, "SOURCE_HEAVE2            %d\n", platform->source_heave2);
      if (platform->source_heave3 >= 0)
        fprintf(fp, "SOURCE_HEAVE3            %d\n", platform->source_heave3);
      fprintf(fp, "##\n");
      fprintf(fp, "## Undefined data sources:\n");
      if (platform->source_bathymetry < 0)
        fprintf(fp, "  ## SOURCE_BATHYMETRY\n");
      if (platform->source_bathymetry1 < 0)
        fprintf(fp, "  ## SOURCE_BATHYMETRY1\n");
      if (platform->source_bathymetry2 < 0)
        fprintf(fp, "  ## SOURCE_BATHYMETRY2\n");
      if (platform->source_bathymetry3 < 0)
        fprintf(fp, "  ## SOURCE_BATHYMETRY3\n");
      if (platform->source_backscatter < 0)
        fprintf(fp, "  ## SOURCE_BACKSCATTER\n");
      if (platform->source_backscatter1 < 0)
        fprintf(fp, "  ## SOURCE_BACKSCATTER1\n");
      if (platform->source_backscatter2 < 0)
        fprintf(fp, "  ## SOURCE_BACKSCATTER2\n");
      if (platform->source_backscatter3 < 0)
        fprintf(fp, "  ## SOURCE_BACKSCATTER3\n");
      if (platform->source_subbottom < 0)
        fprintf(fp, "  ## SOURCE_SUBBOTTOM\n");
      if (platform->source_subbottom1 < 0)
        fprintf(fp, "  ## SOURCE_SUBBOTTOM1\n");
      if (platform->source_subbottom2 < 0)
        fprintf(fp, "  ## SOURCE_SUBBOTTOM2\n");
      if (platform->source_subbottom3 < 0)
        fprintf(fp, "  ## SOURCE_SUBBOTTOM3\n");
      if (platform->source_camera < 0)
        fprintf(fp, "  ## SOURCE_CAMERA\n");
      if (platform->source_camera1 < 0)
        fprintf(fp, "  ## SOURCE_CAMERA1\n");
      if (platform->source_camera2 < 0)
        fprintf(fp, "  ## SOURCE_CAMERA2\n");
      if (platform->source_camera3 < 0)
        fprintf(fp, "  ## SOURCE_CAMERA3\n");
      if (platform->source_position < 0)
        fprintf(fp, "  ## SOURCE_POSITION\n");
      if (platform->source_position1 < 0)
        fprintf(fp, "  ## SOURCE_POSITION1\n");
      if (platform->source_position2 < 0)
        fprintf(fp, "  ## SOURCE_POSITION2\n");
      if (platform->source_position3 < 0)
        fprintf(fp, "  ## SOURCE_POSITION3\n");
      if (platform->source_depth < 0)
        fprintf(fp, "  ## SOURCE_DEPTH\n");
      if (platform->source_depth1 < 0)
        fprintf(fp, "  ## SOURCE_DEPTH1\n");
      if (platform->source_depth2 < 0)
        fprintf(fp, "  ## SOURCE_DEPTH2\n");
      if (platform->source_depth3 < 0)
        fprintf(fp, "  ## SOURCE_DEPTH3\n");
      if (platform->source_heading < 0)
        fprintf(fp, "  ## SOURCE_HEADING\n");
      if (platform->source_heading1 < 0)
        fprintf(fp, "  ## SOURCE_HEADING1\n");
      if (platform->source_heading2 < 0)
        fprintf(fp, "  ## SOURCE_HEADING2\n");
      if (platform->source_heading3 < 0)
        fprintf(fp, "  ## SOURCE_HEADING3\n");
      if (platform->source_rollpitch < 0)
        fprintf(fp, "  ## SOURCE_ROLLPITCH\n");
      if (platform->source_rollpitch1 < 0)
        fprintf(fp, "  ## SOURCE_ROLLPITCH1\n");
      if (platform->source_rollpitch2 < 0)
        fprintf(fp, "  ## SOURCE_ROLLPITCH2\n");
      if (platform->source_rollpitch3 < 0)
        fprintf(fp, "  ## SOURCE_ROLLPITCH3\n");
      if (platform->source_heave < 0)
        fprintf(fp, "  ## SOURCE_HEAVE\n");
      if (platform->source_heave1 < 0)
        fprintf(fp, "  ## SOURCE_HEAVE1\n");
      if (platform->source_heave2 < 0)
        fprintf(fp, "  ## SOURCE_HEAVE2\n");
      if (platform->source_heave3 < 0)
        fprintf(fp, "  ## SOURCE_HEAVE3\n");
      fprintf(fp, "##\n");
      fprintf(fp, "## Sensor list:\n");
      for (int isensor = 0; isensor < platform->num_sensors; isensor++) {
        fprintf(fp, "##\n");
        strcpy(type_string, mb_sensor_type_string[0]);
        for (int i = 0; i < NUM_MB_SENSOR_TYPES; i++) {
          if (platform->sensors[isensor].type == mb_sensor_type_id[i])
            strcpy(type_string, mb_sensor_type_string[i]);
        }
        fprintf(fp, "SENSOR_TYPE               %2d  %3d  ## %s\n", isensor, platform->sensors[isensor].type,
                type_string);
        fprintf(fp, "SENSOR_MODEL                %2d  %s\n", isensor, platform->sensors[isensor].model);
        fprintf(fp, "SENSOR_MANUFACTURER         %2d  %s\n", isensor, platform->sensors[isensor].manufacturer);
        fprintf(fp, "SENSOR_SERIALNUMBER         %2d  %s\n", isensor, platform->sensors[isensor].serialnumber);
        fprintf(fp, "SENSOR_CAPABILITY1          %2d  %10d  ##", isensor, platform->sensors[isensor].capability1);
        if (mb_check_sensor_capability1_position(platform->sensors[isensor].capability1))
          fprintf(fp, " position");
        if (mb_check_sensor_capability1_depth(platform->sensors[isensor].capability1))
          fprintf(fp, " depth");
        if (mb_check_sensor_capability1_altitude(platform->sensors[isensor].capability1))
          fprintf(fp, " altitude");
        if (mb_check_sensor_capability1_velocity(platform->sensors[isensor].capability1))
          fprintf(fp, " velocity");
        if (mb_check_sensor_capability1_acceleration(platform->sensors[isensor].capability1))
          fprintf(fp, " acceleration");
        if (mb_check_sensor_capability1_pressure(platform->sensors[isensor].capability1))
          fprintf(fp, " pressure");
        if (mb_check_sensor_capability1_rollpitch(platform->sensors[isensor].capability1))
          fprintf(fp, " rollpitch");
        if (mb_check_sensor_capability1_heading(platform->sensors[isensor].capability1))
          fprintf(fp, " heading");
        if (mb_check_sensor_capability1_heading(platform->sensors[isensor].capability1))
          fprintf(fp, " heading");
        if (mb_check_sensor_capability1_unused09(platform->sensors[isensor].capability1))
          fprintf(fp, " unused09");
        if (mb_check_sensor_capability1_unused10(platform->sensors[isensor].capability1))
          fprintf(fp, " unused10");
        if (mb_check_sensor_capability1_unused11(platform->sensors[isensor].capability1))
          fprintf(fp, " unused11");
        if (mb_check_sensor_capability1_unused12(platform->sensors[isensor].capability1))
          fprintf(fp, " unused12");
        if (mb_check_sensor_capability1_temperature(platform->sensors[isensor].capability1))
          fprintf(fp, " temperature");
        if (mb_check_sensor_capability1_conductivity(platform->sensors[isensor].capability1))
          fprintf(fp, " conductivity");
        if (mb_check_sensor_capability1_salinity(platform->sensors[isensor].capability1))
          fprintf(fp, " salinity");
        if (mb_check_sensor_capability1_soundspeed(platform->sensors[isensor].capability1))
          fprintf(fp, " soundspeed");
        if (mb_check_sensor_capability1_unused17(platform->sensors[isensor].capability1))
          fprintf(fp, " unused17");
        if (mb_check_sensor_capability1_unused18(platform->sensors[isensor].capability1))
          fprintf(fp, " unused18");
        if (mb_check_sensor_capability1_unused19(platform->sensors[isensor].capability1))
          fprintf(fp, " unused19");
        if (mb_check_sensor_capability1_gravity(platform->sensors[isensor].capability1))
          fprintf(fp, " gravity");
        if (mb_check_sensor_capability1_unused21(platform->sensors[isensor].capability1))
          fprintf(fp, " unused21");
        if (mb_check_sensor_capability1_unused22(platform->sensors[isensor].capability1))
          fprintf(fp, " unused22");
        if (mb_check_sensor_capability1_unused23(platform->sensors[isensor].capability1))
          fprintf(fp, " unused23");
        if (mb_check_sensor_capability1_magneticfield(platform->sensors[isensor].capability1))
          fprintf(fp, " magneticfield");
        if (mb_check_sensor_capability1_unused25(platform->sensors[isensor].capability1))
          fprintf(fp, " unused25");
        if (mb_check_sensor_capability1_unused26(platform->sensors[isensor].capability1))
          fprintf(fp, " unused26");
        if (mb_check_sensor_capability1_unused27(platform->sensors[isensor].capability1))
          fprintf(fp, " unused27");
        if (mb_check_sensor_capability1_unused28(platform->sensors[isensor].capability1))
          fprintf(fp, " unused28");
        if (mb_check_sensor_capability1_unused29(platform->sensors[isensor].capability1))
          fprintf(fp, " unused29");
        if (mb_check_sensor_capability1_unused30(platform->sensors[isensor].capability1))
          fprintf(fp, " unused30");
        if (mb_check_sensor_capability1_unused31(platform->sensors[isensor].capability1))
          fprintf(fp, " unused31");
        fprintf(fp, "\n");
        fprintf(fp, "SENSOR_CAPABILITY2          %2d  %10d  ##", isensor, platform->sensors[isensor].capability2);
        if (mb_check_sensor_capability2_topography_echosounder(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_echosounder");
        if (mb_check_sensor_capability2_topography_interferometry(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_interferometry");
        if (mb_check_sensor_capability2_topography_sass(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_sass");
        if (mb_check_sensor_capability2_topography_multibeam(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_multibeam");
        if (mb_check_sensor_capability2_topography_photogrammetry(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_photogrammetry");
        if (mb_check_sensor_capability2_topography_structurefrommotion(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_structurefrommotion");
        if (mb_check_sensor_capability2_topography_lidar(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_lidar");
        if (mb_check_sensor_capability2_topography_structuredlight(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_structuredlight");
        if (mb_check_sensor_capability2_topography_laserscanner(platform->sensors[isensor].capability2))
          fprintf(fp, " topography_laserscanner");
        if (mb_check_sensor_capability2_unused09(platform->sensors[isensor].capability2))
          fprintf(fp, " unused09");
        if (mb_check_sensor_capability2_unused10(platform->sensors[isensor].capability2))
          fprintf(fp, " unused10");
        if (mb_check_sensor_capability2_unused11(platform->sensors[isensor].capability2))
          fprintf(fp, " unused11");
        if (mb_check_sensor_capability2_backscatter_echosounder(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_echosounder");
        if (mb_check_sensor_capability2_backscatter_sidescan(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_sidescan");
        if (mb_check_sensor_capability2_backscatter_interferometry(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_interferometry");
        if (mb_check_sensor_capability2_backscatter_sass(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_sass");
        if (mb_check_sensor_capability2_backscatter_multibeam(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_multibeam");
        if (mb_check_sensor_capability2_backscatter_lidar(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_lidar");
        if (mb_check_sensor_capability2_backscatter_structuredlight(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_structuredlight");
        if (mb_check_sensor_capability2_backscatter_laserscanner(platform->sensors[isensor].capability2))
          fprintf(fp, " backscatter_laserscanner");
        if (mb_check_sensor_capability2_unused20(platform->sensors[isensor].capability2))
          fprintf(fp, " unused20");
        if (mb_check_sensor_capability2_subbottom_echosounder(platform->sensors[isensor].capability2))
          fprintf(fp, " subbottom_echosounder");
        if (mb_check_sensor_capability2_subbottom_chirp(platform->sensors[isensor].capability2))
          fprintf(fp, " subbottom_chirp");
        if (mb_check_sensor_capability2_unused23(platform->sensors[isensor].capability2))
          fprintf(fp, " unused23");
        if (mb_check_sensor_capability2_photography(platform->sensors[isensor].capability2))
          fprintf(fp, " photography");
        if (mb_check_sensor_capability2_stereophotography(platform->sensors[isensor].capability2))
          fprintf(fp, " stereophotography");
        if (mb_check_sensor_capability2_video(platform->sensors[isensor].capability2))
          fprintf(fp, " video");
        if (mb_check_sensor_capability2_stereovideo(platform->sensors[isensor].capability2))
          fprintf(fp, " stereovideo");
        if (mb_check_sensor_capability2_unused28(platform->sensors[isensor].capability2))
          fprintf(fp, " unused28");
        if (mb_check_sensor_capability2_unused29(platform->sensors[isensor].capability2))
          fprintf(fp, " unused29");
        if (mb_check_sensor_capability2_unused30(platform->sensors[isensor].capability2))
          fprintf(fp, " unused30");
        if (mb_check_sensor_capability2_unused31(platform->sensors[isensor].capability2))
          fprintf(fp, " unused31");
        fprintf(fp, "\n");
        fprintf(fp, "SENSOR_NUM_OFFSETS          %2d  %2d\n", isensor, platform->sensors[isensor].num_offsets);
        for (int ioffset = 0; ioffset < platform->sensors[isensor].num_offsets; ioffset++) {
          if (platform->sensors[isensor].offsets[ioffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC) {
            fprintf(fp,
                    "OFFSET_POSITION             %2d      %2d  %10.6lf  %10.6lf  %10.6lf ## Starboard, Forward, Up "
                    "(meters)\n",
                    isensor, ioffset, platform->sensors[isensor].offsets[ioffset].position_offset_x,
                    platform->sensors[isensor].offsets[ioffset].position_offset_y,
                    platform->sensors[isensor].offsets[ioffset].position_offset_z);
          }
          if (platform->sensors[isensor].offsets[ioffset].attitude_offset_mode == MB_SENSOR_ATTITUDE_OFFSET_STATIC) {
            fprintf(fp,
                    "OFFSET_ATTITUDE             %2d      %2d  %10.6lf  %10.6lf  %10.6lf ## Heading, Roll, Pitch "
                    "(degrees)\n",
                    isensor, ioffset, platform->sensors[isensor].offsets[ioffset].attitude_offset_heading,
                    platform->sensors[isensor].offsets[ioffset].attitude_offset_roll,
                    platform->sensors[isensor].offsets[ioffset].attitude_offset_pitch);
          }
        }
        if (platform->sensors[isensor].time_latency_mode == MB_SENSOR_TIME_LATENCY_STATIC) {
          fprintf(fp, "SENSOR_TIME_LATENCY_STATIC  %2d      %10.6lf  ## Seconds\n", isensor,
                  platform->sensors[isensor].time_latency_static);
        }
        else if (platform->sensors[isensor].time_latency_mode == MB_SENSOR_TIME_LATENCY_MODEL) {
          fprintf(fp, "SENSOR_TIME_LATENCY_MODEL   %2d      %2d\n", isensor,
                  platform->sensors[isensor].num_time_latency);
          for (int i = 0; i < platform->sensors[isensor].num_time_latency; i++) {
            fprintf(fp, "                                     %10.6lf  %10.6lf  ## Seconds, Seconds",
                    platform->sensors[isensor].time_latency_time_d[i],
                    platform->sensors[isensor].time_latency_value[i]);
          }
        }
      }
      fprintf(fp, "##\n");

      /* close the file */
      fclose(fp);
    }
  }
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DATA;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_lever(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset,
                      double heading, double roll, double pitch,
                      double *lever_x, double *lever_y, double *lever_z, int *error) {
  /* reset error */
  *error = MB_ERROR_NO_ERROR;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       targetsensor:    %d\n", targetsensor);
    fprintf(stderr, "dbg2       targetsensoroffset:  %d\n", targetsensoroffset);
    fprintf(stderr, "dbg2       heading:        %f\n", heading);
    fprintf(stderr, "dbg2       roll:        %f\n", roll);
    fprintf(stderr, "dbg2       pitch:        %f\n", pitch);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* call mb_platform_orientation to get platform orientation */
    double pheading;
    double proll;
    double ppitch;
    status = mb_platform_orientation(verbose, platform_ptr, heading, roll, pitch, &pheading, &proll, &ppitch, error);

    /* check that the required sensor id's are sensible for this platform */
    if (targetsensor < 0 || targetsensor >= platform->num_sensors || platform->source_position < 0 ||
        platform->source_position >= platform->num_sensors || platform->source_depth < 0 ||
        platform->source_depth >= platform->num_sensors) {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_PARAMETER;
    }

    /* else proceed */
    else {
      /* get sensor structures */
      struct mb_sensor_struct *sensor_target = &platform->sensors[targetsensor];
      struct mb_sensor_struct *sensor_position = &platform->sensors[platform->source_position];
      struct mb_sensor_struct *sensor_depth = &platform->sensors[platform->source_depth];

      /* start with zero lever */
      *lever_x = 0.0;
      *lever_y = 0.0;
      *lever_z = 0.0;

      /* Convenient calculations for later coordinate operations */
      const double croll = cos(DTR * proll);
      const double sroll = sin(DTR * proll);
      const double cpitch = cos(DTR * ppitch);
      const double spitch = sin(DTR * ppitch);
      const double cheading = cos(DTR * pheading);
      const double sheading = sin(DTR * pheading);

      /* apply change in z due to offset between the depth sensor and the target sensor
      using roll add pitch values corrected for the attitude sensor offset */
      double xx = 0.0;
      double yy = 0.0;
      double zz = 0.0;
      if (sensor_target->offsets[targetsensoroffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC) {
        xx += sensor_target->offsets[targetsensoroffset].position_offset_x;
        yy += sensor_target->offsets[targetsensoroffset].position_offset_y;
        zz += sensor_target->offsets[targetsensoroffset].position_offset_z;
      }
      if (sensor_depth->offsets[0].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC) {
        xx -= sensor_depth->offsets[0].position_offset_x;
        yy -= sensor_depth->offsets[0].position_offset_y;
        zz -= sensor_depth->offsets[0].position_offset_z;
      }

      *lever_z = spitch * yy - cpitch * sroll * xx + cpitch * croll * zz;   // Note: Z up

      /* apply change in x and y due to offset between the position sensor and the target sensor
      using roll, pitch and heading corrected for the attitude sensor offset and the target sensor */
      xx = 0.0;
      yy = 0.0;
      zz = 0.0;
      if (sensor_target->offsets[targetsensoroffset].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC) {
        xx += sensor_target->offsets[targetsensoroffset].position_offset_x;
        yy += sensor_target->offsets[targetsensoroffset].position_offset_y;
        zz += sensor_target->offsets[targetsensoroffset].position_offset_z;
      }
      if (sensor_position->offsets[0].position_offset_mode == MB_SENSOR_POSITION_OFFSET_STATIC) {
        xx -= sensor_position->offsets[0].position_offset_x;
        yy -= sensor_position->offsets[0].position_offset_y;
        zz -= sensor_position->offsets[0].position_offset_z;
      }
      *lever_x =  cpitch * sheading * yy +
                 (cheading * croll + sheading * spitch * sroll) * xx -
                 (croll * sheading * spitch - cheading * sroll) * zz; // Note: X Starboard

      *lever_y =  cheading * cpitch * yy +
                 (cheading * spitch * sroll - croll * sheading) * xx -
                 (sheading * sroll + cheading * croll * spitch) * zz; // Note: Y Forward
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       lever_x:            %f\n", *lever_x);
    fprintf(stderr, "dbg2       lever_y:            %f\n", *lever_y);
    fprintf(stderr, "dbg2       lever_z:            %f\n", *lever_z);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_position(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset, double navlon, double navlat,
                         double sensordepth, double heading, double roll, double pitch, double *targetlon, double *targetlat,
                         double *targetdepth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:             %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       targetsensor:    %d\n", targetsensor);
    fprintf(stderr, "dbg2       targetsensoroffset:  %d\n", targetsensoroffset);
    fprintf(stderr, "dbg2       navlon:            %f\n", navlon);
    fprintf(stderr, "dbg2       navlat:            %f\n", navlat);
    fprintf(stderr, "dbg2       sensordepth:        %f\n", sensordepth);
    fprintf(stderr, "dbg2       heading:            %f\n", heading);
    fprintf(stderr, "dbg2       roll:            %f\n", roll);
    fprintf(stderr, "dbg2       pitch:            %f\n", pitch);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    double lever_x;
    double lever_y;
    double lever_z;

    /* call mb_platform lever to get relative lever offsets */
    status = mb_platform_lever(verbose, platform_ptr, targetsensor, targetsensoroffset, heading, roll, pitch, &lever_x,
                               &lever_y, &lever_z, error);

    /* get local translation between lon lat degrees and meters */
    double mtodeglon;
    double mtodeglat;
    mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);

    /* calculate absolute position and depth for target sensor
        - note that z is positive up but sensordepth is positive down */
    *targetlon = navlon + lever_x * mtodeglon;
    *targetlat = navlat + lever_y * mtodeglat;
    *targetdepth = sensordepth - lever_z;
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       targetlon:    %f\n", *targetlon);
    fprintf(stderr, "dbg2       targetlat:    %f\n", *targetlat);
    fprintf(stderr, "dbg2       targetdepth:      %f\n", *targetdepth);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_orientation(int verbose, void *platform_ptr, double heading, double roll, double pitch, double *platform_heading,
                            double *platform_roll, double *platform_pitch, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       heading:          %f\n", heading);
    fprintf(stderr, "dbg2       roll:          %f\n", roll);
    fprintf(stderr, "dbg2       pitch:          %f\n", pitch);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* check that all sensor id's are sensible for this platform */
    if (platform->source_heading < 0 || platform->source_heading >= platform->num_sensors || platform->source_rollpitch < 0 ||
        platform->source_rollpitch >= platform->num_sensors) {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_PARAMETER;
    }

    /* else proceed */
    else {
      /* get sensor structures */
      struct mb_sensor_struct *sensor_heading = &platform->sensors[platform->source_heading];
      struct mb_sensor_struct *sensor_rollpitch = &platform->sensors[platform->source_rollpitch];

      /* get platform attitude */
      if ((sensor_rollpitch->offsets[0].attitude_offset_mode == MB_SENSOR_ATTITUDE_OFFSET_STATIC) &&
          (sensor_rollpitch->offsets[0].attitude_offset_roll != 0.0 ||
           sensor_rollpitch->offsets[0].attitude_offset_pitch != 0.0 ||
           sensor_heading->offsets[0].attitude_offset_heading != 0.0)) {
        mb_platform_math_attitude_platform(
            verbose, roll, pitch, heading, sensor_rollpitch->offsets[0].attitude_offset_roll,
            sensor_rollpitch->offsets[0].attitude_offset_pitch, sensor_heading->offsets[0].attitude_offset_heading,
            platform_roll, platform_pitch, platform_heading, error);
      }
      else {
        *platform_roll = roll;
        *platform_pitch = pitch;
        *platform_heading = heading;
      }
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       platform_heading:  %f\n", *platform_heading);
    fprintf(stderr, "dbg2       platform_roll:    %f\n", *platform_roll);
    fprintf(stderr, "dbg2       platform_pitch:    %f\n", *platform_pitch);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_orientation_offset(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset,
                                   double *target_hdg_offset, double *target_roll_offset, double *target_pitch_offset,
                                   int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       targetsensor:    %d\n", targetsensor);
    fprintf(stderr, "dbg2       targetsensoroffset:  %d\n", targetsensoroffset);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* check that all sensor id's are sensible for this platform */
    if (targetsensor < 0 || targetsensor >= platform->num_sensors || platform->source_heading < 0 ||
        platform->source_heading >= platform->num_sensors || platform->source_rollpitch < 0 ||
        platform->source_rollpitch >= platform->num_sensors) {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_PARAMETER;
    }

    /* else proceed */
    else {
      /* get sensor structures */
      struct mb_sensor_struct *sensor_target = &platform->sensors[targetsensor];
      struct mb_sensor_struct *sensor_heading = &platform->sensors[platform->source_heading];
      struct mb_sensor_struct *sensor_rollpitch = &platform->sensors[platform->source_rollpitch];

      /* start with zero attitude offset */
      *target_roll_offset = 0.0;
      *target_pitch_offset = 0.0;
      *target_hdg_offset = 0.0;

      /* calculate attitude offset for target sensor */
      mb_platform_math_attitude_offset(verbose, sensor_target->offsets[targetsensoroffset].attitude_offset_roll,
                                       sensor_target->offsets[targetsensoroffset].attitude_offset_pitch,
                                       sensor_target->offsets[targetsensoroffset].attitude_offset_heading,
                                       sensor_rollpitch->offsets[0].attitude_offset_roll,
                                       sensor_rollpitch->offsets[0].attitude_offset_pitch,
                                       sensor_heading->offsets[0].attitude_offset_heading, target_roll_offset,
                                       target_pitch_offset, target_hdg_offset, error);
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       target_roll_offset:    %f\n", *target_roll_offset);
    fprintf(stderr, "dbg2       target_pitch_offset:    %f\n", *target_pitch_offset);
    fprintf(stderr, "dbg2       target_hdg_offset:    %f\n", *target_hdg_offset);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_orientation_target(int verbose, void *platform_ptr, int targetsensor, int targetsensoroffset, double heading,
                                   double roll, double pitch, double *target_heading, double *target_roll, double *target_pitch,
                                   int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:    %p\n", platform_ptr);
    fprintf(stderr, "dbg2       targetsensor:    %d\n", targetsensor);
    fprintf(stderr, "dbg2       targetsensoroffset:  %d\n", targetsensoroffset);
    fprintf(stderr, "dbg2       heading:            %f\n", heading);
    fprintf(stderr, "dbg2       roll:            %f\n", roll);
    fprintf(stderr, "dbg2       pitch:            %f\n", pitch);
  }

  int status = MB_SUCCESS;
  double target_roll_offset, target_pitch_offset, target_hdg_offset;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }

    /* check that all sensor id's are sensible for this platform */
    if (targetsensor < 0 || targetsensor >= platform->num_sensors || platform->source_heading < 0 ||
        platform->source_heading >= platform->num_sensors || platform->source_rollpitch < 0 ||
        platform->source_rollpitch >= platform->num_sensors) {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_PARAMETER;
    }

    /* else proceed */
    else {
      /* get sensor structures */
      struct mb_sensor_struct *sensor_target = &platform->sensors[targetsensor];

      /* start with zero attitude offset */
      target_roll_offset = 0.0;
      target_pitch_offset = 0.0;
      target_hdg_offset = 0.0;

      /* get target attitude offset (respect attitude source) */
      status = mb_platform_orientation_offset(verbose, platform_ptr, targetsensor, targetsensoroffset, &(target_hdg_offset),
                                              &(target_roll_offset), &(target_pitch_offset), error);

      /* get target attitude */
      if ((sensor_target->offsets[0].attitude_offset_mode == MB_SENSOR_ATTITUDE_OFFSET_STATIC) &&
          (target_hdg_offset != 0.0 || target_roll_offset != 0.0 || target_pitch_offset != 0.0)) {
        mb_platform_math_attitude_target(verbose, roll, pitch, heading, target_roll_offset, target_pitch_offset,
                                         target_hdg_offset, target_roll, target_pitch, target_heading, error);
      }
      else {
        *target_roll = roll;
        *target_pitch = pitch;
        *target_heading = heading;
      }
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       target_heading:  %f\n", *target_heading);
    fprintf(stderr, "dbg2       target_roll:    %f\n", *target_roll);
    fprintf(stderr, "dbg2       target_pitch:  %f\n", *target_pitch);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mb_platform_print(int verbose, void *platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:              %d\n", verbose);
    fprintf(stderr, "dbg2       platform_ptr:         %p\n", platform_ptr);
  }

  int status = MB_SUCCESS;

  /* work with valid platform pointer */
  if (platform_ptr != NULL) {
    /* get platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

    if (verbose >= 2 && platform_ptr != NULL) {
      fprintf(stderr, "dbg2       platform->type:                 %d\n", platform->type);
      fprintf(stderr, "dbg2       platform->name:                 %s\n", platform->name);
      fprintf(stderr, "dbg2       platform->organization:         %s\n", platform->organization);
      fprintf(stderr, "dbg2       platform->documentation_url:    %s\n", platform->documentation_url);
      fprintf(stderr, "dbg2       platform->start_time_d:         %f\n", platform->start_time_d);
      fprintf(stderr, "dbg2       platform->end_time_d:           %f\n", platform->end_time_d);
      fprintf(stderr, "dbg2       platform->source_bathymetry:    %d\n", platform->source_bathymetry);
      fprintf(stderr, "dbg2       platform->source_bathymetry1:   %d\n", platform->source_bathymetry1);
      fprintf(stderr, "dbg2       platform->source_bathymetry2:   %d\n", platform->source_bathymetry2);
      fprintf(stderr, "dbg2       platform->source_bathymetry3:   %d\n", platform->source_bathymetry3);
      fprintf(stderr, "dbg2       platform->source_backscatter:   %d\n", platform->source_backscatter);
      fprintf(stderr, "dbg2       platform->source_backscatter1:  %d\n", platform->source_backscatter1);
      fprintf(stderr, "dbg2       platform->source_backscatter2:  %d\n", platform->source_backscatter2);
      fprintf(stderr, "dbg2       platform->source_backscatter3:  %d\n", platform->source_backscatter3);
      fprintf(stderr, "dbg2       platform->source_subbottom:     %d\n", platform->source_subbottom);
      fprintf(stderr, "dbg2       platform->source_subbottom1:    %d\n", platform->source_subbottom1);
      fprintf(stderr, "dbg2       platform->source_subbottom2:    %d\n", platform->source_subbottom2);
      fprintf(stderr, "dbg2       platform->source_subbottom3:    %d\n", platform->source_subbottom3);
      fprintf(stderr, "dbg2       platform->source_camera:        %d\n", platform->source_camera);
      fprintf(stderr, "dbg2       platform->source_camera1:       %d\n", platform->source_camera1);
      fprintf(stderr, "dbg2       platform->source_camera2:       %d\n", platform->source_camera2);
      fprintf(stderr, "dbg2       platform->source_camera3:       %d\n", platform->source_camera3);
      fprintf(stderr, "dbg2       platform->source_position:      %d\n", platform->source_position);
      fprintf(stderr, "dbg2       platform->source_position1:     %d\n", platform->source_position1);
      fprintf(stderr, "dbg2       platform->source_position2:     %d\n", platform->source_position2);
      fprintf(stderr, "dbg2       platform->source_position3:     %d\n", platform->source_position3);
      fprintf(stderr, "dbg2       platform->source_depth:         %d\n", platform->source_depth);
      fprintf(stderr, "dbg2       platform->source_depth1:        %d\n", platform->source_depth1);
      fprintf(stderr, "dbg2       platform->source_depth2:        %d\n", platform->source_depth2);
      fprintf(stderr, "dbg2       platform->source_depth3:        %d\n", platform->source_depth3);
      fprintf(stderr, "dbg2       platform->source_heading:       %d\n", platform->source_heading);
      fprintf(stderr, "dbg2       platform->source_heading1:      %d\n", platform->source_heading1);
      fprintf(stderr, "dbg2       platform->source_heading2:      %d\n", platform->source_heading2);
      fprintf(stderr, "dbg2       platform->source_heading3:      %d\n", platform->source_heading3);
      fprintf(stderr, "dbg2       platform->source_rollpitch:     %d\n", platform->source_rollpitch);
      fprintf(stderr, "dbg2       platform->source_rollpitch1:    %d\n", platform->source_rollpitch1);
      fprintf(stderr, "dbg2       platform->source_rollpitch2:    %d\n", platform->source_rollpitch2);
      fprintf(stderr, "dbg2       platform->source_rollpitch3:    %d\n", platform->source_rollpitch3);
      fprintf(stderr, "dbg2       platform->source_heave:         %d\n", platform->source_heave);
      fprintf(stderr, "dbg2       platform->source_heave1:        %d\n", platform->source_heave1);
      fprintf(stderr, "dbg2       platform->source_heave2:        %d\n", platform->source_heave2);
      fprintf(stderr, "dbg2       platform->source_heave3:        %d\n", platform->source_heave3);
      fprintf(stderr, "dbg2       platform->num_sensors:          %d\n", platform->num_sensors);
      for (int i = 0; i < platform->num_sensors; i++) {
        fprintf(stderr, "dbg2       platform->sensors[%2d].type:                 %d\n", i, platform->sensors[i].type);
        fprintf(stderr, "dbg2       platform->sensors[%2d].model:                %s\n", i, platform->sensors[i].model);
        fprintf(stderr, "dbg2       platform->sensors[%2d].manufacturer:         %s\n", i,
                platform->sensors[i].manufacturer);
        fprintf(stderr, "dbg2       platform->sensors[%2d].serialnumber:         %s\n", i,
                platform->sensors[i].serialnumber);
        fprintf(stderr, "dbg2       platform->sensors[%2d].capability1:          %d\n", i,
                platform->sensors[i].capability1);
        fprintf(stderr, "dbg2       platform->sensors[%2d].capability2:          %d\n", i,
                platform->sensors[i].capability2);
        fprintf(stderr, "dbg2       platform->sensors[%2d].num_offsets:          %d\n", i,
                platform->sensors[i].num_offsets);
        for (int j = 0; j < platform->sensors[i].num_offsets; j++) {
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_mode:          %d\n", i, j,
                  platform->sensors[i].offsets[j].position_offset_mode);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_x:          %f\n", i, j,
                  platform->sensors[i].offsets[j].position_offset_x);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_y:          %f\n", i, j,
                  platform->sensors[i].offsets[j].position_offset_y);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].position_offset_z:          %f\n", i, j,
                  platform->sensors[i].offsets[j].position_offset_z);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_mode:          %d\n", i, j,
                  platform->sensors[i].offsets[j].attitude_offset_mode);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_heading:      %f\n", i, j,
                  platform->sensors[i].offsets[j].attitude_offset_heading);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_roll:          %f\n", i, j,
                  platform->sensors[i].offsets[j].attitude_offset_roll);
          fprintf(stderr, "dbg2       platform->sensors[%2d].offsets[%d].attitude_offset_pitch:      %f\n", i, j,
                  platform->sensors[i].offsets[j].attitude_offset_pitch);
        }
        fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency_mode:  %d\n", i,
                platform->sensors[i].time_latency_mode);
        fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency_static:  %f\n", i,
                platform->sensors[i].time_latency_static);
        fprintf(stderr, "dbg2       platform->sensors[%2d].num_time_latency:    %d\n", i,
                platform->sensors[i].num_time_latency);
        for (int j = 0; j < platform->sensors[i].num_time_latency; j++) {
          fprintf(stderr, "dbg2       platform->sensors[%2d].time_latency[%2d]:    %16.6f %8.6f\n", i, j,
                  platform->sensors[i].time_latency_time_d[j], platform->sensors[i].time_latency_value[j]);
        }
      }
    }
  }

  /* null platform pointer is an error */
  else {
    status = MB_FAILURE;
    *error = MB_ERROR_BAD_DESCRIPTOR;
  }

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:      %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
