/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_reson7k.c  3.00  3/23/2004
 *
 *    Copyright (c) 2004-2023 by
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
 * mbsys_reson7k.c contains the MBIO functions for handling data from
 * Reson SeaBat 7k series sonars.
 * The data formats associated with Reson SeaBat 7k multibeams
 * include:
 *    MBSYS_RESON7K formats (code in mbsys_reson7k.c and mbsys_reson7k.h):
 *      MBF_RESON7KR : MBIO ID 191 - Raw vendor format
 *      MBF_RESON7KP : MBIO ID 192 - Full processed data
 *      MBF_RESON7KP : MBIO ID 193 - Stripped processed data
 *
 * Author:  D. W. Caress
 * Date:  March 23, 2004
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_segy.h"
#include "mb_status.h"
#include "mbsys_reson7k.h"

/* turn on debug statements here */
/* #define MSYS_RESON7KR_DEBUG 1 */

static const char debug_str[] = "dbg2  ";
static const char nodebug_str[] = "  ";

/*--------------------------------------------------------------------*/
int mbsys_reson7k_zero7kheader(int verbose, s7k_header *header, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
  }

  /* Reson 7k data record header information */
  header->Version = 0;
  header->Offset = 0;
  header->SyncPattern = 0;
  header->OffsetToOptionalData = 0;
  header->OptionalDataIdentifier = 0;
  header->s7kTime.Year = 0;
  header->s7kTime.Day = 0;
  header->s7kTime.Seconds = 0.0;
  header->s7kTime.Hours = 0;
  header->s7kTime.Minutes = 0;
  header->Reserved = 0;
  header->RecordType = 0;
  header->DeviceId = 0;
  header->Reserved2 = 0;
  header->SystemEnumerator = 0;
  header->DataSetNumber = 0;
  header->RecordNumber = 0;
  for (int i = 0; i < 8; i++) {
    header->PreviousRecord[i] = 0;
    header->NextRecord[i] = 0;
  }
  header->Flags = 0;
  header->Reserved3 = 0;
  header->Reserved4 = 0;
  header->FragmentedTotal = 0;
  header->FragmentNumber = 0;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* allocate memory for data structure */
  const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_reson7k_struct), (void **)store_ptr, error);

  /* get data structure pointer */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)*store_ptr;

  /* initialize everything */

  /* Type of data record */
  store->kind = MB_DATA_NONE;
  store->type = R7KRECID_None;

  /* ping record id's */
  store->current_ping_number = -1;
  store->read_volatilesettings = false;
  store->read_matchfilter = false;
  store->read_beamgeometry = false;
  store->read_bathymetry = false;
  store->read_backscatter = false;
  store->read_beam = false;
  store->read_verticaldepth = false;
  store->read_tvg = false;
  store->read_image = false;
  store->read_v2pingmotion = false;
  store->read_v2detectionsetup = false;
  store->read_v2beamformed = false;
  store->read_v2detection = false;
  store->read_v2rawdetection = false;
  store->read_v2snippet = false;
  store->read_calibratedsnippet = false;
  store->read_processedsidescan = false;

  /* MB-System time stamp */
  store->time_d = 0;
  for (int i = 0; i < 7; i++)
    store->time_i[i] = 0;

  /* Reference point information (record 1000) */
  s7kr_reference *reference = &store->reference;
  mbsys_reson7k_zero7kheader(verbose, &reference->header, error);
  reference->offset_x = 0.0;
  reference->offset_y = 0.0;
  reference->offset_z = 0.0;
  reference->water_z = 0.0;

  /* Sensor uncalibrated offset position information (record 1001) */
  s7kr_sensoruncal *sensoruncal = &store->sensoruncal;
  mbsys_reson7k_zero7kheader(verbose, &sensoruncal->header, error);
  sensoruncal->offset_x = 0.0;
  sensoruncal->offset_y = 0.0;
  sensoruncal->offset_z = 0.0;
  sensoruncal->offset_roll = 0.0;
  sensoruncal->offset_pitch = 0.0;
  sensoruncal->offset_yaw = 0.0;

  /* Sensor calibrated offset position information (record 1002) */
  s7kr_sensorcal *sensorcal = &store->sensorcal;
  mbsys_reson7k_zero7kheader(verbose, &sensorcal->header, error);
  sensorcal->offset_x = 0.0;
  sensorcal->offset_y = 0.0;
  sensorcal->offset_z = 0.0;
  sensorcal->offset_roll = 0.0;
  sensorcal->offset_pitch = 0.0;
  sensorcal->offset_yaw = 0.0;

  /* Position (record 1003) */
  s7kr_position *position = &store->position;
  mbsys_reson7k_zero7kheader(verbose, &position->header, error);
  position->datum = 0;
  position->latency = 0.0;
  position->latitude = 0.0;
  position->longitude = 0.0;
  position->height = 0.0;
  position->type = 0;
  position->utm_zone = 0;
  position->quality = 0;
  position->method = 0;

  /* Custom attitude (record 1004) */
  s7kr_customattitude *customattitude = &store->customattitude;
  mbsys_reson7k_zero7kheader(verbose, &customattitude->header, error);
  customattitude->bitfield = 0;
  customattitude->reserved = 0;
  customattitude->n = 0;
  customattitude->frequency = 0;
  customattitude->nalloc = 0;
  customattitude->pitch = NULL;
  customattitude->roll = NULL;
  customattitude->heading = NULL;
  customattitude->heave = NULL;
  customattitude->pitchrate = NULL;
  customattitude->rollrate = NULL;
  customattitude->headingrate = NULL;
  customattitude->heaverate = NULL;

  /* Tide (record 1005) */
  s7kr_tide *tide = &store->tide;
  mbsys_reson7k_zero7kheader(verbose, &tide->header, error);
  tide->tide = 0.0;
  tide->source = 0;
  tide->flags = 0;
  tide->gauge = 0;
  tide->datum = 0;
  tide->latency = 0.0;
  tide->latitude = 0.0;
  tide->longitude = 0.0;
  tide->height = 0.0;
  tide->type = 0;
  tide->utm_zone = 0;

  /* Altitude (record 1006) */
  s7kr_altitude *altitude = &store->altitude;
  mbsys_reson7k_zero7kheader(verbose, &altitude->header, error);
  altitude->altitude = 0.0;

  /* Motion over ground (record 1007) */
  s7kr_motion *motion = &store->motion;
  mbsys_reson7k_zero7kheader(verbose, &motion->header, error);
  motion->bitfield = 0;
  motion->reserved = 0;
  motion->n = 0;
  motion->frequency = 0;
  motion->nalloc = 0;
  motion->x = NULL;
  motion->y = NULL;
  motion->z = NULL;
  motion->xa = NULL;
  motion->ya = NULL;
  motion->za = NULL;

  /* Depth (record 1008) */
  s7kr_depth *depth = &store->depth;
  mbsys_reson7k_zero7kheader(verbose, &depth->header, error);
  depth->descriptor = 0;
  depth->correction = 0;
  depth->reserved = 0;
  depth->depth = 0.0;

  /* Sound velocity profile (record 1009) */
  s7kr_svp *svp = &store->svp;
  mbsys_reson7k_zero7kheader(verbose, &svp->header, error);
  svp->position_flag = 0;
  svp->reserved1 = 0;
  svp->reserved2 = 0;
  svp->latitude = 0.0;
  svp->longitude = 0.0;
  svp->n = 0;
  svp->nalloc = 0;
  svp->depth = NULL;
  svp->sound_velocity = NULL;

  /* CTD (record 1010) */
  s7kr_ctd *ctd = &store->ctd;
  mbsys_reson7k_zero7kheader(verbose, &ctd->header, error);
  ctd->frequency = 0.0;
  ctd->velocity_source_flag = 0;
  ctd->velocity_algorithm = 0;
  ctd->conductivity_flag = 0;
  ctd->pressure_flag = 0;
  ctd->position_flag = 0;
  ctd->validity = 0;
  ctd->reserved = 0;
  ctd->latitude = 0.0;
  ctd->longitude = 0.0;
  ctd->sample_rate = 0.0;
  ctd->n = 0;
  ctd->nalloc = 0;
  ctd->conductivity_salinity = NULL;
  ctd->temperature = NULL;
  ctd->pressure_depth = NULL;
  ctd->sound_velocity = NULL;
  ctd->absorption = NULL;

  /* Geodesy (record 1011) */
  s7kr_geodesy *geodesy = &store->geodesy;
  mbsys_reson7k_zero7kheader(verbose, &geodesy->header, error);
  for (int i = 0; i < 32; i++)
    geodesy->spheroid[i] = '\0';
  geodesy->semimajoraxis = 0.0;
  geodesy->flattening = 0.0;
  for (int i = 0; i < 16; i++)
    geodesy->reserved1[i] = '\0';
  for (int i = 0; i < 32; i++)
    geodesy->datum[i] = '\0';
  geodesy->calculation_method = 0;
  geodesy->number_parameters = 0;
  geodesy->dx = 0.0;
  geodesy->dy = 0.0;
  geodesy->dz = 0.0;
  geodesy->rx = 0.0;
  geodesy->ry = 0.0;
  geodesy->rz = 0.0;
  geodesy->scale = 0.0;
  for (int i = 0; i < 35; i++)
    geodesy->reserved2[i] = '\0';
  for (int i = 0; i < 32; i++)
    geodesy->grid_name[i] = '\0';
  geodesy->distance_units = 0;
  geodesy->angular_units = 0;
  geodesy->latitude_origin = 0.0;
  geodesy->central_meriidan = 0.0;
  geodesy->false_easting = 0.0;
  geodesy->false_northing = 0.0;
  geodesy->central_scale_factor = 0.0;
  geodesy->custum_identifier = 0;
  for (int i = 0; i < 50; i++)
    geodesy->reserved3[i] = '\0';

  /* Roll pitch heave (record 1012) */
  s7kr_rollpitchheave *rollpitchheave = &store->rollpitchheave;
  mbsys_reson7k_zero7kheader(verbose, &rollpitchheave->header, error);
  rollpitchheave->roll = 0.0;
  rollpitchheave->pitch = 0.0;
  rollpitchheave->heave = 0.0;

  /* Heading (record 1013) */
  s7kr_heading *heading = &store->heading;
  mbsys_reson7k_zero7kheader(verbose, &heading->header, error);
  heading->heading = 0.0;

  /* Survey Line (record 1014) */
  s7kr_surveyline *surveyline = &store->surveyline;
  mbsys_reson7k_zero7kheader(verbose, &surveyline->header, error);
  surveyline->n = 0;
  surveyline->type = 0;
  surveyline->turnradius = 0.0;
  for (int i = 0; i < 64; i++)
    surveyline->name[i] = '\0';
  surveyline->nalloc = 0;
  surveyline->latitude = NULL;
  surveyline->longitude = NULL;

  /* Navigation (record 1015) */
  s7kr_navigation *navigation = &store->navigation;
  mbsys_reson7k_zero7kheader(verbose, &navigation->header, error);
  navigation->vertical_reference = 0;
  navigation->latitude = 0.0;
  navigation->longitude = 0.0;
  navigation->position_accuracy = 0.0;
  navigation->height = 0.0;
  navigation->height_accuracy = 0.0;
  navigation->speed = 0.0;
  navigation->course = 0.0;
  navigation->heading = 0.0;

  /* Attitude (record 1016) */
  s7kr_attitude *attitude = &store->attitude;
  mbsys_reson7k_zero7kheader(verbose, &attitude->header, error);
  attitude->n = 0;
  attitude->nalloc = 0;
  attitude->delta_time = NULL;
  attitude->roll = NULL;
  attitude->pitch = NULL;
  attitude->heave = NULL;
  attitude->heading = NULL;

  /* Edgetech FS-DW low frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsslo = &store->fsdwsslo;
  mbsys_reson7k_zero7kheader(verbose, &fsdwsslo->header, error);
  fsdwsslo->msec_timestamp = 0;
  fsdwsslo->ping_number = 0;
  fsdwsslo->number_channels = 0;
  fsdwsslo->total_bytes = 0;
  fsdwsslo->data_format = 0;
  for (int i = 0; i < 2; i++) {
    fsdwsslo->channel[i].number = 0;
    fsdwsslo->channel[i].type = 0;
    fsdwsslo->channel[i].data_type = 0;
    fsdwsslo->channel[i].polarity = 0;
    fsdwsslo->channel[i].bytespersample = 0;
    fsdwsslo->channel[i].reserved1[0] = 0;
    fsdwsslo->channel[i].reserved1[1] = 0;
    fsdwsslo->channel[i].reserved1[2] = 0;
    fsdwsslo->channel[i].number_samples = 0;
    fsdwsslo->channel[i].start_time = 0;
    fsdwsslo->channel[i].sample_interval = 0;
    fsdwsslo->channel[i].range = 0.0;
    fsdwsslo->channel[i].voltage = 0.0;
    for (int j = 0; j < 16; j++)
      fsdwsslo->channel[i].name[j] = '\0';
    for (int j = 0; j < 20; j++)
      fsdwsslo->channel[i].reserved2[j] = 0;
    fsdwsslo->channel[i].data_alloc = 0;
    fsdwsslo->channel[i].data = NULL;
  }

  /* Edgetech FS-DW high frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsshi = &store->fsdwsshi;
  mbsys_reson7k_zero7kheader(verbose, &fsdwsshi->header, error);
  fsdwsshi->msec_timestamp = 0;
  fsdwsshi->ping_number = 0;
  fsdwsshi->number_channels = 0;
  fsdwsshi->total_bytes = 0;
  fsdwsshi->data_format = 0;
  for (int i = 0; i < 2; i++) {
    fsdwsshi->channel[i].number = 0;
    fsdwsshi->channel[i].type = 0;
    fsdwsshi->channel[i].data_type = 0;
    fsdwsshi->channel[i].polarity = 0;
    fsdwsshi->channel[i].bytespersample = 0;
    fsdwsshi->channel[i].reserved1[0] = 0;
    fsdwsshi->channel[i].reserved1[1] = 0;
    fsdwsshi->channel[i].reserved1[2] = 0;
    fsdwsshi->channel[i].number_samples = 0;
    fsdwsshi->channel[i].start_time = 0;
    fsdwsshi->channel[i].sample_interval = 0;
    fsdwsshi->channel[i].range = 0.0;
    fsdwsshi->channel[i].voltage = 0.0;
    for (int j = 0; j < 16; j++)
      fsdwsshi->channel[i].name[j] = '\0';
    for (int j = 0; j < 20; j++)
      fsdwsshi->channel[i].reserved2[j] = 0;
    fsdwsshi->channel[i].data_alloc = 0;
    fsdwsshi->channel[i].data = NULL;
  }

  /* Edgetech FS-DW subbottom (record 3001) */
  s7kr_fsdwsb *fsdwsb = &store->fsdwsb;
  mbsys_reson7k_zero7kheader(verbose, &fsdwsb->header, error);
  fsdwsb->msec_timestamp = 0;
  fsdwsb->ping_number = 0;
  fsdwsb->number_channels = 0;
  fsdwsb->total_bytes = 0;
  fsdwsb->data_format = 0;
  fsdwsb->channel.number = 0;
  fsdwsb->channel.type = 0;
  fsdwsb->channel.data_type = 0;
  fsdwsb->channel.polarity = 0;
  fsdwsb->channel.bytespersample = 0;
  fsdwsb->channel.reserved1[0] = 0;
  fsdwsb->channel.reserved1[1] = 0;
  fsdwsb->channel.reserved1[2] = 0;
  fsdwsb->channel.number_samples = 0;
  fsdwsb->channel.start_time = 0;
  fsdwsb->channel.sample_interval = 0;
  fsdwsb->channel.range = 0.0;
  fsdwsb->channel.voltage = 0.0;
  for (int j = 0; j < 16; j++)
    fsdwsb->channel.name[j] = '\0';
  for (int j = 0; j < 20; j++)
    fsdwsb->channel.reserved2[j] = '\0';
  fsdwsb->channel.data_alloc = 0;
  fsdwsb->channel.data = NULL;

  /* Bluefin data frames (record 3100) */
  s7kr_bluefin *bluefin = &store->bluefin;
  mbsys_reson7k_zero7kheader(verbose, &bluefin->header, error);
  bluefin->msec_timestamp = 0;
  bluefin->number_frames = 0;
  bluefin->frame_size = 0;
  bluefin->data_format = 0;
  for (int i = 0; i < 16; i++)
    bluefin->reserved[i] = 0;
  for (int i = 0; i < BLUEFIN_MAX_FRAMES; i++) {
    bluefin->nav[i].packet_size = 0;
    bluefin->nav[i].version = 0;
    bluefin->nav[i].offset = 0;
    bluefin->nav[i].data_type = 0;
    bluefin->nav[i].data_size = 0;
    bluefin->nav[i].s7kTime.Year = 0;
    bluefin->nav[i].s7kTime.Day = 0;
    bluefin->nav[i].s7kTime.Seconds = 0.0;
    bluefin->nav[i].s7kTime.Hours = 0;
    bluefin->nav[i].s7kTime.Minutes = 0;
    bluefin->nav[i].checksum = 0;
    bluefin->nav[i].timedelay = 0;
    bluefin->nav[i].quality = 0;
    bluefin->nav[i].latitude = 0.0;
    bluefin->nav[i].longitude = 0.0;
    bluefin->nav[i].speed = 0.0;
    bluefin->nav[i].depth = 0.0;
    bluefin->nav[i].altitude = 0.0;
    bluefin->nav[i].roll = 0.0;
    bluefin->nav[i].pitch = 0.0;
    bluefin->nav[i].yaw = 0.0;
    bluefin->nav[i].northing_rate = 0.0;
    bluefin->nav[i].easting_rate = 0.0;
    bluefin->nav[i].depth_rate = 0.0;
    bluefin->nav[i].altitude_rate = 0.0;
    bluefin->nav[i].roll_rate = 0.0;
    bluefin->nav[i].pitch_rate = 0.0;
    bluefin->nav[i].yaw_rate = 0.0;
    bluefin->nav[i].position_time = 0.0;
    bluefin->nav[i].depth_time = 0.0;
    bluefin->environmental[i].packet_size = 0;
    bluefin->environmental[i].version = 0;
    bluefin->environmental[i].offset = 0;
    bluefin->environmental[i].data_type = 0;
    bluefin->environmental[i].data_size = 0;
    bluefin->environmental[i].s7kTime.Year = 0;
    bluefin->environmental[i].s7kTime.Day = 0;
    bluefin->environmental[i].s7kTime.Seconds = 0.0;
    bluefin->environmental[i].s7kTime.Hours = 0;
    bluefin->environmental[i].s7kTime.Minutes = 0;
    bluefin->environmental[i].checksum = 0;
    bluefin->environmental[i].reserved1 = 0;
    bluefin->environmental[i].quality = 0;
    bluefin->environmental[i].sound_speed = 0.0;
    bluefin->environmental[i].conductivity = 0.0;
    bluefin->environmental[i].temperature = 0.0;
    bluefin->environmental[i].pressure = 0.0;
    bluefin->environmental[i].salinity = 0.0;
    bluefin->environmental[i].ctd_time = 0.0;
    bluefin->environmental[i].temperature_time = 0.0;
    bluefin->environmental[i].surface_pressure = 0.0;
    bluefin->environmental[i].temperature_counts = 0;
    bluefin->environmental[i].conductivity_frequency = 0.0;
    bluefin->environmental[i].pressure_counts = 0;
    bluefin->environmental[i].pressure_comp_voltage = 0.0;
    bluefin->environmental[i].sensor1 = 0;
    bluefin->environmental[i].sensor2 = 0;
    bluefin->environmental[i].sensor3 = 0;
    bluefin->environmental[i].sensor4 = 0;
    bluefin->environmental[i].sensor5 = 0;
    bluefin->environmental[i].sensor6 = 0;
    bluefin->environmental[i].sensor7 = 0;
    bluefin->environmental[i].sensor8 = 0;
    for (int j = 0; j < 8; j++)
      bluefin->environmental[i].reserved2[j] = 0;
  }

  /* Processed sidescan - MB-System extension to 7k format (record 3199) */
  s7kr_processedsidescan *processedsidescan = &store->processedsidescan;
  mbsys_reson7k_zero7kheader(verbose, &processedsidescan->header, error);
  processedsidescan->serial_number = 0;
  processedsidescan->ping_number = 0;
  processedsidescan->multi_ping = 0;
  processedsidescan->recordversion = 0;
  processedsidescan->ss_source = 0;
  processedsidescan->number_pixels = 0;
  processedsidescan->ss_type = 0;
  processedsidescan->pixelwidth = 0;
  processedsidescan->sensordepth = 0;
  processedsidescan->altitude = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
    processedsidescan->sidescan[i] = 0.0;
    processedsidescan->alongtrack[i] = 0.0;
  }

  /* Reson 7k volatile sonar settings (record 7000) */
  s7kr_volatilesettings *volatilesettings = &store->volatilesettings;
  mbsys_reson7k_zero7kheader(verbose, &volatilesettings->header, error);
  volatilesettings->serial_number = 0;
  volatilesettings->ping_number = 0;
  volatilesettings->multi_ping = 0;
  volatilesettings->frequency = 0.0;
  volatilesettings->sample_rate = 0.0;
  volatilesettings->receiver_bandwidth = 0.0;
  volatilesettings->pulse_width = 0.0;
  volatilesettings->pulse_type = 0;
  volatilesettings->pulse_envelope = 0;
  volatilesettings->pulse_envelope_par = 0.0;
  volatilesettings->pulse_reserved = 0;
  volatilesettings->max_ping_rate = 0.0;
  volatilesettings->ping_period = 0.0;
  volatilesettings->range_selection = 0.0;
  volatilesettings->power_selection = 0.0;
  volatilesettings->gain_selection = 0.0;
  volatilesettings->control_flags = 0;
  volatilesettings->projector_magic_no = 0;
  volatilesettings->steering_vertical = 0.0;
  volatilesettings->steering_horizontal = 0.0;
  volatilesettings->beamwidth_vertical = 0.0;
  volatilesettings->beamwidth_horizontal = 0.0;
  volatilesettings->focal_point = 0.0;
  volatilesettings->projector_weighting = 0;
  volatilesettings->projector_weighting_par = 0.0;
  volatilesettings->transmit_flags = 0;
  volatilesettings->hydrophone_magic_no = 0;
  volatilesettings->receive_weighting = 0;
  volatilesettings->receive_weighting_par = 0.0;
  volatilesettings->receive_flags = 0;
  volatilesettings->receive_width = 0.0;
  volatilesettings->range_minimum = 0.0;
  volatilesettings->range_maximum = 0.0;
  volatilesettings->depth_minimum = 0.0;
  volatilesettings->depth_maximum = 0.0;
  volatilesettings->absorption = 0.0;
  volatilesettings->sound_velocity = 0.0;
  volatilesettings->spreading = 0.0;
  volatilesettings->reserved = 0;

  /* Reson 7k configuration (record 7001) */
  s7kr_configuration *configuration;
  configuration = &store->configuration;
  mbsys_reson7k_zero7kheader(verbose, &configuration->header, error);
  configuration->serial_number = 0;
  configuration->number_devices = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_DEVICE; i++) {
    configuration->device[i].magic_number = 0;
    for (int j = 0; j < 16; j++)
      configuration->device[i].description[j] = '\0';
    configuration->device[i].serial_number = 0;
    configuration->device[i].info_length = 0;
    configuration->device[i].info_alloc = 0;
    configuration->device[i].info = NULL;
  }

  /* Reson 7k match filter (record 7002) */
  s7kr_matchfilter *matchfilter = &store->matchfilter;
  mbsys_reson7k_zero7kheader(verbose, &matchfilter->header, error);
  matchfilter->serial_number = 0;
  matchfilter->ping_number = 0;
  matchfilter->operation = 0;
  matchfilter->start_frequency = 0.0;
  matchfilter->end_frequency = 0.0;

  /* Reson 7k firmware and hardware configuration (record 7003) */
  s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration;
  v2firmwarehardwareconfiguration = &store->v2firmwarehardwareconfiguration;
  mbsys_reson7k_zero7kheader(verbose, &v2firmwarehardwareconfiguration->header, error);
  v2firmwarehardwareconfiguration->device_count = 0;
  v2firmwarehardwareconfiguration->info_length = 0;
  v2firmwarehardwareconfiguration->info_alloc = 0;
  v2firmwarehardwareconfiguration->info = NULL;

  /* Reson 7k beam geometry (record 7004) */
  s7kr_beamgeometry *beamgeometry = &store->beamgeometry;
  mbsys_reson7k_zero7kheader(verbose, &beamgeometry->header, error);
  beamgeometry->serial_number = 0;
  beamgeometry->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    beamgeometry->angle_alongtrack[i] = 0.0;
    beamgeometry->angle_acrosstrack[i] = 0.0;
    beamgeometry->beamwidth_alongtrack[i] = 0.0;
    beamgeometry->beamwidth_acrosstrack[i] = 0.0;
  }

  /* Reson 7k calibration data (record 7005) */
  s7kr_calibration *calibration = &store->calibration;
  mbsys_reson7k_zero7kheader(verbose, &calibration->header, error);
  calibration->serial_number = 0;
  calibration->number_channels = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_RECEIVERS; i++) {
    calibration->gain[i] = 0.0;
    calibration->phase[i] = 0.0;
  }

  /* Reson 7k bathymetry (record 7006) */
  s7kr_bathymetry *bathymetry = &store->bathymetry;
  mbsys_reson7k_zero7kheader(verbose, &bathymetry->header, error);
  bathymetry->serial_number = 0;
  bathymetry->ping_number = 0;
  bathymetry->multi_ping = 0;
  bathymetry->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    bathymetry->range[i] = 0.0;
    bathymetry->quality[i] = 0;
    bathymetry->intensity[i] = 0.0;
    bathymetry->min_depth_gate[i] = 0.0;
    bathymetry->max_depth_gate[i] = 0.0;
  }
  bathymetry->optionaldata = false;
  bathymetry->frequency = 0.0;
  bathymetry->latitude = 0.0;
  bathymetry->longitude = 0.0;
  bathymetry->heading = 0.0;
  bathymetry->height_source = 0;
  bathymetry->tide = 0.0;
  bathymetry->roll = 0.0;
  bathymetry->pitch = 0.0;
  bathymetry->heave = 0.0;
  bathymetry->vehicle_height = 0.0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    bathymetry->depth[i] = 0.0;
    bathymetry->alongtrack[i] = 0.0;
    bathymetry->acrosstrack[i] = 0.0;
    bathymetry->pointing_angle[i] = 0.0;
    bathymetry->azimuth_angle[i] = 0.0;
  }
  bathymetry->acrossalongerror = MB_MAYBE;
  bathymetry->nacrossalongerroryes = 0;
  bathymetry->nacrossalongerrorno = 0;

  /* Reson 7k backscatter imagery data (record 7007) */
  s7kr_backscatter *backscatter = &store->backscatter;
  mbsys_reson7k_zero7kheader(verbose, &backscatter->header, error);
  backscatter->serial_number = 0;
  backscatter->ping_number = 0;
  backscatter->multi_ping = 0;
  backscatter->beam_position = 0.0;
  backscatter->control_flags = 0;
  backscatter->number_samples = 0;
  backscatter->port_beamwidth_x = 0.0;
  backscatter->port_beamwidth_y = 0.0;
  backscatter->stbd_beamwidth_x = 0.0;
  backscatter->stbd_beamwidth_y = 0.0;
  backscatter->port_steering_x = 0.0;
  backscatter->port_steering_y = 0.0;
  backscatter->stbd_steering_x = 0.0;
  backscatter->stbd_steering_y = 0.0;
  backscatter->number_beams = 0;
  backscatter->current_beam = 0;
  backscatter->sample_size = 0;
  backscatter->data_type = 0;
  backscatter->nalloc = 0;
  backscatter->port_data = NULL;
  backscatter->stbd_data = NULL;
  backscatter->optionaldata = false;
  backscatter->frequency = 0.0;
  backscatter->latitude = 0.0;
  backscatter->longitude = 0.0;
  backscatter->heading = 0.0;
  backscatter->altitude = 0.0;

  /* Reson 7k beam data (record 7008) */
  s7kr_beam *beam = &store->beam;
  mbsys_reson7k_zero7kheader(verbose, &beam->header, error);
  beam->serial_number = 0;
  beam->ping_number = 0;
  beam->multi_ping = 0;
  beam->number_beams = 0;
  beam->reserved = 0;
  beam->number_samples = 0;
  beam->record_subset_flag = 0;
  beam->row_column_flag = 0;
  beam->sample_header_id = 0;
  beam->sample_type = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_RECEIVERS; i++) {
    beam->snippets[i].beam_number = 0;
    beam->snippets[i].begin_sample = 0;
    beam->snippets[i].end_sample = 0;
    beam->snippets[i].nalloc_amp = 0;
    beam->snippets[i].nalloc_phase = 0;
    beam->snippets[i].amplitude = NULL;
    beam->snippets[i].phase = NULL;
  }

  /* Reson 7k vertical depth (record 7009) */
  s7kr_verticaldepth *verticaldepth = &store->verticaldepth;
  mbsys_reson7k_zero7kheader(verbose, &verticaldepth->header, error);
  verticaldepth->frequency = 0.0;
  verticaldepth->ping_number = 0;
  verticaldepth->multi_ping = 0;
  verticaldepth->latitude = 0.0;
  verticaldepth->longitude = 0.0;
  verticaldepth->heading = 0.0;
  verticaldepth->alongtrack = 0.0;
  verticaldepth->acrosstrack = 0.0;
  verticaldepth->vertical_depth = 0;

  /* Reson 7k tvg data (record 7010) */
  s7kr_tvg *tvg = &store->tvg;
  mbsys_reson7k_zero7kheader(verbose, &tvg->header, error);
  tvg->serial_number = 0;
  tvg->ping_number = 0;
  tvg->multi_ping = 0;
  tvg->n = 0;
  for (int i = 0; i < 8; i++)
    tvg->reserved[i] = 0;
  tvg->nalloc = 0;
  tvg->tvg = NULL;

  /* Reson 7k image data (record 7011) */
  s7kr_image *image = &store->image;
  mbsys_reson7k_zero7kheader(verbose, &image->header, error);
  image->ping_number = 0;
  image->multi_ping = 0;
  image->width = 0;
  image->height = 0;
  image->color_depth = 0;
  image->width_height_flag = 0;
  image->compression = 0;
  image->nalloc = 0;
  image->image = NULL;

  /* Ping motion (record 7012) */
  s7kr_v2pingmotion *v2pingmotion = &store->v2pingmotion;
  mbsys_reson7k_zero7kheader(verbose, &v2pingmotion->header, error);
  v2pingmotion->serial_number = 0;
  v2pingmotion->ping_number = 0;
  v2pingmotion->multi_ping = 0;
  v2pingmotion->n = 0;
  v2pingmotion->flags = 0;
  v2pingmotion->error_flags = 0;
  v2pingmotion->frequency = 0.0;
  v2pingmotion->nalloc = 0;
  v2pingmotion->pitch = 0.0;
  v2pingmotion->roll = NULL;
  v2pingmotion->heading = NULL;
  v2pingmotion->heave = NULL;

  /* Detection setup (record 7017) */
  s7kr_v2detectionsetup *v2detectionsetup = &store->v2detectionsetup;
  mbsys_reson7k_zero7kheader(verbose, &v2detectionsetup->header, error);
  v2detectionsetup->serial_number = 0;
  v2detectionsetup->ping_number = 0;
  v2detectionsetup->multi_ping = 0;
  v2detectionsetup->number_beams = 0;
  v2detectionsetup->data_field_size = 0;
  v2detectionsetup->detection_algorithm = 0;
  v2detectionsetup->detection_flags = 0;
  v2detectionsetup->minimum_depth = 0.0;
  v2detectionsetup->maximum_depth = 0.0;
  v2detectionsetup->minimum_range = 0.0;
  v2detectionsetup->maximum_range = 0.0;
  v2detectionsetup->minimum_nadir_search = 0.0;
  v2detectionsetup->maximum_nadir_search = 0.0;
  v2detectionsetup->automatic_filter_window = 0.0;
  v2detectionsetup->applied_roll = 0.0;
  v2detectionsetup->depth_gate_tilt = 0.0;
  for (int i = 0; i < 14; i++)
    v2detectionsetup->reserved[i] = 0.0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    v2detectionsetup->beam_descriptor[i] = 0;
    v2detectionsetup->detection_point[i] = 0.0;
    v2detectionsetup->flags[i] = 0;
    v2detectionsetup->auto_limits_min_sample[i] = 0;
    v2detectionsetup->auto_limits_max_sample[i] = 0;
    v2detectionsetup->user_limits_min_sample[i] = 0;
    v2detectionsetup->user_limits_max_sample[i] = 0;
    v2detectionsetup->quality[i] = 0;
    v2detectionsetup->uncertainty[i] = 0;
  }

  /* Reson 7k beamformed magnitude and phase data (record 7018) */
  s7kr_v2beamformed *v2beamformed = &store->v2beamformed;
  mbsys_reson7k_zero7kheader(verbose, &v2beamformed->header, error);
  v2beamformed->serial_number = 0;
  v2beamformed->ping_number = 0;
  v2beamformed->multi_ping = 0;
  v2beamformed->number_beams = 0;
  v2beamformed->number_samples = 0;
  for (int i = 0; i < 32; i++)
    v2beamformed->reserved[i] = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_v2amplitudephase *v2amplitudephase = &v2beamformed->amplitudephase[i];
    v2amplitudephase->beam_number = 0;
    v2amplitudephase->number_samples = 0;
    v2amplitudephase->nalloc = 0;
    v2amplitudephase->amplitude = NULL;
    v2amplitudephase->phase = NULL;
  }

  /* Reson 7k BITE (record 7021) */
  s7kr_v2bite *v2bite = &store->v2bite;
  mbsys_reson7k_zero7kheader(verbose, &v2bite->header, error);
  v2bite->number_reports = 0;
  v2bite->nalloc = 0;
  v2bite->reports = NULL;

  /* Reson 7k center version (record 7022) */
  s7kr_v27kcenterversion *v27kcenterversion = &store->v27kcenterversion;
  mbsys_reson7k_zero7kheader(verbose, &v27kcenterversion->header, error);
  for (int i = 0; i < 32; i++)
    v27kcenterversion->version[i] = 0;

  /* Reson 7k 8k wet end version (record 7023) */
  s7kr_v28kwetendversion *v28kwetendversion = &store->v28kwetendversion;
  mbsys_reson7k_zero7kheader(verbose, &v28kwetendversion->header, error);
  for (int i = 0; i < 32; i++)
    v28kwetendversion->version[i] = 0;

  /* Reson 7k version 2 detection (record 7026) */
  s7kr_v2detection *v2detection = &store->v2detection;
  mbsys_reson7k_zero7kheader(verbose, &v2detection->header, error);
  v2detection->serial_number = 0;
  v2detection->ping_number = 0;
  v2detection->multi_ping = 0;
  v2detection->number_beams = 0;
  v2detection->data_field_size = 0;
  v2detection->corrections = 0;
  v2detection->detection_algorithm = 0;
  v2detection->flags = 0;
  for (int i = 0; i < 64; i++)
    v2detection->reserved[i] = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    v2detection->range[i] = 0.0;
    v2detection->angle_x[i] = 0.0;
    v2detection->angle_y[i] = 0.0;
    v2detection->range_error[i] = 0.0;
    v2detection->angle_x_error[i] = 0.0;
    v2detection->angle_y_error[i] = 0.0;
  }

  /* Reson 7k version 2 raw detection (record 7027) */
  s7kr_v2rawdetection *v2rawdetection = &store->v2rawdetection;
  mbsys_reson7k_zero7kheader(verbose, &v2rawdetection->header, error);
  v2rawdetection->serial_number = 0;
  v2rawdetection->ping_number = 0;
  v2rawdetection->multi_ping = 0;
  v2rawdetection->number_beams = 0;
  v2rawdetection->data_field_size = 0;
  v2rawdetection->detection_algorithm = 0;
  v2rawdetection->detection_flags = 0;
  v2rawdetection->sampling_rate = 0.0;
  v2rawdetection->tx_angle = 0.0;
  for (int i = 0; i < 64; i++)
    v2rawdetection->reserved[i] = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    v2rawdetection->beam_descriptor[i] = 0;
    v2rawdetection->detection_point[i] = 0;
    v2rawdetection->rx_angle[i] = 0;
    v2rawdetection->flags[i] = 0;
    v2rawdetection->quality[i] = 0;
    v2rawdetection->uncertainty[i] = 0;
  }

  /* Reson 7k version 2 snippet (record 7028) */
  s7kr_v2snippet *v2snippet = &store->v2snippet;
  mbsys_reson7k_zero7kheader(verbose, &v2snippet->header, error);
  v2snippet->serial_number = 0;
  v2snippet->ping_number = 0;
  v2snippet->multi_ping = 0;
  v2snippet->number_beams = 0;
  v2snippet->error_flag = 0;
  v2snippet->control_flags = 0;
  for (int i = 0; i < 28; i++)
    v2snippet->reserved[i] = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_v2snippettimeseries *v2snippettimeseries = &v2snippet->snippettimeseries[i];
    v2snippettimeseries->beam_number = 0;
    v2snippettimeseries->begin_sample = 0;
    v2snippettimeseries->detect_sample = 0;
    v2snippettimeseries->end_sample = 0;
    v2snippettimeseries->nalloc = 0;
    v2snippettimeseries->amplitude = NULL;
  }

  /* Reson 7k sonar installation parameters (record 7051) */
  s7kr_installation *installation = &store->installation;
  mbsys_reson7k_zero7kheader(verbose, &installation->header, error);
  installation->frequency = 0.0;
  installation->firmware_version_len = 0;
  for (int i = 0; i < 128; i++)
    installation->firmware_version[i] = 0;
  installation->software_version_len = 0;
  for (int i = 0; i < 128; i++)
    installation->software_version[i] = 0;
  installation->s7k_version_len = 0;
  for (int i = 0; i < 128; i++)
    installation->s7k_version[i] = 0;
  installation->protocal_version_len = 0;
  for (int i = 0; i < 128; i++)
    installation->protocal_version[i] = 0;
  installation->transmit_x = 0.0;
  installation->transmit_y = 0.0;
  installation->transmit_z = 0.0;
  installation->transmit_roll = 0.0;
  installation->transmit_pitch = 0.0;
  installation->transmit_heading = 0.0;
  installation->receive_x = 0.0;
  installation->receive_y = 0.0;
  installation->receive_z = 0.0;
  installation->receive_roll = 0.0;
  installation->receive_pitch = 0.0;
  installation->receive_heading = 0.0;
  installation->motion_x = 0.0;
  installation->motion_y = 0.0;
  installation->motion_z = 0.0;
  installation->motion_roll = 0.0;
  installation->motion_pitch = 0.0;
  installation->motion_heading = 0.0;
  installation->motion_time_delay = 0;
  installation->position_x = 0.0;
  installation->position_y = 0.0;
  installation->position_z = 0.0;
  installation->position_time_delay = 0;
  installation->waterline_z = 0.0;

  /* Reson 7k system event (record 7051) */
  s7kr_systemeventmessage *systemeventmessage = &store->systemeventmessage;
  mbsys_reson7k_zero7kheader(verbose, &systemeventmessage->header, error);
  systemeventmessage->serial_number = 0;
  systemeventmessage->event_id = 0;
  systemeventmessage->message_length = 0;
  systemeventmessage->event_identifier = 0;
  systemeventmessage->message_alloc = 0;
  systemeventmessage->message = NULL;

  /* Reson 7k calibrated snippet (record 7058) */
  s7kr_calibratedsnippet *calibratedsnippet = &store->calibratedsnippet;
  mbsys_reson7k_zero7kheader(verbose, &calibratedsnippet->header, error);
  calibratedsnippet->serial_number = 0;
  calibratedsnippet->ping_number = 0;
  calibratedsnippet->multi_ping = 0;
  calibratedsnippet->number_beams = 0;
  calibratedsnippet->error_flag = 0;
  calibratedsnippet->control_flags = 0;
  for (int i = 0; i < 6; i++)
    calibratedsnippet->reserved[i] = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_calibratedsnippettimeseries *calibratedsnippettimeseries =
        &calibratedsnippet->calibratedsnippettimeseries[i];
    calibratedsnippettimeseries->beam_number = 0;
    calibratedsnippettimeseries->begin_sample = 0;
    calibratedsnippettimeseries->detect_sample = 0;
    calibratedsnippettimeseries->end_sample = 0;
    calibratedsnippettimeseries->nalloc = 0;
    calibratedsnippettimeseries->amplitude = NULL;
  }

  /* Reson 7k file header (record 7200) */
  s7kr_fileheader *fileheader = &store->fileheader;
  mbsys_reson7k_zero7kheader(verbose, &fileheader->header, error);
  for (int i = 0; i < 16; i++)
    fileheader->file_identifier[i] = '\0';
  fileheader->version = 0;
  fileheader->reserved = 0;
  for (int i = 0; i < 16; i++)
    fileheader->session_identifier[i] = '\0';
  fileheader->record_data_size = 0;
  fileheader->number_subsystems = 0;
  for (int i = 0; i < 64; i++)
    fileheader->recording_name[i] = '\0';
  for (int i = 0; i < 16; i++)
    fileheader->recording_version[i] = '\0';
  for (int i = 0; i < 64; i++)
    fileheader->user_defined_name[i] = '\0';
  for (int i = 0; i < 128; i++)
    fileheader->notes[i] = '\0';
  for (int j = 0; j < MBSYS_RESON7K_MAX_DEVICE; j++) {
    fileheader->subsystem[j].device_identifier = 0;
    fileheader->subsystem[j].system_enumerator = 0;
  }

  /* Reson 7k remote control sonar settings (record 7503) */
  s7kr_remotecontrolsettings *remotecontrolsettings =
      &store->remotecontrolsettings;
  mbsys_reson7k_zero7kheader(verbose, &remotecontrolsettings->header, error);
  remotecontrolsettings->serial_number = 0;
  remotecontrolsettings->ping_number = 0;
  remotecontrolsettings->frequency = 0.0;
  remotecontrolsettings->sample_rate = 0.0;
  remotecontrolsettings->receiver_bandwidth = 0.0;
  remotecontrolsettings->pulse_width = 0.0;
  remotecontrolsettings->pulse_type = 0;
  remotecontrolsettings->pulse_envelope = 0;
  remotecontrolsettings->pulse_envelope_par = 0.0;
  remotecontrolsettings->pulse_reserved = 0;
  remotecontrolsettings->max_ping_rate = 0.0;
  remotecontrolsettings->ping_period = 0.0;
  remotecontrolsettings->range_selection = 0.0;
  remotecontrolsettings->power_selection = 0.0;
  remotecontrolsettings->gain_selection = 0.0;
  remotecontrolsettings->control_flags = 0;
  remotecontrolsettings->projector_magic_no = 0;
  remotecontrolsettings->steering_vertical = 0.0;
  remotecontrolsettings->steering_horizontal = 0.0;
  remotecontrolsettings->beamwidth_vertical = 0.0;
  remotecontrolsettings->beamwidth_horizontal = 0.0;
  remotecontrolsettings->focal_point = 0.0;
  remotecontrolsettings->projector_weighting = 0;
  remotecontrolsettings->projector_weighting_par = 0.0;
  remotecontrolsettings->transmit_flags = 0;
  remotecontrolsettings->hydrophone_magic_no = 0;
  remotecontrolsettings->receive_weighting = 0;
  remotecontrolsettings->receive_weighting_par = 0.0;
  remotecontrolsettings->receive_flags = 0;
  remotecontrolsettings->range_minimum = 0.0;
  remotecontrolsettings->range_maximum = 0.0;
  remotecontrolsettings->depth_minimum = 0.0;
  remotecontrolsettings->depth_maximum = 0.0;
  remotecontrolsettings->absorption = 0.0;
  remotecontrolsettings->sound_velocity = 0.0;
  remotecontrolsettings->spreading = 0.0;
  remotecontrolsettings->reserved = 0;
  remotecontrolsettings->tx_offset_x = 0.0;
  remotecontrolsettings->tx_offset_y = 0.0;
  remotecontrolsettings->tx_offset_z = 0.0;
  remotecontrolsettings->head_tilt_x = 0.0;
  remotecontrolsettings->head_tilt_y = 0.0;
  remotecontrolsettings->head_tilt_z = 0.0;
  remotecontrolsettings->ping_on_off = 0;
  remotecontrolsettings->data_sample_types = 0;
  remotecontrolsettings->projector_orientation = 0;
  remotecontrolsettings->beam_angle_mode = 0;
  remotecontrolsettings->r7kcenter_mode = 0;
  remotecontrolsettings->gate_depth_min = 0.0;
  remotecontrolsettings->gate_depth_max = 0.0;
  for (int i = 0; i < 35; i++)
    remotecontrolsettings->reserved2[i] = 0;

  /* Reson 7k remote control sonar settings (record 7503) */
  s7kr_reserved *reserved = &store->reserved;
  mbsys_reson7k_zero7kheader(verbose, &reserved->header, error);
  for (int i = 0; i < R7KHDRSIZE_7kReserved; i++)
    reserved->reserved[i] = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  /* get data structure pointer */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)*store_ptr;

  int status = MB_SUCCESS;

  /* Custom attitude (record 1004) */
  s7kr_customattitude *customattitude = &store->customattitude;
  customattitude->n = 0;
  customattitude->nalloc = 0;
  if (customattitude->pitch != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->pitch), error);
  if (customattitude->roll != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->roll), error);
  if (customattitude->heading != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heading), error);
  if (customattitude->heave != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heave), error);
  if (customattitude->pitchrate != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->pitchrate), error);
  if (customattitude->rollrate != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->rollrate), error);
  if (customattitude->headingrate != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->headingrate), error);
  if (customattitude->heaverate != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(customattitude->heaverate), error);

  /* Motion over ground (record 1007) */
  s7kr_motion *motion = &store->motion;
  motion->n = 0;
  motion->nalloc = 0;
  if (motion->x != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->x), error);
  if (motion->y != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->y), error);
  if (motion->z != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->z), error);
  if (motion->xa != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->xa), error);
  if (motion->ya != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->ya), error);
  if (motion->za != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(motion->za), error);

  /* Sound velocity profile (record 1009) */
  s7kr_svp *svp = &store->svp;
  svp->n = 0;
  svp->nalloc = 0;
  if (svp->depth != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(svp->depth), error);
  if (svp->sound_velocity != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(svp->sound_velocity), error);

  /* CTD (record 1010) */
  s7kr_ctd *ctd = &store->ctd;
  ctd->n = 0;
  ctd->nalloc = 0;
  if (ctd->conductivity_salinity != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->conductivity_salinity), error);
  if (ctd->temperature != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->temperature), error);
  if (ctd->pressure_depth != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->pressure_depth), error);
  if (ctd->sound_velocity != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->sound_velocity), error);
  if (ctd->absorption != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(ctd->absorption), error);

  /* Survey Line (record 1014) */
  s7kr_surveyline *surveyline = &store->surveyline;
  surveyline->n = 0;
  surveyline->nalloc = 0;
  if (surveyline->latitude != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(surveyline->latitude), error);
  if (surveyline->longitude != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(surveyline->longitude), error);

  /* Attitude (record 1016) */
  s7kr_attitude *attitude = &store->attitude;
  attitude->n = 0;
  attitude->nalloc = 0;
  if (attitude->delta_time != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->delta_time), error);
  if (attitude->pitch != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->pitch), error);
  if (attitude->roll != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->roll), error);
  if (attitude->heave != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->heave), error);
  if (attitude->heading != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(attitude->heading), error);

  /* Edgetech FS-DW low frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsslo = &store->fsdwsslo;
  for (int i = 0; i < 2; i++) {
    fsdwsslo->channel[i].data_alloc = 0;
    if (fsdwsslo->channel[i].data != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(fsdwsslo->channel[i].data), error);
  }

  /* Edgetech FS-DW high frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsshi = &store->fsdwsshi;
  for (int i = 0; i < 2; i++) {
    fsdwsshi->channel[i].data_alloc = 0;
    if (fsdwsshi->channel[i].data != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(fsdwsshi->channel[i].data), error);
  }

  /* Edgetech FS-DW subbottom (record 3001) */
  s7kr_fsdwsb *fsdwsb = &store->fsdwsb;
  fsdwsb->channel.data_alloc = 0;
  if (fsdwsb->channel.data != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(fsdwsb->channel.data), error);

  /* Reson 7k configuration (record 7001) */
  s7kr_configuration *configuration = &store->configuration;
  for (int i = 0; i < MBSYS_RESON7K_MAX_DEVICE; i++) {
    configuration->device[i].info_length = 0;
    configuration->device[i].info_alloc = 0;
    if (configuration->device[i].info != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(configuration->device[i].info), error);
  }

  /* Reson 7k firmware and hardware configuration (record 7003) */
  s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration =
      &store->v2firmwarehardwareconfiguration;
  if (v2firmwarehardwareconfiguration->info != NULL && v2firmwarehardwareconfiguration->info_alloc > 0)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2firmwarehardwareconfiguration->info), error);
  v2firmwarehardwareconfiguration->info_length = 0;
  v2firmwarehardwareconfiguration->info_alloc = 0;

  /* Reson 7k backscatter imagery data (record 7007) */
  s7kr_backscatter *backscatter = &store->backscatter;
  backscatter->number_samples = 0;
  backscatter->nalloc = 0;
  if (backscatter->port_data != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(backscatter->port_data), error);
  if (backscatter->stbd_data != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(backscatter->stbd_data), error);

  /* Reson 7k beam data (record 7008) */
  s7kr_beam *beam = &store->beam;
  for (int i = 0; i < MBSYS_RESON7K_MAX_RECEIVERS; i++) {
    beam->snippets[i].begin_sample = 0;
    beam->snippets[i].end_sample = 0;
    beam->snippets[i].nalloc_amp = 0;
    beam->snippets[i].nalloc_phase = 0;
    if (beam->snippets[i].amplitude != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(beam->snippets[i].amplitude), error);
    if (beam->snippets[i].phase != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(beam->snippets[i].phase), error);
  }

  /* Reson 7k tvg data (record 7010) */
  s7kr_tvg *tvg = &store->tvg;
  tvg->serial_number = 0;
  tvg->ping_number = 0;
  tvg->multi_ping = 0;
  tvg->n = 0;
  for (int i = 0; i < 8; i++)
    tvg->reserved[i] = 0;
  tvg->nalloc = 0;
  if (tvg->tvg != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(tvg->tvg), error);

  /* Reson 7k image data (record 7011) */
  s7kr_image *image = &store->image;
  image->width = 0;
  image->height = 0;
  image->nalloc = 0;
  if (image->image != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(image->image), error);

  /* Reson 7k ping motion (record 7012) */
  s7kr_v2pingmotion *v2pingmotion = &store->v2pingmotion;
  if (v2pingmotion->nalloc > 0 && v2pingmotion->roll != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2pingmotion->roll), error);
  if (v2pingmotion->nalloc > 0 && v2pingmotion->heading != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2pingmotion->heading), error);
  if (v2pingmotion->nalloc > 0 && v2pingmotion->heave != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2pingmotion->heave), error);
  v2pingmotion->n = 0;
  v2pingmotion->nalloc = 0;

  /* Reson 7k beamformed magnitude and phase data (record 7018) */
  s7kr_v2beamformed *v2beamformed = &store->v2beamformed;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_v2amplitudephase *amplitudephase = &(v2beamformed->amplitudephase[i]);
    amplitudephase->number_samples = 0;
    amplitudephase->nalloc = 0;
    if (amplitudephase->amplitude != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->amplitude), error);
    if (amplitudephase->phase != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->phase), error);
  }

  /* Reson 7k BITE (record 7021) */
  s7kr_v2bite *v2bite = &store->v2bite;
  v2bite->number_reports = 0;
  v2bite->nalloc = 0;
  if (v2bite->reports != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2bite->reports), error);

  /* Reson 7k version 2 snippet (record 7028) */
  s7kr_v2snippet *v2snippet = &store->v2snippet;
  v2snippet->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_v2snippettimeseries *v2snippettimeseries = &(v2snippet->snippettimeseries[i]);
    v2snippettimeseries->beam_number = 0;
    v2snippettimeseries->begin_sample = 0;
    v2snippettimeseries->detect_sample = 0;
    v2snippettimeseries->end_sample = 0;
    v2snippettimeseries->nalloc = 0;
    if (v2snippettimeseries->amplitude != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(v2snippettimeseries->amplitude), error);
  }

  /* Reson 7k system event (record 7051) */
  s7kr_systemeventmessage *systemeventmessage = &store->systemeventmessage;
  systemeventmessage->message_length = 0;
  systemeventmessage->event_identifier = 0;
  systemeventmessage->message_alloc = 0;
  if (systemeventmessage->message != NULL)
    status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(systemeventmessage->message), error);

  /* Reson 7k calibrated snippet (record 7058) */
  s7kr_calibratedsnippet *calibratedsnippet = &store->calibratedsnippet;
  calibratedsnippet->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7kr_calibratedsnippettimeseries *calibratedsnippettimeseries =
        &(calibratedsnippet->calibratedsnippettimeseries[i]);
    calibratedsnippettimeseries->beam_number = 0;
    calibratedsnippettimeseries->begin_sample = 0;
    calibratedsnippettimeseries->detect_sample = 0;
    calibratedsnippettimeseries->end_sample = 0;
    calibratedsnippettimeseries->nalloc = 0;
    // TODO(schwehr): This was a bug, yes?
    // if (v2snippettimeseries->amplitude != NULL)
    if (calibratedsnippettimeseries->amplitude != NULL)
      status &= mb_freed(verbose, __FILE__, __LINE__, (void **)&(calibratedsnippettimeseries->amplitude), error);
  }

  /* deallocate memory for data structure */
  status &= mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
/* 7K Macros */
int mbsys_reson7k_checkheader(s7k_header header) {
  return ((header.Version > 0) && (header.SyncPattern == 0x0000ffff) && (header.Size > MBSYS_RESON7K_RECORDHEADER_SIZE) &&
          (header.s7kTime.Day >= 1) && (header.s7kTime.Day <= 366) && (header.s7kTime.Seconds >= 0.0f) &&
          (header.s7kTime.Seconds < 60.0f) && (header.s7kTime.Hours <= 23) && (header.s7kTime.Minutes <= 59));
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_header(int verbose, s7k_header *header, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
  }

  /* print Reson 7k data record header information */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     Version:                 %d\n", first, header->Version);
  fprintf(stderr, "%s     Offset:                  %d\n", first, header->Offset);
  fprintf(stderr, "%s     SyncPattern:             %u\n", first, header->SyncPattern);
  fprintf(stderr, "%s     Size:                    %u\n", first, header->Size);
  fprintf(stderr, "%s     OffsetToOptionalData:    %u\n", first, header->OffsetToOptionalData);
  fprintf(stderr, "%s     OptionalDataIdentifier:  %u\n", first, header->OptionalDataIdentifier);
  fprintf(stderr, "%s     s7kTime.Year:            %d\n", first, header->s7kTime.Year);
  fprintf(stderr, "%s     s7kTime.Day:             %d\n", first, header->s7kTime.Day);
  fprintf(stderr, "%s     s7kTime.Seconds:         %f\n", first, header->s7kTime.Seconds);
  fprintf(stderr, "%s     s7kTime.Hours:           %d\n", first, header->s7kTime.Hours);
  fprintf(stderr, "%s     7kTime->Minutes:         %d\n", first, header->s7kTime.Minutes);
  fprintf(stderr, "%s     Reserved:                %d\n", first, header->Reserved);
  fprintf(stderr, "%s     RecordType:              %u\n", first, header->RecordType);
  fprintf(stderr, "%s     DeviceId:                %u\n", first, header->DeviceId);
  fprintf(stderr, "%s     Reserved2:               %d\n", first, header->Reserved2);
  fprintf(stderr, "%s     SystemEnumerator:        %d\n", first, header->SystemEnumerator);
  fprintf(stderr, "%s     DataSetNumber:           %u\n", first, header->DataSetNumber);
  fprintf(stderr, "%s     RecordNumber:            %u\n", first, header->RecordNumber);
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%s     PreviousRecord[%d]:       %d\n", first, i, header->PreviousRecord[i]);
    fprintf(stderr, "%s     NextRecord[%d]:           %d\n", first, i, header->NextRecord[i]);
  }
  fprintf(stderr, "%s     Flags:                   %d\n", first, header->Flags);
  fprintf(stderr, "%s     Reserved3:               %d\n", first, header->Reserved3);
  fprintf(stderr, "%s     Reserved4:               %u\n", first, header->Reserved4);
  fprintf(stderr, "%s     FragmentedTotal:         %u\n", first, header->FragmentedTotal);
  fprintf(stderr, "%s     FragmentNumber:          %u\n", first, header->FragmentNumber);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_reference(int verbose, s7kr_reference *reference, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       reference:  %p\n", (void *)reference);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &reference->header, error);

  /* print Reference point information (record 1000) */
  const char *first = verbose >= 2 ? debug_str : nodebug_str;
  if (verbose >= 2) {
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, reference->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, reference->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, reference->offset_z);
  fprintf(stderr, "%s     water_z:                 %f\n", first, reference->water_z);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sensoruncal(int verbose, s7kr_sensoruncal *sensoruncal, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       sensoruncal:  %p\n", (void *)sensoruncal);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &sensoruncal->header, error);

  /* print Sensor uncalibrated offset position information (record 1001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, sensoruncal->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, sensoruncal->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, sensoruncal->offset_z);
  fprintf(stderr, "%s     offset_roll:             %f\n", first, sensoruncal->offset_roll);
  fprintf(stderr, "%s     offset_pitch:            %f\n", first, sensoruncal->offset_pitch);
  fprintf(stderr, "%s     offset_yaw:              %f\n", first, sensoruncal->offset_yaw);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_sensorcal(int verbose, s7kr_sensorcal *sensorcal, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       sensorcal:    %p\n", (void *)sensorcal);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &sensorcal->header, error);

  /* print Sensor Calibrated offset position information (record 1001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, sensorcal->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, sensorcal->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, sensorcal->offset_z);
  fprintf(stderr, "%s     offset_roll:             %f\n", first, sensorcal->offset_roll);
  fprintf(stderr, "%s     offset_pitch:            %f\n", first, sensorcal->offset_pitch);
  fprintf(stderr, "%s     offset_yaw:              %f\n", first, sensorcal->offset_yaw);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_position(int verbose, s7kr_position *position, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       position:     %p\n", (void *)position);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &position->header, error);

  /* print Sensor Calibrated offset position information (record 1001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     datum:                   %u\n", first, position->datum);
  fprintf(stderr, "%s     latency:                 %f\n", first, position->latency);
  fprintf(stderr, "%s     latitude:                %f\n", first, position->latitude);
  fprintf(stderr, "%s     longitude:               %f\n", first, position->longitude);
  fprintf(stderr, "%s     height:                  %f\n", first, position->height);
  fprintf(stderr, "%s     type:                    %d\n", first, position->type);
  fprintf(stderr, "%s     utm_zone:                %d\n", first, position->utm_zone);
  fprintf(stderr, "%s     quality:                 %d\n", first, position->quality);
  fprintf(stderr, "%s     method:                  %d\n", first, position->method);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_customattitude(int verbose, s7kr_customattitude *customattitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       customattitude:%p\n", (void *)customattitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &customattitude->header, error);

  /* print Custom attitude (record 1004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     bitfield:                   %d\n", first, customattitude->bitfield);
  fprintf(stderr, "%s     reserved:                   %d\n", first, customattitude->reserved);
  fprintf(stderr, "%s     n:                          %d\n", first, customattitude->n);
  fprintf(stderr, "%s     frequency:                  %f\n", first, customattitude->frequency);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, customattitude->nalloc);
  for (int i = 0; i < customattitude->n; i++)
    fprintf(stderr, "%s     i:%d pitch:%f roll:%f heading:%f heave:%f\n", first, i, customattitude->pitch[i],
            customattitude->roll[i], customattitude->heading[i], customattitude->heave[i]);
  for (int i = 0; i < customattitude->n; i++)
    fprintf(stderr, "%s     i:%d pitchrate:%f rollrate:%f headingrate:%f heaverate:%f\n", first, i,
            customattitude->pitchrate[i], customattitude->rollrate[i], customattitude->headingrate[i],
            customattitude->heaverate[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_tide(int verbose, s7kr_tide *tide, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       tide:         %p\n", (void *)tide);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &tide->header, error);

  /* print Tide (record 1005) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     tide:                       %f\n", first, tide->tide);
  fprintf(stderr, "%s     source:                     %d\n", first, tide->source);
  fprintf(stderr, "%s     flags:                      %d\n", first, tide->flags);
  fprintf(stderr, "%s     gauge:                      %d\n", first, tide->gauge);
  fprintf(stderr, "%s     datum:                      %u\n", first, tide->datum);
  fprintf(stderr, "%s     latency:                    %f\n", first, tide->latency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, tide->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, tide->longitude);
  fprintf(stderr, "%s     height:                     %f\n", first, tide->height);
  fprintf(stderr, "%s     type:                       %d\n", first, tide->type);
  fprintf(stderr, "%s     utm_zone:                   %d\n", first, tide->utm_zone);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_altitude(int verbose, s7kr_altitude *altitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       altitude:     %p\n", (void *)altitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &altitude->header, error);

  /* print Altitude (record 1006) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     altitude:                   %f\n", first, altitude->altitude);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_motion(int verbose, s7kr_motion *motion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       motion:       %p\n", (void *)motion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &motion->header, error);

  /* print Motion over ground (record 1007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     bitfield:                   %d\n", first, motion->bitfield);
  fprintf(stderr, "%s     reserved:                   %d\n", first, motion->reserved);
  fprintf(stderr, "%s     n:                          %d\n", first, motion->n);
  fprintf(stderr, "%s     frequency:                  %f\n", first, motion->frequency);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, motion->nalloc);
  for (int i = 0; i < motion->n; i++)
    fprintf(stderr, "%s     i:%d x:%f y:%f z:%f xa:%f ya:%f za:%f\n", first, i, motion->x[i], motion->y[i], motion->z[i],
            motion->xa[i], motion->ya[i], motion->za[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_depth(int verbose, s7kr_depth *depth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       depth:        %p\n", (void *)depth);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &depth->header, error);

  /* print Depth (record 1008) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     descriptor:                  %d\n", first, depth->descriptor);
  fprintf(stderr, "%s     correction:                  %d\n", first, depth->correction);
  fprintf(stderr, "%s     reserved:                    %d\n", first, depth->reserved);
  fprintf(stderr, "%s     depth:                       %f\n", first, depth->depth);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_svp(int verbose, s7kr_svp *svp, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       svp:          %p\n", (void *)svp);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &svp->header, error);

  /* print Sound velocity profile (record 1009) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     position_flag:              %d\n", first, svp->position_flag);
  fprintf(stderr, "%s     reserved1:                  %d\n", first, svp->reserved1);
  fprintf(stderr, "%s     reserved2:                  %d\n", first, svp->reserved2);
  fprintf(stderr, "%s     latitude:                   %f\n", first, svp->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, svp->longitude);
  fprintf(stderr, "%s     n:                          %u\n", first, svp->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, svp->nalloc);
  for (unsigned int i = 0; i < svp->n; i++)
    fprintf(stderr, "%s     i:%d depth:%f sound_velocity:%f\n", first, i, svp->depth[i], svp->sound_velocity[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_ctd(int verbose, s7kr_ctd *ctd, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       ctd:          %p\n", (void *)ctd);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &ctd->header, error);

  /* print CTD (record 1010) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, ctd->frequency);
  fprintf(stderr, "%s     velocity_source_flag:       %d\n", first, ctd->velocity_source_flag);
  fprintf(stderr, "%s     velocity_algorithm:         %d\n", first, ctd->velocity_algorithm);
  fprintf(stderr, "%s     conductivity_flag:          %d\n", first, ctd->conductivity_flag);
  fprintf(stderr, "%s     pressure_flag:              %d\n", first, ctd->pressure_flag);
  fprintf(stderr, "%s     position_flag:              %d\n", first, ctd->position_flag);
  fprintf(stderr, "%s     validity:                   %d\n", first, ctd->validity);
  fprintf(stderr, "%s     reserved:                   %d\n", first, ctd->reserved);
  fprintf(stderr, "%s     latitude:                   %f\n", first, ctd->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, ctd->longitude);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, ctd->sample_rate);
  fprintf(stderr, "%s     n:                          %u\n", first, ctd->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, ctd->nalloc);
  for (unsigned int i = 0; i < ctd->n; i++)
    fprintf(stderr, "%s     i:%d conductivity_salinity:%f temperature:%f pressure_depth:%f sound_velocity:%f absorption:%f\n",
            first, i, ctd->conductivity_salinity[i], ctd->temperature[i], ctd->pressure_depth[i], ctd->sound_velocity[i],
            ctd->absorption[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_geodesy(int verbose, s7kr_geodesy *geodesy, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       geodesy:      %p\n", (void *)geodesy);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &geodesy->header, error);

  /* print Geodesy (record 1011) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     spheroid:                   %s\n", first, geodesy->spheroid);
  fprintf(stderr, "%s     semimajoraxis:              %f\n", first, geodesy->semimajoraxis);
  fprintf(stderr, "%s     flattening:                 %f\n", first, geodesy->flattening);
  fprintf(stderr, "%s     reserved1:                  %s\n", first, geodesy->reserved1);
  fprintf(stderr, "%s     datum:                      %s\n", first, geodesy->datum);
  fprintf(stderr, "%s     calculation_method:         %u\n", first, geodesy->calculation_method);
  fprintf(stderr, "%s     number_parameters:          %u\n", first, geodesy->number_parameters);
  fprintf(stderr, "%s     dx:                         %f\n", first, geodesy->dx);
  fprintf(stderr, "%s     dy:                         %f\n", first, geodesy->dy);
  fprintf(stderr, "%s     dz:                         %f\n", first, geodesy->dz);
  fprintf(stderr, "%s     rx:                         %f\n", first, geodesy->rx);
  fprintf(stderr, "%s     ry:                         %f\n", first, geodesy->ry);
  fprintf(stderr, "%s     rz:                         %f\n", first, geodesy->rz);
  fprintf(stderr, "%s     scale:                      %f\n", first, geodesy->scale);
  fprintf(stderr, "%s     reserved2:                  %s\n", first, geodesy->reserved2);
  fprintf(stderr, "%s     grid_name:                  %s\n", first, geodesy->grid_name);
  fprintf(stderr, "%s     distance_units:             %d\n", first, geodesy->distance_units);
  fprintf(stderr, "%s     angular_units:              %d\n", first, geodesy->angular_units);
  fprintf(stderr, "%s     latitude_origin:            %f\n", first, geodesy->latitude_origin);
  fprintf(stderr, "%s     central_meriidan:           %f\n", first, geodesy->central_meriidan);
  fprintf(stderr, "%s     false_easting:              %f\n", first, geodesy->false_easting);
  fprintf(stderr, "%s     false_northing:             %f\n", first, geodesy->false_northing);
  fprintf(stderr, "%s     central_scale_factor:       %f\n", first, geodesy->central_scale_factor);
  fprintf(stderr, "%s     custum_identifier:          %d\n", first, geodesy->custum_identifier);
  fprintf(stderr, "%s     reserved3:                  %s\n", first, geodesy->reserved3);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_rollpitchheave(int verbose, s7kr_rollpitchheave *rollpitchheave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       rollpitchheave: %p\n", (void *)rollpitchheave);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &rollpitchheave->header, error);

  /* print Roll pitch heave (record 1012) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     roll:                       %f\n", first, rollpitchheave->roll);
  fprintf(stderr, "%s     pitch:                      %f\n", first, rollpitchheave->pitch);
  fprintf(stderr, "%s     heave:                      %f\n", first, rollpitchheave->heave);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_heading(int verbose, s7kr_heading *heading, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       heading:      %p\n", (void *)heading);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &heading->header, error);

  /* print Heading (record 1013) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     heading:                    %f\n", first, heading->heading);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_surveyline(int verbose, s7kr_surveyline *surveyline, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       surveyline:   %p\n", (void *)surveyline);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &surveyline->header, error);

  /* print Survey Line (record 1014) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     n:                          %d\n", first, surveyline->n);
  fprintf(stderr, "%s     type:                       %d\n", first, surveyline->type);
  fprintf(stderr, "%s     turnradius:                 %f\n", first, surveyline->turnradius);
  fprintf(stderr, "%s     name:                       %s\n", first, surveyline->name);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, surveyline->nalloc);
  for (int i = 0; i < surveyline->n; i++)
    fprintf(stderr, "%s     i:%d latitude:%f longitude:%f\n", first, i, surveyline->latitude[i], surveyline->longitude[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_navigation(int verbose, s7kr_navigation *navigation, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       navigation:   %p\n", (void *)navigation);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &navigation->header, error);

  /* print Navigation (record 1015) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     vertical_reference:         %d\n", first, navigation->vertical_reference);
  fprintf(stderr, "%s     latitude:                   %f\n", first, navigation->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, navigation->longitude);
  fprintf(stderr, "%s     position_accuracy:          %f\n", first, navigation->position_accuracy);
  fprintf(stderr, "%s     height:                     %f\n", first, navigation->height);
  fprintf(stderr, "%s     height_accuracy:            %f\n", first, navigation->height_accuracy);
  fprintf(stderr, "%s     speed:                      %f\n", first, navigation->speed);
  fprintf(stderr, "%s     course:                     %f\n", first, navigation->course);
  fprintf(stderr, "%s     heading:                    %f\n", first, navigation->heading);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_attitude(int verbose, s7kr_attitude *attitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       attitude:     %p\n", (void *)attitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &attitude->header, error);

  /* print Attitude (record 1016) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     n:                          %d\n", first, attitude->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, attitude->nalloc);
  for (int i = 0; i < attitude->n; i++)
    fprintf(stderr, "%s     i:%d delta_time:%d pitch:%f roll:%f heading:%f heave:%f\n", first, i, attitude->delta_time[i],
            attitude->pitch[i], attitude->roll[i], attitude->heading[i], attitude->heave[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_rec1022(int verbose, s7kr_rec1022 *rec1022, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       rec1022:      %p\n", (void *)rec1022);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &rec1022->header, error);

  /* print Attitude (record 1016) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     record bytes in hex:        |", first);
  for (int i = 0; i < R7KHDRSIZE_Rec1022; i++) {
    fprintf(stderr, "%x|", rec1022->data[i]);
  }
  fprintf(stderr, "\n");

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fsdwchannel(int verbose, int data_format, s7k_fsdwchannel *fsdwchannel, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       data_format:  %d\n", data_format);
    fprintf(stderr, "dbg2       fsdwchannel:  %p\n", (void *)fsdwchannel);
  }

  /* print Edgetech sidescan or subbottom channel header data */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     number:                     %d\n", first, fsdwchannel->number);
  fprintf(stderr, "%s     type:                       %d\n", first, fsdwchannel->type);
  fprintf(stderr, "%s     data_type:                  %d\n", first, fsdwchannel->data_type);
  fprintf(stderr, "%s     polarity:                   %d\n", first, fsdwchannel->polarity);
  fprintf(stderr, "%s     bytespersample:             %d\n", first, fsdwchannel->bytespersample);
  fprintf(stderr, "%s     reserved1[0]                %d\n", first, fsdwchannel->reserved1[0]);
  fprintf(stderr, "%s     reserved1[1]                %d\n", first, fsdwchannel->reserved1[1]);
  fprintf(stderr, "%s     reserved1[2]                %d\n", first, fsdwchannel->reserved1[2]);
  fprintf(stderr, "%s     number_samples:             %u\n", first, fsdwchannel->number_samples);
  fprintf(stderr, "%s     start_time:                 %u\n", first, fsdwchannel->start_time);
  fprintf(stderr, "%s     sample_interval:            %u\n", first, fsdwchannel->sample_interval);
  fprintf(stderr, "%s     range:                      %f\n", first, fsdwchannel->range);
  fprintf(stderr, "%s     voltage:                    %f\n", first, fsdwchannel->voltage);
  fprintf(stderr, "%s     name:                       %s\n", first, fsdwchannel->name);
  fprintf(stderr, "%s     reserved2:                  %s\n", first, fsdwchannel->reserved2);
  fprintf(stderr, "%s     data_alloc:                 %d\n", first, fsdwchannel->data_alloc);
  short *shortptr = (short *)fsdwchannel->data;
  unsigned short *ushortptr = (unsigned short *)fsdwchannel->data;
  for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
    if (data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
      fprintf(stderr, "%s     data[%d]:                   %d\n", first, i, ushortptr[i]);
    else if (data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
      fprintf(stderr, "%s     data[%d]:                   %d %d\n", first, i, shortptr[2 * i], shortptr[2 * i + 1]);
    else if (data_format == EDGETECH_TRACEFORMAT_RAW)
      fprintf(stderr, "%s     data[%d]:                   %d\n", first, i, ushortptr[i]);
    else if (data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
      fprintf(stderr, "%s     data[%d]:                   %d\n", first, i, ushortptr[i]);
    else if (data_format == EDGETECH_TRACEFORMAT_PIXEL)
      fprintf(stderr, "%s     data[%d]:                   %d\n", first, i, ushortptr[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fsdwssheader(int verbose, s7k_fsdwssheader *fsdwssheader, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       fsdwssheader:  %p\n", (void *)fsdwssheader);
  }

  /* print Edgetech sidescan or subbottom channel header data */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     subsystem:                    %d\n", first, fsdwssheader->subsystem);
  fprintf(stderr, "%s     channelNum:                   %d\n", first, fsdwssheader->channelNum);
  fprintf(stderr, "%s     pingNum:                      %u\n", first, fsdwssheader->pingNum);
  fprintf(stderr, "%s     packetNum:                    %d\n", first, fsdwssheader->packetNum);
  fprintf(stderr, "%s     trigSource:                   %d\n", first, fsdwssheader->trigSource);
  fprintf(stderr, "%s     samples:                      %u\n", first, fsdwssheader->samples);
  fprintf(stderr, "%s     sampleInterval:               %u\n", first, fsdwssheader->sampleInterval);
  fprintf(stderr, "%s     startDepth:                   %u\n", first, fsdwssheader->startDepth);
  fprintf(stderr, "%s     weightingFactor:              %d\n", first, fsdwssheader->weightingFactor);
  fprintf(stderr, "%s     ADCGain:                      %d\n", first, fsdwssheader->ADCGain);
  fprintf(stderr, "%s     ADCMax:                       %d\n", first, fsdwssheader->ADCMax);
  fprintf(stderr, "%s     rangeSetting:                 %d\n", first, fsdwssheader->rangeSetting);
  fprintf(stderr, "%s     pulseID:                      %d\n", first, fsdwssheader->pulseID);
  fprintf(stderr, "%s     markNumber:                   %d\n", first, fsdwssheader->markNumber);
  fprintf(stderr, "%s     dataFormat:                   %d\n", first, fsdwssheader->dataFormat);
  fprintf(stderr, "%s     reserved:                     %d\n", first, fsdwssheader->reserved);
  fprintf(stderr, "%s     millisecondsToday:            %u\n", first, fsdwssheader->millisecondsToday);
  fprintf(stderr, "%s     year:                         %d\n", first, fsdwssheader->year);
  fprintf(stderr, "%s     day:                          %d\n", first, fsdwssheader->day);
  fprintf(stderr, "%s     hour:                         %d\n", first, fsdwssheader->hour);
  fprintf(stderr, "%s     minute:                       %d\n", first, fsdwssheader->minute);
  fprintf(stderr, "%s     second:                       %d\n", first, fsdwssheader->second);
  fprintf(stderr, "%s     heading:                      %d\n", first, fsdwssheader->heading);
  fprintf(stderr, "%s     pitch:                        %d\n", first, fsdwssheader->pitch);
  fprintf(stderr, "%s     roll:                         %d\n", first, fsdwssheader->roll);
  fprintf(stderr, "%s     heave:                        %d\n", first, fsdwssheader->heave);
  fprintf(stderr, "%s     yaw:                          %d\n", first, fsdwssheader->yaw);
  fprintf(stderr, "%s     depth:                        %u\n", first, fsdwssheader->depth);
  fprintf(stderr, "%s     temperature:                  %d\n", first, fsdwssheader->temperature);
  for (int i = 0; i < 2; i++)
    fprintf(stderr, "%s     reserved2[%d]:                 %d\n", first, i, fsdwssheader->reserved2[i]);
  fprintf(stderr, "%s     longitude:                    %d\n", first, fsdwssheader->longitude);
  fprintf(stderr, "%s     latitude:                     %d\n", first, fsdwssheader->latitude);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fsdwsegyheader(int verbose, s7k_fsdwsegyheader *fsdwsegyheader, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       fsdwsegyheader:  %p\n", (void *)fsdwsegyheader);
  }

  /* print Edgetech sidescan or subbottom channel header data */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     sequenceNumber:              %d\n", first, fsdwsegyheader->sequenceNumber);
  fprintf(stderr, "%s     startDepth:                  %d\n", first, fsdwsegyheader->startDepth);
  fprintf(stderr, "%s     pingNum:                     %d\n", first, fsdwsegyheader->pingNum);
  fprintf(stderr, "%s     channelNum:                  %d\n", first, fsdwsegyheader->channelNum);
  for (int i = 0; i < 6; i++)
    fprintf(stderr, "%s     unused1[%d]:                  %d\n", first, i, fsdwsegyheader->unused1[i]);
  fprintf(stderr, "%s     traceIDCode:                 %d\n", first, fsdwsegyheader->traceIDCode);
  for (int i = 0; i < 2; i++)
    fprintf(stderr, "%s     unused2[%d]:                  %d\n", first, i, fsdwsegyheader->unused2[i]);
  fprintf(stderr, "%s     dataFormat:                  %d\n", first, fsdwsegyheader->dataFormat);
  fprintf(stderr, "%s     NMEAantennaeR:               %d\n", first, fsdwsegyheader->NMEAantennaeR);
  fprintf(stderr, "%s     NMEAantennaeO:               %d\n", first, fsdwsegyheader->NMEAantennaeO);
  for (int i = 0; i < 32; i++)
    fprintf(stderr, "%s     RS232[%d]:                   %d\n", first, i, fsdwsegyheader->RS232[i]);
  fprintf(stderr, "%s     sourceCoordX:                %d\n", first, fsdwsegyheader->sourceCoordX);
  fprintf(stderr, "%s     sourceCoordY:                %d\n", first, fsdwsegyheader->sourceCoordY);
  fprintf(stderr, "%s     groupCoordX:                 %d\n", first, fsdwsegyheader->groupCoordX);
  fprintf(stderr, "%s     groupCoordY:                 %d\n", first, fsdwsegyheader->groupCoordY);
  fprintf(stderr, "%s     coordUnits:                  %d\n", first, fsdwsegyheader->coordUnits);
  fprintf(stderr, "%s     annotation:                  %s\n", first, fsdwsegyheader->annotation);
  fprintf(stderr, "%s     samples:                     %d\n", first, fsdwsegyheader->samples);
  fprintf(stderr, "%s     sampleInterval:              %d\n", first, fsdwsegyheader->sampleInterval);
  fprintf(stderr, "%s     ADCGain:                     %d\n", first, fsdwsegyheader->ADCGain);
  fprintf(stderr, "%s     pulsePower:                  %d\n", first, fsdwsegyheader->pulsePower);
  fprintf(stderr, "%s     correlated:                  %d\n", first, fsdwsegyheader->correlated);
  fprintf(stderr, "%s     startFreq:                   %d\n", first, fsdwsegyheader->startFreq);
  fprintf(stderr, "%s     endFreq:                     %d\n", first, fsdwsegyheader->endFreq);
  fprintf(stderr, "%s     sweepLength:                 %d\n", first, fsdwsegyheader->sweepLength);
  for (int i = 0; i < 4; i++)
    fprintf(stderr, "%s     unused7[%d]:                  %d\n", first, i, fsdwsegyheader->unused7[i]);
  fprintf(stderr, "%s     aliasFreq:                   %d\n", first, fsdwsegyheader->aliasFreq);
  fprintf(stderr, "%s     pulseID:                     %d\n", first, fsdwsegyheader->pulseID);
  for (int i = 0; i < 6; i++)
    fprintf(stderr, "%s     unused8[%d]:                  %d\n", first, i, fsdwsegyheader->unused8[i]);
  fprintf(stderr, "%s     year:                        %d\n", first, fsdwsegyheader->year);
  fprintf(stderr, "%s     day:                         %d\n", first, fsdwsegyheader->day);
  fprintf(stderr, "%s     hour:                        %d\n", first, fsdwsegyheader->hour);
  fprintf(stderr, "%s     minute:                      %d\n", first, fsdwsegyheader->minute);
  fprintf(stderr, "%s     second:                      %d\n", first, fsdwsegyheader->second);
  fprintf(stderr, "%s     timeBasis:                   %d\n", first, fsdwsegyheader->timeBasis);
  fprintf(stderr, "%s     weightingFactor:             %d\n", first, fsdwsegyheader->weightingFactor);
  fprintf(stderr, "%s     unused9:                     %d\n", first, fsdwsegyheader->unused9);
  fprintf(stderr, "%s     heading:                     %d\n", first, fsdwsegyheader->heading);
  fprintf(stderr, "%s     pitch:                       %d\n", first, fsdwsegyheader->pitch);
  fprintf(stderr, "%s     roll:                        %d\n", first, fsdwsegyheader->roll);
  fprintf(stderr, "%s     temperature:                 %d\n", first, fsdwsegyheader->temperature);
  fprintf(stderr, "%s     heaveCompensation:           %d\n", first, fsdwsegyheader->heaveCompensation);
  fprintf(stderr, "%s     trigSource:                  %d\n", first, fsdwsegyheader->trigSource);
  fprintf(stderr, "%s     markNumber:                  %d\n", first, fsdwsegyheader->markNumber);
  fprintf(stderr, "%s     NMEAHour:                    %d\n", first, fsdwsegyheader->NMEAHour);
  fprintf(stderr, "%s     NMEAMinutes:                 %d\n", first, fsdwsegyheader->NMEAMinutes);
  fprintf(stderr, "%s     NMEASeconds:                 %d\n", first, fsdwsegyheader->NMEASeconds);
  fprintf(stderr, "%s     NMEACourse:                  %d\n", first, fsdwsegyheader->NMEACourse);
  fprintf(stderr, "%s     NMEASpeed:                   %d\n", first, fsdwsegyheader->NMEASpeed);
  fprintf(stderr, "%s     NMEADay:                     %d\n", first, fsdwsegyheader->NMEADay);
  fprintf(stderr, "%s     NMEAYear:                    %d\n", first, fsdwsegyheader->NMEAYear);
  fprintf(stderr, "%s     millisecondsToday:           %d\n", first, fsdwsegyheader->millisecondsToday);
  fprintf(stderr, "%s     ADCMax:                      %d\n", first, fsdwsegyheader->ADCMax);
  fprintf(stderr, "%s     calConst:                    %d\n", first, fsdwsegyheader->calConst);
  fprintf(stderr, "%s     vehicleID:                   %d\n", first, fsdwsegyheader->vehicleID);
  fprintf(stderr, "%s     softwareVersion:             %s\n", first, fsdwsegyheader->softwareVersion);
  fprintf(stderr, "%s     sphericalCorrection:         %d\n", first, fsdwsegyheader->sphericalCorrection);
  fprintf(stderr, "%s     packetNum:                   %d\n", first, fsdwsegyheader->packetNum);
  fprintf(stderr, "%s     ADCDecimation:               %d\n", first, fsdwsegyheader->ADCDecimation);
  fprintf(stderr, "%s     decimation:                  %d\n", first, fsdwsegyheader->decimation);
  for (int i = 0; i < 7; i++)
    fprintf(stderr, "%s     unuseda[%d]:                  %d\n", first, i, fsdwsegyheader->unuseda[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fsdwss(int verbose, s7kr_fsdwss *fsdwss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       fsdwss:       %p\n", (void *)fsdwss);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &fsdwss->header, error);

  /* print Edgetech FS-DW sidescan (record 3000) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     msec_timestamp:             %d\n", first, fsdwss->msec_timestamp);
  fprintf(stderr, "%s     ping_number:                %u\n", first, fsdwss->ping_number);
  fprintf(stderr, "%s     number_channels:            %d\n", first, fsdwss->number_channels);
  fprintf(stderr, "%s     total_bytes:                %d\n", first, fsdwss->total_bytes);
  fprintf(stderr, "%s     data_format:                %d\n", first, fsdwss->data_format);
  for (int i = 0; i < fsdwss->number_channels; i++) {
    mbsys_reson7k_print_fsdwchannel(verbose, fsdwss->data_format, &fsdwss->channel[i], error);
    mbsys_reson7k_print_fsdwssheader(verbose, &fsdwss->ssheader[i], error);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fsdwsb(int verbose, s7kr_fsdwsb *fsdwsb, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       fsdwsb:       %p\n", (void *)fsdwsb);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &fsdwsb->header, error);

  /* print Edgetech FS-DW subbottom (record 3001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     msec_timestamp:             %d\n", first, fsdwsb->msec_timestamp);
  fprintf(stderr, "%s     ping_number:                %u\n", first, fsdwsb->ping_number);
  fprintf(stderr, "%s     number_channels:            %d\n", first, fsdwsb->number_channels);
  fprintf(stderr, "%s     total_bytes:                %d\n", first, fsdwsb->total_bytes);
  fprintf(stderr, "%s     data_format:                %d\n", first, fsdwsb->data_format);
  mbsys_reson7k_print_fsdwchannel(verbose, fsdwsb->data_format, &fsdwsb->channel, error);
  mbsys_reson7k_print_fsdwsegyheader(verbose, &fsdwsb->segyheader, error);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_bluefin(int verbose, s7kr_bluefin *bluefin, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       bluefin:      %p\n", (void *)bluefin);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &bluefin->header, error);

  /* print Bluefin data frames (record 3100) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     msec_timestamp:             %d\n", first, bluefin->msec_timestamp);
  fprintf(stderr, "%s     number_frames:              %d\n", first, bluefin->number_frames);
  fprintf(stderr, "%s     frame_size:                 %d\n", first, bluefin->frame_size);
  fprintf(stderr, "%s     data_format:                %d\n", first, bluefin->data_format);
  for (int i = 0; i < 16; i++)
    fprintf(stderr, "%s     reserved[%d]:                %d\n", first, i, bluefin->reserved[i]);
  if (bluefin->data_format == R7KRECID_BluefinNav) {
    for (int i = 0; i < MIN(bluefin->number_frames, BLUEFIN_MAX_FRAMES); i++) {
      fprintf(stderr, "%s     nav[%d].packet_size:        %d\n", first, i, bluefin->nav[i].packet_size);
      fprintf(stderr, "%s     nav[%d].version:            %d\n", first, i, bluefin->nav[i].version);
      fprintf(stderr, "%s     nav[%d].offset:             %d\n", first, i, bluefin->nav[i].offset);
      fprintf(stderr, "%s     nav[%d].data_type:          %d\n", first, i, bluefin->nav[i].data_type);
      fprintf(stderr, "%s     nav[%d].data_size:          %d\n", first, i, bluefin->nav[i].data_size);
      fprintf(stderr, "%s     nav[%d].s7kTime.Year:       %d\n", first, i, bluefin->nav[i].s7kTime.Year);
      fprintf(stderr, "%s     nav[%d].s7kTime.Day:        %d\n", first, i, bluefin->nav[i].s7kTime.Day);
      fprintf(stderr, "%s     nav[%d].s7kTime.Seconds:    %f\n", first, i, bluefin->nav[i].s7kTime.Seconds);
      fprintf(stderr, "%s     nav[%d].s7kTime.Hours:      %d\n", first, i, bluefin->nav[i].s7kTime.Hours);
      fprintf(stderr, "%s     nav[%d].7kTime->Minutes:    %d\n", first, i, bluefin->nav[i].s7kTime.Minutes);
      fprintf(stderr, "%s     nav[%d].checksum:           %d\n", first, i, bluefin->nav[i].checksum);
      fprintf(stderr, "%s     nav[%d].timedelay:          %d\n", first, i, bluefin->nav[i].timedelay);
      fprintf(stderr, "%s     nav[%d].quality:            %x\n", first, i, bluefin->nav[i].quality);
      fprintf(stderr, "%s     nav[%d].latitude:           %f\n", first, i, bluefin->nav[i].latitude);
      fprintf(stderr, "%s     nav[%d].longitude:          %f\n", first, i, bluefin->nav[i].longitude);
      fprintf(stderr, "%s     nav[%d].speed:              %f\n", first, i, bluefin->nav[i].speed);
      fprintf(stderr, "%s     nav[%d].depth:              %f\n", first, i, bluefin->nav[i].depth);
      fprintf(stderr, "%s     nav[%d].altitude:           %f\n", first, i, bluefin->nav[i].altitude);
      fprintf(stderr, "%s     nav[%d].roll:               %f\n", first, i, bluefin->nav[i].roll);
      fprintf(stderr, "%s     nav[%d].pitch:              %f\n", first, i, bluefin->nav[i].pitch);
      fprintf(stderr, "%s     nav[%d].yaw:                %f\n", first, i, bluefin->nav[i].yaw);
      fprintf(stderr, "%s     nav[%d].northing_rate:      %f\n", first, i, bluefin->nav[i].northing_rate);
      fprintf(stderr, "%s     nav[%d].easting_rate:       %f\n", first, i, bluefin->nav[i].easting_rate);
      fprintf(stderr, "%s     nav[%d].depth_rate:         %f\n", first, i, bluefin->nav[i].depth_rate);
      fprintf(stderr, "%s     nav[%d].altitude_rate:      %f\n", first, i, bluefin->nav[i].altitude_rate);
      fprintf(stderr, "%s     nav[%d].roll_rate:          %f\n", first, i, bluefin->nav[i].roll_rate);
      fprintf(stderr, "%s     nav[%d].pitch_rate:         %f\n", first, i, bluefin->nav[i].pitch_rate);
      fprintf(stderr, "%s     nav[%d].yaw_rate:           %f\n", first, i, bluefin->nav[i].yaw_rate);
      fprintf(stderr, "%s     nav[%d].position_time:      %f\n", first, i, bluefin->nav[i].position_time);
      fprintf(stderr, "%s     nav[%d].depth_time:         %f\n", first, i, bluefin->nav[i].depth_time);
    }
  }
  else if (bluefin->data_format == R7KRECID_BluefinEnvironmental) {
    for (int i = 0; i < MIN(bluefin->number_frames, BLUEFIN_MAX_FRAMES); i++) {
      fprintf(stderr, "%s     env[%d].packet_size:            %d\n", first, i, bluefin->environmental[i].packet_size);
      fprintf(stderr, "%s     env[%d].version:                %d\n", first, i, bluefin->environmental[i].version);
      fprintf(stderr, "%s     env[%d].offset:                 %d\n", first, i, bluefin->environmental[i].offset);
      fprintf(stderr, "%s     env[%d].data_type:              %d\n", first, i, bluefin->environmental[i].data_type);
      fprintf(stderr, "%s     env[%d].data_size:              %d\n", first, i, bluefin->environmental[i].data_size);
      fprintf(stderr, "%s     env[%d].s7kTime.Year:           %d\n", first, i, bluefin->environmental[i].s7kTime.Year);
      fprintf(stderr, "%s     env[%d].s7kTime.Day:            %d\n", first, i, bluefin->environmental[i].s7kTime.Day);
      fprintf(stderr, "%s     env[%d].s7kTime.Seconds:        %f\n", first, i, bluefin->environmental[i].s7kTime.Seconds);
      fprintf(stderr, "%s     env[%d].s7kTime.Hours:          %d\n", first, i, bluefin->environmental[i].s7kTime.Hours);
      fprintf(stderr, "%s     env[%d].7kTime->Minutes:        %d\n", first, i, bluefin->environmental[i].s7kTime.Minutes);
      fprintf(stderr, "%s     env[%d].checksum:               %d\n", first, i, bluefin->environmental[i].checksum);
      fprintf(stderr, "%s     env[%d].reserved1:              %d\n", first, i, bluefin->environmental[i].reserved1);
      fprintf(stderr, "%s     env[%d].quality:                %d\n", first, i, bluefin->environmental[i].quality);
      fprintf(stderr, "%s     env[%d].sound_speed:            %f\n", first, i, bluefin->environmental[i].sound_speed);
      fprintf(stderr, "%s     env[%d].conductivity:           %f\n", first, i, bluefin->environmental[i].conductivity);
      fprintf(stderr, "%s     env[%d].temperature:            %f\n", first, i, bluefin->environmental[i].temperature);
      fprintf(stderr, "%s     env[%d].pressure:               %f\n", first, i, bluefin->environmental[i].pressure);
      fprintf(stderr, "%s     env[%d].salinity:               %f\n", first, i, bluefin->environmental[i].salinity);
      fprintf(stderr, "%s     env[%d].ctd_time:               %f\n", first, i, bluefin->environmental[i].ctd_time);
      fprintf(stderr, "%s     env[%d].temperature_time:       %f\n", first, i, bluefin->environmental[i].temperature_time);
      fprintf(stderr, "%s     env[%d].surface_pressure:       %f\n", first, i, bluefin->environmental[i].surface_pressure);
      fprintf(stderr, "%s     env[%d].temperature_counts:     %d\n", first, i,
              bluefin->environmental[i].temperature_counts);
      fprintf(stderr, "%s     env[%d].conductivity_frequency: %f\n", first, i,
              bluefin->environmental[i].conductivity_frequency);
      fprintf(stderr, "%s     env[%d].pressure_counts:        %d\n", first, i, bluefin->environmental[i].pressure_counts);
      fprintf(stderr, "%s     env[%d].pressure_comp_voltage:  %f\n", first, i,
              bluefin->environmental[i].pressure_comp_voltage);
      fprintf(stderr, "%s     env[%d].sensor_time_sec:        %d\n", first, i, bluefin->environmental[i].sensor_time_sec);
      fprintf(stderr, "%s     env[%d].sensor_time_nsec:       %d\n", first, i, bluefin->environmental[i].sensor_time_nsec);
      fprintf(stderr, "%s     env[%d].sensor1:                %d\n", first, i, bluefin->environmental[i].sensor1);
      fprintf(stderr, "%s     env[%d].sensor2:                %d\n", first, i, bluefin->environmental[i].sensor2);
      fprintf(stderr, "%s     env[%d].sensor3:                %d\n", first, i, bluefin->environmental[i].sensor3);
      fprintf(stderr, "%s     env[%d].sensor4:                %d\n", first, i, bluefin->environmental[i].sensor4);
      fprintf(stderr, "%s     env[%d].sensor5:                %d\n", first, i, bluefin->environmental[i].sensor5);
      fprintf(stderr, "%s     env[%d].sensor6:                %d\n", first, i, bluefin->environmental[i].sensor6);
      fprintf(stderr, "%s     env[%d].sensor7:                %d\n", first, i, bluefin->environmental[i].sensor7);
      fprintf(stderr, "%s     env[%d].sensor8:                %d\n", first, i, bluefin->environmental[i].sensor8);
      for (int j = 0; j < 8; j++)
        fprintf(stderr, "%s     env[%d].reserved2[%2d]:          %d\n", first, i, j,
                bluefin->environmental[i].reserved2[j]);
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_processedsidescan(int verbose, s7kr_processedsidescan *processedsidescan, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       processedsidescan: %p\n", (void *)processedsidescan);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &processedsidescan->header, error);

  /* print Reson 7k beam geometry (record 7004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, processedsidescan->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, processedsidescan->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, processedsidescan->multi_ping);
  fprintf(stderr, "%s     recordversion:              %u\n", first, processedsidescan->recordversion);
  fprintf(stderr, "%s     ss_source:                  %u\n", first, processedsidescan->ss_source);
  fprintf(stderr, "%s     number_pixels:              %u\n", first, processedsidescan->number_pixels);
  fprintf(stderr, "%s     pixelwidth:                 %f\n", first, processedsidescan->pixelwidth);
  fprintf(stderr, "%s     sensordepth:                 %f\n", first, processedsidescan->sensordepth);
  fprintf(stderr, "%s     altitude:                   %f\n", first, processedsidescan->altitude);
  for (unsigned int i = 0; i < processedsidescan->number_pixels; i++)
    fprintf(stderr, "%s     pixel[%d]:  sidescan:%f alongtrack:%f\n", first, i, processedsidescan->sidescan[i],
            processedsidescan->alongtrack[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_volatilesettings(int verbose, s7kr_volatilesettings *volatilesettings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       volatilesettings:  %p\n", (void *)volatilesettings);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &volatilesettings->header, error);

  /* print Reson 7k volatile sonar settings (record 7000) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, volatilesettings->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, volatilesettings->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, volatilesettings->multi_ping);
  fprintf(stderr, "%s     frequency:                  %f\n", first, volatilesettings->frequency);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, volatilesettings->sample_rate);
  fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, volatilesettings->receiver_bandwidth);
  fprintf(stderr, "%s     pulse_width:                %f\n", first, volatilesettings->pulse_width);
  fprintf(stderr, "%s     pulse_type:                 %u\n", first, volatilesettings->pulse_type);
  fprintf(stderr, "%s     pulse_envelope:             %u\n", first, volatilesettings->pulse_envelope);
  fprintf(stderr, "%s     pulse_envelope_par:         %f\n", first, volatilesettings->pulse_envelope_par);
  fprintf(stderr, "%s     pulse_reserved:             %u\n", first, volatilesettings->pulse_reserved);
  fprintf(stderr, "%s     max_ping_rate:              %f\n", first, volatilesettings->max_ping_rate);
  fprintf(stderr, "%s     ping_period:                %f\n", first, volatilesettings->ping_period);
  fprintf(stderr, "%s     range_selection:            %f\n", first, volatilesettings->range_selection);
  fprintf(stderr, "%s     power_selection:            %f\n", first, volatilesettings->power_selection);
  fprintf(stderr, "%s     gain_selection:             %f\n", first, volatilesettings->gain_selection);
  fprintf(stderr, "%s     control_flags:              %d\n", first, volatilesettings->control_flags);
  fprintf(stderr, "%s     projector_magic_no:         %d\n", first, volatilesettings->projector_magic_no);
  fprintf(stderr, "%s     steering_vertical:          %f\n", first, volatilesettings->steering_vertical);
  fprintf(stderr, "%s     steering_horizontal:        %f\n", first, volatilesettings->steering_horizontal);
  fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, volatilesettings->beamwidth_vertical);
  fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, volatilesettings->beamwidth_horizontal);
  fprintf(stderr, "%s     focal_point:                %f\n", first, volatilesettings->focal_point);
  fprintf(stderr, "%s     projector_weighting:        %d\n", first, volatilesettings->projector_weighting);
  fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, volatilesettings->projector_weighting_par);
  fprintf(stderr, "%s     transmit_flags:             %d\n", first, volatilesettings->transmit_flags);
  fprintf(stderr, "%s     hydrophone_magic_no:        %d\n", first, volatilesettings->hydrophone_magic_no);
  fprintf(stderr, "%s     receive_weighting:          %d\n", first, volatilesettings->receive_weighting);
  fprintf(stderr, "%s     receive_weighting_par:      %f\n", first, volatilesettings->receive_weighting_par);
  fprintf(stderr, "%s     receive_flags:              %d\n", first, volatilesettings->receive_flags);
  fprintf(stderr, "%s     receive_width:              %f\n", first, volatilesettings->receive_width);
  fprintf(stderr, "%s     range_minimum:              %f\n", first, volatilesettings->range_minimum);
  fprintf(stderr, "%s     range_maximum:              %f\n", first, volatilesettings->range_maximum);
  fprintf(stderr, "%s     depth_minimum:              %f\n", first, volatilesettings->depth_minimum);
  fprintf(stderr, "%s     depth_maximum:              %f\n", first, volatilesettings->depth_maximum);
  fprintf(stderr, "%s     absorption:                 %f\n", first, volatilesettings->absorption);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, volatilesettings->sound_velocity);
  fprintf(stderr, "%s     spreading:                  %f\n", first, volatilesettings->spreading);
  fprintf(stderr, "%s     reserved:                   %d\n", first, volatilesettings->reserved);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_device(int verbose, s7k_device *device, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       device:            %p\n", (void *)device);
  }

  /* print Reson 7k device configuration structure */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     magic_number:               %d\n", first, device->magic_number);
  fprintf(stderr, "%s     description:                %s\n", first, device->description);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, device->serial_number);
  fprintf(stderr, "%s     info_length:                %d\n", first, device->info_length);
  fprintf(stderr, "%s     info_alloc:                 %d\n", first, device->info_alloc);
  fprintf(stderr, "%s     info:                       %s\n", first, device->info);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_configuration(int verbose, s7kr_configuration *configuration, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       configuration:     %p\n", (void *)configuration);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &configuration->header, error);

  /* print Reson 7k configuration (record 7001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, configuration->serial_number);
  fprintf(stderr, "%s     number_devices:             %d\n", first, configuration->number_devices);
  for (unsigned int i = 0; i < configuration->number_devices; i++)
    mbsys_reson7k_print_device(verbose, &configuration->device[i], error);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_matchfilter(int verbose, s7kr_matchfilter *matchfilter, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       matchfilter:       %p\n", (void *)matchfilter);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &matchfilter->header, error);

  /* print Reson 7k match filter (record 7002) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, matchfilter->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, matchfilter->ping_number);
  fprintf(stderr, "%s     operation:                  %d\n", first, matchfilter->operation);
  fprintf(stderr, "%s     start_frequency:            %f\n", first, matchfilter->start_frequency);
  fprintf(stderr, "%s     end_frequency:              %f\n", first, matchfilter->end_frequency);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2firmwarehardwareconfiguration(int verbose,
                                                        s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration,
                                                        int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2firmwarehardwareconfiguration:       %p\n", (void *)v2firmwarehardwareconfiguration);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2firmwarehardwareconfiguration->header, error);

  /* print Reson 7k match filter (record 7002) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     device_count:               %d\n", first, v2firmwarehardwareconfiguration->device_count);
  fprintf(stderr, "%s     info_length:                %d\n", first, v2firmwarehardwareconfiguration->info_length);
  fprintf(stderr, "%s     info:                       \n", first);
  fprintf(stderr, "%s\n%s\n", v2firmwarehardwareconfiguration->info, first);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_beamgeometry(int verbose, s7kr_beamgeometry *beamgeometry, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       beamgeometry:      %p\n", (void *)beamgeometry);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &beamgeometry->header, error);

  /* print Reson 7k beam geometry (record 7004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, beamgeometry->serial_number);
  fprintf(stderr, "%s     number_beams:               %u\n", first, beamgeometry->number_beams);
  for (unsigned int i = 0; i < beamgeometry->number_beams; i++)
    fprintf(stderr,
            "%s     beam[%d]:  angle_alongtrack:%f angle_acrosstrack:%f beamwidth_alongtrack:%f beamwidth_acrosstrack:%f\n",
            first, i, beamgeometry->angle_alongtrack[i], beamgeometry->angle_acrosstrack[i],
            beamgeometry->beamwidth_alongtrack[i], beamgeometry->beamwidth_acrosstrack[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_calibration(int verbose, s7kr_calibration *calibration, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       calibration:       %p\n", (void *)calibration);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &calibration->header, error);

  /* print Reson 7k calibration data (record 7005) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, calibration->serial_number);
  fprintf(stderr, "%s     number_channels:            %d\n", first, calibration->number_channels);
  for (int i = 0; i < calibration->number_channels; i++)
    fprintf(stderr, "%s     channel[%d]:  gain:%f phase:%f\n", first, i, calibration->gain[i], calibration->phase[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_bathymetry(int verbose, s7kr_bathymetry *bathymetry, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       bathymetry:        %p\n", (void *)bathymetry);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &bathymetry->header, error);

  /* print Reson 7k bathymetry (record 7006) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, bathymetry->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, bathymetry->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, bathymetry->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, bathymetry->number_beams);
  fprintf(stderr, "%s     layer_comp_flag:            %d\n", first, bathymetry->layer_comp_flag);
  fprintf(stderr, "%s     sound_vel_flag:             %d\n", first, bathymetry->sound_vel_flag);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, bathymetry->sound_velocity);
  for (unsigned int i = 0; i < bathymetry->number_beams; i++)
    fprintf(stderr, "%s     beam[%d]:  range:%f quality:%d intensity:%f min_depth_gate:%f min_depth_gate:%f\n", first, i,
            bathymetry->range[i], bathymetry->quality[i], bathymetry->intensity[i], bathymetry->min_depth_gate[i],
            bathymetry->max_depth_gate[i]);
  fprintf(stderr, "%s     optionaldata:               %d\n", first, bathymetry->optionaldata);
  fprintf(stderr, "%s     frequency:                  %f\n", first, bathymetry->frequency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, bathymetry->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, bathymetry->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, bathymetry->heading);
  fprintf(stderr, "%s     height_source:              %d\n", first, bathymetry->height_source);
  fprintf(stderr, "%s     tide:                       %f\n", first, bathymetry->tide);
  fprintf(stderr, "%s     roll:                       %f\n", first, bathymetry->roll);
  fprintf(stderr, "%s     pitch:                      %f\n", first, bathymetry->pitch);
  fprintf(stderr, "%s     heave:                      %f\n", first, bathymetry->heave);
  fprintf(stderr, "%s     vehicle_height:             %f\n", first, bathymetry->vehicle_height);
  for (unsigned int i = 0; i < bathymetry->number_beams; i++)
    fprintf(stderr, "%s     beam[%d]:  depth:%f ltrack:%f xtrack:%f angles: %f %f\n", first, i, bathymetry->depth[i],
            bathymetry->alongtrack[i], bathymetry->acrosstrack[i], bathymetry->pointing_angle[i],
            bathymetry->azimuth_angle[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_backscatter(int verbose, s7kr_backscatter *backscatter, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       backscatter:       %p\n", (void *)backscatter);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &backscatter->header, error);

  /* print Reson 7k backscatter imagery data (record 7007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, backscatter->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, backscatter->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, backscatter->multi_ping);
  fprintf(stderr, "%s     beam_position:              %f\n", first, backscatter->beam_position);
  fprintf(stderr, "%s     control_flags:              %d\n", first, backscatter->control_flags);
  fprintf(stderr, "%s     number_samples:             %d\n", first, backscatter->number_samples);
  fprintf(stderr, "%s     port_beamwidth_x:           %f\n", first, backscatter->port_beamwidth_x);
  fprintf(stderr, "%s     port_beamwidth_y:           %f\n", first, backscatter->port_beamwidth_y);
  fprintf(stderr, "%s     stbd_beamwidth_x:           %f\n", first, backscatter->stbd_beamwidth_x);
  fprintf(stderr, "%s     stbd_beamwidth_y:           %f\n", first, backscatter->stbd_beamwidth_y);
  fprintf(stderr, "%s     port_steering_x:            %f\n", first, backscatter->port_steering_x);
  fprintf(stderr, "%s     port_steering_y:            %f\n", first, backscatter->port_steering_y);
  fprintf(stderr, "%s     stbd_steering_x:            %f\n", first, backscatter->stbd_steering_x);
  fprintf(stderr, "%s     stbd_steering_y:            %f\n", first, backscatter->stbd_steering_y);
  fprintf(stderr, "%s     number_beams:               %u\n", first, backscatter->number_beams);
  fprintf(stderr, "%s     current_beam:               %d\n", first, backscatter->current_beam);
  fprintf(stderr, "%s     sample_size:                %d\n", first, backscatter->sample_size);
  fprintf(stderr, "%s     data_type:                  %d\n", first, backscatter->data_type);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, backscatter->nalloc);
  if (backscatter->sample_size == 1) {
    mb_s_char *charptr = (mb_s_char *)backscatter->port_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     port backscatter[%d]:  %d\n", first, i, charptr[i]);
    charptr = (mb_s_char *)backscatter->stbd_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     stbd backscatter[%d]:  %d\n", first, i, charptr[i]);
  } else if (backscatter->sample_size == 2) {
    short *shortptr = (short *)backscatter->port_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     port backscatter[%d]:  %d\n", first, i, shortptr[i]);
    shortptr = (short *)backscatter->stbd_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     stbd backscatter[%d]:  %d\n", first, i, shortptr[i]);
  } else if (backscatter->sample_size == 4) {
    int *intptr = (int *)backscatter->port_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     port backscatter[%d]:  %d\n", first, i, intptr[i]);
    intptr = (int *)backscatter->stbd_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++)
      fprintf(stderr, "%s     stbd backscatter[%d]:  %d\n", first, i, intptr[i]);
  }
  fprintf(stderr, "%s     optionaldata:               %d\n", first, backscatter->optionaldata);
  fprintf(stderr, "%s     frequency:                  %f\n", first, backscatter->frequency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, backscatter->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, backscatter->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, backscatter->heading);
  fprintf(stderr, "%s     altitude:                   %f\n", first, backscatter->altitude);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_beam(int verbose, s7kr_beam *beam, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       beam:              %p\n", (void *)beam);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &beam->header, error);

  /* print Reson 7k beam data (record 7007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, beam->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, beam->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, beam->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, beam->number_beams);
  fprintf(stderr, "%s     reserved:                   %d\n", first, beam->reserved);
  fprintf(stderr, "%s     number_samples:             %d\n", first, beam->number_samples);
  fprintf(stderr, "%s     record_subset_flag:         %d\n", first, beam->record_subset_flag);
  fprintf(stderr, "%s     row_column_flag:            %d\n", first, beam->row_column_flag);
  fprintf(stderr, "%s     sample_header_id:           %d\n", first, beam->sample_header_id);
  fprintf(stderr, "%s     sample_type:                %d\n", first, beam->sample_type);

  const int sample_type_amp = beam->sample_type & 15;
  const int sample_type_phase = (beam->sample_type >> 4) & 15;
  const int sample_type_iandq = (beam->sample_type >> 8) & 15;
  const int sample_type_beamforming = (beam->sample_type >> 12) & 15;
  fprintf(stderr, "%s     sample_type amplitude:      %d\n", first, sample_type_amp);
  fprintf(stderr, "%s     sample_type phase:          %d\n", first, sample_type_phase);
  fprintf(stderr, "%s     sample_type I and Q:        %d\n", first, sample_type_iandq);
  fprintf(stderr, "%s     sample_type beamforming:    %d\n", first, sample_type_beamforming);

  for (int i = 0; i < beam->number_beams; i++) {
    s7kr_snippet *snippet = &beam->snippets[i];
    fprintf(stderr, "%s     beam[%d]:%d   begin_sample:%d end_sample:%d nalloc_amp:%d nalloc_phase:%d\n", first, i,
            snippet->beam_number, snippet->begin_sample, snippet->end_sample, snippet->nalloc_amp, snippet->nalloc_phase);
    const mb_u_char *ucharptramp = (mb_u_char *)snippet->amplitude;
    const mb_u_char *ucharptrphase = (mb_u_char *)snippet->phase;
    const unsigned short *ushortptramp = (unsigned short *)snippet->amplitude;
    const unsigned short *ushortptrphase = (unsigned short *)snippet->phase;
    const unsigned int *uintptramp = (unsigned int *)snippet->amplitude;
    const unsigned int *uintptrphase = (unsigned int *)snippet->phase;
    const short *shortptramp = (short *)snippet->amplitude;
    const short *shortptrphase = (short *)snippet->phase;
    const int *intptramp = (int *)snippet->amplitude;
    const int *intptrphase = (int *)snippet->phase;
    for (unsigned int j = 0; j <= snippet->end_sample - snippet->begin_sample; j++) {
      fprintf(stderr, "%s     sample[%d]:%d", first, j, snippet->begin_sample + j);
      if (sample_type_amp == 1)
        fprintf(stderr, "   amplitude:%d", ucharptramp[j]);
      else if (sample_type_amp == 2)
        fprintf(stderr, "   amplitude:%d", ushortptramp[j]);
      else if (sample_type_amp == 3)
        fprintf(stderr, "   amplitude:%d", uintptramp[j]);
      if (sample_type_phase == 1)
        fprintf(stderr, "   phase:%d", ucharptrphase[j]);
      else if (sample_type_phase == 2)
        fprintf(stderr, "   phase:%d", ushortptrphase[j]);
      else if (sample_type_phase == 3)
        fprintf(stderr, "   phase:%d", uintptrphase[j]);
      if (sample_type_iandq == 1)
        fprintf(stderr, "   amplitude:%d   phase:%d", shortptramp[j], shortptrphase[j]);
      else if (sample_type_iandq == 2)
        fprintf(stderr, "   amplitude:%d   phase:%d", intptramp[j], intptrphase[j]);
      fprintf(stderr, "\n");
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_verticaldepth(int verbose, s7kr_verticaldepth *verticaldepth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       verticaldepth:     %p\n", (void *)verticaldepth);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &verticaldepth->header, error);

  /* print Reson 7k vertical depth data (record 7009) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, verticaldepth->frequency);
  fprintf(stderr, "%s     ping_number:                %u\n", first, verticaldepth->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, verticaldepth->multi_ping);
  fprintf(stderr, "%s     latitude:                   %f\n", first, verticaldepth->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, verticaldepth->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, verticaldepth->heading);
  fprintf(stderr, "%s     alongtrack:                 %f\n", first, verticaldepth->alongtrack);
  fprintf(stderr, "%s     acrosstrack:                %f\n", first, verticaldepth->acrosstrack);
  fprintf(stderr, "%s     vertical_depth:             %f\n", first, verticaldepth->vertical_depth);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_tvg(int verbose, s7kr_tvg *tvg, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       tvg:               %p\n", (void *)tvg);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &tvg->header, error);

  /* print Reson 7k tvg data (record 7010) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, tvg->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, tvg->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, tvg->multi_ping);
  fprintf(stderr, "%s     n:                          %d\n", first, tvg->n);
  for (int i = 0; i < 8; i++)
    fprintf(stderr, "%s     reserved[%d]:                %d\n", first, i, tvg->reserved[i]);
  for (unsigned int i = 0; i < tvg->n; i++) {
    float *tvg_float = (float *)tvg->tvg;
    fprintf(stderr, "%s     tvg[%d]:  %f\n", first, i, tvg_float[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_image(int verbose, s7kr_image *image, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       image:             %p\n", (void *)image);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &image->header, error);

  /* print Reson 7k image imagery data (record 7007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     ping_number:                %u\n", first, image->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, image->multi_ping);
  fprintf(stderr, "%s     width:                      %d\n", first, image->width);
  fprintf(stderr, "%s     height:                     %d\n", first, image->height);
  fprintf(stderr, "%s     color_depth:                %d\n", first, image->color_depth);
  fprintf(stderr, "%s     width_height_flag:          %d\n", first, image->width_height_flag);
  fprintf(stderr, "%s     compression:                %d\n", first, image->compression);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, image->nalloc);
  if (image->color_depth == 1) {
    const mb_s_char *charptr = (mb_s_char *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++)
      fprintf(stderr, "%s     image[%d]:  %hhu\n", first, i, charptr[i]);
  }
  else if (image->color_depth == 2) {
    const short *shortptr = (short *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++)
      fprintf(stderr, "%s     image[%d]:  %hu\n", first, i, shortptr[i]);
  }
  else if (image->color_depth == 4) {
    const int *intptr = (int *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++)
      fprintf(stderr, "%s     image[%d]:  %u\n", first, i, intptr[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2pingmotion(int verbose, s7kr_v2pingmotion *v2pingmotion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2pingmotion:      %p\n", (void *)v2pingmotion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2pingmotion->header, error);

  /* print Reson 7k ping motion (record 7012) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2pingmotion->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2pingmotion->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2pingmotion->multi_ping);
  fprintf(stderr, "%s     n:                          %d\n", first, v2pingmotion->n);
  fprintf(stderr, "%s     flags:                      %d\n", first, v2pingmotion->flags);
  fprintf(stderr, "%s     error_flags:                %d\n", first, v2pingmotion->error_flags);
  fprintf(stderr, "%s     frequency:                  %f\n", first, v2pingmotion->frequency);
  fprintf(stderr, "%s     pitch:                      %f\n", first, v2pingmotion->pitch);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, v2pingmotion->nalloc);
  fprintf(stderr, "%s     beam  roll    heading    heave\n", first);
  fprintf(stderr, "%s     ----  ----    -------    -----\n", first);
  for (unsigned int i = 0; i < v2pingmotion->n; i++) {
    fprintf(stderr, "%s     %3d  %10g  %10g  %10g\n", first, i, v2pingmotion->roll[i], v2pingmotion->heading[i],
            v2pingmotion->heave[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2detectionsetup(int verbose, s7kr_v2detectionsetup *v2detectionsetup, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2detectionsetup:  %p\n", (void *)v2detectionsetup);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2detectionsetup->header, error);

  /* print Reson 7k detection setup (record 7017) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2detectionsetup->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2detectionsetup->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2detectionsetup->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, v2detectionsetup->number_beams);
  fprintf(stderr, "%s     data_field_size:            %d\n", first, v2detectionsetup->data_field_size);
  fprintf(stderr, "%s     detection_algorithm:        %d\n", first, v2detectionsetup->detection_algorithm);
  fprintf(stderr, "%s     detection_flags:            %d\n", first, v2detectionsetup->detection_flags);
  fprintf(stderr, "%s     minimum_depth:              %f\n", first, v2detectionsetup->minimum_depth);
  fprintf(stderr, "%s     maximum_depth:              %f\n", first, v2detectionsetup->maximum_depth);
  fprintf(stderr, "%s     minimum_range:              %f\n", first, v2detectionsetup->minimum_range);
  fprintf(stderr, "%s     maximum_range:              %f\n", first, v2detectionsetup->maximum_range);
  fprintf(stderr, "%s     minimum_nadir_search:       %f\n", first, v2detectionsetup->minimum_nadir_search);
  fprintf(stderr, "%s     maximum_nadir_search:       %f\n", first, v2detectionsetup->maximum_nadir_search);
  fprintf(stderr, "%s     automatic_filter_window:    %u\n", first, v2detectionsetup->automatic_filter_window);
  fprintf(stderr, "%s     applied_roll:               %f\n", first, v2detectionsetup->applied_roll);
  fprintf(stderr, "%s     depth_gate_tilt:            %f\n", first, v2detectionsetup->depth_gate_tilt);
  for (int i = 0; i < 14; i++) {
    fprintf(stderr, "%s     reserved[%2d]:               %f\n", first, i, v2detectionsetup->reserved[i]);
  }
  fprintf(stderr, "%s     beam  descriptor pick flag amin amax umin umax quality uncertainty\n", first);
  fprintf(stderr, "%s     ---------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < v2detectionsetup->number_beams; i++) {
    fprintf(stderr, "%s     %3d %u %10.3f %u %u %u %u %u %u %f\n", first, i, v2detectionsetup->beam_descriptor[i],
            v2detectionsetup->detection_point[i], v2detectionsetup->flags[i], v2detectionsetup->auto_limits_min_sample[i],
            v2detectionsetup->auto_limits_max_sample[i], v2detectionsetup->user_limits_min_sample[i],
            v2detectionsetup->user_limits_max_sample[i], v2detectionsetup->quality[i], v2detectionsetup->uncertainty[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2beamformed(int verbose, s7kr_v2beamformed *v2beamformed, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2beamformed:      %p\n", (void *)v2beamformed);
  }

  /* Reson 7k beamformed magnitude and phase data (record 7018) */
  mbsys_reson7k_print_header(verbose, &v2beamformed->header, error);

  /* print Reson 7k detection setup (record 7017) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2beamformed->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2beamformed->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2beamformed->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, v2beamformed->number_beams);
  fprintf(stderr, "%s     number_samples:             %d\n", first, v2beamformed->number_samples);
  fprintf(stderr, "%s     reserved:                   ", first);
  for (int i = 0; i < 32; i++)
    fprintf(stderr, "%u ", v2beamformed->reserved[i]);
  fprintf(stderr, "\n");
  for (int i = 0; i < v2beamformed->number_beams; i++) {
    s7kr_v2amplitudephase *v2amplitudephase = &(v2beamformed->amplitudephase[i]);
    fprintf(stderr, "%s     beam_number:                %d\n", first, v2amplitudephase->beam_number);
    fprintf(stderr, "%s     number_samples:             %d\n", first, v2amplitudephase->number_samples);
    for (unsigned int j = 0; j < v2amplitudephase->number_samples; j++) {
      fprintf(stderr, "%s     beam[%d] sample[%d] amplitude:%u phase:%d\n", first, i, j, v2amplitudephase->amplitude[j],
              v2amplitudephase->phase[j]);
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2bite(int verbose, s7kr_v2bite *v2bite, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2bite:      %p\n", (void *)v2bite);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2bite->header, error);

  /* Reson 7k BITE (record 7021) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     number_reports:             %u\n", first, v2bite->number_reports);
  for (int i = 0; i < v2bite->number_reports; i++) {
    s7kr_v2bitereport *v2bitereport = &(v2bite->reports[i]);
    fprintf(stderr, "%s     source_name:                %s\n", first, v2bitereport->source_name);
    fprintf(stderr, "%s     source_address:             %u\n", first, v2bitereport->source_address);
    fprintf(stderr, "%s     frequency:                  %f\n", first, v2bitereport->frequency);
    fprintf(stderr, "%s     enumerator:                 %u\n", first, v2bitereport->enumerator);
    s7k_time *s7ktime = &(v2bitereport->downlink_time);
    fprintf(stderr, "%s     downlink_time:              %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
            s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
    s7ktime = &(v2bitereport->uplink_time);
    fprintf(stderr, "%s     uplink_time:                %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
            s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
    s7ktime = &(v2bitereport->bite_time);
    fprintf(stderr, "%s     bite_time:                  %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n", first, s7ktime->Year, s7ktime->Day,
            s7ktime->Hours, s7ktime->Minutes, s7ktime->Seconds);
    fprintf(stderr, "%s     status:                     %u\n", first, v2bitereport->status);
    fprintf(stderr, "%s     number_bite:                %u\n", first, v2bitereport->number_bite);
    fprintf(stderr, "%s     bite_status:                ", first);
    for (int j = 0; j < 32; j++)
      fprintf(stderr, "%u ", v2bitereport->bite_status[j]);
    fprintf(stderr, "\n");
    for (int j = 0; j < v2bitereport->number_bite; j++) {
      s7kr_v2bitefield *v2bitefield = &(v2bitereport->bitefield[j]);
      fprintf(stderr, "%s     reserved[%2d]:               %u\n", first, j, v2bitefield->reserved);
      fprintf(stderr, "%s     name[%2d]:                   %s\n", first, j, v2bitefield->name);
      fprintf(stderr, "%s     device_type[%2d]:            %d\n", first, j, v2bitefield->device_type);
      fprintf(stderr, "%s     minimum[%2d]:                %f\n", first, j, v2bitefield->minimum);
      fprintf(stderr, "%s     maximum[%2d]:                %f\n", first, j, v2bitefield->maximum);
      fprintf(stderr, "%s     value[%2d]:                  %f\n", first, j, v2bitefield->value);
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v27kcenterversion(int verbose, s7kr_v27kcenterversion *v27kcenterversion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v27kcenterversion: %p\n", (void *)v27kcenterversion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v27kcenterversion->header, error);

  /* Reson 7k center version (record 7022) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     version:                    %s\n", first, v27kcenterversion->version);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v28kwetendversion(int verbose, s7kr_v28kwetendversion *v28kwetendversion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v28kwetendversion:      %p\n", (void *)v28kwetendversion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v28kwetendversion->header, error);

  /* Reson 7k 8k wet end version (record 7023) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     version:                    %s\n", first, v28kwetendversion->version);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2detection(int verbose, s7kr_v2detection *v2detection, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2detection:      %p\n", (void *)v2detection);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2detection->header, error);

  /* print Reson 7k version 2 detection (record 7026) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2detection->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2detection->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2detection->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, v2detection->number_beams);
  fprintf(stderr, "%s     data_field_size:            %d\n", first, v2detection->data_field_size);
  fprintf(stderr, "%s     corrections:                %llu\n", first, v2detection->corrections);
  fprintf(stderr, "%s     detection_algorithm:        %d\n", first, v2detection->detection_algorithm);
  fprintf(stderr, "%s     flags:                      %d\n", first, v2detection->flags);
  for (int i = 0; i < 64; i++)
    fprintf(stderr, "%u ", v2detection->reserved[i]);
  fprintf(stderr, "%s     beam  range angle_x angle_y range_error angle_x_error angle_y_error\n", first);
  fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < v2detection->number_beams; i++) {
    fprintf(stderr, "%s     %3d %f %f %f %f %f %f\n", first, i, v2detection->range[i], v2detection->angle_x[i],
            v2detection->angle_y[i], v2detection->range_error[i], v2detection->angle_x_error[i],
            v2detection->angle_y_error[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2rawdetection(int verbose, s7kr_v2rawdetection *v2rawdetection, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2rawdetection:      %p\n", (void *)v2rawdetection);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2rawdetection->header, error);

  /* print Reson 7k version 2 raw detection (record 7027) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2rawdetection->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2rawdetection->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2rawdetection->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, v2rawdetection->number_beams);
  fprintf(stderr, "%s     data_field_size:            %d\n", first, v2rawdetection->data_field_size);
  fprintf(stderr, "%s     detection_algorithm:        %d\n", first, v2rawdetection->detection_algorithm);
  fprintf(stderr, "%s     detection_flags:            %d\n", first, v2rawdetection->detection_flags);
  fprintf(stderr, "%s     sampling_rate:              %f\n", first, v2rawdetection->sampling_rate);
  fprintf(stderr, "%s     tx_angle:                   %f\n", first, v2rawdetection->tx_angle);
  fprintf(stderr, "%s     reserved:                   ", first);
  for (int i = 0; i < 64; i++)
    fprintf(stderr, "%u ", v2rawdetection->reserved[i]);
  fprintf(stderr, "\n%s     beam  beam_descriptor detection_point rx_angle flags quality uncertainty\n", first);
  fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
    fprintf(stderr, "%s     %3d %u %f %f %u %u %f\n", first, i, v2rawdetection->beam_descriptor[i],
            v2rawdetection->detection_point[i], v2rawdetection->rx_angle[i], v2rawdetection->flags[i],
            v2rawdetection->quality[i], v2rawdetection->uncertainty[i]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_v2snippet(int verbose, s7kr_v2snippet *v2snippet, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       v2snippet:      %p\n", (void *)v2snippet);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &v2snippet->header, error);

  /* print Reson 7k version 2 snippet (record 7028) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, v2snippet->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, v2snippet->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, v2snippet->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, v2snippet->number_beams);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, v2snippet->error_flag);
  fprintf(stderr, "%s     control_flags:              %u\n", first, v2snippet->control_flags);
  for (int i = 0; i < 28; i++)
    fprintf(stderr, "%u ", v2snippet->reserved[i]);
  for (int i = 0; i < v2snippet->number_beams; i++) {
    s7kr_v2snippettimeseries *v2snippettimeseries = &(v2snippet->snippettimeseries[i]);
    fprintf(stderr, "%s     beam: %u begin:%u detect:%u end:%u\n", first, v2snippettimeseries->beam_number,
            v2snippettimeseries->begin_sample, v2snippettimeseries->detect_sample, v2snippettimeseries->end_sample);
    for (unsigned int j = 0; j < v2snippettimeseries->end_sample - v2snippettimeseries->begin_sample + 1; j++)
      fprintf(stderr, "%s     amplitude[%d]:%d\n", first, v2snippettimeseries->begin_sample + j,
              v2snippettimeseries->amplitude[j]);
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_calibratedsnippet(int verbose, s7kr_calibratedsnippet *calibratedsnippet, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       calibratedsnippet:      %p\n", (void *)calibratedsnippet);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &calibratedsnippet->header, error);

  /* print Reson 7k version 2 snippet (record 7028) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, calibratedsnippet->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, calibratedsnippet->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, calibratedsnippet->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, calibratedsnippet->number_beams);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, calibratedsnippet->error_flag);
  fprintf(stderr, "%s     control_flags:              %u\n", first, calibratedsnippet->control_flags);
  fprintf(stderr, "%s     absorption:                 %f\n", first, calibratedsnippet->absorption);
  for (int i = 0; i < 6; i++)
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, calibratedsnippet->reserved[i]);
  for (int i = 0; i < calibratedsnippet->number_beams; i++) {
    s7kr_calibratedsnippettimeseries *calibratedsnippettimeseries =
        &(calibratedsnippet->calibratedsnippettimeseries[i]);
    fprintf(stderr, "%s     beam: %u begin:%u detect:%u end:%u\n", first, calibratedsnippettimeseries->beam_number,
            calibratedsnippettimeseries->begin_sample, calibratedsnippettimeseries->detect_sample,
            calibratedsnippettimeseries->end_sample);
    for (unsigned int j = 0; j < calibratedsnippettimeseries->end_sample - calibratedsnippettimeseries->begin_sample + 1; j++)
      fprintf(stderr, "%s     snippet amplitude[%d]:%f\n", first, calibratedsnippettimeseries->begin_sample + j,
              calibratedsnippettimeseries->amplitude[j]);
    if (calibratedsnippet->control_flags & 0x40 && calibratedsnippettimeseries->footprints != NULL) {
      for (unsigned int j = 0; j < calibratedsnippettimeseries->end_sample - calibratedsnippettimeseries->begin_sample + 1; j++)
        fprintf(stderr, "%s     footprints[%d]:%f\n", first, calibratedsnippettimeseries->begin_sample + j,
                calibratedsnippettimeseries->footprints[j]);
      }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_installation(int verbose, s7kr_installation *installation, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       installation:      %p\n", (void *)installation);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &installation->header, error);

  /* print Reson 7k sonar installation parameters (record 7051) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, installation->frequency);
  fprintf(stderr, "%s     firmware_version_len:       %d\n", first, installation->firmware_version_len);
  fprintf(stderr, "%s     firmware_version:           %s\n", first, installation->firmware_version);
  fprintf(stderr, "%s     software_version_len:       %d\n", first, installation->software_version_len);
  fprintf(stderr, "%s     software_version:           %s\n", first, installation->software_version);
  fprintf(stderr, "%s     s7k_version_len:            %d\n", first, installation->s7k_version_len);
  fprintf(stderr, "%s     s7k_version:                %s\n", first, installation->s7k_version);
  fprintf(stderr, "%s     protocal_version_len:       %d\n", first, installation->protocal_version_len);
  fprintf(stderr, "%s     protocal_version:           %s\n", first, installation->protocal_version);
  fprintf(stderr, "%s     transmit_x:                 %f\n", first, installation->transmit_x);
  fprintf(stderr, "%s     transmit_y:                 %f\n", first, installation->transmit_y);
  fprintf(stderr, "%s     transmit_z:                 %f\n", first, installation->transmit_z);
  fprintf(stderr, "%s     transmit_roll:              %f\n", first, installation->transmit_roll);
  fprintf(stderr, "%s     transmit_pitch:             %f\n", first, installation->transmit_pitch);
  fprintf(stderr, "%s     transmit_heading:           %f\n", first, installation->transmit_heading);
  fprintf(stderr, "%s     transmit_x:                 %f\n", first, installation->transmit_x);
  fprintf(stderr, "%s     transmit_x:                 %f\n", first, installation->transmit_x);
  fprintf(stderr, "%s     receive_x:                  %f\n", first, installation->receive_x);
  fprintf(stderr, "%s     receive_y:                  %f\n", first, installation->receive_y);
  fprintf(stderr, "%s     receive_z:                  %f\n", first, installation->receive_z);
  fprintf(stderr, "%s     receive_roll:               %f\n", first, installation->receive_roll);
  fprintf(stderr, "%s     receive_pitch:              %f\n", first, installation->receive_pitch);
  fprintf(stderr, "%s     receive_heading:            %f\n", first, installation->receive_heading);
  fprintf(stderr, "%s     receive_x:                  %f\n", first, installation->receive_x);
  fprintf(stderr, "%s     receive_x:                  %f\n", first, installation->receive_x);
  fprintf(stderr, "%s     motion_x:                   %f\n", first, installation->motion_x);
  fprintf(stderr, "%s     motion_y:                   %f\n", first, installation->motion_y);
  fprintf(stderr, "%s     motion_z:                   %f\n", first, installation->motion_z);
  fprintf(stderr, "%s     motion_roll:                %f\n", first, installation->motion_roll);
  fprintf(stderr, "%s     motion_pitch:               %f\n", first, installation->motion_pitch);
  fprintf(stderr, "%s     motion_heading:             %f\n", first, installation->motion_heading);
  fprintf(stderr, "%s     motion_x:                   %f\n", first, installation->motion_x);
  fprintf(stderr, "%s     motion_x:                   %f\n", first, installation->motion_x);
  fprintf(stderr, "%s     motion_time_delay:          %d\n", first, installation->motion_time_delay);
  fprintf(stderr, "%s     position_x:                 %f\n", first, installation->position_x);
  fprintf(stderr, "%s     position_y:                 %f\n", first, installation->position_y);
  fprintf(stderr, "%s     position_z:                 %f\n", first, installation->position_z);
  fprintf(stderr, "%s     position_time_delay:        %d\n", first, installation->position_time_delay);
  fprintf(stderr, "%s     waterline_z:                %f\n", first, installation->waterline_z);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_systemeventmessage(int verbose, s7kr_systemeventmessage *systemeventmessage, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       systemeventmessage:%p\n", (void *)systemeventmessage);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &systemeventmessage->header, error);

  /* print Reson 7k system event (record 7051) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, systemeventmessage->serial_number);
  fprintf(stderr, "%s     event_id:                   %d\n", first, systemeventmessage->event_id);
  fprintf(stderr, "%s     message_length:             %d\n", first, systemeventmessage->message_length);
  fprintf(stderr, "%s     event_identifier:           %d\n", first, systemeventmessage->event_identifier);
  fprintf(stderr, "%s     message_alloc:              %d\n", first, systemeventmessage->message_alloc);
  fprintf(stderr, "%s     message:                    %s\n", first, systemeventmessage->message);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_subsystem(int verbose, s7kr_subsystem *subsystem, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       subsystem:         %p\n", (void *)subsystem);
  }

  /* print Reson 7k subsystem structure */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     device_identifier:          %d\n", first, subsystem->device_identifier);
  fprintf(stderr, "%s     system_enumerator:          %d\n", first, subsystem->system_enumerator);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_fileheader(int verbose, s7kr_fileheader *fileheader, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       fileheader:        %p\n", (void *)fileheader);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &fileheader->header, error);

  /* print Reson 7k system event (record 7051) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     file_identifier:            0x", first);
  for (int i = 0; i < 16; i++)
    fprintf(stderr, "%hhx", fileheader->file_identifier[i]);
  fprintf(stderr, "\n");
  fprintf(stderr, "%s     version:                    %d\n", first, fileheader->version);
  fprintf(stderr, "%s     reserved:                   %d\n", first, fileheader->reserved);
  fprintf(stderr, "%s     session_identifier:         %s\n", first, fileheader->session_identifier);
  fprintf(stderr, "%s     record_data_size:           %d\n", first, fileheader->record_data_size);
  fprintf(stderr, "%s     number_subsystems:          %d\n", first, fileheader->number_subsystems);
  fprintf(stderr, "%s     recording_name:             %s\n", first, fileheader->recording_name);
  fprintf(stderr, "%s     recording_version:          %s\n", first, fileheader->recording_version);
  fprintf(stderr, "%s     user_defined_name:          %s\n", first, fileheader->user_defined_name);
  fprintf(stderr, "%s     notes:                      %s\n", first, fileheader->notes);
  for (unsigned int i = 0; i < fileheader->number_subsystems; i++)
    mbsys_reson7k_print_subsystem(verbose, &fileheader->subsystem[i], error);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_remotecontrolsettings(int verbose, s7kr_remotecontrolsettings *remotecontrolsettings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       remotecontrolsettings:  %p\n", (void *)remotecontrolsettings);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &remotecontrolsettings->header, error);

  /* print Reson 7k remote control sonar settings (record 7503) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, remotecontrolsettings->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, remotecontrolsettings->ping_number);
  fprintf(stderr, "%s     frequency:                  %f\n", first, remotecontrolsettings->frequency);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, remotecontrolsettings->sample_rate);
  fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, remotecontrolsettings->receiver_bandwidth);
  fprintf(stderr, "%s     pulse_width:                %f\n", first, remotecontrolsettings->pulse_width);
  fprintf(stderr, "%s     pulse_type:                 %d\n", first, remotecontrolsettings->pulse_type);
  fprintf(stderr, "%s     pulse_envelope:             %d\n", first, remotecontrolsettings->pulse_envelope);
  fprintf(stderr, "%s     pulse_envelope_par:         %f\n", first, remotecontrolsettings->pulse_envelope_par);
  fprintf(stderr, "%s     pulse_reserved:             %d\n", first, remotecontrolsettings->pulse_reserved);
  fprintf(stderr, "%s     max_ping_rate:              %f\n", first, remotecontrolsettings->max_ping_rate);
  fprintf(stderr, "%s     ping_period:                %f\n", first, remotecontrolsettings->ping_period);
  fprintf(stderr, "%s     range_selection:            %f\n", first, remotecontrolsettings->range_selection);
  fprintf(stderr, "%s     power_selection:            %f\n", first, remotecontrolsettings->power_selection);
  fprintf(stderr, "%s     gain_selection:             %f\n", first, remotecontrolsettings->gain_selection);
  fprintf(stderr, "%s     control_flags:              %d\n", first, remotecontrolsettings->control_flags);
  fprintf(stderr, "%s     projector_magic_no:         %d\n", first, remotecontrolsettings->projector_magic_no);
  fprintf(stderr, "%s     steering_vertical:          %f\n", first, remotecontrolsettings->steering_vertical);
  fprintf(stderr, "%s     steering_horizontal:        %f\n", first, remotecontrolsettings->steering_horizontal);
  fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, remotecontrolsettings->beamwidth_vertical);
  fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, remotecontrolsettings->beamwidth_horizontal);
  fprintf(stderr, "%s     focal_point:                %f\n", first, remotecontrolsettings->focal_point);
  fprintf(stderr, "%s     projector_weighting:        %d\n", first, remotecontrolsettings->projector_weighting);
  fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, remotecontrolsettings->projector_weighting_par);
  fprintf(stderr, "%s     transmit_flags:             %d\n", first, remotecontrolsettings->transmit_flags);
  fprintf(stderr, "%s     hydrophone_magic_no:        %d\n", first, remotecontrolsettings->hydrophone_magic_no);
  fprintf(stderr, "%s     receive_weighting:          %d\n", first, remotecontrolsettings->receive_weighting);
  fprintf(stderr, "%s     receive_weighting_par:      %f\n", first, remotecontrolsettings->receive_weighting_par);
  fprintf(stderr, "%s     receive_flags:              %d\n", first, remotecontrolsettings->receive_flags);
  fprintf(stderr, "%s     range_minimum:              %f\n", first, remotecontrolsettings->range_minimum);
  fprintf(stderr, "%s     range_maximum:              %f\n", first, remotecontrolsettings->range_maximum);
  fprintf(stderr, "%s     depth_minimum:              %f\n", first, remotecontrolsettings->depth_minimum);
  fprintf(stderr, "%s     depth_maximum:              %f\n", first, remotecontrolsettings->depth_maximum);
  fprintf(stderr, "%s     absorption:                 %f\n", first, remotecontrolsettings->absorption);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, remotecontrolsettings->sound_velocity);
  fprintf(stderr, "%s     spreading:                  %f\n", first, remotecontrolsettings->spreading);
  fprintf(stderr, "%s     reserved:                   %d\n", first, remotecontrolsettings->reserved);
  fprintf(stderr, "%s     tx_offset_x:                %f\n", first, remotecontrolsettings->tx_offset_x);
  fprintf(stderr, "%s     tx_offset_y:                %f\n", first, remotecontrolsettings->tx_offset_y);
  fprintf(stderr, "%s     tx_offset_z:                %f\n", first, remotecontrolsettings->tx_offset_z);
  fprintf(stderr, "%s     head_tilt_x:                %f\n", first, remotecontrolsettings->head_tilt_x);
  fprintf(stderr, "%s     head_tilt_y:                %f\n", first, remotecontrolsettings->head_tilt_y);
  fprintf(stderr, "%s     head_tilt_z:                %f\n", first, remotecontrolsettings->head_tilt_z);
  fprintf(stderr, "%s     ping_on_off:                %d\n", first, remotecontrolsettings->ping_on_off);
  fprintf(stderr, "%s     data_sample_types:          %d\n", first, remotecontrolsettings->data_sample_types);
  fprintf(stderr, "%s     projector_orientation:      %d\n", first, remotecontrolsettings->projector_orientation);
  fprintf(stderr, "%s     beam_angle_mode:            %d\n", first, remotecontrolsettings->beam_angle_mode);
  fprintf(stderr, "%s     r7kcenter_mode:             %d\n", first, remotecontrolsettings->r7kcenter_mode);
  fprintf(stderr, "%s     gate_depth_min:             %f\n", first, remotecontrolsettings->gate_depth_min);
  fprintf(stderr, "%s     gate_depth_max:             %f\n", first, remotecontrolsettings->gate_depth_max);
  for (int i = 0; i < 35; i++)
    fprintf(stderr, "%s     reserved2[i]:               %d\n", first, remotecontrolsettings->reserved2[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_reserved(int verbose, s7kr_reserved *reserved, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       reserved:          %p\n", (void *)reserved);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &reserved->header, error);

  /* print Reson 7k Reserved (well, unknown really...) (record 7504) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  for (int i = 0; i < R7KHDRSIZE_7kReserved; i++)
    fprintf(stderr, "%s     reserved[%d]:               %u\n", first, i, reserved->reserved[i]);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_roll(int verbose, s7kr_roll *roll, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       roll:              %p\n", (void *)roll);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &roll->header, error);

  /* print Reson 7k Roll (record 7600) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     roll:                       %f\n", first, roll->roll);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_pitch(int verbose, s7kr_pitch *pitch, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       pitch:             %p\n", (void *)pitch);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &pitch->header, error);

  /* print Reson 7k Pitch (record 7601) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     pitch:                      %f\n", first, pitch->pitch);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_soundvelocity(int verbose, s7kr_soundvelocity *soundvelocity, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       soundvelocity:     %p\n", (void *)soundvelocity);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &soundvelocity->header, error);

  /* print Reson 7k Sound Velocity (record 7610) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     soundvelocity:              %f\n", first, soundvelocity->soundvelocity);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_absorptionloss(int verbose, s7kr_absorptionloss *absorptionloss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       absorptionloss:    %p\n", (void *)absorptionloss);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &absorptionloss->header, error);

  /* print Reson 7k Absorption Loss (record 7611) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     absorptionloss:             %f\n", first, absorptionloss->absorptionloss);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_print_spreadingloss(int verbose, s7kr_spreadingloss *spreadingloss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       spreadingloss:     %p\n", (void *)spreadingloss);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k_print_header(verbose, &spreadingloss->header, error);

  /* print Reson 7k Spreading Loss (record 7611) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     spreadingloss:              %f\n", first, spreadingloss->spreadingloss);

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get beam and pixel numbers */
    s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
    *nbath = bathymetry->number_beams;
    *namp = *nbath;
    *nss = 0;
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
    fprintf(stderr, "dbg2        namp:      %d\n", *namp);
    fprintf(stderr, "dbg2        nss:       %d\n", *nss);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)mb_io_ptr->store_data;

  /* extract data from structure */
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  *pingnumber = bathymetry->ping_number;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       pingnumber: %u\n", *pingnumber);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get sonar type */
  *sonartype = MB_TOPOGRAPHY_TYPE_MULTIBEAM;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       sonartype:  %d\n", *sonartype);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get sidescan type */
  *ss_type = MB_SIDESCAN_LINEAR;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       ss_type:    %d\n", *ss_type);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_reson7k.h:/^struct mbsys_reson7k_struct/ */
                             void *platform_ptr, void *preprocess_pars_ptr, int *error) {

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

  /* get data structure pointers */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

  /* get saved values */
  double *pixel_size = (double *)&mb_io_ptr->saved1;
  double *swath_width = (double *)&mb_io_ptr->saved2;

  bool kluge_beampatternsnell = false;
  bool kluge_soundspeedsnell = false;
  bool kluge_zeroattitudecorrection = false;
  bool kluge_zeroalongtrackangles = false;

  /* kluge parameters */
  double kluge_beampatternsnellfactor = 1.0;
  double kluge_soundspeedsnellfactor = 1.0;

  /* get kluges */
  for (int i = 0; i < pars->n_kluge; i++) {
    if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
      kluge_beampatternsnell = true;
      kluge_beampatternsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
      kluge_soundspeedsnell = true;
      kluge_soundspeedsnellfactor = *((double *)&pars->kluge_pars[i * MB_PR_KLUGE_PAR_SIZE]);
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROATTITUDECORRECTION) {
      kluge_zeroattitudecorrection = true;
    }
    else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
      kluge_zeroalongtrackangles = true;
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
    fprintf(stderr, "dbg2       n_kluge:                       %d\n", pars->n_kluge);
    for (int i = 0; i < pars->n_kluge; i++) {
      fprintf(stderr, "dbg2       kluge_id[%d]:                    %d\n", i, pars->kluge_id[i]);
      if (pars->kluge_id[i] == MB_PR_KLUGE_BEAMTWEAK) {
        fprintf(stderr, "dbg2       kluge_beampatternsnell:        %d\n", kluge_beampatternsnell);
        fprintf(stderr, "dbg2       kluge_beampatternsnellfactor:  %f\n", kluge_beampatternsnellfactor);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_SOUNDSPEEDTWEAK) {
        fprintf(stderr, "dbg2       kluge_soundspeedsnell:         %d\n", kluge_soundspeedsnell);
        fprintf(stderr, "dbg2       kluge_soundspeedsnellfactor:   %f\n", kluge_soundspeedsnellfactor);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROATTITUDECORRECTION) {
        fprintf(stderr, "dbg2       kluge_zeroattitudecorrection:  %d\n", kluge_zeroattitudecorrection);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
        fprintf(stderr, "dbg2       kluge_zeroalongtrackangles:    %d\n", kluge_zeroalongtrackangles);
      }
    }
  }

  int status = MB_SUCCESS;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

  /* deal with a survey record */
  else if (store->kind == MB_DATA_DATA) {

    s7k_header *header = NULL;
    int time_i[7];
    int time_j[5];
    double time_d = 0.0;

    s7kr_volatilesettings *volatilesettings = &(store->volatilesettings);

    /* print out record headers */
    if (store->read_volatilesettings) {
      header = &(volatilesettings->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kVolatileSonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
    }
    if (store->read_matchfilter) {
      s7kr_matchfilter *matchfilter = &(store->matchfilter);
      header = &(matchfilter->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kMatchFilter:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
    }

    s7kr_beamgeometry *beamgeometry = &(store->beamgeometry);
    if (store->read_beamgeometry) {
      header = &(beamgeometry->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kBeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                beamgeometry->number_beams);
    }

    s7kr_remotecontrolsettings *remotecontrolsettings =
        &(store->remotecontrolsettings);
    if (store->read_remotecontrolsettings) {
      header = &(remotecontrolsettings->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kremotecontrolsettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber);
    }
    if (store->read_backscatter) {
      s7kr_backscatter *backscatter = &(store->backscatter);
      header = &(backscatter->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u "
                "ping:%u samples:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                backscatter->ping_number, backscatter->number_samples);
    }
    if (store->read_beam) {
      s7kr_beam *beam = &(store->beam);
      header = &(beam->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u ping:%u "
                "beams:%d samples:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                beam->ping_number, beam->number_beams, beam->number_samples);
    }
    if (store->read_verticaldepth) {
      s7kr_verticaldepth *verticaldepth = &(store->verticaldepth);
      header = &(verticaldepth->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kVerticalDepth: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u ping:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                verticaldepth->ping_number);
    }
    if (store->read_image) {
      s7kr_image *image = &(store->image);
      header = &(image->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u "
                "ping:%u width:%u height:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                image->ping_number, image->width, image->height);
    }

    if (store->read_bathymetry != true) {
      status = MB_FAILURE;
      *error = MB_ERROR_IGNORE;
    } else {
      s7kr_bathymetry *bathymetry = &(store->bathymetry);

      header = &(bathymetry->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);

      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%u "
                "ping:%u beams:%u\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], header->RecordNumber,
                bathymetry->ping_number, bathymetry->number_beams);

      /*--------------------------------------------------------------*/
      /* apply any required fixes to survey data beam flags */
      /*--------------------------------------------------------------*/

      /* fix version 4 quality flags */
      if (bathymetry->header.Version < 5) {
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          if ((bathymetry->quality[i]) < 16) {
            if (bathymetry->range[i] > 0.007) {
              bathymetry->quality[i] = 23;
            }
            else if (bathymetry->range[i] > 0.0) {
              bathymetry->quality[i] = 20;
            }
            else {
              bathymetry->quality[i] = 0;
            }
          }
        }
      }

      /* fix early version 5 quality flags */
      else if (bathymetry->header.Version == 5 && header->s7kTime.Year < 2006) {
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          /* phase picks */
          if ((bathymetry->quality[i]) == 8) {
            bathymetry->quality[i] = 32 + 15;
          }
          else if ((bathymetry->quality[i]) == 4) {
            bathymetry->quality[i] = 16 + 15;
          }
        }
      }

      /* fix early MBARI version 5 quality flags */
      else if (bathymetry->header.Version == 5 && store->nrec_bluefinnav > 0 && header->s7kTime.Year < 2008) {
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          /* phase picks */
          if ((bathymetry->quality[i]) == 4) {
            bathymetry->quality[i] = 32 + 15;
          }
          else if ((bathymetry->quality[i]) == 2) {
            bathymetry->quality[i] = 16 + 15;
          }
        }
      }

      /* fix upgraded MBARI version 5 quality flags */
      else if (bathymetry->header.Version >= 5 && store->nrec_bluefinnav > 0 && header->s7kTime.Year <= 2010) {
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          bathymetry->quality[i] = bathymetry->quality[i] & 15;

          /* phase or amplitude picks */
          if (bathymetry->quality[i] & 8) {
            bathymetry->quality[i] += 32;
          }
          else if (bathymetry->quality[i] & 4) {
            bathymetry->quality[i] += 16;
          }

          /* flagged by sonar */
          if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0) {
            bathymetry->quality[i] += 64;
          }
        }
      }

      /* fix upgraded version 5 quality flags */
      else if (bathymetry->header.Version >= 5) {
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          bathymetry->quality[i] = bathymetry->quality[i] & 15;

          /* phase or amplitude picks */
          if (bathymetry->quality[i] & 8) {
            bathymetry->quality[i] += 32;
          }
          else if (bathymetry->quality[i] & 4) {
            bathymetry->quality[i] += 16;
          }

          /* flagged by sonar */
          if ((bathymetry->quality[i] & 3) == 3
            && pars->sounding_amplitude_filter
            && (double)bathymetry->intensity[i] < pars->sounding_amplitude_threshold) {
            bathymetry->quality[i] += 64;
          }
          else if ((bathymetry->quality[i] & 3) == 3) {
          }
          else if ((bathymetry->quality[i] & 3) == 0 && bathymetry->quality[i] > 0) {
            bathymetry->quality[i] += 64;
          }
          else if (bathymetry->quality[i] > 0) {
            bathymetry->quality[i] += 64;
          }
        }
      }

      /* if requested ignore water column data
       * (will not be included in any output file) */
      if (pars->ignore_water_column
        && store->read_v2beamformed)
        store->read_v2beamformed = false;

      s7k_time s7kTime;

      /*--------------------------------------------------------------*/
      /* change timestamp if indicated */
      /*--------------------------------------------------------------*/
      if (pars->timestamp_changed) {
        time_d = pars->time_d;
        mb_get_date(verbose, time_d, time_i);
        mb_get_jtime(verbose, time_i, time_j);
        s7kTime.Year = time_i[0];
        s7kTime.Day = time_j[1];
        s7kTime.Hours = time_i[3];
        s7kTime.Minutes = time_i[4];
        s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
        if (verbose > 1)
          fprintf(stderr, "Timestamp changed in function %s: "
                "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d | ping_number:%u\n",
                __func__, time_i[0], time_i[1], time_i[2], time_i[3],
                time_i[4], time_i[5], time_i[6], bathymetry->ping_number);

        /* apply the timestamp to all of the relevant data records */
        if (store->read_volatilesettings)
          store->volatilesettings.header.s7kTime = s7kTime;
        if (store->read_matchfilter)
          store->matchfilter.header.s7kTime = s7kTime;
        if (store->read_beamgeometry)
          store->beamgeometry.header.s7kTime = s7kTime;
        if (store->read_remotecontrolsettings)
          store->remotecontrolsettings.header.s7kTime = s7kTime;
        if (store->read_bathymetry)
          store->bathymetry.header.s7kTime = s7kTime;
        if (store->read_backscatter)
          store->backscatter.header.s7kTime = s7kTime;
        if (store->read_beam)
          store->beam.header.s7kTime = s7kTime;
        if (store->read_verticaldepth)
          store->verticaldepth.header.s7kTime = s7kTime;
        if (store->read_image)
          store->image.header.s7kTime = s7kTime;
        if (store->read_v2pingmotion)
          store->v2pingmotion.header.s7kTime = s7kTime;
        if (store->read_v2detectionsetup)
          store->v2detectionsetup.header.s7kTime = s7kTime;
        if (store->read_v2beamformed)
          store->v2beamformed.header.s7kTime = s7kTime;
        if (store->read_v2detection)
          store->v2detection.header.s7kTime = s7kTime;
        if (store->read_v2rawdetection)
          store->v2rawdetection.header.s7kTime = s7kTime;
        if (store->read_v2snippet)
          store->v2snippet.header.s7kTime = s7kTime;
        if (store->read_calibratedsnippet)
          store->calibratedsnippet.header.s7kTime = s7kTime;
        if (store->read_processedsidescan)
          store->processedsidescan.header.s7kTime = s7kTime;
      }

      /*--------------------------------------------------------------*/
      /* interpolate ancillary values  */
      /*--------------------------------------------------------------*/
      int interp_status = MB_SUCCESS;
      int interp_error = MB_ERROR_NO_ERROR;

      double navlon = 0.0;
      double navlat = 0.0;
      int jnav = 0;
      interp_status &= mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
                                                 &navlon, &jnav, &interp_error);
      interp_status &= mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
                                                &navlat, &jnav, &interp_error);
      double speed = 0.0;
      interp_status &= mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed,
                                       &jnav, &interp_error);

      /* interpolate sensordepth */
      double sensordepth = 0.0;
      int jsensordepth = 0;
      interp_status &= mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                       pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);

      /* interpolate heading */
      int jheading = 0;
      double heading;
      interp_status &= mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                               pars->n_heading, time_d, &heading, &jheading, &interp_error);

      /* interpolate altitude */
      int jaltitude = 0;
      double altitude = 0.0;
      interp_status &= mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
                                       time_d, &altitude, &jaltitude, &interp_error);

      /* interpolate attitude */
      int jattitude = 0;
      double roll;
      interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                       time_d, &roll, &jattitude, &interp_error);
      double pitch;
      interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                       time_d, &pitch, &jattitude, &interp_error);
      double heave = 0.0;
      interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                       time_d, &heave, &jattitude, &interp_error);

      /* interpolate soundspeed */
      int jsoundspeed = 0;
      double soundspeednew;
      interp_status &= mb_linear_interp(verbose, pars->soundspeed_time_d - 1, pars->soundspeed_soundspeed - 1, pars->n_soundspeed,
                                       time_d, &soundspeednew, &jsoundspeed, &interp_error);

      /* do lever arm correction */
      double pitchc;
      double rollc;
      double headingc;
      if (platform != NULL) {
        /* calculate sonar position position */
        status &= mb_platform_position(verbose, (void *)platform, pars->target_sensor, 0, navlon, navlat, sensordepth,
                                      heading, roll, pitch, &navlon, &navlat, &sensordepth, error);

        /* calculate sonar attitude */
        status &= mb_platform_orientation_target(verbose, (void *)platform, pars->target_sensor, 0, heading, roll, pitch,
                                                &headingc, &rollc, &pitchc, error);
      }

      /* get local translation between lon lat degrees and meters */
      double mtodeglon;
      double mtodeglat;
      mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
      // const double headingx = sin(DTR * headingc);
      // const double headingy = cos(DTR * headingc);

      /* if a valid speed is not available calculate it */
      if (interp_status == MB_SUCCESS && speed <= 0.0) {
        int j1;
        int j2;
        if (jnav > 1) {
          j1 = jnav - 2;
          j2 = jnav - 1;
        }
        else {
          j1 = jnav - 1;
          j2 = jnav;
        }
        const double dx = (pars->nav_lon[j2] - pars->nav_lon[j1]) / mtodeglon;
        const double dy = (pars->nav_lat[j2] - pars->nav_lat[j1]) / mtodeglat;
        const double dt = (pars->nav_time_d[j2] - pars->nav_time_d[j1]);
        if (dt > 0.0)
          speed = sqrt(dx * dx + dy * dy) / dt;
      }

      mb_3D_orientation tx_align;
      mb_3D_orientation tx_orientation;
      mb_3D_orientation rx_align;
      mb_3D_orientation rx_orientation;
      double beamAzimuth;
      double beamDepression;

      double beamheading = 0.0;
      double beamroll = 0.0;
      double beampitch = 0.0;
      double beamheave;
      s7kr_v2detection *v2detection = &(store->v2detection);
      s7kr_v2detectionsetup *v2detectionsetup = &(store->v2detectionsetup);
      s7kr_v2rawdetection *v2rawdetection = &(store->v2rawdetection);
      s7kr_bluefin *bluefin = &(store->bluefin);

      /* if the optional data are not all available, this ping
          is not useful, and is discarded by setting
          *error to MB_ERROR_MISSING_NAVATTITUDE */
      //if (interp_status == MB_FAILURE) {
      //  status = MB_FAILURE;
      //  *error = MB_ERROR_MISSING_NAVATTITUDE;
      //}

      /*--------------------------------------------------------------*/
      /* recalculate bathymetry  */
      /*--------------------------------------------------------------*/
      if (status == MB_SUCCESS && (!bathymetry->optionaldata || pars->recalculate_bathymetry)) {

        if (verbose >= 2) {
          fprintf(stderr, "\ndbg2 Recalculating bathymetry in %s: 7k ping records read:\n", __func__);
          fprintf(stderr, "dbg2      current_ping_number:           %d\n", store->current_ping_number);
          fprintf(stderr, "dbg2      read_volatilesettings:         %d\n", store->read_volatilesettings);
          fprintf(stderr, "dbg2      read_matchfilter:              %d\n", store->read_matchfilter);
          fprintf(stderr, "dbg2      read_beamgeometry:             %d\n", store->read_beamgeometry);
          fprintf(stderr, "dbg2      read_remotecontrolsettings:    %d\n", store->read_remotecontrolsettings);
          fprintf(stderr, "dbg2      read_bathymetry:               %d\n", store->read_bathymetry);
          fprintf(stderr, "dbg2      read_backscatter:              %d\n", store->read_backscatter);
          fprintf(stderr, "dbg2      read_beam:                     %d\n", store->read_beam);
          fprintf(stderr, "dbg2      read_verticaldepth:            %d\n", store->read_verticaldepth);
          fprintf(stderr, "dbg2      read_tvg:                      %d\n", store->read_tvg);
          fprintf(stderr, "dbg2      read_image:                    %d\n", store->read_image);
          fprintf(stderr, "dbg2      read_v2pingmotion:             %d\n", store->read_v2pingmotion);
          fprintf(stderr, "dbg2      read_v2detectionsetup:         %d\n", store->read_v2detectionsetup);
          fprintf(stderr, "dbg2      read_v2beamformed:             %d\n", store->read_v2beamformed);
          fprintf(stderr, "dbg2      read_v2detection:              %d\n", store->read_v2detection);
          fprintf(stderr, "dbg2      read_v2rawdetection:           %d\n", store->read_v2rawdetection);
          fprintf(stderr, "dbg2      read_v2snippet:                %d\n", store->read_v2snippet);
          fprintf(stderr, "dbg2      read_calibratedsnippet:        %d\n", store->read_calibratedsnippet);
          fprintf(stderr, "dbg2      read_processedsidescan:        %d\n", store->read_processedsidescan);
        }

        /* initialize all of the beams */
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          if (store->read_v2rawdetection ||
              (store->read_v2detection && store->read_v2detectionsetup))
            bathymetry->quality[i] = 0;
          bathymetry->depth[i] = 0.0;
          bathymetry->acrosstrack[i] = 0.0;
          bathymetry->alongtrack[i] = 0.0;
          bathymetry->pointing_angle[i] = 0.0;
          bathymetry->azimuth_angle[i] = 0.0;
        }

        /* set ping values */
        bathymetry->longitude = DTR * navlon;
        bathymetry->latitude = DTR * navlat;
        bathymetry->heading = DTR * headingc;
        bathymetry->height_source = 1;
        bathymetry->tide = 0.0;
        bathymetry->roll = DTR * rollc;
        bathymetry->pitch = DTR * pitchc;
        bathymetry->heave = heave;
        if ((volatilesettings->receive_flags & 0x2) != 0) {
          bathymetry->vehicle_height = -sensordepth - heave;
        }
        else {
          bathymetry->vehicle_height = -sensordepth;
        }

        /* get ready to calculate bathymetry */
        double soundspeed;
        if (volatilesettings->sound_velocity > 0.0)
          soundspeed = volatilesettings->sound_velocity;
        else if (remotecontrolsettings->sound_velocity > 0.0)
          soundspeed = remotecontrolsettings->sound_velocity;
        else if (bathymetry->sound_velocity > 0.0)
          soundspeed = bathymetry->sound_velocity;
        else if (bluefin->environmental[0].sound_speed > 0.0)
          soundspeed = bluefin->environmental[0].sound_speed;
        else
          soundspeed = 1500.0;

        /* zero atttitude correction if requested */
        if (kluge_zeroattitudecorrection) {
          roll = 0.0;
          pitch = 0.0;
        }

        /* zero alongtrack angles if requested */
        if (kluge_zeroalongtrackangles) {
          for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
            beamgeometry->angle_alongtrack[i] = 0.0;
          }
        }

        /* if requested apply kluge scaling of rx beam angles */
        if (kluge_beampatternsnell) {
          /*
           * v2rawdetection record
           */
          if (store->read_v2rawdetection) {
            for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
              v2rawdetection->rx_angle[i]
                = asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
                           * sin(v2rawdetection->rx_angle[i]))));
            }
          }

          /*
           * v2detection record with or without v2detectionsetup
           */
          if (store->read_v2detection) {
            for (unsigned int i = 0; i < v2detection->number_beams; i++) {
              v2detection->angle_x[i]
                = asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
                           * sin(v2detection->angle_x[i]))));
            }
          }

          /*
           * beamgeometry record
           */
          if (store->read_beamgeometry) {
            for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
              beamgeometry->angle_acrosstrack[i] =
                  asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
                           * sin(beamgeometry->angle_acrosstrack[i]))));
            }
          }
        }

        /* Change the sound speed used to calculate bathymetry */
        double soundspeedsnellfactor = 1.0;
        if (pars->modify_soundspeed && pars->n_soundspeed > 0) {
          soundspeedsnellfactor = soundspeednew / soundspeed;
          soundspeed = soundspeednew;
          bathymetry->sound_velocity = soundspeed;
        }

        /* if requested apply kluge scaling of sound speed - which means
            changing beam angles by Snell's law and changing the sound
            speed used to calculate bathymetry */
        if (kluge_soundspeedsnell) {
          /*
           * sound speed
           */
          soundspeedsnellfactor *= kluge_soundspeedsnellfactor;
          soundspeed *= kluge_soundspeedsnellfactor;
        }

        if (pars->modify_soundspeed || kluge_soundspeedsnell) {
          /* change the sound speed recorded for the current ping and
           * then use it to alter the beam angles and recalculated the
           * bathymetry
           */
          volatilesettings->sound_velocity = soundspeed;

          /*
           * v2rawdetection record
           */
          if (store->read_v2rawdetection) {
            for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
              v2rawdetection->rx_angle[i] =
                  asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                           * sin(v2rawdetection->rx_angle[i]))));
            }
          }

          /*
           * v2detection record with or without v2detectionsetup
           */
          if (store->read_v2detection) {
            for (unsigned int i = 0; i < v2detection->number_beams; i++) {
              v2detection->angle_x[i]
                = asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                           * sin(v2detection->angle_x[i]))));
            }
          }

          /*
           * beamgeometry record
           */
          if (store->read_beamgeometry) {
            for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
              beamgeometry->angle_acrosstrack[i] =
                  asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                           * sin(beamgeometry->angle_acrosstrack[i]))));
            }
          }
        }

        /* get transducer angular offsets */
        if (platform != NULL) {
          status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 0,
                                                  &(tx_align.heading), &(tx_align.roll), &(tx_align.pitch), error);

          status &= mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 1,
                                                  &(rx_align.heading), &(rx_align.roll), &(rx_align.pitch), error);
        }

        // deal with reverse mounted tx array
        double tx_steer_sign = 1.0;
        if (tx_align.heading > 90.0 && tx_align.heading < 270.0) {
            tx_align.heading -= 180.0;
            tx_steer_sign = -1.0;
        }

        // deal with reverse mounted rx array
        double rx_steer_sign = 1.0;
        if (rx_align.heading > 90.0 && rx_align.heading < 270.0) {
            rx_align.heading -= 180.0;
            rx_steer_sign = -1.0;
        }

        /* loop over detections as available - the 7k format has used several
           different records over the years, so there are several different
           cases that must be handled */

        double reference_heading;
        double tx_steer;
        double rx_steer;

        /* case of v2rawdetection record */
        if (store->read_v2rawdetection) {
          for (unsigned int j = 0; j < v2rawdetection->number_beams; j++) {
            /* beam id */
            const int i = v2rawdetection->beam_descriptor[j];

            /* get range and quality */
            bathymetry->range[i] = v2rawdetection->detection_point[j] / v2rawdetection->sampling_rate;
            bathymetry->quality[i] = v2rawdetection->quality[j];

              /* get roll at bottom return time for this beam */
              if (pars->n_attitude > 0) {
                  /* interp_status &= */
                      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                       time_d + bathymetry->range[i], &beamroll, &jattitude, &interp_error);
              }
              else {
                  beamroll = roll;
              }

              /* get pitch at bottom return time for this beam */
              if (pars->n_attitude > 0) {
                  /* interp_status &= */
                      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                       time_d + bathymetry->range[i], &beampitch, &jattitude, &interp_error);
              }
              else {
                  beampitch = pitch;
              }

              /* get heading at bottom return time for this beam */
              if (pars->n_heading > 0) {
                  /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                           pars->n_heading, time_d + bathymetry->range[i], &beamheading,
                                           &jheading, &interp_error);
              }
              else {
                  beamheading = heading;
              }

              /* get heave at bottom return time for this beam */
              if (pars->n_attitude > 0) {
                  /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1,
                                               pars->n_attitude, time_d + bathymetry->range[i], &beamheave,
                                               &jattitude, &interp_error);
              }
              else {
                  beamheave = heave;
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
            tx_steer = tx_steer_sign * RTD * v2rawdetection->tx_angle;
            tx_orientation.roll = roll;
            tx_orientation.pitch = pitch;
            tx_orientation.heading = heading;
            rx_steer = -rx_steer_sign * RTD * v2rawdetection->rx_angle[j];
            rx_orientation.roll = beamroll;
            rx_orientation.pitch = beampitch;
            rx_orientation.heading = beamheading;
            reference_heading = heading;
//fprintf(stderr,"%s:%4.4d:%s: beam:%d tx:%f %f %f %f rx:%f %f %f %f hdg:%f\n",
//__FILE__, __LINE__, __func__,
//j, tx_steer, tx_orientation.roll, tx_orientation.pitch, tx_orientation.heading,
//rx_steer, rx_orientation.roll, rx_orientation.pitch, rx_orientation.heading,
//reference_heading);

            status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                                 reference_heading, &beamAzimuth, &beamDepression, error);
            const double theta = 90.0 - beamDepression;
            double phi = 90.0 - beamAzimuth;
            if (phi < 0.0)
              phi += 360.0;

            /* calculate bathymetry */
            const double rr = 0.5 * soundspeed * bathymetry->range[i];
            const double xx = rr * sin(DTR * theta);
            const double zz = rr * cos(DTR * theta);
            bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
            bathymetry->alongtrack[i] = xx * sin(DTR * phi);
            bathymetry->depth[i] = zz + sensordepth - 0.5 * (heave + beamheave);
            bathymetry->pointing_angle[i] = DTR * theta;
            bathymetry->azimuth_angle[i] = DTR * phi;
//fprintf(stderr,"%s:%4.4d:%s: beam:%d zxy:%f %f %f  angles:%f %f\n",
//__FILE__, __LINE__, __func__,
//j, bathymetry->depth[i], bathymetry->acrosstrack[i], bathymetry->alongtrack[i],
//bathymetry->pointing_angle[i], bathymetry->azimuth_angle[i]);
          }
        }

        /* case of v2detection record with v2detectionsetup */
        else if (store->read_v2detection && store->read_v2detectionsetup) {
          for (unsigned int j = 0; j < v2detection->number_beams; j++) {
            const int i = v2detectionsetup->beam_descriptor[j];

            bathymetry->range[i] = v2detection->range[j];
            bathymetry->quality[i] = v2detectionsetup->quality[j];

                        /* compensate for roll if not already compensated */
                        if ((volatilesettings->receive_flags & 0x1) != 0) {
                            beamroll = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* get roll at bottom return time for this beam */
                            /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1,
                                                             pars->n_attitude, time_d + bathymetry->range[i], &beamroll,
                                                             &jattitude, &interp_error);
                        }
                        else {
                            beamroll = roll;
                        }

                        /* compensate for pitch if not already compensated */
                        if ((volatilesettings->transmit_flags & 0xF) != 0) {
                            beampitch = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* interp_status &= */
                                mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                                 time_d + bathymetry->range[i], &beampitch, &jattitude, &interp_error);
                        }
                        else {
                            beampitch = pitch;
                        }

                        /* get heading at bottom return time for this beam */
                        if (pars->n_heading > 0) {
                            /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                                 pars->n_heading, time_d + bathymetry->range[i], &beamheading,
                                                                 &jheading, &interp_error);
                        }
                        else {
                            beamheading = heading;
                        }

                        /* compensate for heave if not already compensated */
                        if ((volatilesettings->receive_flags & 0x2) != 0) {
                            beamheave = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1,
                                                             pars->n_attitude, time_d + bathymetry->range[i], &beamheave,
                                                             &jattitude, &interp_error);
                        }
                        else {
                            beamheave = heave;
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
            tx_steer = tx_steer_sign * RTD * v2detection->angle_y[j];
            tx_orientation.roll = roll;
            tx_orientation.pitch = pitch;
            tx_orientation.heading = heading;
            rx_steer = -rx_steer_sign * RTD * v2detection->angle_x[j];
            rx_orientation.roll = beamroll;
            rx_orientation.pitch = beampitch;
            rx_orientation.heading = beamheading;
            reference_heading = heading;

            status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                                 reference_heading, &beamAzimuth, &beamDepression, error);
            const double theta = 90.0 - beamDepression;
            double phi = 90.0 - beamAzimuth;
            if (phi < 0.0)
              phi += 360.0;

            /* calculate bathymetry */
            const double rr = 0.5 * soundspeed * bathymetry->range[i];
            const double xx = rr * sin(DTR * theta);
            const double zz = rr * cos(DTR * theta);
            bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
            bathymetry->alongtrack[i] = xx * sin(DTR * phi);
            bathymetry->depth[i] = zz + sensordepth - 0.5 * (heave + beamheave);
            bathymetry->pointing_angle[i] = DTR * theta;
            bathymetry->azimuth_angle[i] = DTR * phi;
          }
        }

        /* case of v2detection record */
        else if (store->read_v2detection) {
          /* now loop over the detects */
          for (unsigned int i = 0; i < v2detection->number_beams; i++) {
            bathymetry->range[i] = v2detection->range[i];
            /* bathymetry->quality[i] set in bathymetry record */

                        /* compensate for roll if not already compensated */
                        if ((volatilesettings->receive_flags & 0x1) != 0) {
                            beamroll = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* get roll at bottom return time for this beam */
                            /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1,
                                                             pars->n_attitude, time_d + bathymetry->range[i], &beamroll,
                                                             &jattitude, &interp_error);
                        }
                        else {
                            beamroll = roll;
                        }

                        /* compensate for pitch if not already compensated */
                        if ((volatilesettings->transmit_flags & 0xF) != 0) {
                            beampitch = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* interp_status &= */
                                mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                                 time_d + bathymetry->range[i], &beampitch, &jattitude, &interp_error);
                        }
                        else {
                            beampitch = pitch;
                        }

                        /* get heading at bottom return time for this beam */
                        if (pars->n_heading > 0) {
                            /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                                 pars->n_heading, time_d + bathymetry->range[i], &beamheading,
                                                                 &jheading, &interp_error);
                        }
                        else {
                            beamheading = heading;
                        }

                        /* compensate for heave if not already compensated */
                        if ((volatilesettings->receive_flags & 0x2) != 0) {
                            beamheave = 0.0;
                        }
                        else if (pars->n_attitude > 0) {
                            /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1,
                                                             pars->n_attitude, time_d + bathymetry->range[i], &beamheave,
                                                             &jattitude, &interp_error);
                        }
                        else {
                            beamheave = heave;
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
            tx_steer = tx_steer_sign * RTD * v2detection->angle_y[i];
            tx_orientation.roll = roll;
            tx_orientation.pitch = pitch;
            tx_orientation.heading = heading;
            rx_steer = -rx_steer_sign * RTD * v2detection->angle_x[i];
            rx_orientation.roll = beamroll;
            rx_orientation.pitch = beampitch;
            rx_orientation.heading = beamheading;
            reference_heading = heading;

            status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                                 reference_heading, &beamAzimuth, &beamDepression, error);
            const double theta = 90.0 - beamDepression;
            double phi = 90.0 - beamAzimuth;
            if (phi < 0.0)
              phi += 360.0;

            /* calculate bathymetry */
            const double rr = 0.5 * soundspeed * bathymetry->range[i];
            const double xx = rr * sin(DTR * theta);
            const double zz = rr * cos(DTR * theta);
            bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
            bathymetry->alongtrack[i] = xx * sin(DTR * phi);
            bathymetry->depth[i] = zz + sensordepth - 0.5 * (heave + beamheave);
            bathymetry->pointing_angle[i] = DTR * theta;
            bathymetry->azimuth_angle[i] = DTR * phi;
          }
        }

        /* else default case of beamgeometry record */
        else {
          /* loop over all beams */
          for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
            /* bathymetry->range[i] set */
            /* bathymetry->quality[i] set */
            if ((bathymetry->quality[i] & 15) > 0) {

              /* compensate for roll if not already compensated */
              if ((volatilesettings->receive_flags & 0x1) != 0) {
                  beamroll = 0.0;
              }
              else if (pars->n_attitude > 0) {
                  /* get roll at bottom return time for this beam */
                  /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1,
                                                   pars->n_attitude, time_d + bathymetry->range[i], &beamroll,
                                                   &jattitude, &interp_error);
              }
              else {
                  beamroll = roll;
              }

              /* compensate for pitch if not already compensated */
              if ((volatilesettings->transmit_flags & 0xF) != 0) {
                  beampitch = 0.0;
              }
              else if (pars->n_attitude > 0) {
                  /* interp_status &= */
                      mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                       time_d + bathymetry->range[i], &beampitch, &jattitude, &interp_error);
              }
              else {
                  beampitch = pitch;
              }

              /* get heading at bottom return time for this beam */
              if (pars->n_heading > 0) {
                  /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                       pars->n_heading, time_d + bathymetry->range[i], &beamheading,
                                                       &jheading, &interp_error);
              }
              else {
                  beamheading = heading;
              }

              /* compensate for heave if not already compensated */
              if ((volatilesettings->receive_flags & 0x2) != 0) {
                  beamheave = 0.0;
              }
              else if (pars->n_attitude > 0) {
                  /* interp_status &= */ mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1,
                                                   pars->n_attitude, time_d + bathymetry->range[i], &beamheave,
                                                   &jattitude, &interp_error);
              }
              else {
                  beamheave = heave;
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
              tx_steer = tx_steer_sign * RTD * beamgeometry->angle_alongtrack[i];
              tx_orientation.roll = roll;
              tx_orientation.pitch = pitch;
              tx_orientation.heading = heading;
              rx_steer = -rx_steer_sign * RTD * beamgeometry->angle_acrosstrack[i];
              rx_orientation.roll = beamroll;
              rx_orientation.pitch = beampitch;
              rx_orientation.heading = beamheading;
              reference_heading = heading;

              status = mb_beaudoin(verbose, tx_align, tx_orientation, tx_steer, rx_align, rx_orientation, rx_steer,
                                   reference_heading, &beamAzimuth, &beamDepression, error);
              const double theta = 90.0 - beamDepression;
              double phi = 90.0 - beamAzimuth;
              if (phi < 0.0)
                phi += 360.0;

              /* calculate bathymetry */
              const double rr = 0.5 * soundspeed * bathymetry->range[i];
              const double xx = rr * sin(DTR * theta);
              const double zz = rr * cos(DTR * theta);
              bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
              bathymetry->alongtrack[i] = xx * sin(DTR * phi);
              bathymetry->depth[i] = zz + sensordepth - 0.5 * (heave + beamheave);
              bathymetry->pointing_angle[i] = DTR * theta;
              bathymetry->azimuth_angle[i] = DTR * phi;
            }
          }
        }

        /* set flag */
        bathymetry->optionaldata = true;
        bathymetry->header.OffsetToOptionalData =
            MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kBathymetricData + bathymetry->number_beams * 9;

        /* control parameters */
        int ss_source = R7KRECID_None;

        if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_SNIPPET)
          ss_source = R7KRECID_7kV2SnippetData;
        else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_CALIBRATEDSNIPPET)
          ss_source = R7KRECID_7kCalibratedSnippetData;
        else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_WIDEBEAMBACKSCATTER)
          ss_source = R7KRECID_7kBackscatterImageData;

        /* regenerate sidescan */
        status &= mbsys_reson7k_makess_source(verbose, mbio_ptr, store_ptr, ss_source, false, pixel_size, false, swath_width,
                                      true, error);
      }
      /*--------------------------------------------------------------*/
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
int mbsys_reson7k_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_installation *installation = (s7kr_installation *)&store->installation;

  int status = MB_SUCCESS;

  /* if needed allocate a new platform structure */
  if (*platform_ptr == NULL) {
    status = mb_platform_init(verbose, (void **)platform_ptr, error);
  }

  /* extract sensor offsets from installation record */
  if (*platform_ptr != NULL) {
    /* get pointer to platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

    /* look for multibeam sensor, add it if necessary */
    int sensor_multibeam = -1;
    for (int isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
      if (platform->sensors[isensor].type == MB_SENSOR_TYPE_SONAR_MULTIBEAM &&
          platform->sensors[isensor].num_offsets == 2) {
        sensor_multibeam = isensor;
      }
    }
    if (sensor_multibeam < 0) {
      /* set sensor 0 (multibeam) */
      status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_SONAR_MULTIBEAM, NULL, "Reson", NULL,
                                      MB_SENSOR_CAPABILITY1_NONE, MB_SENSOR_CAPABILITY2_TOPOGRAPHY_MULTIBEAM, 2, 0, error);
      if (status == MB_SUCCESS) {
        sensor_multibeam = platform->num_sensors - 1;
      }
    }
    if (sensor_multibeam >= 0 && platform->sensors[sensor_multibeam].num_offsets == 2) {
      if (status == MB_SUCCESS) {
        platform->source_bathymetry = sensor_multibeam;
        platform->source_backscatter = sensor_multibeam;
      }
      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(
            verbose, (void *)platform, 0, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)installation->transmit_x,
            (double)installation->transmit_y, (double)installation->transmit_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
            (double)installation->transmit_heading, (double)installation->transmit_roll,
            (double)installation->transmit_pitch, error);
      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 0, 1, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)installation->receive_x, (double)installation->receive_y,
                                               (double)installation->receive_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
                                               (double)installation->receive_heading, (double)installation->receive_roll,
                                               (double)installation->receive_pitch, error);
    }

    /* look for position sensor, add it if necessary */
    int sensor_position = -1;
    if (platform->source_position1 >= 0)
      sensor_position = platform->source_position1;
    for (int isensor = 0; isensor < platform->num_sensors && sensor_position < 0; isensor++) {
      if (platform->sensors[isensor].type == MB_SENSOR_TYPE_POSITION && platform->sensors[isensor].num_offsets == 1) {
        sensor_position = isensor;
      }
    }
    if (sensor_position < 0) {
      /* set sensor 1 (position) */
      const int ntimelag = 0;
      status = mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_POSITION, NULL, NULL, NULL, 0, 0, 1,
                                      ntimelag, error);
      if (status == MB_SUCCESS) {
        sensor_position = platform->num_sensors - 1;
      }
    }
    if (sensor_position >= 0 && platform->sensors[sensor_position].num_offsets == 1) {
      if (status == MB_SUCCESS) {
        platform->source_position1 = sensor_position;
        platform->source_depth1 = sensor_position;
        platform->source_position = sensor_position;
        platform->source_depth = sensor_position;
      }

      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 1, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)installation->position_x, (double)installation->position_y,
                                               (double)installation->position_z, MB_SENSOR_ATTITUDE_OFFSET_NONE,
                                               (double)0.0, (double)0.0, (double)0.0, error);
      if (status == MB_SUCCESS && installation->position_time_delay != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.001 * installation->position_time_delay), 0, NULL, NULL, error);
      }
    }

    /* look for attitude sensor, add it if necessary */
    int sensor_attitude = -1;
    if (platform->source_rollpitch1 >= 0)
      sensor_attitude = platform->source_rollpitch1;
    for (int isensor = 0; isensor < platform->num_sensors && sensor_attitude < 0; isensor++) {
      if ((platform->sensors[isensor].type == MB_SENSOR_TYPE_VRU || platform->sensors[isensor].type == MB_SENSOR_TYPE_IMU ||
           platform->sensors[isensor].type == MB_SENSOR_TYPE_INS) &&
          platform->sensors[isensor].num_offsets == 1) {
        sensor_attitude = isensor;
      }
    }
    if (sensor_attitude < 0) {
      /* set sensor 2 (attitude) */
      const int ntimelag = 0;
      status =
          mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, 0, 0, 1, ntimelag, error);
      if (status == MB_SUCCESS) {
        sensor_attitude = platform->num_sensors - 1;
      }
    }
    if (sensor_attitude >= 0 && platform->sensors[sensor_attitude].num_offsets == 1) {
      if (status == MB_SUCCESS) {
        platform->source_rollpitch1 = sensor_attitude;
        platform->source_heading1 = sensor_attitude;
        platform->source_rollpitch = sensor_attitude;
        platform->source_heading = sensor_attitude;
      }

      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 2, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)installation->motion_x, (double)installation->motion_y,
                                               (double)installation->motion_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
                                               (double)installation->motion_heading, (double)installation->motion_roll,
                                               (double)installation->motion_pitch, error);
      if (status == MB_SUCCESS && installation->motion_time_delay != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.001 * installation->motion_time_delay), 0, NULL, NULL, error);
      }
    }

    /* print platform */
    if (verbose >= 2) {
      status = mb_platform_print(verbose, (void *)platform, error);
    }
  }
  else {
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
int mbsys_reson7k_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
                          double *navlat, double *speed, double *heading, int *nbath, int *namp, int *nss, char *beamflag,
                          double *bath, double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss,
                          double *ssacrosstrack, double *ssalongtrack, char *comment, int *error) {
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
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  s7kr_bluefin *bluefin = (s7kr_bluefin *)&store->bluefin;
  s7kr_processedsidescan *processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
  s7kr_volatilesettings *volatilesettings = (s7kr_volatilesettings *)&(store->volatilesettings);
  s7kr_beamgeometry *beamgeometry = (s7kr_beamgeometry *)&(store->beamgeometry);
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  // s7kr_backscatter *backscatter = (s7kr_backscatter *)&store->backscatter;
  // s7kr_beam *beam = (s7kr_beam *)&store->beam;
  s7kr_position *position = (s7kr_position *)&store->position;
  s7kr_systemeventmessage *systemeventmessage = (s7kr_systemeventmessage *)&store->systemeventmessage;
  s7kr_fsdwsb *fsdwsb = &(store->fsdwsb);
  s7kr_fsdwss *fsdwsslo = &(store->fsdwsslo);
  s7kr_fsdwss *fsdwsshi = &(store->fsdwsshi);

  /* get saved values */
  // double *pixel_size = (double *)&mb_io_ptr->saved1;
  // double *swath_width = (double *)&mb_io_ptr->saved2;

  s7k_fsdwsegyheader *fsdwsegyheader;
  s7k_fsdwssheader *fsdwssheader;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get interpolated nav heading and speed  */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    if (bathymetry->optionaldata)
      *heading = RTD * bathymetry->heading;

    /* get navigation */
    if (bathymetry->optionaldata && bathymetry->longitude != 0.0 && bathymetry->latitude != 0.0) {
      *navlon = RTD * bathymetry->longitude;
      *navlat = RTD * bathymetry->latitude;
    }

    /* set beamwidths in mb_io structure */
    if (store->read_volatilesettings) {
      mb_io_ptr->beamwidth_xtrack = RTD * volatilesettings->receive_width;
      mb_io_ptr->beamwidth_ltrack = RTD * volatilesettings->beamwidth_vertical;
    }
    else if (store->read_beamgeometry) {
      mb_io_ptr->beamwidth_xtrack = RTD * beamgeometry->beamwidth_acrosstrack[beamgeometry->number_beams / 2];
      mb_io_ptr->beamwidth_ltrack = RTD * beamgeometry->beamwidth_alongtrack[beamgeometry->number_beams / 2];
    }
    mb_io_ptr->beamwidth_xtrack = MIN(mb_io_ptr->beamwidth_xtrack, 2.0);
    mb_io_ptr->beamwidth_ltrack = MIN(mb_io_ptr->beamwidth_ltrack, 2.0);

    /* read distance and depth values into storage arrays */
    *nbath = bathymetry->number_beams;
    *namp = *nbath;
    for (int i = 0; i < *nbath; i++) {
      bath[i] = bathymetry->depth[i];

      /* beamflagging scheme:
          Reson quality flags use bits 0-3
              bit 0: brightness test
              bit 1: colinearity test
              bit 2: amplitude pick
              bit 3: phase pick
          Early MB scheme (through 2007) - use bits 0-5
              null: 0
              flagged: 2
              good: 15
              amplitude: +16
              phase: +32
          Current MB scheme (>= 2008) - use bits 4-7
              - bits 0-3 left in original values
              - beam valid if bit 4 or 5 are set
              - beam flagged if bit 6 or 7 set
              bit 4: on = amplitude
              bit 5: on = phase
              bit 6: on = auto flag
              bit 7: on = manual flag */
      if (bathymetry->quality[i] == 0) {
        beamflag[i] = MB_FLAG_NULL;
      }
      else if (bathymetry->quality[i] & 64) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
      }
      else if (bathymetry->quality[i] & 128) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
      }
      else if (bathymetry->quality[i] & 240) {
        beamflag[i] = MB_FLAG_NONE;
      }
      else if ((bathymetry->quality[i] & 3) == 3) {
        beamflag[i] = MB_FLAG_NONE;
      }
      else if ((bathymetry->quality[i] & 15) == 0) {
        beamflag[i] = MB_FLAG_NULL;
      }
      else if ((bathymetry->quality[i] & 3) == 0) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
      }
      else {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
      }
#ifdef MSYS_RESON7KR_DEBUG
      fprintf(stderr, "EXTRACT: beam:%d quality:%d q&240:%d ", i, bathymetry->quality[i], bathymetry->quality[i] & 240);
      if (bathymetry->quality[i] & 1)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 2)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 4)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 8)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 16)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 32)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 64)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 128)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      fprintf(stderr, " flag:%d\n", beamflag[i]);
#endif
      bathacrosstrack[i] = bathymetry->acrosstrack[i];
      bathalongtrack[i] = bathymetry->alongtrack[i];
      if (bathymetry->intensity[i] > 0.0)
        amp[i] = 20.0 * log10((double)bathymetry->intensity[i]);
      else
        amp[i] = 0.0;
    }

    /* extract sidescan */
    *nss = 0;
    if (store->read_processedsidescan) {
      *nss = processedsidescan->number_pixels;
      for (int i = 0; i < (int)processedsidescan->number_pixels; i++) {
        ss[i] = processedsidescan->sidescan[i];
        ssacrosstrack[i] = processedsidescan->pixelwidth * (i - (int)processedsidescan->number_pixels / 2);
        ssalongtrack[i] = processedsidescan->alongtrack[i];
      }
      for (int i = processedsidescan->number_pixels; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
        ss[i] = MB_SIDESCAN_NULL;
        ssacrosstrack[i] = 0.0;
        ssalongtrack[i] = 0.0;
      }
    }
    else {
      for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
        ss[i] = MB_SIDESCAN_NULL;
        ssacrosstrack[i] = 0.0;
        ssalongtrack[i] = 0.0;
      }
    }

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
  else if (*kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get navigation */
    *navlon = RTD * position->longitude;
    *navlat = RTD * position->latitude;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

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

    /* get heading */
    *heading = RTD * bluefin->nav[0].yaw;

    /* get speed */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get navigation */
    *navlon = RTD * bluefin->nav[0].longitude;
    *navlat = RTD * bluefin->nav[0].latitude;

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

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
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* get edgetech segy header */
    fsdwsegyheader = &(fsdwsb->segyheader);

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    if (fsdwsegyheader->heading != 0)
      *heading = 0.01 * fsdwsegyheader->heading;
    else if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed and position */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get position */
    if (fsdwsegyheader->sourceCoordX != 0 || fsdwsegyheader->sourceCoordY != 0) {
      *navlon = ((double)fsdwsegyheader->sourceCoordX) / 360000.0;
      *navlat = ((double)fsdwsegyheader->sourceCoordY) / 360000.0;
    }

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

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
      fprintf(stderr, "dbg4       longitude:  %f\n", *navlon);
      fprintf(stderr, "dbg4       latitude:   %f\n", *navlat);
      fprintf(stderr, "dbg4       speed:      %f\n", *speed);
      fprintf(stderr, "dbg4       heading:    %f\n", *heading);
    }

    /* done translating values */
  }

  /* extract data from sidescan structure */
  else if (*kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3) {
    /* get edgetech sidescan header */
    if (*kind == MB_DATA_SIDESCAN2)
      fsdwssheader = &(fsdwsslo->ssheader[0]);
    else // if (*kind == MB_DATA_SIDESCAN3)
      fsdwssheader = &(fsdwsshi->ssheader[0]);

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    if (fsdwssheader->heading != 0)
      *heading = 0.01 * fsdwssheader->heading;
    else if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed and position */
    *speed = 0.0;
    mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get position */
    if (fsdwssheader->longitude != 0 || fsdwssheader->latitude != 0) {
      *navlon = ((double)fsdwssheader->longitude) / 360000.0;
      *navlat = ((double)fsdwssheader->latitude) / 360000.0;
    }

    /* set beam and pixel numbers */
    *nbath = 0;
    *namp = 0;
    *nss = 0;

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
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    if (systemeventmessage->message_length > 0) {
      strncpy(comment, systemeventmessage->message, MIN(MB_COMMENT_MAXLINE - 1, systemeventmessage->message_length));
    }

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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
  s7k_fsdwsegyheader *fsdwsegyheader = NULL;
  s7k_fsdwssheader *fsdwssheader = NULL;
  unsigned int msglen = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:       %d\n", kind);
  }
  if (verbose >= 2 && (kind == MB_DATA_DATA || kind == MB_DATA_NAV1 || kind == MB_DATA_NAV2)) {
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
  if (verbose >= 2 && kind == MB_DATA_COMMENT) {
    fprintf(stderr, "dbg2       comment:     \ndbg2       %s\n", comment);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bluefin *bluefin = (s7kr_bluefin *)&store->bluefin;
  // s7kr_volatilesettings *volatilesettings = (s7kr_volatilesettings *)&store->volatilesettings;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  // s7kr_backscatter *backscatter = (s7kr_backscatter *)&store->backscatter;
  // s7kr_beam *beam = (s7kr_beam *)&store->beam;
  s7kr_processedsidescan *processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
  s7kr_position *position = (s7kr_position *)&store->position;
  s7kr_systemeventmessage *systemeventmessage = (s7kr_systemeventmessage *)&store->systemeventmessage;
  s7kr_fsdwsb *fsdwsb = &(store->fsdwsb);
  s7kr_fsdwss *fsdwsslo = &(store->fsdwsslo);
  s7kr_fsdwss *fsdwsshi = &(store->fsdwsshi);

  /* set data kind */
  store->kind = kind;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    bathymetry->longitude = DTR * navlon;
    bathymetry->latitude = DTR * navlat;

    /* get heading */
    bathymetry->heading = DTR * heading;

    /* get speed  */

    /* read distance and depth values into storage arrays */
    bathymetry->number_beams = nbath;
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      bathymetry->depth[i] = bath[i];
      if (beamflag[i] == MB_FLAG_NULL)
        bathymetry->quality[i] = 0;
      else if (mb_beam_check_flag_manual(beamflag[i]))
        bathymetry->quality[i] = (bathymetry->quality[i] & 63) + 128;
      else if (mb_beam_check_flag(beamflag[i]))
        bathymetry->quality[i] = (bathymetry->quality[i] & 63) + 64;
      else {
        bathymetry->quality[i] = (bathymetry->quality[i] & 63);
        if (!(bathymetry->quality[i] & 12))
          bathymetry->quality[i] = bathymetry->quality[i] | 16;
        else if (bathymetry->quality[i] & 4)
          bathymetry->quality[i] = bathymetry->quality[i] | 16;
        else if (bathymetry->quality[i] & 8)
          bathymetry->quality[i] = bathymetry->quality[i] | 32;
      }
      bathymetry->acrosstrack[i] = bathacrosstrack[i];
      bathymetry->alongtrack[i] = bathalongtrack[i];
      bathymetry->intensity[i] = (float)(pow(10.0, (amp[i] / 20.0)));
#ifdef MSYS_RESON7KR_DEBUG
      fprintf(stderr, "INSERT: beam:%d quality:%d q&240:%d ", i, bathymetry->quality[i], bathymetry->quality[i] & 240);
      if (bathymetry->quality[i] & 1)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 2)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 4)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 8)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 16)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 32)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 64)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      if (bathymetry->quality[i] & 128)
        fprintf(stderr, "1");
      else
        fprintf(stderr, "0");
      fprintf(stderr, " flag:%d\n", beamflag[i]);
#endif
    }

    /* insert the sidescan */
    processedsidescan->number_pixels = nss;
    int ixmin = nss - 1;
    int ixmax = 0;
    for (int i = 0; i < nss; i++) {
      if (ss[i] != MB_SIDESCAN_NULL) {
        if (i < ixmin) ixmin = i;
        ixmax = i;
      }
    }
    if (ixmax > ixmin)
      processedsidescan->pixelwidth = (ssacrosstrack[ixmax] - ssacrosstrack[ixmin]) / (ixmax - ixmin);
    else
      processedsidescan->pixelwidth = 1.0;
    for (unsigned int i = 0; i < processedsidescan->number_pixels; i++) {
      processedsidescan->sidescan[i] = ss[i];
      processedsidescan->alongtrack[i] = ssalongtrack[i];
    }
    for (int i = processedsidescan->number_pixels; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      processedsidescan->sidescan[i] = 0.0;
      processedsidescan->alongtrack[i] = 0.0;
    }
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    position->longitude = DTR * navlon;
    position->latitude = DTR * navlat;

    /* get heading */

    /* get speed  */
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    bluefin->nav[0].longitude = DTR * navlon;
    bluefin->nav[0].latitude = DTR * navlat;

    /* get heading */
    bluefin->nav[0].yaw = DTR * heading;

    /* get speed  */
  }

  /* insert data in subbottom structure */
  else if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* get edgetech segy header */
    fsdwsegyheader = &(fsdwsb->segyheader);

    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    fsdwsegyheader->sourceCoordX = (int)(navlon * 360000.0);
    fsdwsegyheader->sourceCoordY = (int)(navlat * 360000.0);

    /* get heading */
    fsdwsegyheader->heading = (int)(100 * heading);

    /* get speed  */
  }

  /* insert data in sidescan structure */
  else if (store->kind == MB_DATA_SIDESCAN2 || store->kind == MB_DATA_SIDESCAN3) {
    /* get edgetech sidescan header */
    if (store->kind == MB_DATA_SIDESCAN2)
      fsdwssheader = &(fsdwsslo->ssheader[0]);
    else // if (store->kind == MB_DATA_SIDESCAN3)
      fsdwssheader = &(fsdwsshi->ssheader[0]);

    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    fsdwssheader->longitude = (int)(navlon * 360000.0);
    fsdwssheader->latitude = (int)(navlat * 360000.0);

    /* get heading */
    fsdwssheader->heading = (int)(100 * heading);

    /* get speed  */
  }

  /* insert comment in structure */
  else if (store->kind == MB_DATA_COMMENT) {
    /* make sure memory is allocated for comment */
    msglen = MIN(strlen(comment) + 1, MB_COMMENT_MAXLINE);
    if (msglen % 2 > 0)
      msglen++;
    if (systemeventmessage->message_alloc < msglen) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, msglen, (void **)&(systemeventmessage->message), error);
      if (status != MB_SUCCESS) {
        systemeventmessage->message_alloc = 0;
        systemeventmessage->message = NULL;
      }
      else {
        systemeventmessage->message_alloc = msglen;
      }
    }

    /* copy comment */
    if (status == MB_SUCCESS) {
      store->type = R7KRECID_7kSystemEventMessage;
      systemeventmessage->serial_number = 0;
      systemeventmessage->event_id = 1;
      systemeventmessage->message_length = msglen;
      systemeventmessage->event_identifier = 0;
      memset((void *)systemeventmessage->message, 0, systemeventmessage->message_alloc);
      strncpy(systemeventmessage->message, comment, msglen);
      systemeventmessage->header.Size =
          MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kSystemEventMessage + msglen + MBSYS_RESON7K_RECORDTAIL_SIZE;
      systemeventmessage->header.OffsetToOptionalData = 0;
      systemeventmessage->header.OptionalDataIdentifier = 0;
      systemeventmessage->header.Reserved = 0;
      systemeventmessage->header.RecordType = R7KRECID_7kSystemEventMessage;
      systemeventmessage->header.DeviceId = 0;
      systemeventmessage->header.SystemEnumerator = 0;
      systemeventmessage->header.DataSetNumber = 0;
      systemeventmessage->header.RecordNumber = 0;
      for (int i = 0; i < 8; i++) {
        systemeventmessage->header.PreviousRecord[i] = -1;
        systemeventmessage->header.NextRecord[i] = -1;
      }
      systemeventmessage->header.Flags = 0;
      systemeventmessage->header.Reserved2 = 0;
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
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
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
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  // s7kr_depth *depth = (s7kr_depth *)&store->depth;
  // s7kr_attitude *attitude = (s7kr_attitude *)&store->attitude;
  s7kr_ctd *ctd = (s7kr_ctd *)&store->ctd;
  s7kr_beamgeometry *beamgeometry = (s7kr_beamgeometry *)&store->beamgeometry;
  s7kr_reference *reference = (s7kr_reference *)&store->reference;

  double heave_use, roll, pitch;
  // double alpha, beta, theta, phi;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get depth offset (heave + sonar depth) */
    if (bathymetry->sound_velocity > 0.0)
      *ssv = bathymetry->sound_velocity;
    else if (ctd != NULL && ctd->n > 0)
      *ssv = ctd->sound_velocity[0];
    else
      *ssv = 1500.0;

    /* get attitude data */
    if (bathymetry->optionaldata) {
      heave_use = bathymetry->heave;
    }
    else if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave_use, &roll, &pitch, error);
    }

    /* get draft */
    if (bathymetry->optionaldata) {
      *draft = -bathymetry->vehicle_height + reference->water_z;
      heave_use = 0.0;
    }
    else if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      heave_use = 0.0;
    }
    else {
      *draft = reference->water_z;
    }

    /* get travel times, angles */
    *nbeams = bathymetry->number_beams;
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      ttimes[i] = bathymetry->range[i];
      if (bathymetry->optionaldata) {
        angles[i] = RTD * bathymetry->pointing_angle[i];
        angles_forward[i] = RTD * bathymetry->azimuth_angle[i];
      }
      else {
        const double alpha = RTD * beamgeometry->angle_alongtrack[i] + bathymetry->pitch;
        const double beta = 90.0 - RTD * beamgeometry->angle_acrosstrack[i] + bathymetry->roll;
	      double phi;
	      double theta;
        mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
        angles[i] = theta;
        angles_forward[i] = phi;
      }
      if (bathymetry->header.DeviceId == 7100)
        angles_null[i] = angles[i];
      else
        angles_null[i] = 0.0;
      heave[i] = heave_use;
      alongtrack_offset[i] = 0.0;
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* read distance and depth values into storage arrays */
    *nbeams = bathymetry->number_beams;
    for (int i = 0; i < *nbeams; i++) {
      const mb_u_char detect = (bathymetry->quality[i] & 48) >> 4;
      if (detect == 0)
        detects[i] = MB_DETECT_UNKNOWN;
      else if (detect == 1)
        detects[i] = MB_DETECT_AMPLITUDE;
      else if (detect == 2)
        detects[i] = MB_DETECT_PHASE;
      else
        detects[i] = MB_DETECT_UNKNOWN;
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get survey data structure */
    s7kr_volatilesettings *volatilesettings = &(store->volatilesettings);
    // s7k_header *header = &(volatilesettings->header);

    /* get transmit_gain (dB) */
    *transmit_gain = (double)volatilesettings->power_selection;

    /* get pulse_length (usec) */
    *pulse_length = (double)volatilesettings->pulse_width;

    /* get receive_gain (dB) */
    *receive_gain = (double)volatilesettings->gain_selection;

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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transducer_depth,
                                   double *altitudev, int *error) {
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
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  // s7kr_attitude *attitude = (s7kr_attitude *)&store->attitude;
  s7kr_altitude *altitude = (s7kr_altitude *)&store->altitude;
  // s7kr_depth *depth = (s7kr_depth *)&store->depth;
  s7kr_reference *reference = (s7kr_reference *)&store->reference;

  double heave, roll, pitch;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get transducer depth and altitude */
    if (bathymetry->optionaldata) {
      *transducer_depth = -bathymetry->vehicle_height + reference->water_z;
    }
    else if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, transducer_depth, error);
    }
    else if (mb_io_ptr->nattitude > 0) {
      *transducer_depth = reference->water_z;
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave, &roll, &pitch, error);
      *transducer_depth += heave;
    }
    else {
      *transducer_depth = reference->water_z;
    }

    /* get altitude */
    bool altitude_found = false;
    if (mb_io_ptr->naltitude > 0) {
      mb_altint_interp(verbose, mbio_ptr, store->time_d, altitudev, error);
      altitude_found = true;
    }
    if (!altitude_found && bathymetry->optionaldata) {
      /* get depth closest to nadir */
      double xtrackmin = 999999.9;
      char flag;
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        if (bathymetry->quality[i] == 0) {
          flag = MB_FLAG_NULL;
        }
        else if (bathymetry->quality[i] & 64) {
          flag = MB_FLAG_FLAG + MB_FLAG_FILTER;
        }
        else if (bathymetry->quality[i] & 128) {
          flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        }
        else if (bathymetry->quality[i] & 240) {
          flag = MB_FLAG_NONE;
        }
        else if ((bathymetry->quality[i] & 3) == 3) {
          flag = MB_FLAG_NONE;
        }
        else if ((bathymetry->quality[i] & 15) == 0) {
          flag = MB_FLAG_NULL;
        }
        else if ((bathymetry->quality[i] & 3) == 0) {
          flag = MB_FLAG_FLAG + MB_FLAG_FILTER;
        }
        else {
          flag = MB_FLAG_FLAG + MB_FLAG_MANUAL;
        }

        if ((flag == MB_FLAG_NONE) && fabs((double)bathymetry->acrosstrack[i]) < xtrackmin) {
          *altitudev = bathymetry->depth[i] - *transducer_depth;
          altitude_found = true;
          xtrackmin = fabs((double)bathymetry->acrosstrack[i]);
        }
      }
    }
    if (!altitude_found && altitude->altitude > 0.0) {
      *altitudev = altitude->altitude;
    }
    else if (!altitude_found) {
      *altitudev = 0.0;
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
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
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  // s7kr_attitude *attitude = (s7kr_attitude *)&store->attitude;
  s7kr_reference *reference = (s7kr_reference *)&store->reference;
  s7kr_bluefin *bluefin = (s7kr_bluefin *)&store->bluefin;
  // s7kr_depth *depth = (s7kr_depth *)&store->depth;
  s7kr_navigation *navigation = &(store->navigation);
  s7kr_fsdwsb *fsdwsb = &(store->fsdwsb);
  s7kr_fsdwss *fsdwsslo = &(store->fsdwsslo);
  s7kr_fsdwss *fsdwsshi = &(store->fsdwsshi);

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  s7k_fsdwssheader *fsdwssheader;
  s7k_fsdwsegyheader *fsdwsegyheader;

  /* extract data from ping structure */
  if (*kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get interpolated nav heading and speed  */
    *speed = 0.0;
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    if (bathymetry->optionaldata) {
      *heading = RTD * bathymetry->heading;

      *navlon = RTD * bathymetry->longitude;
      *navlat = RTD * bathymetry->latitude;

      *draft = -bathymetry->vehicle_height + reference->water_z;
    } else if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
    } else {
      *draft = reference->water_z;
    }

    /* get attitude  */
    if (bathymetry->optionaldata) {
      *roll = RTD * bathymetry->roll;
      *pitch = RTD * bathymetry->pitch;
      *heave = bathymetry->heave;
    }
    else {
      if (mb_io_ptr->nattitude > 0) {
        mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
      }
    }

    /* done translating values */
  }

  /* extract data from nav structure */
  else if (*kind == MB_DATA_NAV1) {
    /* get position data structure */
    s7kr_position *position = (s7kr_position *)&store->position;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation and heading */
    *speed = 0.0;
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    *navlon = RTD * position->longitude;
    *navlat = RTD * position->latitude;

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      // TODO(schwehr): Was something else intended in the if?
      // if (mb_io_ptr->nsensordepth > 0)
        mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else if (bathymetry->optionaldata) {
      *draft = -bathymetry->vehicle_height + reference->water_z;
      *heave = 0.0;
    }
    else {
      *draft = reference->water_z;
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    *heading = RTD * bluefin->nav[0].yaw;

    /* get speed */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get navigation */
    *navlon = RTD * bluefin->nav[0].longitude;
    *navlat = RTD * bluefin->nav[0].latitude;

    /* get roll pitch and heave */
    *roll = RTD * bluefin->nav[0].roll;
    *pitch = RTD * bluefin->nav[0].pitch;
    *heave = 0.0;

    /* get draft  */
    *draft = bluefin->nav[0].depth;

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV3) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = RTD * navigation->longitude;
    navlat[0] = RTD * navigation->latitude;

    /* get speed  */
    speed[0] = 0.0;

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    else if (bathymetry->optionaldata)
      heading[0] = RTD * bathymetry->heading;

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
    }
    else if (bathymetry->optionaldata) {
      roll[0] = RTD * bathymetry->roll;
      pitch[0] = RTD * bathymetry->pitch;
      heave[0] = bathymetry->heave;
    }
    else {
      roll[0] = 0.0;
      pitch[0] = 0.0;
      heave[0] = 0.0;
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* get edgetech segy header */
    fsdwsegyheader = &(fsdwsb->segyheader);

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    if (fsdwsegyheader->heading != 0)
      *heading = 0.01 * fsdwsegyheader->heading;
    else
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed and position */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get position */
    if (fsdwsegyheader->sourceCoordX != 0 || fsdwsegyheader->sourceCoordY != 0) {
      *navlon = ((double)fsdwsegyheader->sourceCoordX) / 360000.0;
      *navlat = ((double)fsdwsegyheader->sourceCoordY) / 360000.0;
    }

    /* get roll pitch and heave */
    *roll = 0.01 * fsdwsegyheader->roll;
    *pitch = 0.01 * fsdwsegyheader->pitch;
    *heave = 0.0;

    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    *draft = reference->water_z;

    /* done translating values */
  }

  /* extract data from sidescan structure */
  else if (*kind == MB_DATA_SIDESCAN2 || *kind == MB_DATA_SIDESCAN3) {
    /* get edgetech sidescan header */
    if (*kind == MB_DATA_SIDESCAN2)
      fsdwssheader = &(fsdwsslo->ssheader[0]);
    else // if (*kind == MB_DATA_SIDESCAN3)
      fsdwssheader = &(fsdwsshi->ssheader[0]);

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    if (fsdwssheader->heading != 0)
      *heading = 0.01 * fsdwssheader->heading;
    else if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed and position */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get position */
    if (fsdwssheader->longitude != 0 || fsdwssheader->latitude != 0) {
      *navlon = ((double)fsdwssheader->longitude) / 360000.0;
      *navlat = ((double)fsdwssheader->latitude) / 360000.0;
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
  /*if (status == MB_SUCCESS)
  fprintf(stderr,"%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d %f %f %f %f %f %f %f %f\n",
  time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6],
  *navlon,*navlat,*speed,*heading,*draft,*roll,*pitch,*heave);*/

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr, int nmax, int *kind, int *n, int *time_i,
                               double *time_d, double *navlon, double *navlat, double *speed, double *heading, double *draft,
                               double *roll, double *pitch, double *heave, int *error) {
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
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  s7kr_bluefin *bluefin = (s7kr_bluefin *)&store->bluefin;
  s7kr_position *position = (s7kr_position *)&store->position;
  s7kr_customattitude *customattitude = (s7kr_customattitude *) &store->customattitude;
  // s7kr_depth *depth = (s7kr_depth *)&store->depth;
  s7kr_attitude *attitude = (s7kr_attitude *)&store->attitude;
  s7kr_reference *reference = (s7kr_reference *)&store->reference;
  s7kr_navigation *navigation = &(store->navigation);
  s7kr_rollpitchheave *rollpitchheave = &(store->rollpitchheave);
  s7kr_heading *headings = &(store->heading);

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from ping structure */
  if (*kind == MB_DATA_DATA) {
    /* just one navigation value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get interpolated nav heading and speed  */
    speed[0] = 0.0;
    mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
    mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]), error);

    if (bathymetry->optionaldata) {
      heading[0] = RTD * bathymetry->heading;

      navlon[0] = RTD * bathymetry->longitude;
      navlat[0] = RTD * bathymetry->latitude;

      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    } else if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    } else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    if (bathymetry->optionaldata) {
      roll[0] = RTD * bathymetry->roll;
      pitch[0] = RTD * bathymetry->pitch;
      heave[0] = bathymetry->heave;
    }
    else {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
    }

    /* done translating values */
  }

  /* extract data from nav structure */
  else if (*kind == MB_DATA_NAV1) {
    /* just one navigation value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation and heading */
    speed[0] = 0.0;
    mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
    mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]), error);
    navlon[0] = RTD * position->longitude;
    navlat[0] = RTD * position->latitude;

    /* get roll pitch and heave */
    mb_attint_interp(verbose, mbio_ptr, *time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &draft[0], error);
      heave[0] = 0.0;
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
      heave[0] = 0.0;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV2) {
    /* get number of available navigation values */
    if (bluefin->data_format == 0 && bluefin->number_frames > 0)
      *n = bluefin->number_frames;
    else
      *n = 0;

    /* loop over navigation values */
    for (int inav = 0; inav < *n; inav++) {
      /* get time */
      time_d[inav] = bluefin->nav[inav].position_time;
      mb_get_date(verbose, time_d[inav], &(time_i[7 * inav]));

      /* get heading */
      heading[inav] = RTD * bluefin->nav[inav].yaw;

      /* get speed */
      speed[inav] = 0.0;
      mb_navint_interp(verbose, mbio_ptr, time_d[inav], heading[inav], speed[inav], &(navlon[inav]), &(navlat[inav]),
                       &(speed[inav]), error);

      /* get navigation */
      navlon[inav] = RTD * bluefin->nav[inav].longitude;
      navlat[inav] = RTD * bluefin->nav[inav].latitude;

      /* get roll pitch and heave */
      roll[inav] = RTD * bluefin->nav[inav].roll;
      pitch[inav] = RTD * bluefin->nav[inav].pitch;
      heave[inav] = 0.0;

      /* get draft  */
      draft[inav] = bluefin->nav[inav].depth;
    }

    /* done translating values */
  }

  /* extract data from structure */
  else if (*kind == MB_DATA_NAV3) {
    /* get number of available navigation values */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get navigation */
    navlon[0] = RTD * navigation->longitude;
    navlat[0] = RTD * navigation->latitude;

    /* get speed  */
    speed[0] = 0.0;

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);
    else if (bathymetry->optionaldata)
      heading[0] = RTD * bathymetry->heading;

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
    }
    else if (bathymetry->optionaldata) {
      roll[0] = RTD * bathymetry->roll;
      pitch[0] = RTD * bathymetry->pitch;
      heave[0] = bathymetry->heave;
    }
    else {
      roll[0] = 0.0;
      pitch[0] = 0.0;
      heave[0] = 0.0;
    }

    /* done translating values */
  }

  /* extract data from attitude structure */
  else if (*kind == MB_DATA_ATTITUDE && store->type == R7KRECID_Attitude) {
    *n = attitude->n;

    for (int iatt=0; iatt < attitude->n; iatt++) {
      /* get time */
      time_d[iatt] = store->time_d + attitude->delta_time[iatt];
      mb_get_date(verbose, time_d[iatt], &(time_i[7 * iatt]));

      /* get interpolated nav heading and speed  */
      speed[iatt] = 0.0;
      heading[iatt] = RTD * attitude->heading[iatt];
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, time_d[iatt], heading[iatt], speed[iatt], &(navlon[iatt]), &(navlat[iatt]), &(speed[iatt]),
                         error);
      else if (bathymetry->optionaldata) {
        navlon[iatt] = RTD * bathymetry->longitude;
        navlat[iatt] = RTD * bathymetry->latitude;
      }

      /* get draft  */
      if (mb_io_ptr->nsensordepth > 0) {
        mb_depint_interp(verbose, mbio_ptr, time_d[iatt], &(draft[iatt]), error);
      }
      else if (bathymetry->optionaldata) {
        draft[iatt] = -bathymetry->vehicle_height + reference->water_z;
      }
      else {
        draft[iatt] = reference->water_z;
      }

      /* get attitude  */
      roll[iatt] = RTD * attitude->roll[iatt];
      pitch[iatt] = RTD * attitude->pitch[iatt];
      heave[iatt] = attitude->heave[iatt];
    }

    /* done translating values */
  }

  /* extract data from attitude structure */
  else if (*kind == MB_DATA_ATTITUDE && store->type == R7KRECID_RollPitchHeave) {
    /* just one navigation value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get interpolated nav heading and speed  */
    speed[0] = 0.0;
    heading[0] = 0.0;
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, &(heading[0]), error);
    else if (bathymetry->optionaldata)
      heading[0] = RTD * bathymetry->heading;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]),
                       error);
    else if (bathymetry->optionaldata) {
      navlon[0] = RTD * bathymetry->longitude;
      navlat[0] = RTD * bathymetry->latitude;
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    roll[0] = RTD * rollpitchheave->roll;
    pitch[0] = RTD * rollpitchheave->pitch;
    heave[0] = rollpitchheave->heave;

    /* done translating values */
  }

  /* extract data from customattitude structure */
  else if (*kind == MB_DATA_ATTITUDE && store->type == R7KRECID_CustomAttitude) {
    *n = customattitude->n;

    for (int iatt=0; iatt < customattitude->n; iatt++) {
      /* get time */
      time_d[iatt] = (double)(store->time_d + ((double)iatt) / ((double)customattitude->frequency));
      mb_get_date(verbose, time_d[iatt], &(time_i[7 * iatt]));

      /* get interpolated nav heading and speed  */
      speed[iatt] = 0.0;
      heading[iatt] = RTD * customattitude->heading[iatt];
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, time_d[iatt], heading[iatt], speed[iatt], &(navlon[iatt]), &(navlat[iatt]), &(speed[iatt]),
                         error);
      else if (bathymetry->optionaldata) {
        navlon[iatt] = RTD * bathymetry->longitude;
        navlat[iatt] = RTD * bathymetry->latitude;
      }

      /* get draft  */
      if (mb_io_ptr->nsensordepth > 0) {
        mb_depint_interp(verbose, mbio_ptr, time_d[iatt], &(draft[iatt]), error);
      }
      else if (bathymetry->optionaldata) {
        draft[iatt] = -bathymetry->vehicle_height + reference->water_z;
      }
      else {
        draft[iatt] = reference->water_z;
      }

      /* get attitude  */
      roll[iatt] = RTD * customattitude->roll[iatt];
      pitch[iatt] = RTD * customattitude->pitch[iatt];
      heave[iatt] = customattitude->heave[iatt];
    }

    /* done translating values */
  }

  /* extract data from heading structure */
  else if (*kind == MB_DATA_HEADING) {
    /* just one heading value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get interpolated nav heading and speed  */
    speed[0] = 0.0;
    heading[0] = RTD * headings->heading;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]),
                       error);
    else if (bathymetry->optionaldata) {
      navlon[0] = RTD * bathymetry->longitude;
      navlat[0] = RTD * bathymetry->latitude;
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
    }
    else if (bathymetry->optionaldata) {
      roll[0] = RTD * bathymetry->roll;
      pitch[0] = RTD * bathymetry->pitch;
      heave[0] = bathymetry->heave;
    }
    else {
      roll[0] = 0.0;
      pitch[0] = 0.0;
      heave[0] = 0.0;
    }

    /* done translating values */
  }

  /* extract data from sensordepth structure */
  else if (*kind == MB_DATA_SENSORDEPTH) {
    /* just one sensordepth value */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    time_d[0] = store->time_d;

    /* get interpolated nav heading and speed  */
    speed[0] = 0.0;
    heading[0] = RTD * headings->heading;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, heading[0], speed[0], &(navlon[0]), &(navlat[0]), &(speed[0]),
                       error);
    else if (bathymetry->optionaldata) {
      navlon[0] = RTD * bathymetry->longitude;
      navlat[0] = RTD * bathymetry->latitude;
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, &(draft[0]), error);
    }
    else if (bathymetry->optionaldata) {
      draft[0] = -bathymetry->vehicle_height + reference->water_z;
    }
    else {
      draft[0] = reference->water_z;
    }

    /* get attitude  */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, store->time_d, &(heave[0]), &(roll[0]), &(pitch[0]), error);
    }
    else if (bathymetry->optionaldata) {
      roll[0] = RTD * bathymetry->roll;
      pitch[0] = RTD * bathymetry->pitch;
      heave[0] = bathymetry->heave;
    }
    else {
      roll[0] = 0.0;
      pitch[0] = 0.0;
      heave[0] = 0.0;
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

    /* get number of available navigation values */
    *n = 1;

    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
                             double navlat, double speed, double heading, double draft, double roll, double pitch, double heave,
                             int *error) {
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

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
  s7kr_position *position = (s7kr_position *)&store->position;
  // s7kr_depth *depth = (s7kr_depth *)&store->depth;
  // s7kr_attitude *attitude = (s7kr_attitude *)&store->attitude;
  s7kr_reference *reference = (s7kr_reference *)&store->reference;

  /* insert data in ping structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    bathymetry->longitude = DTR * navlon;
    bathymetry->latitude = DTR * navlat;

    /* get heading */
    bathymetry->heading = DTR * heading;

    /* get speed  */

    /* get draft  */
    bathymetry->vehicle_height = reference->water_z - draft;

    /* get roll pitch and heave */
    bathymetry->heave = heave;
    bathymetry->pitch = DTR * pitch;
    bathymetry->roll = DTR * roll;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get navigation */
    position->longitude = DTR * navlon;
    position->latitude = DTR * navlat;

    /* get heading */

    /* get speed  */

    /* get draft  */

    /* get roll pitch and heave */
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_svp *svp = (s7kr_svp *)&store->svp;
  s7kr_ctd *ctd = (s7kr_ctd *)&store->ctd;;
  double latitude = 0.0;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_VELOCITY_PROFILE) {
    /* get number of depth-velocity pairs */
    *nsvp = svp->n;

    /* get profile */
    for (int i = 0; i < *nsvp; i++) {
      depth[i] = svp->depth[i];
      velocity[i] = svp->sound_velocity[i];
    }

    /* done translating values */
  }

  else if (*kind == MB_DATA_CTD) {
    /* get number of depth-velocity pairs */
    *nsvp = ctd->n;

    if (ctd->pressure_flag == 0) {
      if (ctd->latitude != 0.0) {
        latitude = RTD * ctd->latitude;
      }
      else if (store->read_bathymetry && store->bathymetry.optionaldata
          && store->bathymetry.latitude != 0.0) {
          latitude = RTD * store->bathymetry.latitude;
      }
      else if (store->position.latitude != 0.0 && store->position.type == 0) {
        latitude = RTD * store->position.latitude;
      }
      else if (store->navigation.latitude != 0.0) {
        latitude = RTD * store->navigation.latitude;
      }
    }

    /* get profile */
    for (int i = 0; i < *nsvp; i++) {
      if (ctd->pressure_flag == 0) {
        status = mb_seabird_depth(verbose, (double)ctd->pressure_depth[i], latitude, &depth[i], error);
      }
      else {
        depth[i] = ctd->pressure_depth[i];
      }
      velocity[i] = ctd->sound_velocity[i];
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
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

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_svp *svp = (s7kr_svp *)&store->svp;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_VELOCITY_PROFILE) {
    /* allocate memory if necessary */
    if (svp->nalloc < (unsigned int) nsvp) {
      status &= mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(svp->depth), error);
      status &= mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(svp->sound_velocity), error);
      if (status == MB_SUCCESS) {
        svp->nalloc = nsvp;
      }
      else {
        svp->n = 0;
        svp->nalloc = 0;
      }
    }

    /* get profile */
    if (status == MB_SUCCESS) {
      svp->n = nsvp;
      for (unsigned int i = 0; i < svp->n; i++) {
        svp->depth[i] = depth[i];
        svp->sound_velocity[i] = velocity[i];
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
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_segytraceheader(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void *segytraceheader_ptr,
                                          int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:           %d\n", *kind);
    fprintf(stderr, "dbg2       segytraceheader_ptr: %p\n", (void *)segytraceheader_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  struct mb_segytraceheader_struct *mb_segytraceheader_ptr;
  s7kr_bathymetry *bathymetry;
  s7kr_fsdwsb *fsdwsb;
  s7k_fsdwchannel *fsdwchannel;
  double dsensordepth, dsonaraltitude;
  double longitude, latitude;
  double speed, heading;
  double roll, pitch, heave;
  int time_j[5];

  /* extract data from structure */
  if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* get relevant structures */
    mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segytraceheader_ptr;
    bathymetry = &(store->bathymetry);
    s7kr_bluefin *bluefin = &(store->bluefin);
    s7kr_ctd *ctd = &(store->ctd);
    fsdwsb = &(store->fsdwsb);
    // s7k_header *header = &(fsdwsb->header);
    fsdwchannel = &(fsdwsb->channel);
    // s7k_fsdwsegyheader *fsdwsegyheader = &(fsdwsb->segyheader);

    /* get needed values */
    mb_depint_interp(verbose, mbio_ptr, store->time_d, &dsensordepth, error);
    mb_altint_interp(verbose, mbio_ptr, store->time_d, &dsonaraltitude, error);
    double dwaterdepth = dsensordepth + dsonaraltitude;

    /* if possible get altitude from nadir of multibeam bathymetry */
    if (bathymetry->optionaldata) {
      /* get depth closest to nadir */
      double xtrackmin = 999999.9;
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        if (((bathymetry->quality[i] & 15) == 15) && fabs((double)bathymetry->acrosstrack[i]) < xtrackmin) {
          dwaterdepth = bathymetry->depth[i];
          dsonaraltitude = bathymetry->depth[i] - dsensordepth;
          xtrackmin = fabs((double)bathymetry->acrosstrack[i]);
        }
      }
    }

    /* get needed values */
    const int sensordepth = (int)(100 * dsensordepth);
    const int waterdepth = (int)(100 * dwaterdepth);
    int watersoundspeed;
    if (ctd->n > 0)
      watersoundspeed = (int)(ctd->sound_velocity[ctd->n - 1]);
    else if (bluefin->environmental[0].sound_speed > 0.0)
      watersoundspeed = (int)(bluefin->environmental[0].sound_speed);
    else
      watersoundspeed = 1500;
    const float fwatertime = 2.0 * dwaterdepth / ((double)watersoundspeed);

    mb_hedint_interp(verbose, mbio_ptr, store->time_d, &heading, error);
    speed = 0.0;
    mb_navint_interp(verbose, mbio_ptr, store->time_d, heading, speed, &longitude, &latitude, &speed, error);
    mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave, &roll, &pitch, error);
    if (longitude == 0.0 && latitude == 0.0 && bathymetry->longitude != 0.0 && bathymetry->latitude != 0.0) {
      longitude = RTD * bathymetry->longitude;
      latitude = RTD * bathymetry->latitude;
    }
    mb_get_jtime(verbose, store->time_i, time_j);

    /* extract the data */
    mb_segytraceheader_ptr->seq_num = fsdwsb->ping_number;
    mb_segytraceheader_ptr->seq_reel = fsdwsb->ping_number;
    mb_segytraceheader_ptr->shot_num = fsdwsb->ping_number;
    mb_segytraceheader_ptr->shot_tr = 1;
    mb_segytraceheader_ptr->espn = 0;
    mb_segytraceheader_ptr->rp_num = fsdwsb->ping_number;
    mb_segytraceheader_ptr->rp_tr = 1;
    mb_segytraceheader_ptr->trc_id = 1;
    mb_segytraceheader_ptr->num_vstk = 0;
    mb_segytraceheader_ptr->cdp_fold = 0;
    mb_segytraceheader_ptr->use = fsdwsb->data_format;
    mb_segytraceheader_ptr->range = 0;
    mb_segytraceheader_ptr->grp_elev = -sensordepth;
    mb_segytraceheader_ptr->src_elev = -sensordepth;
    mb_segytraceheader_ptr->src_depth = sensordepth;
    mb_segytraceheader_ptr->grp_datum = 0;
    mb_segytraceheader_ptr->src_datum = 0;
    mb_segytraceheader_ptr->src_wbd = waterdepth;
    mb_segytraceheader_ptr->grp_wbd = waterdepth;
    mb_segytraceheader_ptr->elev_scalar = -100; /* 0.01 m precision for depths */
    mb_segytraceheader_ptr->coord_scalar = -100; /* 0.01 arc second precision for position
                         = 0.3 m precision at equator */
    mb_segytraceheader_ptr->src_long = (int)(longitude * 360000.0);
    mb_segytraceheader_ptr->src_lat = (int)(latitude * 360000.0);
    mb_segytraceheader_ptr->grp_long = (int)(longitude * 360000.0);
    mb_segytraceheader_ptr->grp_lat = (int)(latitude * 360000.0);
    mb_segytraceheader_ptr->coord_units = 2;
    mb_segytraceheader_ptr->wvel = watersoundspeed;
    mb_segytraceheader_ptr->sbvel = 0;
    mb_segytraceheader_ptr->src_up_vel = 0;
    mb_segytraceheader_ptr->grp_up_vel = 0;
    mb_segytraceheader_ptr->src_static = 0;
    mb_segytraceheader_ptr->grp_static = 0;
    mb_segytraceheader_ptr->tot_static = 0;
    mb_segytraceheader_ptr->laga = 0;
    mb_segytraceheader_ptr->delay_mils = 0;
    mb_segytraceheader_ptr->smute_mils = 0;
    mb_segytraceheader_ptr->emute_mils = 0;
    mb_segytraceheader_ptr->nsamps = fsdwchannel->number_samples;
    mb_segytraceheader_ptr->si_micros = fsdwchannel->sample_interval;
    for (int i = 0; i < 19; i++)
      mb_segytraceheader_ptr->other_1[i] = 0;
    mb_segytraceheader_ptr->year = store->time_i[0];
    mb_segytraceheader_ptr->day_of_yr = time_j[1];
    mb_segytraceheader_ptr->hour = store->time_i[3];
    mb_segytraceheader_ptr->min = store->time_i[4];
    mb_segytraceheader_ptr->sec = store->time_i[5];
    mb_segytraceheader_ptr->mils = store->time_i[6] / 1000;
    mb_segytraceheader_ptr->tr_weight = 1;
    for (int i = 0; i < 5; i++)
      mb_segytraceheader_ptr->other_2[i] = 0;
    mb_segytraceheader_ptr->delay = 0.0;
    mb_segytraceheader_ptr->smute_sec = 0.0;
    mb_segytraceheader_ptr->emute_sec = 0.0;
    mb_segytraceheader_ptr->si_secs = 0.000001 * ((float)fsdwchannel->sample_interval);
    mb_segytraceheader_ptr->wbt_secs = fwatertime;
    mb_segytraceheader_ptr->end_of_rp = 0;
    mb_segytraceheader_ptr->dummy1 = 0.0;
    mb_segytraceheader_ptr->dummy2 = 0.0;
    mb_segytraceheader_ptr->dummy3 = 0.0;
    mb_segytraceheader_ptr->dummy4 = 0.0;
    mb_segytraceheader_ptr->soundspeed = watersoundspeed;
    mb_segytraceheader_ptr->distance = 0.0;
    mb_segytraceheader_ptr->roll = (float)roll;
    mb_segytraceheader_ptr->pitch = (float)pitch;
    mb_segytraceheader_ptr->heading = (float)heading;

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
    if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
      mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segytraceheader_ptr;
      fprintf(stderr, "dbg2       seq_num:           %d\n", mb_segytraceheader_ptr->seq_num);
      fprintf(stderr, "dbg2       seq_reel:          %d\n", mb_segytraceheader_ptr->seq_reel);
      fprintf(stderr, "dbg2       shot_num:          %d\n", mb_segytraceheader_ptr->shot_num);
      fprintf(stderr, "dbg2       shot_tr:           %d\n", mb_segytraceheader_ptr->shot_tr);
      fprintf(stderr, "dbg2       espn:              %d\n", mb_segytraceheader_ptr->espn);
      fprintf(stderr, "dbg2       rp_num:            %d\n", mb_segytraceheader_ptr->rp_num);
      fprintf(stderr, "dbg2       rp_tr:             %d\n", mb_segytraceheader_ptr->rp_tr);
      fprintf(stderr, "dbg2       trc_id:            %d\n", mb_segytraceheader_ptr->trc_id);
      fprintf(stderr, "dbg2       num_vstk:          %d\n", mb_segytraceheader_ptr->num_vstk);
      fprintf(stderr, "dbg2       cdp_fold:          %d\n", mb_segytraceheader_ptr->cdp_fold);
      fprintf(stderr, "dbg2       use:               %d\n", mb_segytraceheader_ptr->use);
      fprintf(stderr, "dbg2       range:             %d\n", mb_segytraceheader_ptr->range);
      fprintf(stderr, "dbg2       grp_elev:          %d\n", mb_segytraceheader_ptr->grp_elev);
      fprintf(stderr, "dbg2       src_elev:          %d\n", mb_segytraceheader_ptr->src_elev);
      fprintf(stderr, "dbg2       src_depth:         %d\n", mb_segytraceheader_ptr->src_depth);
      fprintf(stderr, "dbg2       grp_datum:         %d\n", mb_segytraceheader_ptr->grp_datum);
      fprintf(stderr, "dbg2       src_datum:         %d\n", mb_segytraceheader_ptr->src_datum);
      fprintf(stderr, "dbg2       src_wbd:           %d\n", mb_segytraceheader_ptr->src_wbd);
      fprintf(stderr, "dbg2       grp_wbd:           %d\n", mb_segytraceheader_ptr->grp_wbd);
      fprintf(stderr, "dbg2       elev_scalar:       %d\n", mb_segytraceheader_ptr->elev_scalar);
      fprintf(stderr, "dbg2       coord_scalar:      %d\n", mb_segytraceheader_ptr->coord_scalar);
      fprintf(stderr, "dbg2       src_long:          %d\n", mb_segytraceheader_ptr->src_long);
      fprintf(stderr, "dbg2       src_lat:           %d\n", mb_segytraceheader_ptr->src_lat);
      fprintf(stderr, "dbg2       grp_long:          %d\n", mb_segytraceheader_ptr->grp_long);
      fprintf(stderr, "dbg2       grp_lat:           %d\n", mb_segytraceheader_ptr->grp_lat);
      fprintf(stderr, "dbg2       coord_units:       %d\n", mb_segytraceheader_ptr->coord_units);
      fprintf(stderr, "dbg2       wvel:              %d\n", mb_segytraceheader_ptr->wvel);
      fprintf(stderr, "dbg2       sbvel:             %d\n", mb_segytraceheader_ptr->sbvel);
      fprintf(stderr, "dbg2       src_up_vel:        %d\n", mb_segytraceheader_ptr->src_up_vel);
      fprintf(stderr, "dbg2       grp_up_vel:        %d\n", mb_segytraceheader_ptr->grp_up_vel);
      fprintf(stderr, "dbg2       src_static:        %d\n", mb_segytraceheader_ptr->src_static);
      fprintf(stderr, "dbg2       grp_static:        %d\n", mb_segytraceheader_ptr->grp_static);
      fprintf(stderr, "dbg2       tot_static:        %d\n", mb_segytraceheader_ptr->tot_static);
      fprintf(stderr, "dbg2       laga:              %d\n", mb_segytraceheader_ptr->laga);
      fprintf(stderr, "dbg2       delay_mils:        %d\n", mb_segytraceheader_ptr->delay_mils);
      fprintf(stderr, "dbg2       smute_mils:        %d\n", mb_segytraceheader_ptr->smute_mils);
      fprintf(stderr, "dbg2       emute_mils:        %d\n", mb_segytraceheader_ptr->emute_mils);
      fprintf(stderr, "dbg2       nsamps:            %d\n", mb_segytraceheader_ptr->nsamps);
      fprintf(stderr, "dbg2       si_micros:         %d\n", mb_segytraceheader_ptr->si_micros);
      for (int i = 0; i < 19; i++)
        fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
      fprintf(stderr, "dbg2       year:              %d\n", mb_segytraceheader_ptr->year);
      fprintf(stderr, "dbg2       day_of_yr:         %d\n", mb_segytraceheader_ptr->day_of_yr);
      fprintf(stderr, "dbg2       hour:              %d\n", mb_segytraceheader_ptr->hour);
      fprintf(stderr, "dbg2       min:               %d\n", mb_segytraceheader_ptr->min);
      fprintf(stderr, "dbg2       sec:               %d\n", mb_segytraceheader_ptr->sec);
      fprintf(stderr, "dbg2       mils:              %d\n", mb_segytraceheader_ptr->mils);
      fprintf(stderr, "dbg2       tr_weight:         %d\n", mb_segytraceheader_ptr->tr_weight);
      for (int i = 0; i < 5; i++)
        fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
      fprintf(stderr, "dbg2       delay:             %f\n", mb_segytraceheader_ptr->delay);
      fprintf(stderr, "dbg2       smute_sec:         %f\n", mb_segytraceheader_ptr->smute_sec);
      fprintf(stderr, "dbg2       emute_sec:         %f\n", mb_segytraceheader_ptr->emute_sec);
      fprintf(stderr, "dbg2       si_secs:           %f\n", mb_segytraceheader_ptr->si_secs);
      fprintf(stderr, "dbg2       wbt_secs:          %f\n", mb_segytraceheader_ptr->wbt_secs);
      fprintf(stderr, "dbg2       end_of_rp:         %d\n", mb_segytraceheader_ptr->end_of_rp);
      fprintf(stderr, "dbg2       dummy1:            %f\n", mb_segytraceheader_ptr->dummy1);
      fprintf(stderr, "dbg2       dummy2:            %f\n", mb_segytraceheader_ptr->dummy2);
      fprintf(stderr, "dbg2       dummy3:            %f\n", mb_segytraceheader_ptr->dummy3);
      fprintf(stderr, "dbg2       dummy4:            %f\n", mb_segytraceheader_ptr->dummy4);
      fprintf(stderr, "dbg2       soundspeed:        %f\n", mb_segytraceheader_ptr->soundspeed);
      fprintf(stderr, "dbg2       distance:          %f\n", mb_segytraceheader_ptr->distance);
      fprintf(stderr, "dbg2       roll:              %f\n", mb_segytraceheader_ptr->roll);
      fprintf(stderr, "dbg2       pitch:             %f\n", mb_segytraceheader_ptr->pitch);
      fprintf(stderr, "dbg2       heading:           %f\n", mb_segytraceheader_ptr->heading);
    }
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_extract_segy(int verbose, void *mbio_ptr, void *store_ptr, int *sampleformat, int *kind, void *segyheader_ptr,
                               float *segydata, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:            %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:         %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       sampleformat:      %d\n", *sampleformat);
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       segyheader_ptr:    %p\n", (void *)segyheader_ptr);
    fprintf(stderr, "dbg2       segydata:          %p\n", (void *)segydata);
  }

  struct mb_segytraceheader_struct *mb_segytraceheader_ptr;

  /* get data structure pointer */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* extract segy header */
    status = mbsys_reson7k_extract_segytraceheader(verbose, mbio_ptr, store_ptr, kind, segyheader_ptr, error);

    /* get relevant structures */
    mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segyheader_ptr;
    s7kr_fsdwsb *fsdwsb = &(store->fsdwsb);
    // s7k_header *header = &(fsdwsb->header);
    s7k_fsdwchannel *fsdwchannel = &(fsdwsb->channel);
    s7k_fsdwsegyheader *fsdwsegyheader = &(fsdwsb->segyheader);
    short *shortptr = (short *)fsdwchannel->data;
    unsigned short *ushortptr = (unsigned short *)fsdwchannel->data;

    /* get the trace weight */
    const double weight = exp(MB_LN_2 * ((double)fsdwsegyheader->weightingFactor));

    /* extract the data */
    if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_ENVELOPE) {
      *sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        segydata[i] = (float)(((double)ushortptr[i]) / weight);
      }
    }
    else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_ANALYTIC) {
      /* if no format specified do envelope by default */
      if (*sampleformat == MB_SEGY_SAMPLEFORMAT_NONE)
        *sampleformat = MB_SEGY_SAMPLEFORMAT_ENVELOPE;

      /* convert analytic data to desired envelope */
      if (*sampleformat == MB_SEGY_SAMPLEFORMAT_ENVELOPE) {
        for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
          segydata[i] =
              (float)(sqrt((double)(shortptr[2 * i] * shortptr[2 * i] + shortptr[2 * i + 1] * shortptr[2 * i + 1])) /
                      weight);
        }
      }

      /* else extract desired analytic data */
      else if (*sampleformat == MB_SEGY_SAMPLEFORMAT_ANALYTIC) {
        for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
          segydata[2 * i] = (float)(((double)shortptr[2 * i]) / weight);
          segydata[2 * i + 1] = (float)(((double)shortptr[2 * i + 1]) / weight);
        }
      }

      /* else extract desired real trace from analytic data */
      else if (*sampleformat == MB_SEGY_SAMPLEFORMAT_TRACE) {
        for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
          segydata[i] = (float)(((double)shortptr[2 * i]) / weight);
        }
      }
    }
    else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_RAW) {
      *sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        segydata[i] = (float)(((double)ushortptr[i]) / weight);
      }
    }
    else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC) {
      *sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        segydata[i] = (float)(((double)ushortptr[i]) / weight);
      }
    }
    else if (fsdwsb->data_format == EDGETECH_TRACEFORMAT_PIXEL) {
      *sampleformat = MB_SEGY_SAMPLEFORMAT_TRACE;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        segydata[i] = (float)(((double)ushortptr[i]) / weight);
      }
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
    fprintf(stderr, "dbg2       sampleformat:      %d\n", *sampleformat);
    fprintf(stderr, "dbg2       kind:              %d\n", *kind);
    fprintf(stderr, "dbg2       seq_num:           %d\n", mb_segytraceheader_ptr->seq_num);
    fprintf(stderr, "dbg2       seq_reel:          %d\n", mb_segytraceheader_ptr->seq_reel);
    fprintf(stderr, "dbg2       shot_num:          %d\n", mb_segytraceheader_ptr->shot_num);
    fprintf(stderr, "dbg2       shot_tr:           %d\n", mb_segytraceheader_ptr->shot_tr);
    fprintf(stderr, "dbg2       espn:              %d\n", mb_segytraceheader_ptr->espn);
    fprintf(stderr, "dbg2       rp_num:            %d\n", mb_segytraceheader_ptr->rp_num);
    fprintf(stderr, "dbg2       rp_tr:             %d\n", mb_segytraceheader_ptr->rp_tr);
    fprintf(stderr, "dbg2       trc_id:            %d\n", mb_segytraceheader_ptr->trc_id);
    fprintf(stderr, "dbg2       num_vstk:          %d\n", mb_segytraceheader_ptr->num_vstk);
    fprintf(stderr, "dbg2       cdp_fold:          %d\n", mb_segytraceheader_ptr->cdp_fold);
    fprintf(stderr, "dbg2       use:               %d\n", mb_segytraceheader_ptr->use);
    fprintf(stderr, "dbg2       range:             %d\n", mb_segytraceheader_ptr->range);
    fprintf(stderr, "dbg2       grp_elev:          %d\n", mb_segytraceheader_ptr->grp_elev);
    fprintf(stderr, "dbg2       src_elev:          %d\n", mb_segytraceheader_ptr->src_elev);
    fprintf(stderr, "dbg2       src_depth:         %d\n", mb_segytraceheader_ptr->src_depth);
    fprintf(stderr, "dbg2       grp_datum:         %d\n", mb_segytraceheader_ptr->grp_datum);
    fprintf(stderr, "dbg2       src_datum:         %d\n", mb_segytraceheader_ptr->src_datum);
    fprintf(stderr, "dbg2       src_wbd:           %d\n", mb_segytraceheader_ptr->src_wbd);
    fprintf(stderr, "dbg2       grp_wbd:           %d\n", mb_segytraceheader_ptr->grp_wbd);
    fprintf(stderr, "dbg2       elev_scalar:       %d\n", mb_segytraceheader_ptr->elev_scalar);
    fprintf(stderr, "dbg2       coord_scalar:      %d\n", mb_segytraceheader_ptr->coord_scalar);
    fprintf(stderr, "dbg2       src_long:          %d\n", mb_segytraceheader_ptr->src_long);
    fprintf(stderr, "dbg2       src_lat:           %d\n", mb_segytraceheader_ptr->src_lat);
    fprintf(stderr, "dbg2       grp_long:          %d\n", mb_segytraceheader_ptr->grp_long);
    fprintf(stderr, "dbg2       grp_lat:           %d\n", mb_segytraceheader_ptr->grp_lat);
    fprintf(stderr, "dbg2       coord_units:       %d\n", mb_segytraceheader_ptr->coord_units);
    fprintf(stderr, "dbg2       wvel:              %d\n", mb_segytraceheader_ptr->wvel);
    fprintf(stderr, "dbg2       sbvel:             %d\n", mb_segytraceheader_ptr->sbvel);
    fprintf(stderr, "dbg2       src_up_vel:        %d\n", mb_segytraceheader_ptr->src_up_vel);
    fprintf(stderr, "dbg2       grp_up_vel:        %d\n", mb_segytraceheader_ptr->grp_up_vel);
    fprintf(stderr, "dbg2       src_static:        %d\n", mb_segytraceheader_ptr->src_static);
    fprintf(stderr, "dbg2       grp_static:        %d\n", mb_segytraceheader_ptr->grp_static);
    fprintf(stderr, "dbg2       tot_static:        %d\n", mb_segytraceheader_ptr->tot_static);
    fprintf(stderr, "dbg2       laga:              %d\n", mb_segytraceheader_ptr->laga);
    fprintf(stderr, "dbg2       delay_mils:        %d\n", mb_segytraceheader_ptr->delay_mils);
    fprintf(stderr, "dbg2       smute_mils:        %d\n", mb_segytraceheader_ptr->smute_mils);
    fprintf(stderr, "dbg2       emute_mils:        %d\n", mb_segytraceheader_ptr->emute_mils);
    fprintf(stderr, "dbg2       nsamps:            %d\n", mb_segytraceheader_ptr->nsamps);
    fprintf(stderr, "dbg2       si_micros:         %d\n", mb_segytraceheader_ptr->si_micros);
    for (int i = 0; i < 19; i++)
      fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
    fprintf(stderr, "dbg2       year:              %d\n", mb_segytraceheader_ptr->year);
    fprintf(stderr, "dbg2       day_of_yr:         %d\n", mb_segytraceheader_ptr->day_of_yr);
    fprintf(stderr, "dbg2       hour:              %d\n", mb_segytraceheader_ptr->hour);
    fprintf(stderr, "dbg2       min:               %d\n", mb_segytraceheader_ptr->min);
    fprintf(stderr, "dbg2       sec:               %d\n", mb_segytraceheader_ptr->sec);
    fprintf(stderr, "dbg2       mils:              %d\n", mb_segytraceheader_ptr->mils);
    fprintf(stderr, "dbg2       tr_weight:         %d\n", mb_segytraceheader_ptr->tr_weight);
    for (int i = 0; i < 5; i++)
      fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
    fprintf(stderr, "dbg2       delay:             %f\n", mb_segytraceheader_ptr->delay);
    fprintf(stderr, "dbg2       smute_sec:         %f\n", mb_segytraceheader_ptr->smute_sec);
    fprintf(stderr, "dbg2       emute_sec:         %f\n", mb_segytraceheader_ptr->emute_sec);
    fprintf(stderr, "dbg2       si_secs:           %f\n", mb_segytraceheader_ptr->si_secs);
    fprintf(stderr, "dbg2       wbt_secs:          %f\n", mb_segytraceheader_ptr->wbt_secs);
    fprintf(stderr, "dbg2       end_of_rp:         %d\n", mb_segytraceheader_ptr->end_of_rp);
    fprintf(stderr, "dbg2       dummy1:            %f\n", mb_segytraceheader_ptr->dummy1);
    fprintf(stderr, "dbg2       dummy2:            %f\n", mb_segytraceheader_ptr->dummy2);
    fprintf(stderr, "dbg2       dummy3:            %f\n", mb_segytraceheader_ptr->dummy3);
    fprintf(stderr, "dbg2       dummy4:            %f\n", mb_segytraceheader_ptr->dummy4);
    fprintf(stderr, "dbg2       soundspeed:        %f\n", mb_segytraceheader_ptr->soundspeed);
    fprintf(stderr, "dbg2       distance:          %f\n", mb_segytraceheader_ptr->distance);
    fprintf(stderr, "dbg2       roll:              %f\n", mb_segytraceheader_ptr->roll);
    fprintf(stderr, "dbg2       pitch:             %f\n", mb_segytraceheader_ptr->pitch);
    fprintf(stderr, "dbg2       heading:           %f\n", mb_segytraceheader_ptr->heading);
    for (int i = 0; i < mb_segytraceheader_ptr->nsamps; i++)
      fprintf(stderr, "dbg2       segydata[%d]:      %f\n", i, segydata[i]);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_insert_segy(int verbose, void *mbio_ptr, void *store_ptr, int kind, void *segyheader_ptr, float *segydata,
                              int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       kind:           %d\n", kind);
    fprintf(stderr, "dbg2       segyheader_ptr: %p\n", (void *)segyheader_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  store->kind = kind;

  int status = MB_SUCCESS;

  struct mb_segytraceheader_struct *mb_segytraceheader_ptr = NULL;

  /* insert data to structure */
  if (store->kind == MB_DATA_SUBBOTTOM_SUBBOTTOM) {
    /* get relevant structures */
    mb_segytraceheader_ptr = (struct mb_segytraceheader_struct *)segyheader_ptr;
    // s7kr_bathymetry *bathymetry = &(store->bathymetry);
    // s7kr_ctd *ctd = &(store->ctd);
    s7kr_fsdwsb *fsdwsb = &(store->fsdwsb);
    s7k_header *header = &(fsdwsb->header);
    s7k_fsdwchannel *fsdwchannel = &(fsdwsb->channel);
    s7k_fsdwsegyheader *fsdwsegyheader = &(fsdwsb->segyheader);

    /* get needed values */
    double dsensordepth;
    mb_depint_interp(verbose, mbio_ptr, store->time_d, &dsensordepth, error);
    double dsonaraltitude;
    mb_altint_interp(verbose, mbio_ptr, store->time_d, &dsonaraltitude, error);
    // const double dwaterdepth = dsensordepth + dsonaraltitude;
    // int watersoundspeed;
    // if (ctd->n > 0)
    //   watersoundspeed = (int)(ctd->sound_velocity[ctd->n - 1]);
    // else
    //   watersoundspeed = 1500;
    // const float fwatertime = 2.0 * 0.01 * ((double)waterdepth) / ((double)watersoundspeed);
    int time_j[5];
    mb_get_jtime(verbose, store->time_i, time_j);

    /* extract the data */
    if (mb_segytraceheader_ptr->shot_num != 0)
      fsdwsb->ping_number = mb_segytraceheader_ptr->shot_num;
    else if (mb_segytraceheader_ptr->seq_reel != 0)
      fsdwsb->ping_number = mb_segytraceheader_ptr->seq_reel;
    else if (mb_segytraceheader_ptr->seq_num != 0)
      fsdwsb->ping_number = mb_segytraceheader_ptr->seq_num;
    else if (mb_segytraceheader_ptr->rp_num != 0)
      fsdwsb->ping_number = mb_segytraceheader_ptr->rp_num;
    else
      fsdwsb->ping_number = 0;
    fsdwsb->data_format = mb_segytraceheader_ptr->use;
    // int sensordepth = (int)(100 * dsensordepth);
    // if (mb_segytraceheader_ptr->grp_elev != 0)
    //   sensordepth = -mb_segytraceheader_ptr->grp_elev;
    // else if (mb_segytraceheader_ptr->src_elev != 0)
    //   sensordepth = -mb_segytraceheader_ptr->src_elev;
    // else if (mb_segytraceheader_ptr->src_depth != 0)
    //   sensordepth = mb_segytraceheader_ptr->src_depth;
    // else
    //   sensordepth = 0;

    // float factor;
    // if (mb_segytraceheader_ptr->elev_scalar < 0)
    //   factor = 1.0 / ((float)(-mb_segytraceheader_ptr->elev_scalar));
    // else
    //   factor = (float)mb_segytraceheader_ptr->elev_scalar;

    // int waterdepth = (int)(100 * dwaterdepth);
    // if (mb_segytraceheader_ptr->src_wbd != 0)
    //   waterdepth = -mb_segytraceheader_ptr->grp_elev;
    // else if (mb_segytraceheader_ptr->grp_wbd != 0)
    //   waterdepth = -mb_segytraceheader_ptr->src_elev;
    // else
    //   waterdepth = 0;
    // if (mb_segytraceheader_ptr->coord_scalar < 0)
    //   factor = 1.0 / ((float)(-mb_segytraceheader_ptr->coord_scalar)) / 3600.0;
    // else
    //   factor = (float)mb_segytraceheader_ptr->coord_scalar / 3600.0;
    fsdwchannel->number_samples = mb_segytraceheader_ptr->nsamps;
    fsdwchannel->sample_interval = mb_segytraceheader_ptr->si_micros;
    time_j[0] = mb_segytraceheader_ptr->year;
    time_j[1] = mb_segytraceheader_ptr->day_of_yr;
    time_j[2] = 60 * mb_segytraceheader_ptr->hour + mb_segytraceheader_ptr->min;
    time_j[3] = mb_segytraceheader_ptr->sec;
    time_j[4] = 1000 * mb_segytraceheader_ptr->mils;
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
    header->s7kTime.Year = time_j[0];
    header->s7kTime.Day = time_j[1];
    header->s7kTime.Seconds = 0.000001 * store->time_i[6] + store->time_i[5];
    header->s7kTime.Hours = store->time_i[3];
    header->s7kTime.Minutes = store->time_i[4];

    /* get max data value */
    float datamax = 0.0;
    for (int i = 0; i < mb_segytraceheader_ptr->nsamps; i++) {
      if (fabs(segydata[i]) > datamax)
        datamax = fabs(segydata[i]);
    }
    if (datamax > 0.0) {
      fsdwsegyheader->weightingFactor = (short)(log(datamax) / MB_LN_2) - 15;
    }
    else
      fsdwsegyheader->weightingFactor = 0;
    const double weight = pow(2.0, (double)fsdwsegyheader->weightingFactor);
    fsdwchannel->bytespersample = 2;

    /* make sure enough memory is allocated for channel data */
    const unsigned int data_size = fsdwchannel->bytespersample * fsdwchannel->number_samples;
    if (fsdwchannel->data_alloc < data_size) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(fsdwchannel->data), error);
      if (status == MB_SUCCESS) {
        fsdwchannel->data_alloc = data_size;
      }
      else {
        fsdwchannel->data_alloc = 0;
        fsdwchannel->number_samples = 0;
      }
    }

    /* copy over the data */
    if (fsdwchannel->data_alloc >= data_size) {
      short *shortptr = (short *)fsdwchannel->data;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        shortptr[i] = (short)(segydata[i] * weight);
      }
    }

    /* done translating values */
  }

  /* deal with comment */
  else if (kind == MB_DATA_COMMENT) {
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
    fprintf(stderr, "dbg2       seq_num:           %d\n", mb_segytraceheader_ptr->seq_num);
    fprintf(stderr, "dbg2       seq_reel:          %d\n", mb_segytraceheader_ptr->seq_reel);
    fprintf(stderr, "dbg2       shot_num:          %d\n", mb_segytraceheader_ptr->shot_num);
    fprintf(stderr, "dbg2       shot_tr:           %d\n", mb_segytraceheader_ptr->shot_tr);
    fprintf(stderr, "dbg2       espn:              %d\n", mb_segytraceheader_ptr->espn);
    fprintf(stderr, "dbg2       rp_num:            %d\n", mb_segytraceheader_ptr->rp_num);
    fprintf(stderr, "dbg2       rp_tr:             %d\n", mb_segytraceheader_ptr->rp_tr);
    fprintf(stderr, "dbg2       trc_id:            %d\n", mb_segytraceheader_ptr->trc_id);
    fprintf(stderr, "dbg2       num_vstk:          %d\n", mb_segytraceheader_ptr->num_vstk);
    fprintf(stderr, "dbg2       cdp_fold:          %d\n", mb_segytraceheader_ptr->cdp_fold);
    fprintf(stderr, "dbg2       use:               %d\n", mb_segytraceheader_ptr->use);
    fprintf(stderr, "dbg2       range:             %d\n", mb_segytraceheader_ptr->range);
    fprintf(stderr, "dbg2       grp_elev:          %d\n", mb_segytraceheader_ptr->grp_elev);
    fprintf(stderr, "dbg2       src_elev:          %d\n", mb_segytraceheader_ptr->src_elev);
    fprintf(stderr, "dbg2       src_depth:         %d\n", mb_segytraceheader_ptr->src_depth);
    fprintf(stderr, "dbg2       grp_datum:         %d\n", mb_segytraceheader_ptr->grp_datum);
    fprintf(stderr, "dbg2       src_datum:         %d\n", mb_segytraceheader_ptr->src_datum);
    fprintf(stderr, "dbg2       src_wbd:           %d\n", mb_segytraceheader_ptr->src_wbd);
    fprintf(stderr, "dbg2       grp_wbd:           %d\n", mb_segytraceheader_ptr->grp_wbd);
    fprintf(stderr, "dbg2       elev_scalar:       %d\n", mb_segytraceheader_ptr->elev_scalar);
    fprintf(stderr, "dbg2       coord_scalar:      %d\n", mb_segytraceheader_ptr->coord_scalar);
    fprintf(stderr, "dbg2       src_long:          %d\n", mb_segytraceheader_ptr->src_long);
    fprintf(stderr, "dbg2       src_lat:           %d\n", mb_segytraceheader_ptr->src_lat);
    fprintf(stderr, "dbg2       grp_long:          %d\n", mb_segytraceheader_ptr->grp_long);
    fprintf(stderr, "dbg2       grp_lat:           %d\n", mb_segytraceheader_ptr->grp_lat);
    fprintf(stderr, "dbg2       coord_units:       %d\n", mb_segytraceheader_ptr->coord_units);
    fprintf(stderr, "dbg2       wvel:              %d\n", mb_segytraceheader_ptr->wvel);
    fprintf(stderr, "dbg2       sbvel:             %d\n", mb_segytraceheader_ptr->sbvel);
    fprintf(stderr, "dbg2       src_up_vel:        %d\n", mb_segytraceheader_ptr->src_up_vel);
    fprintf(stderr, "dbg2       grp_up_vel:        %d\n", mb_segytraceheader_ptr->grp_up_vel);
    fprintf(stderr, "dbg2       src_static:        %d\n", mb_segytraceheader_ptr->src_static);
    fprintf(stderr, "dbg2       grp_static:        %d\n", mb_segytraceheader_ptr->grp_static);
    fprintf(stderr, "dbg2       tot_static:        %d\n", mb_segytraceheader_ptr->tot_static);
    fprintf(stderr, "dbg2       laga:              %d\n", mb_segytraceheader_ptr->laga);
    fprintf(stderr, "dbg2       delay_mils:        %d\n", mb_segytraceheader_ptr->delay_mils);
    fprintf(stderr, "dbg2       smute_mils:        %d\n", mb_segytraceheader_ptr->smute_mils);
    fprintf(stderr, "dbg2       emute_mils:        %d\n", mb_segytraceheader_ptr->emute_mils);
    fprintf(stderr, "dbg2       nsamps:            %d\n", mb_segytraceheader_ptr->nsamps);
    fprintf(stderr, "dbg2       si_micros:         %d\n", mb_segytraceheader_ptr->si_micros);
    for (int i = 0; i < 19; i++)
      fprintf(stderr, "dbg2       other_1[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_1[i]);
    fprintf(stderr, "dbg2       year:              %d\n", mb_segytraceheader_ptr->year);
    fprintf(stderr, "dbg2       day_of_yr:         %d\n", mb_segytraceheader_ptr->day_of_yr);
    fprintf(stderr, "dbg2       hour:              %d\n", mb_segytraceheader_ptr->hour);
    fprintf(stderr, "dbg2       min:               %d\n", mb_segytraceheader_ptr->min);
    fprintf(stderr, "dbg2       sec:               %d\n", mb_segytraceheader_ptr->sec);
    fprintf(stderr, "dbg2       mils:              %d\n", mb_segytraceheader_ptr->mils);
    fprintf(stderr, "dbg2       tr_weight:         %d\n", mb_segytraceheader_ptr->tr_weight);
    for (int i = 0; i < 5; i++)
      fprintf(stderr, "dbg2       other_2[%2d]:       %d\n", i, mb_segytraceheader_ptr->other_2[i]);
    fprintf(stderr, "dbg2       delay:             %f\n", mb_segytraceheader_ptr->delay);
    fprintf(stderr, "dbg2       smute_sec:         %f\n", mb_segytraceheader_ptr->smute_sec);
    fprintf(stderr, "dbg2       emute_sec:         %f\n", mb_segytraceheader_ptr->emute_sec);
    fprintf(stderr, "dbg2       si_secs:           %f\n", mb_segytraceheader_ptr->si_secs);
    fprintf(stderr, "dbg2       wbt_secs:          %f\n", mb_segytraceheader_ptr->wbt_secs);
    fprintf(stderr, "dbg2       end_of_rp:         %d\n", mb_segytraceheader_ptr->end_of_rp);
    fprintf(stderr, "dbg2       dummy1:            %f\n", mb_segytraceheader_ptr->dummy1);
    fprintf(stderr, "dbg2       dummy2:            %f\n", mb_segytraceheader_ptr->dummy2);
    fprintf(stderr, "dbg2       dummy3:            %f\n", mb_segytraceheader_ptr->dummy3);
    fprintf(stderr, "dbg2       dummy4:            %f\n", mb_segytraceheader_ptr->dummy4);
    fprintf(stderr, "dbg2       soundspeed:        %f\n", mb_segytraceheader_ptr->soundspeed);
    fprintf(stderr, "dbg2       distance:          %f\n", mb_segytraceheader_ptr->distance);
    fprintf(stderr, "dbg2       roll:              %f\n", mb_segytraceheader_ptr->roll);
    fprintf(stderr, "dbg2       pitch:             %f\n", mb_segytraceheader_ptr->pitch);
    fprintf(stderr, "dbg2       heading:           %f\n", mb_segytraceheader_ptr->heading);
    fprintf(stderr, "dbg2       error:             %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:            %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                      double *temperature, double *depth, double *salinity, double *soundspeed, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract ctd data from bluefin environmental SSV record */
  if (*kind == MB_DATA_SSV) {
    s7kr_bluefin *bluefin = &(store->bluefin);
    // s7k_header *header = &(bluefin->header);

    *nctd = 0;
    for (int i = 0; i < bluefin->number_frames; i++) {
      s7k_bluefin_environmental *environmental = &(bluefin->environmental[i]);
      if (environmental->ctd_time > 0.0 && *nctd < MB_CTD_MAX) {
        /* get time_d if needed */
        if (environmental->ctd_time < 10000.0) {
          int time_j[5];
          time_j[0] = environmental->s7kTime.Year;
          time_j[1] = environmental->s7kTime.Day;
          time_j[2] = 60 * environmental->s7kTime.Hours + environmental->s7kTime.Minutes;
          time_j[3] = (int)environmental->s7kTime.Seconds;
          time_j[4] = (int)(1000000 * (environmental->s7kTime.Seconds - time_j[3]));
          int time_i[7];
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &environmental->ctd_time);
        }

        /* get values */
        time_d[*nctd] = environmental->ctd_time;
        conductivity[*nctd] = environmental->conductivity;
        temperature[*nctd] = environmental->temperature;
        depth[*nctd] = environmental->pressure;
        salinity[*nctd] = environmental->salinity;
        soundspeed[*nctd] = environmental->sound_speed;
        (*nctd)++;
      }
    }
  } else if (*kind == MB_DATA_CTD) {
    /* extract ctd data from CTD record */
    s7kr_ctd *ctd = &(store->ctd);
    s7k_header *header = &(ctd->header);

    /* get time */
    int time_j[5];
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    int time_i[7];
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &time_d[0]);

    *nctd = MIN(ctd->n, MB_CTD_MAX);
    for (int i = 0; i < *nctd; i++) {
      time_d[i] = time_d[0];
      if (ctd->sample_rate > 0.0)
        time_d[i] += i * (1.0 / ctd->sample_rate);
      if (ctd->conductivity_flag == 0)
        conductivity[i] = ctd->conductivity_salinity[i];
      else
        salinity[i] = ctd->conductivity_salinity[i];
      temperature[i] = ctd->temperature[i];
      depth[i] = ctd->pressure_depth[i];
      soundspeed[i] = ctd->sound_velocity[i];
    }
  } else {
    /* else failure */
    *nctd = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nctd:          %d\n", *nctd);
    for (int i = 0; i < *nctd; i++) {
      fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
      fprintf(stderr, "dbg2       conductivity:  %f\n", conductivity[i]);
      fprintf(stderr, "dbg2       temperature:   %f\n", temperature[i]);
      fprintf(stderr, "dbg2       depth:         %f\n", depth[i]);
      fprintf(stderr, "dbg2       salinity:      %f\n", salinity[i]);
      fprintf(stderr, "dbg2       soundspeed:    %f\n", soundspeed[i]);
    }
  }
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsamples, double *time_d,
                                   double *sensor1, double *sensor2, double *sensor3, double *sensor4, double *sensor5,
                                   double *sensor6, double *sensor7, double *sensor8, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract ctd data from bluefin environmental SSV record */
  if (*kind == MB_DATA_SSV) {
    s7kr_bluefin *bluefin = &(store->bluefin);
    // s7k_header *header = &(bluefin->header);

    *nsamples = 0;
    for (int i = 0; i < bluefin->number_frames; i++) {
      s7k_bluefin_environmental *environmental = &(bluefin->environmental[i]);
      time_d[*nsamples] = environmental->sensor_time_sec + 0.000000001 * environmental->sensor_time_nsec;
      sensor1[*nsamples] = -5.0 + ((double)environmental->sensor1) / 6553.6;
      sensor2[*nsamples] = -5.0 + ((double)environmental->sensor2) / 6553.6;
      sensor3[*nsamples] = -5.0 + ((double)environmental->sensor3) / 6553.6;
      sensor4[*nsamples] = -5.0 + ((double)environmental->sensor4) / 6553.6;
      sensor5[*nsamples] = -5.0 + ((double)environmental->sensor5) / 6553.6;
      sensor6[*nsamples] = -5.0 + ((double)environmental->sensor6) / 6553.6;
      sensor7[*nsamples] = -5.0 + ((double)environmental->sensor7) / 6553.6;
      sensor8[*nsamples] = -5.0 + ((double)environmental->sensor8) / 6553.6;
      (*nsamples)++;
    }
  }

  /* else failure */
  else {
    *nsamples = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
  }
  if (verbose >= 2 && *error == MB_ERROR_NO_ERROR) {
    fprintf(stderr, "dbg2       nsamples:   %d\n", *nsamples);
    for (int i = 0; i < *nsamples; i++) {
      fprintf(stderr, "dbg2       time_d:        %f\n", time_d[i]);
      fprintf(stderr, "dbg2       sensor1:       %f\n", sensor1[i]);
      fprintf(stderr, "dbg2       sensor2:       %f\n", sensor2[i]);
      fprintf(stderr, "dbg2       sensor3:       %f\n", sensor3[i]);
      fprintf(stderr, "dbg2       sensor4:       %f\n", sensor4[i]);
      fprintf(stderr, "dbg2       sensor5:       %f\n", sensor5[i]);
      fprintf(stderr, "dbg2       sensor6:       %f\n", sensor6[i]);
      fprintf(stderr, "dbg2       sensor7:       %f\n", sensor7[i]);
      fprintf(stderr, "dbg2       sensor8:       %f\n", sensor8[i]);
    }
  }

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {
  char *charptr;
  char *copycharptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  struct mbsys_reson7k_struct *copy = (struct mbsys_reson7k_struct *)copy_ptr;

  /* copy over structures, allocating memory where necessary */

  /* Type of data record */
  copy->kind = store->kind; /* MB-System record ID */
  copy->type = store->type; /* Reson record ID */

  /* MB-System time stamp */
  copy->time_d = store->time_d;
  for (int i = 0; i < 7; i++)
    copy->time_i[i] = store->time_i[i];

  /* Reference point information (record 1000) */
  /*  Note: these offsets should be zero for submersible vehicles */
  copy->reference = store->reference;

  /* Sensor uncalibrated offset position information (record 1001) */
  copy->sensoruncal = store->sensoruncal;

  /* Sensor calibrated offset position information (record 1002) */
  copy->sensorcal = store->sensorcal;

  /* Position (record 1003) */
  copy->position = store->position;

  /* Attitude (record 1004) */
  s7kr_attitude *attitude = &copy->attitude;
  copy->attitude = store->attitude;
  copy->attitude.nalloc = attitude->nalloc;
  copy->attitude.pitch = attitude->pitch;
  copy->attitude.roll = attitude->roll;
  copy->attitude.heading = attitude->heading;
  copy->attitude.heave = attitude->heave;

  int status = MB_SUCCESS;

  if (status == MB_SUCCESS && copy->attitude.nalloc < copy->attitude.n * sizeof(float)) {
    copy->attitude.nalloc = copy->attitude.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.heading), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->attitude.nalloc, (void **)&(copy->attitude.heave), error);
    if (status != MB_SUCCESS) {
      copy->attitude.n = 0;
      copy->attitude.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0; i < copy->attitude.n; i++) {
      copy->attitude.pitch[i] = store->attitude.pitch[i];
      copy->attitude.roll[i] = store->attitude.roll[i];
      copy->attitude.heading[i] = store->attitude.heading[i];
      copy->attitude.heave[i] = store->attitude.heave[i];
    }
  }

  /* Tide (record 1005) */
  copy->tide = store->tide;

  /* Altitude (record 1006) */
  copy->altitude = store->altitude;

  /* Motion over ground (record 1007) */
  s7kr_motion *motion = &copy->motion;
  copy->motion = store->motion;
  copy->motion.nalloc = motion->nalloc;
  copy->motion.x = motion->x;
  copy->motion.y = motion->y;
  copy->motion.z = motion->z;
  copy->motion.xa = motion->xa;
  copy->motion.ya = motion->ya;
  copy->motion.za = motion->za;
  if (status == MB_SUCCESS && copy->motion.nalloc < copy->motion.n * sizeof(float)) {
    copy->motion.nalloc = copy->motion.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.x), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.y), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.z), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.xa), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.ya), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->motion.nalloc, (void **)&(copy->motion.za), error);
    if (status != MB_SUCCESS) {
      copy->motion.n = 0;
      copy->motion.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0; i < copy->motion.n; i++) {
      copy->motion.x[i] = store->motion.x[i];
      copy->motion.y[i] = store->motion.y[i];
      copy->motion.z[i] = store->motion.z[i];
      copy->motion.xa[i] = store->motion.xa[i];
      copy->motion.ya[i] = store->motion.ya[i];
      copy->motion.za[i] = store->motion.za[i];
    }
  }

  /* Depth (record 1008) */
  copy->depth = store->depth;

  /* Sound velocity profile (record 1009) */
  s7kr_svp *svp = &copy->svp;
  copy->svp = store->svp;
  copy->svp.nalloc = svp->nalloc;
  copy->svp.depth = svp->depth;
  copy->svp.sound_velocity = svp->sound_velocity;
  if (status == MB_SUCCESS && copy->svp.nalloc < copy->svp.n * sizeof(float)) {
    copy->svp.nalloc = copy->svp.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->svp.nalloc, (void **)&(copy->svp.depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->svp.nalloc, (void **)&(copy->svp.sound_velocity), error);
    if (status != MB_SUCCESS) {
      copy->svp.n = 0;
      copy->svp.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0; i < copy->svp.n; i++) {
      copy->svp.depth[i] = store->svp.depth[i];
      copy->svp.sound_velocity[i] = store->svp.sound_velocity[i];
    }
  }

  /* CTD (record 1010) */
  s7kr_ctd *ctd = &copy->ctd;
  copy->ctd = store->ctd;
  copy->ctd.nalloc = ctd->nalloc;
  copy->ctd.conductivity_salinity = ctd->conductivity_salinity;
  copy->ctd.temperature = ctd->temperature;
  copy->ctd.pressure_depth = ctd->pressure_depth;
  copy->ctd.sound_velocity = ctd->sound_velocity;
  copy->ctd.absorption = ctd->absorption;
  if (status == MB_SUCCESS && copy->ctd.nalloc < copy->ctd.n * sizeof(float)) {
    copy->ctd.nalloc = copy->ctd.n * sizeof(float);
    if (status == MB_SUCCESS)
      status =
          mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.conductivity_salinity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.temperature), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.pressure_depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.sound_velocity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->ctd.nalloc, (void **)&(copy->ctd.absorption), error);
    if (status != MB_SUCCESS) {
      copy->ctd.n = 0;
      copy->ctd.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0; i < copy->ctd.n; i++) {
      copy->ctd.conductivity_salinity[i] = store->ctd.conductivity_salinity[i];
      copy->ctd.temperature[i] = store->ctd.temperature[i];
      copy->ctd.pressure_depth[i] = store->ctd.pressure_depth[i];
      copy->ctd.sound_velocity[i] = store->ctd.sound_velocity[i];
      copy->ctd.absorption[i] = store->ctd.absorption[i];
    }
  }

  /* Geodesy (record 1011) */
  copy->geodesy = store->geodesy;

  /* Edgetech FS-DW low frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsslo = &copy->fsdwsslo;
  copy->fsdwsslo = store->fsdwsslo;
  for (int j = 0; j < 2; j++) {
    copy->fsdwsslo.channel[j].data_alloc = fsdwsslo->channel[j].data_alloc;
    copy->fsdwsslo.channel[j].data = fsdwsslo->channel[j].data;
    if (status == MB_SUCCESS && copy->fsdwsslo.channel[j].data_alloc <
                                    copy->fsdwsslo.channel[j].number_samples * copy->fsdwsslo.channel[j].bytespersample) {
      copy->fsdwsslo.channel[j].data_alloc =
          copy->fsdwsslo.channel[j].number_samples * copy->fsdwsslo.channel[j].bytespersample;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsslo.channel[j].data_alloc,
                             (void **)&(copy->fsdwsslo.channel[j].data), error);
      if (status != MB_SUCCESS) {
        copy->fsdwsslo.channel[j].data_alloc = 0;
        copy->fsdwsslo.channel[j].number_samples = 0;
      }
    }
    if (status == MB_SUCCESS) {
      for (unsigned int i = 0; i < copy->fsdwsslo.channel[j].data_alloc; i++) {
        copy->fsdwsslo.channel[j].data[i] = store->fsdwsslo.channel[j].data[i];
      }
    }
  }

  /* Edgetech FS-DW high frequency sidescan (record 3000) */
  s7kr_fsdwss *fsdwsshi = &copy->fsdwsshi;
  copy->fsdwsshi = store->fsdwsshi;
  for (int j = 0; j < 2; j++) {
    copy->fsdwsshi.channel[j].data_alloc = fsdwsshi->channel[j].data_alloc;
    copy->fsdwsshi.channel[j].data = fsdwsshi->channel[j].data;
    if (status == MB_SUCCESS && copy->fsdwsshi.channel[j].data_alloc <
                                    copy->fsdwsshi.channel[j].number_samples * copy->fsdwsshi.channel[j].bytespersample) {
      copy->fsdwsshi.channel[j].data_alloc =
          copy->fsdwsshi.channel[j].number_samples * copy->fsdwsshi.channel[j].bytespersample;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsshi.channel[j].data_alloc,
                             (void **)&(copy->fsdwsshi.channel[j].data), error);
      if (status != MB_SUCCESS) {
        copy->fsdwsshi.channel[j].data_alloc = 0;
        copy->fsdwsshi.channel[j].number_samples = 0;
      }
    }
    if (status == MB_SUCCESS) {
      for (unsigned int i = 0; i < copy->fsdwsshi.channel[j].data_alloc; i++) {
        copy->fsdwsshi.channel[j].data[i] = store->fsdwsshi.channel[j].data[i];
      }
    }
  }

  /* Edgetech FS-DW subbottom (record 3001) */
  s7kr_fsdwsb *fsdwsb = &copy->fsdwsb;
  copy->fsdwsb = store->fsdwsb;
  copy->fsdwsb.channel.data_alloc = fsdwsb->channel.data_alloc;
  copy->fsdwsb.channel.data = fsdwsb->channel.data;
  if (status == MB_SUCCESS &&
      copy->fsdwsb.channel.data_alloc < copy->fsdwsb.channel.number_samples * copy->fsdwsb.channel.bytespersample) {
    copy->fsdwsb.channel.data_alloc = copy->fsdwsb.channel.number_samples * copy->fsdwsb.channel.bytespersample;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, store->fsdwsb.channel.data_alloc,
                           (void **)&(copy->fsdwsb.channel.data), error);
    if (status != MB_SUCCESS) {
      copy->fsdwsb.channel.data_alloc = 0;
      copy->fsdwsb.channel.number_samples = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0; i < copy->fsdwsb.channel.data_alloc; i++) {
      copy->fsdwsb.channel.data[i] = store->fsdwsb.channel.data[i];
    }
  }

  /* Bluefin Environmental Data Frame (can be included in record 3100) */
  copy->bluefin = store->bluefin;

  /* Reson 7k volatile sonar settings (record 7000) */
  copy->volatilesettings = store->volatilesettings;

  /* Reson 7k configuration (record 7001) */
  s7kr_configuration *configuration = &copy->configuration;
  copy->configuration = store->configuration;
  for (int j = 0; j < MBSYS_RESON7K_MAX_DEVICE; j++) {
    copy->configuration.device[j].info_alloc = configuration->device[j].info_alloc;
    copy->configuration.device[j].info = configuration->device[j].info;
    if (status == MB_SUCCESS && copy->configuration.device[j].info_alloc < copy->configuration.device[j].info_length) {
      copy->configuration.device[j].info_alloc = copy->configuration.device[j].info_length;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, copy->configuration.device[j].info_alloc,
                             (void **)&(copy->configuration.device[j].info), error);
      if (status != MB_SUCCESS) {
        copy->configuration.device[j].info_alloc = 0;
        copy->configuration.device[j].info_length = 0;
      }
    }
    if (status == MB_SUCCESS) {
      for (unsigned int i = 0; i < copy->configuration.device[j].info_length; i++) {
        copy->configuration.device[j].info[i] = store->configuration.device[j].info[i];
      }
    }
  }

  /* Reson 7k beam geometry (record 7004) */
  copy->beamgeometry = store->beamgeometry;

  /* Reson 7k calibration (record 7005) */
  copy->calibration = store->calibration;

  /* Reson 7k bathymetry (record 7006) */
  copy->bathymetry = store->bathymetry;

  /* Reson 7k backscatter imagery data (record 7007) */
  s7kr_backscatter *backscatter = &copy->backscatter;
  copy->backscatter = store->backscatter;
  copy->backscatter.nalloc = backscatter->nalloc;
  copy->backscatter.port_data = backscatter->port_data;
  copy->backscatter.stbd_data = backscatter->stbd_data;
  if (status == MB_SUCCESS && copy->backscatter.nalloc < copy->backscatter.number_samples * copy->backscatter.sample_size) {
    copy->backscatter.nalloc = copy->backscatter.number_samples * copy->backscatter.sample_size;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->backscatter.nalloc, (void **)&(copy->backscatter.port_data),
                           error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->backscatter.nalloc, (void **)&(copy->backscatter.stbd_data),
                           error);
    if (status != MB_SUCCESS) {
      copy->backscatter.nalloc = 0;
      copy->backscatter.number_samples = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0; i < copy->backscatter.number_samples; i++) {
      copy->backscatter.port_data[i] = store->backscatter.port_data[i];
      copy->backscatter.stbd_data[i] = store->backscatter.stbd_data[i];
    }
  }

  /* Reson 7k beam data (record 7008) */
  s7kr_beam *beam = &copy->beam;
  copy->beam = store->beam;
  for (int i = 0; i < MBSYS_RESON7K_MAX_RECEIVERS; i++) {
    copy->beam.snippets[i].nalloc_amp = beam->snippets[i].nalloc_amp;
    copy->beam.snippets[i].nalloc_phase = beam->snippets[i].nalloc_phase;
    copy->beam.snippets[i].amplitude = beam->snippets[i].amplitude;
    copy->beam.snippets[i].phase = beam->snippets[i].phase;
    if (status == MB_SUCCESS && (copy->beam.snippets[i].nalloc_amp < store->beam.snippets[i].nalloc_amp ||
                                 copy->beam.snippets[i].nalloc_phase < store->beam.snippets[i].nalloc_phase)) {
      copy->beam.snippets[i].nalloc_amp = store->beam.snippets[i].nalloc_amp;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, copy->beam.snippets[i].nalloc_amp,
                             (void **)&(copy->beam.snippets[i].amplitude), error);
      copy->beam.snippets[i].nalloc_phase = store->beam.snippets[i].nalloc_phase;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, copy->beam.snippets[i].nalloc_phase,
                             (void **)&(copy->beam.snippets[i].phase), error);
      if (status != MB_SUCCESS) {
        copy->beam.snippets[i].nalloc_amp = 0;
        copy->beam.snippets[i].nalloc_phase = 0;
        copy->beam.snippets[i].end_sample = 0;
        copy->beam.snippets[i].begin_sample = 0;
      }
    }
    if (status == MB_SUCCESS) {
      copycharptr = (char *)(copy->beam.snippets[i].amplitude);
      charptr = (char *)(store->beam.snippets[i].amplitude);
      for (unsigned int j = 0; j < copy->beam.snippets[i].nalloc_amp; j++)
        copycharptr[j] = charptr[j];
      copycharptr = (char *)(copy->beam.snippets[i].phase);
      charptr = (char *)(store->beam.snippets[i].phase);
      for (unsigned int j = 0; j < copy->beam.snippets[i].nalloc_phase; j++)
        copycharptr[j] = charptr[j];
    }
  }

  /* Reson 7k vertical depth (record 7009) */
  copy->verticaldepth = store->verticaldepth;

  /* Reson 7k tvg data (record 7010) */
  s7kr_tvg *tvg = &copy->tvg;
  copy->tvg = store->tvg;
  copy->tvg.nalloc = tvg->nalloc;
  copy->tvg.tvg = tvg->tvg;
  unsigned int nalloc = tvg->n * sizeof(float);
  if (status == MB_SUCCESS && copy->tvg.nalloc < nalloc) {
    copy->tvg.nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->tvg.nalloc, (void **)&(copy->tvg.tvg), error);
    if (status != MB_SUCCESS) {
      copy->tvg.nalloc = 0;
      copy->tvg.n = 0;
    }
  }
  if (status == MB_SUCCESS) {
    copycharptr = (char *)(copy->tvg.tvg);
    charptr = (char *)(store->tvg.tvg);
    for (unsigned int j = 0; j < nalloc; j++)
      copycharptr[j] = charptr[j];
  }

  /* Reson 7k image data (record 7011) */
  s7kr_image *image = &copy->image;
  copy->image = store->image;
  copy->image.nalloc = image->nalloc;
  copy->image.image = image->image;
  nalloc = image->width * image->height * image->color_depth;
  if (status == MB_SUCCESS && copy->image.nalloc < nalloc) {
    copy->image.nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->image.nalloc, (void **)&(copy->image.image), error);
    if (status != MB_SUCCESS) {
      copy->image.nalloc = 0;
      copy->image.width = 0;
      copy->image.height = 0;
      copy->image.color_depth = 0;
    }
  }
  if (status == MB_SUCCESS) {
    copycharptr = (char *)(copy->image.image);
    charptr = (char *)(store->image.image);
    for (unsigned int j = 0; j < nalloc; j++)
      copycharptr[j] = charptr[j];
  }

  /* Reson 7k system event (record 7051) */
  s7kr_systemeventmessage *systemeventmessage = &copy->systemeventmessage;
  copy->systemeventmessage = store->systemeventmessage;
  copy->systemeventmessage.message_alloc = systemeventmessage->message_alloc;
  copy->systemeventmessage.message = systemeventmessage->message;
  if (status == MB_SUCCESS && copy->systemeventmessage.message_alloc < copy->systemeventmessage.message_length) {
    copy->systemeventmessage.message_alloc = copy->systemeventmessage.message_length;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->systemeventmessage.message_alloc,
                           (void **)&(copy->systemeventmessage.message), error);
    if (status != MB_SUCCESS) {
      copy->systemeventmessage.event_id = 0;
      copy->systemeventmessage.message_alloc = 0;
      copy->systemeventmessage.message_length = 0;
      copy->systemeventmessage.event_identifier = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0; i < copy->systemeventmessage.message_length; i++) {
      copy->systemeventmessage.message[i] = store->systemeventmessage.message[i];
    }
  }

  /* Reson 7k file header (record 7200) */
  copy->fileheader = store->fileheader;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k_makess_source(int verbose, void *mbio_ptr, void *store_ptr,
                        int source, int pixel_size_set, double *pixel_size,
                        int swath_width_set, double *swath_width, int pixel_int,
                        int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:        %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:       %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       source:          %d\n", source);
    fprintf(stderr, "dbg2       pixel_size_set:  %d\n", pixel_size_set);
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width_set: %d\n", swath_width_set);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       pixel_int:       %d\n", pixel_int);
  }

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* if necessary pick a source for the backscatter */
  if (store->kind == MB_DATA_DATA && source == R7KRECID_None) {
    if (store->read_calibratedsnippet)
      source = R7KRECID_7kCalibratedSnippetData;
    else if (store->read_v2snippet)
      source = R7KRECID_7kV2SnippetData;
    else if (store->read_beam)
      source = R7KRECID_7kBeamData;
    else if (store->read_backscatter)
      source = R7KRECID_7kBackscatterImageData;
  }

  /* calculate sidescan from the desired source data if it is available */
  if (store->kind == MB_DATA_DATA && ((source == R7KRECID_7kV2SnippetData && store->read_v2snippet) ||
                                      (source == R7KRECID_7kCalibratedSnippetData && store->read_calibratedsnippet) ||
                                      (source == R7KRECID_7kBeamData && store->read_beam) ||
                                      (source == R7KRECID_7kBackscatterImageData && store->read_backscatter))) {
    int nbathsort;
    double bathsort[MBSYS_RESON7K_MAX_BEAMS];
    char beamflag[MBSYS_RESON7K_MAX_BEAMS];
    double ss_spacing;
    double ss_spacing_use;
    double soundspeed;
    int iminxtrack;
    double minxtrack;
    double maxxtrack;
    int nrangetable;
    double rangetable[MBSYS_RESON7K_MAX_BEAMS];
    double acrosstracktable[MBSYS_RESON7K_MAX_BEAMS];
    double acrosstracktablemin;
    double alongtracktable[MBSYS_RESON7K_MAX_BEAMS];
    int irangenadir;
    // int irange;
    int found;
    int pixel_int_use;
    unsigned int nsample_use;
    unsigned int sample_start;
    unsigned int sample_detect;
    unsigned int sample_end;
    double angle;
    // double altitude;
    double xtrack;
    double xtrackss;
    double ltrackss;
    double factor;
    double range;
    double beam_foot;
    double beamwidth;
    double sint;
    // int first, last;

    s7kr_reference *reference = (s7kr_reference *)&store->reference;
    s7kr_volatilesettings *volatilesettings = (s7kr_volatilesettings *)&store->volatilesettings;
    s7kr_beamgeometry *beamgeometry = (s7kr_beamgeometry *)&store->beamgeometry;
    s7kr_bathymetry *bathymetry = (s7kr_bathymetry *)&store->bathymetry;
    s7kr_backscatter *backscatter = (s7kr_backscatter *)&store->backscatter;
    s7kr_v2snippet *v2snippet = (s7kr_v2snippet *)&store->v2snippet;
    s7kr_calibratedsnippet *calibratedsnippet = (s7kr_calibratedsnippet *)&store->calibratedsnippet;
    s7kr_beam *beam = (s7kr_beam *)&store->beam;
    s7kr_processedsidescan *processedsidescan = (s7kr_processedsidescan *)&store->processedsidescan;
    s7kr_bluefin *bluefin = (s7kr_bluefin *)&store->bluefin;
    s7kr_soundvelocity *soundvelocity = (s7kr_soundvelocity *)&store->soundvelocity;

    /* get beamflags - only use snippets from good beams */
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      /* beamflagging scheme:
          Reson quality flags use bits 0-3
              bit 0: brightness test
              bit 1: colinearity test
              bit 2: amplitude pick
              bit 3: phase pick
          Early MB scheme (through 2007) - use bits 0-5
              null: 0
              flagged: 2
              good: 15
              amplitude: +16
              phase: +32
          Current MB scheme (>= 2008) - use bits 4-7
              - bits 0-3 left in original values
              - beam valid if bit 4 or 5 are set
              - beam flagged if bit 6 or 7 set
              bit 4: on = amplitude
              bit 5: on = phase
              bit 6: on = auto flag
              bit 7: on = manual flag */
      if (bathymetry->quality[i] == 0) {
        beamflag[i] = MB_FLAG_NULL;
      } else if (bathymetry->quality[i] & 64) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
      } else if (bathymetry->quality[i] & 128) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
      } else if (bathymetry->quality[i] & 240) {
        beamflag[i] = MB_FLAG_NONE;
      } else if ((bathymetry->quality[i] & 3) == 3) {
        beamflag[i] = MB_FLAG_NONE;
      } else if ((bathymetry->quality[i] & 15) == 0) {
        beamflag[i] = MB_FLAG_NULL;
      } else if ((bathymetry->quality[i] & 3) == 0) {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_FILTER;
      } else {
        beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MANUAL;
      }
    }

    /* get beam angle size */
    beamwidth = 2.0 * RTD * volatilesettings->receive_width;

    /* get soundspeed */
    if (volatilesettings->sound_velocity > 0.0)
      soundspeed = volatilesettings->sound_velocity;
    else if (soundvelocity->soundvelocity > 0.0)
      soundspeed = soundvelocity->soundvelocity;
    else if (bluefin->environmental[0].sound_speed > 0.0)
      soundspeed = bluefin->environmental[0].sound_speed;
    else
      soundspeed = 1500.0;

    /* get raw pixel size */
    ss_spacing = 0.5 * soundspeed / volatilesettings->sample_rate;

    /* get median depth relative to the sonar and check for min max xtrack */
    nbathsort = 0;
    minxtrack = 0.0;
    maxxtrack = 0.0;
    iminxtrack = bathymetry->number_beams / 2;
    found = false;
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      if (mb_beam_ok(beamflag[i])) {
        bathsort[nbathsort] = bathymetry->depth[i] + bathymetry->vehicle_height;
        nbathsort++;

        if (!found || fabs(bathymetry->acrosstrack[i]) < minxtrack) {
          minxtrack = fabs(bathymetry->acrosstrack[i]);
          iminxtrack = i;
          found = true;
        }

        maxxtrack = MAX(fabs(bathymetry->acrosstrack[i]), maxxtrack);
      }
    }

    /* set number of pixels */
    const int nss = MIN(4 * bathymetry->number_beams, MBSYS_RESON7K_MAX_PIXELS);

    /* get sidescan pixel size */
    if (!swath_width_set && bathymetry->number_beams > 0) {
      (*swath_width) = MAX(fabs(RTD * beamgeometry->angle_acrosstrack[0]),
                           fabs(RTD * beamgeometry->angle_acrosstrack[bathymetry->number_beams - 1]));
    }
    if (!pixel_size_set && nbathsort > 0) {
      /* calculate pixel size implied using swath width and nadir altitude */
      qsort((void *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
      double pixel_size_calc = 2.1 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / nss;

      /* use pixel size based on actual swath width if that is larger than the first value */
      pixel_size_calc = MAX(pixel_size_calc, 2.1 * maxxtrack / nss);

      /* make sure the pixel size is at least equivalent to a 0.1 degree nadir beamwidth */
      pixel_size_calc = MAX(pixel_size_calc, bathsort[nbathsort / 2] * sin(DTR * 0.1));

      /* if the pixel size appears to be changing in size, moderate the change */
      if ((*pixel_size) <= 0.0)
        (*pixel_size) = pixel_size_calc;
      else if (0.95 * (*pixel_size) > pixel_size_calc)
        (*pixel_size) = 0.95 * (*pixel_size);
      else if (1.05 * (*pixel_size) < pixel_size_calc)
        (*pixel_size) = 1.05 * (*pixel_size);
      else
        (*pixel_size) = pixel_size_calc;
    }

    /* get pixel interpolation */
    pixel_int_use = pixel_int + 1;

    int ss_cnt[MBSYS_RESON7K_MAX_PIXELS];
    double ss[MBSYS_RESON7K_MAX_PIXELS];
    double ssacrosstrack[MBSYS_RESON7K_MAX_PIXELS];
    double ssalongtrack[MBSYS_RESON7K_MAX_PIXELS];

    /* zero the sidescan */
    for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      ss[i] = 0.0;
      ssacrosstrack[i] = 0.0;
      ssalongtrack[i] = 0.0;
      ss_cnt[i] = 0;
    }
    for (int i = 0; i < nss; i++) {
      ssacrosstrack[i] = (*pixel_size) * (double)(i - (nss / 2));
    }

    /* loop over raw backscatter or sidescan from the desired source,
     *   putting each raw sample into the binning arrays */

    mb_u_char *data_uchar;
    unsigned short *data_ushort;
    unsigned int *data_uint;

    /* use calibrated snippet data
       error_flag = 0 is calibrated snippet data
       error_flag = 1 is uncalibrated snippet data
       error_flag > 1 indicates a problem */
    if (source == R7KRECID_7kCalibratedSnippetData && calibratedsnippet->error_flag < 3) {
      for (int i = 0; i < calibratedsnippet->number_beams; i++) {
        s7kr_calibratedsnippettimeseries *calibratedsnippettimeseries =
            (s7kr_calibratedsnippettimeseries *)&(calibratedsnippet->calibratedsnippettimeseries[i]);
        const int ibeam = calibratedsnippettimeseries->beam_number;

        /* only use snippets from non-null and unflagged beams */
        /* note: errors have been observed in data produced by a Reson
            simulator in which the detect_sample was
            was outside the range of begin_sample to end_sample
            - the current code effectively ignores this case because
            sample_end < sample_start, so no samples are processed. */
        if (mb_beam_ok(beamflag[ibeam])) {
          // const int nsample = calibratedsnippettimeseries->end_sample - calibratedsnippettimeseries->begin_sample + 1;
          // const double altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
          xtrack = bathymetry->acrosstrack[ibeam];
          range = 0.5 * soundspeed * bathymetry->range[ibeam];
          angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
          beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
          sint = fabs(sin(DTR * angle));
          nsample_use = beam_foot / ss_spacing;
          if (sint < nsample_use * ss_spacing / beam_foot)
            ss_spacing_use = beam_foot / nsample_use;
          else
            ss_spacing_use = ss_spacing / sint;
          sample_start = MAX((calibratedsnippettimeseries->detect_sample - (nsample_use / 2)),
                             calibratedsnippettimeseries->begin_sample);
          sample_end = MIN((calibratedsnippettimeseries->detect_sample + (nsample_use / 2)),
                           calibratedsnippettimeseries->end_sample);
          for (unsigned int k = sample_start; k <= sample_end; k++) {
            if (xtrack < 0.0)
              xtrackss = xtrack - ss_spacing_use * (k - (int)calibratedsnippettimeseries->detect_sample);
            else
              xtrackss = xtrack + ss_spacing_use * (k - (int)calibratedsnippettimeseries->detect_sample);
            int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
            kk = MIN(MAX(0, kk), nss - 1);
            ss[kk] +=
                (double)calibratedsnippettimeseries->amplitude[k - (int)calibratedsnippettimeseries->begin_sample];
            ssalongtrack[kk] += bathymetry->alongtrack[i];
            ss_cnt[kk]++;
          }
        }
      }
    }

    /* use v2 snippet data */
    else if (source == R7KRECID_7kV2SnippetData && !v2snippet->error_flag) {
      for (int i = 0; i < v2snippet->number_beams; i++) {
        s7kr_v2snippettimeseries *snippettimeseries = (s7kr_v2snippettimeseries *)&(v2snippet->snippettimeseries[i]);
        const int ibeam = snippettimeseries->beam_number;

        /* only use snippets from non-null and unflagged beams */
        /* note: errors have been observed in data produced by a Reson
            simulator in which the detect_sample was
            was outside the range of begin_sample to end_sample
            - the current code effectively ignores this case because
            sample_end < sample_start, so no samples are processed. */
        if (mb_beam_ok(beamflag[ibeam])) {
          // const int nsample = snippettimeseries->end_sample - snippettimeseries->begin_sample + 1;
          // const double altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
          xtrack = bathymetry->acrosstrack[ibeam];
          range = 0.5 * soundspeed * bathymetry->range[ibeam];
          angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
          beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
          sint = fabs(sin(DTR * angle));
          nsample_use = beam_foot / ss_spacing;
          if (sint < nsample_use * ss_spacing / beam_foot)
            ss_spacing_use = beam_foot / nsample_use;
          else
            ss_spacing_use = ss_spacing / sint;
          sample_start =
              MAX((snippettimeseries->detect_sample - (nsample_use / 2)), snippettimeseries->begin_sample);
          sample_end =
              MIN((snippettimeseries->detect_sample + (nsample_use / 2)), snippettimeseries->end_sample);
          for (unsigned int k = sample_start; k <= sample_end; k++) {
            if (xtrack < 0.0)
              xtrackss = xtrack - ss_spacing_use * (k - (int)snippettimeseries->detect_sample);
            else
              xtrackss = xtrack + ss_spacing_use * (k - (int)snippettimeseries->detect_sample);
            int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
            kk = MIN(MAX(0, kk), nss - 1);
            ss[kk] += (double)snippettimeseries->amplitude[k - (int)snippettimeseries->begin_sample];
            ssalongtrack[kk] += bathymetry->alongtrack[i];
            ss_cnt[kk]++;
          }
        }
      }
    }

    /* use old snippet data */
    else if (source == R7KRECID_7kBeamData) {
      for (int i = 0; i < beam->number_beams; i++) {
        s7kr_snippet *snippet = (s7kr_snippet *)&(beam->snippets[i]);
        const int ibeam = snippet->beam_number;

        /* only use snippets from non-null and unflagged beams */
        if (mb_beam_ok(beamflag[ibeam])) {
          // const int nsample = snippet->end_sample - snippet->begin_sample + 1;
          // const double altitude = bathymetry->depth[ibeam] + bathymetry->vehicle_height;
          xtrack = bathymetry->acrosstrack[ibeam];
          range = 0.5 * soundspeed * bathymetry->range[ibeam];
          angle = RTD * beamgeometry->angle_acrosstrack[ibeam];
          beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
          sint = fabs(sin(DTR * angle));
          nsample_use = beam_foot / ss_spacing;
          if (sint < nsample_use * ss_spacing / beam_foot)
            ss_spacing_use = beam_foot / nsample_use;
          else
            ss_spacing_use = ss_spacing / sint;
          sample_detect = volatilesettings->sample_rate * bathymetry->range[ibeam];
          sample_start = MAX(sample_detect - (nsample_use / 2), snippet->begin_sample);
          sample_end = MIN(sample_detect + (nsample_use / 2), snippet->end_sample);
          if ((beam->sample_type & 15) == 3)
            data_uint = (unsigned int *)snippet->amplitude;
          else if ((beam->sample_type & 15) == 2)
            data_ushort = (unsigned short *)snippet->amplitude;
          else
            data_uchar = (mb_u_char *)snippet->amplitude;
          for (unsigned int k = sample_start; k <= sample_end; k++) {
            if (xtrack < 0.0)
              xtrackss = xtrack - ss_spacing_use * (k - sample_detect);
            else
              xtrackss = xtrack + ss_spacing_use * (k - sample_detect);
            int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
            kk = MIN(MAX(0, kk), nss - 1);
            if ((beam->sample_type & 15) == 3)
              ss[kk] += (double)data_uint[k - snippet->begin_sample];
            else if ((beam->sample_type & 15) == 2)
              ss[kk] += (double)data_ushort[k - snippet->begin_sample];
            else
              ss[kk] += (double)data_uchar[k - snippet->begin_sample];
            ssalongtrack[kk] += bathymetry->alongtrack[ibeam];
            ss_cnt[kk]++;
          }
        }
      }
    }

    /* use backscatter record - basically a pseudosidescan */
    else if (source == R7KRECID_7kBackscatterImageData) {
      /* get acrosstrack distance versus range table from bathymetry */
      nrangetable = 0;
      irangenadir = 0;
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        if (mb_beam_ok(beamflag[i])) {
          rangetable[nrangetable] = bathymetry->range[i];
          acrosstracktable[nrangetable] = bathymetry->acrosstrack[i];
          alongtracktable[nrangetable] = bathymetry->alongtrack[i];
          if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
            irangenadir = nrangetable;
            acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
          }
          nrangetable++;
        }
      }

      /* lay out port side */
      data_uchar = (mb_u_char *)backscatter->port_data;
      data_ushort = (unsigned short *)backscatter->port_data;
      data_uint = (unsigned int *)backscatter->port_data;
      sample_start = rangetable[irangenadir] * volatilesettings->sample_rate;
      sample_end = MIN(rangetable[0] * volatilesettings->sample_rate, backscatter->number_samples - 1);
      int irange = irangenadir;
      for (unsigned int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)volatilesettings->sample_rate);
        found = false;
        for (int j = irange; j > 0 && !found; j--) {
          if (range >= rangetable[j] && range < rangetable[j - 1]) {
            irange = j;
            found = true;
          }
        }
        factor = (range - rangetable[irange]) / (rangetable[irange - 1] - rangetable[irange]);
        xtrackss = acrosstracktable[irange] + factor * (acrosstracktable[irange - 1] - acrosstracktable[irange]);
        ltrackss = alongtracktable[irange] + factor * (alongtracktable[irange - 1] - alongtracktable[irange]);
        const int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
        if (kk >= 0 && kk < nss) {
          if (backscatter->sample_size == 1)
            ss[kk] += (double)data_uchar[i];
          else if (backscatter->sample_size == 2)
            ss[kk] += (double)data_ushort[i];
          else
            ss[kk] += (double)data_uint[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }

      /* lay out starboard side */
      data_uchar = (mb_u_char *)backscatter->stbd_data;
      data_ushort = (unsigned short *)backscatter->stbd_data;
      data_uint = (unsigned int *)backscatter->stbd_data;
      sample_start = rangetable[irangenadir] * volatilesettings->sample_rate;
      sample_end = MIN(rangetable[nrangetable - 1] * volatilesettings->sample_rate, backscatter->number_samples - 1);
      irange = irangenadir;
      for (unsigned int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)volatilesettings->sample_rate);
        found = false;
        for (int j = irange; j < nrangetable - 1 && !found; j++) {
          if (range >= rangetable[j] && range < rangetable[j + 1]) {
            irange = j;
            found = true;
          }
        }
        factor = (range - rangetable[irange]) / (rangetable[irange + 1] - rangetable[irange]);
        xtrackss = acrosstracktable[irange] + factor * (acrosstracktable[irange + 1] - acrosstracktable[irange]);
        ltrackss = alongtracktable[irange] + factor * (alongtracktable[irange + 1] - alongtracktable[irange]);
        const int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
        if (kk >= 0 && kk < nss) {
          if (backscatter->sample_size == 1)
            ss[kk] += (double)data_uchar[i];
          else if (backscatter->sample_size == 2)
            ss[kk] += (double)data_ushort[i];
          else
            ss[kk] += (double)data_uint[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }
    }

    /* average the sidescan */
    int first = nss;
    int last = -1;
    for (int k = 0; k < nss; k++) {
      if (ss_cnt[k] > 0) {
        ss[k] /= ss_cnt[k];
        ssalongtrack[k] /= ss_cnt[k];
        first = MIN(first, k);
        last = k;
      }
      else
        ss[k] = MB_SIDESCAN_NULL;
    }

    /* interpolate the sidescan */
    int k1 = first;
    int k2 = first;
    for (int k = first + 1; k < last; k++) {
      if (ss_cnt[k] <= 0) {
        if (k2 <= k) {
          k2 = k + 1;
          while (k2 < last && ss_cnt[k2] <= 0)
            k2++;
        }
        if (k2 - k1 <= pixel_int_use) {
          ss[k] = ss[k1] + (ss[k2] - ss[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
          ssacrosstrack[k] = (k - nss / 2) * (*pixel_size);
          ssalongtrack[k] =
              ssalongtrack[k1] + (ssalongtrack[k2] - ssalongtrack[k1]) * ((double)(k - k1)) / ((double)(k2 - k1));
        }
      }
      else {
        k1 = k;
      }
    }

    /* embed the sidescan into the processed sidescan record */
    store->read_processedsidescan = true;
    processedsidescan->header = bathymetry->header;
    processedsidescan->header.Offset = R7KRECID_ProcessedSidescan;
    processedsidescan->header.Size =
        MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE + R7KHDRSIZE_ProcessedSidescan + nss * 8;
    processedsidescan->header.OffsetToOptionalData = 0;
    processedsidescan->header.OptionalDataIdentifier = 0;
    processedsidescan->header.RecordType = R7KRECID_ProcessedSidescan;
    processedsidescan->serial_number = bathymetry->serial_number;
    processedsidescan->ping_number = bathymetry->ping_number;
    processedsidescan->multi_ping = bathymetry->multi_ping;
    processedsidescan->recordversion = 1;
    processedsidescan->ss_source = source;
    processedsidescan->number_pixels = nss;
    processedsidescan->ss_type = MB_SIDESCAN_LINEAR;
    processedsidescan->pixelwidth = *pixel_size;
    processedsidescan->sensordepth = -bathymetry->vehicle_height + reference->water_z;
    processedsidescan->altitude = bathymetry->depth[iminxtrack] - processedsidescan->sensordepth;
    for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      processedsidescan->sidescan[i] = ss[i];
      processedsidescan->alongtrack[i] = ssalongtrack[i];
    }

    if (verbose >= 2) {
      fprintf(stderr, "\ndbg2  Sidescan regenerated in <%s>\n", __func__);
      fprintf(stderr, "dbg2       pixels_ss:  %d\n", nss);
      for (int i = 0; i < nss; i++)
        fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
                ssacrosstrack[i], ssalongtrack[i]);
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

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
                         int swath_width_set, double *swath_width, int pixel_int, int *error) {
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

  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  // check for a prior sidescan source choice
  int source = store->processedsidescan.ss_source;

  /* if necessary pick a source for the backscatter */
  if (source == R7KRECID_None) {
    if (store->read_calibratedsnippet)
      source = R7KRECID_7kCalibratedSnippetData;
    else if (store->read_v2snippet)
      source = R7KRECID_7kV2SnippetData;
    else if (store->read_beam)
      source = R7KRECID_7kBeamData;
    else if (store->read_backscatter)
      source = R7KRECID_7kBackscatterImageData;
  }

  int status = mbsys_reson7k_makess_source(verbose, mbio_ptr, store_ptr, source,
                        pixel_size_set, pixel_size, swath_width_set, swath_width,
                        pixel_int, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return value:\n");
    fprintf(stderr, "dbg2       pixel_size:      %f\n", *pixel_size);
    fprintf(stderr, "dbg2       swath_width:     %f\n", *swath_width);
    fprintf(stderr, "dbg2       error:           %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:          %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
