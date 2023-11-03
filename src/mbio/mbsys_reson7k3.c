/*--------------------------------------------------------------------
 *    The MB-system:  mbsys_reson7k.c  3.00  1/8/2019
 *
 *    Copyright (c) 2019-2023 by
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
 * mbsys_reson7k3.c contains the MBIO functions for handling data from
 * Teledyne multibeam sonars in the Reson S7K version 3 format.
 * The data formats associated with S7K version 3 format data
 * include:
 *    MBSYS_RESON7K3 formats (code in mbsys_reson7k.c and mbsys_reson7k.h):
 *      MBF_RESON7K3 : MBIO ID 89 - Teledyne S7K Version 3 multibeam data
 *
 * Authors:  C. S. Ferreira & D. W. Caress
 * Date:  March 2019
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
#include "mbsys_reson7k3.h"

/* turn on debug statements here */
//#define MBSYS_RESON7K3_DEBUG 1

static const char debug_str[] = "dbg2  ";
static const char nodebug_str[] = "  ";

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_zero7kheader(int verbose, s7k3_header *header, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
  }

  /* Reson 7k data record header information */
  memset((void *)header, 0 , sizeof(s7k3_header));

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
int mbsys_reson7k3_alloc(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* allocate memory for data structure */
  const int status = mb_mallocd(verbose, __FILE__, __LINE__, sizeof(struct mbsys_reson7k3_struct), (void **)store_ptr, error);

  /* get data structure pointer */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)*store_ptr;

  /* initialize everything */
  memset(*store_ptr, 0, sizeof(struct mbsys_reson7k3_struct));

  /* Type of data record */
  store->kind = MB_DATA_NONE;
  store->type = R7KRECID_None;

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
int mbsys_reson7k3_deall(int verbose, void *mbio_ptr, void **store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)*store_ptr);
  }

  /* get data structure pointer */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)*store_ptr;

  /* Custom Attitude (record 1004) */
  s7k3_CustomAttitude *CustomAttitude = &store->CustomAttitude;
  CustomAttitude->n = 0;
  CustomAttitude->nalloc = 0;

  int status = MB_SUCCESS;

  if (CustomAttitude->pitch != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->pitch), error);
  if (CustomAttitude->roll != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->roll), error);
  if (CustomAttitude->heading != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->heading), error);
  if (CustomAttitude->heave != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->heave), error);
  if (CustomAttitude->pitchrate != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->pitchrate), error);
  if (CustomAttitude->rollrate != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->rollrate), error);
  if (CustomAttitude->headingrate != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->headingrate), error);
  if (CustomAttitude->heaverate != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CustomAttitude->heaverate), error);

  /* Motion over ground (record 1007) */
  s7k3_MotionOverGround *MotionOverGround = &store->MotionOverGround;
  MotionOverGround->n = 0;
  MotionOverGround->nalloc = 0;
  if (MotionOverGround->x != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->x), error);
  if (MotionOverGround->y != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->y), error);
  if (MotionOverGround->z != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->z), error);
  if (MotionOverGround->xa != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->xa), error);
  if (MotionOverGround->ya != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->ya), error);
  if (MotionOverGround->za != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(MotionOverGround->za), error);

  /* Sound velocity profile (record 1009) */
  s7k3_SoundVelocityProfile *SoundVelocityProfile = &store->SoundVelocityProfile;
  SoundVelocityProfile->n = 0;
  SoundVelocityProfile->nalloc = 0;
  if (SoundVelocityProfile->depth != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SoundVelocityProfile->depth), error);
  if (SoundVelocityProfile->sound_velocity != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SoundVelocityProfile->sound_velocity), error);

  /* CTD (record 1010) */
  s7k3_CTD *CTD = &store->CTD;
  CTD->n = 0;
  CTD->nalloc = 0;
  if (CTD->conductivity_salinity != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CTD->conductivity_salinity), error);
  if (CTD->temperature != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CTD->temperature), error);
  if (CTD->pressure_depth != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CTD->pressure_depth), error);
  if (CTD->sound_velocity != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CTD->sound_velocity), error);
  if (CTD->absorption != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(CTD->absorption), error);

  /* Survey Line (record 1014) */
  s7k3_SurveyLine *SurveyLine = &store->SurveyLine;
  SurveyLine->n = 0;
  SurveyLine->nalloc = 0;
  if (SurveyLine->latitude_northing != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SurveyLine->latitude_northing), error);
  if (SurveyLine->longitude_easting != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SurveyLine->longitude_easting), error);

  /* Attitude (record 1016) */
  s7k3_Attitude *Attitude = &store->Attitude;
  Attitude->n = 0;
  Attitude->nalloc = 0;
  if (Attitude->delta_time != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Attitude->delta_time), error);
  if (Attitude->roll != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Attitude->roll), error);
  if (Attitude->pitch != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Attitude->pitch), error);
  if (Attitude->heave != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Attitude->heave), error);
  if (Attitude->heading != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Attitude->heading), error);

  /* Sonar Pipe Environment (record 2004) */
  s7k3_SonarPipeEnvironment *SonarPipeEnvironment = &store->SonarPipeEnvironment;
  SonarPipeEnvironment->n = 0;

  /* Reson 7k Configuration (record 7001) */
  s7k3_Configuration *Configuration = &store->Configuration;
  for (int i = 0; i < MBSYS_RESON7K_MAX_DEVICE; i++) {
    Configuration->device[i].info_length = 0;
    Configuration->device[i].info_alloc = 0;
    if (Configuration->device[i].info != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Configuration->device[i].info), error);
  }

  /* Reson 7k firmware and hardware Configuration (record 7003) */
  s7k3_FirmwareHardwareConfiguration *FirmwareHardwareConfiguration = &store->FirmwareHardwareConfiguration;
  if (FirmwareHardwareConfiguration->info != NULL && FirmwareHardwareConfiguration->info_alloc > 0)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(FirmwareHardwareConfiguration->info), error);
  FirmwareHardwareConfiguration->info_length = 0;
  FirmwareHardwareConfiguration->info_alloc = 0;

  /* Reson 7k Side Scan Data (record 7007) */
  s7k3_SideScan *SideScan = &store->SideScan;
  SideScan->number_samples = 0;
  SideScan->nalloc = 0;
  if (SideScan->port_data != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SideScan->port_data), error);
  if (SideScan->stbd_data != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SideScan->stbd_data), error);

  /* Reson 7k Generic Water Column data (record 7008) */

  /* Reson 7k TVG data (record 7010) */
  s7k3_TVG *TVG = &store->TVG;
  TVG->serial_number = 0;
  TVG->ping_number = 0;
  TVG->multi_ping = 0;
  TVG->n = 0;
  for (int i = 0; i < 8; i++)
    TVG->reserved[i] = 0;
  TVG->nalloc = 0;
  if (TVG->tvg != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(TVG->tvg), error);

  /* Reson 7k image data (record 7011) */
  s7k3_Image *Image = &store->Image;
  Image->width = 0;
  Image->height = 0;
  Image->nalloc = 0;
  if (Image->image != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(Image->image), error);

  /* Reson 7k ping MotionOverGround (record 7012) */
  s7k3_PingMotion *PingMotion = &store->PingMotion;
  if (PingMotion->n > 0 && PingMotion->roll != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(PingMotion->roll), error);
  if (PingMotion->n > 0 && PingMotion->heading != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(PingMotion->heading), error);
  if (PingMotion->n > 0 && PingMotion->heave != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(PingMotion->heave), error);
  PingMotion->n = 0;
  PingMotion->nalloc = 0;

  /* Reson 7k Adaptive Gate (record 7014) */


  /* Reson 7k Beamformed magnitude and phase data (record 7018) */
  s7k3_Beamformed *Beamformed = &store->Beamformed;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7k3_amplitudephase *amplitudephase = &(Beamformed->amplitudephase[i]);
    amplitudephase->number_samples = 0;
    amplitudephase->nalloc = 0;
    if (amplitudephase->amplitude != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->amplitude), error);
    if (amplitudephase->phase != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(amplitudephase->phase), error);
  }

  /* Reson 7k Vernier Processing Data Raw (record 7019) */


  /* Reson 7k BITE (record 7021) */
  s7k3_BITE *BITE = &store->BITE;
  BITE->number_reports = 0;
  BITE->nalloc = 0;
  if (BITE->bitereports != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(BITE->bitereports), error);

  /* Reson 7k Snippet data (record 7028) */
  s7k3_Snippet *Snippet = &store->Snippet;
  Snippet->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7k3_snippetdata *snippetdata = &(Snippet->snippetdata[i]);
    snippetdata->beam_number = 0;
    snippetdata->begin_sample = 0;
    snippetdata->detect_sample = 0;
    snippetdata->end_sample = 0;
    snippetdata->nalloc = 0;
    if (snippetdata->amplitude != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetdata->amplitude), error);
  }

  /* Reson 7k Compressed Beamformed Magnitude Data (Record 7041) */

  /* Reson 7k Compressed Water Column Data (Record 7042) */
  s7k3_CompressedWaterColumn *CompressedWaterColumn = &(store->CompressedWaterColumn);
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7k3_compressedwatercolumndata *compressedwatercolumndata =
        (s7k3_compressedwatercolumndata *)&(CompressedWaterColumn->compressedwatercolumndata[i]);
    if (compressedwatercolumndata->data != NULL) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(compressedwatercolumndata->data), error);
    }
    compressedwatercolumndata->beam_number = 0;
    compressedwatercolumndata->segment_number = 0;
    compressedwatercolumndata->samples = 0;
    compressedwatercolumndata->nalloc = 0;
  }
  CompressedWaterColumn->number_beams = 0;
  CompressedWaterColumn->samples = 0;

  /* Reson 7k Segmented Raw Detection Data (Record 7047) */
  /* Reson 7k Calibrated Beam Data (Record 7048) */
  /* Reson 7k System Events (Record 7050) */

  /* Reson 7k System Event Message (record 7051) */
  s7k3_SystemEventMessage *SystemEventMessage = &store->SystemEventMessage;
  SystemEventMessage->message_length = 0;
  SystemEventMessage->event_identifier = 0;
  SystemEventMessage->message_alloc = 0;
  if (SystemEventMessage->message != NULL)
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(SystemEventMessage->message), error);

  /* Reson 7k RDR Recording Status (Record 7052) */
  /* Reson 7k Calibrated SideScan Data (record 7057) */
  /* Reson 7k File Catalog (part of Record 7300) */

  /* Reson 7k Snippet Backscattering Strength (Record 7058) */
  s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength = &store->SnippetBackscatteringStrength;
  SnippetBackscatteringStrength->number_beams = 0;
  for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
    s7k3_snippetbackscatteringstrengthdata *snippetbackscatteringstrengthdata =
        &(SnippetBackscatteringStrength->snippetbackscatteringstrengthdata[i]);
    snippetbackscatteringstrengthdata->beam_number = 0;
    snippetbackscatteringstrengthdata->begin_sample = 0;
    snippetbackscatteringstrengthdata->bottom_sample = 0;
    snippetbackscatteringstrengthdata->end_sample = 0;
    snippetbackscatteringstrengthdata->nalloc = 0;
    if (snippetbackscatteringstrengthdata->bs != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetbackscatteringstrengthdata->bs), error);
    if (snippetbackscatteringstrengthdata->footprints != NULL)
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(snippetbackscatteringstrengthdata->footprints), error);
  }

  /* Reson 7k MB2 Specific Status (Record 7059) */
  /* Reson 7k file header (record 7200) */

  /* Reson 7k File Catalog (Record 7300) */
  s7k3_FileCatalog *FileCatalog = &store->FileCatalog_read;
  FileCatalog->n = 0;
  FileCatalog->nalloc = 0;
  if (FileCatalog->filecatalogdata != NULL) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(FileCatalog->filecatalogdata), error);
  }
  FileCatalog = &store->FileCatalog_write;
  FileCatalog->n = 0;
  FileCatalog->nalloc = 0;
  if (FileCatalog->filecatalogdata != NULL) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&(FileCatalog->filecatalogdata), error);
  }

  /* Reson 7k Time Message (Record 7400) */
  /* Reson 7k Remote Control Acknowledge (Record 7501) */
  /* Reson 7k Remote Control Not Acknowledge (Record 7502) */
  /* Reson 7k Remote Control Sonar Settings (record 7503) */
  /* Reson 7k Common System Settings (Record 7504) */
  /* Reson 7k SV Filtering (record 7510) */
  /* Reson 7k System Lock Status (record 7511) */
  /* Reson 7k Sound Velocity (record 7610) */
  /* Reson 7k Absorption Loss (record 7611) */
  /* Reson 7k Spreading Loss (record 7612) */

  /* deallocate memory for data structure */
  status = mb_freed(verbose, __FILE__, __LINE__, (void **)store_ptr, error);

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
int mbsys_reson7k3_checkheader(s7k3_header header) {
  return
      (header.Version > 0) && (header.SyncPattern == 0x0000ffff) && (header.Size > MBSYS_RESON7K_RECORDHEADER_SIZE) &&
      (header.s7kTime.Day >= 1) && (header.s7kTime.Day <= 366) && (header.s7kTime.Seconds >= 0.0f) &&
      (header.s7kTime.Seconds < 60.0f) && (header.s7kTime.Hours <= 23) && (header.s7kTime.Minutes <= 59);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_header(int verbose, s7k3_header *header, int *error) {
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
  fprintf(stderr, "%s     SyncPattern:             %d\n", first, header->SyncPattern);
  fprintf(stderr, "%s     Size:                    %d\n", first, header->Size);
  fprintf(stderr, "%s     OptionalDataOffset:      %d\n", first, header->OptionalDataOffset);
  fprintf(stderr, "%s     OptionalDataIdentifier:  %d\n", first, header->OptionalDataIdentifier);
  fprintf(stderr, "%s     s7kTime:                 %4u-%3.3u-%2.2u:%2.2u:%9.6f\n",
            first, header->s7kTime.Year, header->s7kTime.Day, header->s7kTime.Hours,
            header->s7kTime.Minutes, header->s7kTime.Seconds);
  fprintf(stderr, "%s     s7kTime.Year:            %d\n", first, header->s7kTime.Year);
  fprintf(stderr, "%s     s7kTime.Day:             %d\n", first, header->s7kTime.Day);
  fprintf(stderr, "%s     s7kTime.Seconds:         %f\n", first, header->s7kTime.Seconds);
  fprintf(stderr, "%s     s7kTime.Hours:           %d\n", first, header->s7kTime.Hours);
  fprintf(stderr, "%s     s7kTime.Minutes:         %d\n", first, header->s7kTime.Minutes);
  fprintf(stderr, "%s     RecordVersion:           %d\n", first, header->RecordVersion);
  fprintf(stderr, "%s     RecordType:              %d\n", first, header->RecordType);
  fprintf(stderr, "%s     DeviceId:                %d\n", first, header->DeviceId);
  fprintf(stderr, "%s     Reserved:                %d\n", first, header->Reserved);
  fprintf(stderr, "%s     SystemEnumerator:        %d\n", first, header->SystemEnumerator);
  fprintf(stderr, "%s     Reserved2:               %d\n", first, header->Reserved2);
  fprintf(stderr, "%s     Flags:                   %d\n", first, header->Flags);
  fprintf(stderr, "%s     Reserved3:               %d\n", first, header->Reserved3);
  fprintf(stderr, "%s     Reserved4:               %d\n", first, header->Reserved4);
  fprintf(stderr, "%s     FragmentedTotal:         %d\n", first, header->FragmentedTotal);
  fprintf(stderr, "%s     FragmentNumber:          %d\n", first, header->FragmentNumber);

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
int mbsys_reson7k3_print_ReferencePoint(int verbose, s7k3_ReferencePoint *ReferencePoint, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       ReferencePoint:  %p\n", (void *)ReferencePoint);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &ReferencePoint->header, error);

  /* print Reference point information (record 1000) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, ReferencePoint->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, ReferencePoint->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, ReferencePoint->offset_z);
  fprintf(stderr, "%s     water_z:                 %f\n", first, ReferencePoint->water_z);

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
int mbsys_reson7k3_print_UncalibratedSensorOffset(int verbose, s7k3_UncalibratedSensorOffset *UncalibratedSensorOffset, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       UncalibratedSensorOffset:  %p\n", (void *)UncalibratedSensorOffset);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &UncalibratedSensorOffset->header, error);

  /* print Sensor uncalibrated offset Position information (record 1001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, UncalibratedSensorOffset->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, UncalibratedSensorOffset->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, UncalibratedSensorOffset->offset_z);
  fprintf(stderr, "%s     offset_roll:             %f\n", first, UncalibratedSensorOffset->offset_roll);
  fprintf(stderr, "%s     offset_pitch:            %f\n", first, UncalibratedSensorOffset->offset_pitch);
  fprintf(stderr, "%s     offset_yaw:              %f\n", first, UncalibratedSensorOffset->offset_yaw);

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
int mbsys_reson7k3_print_CalibratedSensorOffset(int verbose, s7k3_CalibratedSensorOffset *CalibratedSensorOffset, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       CalibratedSensorOffset:    %p\n", (void *)CalibratedSensorOffset);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CalibratedSensorOffset->header, error);

  /* print Sensor Calibrated offset Position information (record 1002) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     offset_x:                %f\n", first, CalibratedSensorOffset->offset_x);
  fprintf(stderr, "%s     offset_y:                %f\n", first, CalibratedSensorOffset->offset_y);
  fprintf(stderr, "%s     offset_z:                %f\n", first, CalibratedSensorOffset->offset_z);
  fprintf(stderr, "%s     offset_roll:             %f\n", first, CalibratedSensorOffset->offset_roll);
  fprintf(stderr, "%s     offset_pitch:            %f\n", first, CalibratedSensorOffset->offset_pitch);
  fprintf(stderr, "%s     offset_yaw:              %f\n", first, CalibratedSensorOffset->offset_yaw);

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
int mbsys_reson7k3_print_Position(int verbose, s7k3_Position *Position, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Position:     %p\n", (void *)Position);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Position->header, error);

  /* print Position (record 1003) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     datum:                   %d\n", first, Position->datum);
  fprintf(stderr, "%s     latency:                 %f\n", first, Position->latency);
  fprintf(stderr, "%s     latitude:                %f\n", first, Position->latitude_northing);
  fprintf(stderr, "%s     longitude:               %f\n", first, Position->longitude_easting);
  fprintf(stderr, "%s     height:                  %f\n", first, Position->height);
  fprintf(stderr, "%s     type:                    %d\n", first, Position->type);
  fprintf(stderr, "%s     utm_zone:                %d\n", first, Position->utm_zone);
  fprintf(stderr, "%s     quality:                 %d\n", first, Position->quality);
  fprintf(stderr, "%s     method:                  %d\n", first, Position->method);
  fprintf(stderr, "%s     nsat:                  %d\n", first, Position->nsat);

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
int mbsys_reson7k3_print_CustomAttitude(int verbose, s7k3_CustomAttitude *CustomAttitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       CustomAttitude:%p\n", (void *)CustomAttitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CustomAttitude->header, error);

  /* print Custom Attitude (record 1004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     bitfield:                   %d\n", first, CustomAttitude->fieldmask);
  fprintf(stderr, "%s     reserved:                   %d\n", first, CustomAttitude->reserved);
  fprintf(stderr, "%s     n:                          %d\n", first, CustomAttitude->n);
  fprintf(stderr, "%s     frequency:                  %f\n", first, CustomAttitude->frequency);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, CustomAttitude->nalloc);
  for (int i = 0; i < CustomAttitude->n; i++)
    fprintf(stderr, "%s     i:%d pitch:%f roll:%f heading:%f heave:%f\n", first, i, CustomAttitude->pitch[i],
            CustomAttitude->roll[i], CustomAttitude->heading[i], CustomAttitude->heave[i]);
  for (int i = 0; i < CustomAttitude->n; i++)
    fprintf(stderr, "%s     i:%d pitchrate:%f rollrate:%f headingrate:%f heaverate:%f\n", first, i,
            CustomAttitude->pitchrate[i], CustomAttitude->rollrate[i], CustomAttitude->headingrate[i],
            CustomAttitude->heaverate[i]);

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
int mbsys_reson7k3_print_Tide(int verbose, s7k3_Tide *Tide, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Tide:         %p\n", (void *)Tide);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Tide->header, error);

  /* print Tide (record 1005) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     tide:                       %f\n", first, Tide->tide);
  fprintf(stderr, "%s     source:                     %d\n", first, Tide->source);
  fprintf(stderr, "%s     flags:                      %d\n", first, Tide->flags);
  fprintf(stderr, "%s     gauge:                      %d\n", first, Tide->gauge);
  fprintf(stderr, "%s     datum:                      %d\n", first, Tide->datum);
  fprintf(stderr, "%s     latency:                    %f\n", first, Tide->latency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, Tide->latitude_northing);
  fprintf(stderr, "%s     longitude:                  %f\n", first, Tide->longitude_easting);
  fprintf(stderr, "%s     height:                     %f\n", first, Tide->height);
  fprintf(stderr, "%s     type:                       %d\n", first, Tide->type);
  fprintf(stderr, "%s     utm_zone:                   %d\n", first, Tide->utm_zone);

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
int mbsys_reson7k3_print_Altitude(int verbose, s7k3_Altitude *Altitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Altitude:     %p\n", (void *)Altitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Altitude->header, error);

  /* print Altitude (record 1006) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     Altitude:                   %f\n", first, Altitude->altitude);

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
int mbsys_reson7k3_print_MotionOverGround(int verbose, s7k3_MotionOverGround *MotionOverGround, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       MotionOverGround:       %p\n", (void *)MotionOverGround);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &MotionOverGround->header, error);

  /* print Motion over ground (record 1007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     bitfield:                   %d\n", first, MotionOverGround->flags);
  fprintf(stderr, "%s     reserved:                   %d\n", first, MotionOverGround->reserved);
  fprintf(stderr, "%s     n:                          %d\n", first, MotionOverGround->n);
  fprintf(stderr, "%s     frequency:                  %f\n", first, MotionOverGround->frequency);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, MotionOverGround->nalloc);
  for (int i = 0; i < MotionOverGround->n; i++)
    fprintf(stderr, "%s     i:%d x:%f y:%f z:%f xa:%f ya:%f za:%f\n", first, i, MotionOverGround->x[i], MotionOverGround->y[i], MotionOverGround->z[i],
            MotionOverGround->xa[i], MotionOverGround->ya[i], MotionOverGround->za[i]);

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
int mbsys_reson7k3_print_Depth(int verbose, s7k3_Depth *Depth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Depth:        %p\n", (void *)Depth);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Depth->header, error);

  /* print Depth (record 1008) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     descriptor:                  %d\n", first, Depth->descriptor);
  fprintf(stderr, "%s     correction:                  %d\n", first, Depth->correction);
  fprintf(stderr, "%s     reserved:                    %d\n", first, Depth->reserved);
  fprintf(stderr, "%s     depth:                       %f\n", first, Depth->depth);

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
int mbsys_reson7k3_print_SoundVelocityProfile(int verbose, s7k3_SoundVelocityProfile *SoundVelocityProfile, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       SoundVelocityProfile:          %p\n", (void *)SoundVelocityProfile);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SoundVelocityProfile->header, error);

  /* print Sound velocity profile (record 1009) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     position_flag:              %d\n", first, SoundVelocityProfile->position_flag);
  fprintf(stderr, "%s     reserved1:                  %d\n", first, SoundVelocityProfile->reserved1);
  fprintf(stderr, "%s     reserved2:                  %d\n", first, SoundVelocityProfile->reserved2);
  fprintf(stderr, "%s     latitude:                   %f\n", first, SoundVelocityProfile->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, SoundVelocityProfile->longitude);
  fprintf(stderr, "%s     n:                          %d\n", first, SoundVelocityProfile->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, SoundVelocityProfile->nalloc);
  for (unsigned int i = 0; i < SoundVelocityProfile->n; i++)
    fprintf(stderr, "%s     i:%d depth:%f sound_velocity:%f\n", first, i, SoundVelocityProfile->depth[i], SoundVelocityProfile->sound_velocity[i]);

  int status = MB_SUCCESS;

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
int mbsys_reson7k3_print_CTD(int verbose, s7k3_CTD *CTD, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       CTD:          %p\n", (void *)CTD);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CTD->header, error);

  /* print CTD (record 1010) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, CTD->frequency);
  fprintf(stderr, "%s     velocity_source_flag:       %d\n", first, CTD->velocity_source_flag);
  fprintf(stderr, "%s     velocity_algorithm:         %d\n", first, CTD->velocity_algorithm);
  fprintf(stderr, "%s     conductivity_flag:          %d\n", first, CTD->conductivity_flag);
  fprintf(stderr, "%s     pressure_flag:              %d\n", first, CTD->pressure_flag);
  fprintf(stderr, "%s     position_flag:              %d\n", first, CTD->position_flag);
  fprintf(stderr, "%s     validity:                   %d\n", first, CTD->validity);
  fprintf(stderr, "%s     reserved:                   %d\n", first, CTD->reserved);
  fprintf(stderr, "%s     latitude:                   %f\n", first, CTD->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, CTD->longitude);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, CTD->sample_rate);
  fprintf(stderr, "%s     n:                          %d\n", first, CTD->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, CTD->nalloc);
  for (unsigned int i = 0; i < CTD->n; i++)
    fprintf(stderr, "%s     i:%d conductivity_salinity:%f temperature:%f pressure_depth:%f sound_velocity:%f absorption:%f\n",
            first, i, CTD->conductivity_salinity[i], CTD->temperature[i], CTD->pressure_depth[i], CTD->sound_velocity[i],
            CTD->absorption[i]);

  int status = MB_SUCCESS;

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
int mbsys_reson7k3_print_Geodesy(int verbose, s7k3_Geodesy *Geodesy, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Geodesy:      %p\n", (void *)Geodesy);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Geodesy->header, error);

  /* print Geodesy (record 1011) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     spheroid:                   %s\n", first, Geodesy->spheroid);
  fprintf(stderr, "%s     semimajoraxis:              %f\n", first, Geodesy->semimajoraxis);
  fprintf(stderr, "%s     flattening:                 %f\n", first, Geodesy->flattening);
  fprintf(stderr, "%s     reserved1:                  %s\n", first, Geodesy->reserved1);
  fprintf(stderr, "%s     datum:                      %s\n", first, Geodesy->datum);
  fprintf(stderr, "%s     calculation_method:         %d\n", first, Geodesy->calculation_method);
  fprintf(stderr, "%s     number_parameters:          %d\n", first, Geodesy->number_parameters);
  fprintf(stderr, "%s     dx:                         %f\n", first, Geodesy->dx);
  fprintf(stderr, "%s     dy:                         %f\n", first, Geodesy->dy);
  fprintf(stderr, "%s     dz:                         %f\n", first, Geodesy->dz);
  fprintf(stderr, "%s     rx:                         %f\n", first, Geodesy->rx);
  fprintf(stderr, "%s     ry:                         %f\n", first, Geodesy->ry);
  fprintf(stderr, "%s     rz:                         %f\n", first, Geodesy->rz);
  fprintf(stderr, "%s     scale:                      %f\n", first, Geodesy->scale);
  fprintf(stderr, "%s     reserved2:                  %s\n", first, Geodesy->reserved2);
  fprintf(stderr, "%s     grid_name:                  %s\n", first, Geodesy->grid_name);
  fprintf(stderr, "%s     distance_units:             %d\n", first, Geodesy->distance_units);
  fprintf(stderr, "%s     angular_units:              %d\n", first, Geodesy->angular_units);
  fprintf(stderr, "%s     latitude_origin:            %f\n", first, Geodesy->latitude_origin);
  fprintf(stderr, "%s     central_meriidan:           %f\n", first, Geodesy->central_meridian);
  fprintf(stderr, "%s     false_easting:              %f\n", first, Geodesy->false_easting);
  fprintf(stderr, "%s     false_northing:             %f\n", first, Geodesy->false_northing);
  fprintf(stderr, "%s     central_scale_factor:       %f\n", first, Geodesy->central_scale_factor);
  fprintf(stderr, "%s     custom_identifier:          %d\n", first, Geodesy->custom_identifier);
  fprintf(stderr, "%s     reserved3:                  %s\n", first, Geodesy->reserved3);

  int status = MB_SUCCESS;

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
int mbsys_reson7k3_print_RollPitchHeave(int verbose, s7k3_RollPitchHeave *RollPitchHeave, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       RollPitchHeave: %p\n", (void *)RollPitchHeave);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &RollPitchHeave->header, error);

  /* print Roll pitch heave (record 1012) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     roll:                       %f\n", first, RollPitchHeave->roll);
  fprintf(stderr, "%s     pitch:                      %f\n", first, RollPitchHeave->pitch);
  fprintf(stderr, "%s     heave:                      %f\n", first, RollPitchHeave->heave);

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
int mbsys_reson7k3_print_Heading(int verbose, s7k3_Heading *Heading, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Heading:      %p\n", (void *)Heading);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Heading->header, error);

  /* print Heading (record 1013) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     Heading:                    %f\n", first, Heading->heading);

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
int mbsys_reson7k3_print_SurveyLine(int verbose, s7k3_SurveyLine *SurveyLine, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       SurveyLine:   %p\n", (void *)SurveyLine);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SurveyLine->header, error);

  /* print Survey Line (record 1014) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     n:                          %d\n", first, SurveyLine->n);
  fprintf(stderr, "%s     type:                       %d\n", first, SurveyLine->type);
  fprintf(stderr, "%s     turnradius:                 %f\n", first, SurveyLine->turnradius);
  fprintf(stderr, "%s     name:                       %s\n", first, SurveyLine->name);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, SurveyLine->nalloc);
  for (int i = 0; i < SurveyLine->n; i++)
    fprintf(stderr, "%s     i:%d latitude_northing:%f longitude_easting:%f\n", first, i, SurveyLine->latitude_northing[i], SurveyLine->longitude_easting[i]);

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
int mbsys_reson7k3_print_Navigation(int verbose, s7k3_Navigation *Navigation, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Navigation:   %p\n", (void *)Navigation);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Navigation->header, error);

  /* print Navigation (record 1015) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     vertical_reference:         %d\n", first, Navigation->vertical_reference);
  fprintf(stderr, "%s     latitude:                   %f\n", first, Navigation->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, Navigation->longitude);
  fprintf(stderr, "%s     position_accuracy:          %f\n", first, Navigation->position_accuracy);
  fprintf(stderr, "%s     height:                     %f\n", first, Navigation->height);
  fprintf(stderr, "%s     height_accuracy:            %f\n", first, Navigation->height_accuracy);
  fprintf(stderr, "%s     speed:                      %f\n", first, Navigation->speed);
  fprintf(stderr, "%s     course:                     %f\n", first, Navigation->course);
  fprintf(stderr, "%s     heading:                    %f\n", first, Navigation->heading);

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
int mbsys_reson7k3_print_Attitude(int verbose, s7k3_Attitude *Attitude, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Attitude:     %p\n", (void *)Attitude);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Attitude->header, error);

  /* print Attitude (record 1016) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     n:                          %d\n", first, Attitude->n);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, Attitude->nalloc);
  for (int i = 0; i < Attitude->n; i++)
    fprintf(stderr, "%s     i:%d delta_time:%d roll:%f pitch:%f heading:%f heave:%f\n", first, i, Attitude->delta_time[i],
            Attitude->roll[i], Attitude->pitch[i], Attitude->heave[i], Attitude->heading[i]);

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
int mbsys_reson7k3_print_PanTilt(int verbose, s7k3_PanTilt *PanTilt, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       Navigation:   %p\n", (void *)PanTilt);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &PanTilt->header, error);

  /* print Pan Tilt (record 1017) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     pan:                    %f\n", first, PanTilt->pan);
  fprintf(stderr, "%s     tilt:                   %f\n", first, PanTilt->tilt);

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

int mbsys_reson7k3_print_SonarInstallationIDs(int verbose, s7k3_SonarInstallationIDs *SonarInstallationIDs, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
    fprintf(stderr, "dbg2       SonarInstallationIDs:  %p\n", (void *)SonarInstallationIDs);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SonarInstallationIDs->header, error);

  /* print SonarInstallationIDs (record 1018) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     system_id:              %d\n", first, SonarInstallationIDs->system_id);
  fprintf(stderr, "%s     tx_id:                  %d\n", first, SonarInstallationIDs->tx_id);
  fprintf(stderr, "%s     rx_id:                  %d\n", first, SonarInstallationIDs->rx_id);
  fprintf(stderr, "%s     std_id:                 %d\n", first, SonarInstallationIDs->std_id);
  fprintf(stderr, "%s     conf_pars:              %d\n", first, SonarInstallationIDs->conf_pars);
  fprintf(stderr, "%s     tx_length:              %f\n", first, SonarInstallationIDs->tx_length);
  fprintf(stderr, "%s     tx_width:               %f\n", first, SonarInstallationIDs->tx_width);
  fprintf(stderr, "%s     tx_height:              %f\n", first, SonarInstallationIDs->tx_height);
  fprintf(stderr, "%s     tx_radius:              %f\n", first, SonarInstallationIDs->tx_radius);
  fprintf(stderr, "%s     offset_srp2tx_x         %f\n", first, SonarInstallationIDs->offset_srp2tx_x);
  fprintf(stderr, "%s     offset_srp2tx_y         %f\n", first, SonarInstallationIDs->offset_srp2tx_y);
  fprintf(stderr, "%s     offset_srp2tx_z         %f\n", first, SonarInstallationIDs->offset_srp2tx_z);
  fprintf(stderr, "%s     offset_tx_roll          %f\n", first, SonarInstallationIDs->offset_tx_roll);
  fprintf(stderr, "%s     offset_tx_pitch         %f\n", first, SonarInstallationIDs->offset_tx_pitch);
  fprintf(stderr, "%s     offset_tx_yaw           %f\n", first, SonarInstallationIDs->offset_tx_yaw);
  fprintf(stderr, "%s     rx_length               %f\n", first, SonarInstallationIDs->rx_length);
  fprintf(stderr, "%s     rx_width                %f\n", first, SonarInstallationIDs->rx_width);
  fprintf(stderr, "%s     rx_height               %f\n", first, SonarInstallationIDs->rx_height);
  fprintf(stderr, "%s     rx_radius               %f\n", first, SonarInstallationIDs->rx_radius);
  fprintf(stderr, "%s     offset_srp2rx_x         %f\n", first, SonarInstallationIDs->offset_srp2rx_x);
  fprintf(stderr, "%s     offset_srp2rx_y         %f\n", first, SonarInstallationIDs->offset_srp2rx_y);
  fprintf(stderr, "%s     offset_srp2rx_z         %f\n", first, SonarInstallationIDs->offset_srp2rx_z);
  fprintf(stderr, "%s     offset_rx_roll          %f\n", first, SonarInstallationIDs->offset_rx_roll);
  fprintf(stderr, "%s     offset_rx_pitch         %f\n", first, SonarInstallationIDs->offset_rx_pitch);
  fprintf(stderr, "%s     offset_rx_yaw           %f\n", first, SonarInstallationIDs->offset_rx_yaw);
  fprintf(stderr, "%s     frequency               %f\n", first, SonarInstallationIDs->frequency);
  fprintf(stderr, "%s     offset_vrp2srp_x        %f\n", first, SonarInstallationIDs->offset_vrp2srp_x);
  fprintf(stderr, "%s     offset_vrp2srp_y        %f\n", first, SonarInstallationIDs->offset_vrp2srp_y);
  fprintf(stderr, "%s     offset_vrp2srp_z        %f\n", first, SonarInstallationIDs->offset_vrp2srp_z);
  fprintf(stderr, "%s     cable_length            %d\n", first, SonarInstallationIDs->cable_length);
  for (int i = 0; i < 44; i++) {
    fprintf(stderr, "%s     reserved[%2d]:          %d\n", first, i, SonarInstallationIDs->reserved[i]);
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

int mbsys_reson7k3_print_Mystery(int verbose, s7k3_Mystery *Mystery, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
    fprintf(stderr, "dbg2       Mystery:               %p\n", (void *)Mystery);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Mystery->header, error);

  /* print Mystery (record 1018) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     data:                   \n", first);
  for (int i = 0; i < R7KHDRSIZE_Mystery; i++) {
    fprintf(stderr, "%u \n", Mystery->data[i]);
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

int mbsys_reson7k3_print_SonarPipeEnvironment(int verbose, s7k3_SonarPipeEnvironment *SonarPipeEnvironment, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
    fprintf(stderr, "dbg2       SonarPipeEnvironment:  %p\n", (void *)SonarPipeEnvironment);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SonarPipeEnvironment->header, error);

  /* print SonarPipeEnvironment (record 1018) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     pipe_number:                %u\n", first, SonarPipeEnvironment->pipe_number);
  fprintf(stderr, "%s     s7kTime:                    %4u-%3.3u-%2.2u:%2.2u:%9.6f\n",
            first, SonarPipeEnvironment->s7kTime.Year, SonarPipeEnvironment->s7kTime.Day,
            SonarPipeEnvironment->s7kTime.Hours, SonarPipeEnvironment->s7kTime.Minutes,
            SonarPipeEnvironment->s7kTime.Seconds);
  fprintf(stderr, "%s     ping_number:                 %u\n", first, SonarPipeEnvironment->ping_number);
  fprintf(stderr, "%s     multiping_number:            %u\n", first, SonarPipeEnvironment->multiping_number);
  fprintf(stderr, "%s     pipe_diameter:               %f\n", first, SonarPipeEnvironment->pipe_diameter);
  fprintf(stderr, "%s     sound_velocity:              %f\n", first, SonarPipeEnvironment->sound_velocity);
  fprintf(stderr, "%s     sample_rate:                 %f\n", first, SonarPipeEnvironment->sample_rate);
  fprintf(stderr, "%s     finished:                    %u\n", first, SonarPipeEnvironment->finished);
  fprintf(stderr, "%s     points_number:               %u\n", first, SonarPipeEnvironment->points_number);
  fprintf(stderr, "%s     n:                           %u\n", first, SonarPipeEnvironment->n);
  for (int i = 0; i < 10; i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, SonarPipeEnvironment->reserved[i]);
  }
  for (int i = 0; i < 5; i++) {
    fprintf(stderr, "%s     Points[%d] (x y z angle #):  %.3f %.3f %.3f %.3f %.3f\n",
                  first, i, SonarPipeEnvironment->x[i], SonarPipeEnvironment->y[i],
                  SonarPipeEnvironment->z[i], SonarPipeEnvironment->angle[i],
                  SonarPipeEnvironment->sample_number[i]);
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

int mbsys_reson7k3_print_ContactOutput(int verbose, s7k3_ContactOutput *ContactOutput, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:               %d\n", verbose);
    fprintf(stderr, "dbg2       ContactOutput:         %p\n", (void *)ContactOutput);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &ContactOutput->header, error);

  /* print ContactOutput (record 1018) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     target_id:                  %u\n", first, ContactOutput->target_id);
  fprintf(stderr, "%s     ping_number:                %u\n", first, ContactOutput->ping_number);
  fprintf(stderr, "%s     s7kTime:                    %4u-%3.3u-%2.2u:%2.2u:%9.6f\n",
            first, ContactOutput->s7kTime.Year, ContactOutput->s7kTime.Day,
            ContactOutput->s7kTime.Hours, ContactOutput->s7kTime.Minutes,
            ContactOutput->s7kTime.Seconds);
  fprintf(stderr, "%s     operator_name:               %s\n", first, ContactOutput->operator_name);
  fprintf(stderr, "%s     contact_state:               %u\n", first, ContactOutput->contact_state);
  fprintf(stderr, "%s     range:                       %f\n", first, ContactOutput->range);
  fprintf(stderr, "%s     bearing:                     %f\n", first, ContactOutput->bearing);
  fprintf(stderr, "%s     info_flags:                  %u\n", first, ContactOutput->info_flags);
  fprintf(stderr, "%s     latitude:                    %f\n", first, ContactOutput->latitude);
  fprintf(stderr, "%s     longitude:                   %f\n", first, ContactOutput->longitude);
  fprintf(stderr, "%s     azimuth:                     %f\n", first, ContactOutput->azimuth);
  fprintf(stderr, "%s     contact_length:              %f\n", first, ContactOutput->contact_length);
  fprintf(stderr, "%s     contact_width:               %f\n", first, ContactOutput->contact_width);
  fprintf(stderr, "%s     classification:              %s\n", first, ContactOutput->classification);
  fprintf(stderr, "%s     description:                 %s\n", first, ContactOutput->description);

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

int mbsys_reson7k3_print_ProcessedSideScan(int verbose, s7k3_ProcessedSideScan *ProcessedSideScan, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       ProcessedSideScan: %p\n", (void *)ProcessedSideScan);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &ProcessedSideScan->header, error);

  /* print Reson 7k beam geometry (record 7004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) ProcessedSideScan->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, ProcessedSideScan->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, ProcessedSideScan->multi_ping);
  fprintf(stderr, "%s     recordversion:              %u\n", first, ProcessedSideScan->recordversion);
  fprintf(stderr, "%s     ss_source:                  %u\n", first, ProcessedSideScan->ss_source);
  fprintf(stderr, "%s     number_pixels:              %u\n", first, ProcessedSideScan->number_pixels);
  fprintf(stderr, "%s     pixelwidth:                 %f\n", first, ProcessedSideScan->pixelwidth);
  fprintf(stderr, "%s     sensordepth:                 %f\n", first, ProcessedSideScan->sensordepth);
  fprintf(stderr, "%s     altitude:                   %f\n", first, ProcessedSideScan->altitude);
  for (unsigned int i = 0; i < ProcessedSideScan->number_pixels; i++)
    fprintf(stderr, "%s     pixel[%d]:  sidescan:%f alongtrack:%f\n", first, i, ProcessedSideScan->sidescan[i],
            ProcessedSideScan->alongtrack[i]);

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

int mbsys_reson7k3_print_SonarSettings(int verbose, s7k3_SonarSettings *SonarSettings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SonarSettings:  %p\n", (void *)SonarSettings);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SonarSettings->header, error);

  /* print Reson 7k Ssonar settings (record 7000) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SonarSettings->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, SonarSettings->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, SonarSettings->multi_ping);
  fprintf(stderr, "%s     frequency:                  %f\n", first, SonarSettings->frequency);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, SonarSettings->sample_rate);
  fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, SonarSettings->receiver_bandwidth);
  fprintf(stderr, "%s     tx_pulse_width:             %f\n", first, SonarSettings->tx_pulse_width);
  fprintf(stderr, "%s     tx_pulse_type:              %d\n", first, SonarSettings->tx_pulse_type);
  fprintf(stderr, "%s     tx_pulse_envelope:          %d\n", first, SonarSettings->tx_pulse_envelope);
  fprintf(stderr, "%s     tx_pulse_envelope_par:      %f\n", first, SonarSettings->tx_pulse_envelope_par);
  fprintf(stderr, "%s     tx_pulse_mode:              %d\n", first, SonarSettings->tx_pulse_mode);
  fprintf(stderr, "%s     max_ping_rate:              %f\n", first, SonarSettings->max_ping_rate);
  fprintf(stderr, "%s     ping_period:                %f\n", first, SonarSettings->ping_period);
  fprintf(stderr, "%s     range_selection:            %f\n", first, SonarSettings->range_selection);
  fprintf(stderr, "%s     power_selection:            %f\n", first, SonarSettings->power_selection);
  fprintf(stderr, "%s     gain_selection:             %f\n", first, SonarSettings->gain_selection);
  fprintf(stderr, "%s     control_flags:              %d\n", first, SonarSettings->control_flags);
  fprintf(stderr, "%s     projector_magic_no:         %d\n", first, SonarSettings->projector_id);
  fprintf(stderr, "%s     steering_vertical:          %f\n", first, SonarSettings->steering_vertical);
  fprintf(stderr, "%s     steering_horizontal:        %f\n", first, SonarSettings->steering_horizontal);
  fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, SonarSettings->beamwidth_vertical);
  fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, SonarSettings->beamwidth_horizontal);
  fprintf(stderr, "%s     focal_point:                %f\n", first, SonarSettings->focal_point);
  fprintf(stderr, "%s     projector_weighting:        %d\n", first, SonarSettings->projector_weighting);
  fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, SonarSettings->projector_weighting_par);
  fprintf(stderr, "%s     transmit_flags:             %d\n", first, SonarSettings->transmit_flags);
  fprintf(stderr, "%s     hydrophone_magic_no:        %d\n", first, SonarSettings->hydrophone_id);
  fprintf(stderr, "%s     rx_weighting:               %d\n", first, SonarSettings->rx_weighting);
  fprintf(stderr, "%s     rx_weighting_par:           %f\n", first, SonarSettings->rx_weighting_par);
  fprintf(stderr, "%s     rx_flags:                   %d\n", first, SonarSettings->rx_flags);
  fprintf(stderr, "%s     rx_width:                   %f\n", first, SonarSettings->rx_width);
  fprintf(stderr, "%s     range_minimum:              %f\n", first, SonarSettings->range_minimum);
  fprintf(stderr, "%s     range_maximum:              %f\n", first, SonarSettings->range_maximum);
  fprintf(stderr, "%s     depth_minimum:              %f\n", first, SonarSettings->depth_minimum);
  fprintf(stderr, "%s     depth_maximum:              %f\n", first, SonarSettings->depth_maximum);
  fprintf(stderr, "%s     absorption:                 %f\n", first, SonarSettings->absorption);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, SonarSettings->sound_velocity);
  fprintf(stderr, "%s     spreading:                  %f\n", first, SonarSettings->spreading);
  fprintf(stderr, "%s     reserved:                   %d\n", first, SonarSettings->reserved);

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
int mbsys_reson7k3_print_device(int verbose, s7k3_device *device, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       device:            %p\n", (void *)device);
  }

  /* print Reson 7k device Configuration structure (part of record 7001) */
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
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) device->serial_number);
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
int mbsys_reson7k3_print_Configuration(int verbose, s7k3_Configuration *Configuration, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       Configuration:     %p\n", (void *)Configuration);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Configuration->header, error);

  /* print Reson 7k Configuration (record 7001) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) Configuration->serial_number);
  fprintf(stderr, "%s     number_devices:             %llu\n", first, (long long unsigned) Configuration->number_devices);
  for (unsigned int i = 0; i < Configuration->number_devices; i++)
    mbsys_reson7k3_print_device(verbose, &Configuration->device[i], error);

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
int mbsys_reson7k3_print_MatchFilter(int verbose, s7k3_MatchFilter *MatchFilter, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       MatchFilter:       %p\n", (void *)MatchFilter);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &MatchFilter->header, error);

  /* print Reson 7k match filter (record 7002) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) MatchFilter->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, MatchFilter->ping_number);
  fprintf(stderr, "%s     operation:                  %d\n", first, MatchFilter->operation);
  fprintf(stderr, "%s     start_frequency:            %f\n", first, MatchFilter->start_frequency);
  fprintf(stderr, "%s     end_frequency:              %f\n", first, MatchFilter->end_frequency);
  fprintf(stderr, "%s     window_type:                %u\n", first, MatchFilter->window_type);
  fprintf(stderr, "%s     shading:                    %f\n", first, MatchFilter->shading);
  fprintf(stderr, "%s     pulse_width:                %f\n", first, MatchFilter->pulse_width);
  for (int i = 0;i<13;i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, MatchFilter->reserved[i]);
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
int mbsys_reson7k3_print_FirmwareHardwareConfiguration(int verbose,
                                                        s7k3_FirmwareHardwareConfiguration *FirmwareHardwareConfiguration,
                                                        int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       FirmwareHardwareConfiguration:       %p\n", (void *)FirmwareHardwareConfiguration);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &FirmwareHardwareConfiguration->header, error);

  /* print Reson firmware and hardware Configuration (record 7003) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     device_count:               %d\n", first, FirmwareHardwareConfiguration->device_count);
  fprintf(stderr, "%s     info_length:                %d\n", first, FirmwareHardwareConfiguration->info_length);
  fprintf(stderr, "%s     info:                       \n", first);
  fprintf(stderr, "%s\n%s\n", FirmwareHardwareConfiguration->info, first);

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
int mbsys_reson7k3_print_BeamGeometry(int verbose, s7k3_BeamGeometry *BeamGeometry, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       BeamGeometry:      %p\n", (void *)BeamGeometry);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &BeamGeometry->header, error);

  /* print Reson 7k beam geometry (record 7004) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) BeamGeometry->serial_number);
  fprintf(stderr, "%s     number_beams:               %u\n", first, BeamGeometry->number_beams);
  for (unsigned int i = 0; i < BeamGeometry->number_beams; i++)
    fprintf(stderr,
            "%s     beam[%d]:  angle_alongtrack:%f angle_acrosstrack:%f beamwidth_alongtrack:%f beamwidth_acrosstrack:%f\n",
            first, i, BeamGeometry->angle_alongtrack[i], BeamGeometry->angle_acrosstrack[i],
            BeamGeometry->beamwidth_alongtrack[i], BeamGeometry->beamwidth_acrosstrack[i]);

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
int mbsys_reson7k3_print_Bathymetry(int verbose, s7k3_Bathymetry *Bathymetry, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       Bathymetry:        %p\n", (void *)Bathymetry);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Bathymetry->header, error);

  /* print Reson 7k Bathymetry (record 7006) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) Bathymetry->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, Bathymetry->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, Bathymetry->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, Bathymetry->number_beams);
  fprintf(stderr, "%s     layer_comp_flag:            %d\n", first, Bathymetry->layer_comp_flag);
  fprintf(stderr, "%s     sound_vel_flag:             %d\n", first, Bathymetry->sound_vel_flag);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, Bathymetry->sound_velocity);
  for (unsigned int i = 0; i < Bathymetry->number_beams; i++)
    fprintf(stderr, "%s     beam[%d]:  range:%f quality:%d intensity:%f min_depth_gate:%f min_depth_gate:%f\n", first, i,
            Bathymetry->range[i], Bathymetry->quality[i], Bathymetry->intensity[i], Bathymetry->min_depth_gate[i],
            Bathymetry->max_depth_gate[i]);
  fprintf(stderr, "%s     optionaldata:               %d\n", first, Bathymetry->optionaldata);
  fprintf(stderr, "%s     frequency:                  %f\n", first, Bathymetry->frequency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, Bathymetry->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, Bathymetry->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, Bathymetry->heading);
  fprintf(stderr, "%s     height_source:              %d\n", first, Bathymetry->height_source);
  fprintf(stderr, "%s     tide:                       %f\n", first, Bathymetry->tide);
  fprintf(stderr, "%s     roll:                       %f\n", first, Bathymetry->roll);
  fprintf(stderr, "%s     pitch:                      %f\n", first, Bathymetry->pitch);
  fprintf(stderr, "%s     heave:                      %f\n", first, Bathymetry->heave);
  fprintf(stderr, "%s     vehicle_depth:              %f\n", first, Bathymetry->vehicle_depth);
  for (unsigned int i = 0; i < Bathymetry->number_beams; i++)
    fprintf(stderr, "%s     beam[%d]:  depth:%f ltrack:%f xtrack:%f angles: %f %f\n", first, i, Bathymetry->depth[i],
            Bathymetry->alongtrack[i], Bathymetry->acrosstrack[i], Bathymetry->pointing_angle[i],
            Bathymetry->azimuth_angle[i]);

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
int mbsys_reson7k3_print_SideScan(int verbose, s7k3_SideScan *SideScan, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SideScan:       %p\n", (void *)SideScan);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SideScan->header, error);

  /* print Reson 7k SideScan data (record 7007) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SideScan->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, SideScan->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, SideScan->multi_ping);
  fprintf(stderr, "%s     beam_position:              %f\n", first, SideScan->beam_position);
  fprintf(stderr, "%s     control_flags:              %u\n", first, SideScan->control_flags);
  fprintf(stderr, "%s     number_samples:             %u\n", first, SideScan->number_samples);
  fprintf(stderr, "%s     nadir_depth:                %u\n", first, SideScan->nadir_depth);
  for (int i = 0;i<7;i++) {
     fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, SideScan->reserved[i]);
  }
  fprintf(stderr, "%s     number_beams:               %u\n", first, SideScan->number_beams);
  fprintf(stderr, "%s     current_beam:               %u\n", first, SideScan->current_beam);
  fprintf(stderr, "%s     sample_size:                %u\n", first, SideScan->sample_size);
  fprintf(stderr, "%s     data_type:                  %u\n", first, SideScan->data_type);
  fprintf(stderr, "%s     nalloc:                     %u\n", first, SideScan->nalloc);

  if (SideScan->sample_size == 1) {
    mb_s_char *charptr = (mb_s_char *)SideScan->port_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     port SideScan[%d]:  %d\n", first, i, charptr[i]);
    charptr = (mb_s_char *)SideScan->stbd_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     stbd SideScan[%d]:  %d\n", first, i, charptr[i]);
  } else if (SideScan->sample_size == 2) {
    short *shortptr = (short *)SideScan->port_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     port SideScan[%d]:  %d\n", first, i, shortptr[i]);
    shortptr = (short *)SideScan->stbd_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     stbd SideScan[%d]:  %d\n", first, i, shortptr[i]);
  } else if (SideScan->sample_size == 4) {
    int *intptr = (int *)SideScan->port_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     port SideScan[%d]:  %d\n", first, i, intptr[i]);
    intptr = (int *)SideScan->stbd_data;
    for (unsigned int i = 0; i < SideScan->number_samples; i++)
      fprintf(stderr, "%s     stbd SideScan[%d]:  %d\n", first, i, intptr[i]);
  }

  fprintf(stderr, "%s     optionaldata:               %u\n", first, SideScan->optionaldata);
  fprintf(stderr, "%s     frequency:                  %f\n", first, SideScan->frequency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, SideScan->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, SideScan->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, SideScan->heading);
  fprintf(stderr, "%s     altitude:                   %f\n", first, SideScan->altitude);
  fprintf(stderr, "%s     depth:                      %f\n", first, SideScan->depth);

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

int mbsys_reson7k3_print_WaterColumn(int verbose, s7k3_WaterColumn *WaterColumn, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       WaterColumn:       %p\n", (void *)WaterColumn);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &WaterColumn->header, error);

  /* print Reson 7k vertical depth data (record 7009) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first,(long long unsigned)  WaterColumn->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, WaterColumn->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, WaterColumn->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, WaterColumn->number_beams);
  fprintf(stderr, "%s     reserved:                   %u\n", first, WaterColumn->reserved);
  fprintf(stderr, "%s     samples:                    %u\n", first, WaterColumn->samples);
  fprintf(stderr, "%s     subset_flag:                %u\n", first, WaterColumn->subset_flag);
  fprintf(stderr, "%s     column_flag:                %u\n", first, WaterColumn->column_flag);
  fprintf(stderr, "%s     reserved2:                  %u\n", first, WaterColumn->reserved2);
  fprintf(stderr, "%s     sample_type:                %u\n", first, WaterColumn->sample_type);
  int sample_type_amp = WaterColumn->sample_type & 15;
  int sample_type_phase = (WaterColumn->sample_type >> 4) & 15;
  int sample_type_iandq = (WaterColumn->sample_type >> 8) & 15;
  char *charptr;
  unsigned short *ushortptr;
  unsigned int *uintptr;
  short *shortptramp;
  short *shortptrphase;
  int *intptramp;
  int *intptrphase;
  for (unsigned int i = 0; i < WaterColumn->number_beams; i++) {
    s7k3_wcd *wcd = &WaterColumn->wcd[i];
    fprintf(stderr, "%s     beam[%3d] # begin end:   %u   %u %u\n", first, i, wcd->beam_number,  wcd->begin_sample,  wcd->end_sample);
    int nsamples = wcd->end_sample - wcd->begin_sample + 1;
    for (int j = 0; j < nsamples; j++) {
      fprintf(stderr, "%s   sample[%4d | %4d] ", first, j, j + wcd->begin_sample);
      if (sample_type_amp == 1) {
        charptr = (char *)wcd->amplitude;
        fprintf(stderr, "amplitude: %u  ", charptr[j]);
      }
      else if (sample_type_amp == 2) {
        ushortptr = (unsigned short *)wcd->amplitude;
        fprintf(stderr, "amplitude: %u  ", ushortptr[j]);
      }
      else if (sample_type_amp == 3) {
        uintptr = (unsigned int *)wcd->amplitude;
        fprintf(stderr, "amplitude: %u  ", uintptr[j]);
      }
      if (sample_type_phase == 1) {
        charptr = (char *)wcd->phase;
        fprintf(stderr, "phase: %u  ", charptr[j]);
      }
      else if (sample_type_phase == 2) {
        ushortptr = (unsigned short *)wcd->phase;
        fprintf(stderr, "phase: %u  ", ushortptr[j]);
      }
      else if (sample_type_phase == 3) {
        uintptr = (unsigned int *)wcd->phase;
        fprintf(stderr, "phase: %u  ", uintptr[j]);
      }
      if (sample_type_iandq == 1) {
        shortptramp = (short *)wcd->amplitude;
        shortptrphase = (short *)wcd->phase;
        fprintf(stderr, "amplitude: %u  phase: %u  ", shortptramp[j], shortptrphase[j]);
      }
      else if (sample_type_iandq == 2) {
        intptramp = (int *)wcd->amplitude;
        intptrphase = (int *)wcd->phase;
        fprintf(stderr, "amplitude: %u  phase: %u  ", intptramp[j], intptrphase[j]);
      }
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
int mbsys_reson7k3_print_VerticalDepth(int verbose, s7k3_VerticalDepth *VerticalDepth, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       VerticalDepth:     %p\n", (void *)VerticalDepth);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &VerticalDepth->header, error);

  /* print Reson 7k vertical depth data (record 7009) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, VerticalDepth->frequency);
  fprintf(stderr, "%s     ping_number:                %u\n", first, VerticalDepth->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, VerticalDepth->multi_ping);
  fprintf(stderr, "%s     latitude:                   %f\n", first, VerticalDepth->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, VerticalDepth->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, VerticalDepth->heading);
  fprintf(stderr, "%s     alongtrack:                 %f\n", first, VerticalDepth->alongtrack);
  fprintf(stderr, "%s     acrosstrack:                %f\n", first, VerticalDepth->acrosstrack);
  fprintf(stderr, "%s     vertical_depth:             %f\n", first, VerticalDepth->vertical_depth);

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
int mbsys_reson7k3_print_TVG(int verbose, s7k3_TVG *TVG, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       TVG:               %p\n", (void *)TVG);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &TVG->header, error);

  /* print Reson 7k TVG data (record 7010) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) TVG->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, TVG->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, TVG->multi_ping);
  fprintf(stderr, "%s     n:                          %d\n", first, TVG->n);
  for (int i = 0; i < 8; i++)
    fprintf(stderr, "%s     reserved[%d]:                %d\n", first, i, TVG->reserved[i]);
  for (unsigned int i = 0; i < TVG->n; i++) {
    float *tvg_float = (float *)TVG->tvg;
    fprintf(stderr, "%s     TVG[%d]:  %f\n", first, i, tvg_float[i]);
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
int mbsys_reson7k3_print_Image(int verbose, s7k3_Image *Image, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       Image:             %p\n", (void *)Image);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Image->header, error);

  /* print Reson 7k image imagery data (record 7011) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     ping_number:                %u\n", first, Image->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, Image->multi_ping);
  fprintf(stderr, "%s     width:                      %d\n", first, Image->width);
  fprintf(stderr, "%s     height:                     %d\n", first, Image->height);
  fprintf(stderr, "%s     color_depth:                %d\n", first, Image->color_depth);
  fprintf(stderr, "%s     reserved:                   %d\n", first, Image->reserved);
  fprintf(stderr, "%s     compression:                %d\n", first, Image->compression);
  fprintf(stderr, "%s     samples:                    %d\n", first, Image->samples);
  fprintf(stderr, "%s     flag:                       %d\n", first, Image->flag);
  fprintf(stderr, "%s     rx_delay:                   %f\n", first, Image->rx_delay);
  for (int i = 0;i<6;i++) {
    fprintf(stderr, "%s     reserved[%d]:                %d\n", first, i, Image->reserved2[i]);
  }
  fprintf(stderr, "%s     nalloc:                     %d\n", first, Image->nalloc);

  if (Image->color_depth == 1) {
    mb_s_char *charptr = (mb_s_char *)Image->image;
    for (unsigned int i = 0; i < Image->width * Image->height; i++)
      fprintf(stderr, "%s     Image[%d]:  %hhu\n", first, i, charptr[i]);
  } else if (Image->color_depth == 2) {
    short *shortptr = (short *)Image->image;
    for (unsigned int i = 0; i < Image->width * Image->height; i++)
      fprintf(stderr, "%s     Image[%d]:  %hu\n", first, i, shortptr[i]);
  } else if (Image->color_depth == 4) {
    int *intptr = (int *)Image->image;
    for (unsigned int i = 0; i < Image->width * Image->height; i++)
      fprintf(stderr, "%s     Image[%d]:  %u\n", first, i, intptr[i]);
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
int mbsys_reson7k3_print_PingMotion(int verbose, s7k3_PingMotion *PingMotion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       PingMotion:      %p\n", (void *)PingMotion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &PingMotion->header, error);

  /* print Reson 7k ping MotionOverGround (record 7012) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) PingMotion->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, PingMotion->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, PingMotion->multi_ping);
  fprintf(stderr, "%s     n:                          %d\n", first, PingMotion->n);
  fprintf(stderr, "%s     flags:                      %d\n", first, PingMotion->flags);
  fprintf(stderr, "%s     error_flags:                %d\n", first, PingMotion->error_flags);
  fprintf(stderr, "%s     frequency:                  %f\n", first, PingMotion->frequency);
  fprintf(stderr, "%s     pitch:                      %f\n", first, PingMotion->pitch);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, PingMotion->nalloc);
  fprintf(stderr, "%s     beam  roll    heading    heave\n", first);
  fprintf(stderr, "%s     ----  ----    -------    -----\n", first);
  for (unsigned int i = 0; i < PingMotion->n; i++) {
    fprintf(stderr, "%s     %3d  %10g  %10g  %10g\n", first, i, PingMotion->roll[i], PingMotion->heading[i],
            PingMotion->heave[i]);
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

int mbsys_reson7k3_print_AdaptiveGate(int verbose, s7k3_AdaptiveGate *AdaptiveGate, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       AdaptiveGate:      %p\n", (void *)AdaptiveGate);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &AdaptiveGate->header, error);

  /* print Reson 7k detection setup (record 7017) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     record_size:                %u\n", first, AdaptiveGate->record_size);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) AdaptiveGate->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, AdaptiveGate->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, AdaptiveGate->multi_ping);
  fprintf(stderr, "%s     n:               %u\n", first, AdaptiveGate->n);
  fprintf(stderr, "%s     gate_size:            %u\n", first, AdaptiveGate->gate_size);
  fprintf(stderr, "%s     beam   angle   min_limit    max_limit\n", first);
  fprintf(stderr, "%s     -------------------------------------\n", first);
  for (unsigned int i = 0; i < AdaptiveGate->n; i++) {
    fprintf(stderr, "%s     %3d  %f   %f   %f\n", first,
            i, AdaptiveGate->angle[i], AdaptiveGate->min_limit[i],
            AdaptiveGate->max_limit[i]);
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
int mbsys_reson7k3_print_DetectionDataSetup(int verbose, s7k3_DetectionDataSetup *DetectionDataSetup, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       DetectionDataSetup:  %p\n", (void *)DetectionDataSetup);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &DetectionDataSetup->header, error);

  /* print Reson 7k detection setup (record 7017) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) DetectionDataSetup->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, DetectionDataSetup->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, DetectionDataSetup->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, DetectionDataSetup->number_beams);
  fprintf(stderr, "%s     data_block_size:            %d\n", first, DetectionDataSetup->data_block_size);
  fprintf(stderr, "%s     detection_algorithm:        %d\n", first, DetectionDataSetup->detection_algorithm);
  fprintf(stderr, "%s     detection_flags:            %d\n", first, DetectionDataSetup->detection_flags);
  fprintf(stderr, "%s     minimum_depth:              %f\n", first, DetectionDataSetup->minimum_depth);
  fprintf(stderr, "%s     maximum_depth:              %f\n", first, DetectionDataSetup->maximum_depth);
  fprintf(stderr, "%s     minimum_range:              %f\n", first, DetectionDataSetup->minimum_range);
  fprintf(stderr, "%s     maximum_range:              %f\n", first, DetectionDataSetup->maximum_range);
  fprintf(stderr, "%s     minimum_nadir_search:       %f\n", first, DetectionDataSetup->minimum_nadir_search);
  fprintf(stderr, "%s     maximum_nadir_search:       %f\n", first, DetectionDataSetup->maximum_nadir_search);
  fprintf(stderr, "%s     automatic_filter_window:    %u\n", first, DetectionDataSetup->automatic_filter_window);
  fprintf(stderr, "%s     applied_roll:               %f\n", first, DetectionDataSetup->applied_roll);
  fprintf(stderr, "%s     depth_gate_tilt:            %f\n", first, DetectionDataSetup->depth_gate_tilt);
  fprintf(stderr, "%s     nadir_depth:                %f\n", first, DetectionDataSetup->nadir_depth);
  for (int i = 0; i < 13; i++) {
    fprintf(stderr, "%s     reserved[%2d]:               %u\n", first, i, DetectionDataSetup->reserved[i]);
  }
  fprintf(stderr, "%s     beam  descriptor pick flag amin amax umin umax quality uncertainty\n", first);
  fprintf(stderr, "%s     ---------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < DetectionDataSetup->number_beams; i++) {
    fprintf(stderr, "%s     %3d %u %10.3f %u %f %f %f %f %u %f\n", first, i, DetectionDataSetup->beam_descriptor[i],
            DetectionDataSetup->detection_point[i], DetectionDataSetup->flags[i], DetectionDataSetup->auto_limits_min_sample[i],
            DetectionDataSetup->auto_limits_max_sample[i], DetectionDataSetup->user_limits_min_sample[i],
            DetectionDataSetup->user_limits_max_sample[i], DetectionDataSetup->quality[i], DetectionDataSetup->uncertainty[i]);
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
int mbsys_reson7k3_print_Beamformed(int verbose, s7k3_Beamformed *Beamformed, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       Beamformed:      %p\n", (void *)Beamformed);
  }

  /* Reson 7k Beamformed magnitude and phase data (record 7018) */
  mbsys_reson7k3_print_header(verbose, &Beamformed->header, error);

  /* print Reson 7k Beamformed Data (record 7018)  */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) Beamformed->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, Beamformed->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, Beamformed->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, Beamformed->number_beams);
  fprintf(stderr, "%s     number_samples:             %d\n", first, Beamformed->number_samples);
  fprintf(stderr, "%s     number_samples:             %d\n", first, Beamformed->number_samples);
  fprintf(stderr, "%s     reserved:                   ", first);
  for (int i = 0; i < 8; i++)
    fprintf(stderr, "%u ", Beamformed->reserved[i]);
  fprintf(stderr, "\n");
  for (int i = 0; i < Beamformed->number_beams; i++) {
    s7k3_amplitudephase *amplitudephase = &(Beamformed->amplitudephase[i]);
    fprintf(stderr, "%s     beam_number:                %d\n", first, amplitudephase->beam_number);
    fprintf(stderr, "%s     number_samples:             %d\n", first, amplitudephase->number_samples);
    for (unsigned int j = 0; j < amplitudephase->number_samples; j++) {
      fprintf(stderr, "%s     beam[%d] sample[%d] amplitude:%u phase:%d\n", first, i, j, amplitudephase->amplitude[j],
              amplitudephase->phase[j]);
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

int mbsys_reson7k3_print_VernierProcessingDataRaw(int verbose, s7k3_VernierProcessingDataRaw *VernierProcessingDataRaw, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       VernierProcessingDataRaw: %p\n", (void *)VernierProcessingDataRaw);
  }

  /* Reson 7k VernierProcessingDataRaw magnitude and phase data (record 7018) */
  mbsys_reson7k3_print_header(verbose, &VernierProcessingDataRaw->header, error);

  /* print Reson 7k VernierProcessingDataRaw Data (record 7018)  */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) VernierProcessingDataRaw->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, VernierProcessingDataRaw->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, VernierProcessingDataRaw->multi_ping);
  fprintf(stderr, "%s     reference_array:            %u\n", first, VernierProcessingDataRaw->reference_array);
  fprintf(stderr, "%s     pair1_array2:               %u\n", first, VernierProcessingDataRaw->pair1_array2);
  fprintf(stderr, "%s     decimator:                  %u\n", first, VernierProcessingDataRaw->decimator);
  fprintf(stderr, "%s     beam_number:                %u\n", first, VernierProcessingDataRaw->beam_number);
  fprintf(stderr, "%s     n:                          %u\n", first, VernierProcessingDataRaw->n);
  fprintf(stderr, "%s     decimated_samples:          %u\n", first, VernierProcessingDataRaw->decimated_samples);
  fprintf(stderr, "%s     first_sample:               %u\n", first, VernierProcessingDataRaw->first_sample);
  for (int i = 0; i < 2; i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, VernierProcessingDataRaw->reserved[i]);
  }
  fprintf(stderr, "%s     smoothing_type:             %u\n", first, VernierProcessingDataRaw->smoothing_type);
  fprintf(stderr, "%s     smoothing_length:           %u\n", first, VernierProcessingDataRaw->smoothing_length);
  for (int i = 0; i < 2; i++) {
    fprintf(stderr, "%s     reserved2[%d]:               %u\n", first, i, VernierProcessingDataRaw->reserved2[i]);
  }
  fprintf(stderr, "%s     magnitude:                  %f\n", first, VernierProcessingDataRaw->magnitude);
  fprintf(stderr, "%s     min_qf:                     %f\n", first, VernierProcessingDataRaw->min_qf);
  fprintf(stderr, "%s     max_qf:                     %f\n", first, VernierProcessingDataRaw->max_qf);
  fprintf(stderr, "%s     min_angle:                  %f\n", first, VernierProcessingDataRaw->min_angle);
  fprintf(stderr, "%s     max_angle:                  %f\n", first, VernierProcessingDataRaw->max_angle);
  fprintf(stderr, "%s     elevation_coverage:         %f\n", first, VernierProcessingDataRaw->elevation_coverage);
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "%s     reserved3[%d]:               %u\n", first, i, VernierProcessingDataRaw->reserved3[i]);
  }
  fprintf(stderr, "%s     nalloc:                     %u\n", first, VernierProcessingDataRaw->nalloc);
  for (int i = 0; i < VernierProcessingDataRaw->beam_number; i++) {
    s7k3_anglemagnitude *anglemagnitude = &(VernierProcessingDataRaw->anglemagnitude[i]);
    for (unsigned int j = 0; j < VernierProcessingDataRaw->decimated_samples; j++) {
      fprintf(stderr, "%s     beam[%d] sample[%d] angle:%u magnitude:%u "
      "coherence:%u cross_power:%u quality_factor:%u reserved:%u\n",
      first, i, j, anglemagnitude->angle[j], anglemagnitude->magnitude[j],
      anglemagnitude->coherence[j], anglemagnitude->cross_power[j],
      anglemagnitude->quality_factor[j], anglemagnitude->reserved[j]);
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
int mbsys_reson7k3_print_BITE(int verbose, s7k3_BITE *BITE, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       BITE:      %p\n", (void *)BITE);
  }

  /* print Reson 7k data record header information */
  if (verbose > 0)
  	mbsys_reson7k3_print_header(verbose, &BITE->header, error);

  /* Reson 7k BITE (record 7021) */
  const char *first;
  if (verbose >= 2) {
    first = debug_str;
  }
  else if (verbose == 1) {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  	fprintf(stderr, "%sBITE Structure Contents:\n", first);
  	fprintf(stderr, "%s     number_reports:             %u\n", first, BITE->number_reports);
  	for (int i = 0; i < BITE->number_reports; i++) {
    	s7k3_bitereport *bitereport = &(BITE->bitereports[i]);
    	fprintf(stderr, "%s     source_name:                %s\n", first, bitereport->source_name);
    	fprintf(stderr, "%s     source_address:             %u\n", first, bitereport->source_address);
    	fprintf(stderr, "%s     frequency:                  %f\n", first, bitereport->reserved);
    	fprintf(stderr, "%s     enumerator:                 %u\n", first, bitereport->reserved2);
    	s7k3_time *s7kTime = &(bitereport->downlink_time);
    	fprintf(stderr, "%s     downlink_time:              %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n",
            first, s7kTime->Year, s7kTime->Day,
            s7kTime->Hours, s7kTime->Minutes, s7kTime->Seconds);
    	s7kTime = &(bitereport->uplink_time);
    	fprintf(stderr, "%s     uplink_time:                %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n",
            first, s7kTime->Year, s7kTime->Day,
            s7kTime->Hours, s7kTime->Minutes, s7kTime->Seconds);
    	s7kTime = &(bitereport->bite_time);
    	fprintf(stderr, "%s     bite_time:                  %4.4d/%3.3d %2.2d:%2.2d:%9.6f\n",
            first, s7kTime->Year, s7kTime->Day,
            s7kTime->Hours, s7kTime->Minutes, s7kTime->Seconds);
    	fprintf(stderr, "%s     status:                     %u\n", first, bitereport->status);
    	fprintf(stderr, "%s     number_bite:                %u\n", first, bitereport->number_bite);
    	fprintf(stderr, "%s     bite_status:                ", first);
    	for (int j = 0; j < 4; j++)
      	  fprintf(stderr, "%llu ", (long long unsigned) bitereport->bite_status[j]);
    	  fprintf(stderr, "\n");
    	  for (int j = 0; j < bitereport->number_bite; j++) {
      		s7k3_bitefield *bitefield = &(bitereport->bitefield[j]);
      		fprintf(stderr, "%s     field[%2d]:                  %u\n", first, j, bitefield->field);
      		fprintf(stderr, "%s     name[%2d]:                   %s\n", first, j, bitefield->name);
      		fprintf(stderr, "%s     device_type[%2d]:            %d\n", first, j, bitefield->device_type);
      		fprintf(stderr, "%s     minimum[%2d]:                %f\n", first, j, bitefield->minimum);
      		fprintf(stderr, "%s     maximum[%2d]:                %f\n", first, j, bitefield->maximum);
      		fprintf(stderr, "%s     value[%2d]:                  %f\n", first, j, bitefield->value);
    	  }
  	  }
  }
  else {
  	fprintf(stdout, "\n");
  	for (int i = 0; i < BITE->number_reports; i++) {
    	s7k3_bitereport *bitereport = &(BITE->bitereports[i]);
    	s7k3_time *s7kTime = &(bitereport->bite_time);
  		fprintf(stdout, "BITE %d of %d: %2u %16s  %4.4d/%3.3d %2.2d:%2.2d:%9.6f ",
  			i, BITE->number_reports, bitereport->source_address, bitereport->source_name, 
            s7kTime->Year, s7kTime->Day,
            s7kTime->Hours, s7kTime->Minutes, s7kTime->Seconds);
        if (bitereport->status & 0x01)
        	fprintf(stdout, "Uplink Error   ");
        else
        	fprintf(stdout, "Uplink Ok      ");
        if (bitereport->status & 0x02)
        	fprintf(stdout, "Downlink Error ");
        else
        	fprintf(stdout, "Downlink Ok    ");
        if (bitereport->status & 0x04)
        	fprintf(stdout, "BITE Error     ");
        else
        	fprintf(stdout, "BITE Ok        ");
        if ((bitereport->status & 0x18) == 0)
        	fprintf(stdout, "-Status Ok\n");
        else if ((bitereport->status & 0x18) == 0x8)
        	fprintf(stdout, "-Status Warning\n");
        else if ((bitereport->status & 0x18) == 0x10)
        	fprintf(stdout, "-Status Error\n");
        else if ((bitereport->status & 0x18) == 0x18)
        	fprintf(stdout, "-Status Fatal\n");
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
int mbsys_reson7k3_print_SonarSourceVersion(int verbose, s7k3_SonarSourceVersion *SonarSourceVersion, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SonarSourceVersion: %p\n", (void *)SonarSourceVersion);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SonarSourceVersion->header, error);

  /* Reson 7k center version (record 7022) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     version:                    %s\n", first, SonarSourceVersion->version);

  int status = MB_SUCCESS;

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
int mbsys_reson7k3_print_WetEndVersion8k(int verbose, s7k3_WetEndVersion8k *WetEndVersion8k, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       WetEndVersion8k:      %p\n", (void *)WetEndVersion8k);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &WetEndVersion8k->header, error);

  /* Reson 7k 8k wet end version (record 7023) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     version:                    %s\n", first, WetEndVersion8k->version);

  int status = MB_SUCCESS;

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
int mbsys_reson7k3_print_RawDetection(int verbose, s7k3_RawDetection *RawDetection, int *error) {
  s7k3_rawdetectiondata *rawdetectiondata;
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       RawDetection:      %p\n", (void *)RawDetection);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &RawDetection->header, error);

  /* print Reson 7k raw detection (record 7027) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) RawDetection->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, RawDetection->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, RawDetection->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, RawDetection->number_beams);
  fprintf(stderr, "%s     data_field_size:            %d\n", first, RawDetection->data_field_size);
  fprintf(stderr, "%s     detection_algorithm:        %d\n", first, RawDetection->detection_algorithm);
  fprintf(stderr, "%s     detection_flags:            %d\n", first, RawDetection->flags);
  fprintf(stderr, "%s     sampling_rate:              %f\n", first, RawDetection->sampling_rate);
  fprintf(stderr, "%s     tx_angle:                   %f\n", first, RawDetection->tx_angle);
  fprintf(stderr, "%s     applied_roll:               %f\n", first, RawDetection->applied_roll);
  fprintf(stderr, "%s     reserved:                   ", first);
  for (int i = 0; i < 15; i++)
    fprintf(stderr, "%u ", RawDetection->reserved[i]);
  fprintf(stderr, "\n%s     beam  beam_descriptor detection_point rx_angle flags quality uncertainty intensity min max\n", first);
  fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
    rawdetectiondata  = (s7k3_rawdetectiondata *) &RawDetection->rawdetectiondata[i];
    fprintf(stderr, "%s     %3d %3u %12.6f %10.6f %3u %11u %.6f %.6f %.6f %.6f\n", first, i, rawdetectiondata->beam_descriptor,
            rawdetectiondata->detection_point, rawdetectiondata->rx_angle, rawdetectiondata->flags,
            rawdetectiondata->quality, rawdetectiondata->uncertainty, rawdetectiondata->signal_strength,
            rawdetectiondata->min_limit, rawdetectiondata->max_limit);
  }
  fprintf(stderr, "%s     optionaldata:                %u\n", first, RawDetection->optionaldata);
  if (RawDetection->optionaldata != false) {
    fprintf(stderr, "%s     frequency:                   %.6f\n", first, RawDetection->frequency);
    fprintf(stderr, "%s     latitude:                    %.6f\n", first, RawDetection->latitude);
    fprintf(stderr, "%s     longitude:                   %.6f\n", first, RawDetection->longitude);
    fprintf(stderr, "%s     heading:                     %.6f\n", first, RawDetection->heading);
    fprintf(stderr, "%s     height_source:               %u\n", first, RawDetection->height_source);
    fprintf(stderr, "%s     tide:                        %.6f\n", first, RawDetection->tide);
    fprintf(stderr, "%s     roll:                        %.6f\n", first, RawDetection->roll);
    fprintf(stderr, "%s     pitch:                       %.6f\n", first, RawDetection->pitch);
    fprintf(stderr, "%s     heave:                       %.6f\n", first, RawDetection->heave);
    fprintf(stderr, "%s     vehicle_depth:               %.6f\n", first, RawDetection->vehicle_depth);
    fprintf(stderr, "\n%s       sdg beam      depth     alongtrack  acrosstrack  pointing_angle  azimuth_angle\n", first);
    fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
    for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
      rawdetectiondata  = (s7k3_rawdetectiondata *) &RawDetection->rawdetectiondata[i];
      s7k3_bathydata *bathydata = (s7k3_bathydata *)&RawDetection->bathydata[i];
      fprintf(stderr, "%s     %4d %4u %12.3f %12.3f %12.3f   %11.6f %11.6f\n",
                        first, i, rawdetectiondata->beam_descriptor,
                        bathydata->depth, bathydata->alongtrack, bathydata->acrosstrack,
                        bathydata->pointing_angle, bathydata->azimuth_angle);
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
int mbsys_reson7k3_print_Snippet(int verbose, s7k3_Snippet *Snippet, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       Snippet:      %p\n", (void *)Snippet);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &Snippet->header, error);

  /* print Reson 7k Snippet (record 7028) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) Snippet->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, Snippet->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, Snippet->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, Snippet->number_beams);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, Snippet->error_flag);
  fprintf(stderr, "%s     control_flags:              %u\n", first, Snippet->control_flags);
  fprintf(stderr, "%s     flags:                      %u\n", first, Snippet->flags);
  for (int i = 0; i < 6; i++)
    fprintf(stderr, "%s     reserved[%d]:               %u\n", first, i, Snippet->reserved[i]);


  for (int i = 0; i < Snippet->number_beams; i++) {
    s7k3_snippetdata *snippetdata = &(Snippet->snippetdata[i]);
    fprintf(stderr, "%s     %5d beam: %u begin:%u detect:%u end:%u nalloc:%u\n",
            first, i, snippetdata->beam_number,
            snippetdata->begin_sample, snippetdata->detect_sample, snippetdata->end_sample,
            snippetdata->nalloc);
    const int nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
    // u16 *u16_ptr = (u16 *)snippetdata->amplitude;
    u32 *u32_ptr = (u32 *)snippetdata->amplitude;
    fprintf(stderr, "%s     ", first);
    for (int j = 0; j < nsample; j++) {
      if ((Snippet->flags & 0x01) != 0) {
        fprintf(stderr, "%9d ", u32_ptr[j]);
      } else {
        fprintf(stderr, "%9d ", u32_ptr[j]);
      }
      if (j == nsample - 1)
        fprintf(stderr, "\n");
      else if ((j+1) % 10 == 0) fprintf(stderr, "\n%s     ", first);
    }
  }
  fprintf(stderr, "%s     optionaldata:               %u\n", first, Snippet->optionaldata);
  if (Snippet->optionaldata) {
    fprintf(stderr, "%s     frequency:                  %f\n", first, Snippet->frequency);
    fprintf(stderr, "%s     latitude:                   %f\n", first, Snippet->latitude);
    fprintf(stderr, "%s     longitude:                  %f\n", first, Snippet->longitude);
    fprintf(stderr, "%s     heading:                    %f\n", first, Snippet->heading);
    for (int i = 0; i < Snippet->number_beams; i++) {
      s7k3_snippetdata *snippetdata = &(Snippet->snippetdata[i]);
      fprintf(stderr, "%s     %5d beam: %u beam_alongtrack:%f beam_acrosstrack:%f center_sample:%u\n",
            first, i, snippetdata->beam_number,
            Snippet->beam_alongtrack[i], Snippet->beam_acrosstrack[i], Snippet->center_sample[i]);
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

int mbsys_reson7k3_print_VernierProcessingDataFiltered(int verbose, s7k3_VernierProcessingDataFiltered *VernierProcessingDataFiltered, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       VernierProcessingDataFiltered:      %p\n", (void *)VernierProcessingDataFiltered);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &VernierProcessingDataFiltered->header, error);

  /* print Reson 7k VernierProcessingDataFiltered (record 7029) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) VernierProcessingDataFiltered->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, VernierProcessingDataFiltered->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, VernierProcessingDataFiltered->multi_ping);
  fprintf(stderr, "%s     number_soundings:           %u\n", first, VernierProcessingDataFiltered->number_soundings);
  fprintf(stderr, "%s     min_angle:                  %f\n", first, VernierProcessingDataFiltered->min_angle);
  fprintf(stderr, "%s     max_angle:                  %f\n", first, VernierProcessingDataFiltered->max_angle);
  fprintf(stderr, "%s     repeat_size:                %u\n", first, VernierProcessingDataFiltered->repeat_size);
  for (int i = 0; i < VernierProcessingDataFiltered->number_soundings; i++) {
    s7k3_vernierprocessingdatasoundings *vernierprocessingdatasoundings
              = &(VernierProcessingDataFiltered->vernierprocessingdatasoundings[i]);
    fprintf(stderr, "%s     sounding[%d]:               %f %u %f %f\n", first, i,
            vernierprocessingdatasoundings->beam_angle, vernierprocessingdatasoundings->sample,
            vernierprocessingdatasoundings->elevation, vernierprocessingdatasoundings->reserved);
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
int mbsys_reson7k3_print_InstallationParameters(int verbose, s7k3_InstallationParameters *InstallationParameters, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       InstallationParameters:      %p\n", (void *)InstallationParameters);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &InstallationParameters->header, error);

  /* print Reson 7k sonar InstallationParameters parameters (record 7030) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     frequency:                  %f\n", first, InstallationParameters->frequency);
  fprintf(stderr, "%s     firmware_version_len:       %d\n", first, InstallationParameters->firmware_version_len);
  fprintf(stderr, "%s     firmware_version:           %s\n", first, InstallationParameters->firmware_version);
  fprintf(stderr, "%s     software_version_len:       %d\n", first, InstallationParameters->software_version_len);
  fprintf(stderr, "%s     software_version:           %s\n", first, InstallationParameters->software_version);
  fprintf(stderr, "%s     s7k3_version_len:           %d\n", first, InstallationParameters->s7k3_version_len);
  fprintf(stderr, "%s     s7k3_version:               %s\n", first, InstallationParameters->s7k3_version);
  fprintf(stderr, "%s     protocal_version_len:       %d\n", first, InstallationParameters->protocal_version_len);
  fprintf(stderr, "%s     protocal_version:           %s\n", first, InstallationParameters->protocal_version);
  fprintf(stderr, "%s     transmit_x:                 %f\n", first, InstallationParameters->transmit_x);
  fprintf(stderr, "%s     transmit_y:                 %f\n", first, InstallationParameters->transmit_y);
  fprintf(stderr, "%s     transmit_z:                 %f\n", first, InstallationParameters->transmit_z);
  fprintf(stderr, "%s     transmit_roll:              %f\n", first, InstallationParameters->transmit_roll);
  fprintf(stderr, "%s     transmit_pitch:             %f\n", first, InstallationParameters->transmit_pitch);
  fprintf(stderr, "%s     transmit_heading:           %f\n", first, InstallationParameters->transmit_heading);
  fprintf(stderr, "%s     receive_x:                  %f\n", first, InstallationParameters->receive_x);
  fprintf(stderr, "%s     receive_y:                  %f\n", first, InstallationParameters->receive_y);
  fprintf(stderr, "%s     receive_z:                  %f\n", first, InstallationParameters->receive_z);
  fprintf(stderr, "%s     receive_roll:               %f\n", first, InstallationParameters->receive_roll);
  fprintf(stderr, "%s     receive_pitch:              %f\n", first, InstallationParameters->receive_pitch);
  fprintf(stderr, "%s     receive_heading:            %f\n", first, InstallationParameters->receive_heading);
  fprintf(stderr, "%s     motion_x:                   %f\n", first, InstallationParameters->motion_x);
  fprintf(stderr, "%s     motion_y:                   %f\n", first, InstallationParameters->motion_y);
  fprintf(stderr, "%s     motion_z:                   %f\n", first, InstallationParameters->motion_z);
  fprintf(stderr, "%s     motion_roll:                %f\n", first, InstallationParameters->motion_roll);
  fprintf(stderr, "%s     motion_pitch:               %f\n", first, InstallationParameters->motion_pitch);
  fprintf(stderr, "%s     motion_heading:             %f\n", first, InstallationParameters->motion_heading);
  fprintf(stderr, "%s     motion_time_delay:          %d\n", first, InstallationParameters->motion_time_delay);
  fprintf(stderr, "%s     Position_x:                 %f\n", first, InstallationParameters->position_x);
  fprintf(stderr, "%s     Position_y:                 %f\n", first, InstallationParameters->position_y);
  fprintf(stderr, "%s     Position_z:                 %f\n", first, InstallationParameters->position_z);
  fprintf(stderr, "%s     Position_time_delay:        %d\n", first, InstallationParameters->position_time_delay);
  fprintf(stderr, "%s     waterline_z:                %f\n", first, InstallationParameters->waterline_z);

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

int mbsys_reson7k3_print_BITESummary(int verbose, s7k3_BITESummary *BITESummary, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       BITESummary:      %p\n", (void *)BITESummary);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &BITESummary->header, error);

  /* print Reson 7k sonar BITESummary parameters (record 7030) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     total_items:                %u\n", first, BITESummary->total_items);
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "%s     warnings[%d]:               %u\n", first, i, BITESummary->warnings[i]);
  }
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "%s     errors[%d]:                 %u\n", first, i, BITESummary->errors[i]);
  }
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "%s     fatals[%d]:                 %u\n", first, i, BITESummary->fatals[i]);
  }
  for (int i = 0; i < 2; i++) {
    fprintf(stderr, "%s     reserved[%d]:               %u\n", first, i, BITESummary->reserved[i]);
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
int mbsys_reson7k3_print_CompressedBeamformedMagnitude(int verbose, s7k3_CompressedBeamformedMagnitude *CompressedBeamformedMagnitude, int *error) {
  (void) verbose;  // Unused arg
  (void) CompressedBeamformedMagnitude;  // Unused arg
  (void) error;  // Unused arg
  // Notdone
  // Not implemented because documentation is vague about the actual sample size
  // and because this record is deprecated and unlikely to be part of a 7k3
  // data stream
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_CompressedWaterColumn(int verbose, s7k3_CompressedWaterColumn *CompressedWaterColumn, int *error) {
  s7k3_compressedwatercolumndata *compressedwatercolumndata;
  size_t samplesize;
  char *m1ptr, *p1ptr;
  short *m2ptr, *p2ptr;
  int *m4ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       CompressedWaterColumn:      %p\n", (void *)CompressedWaterColumn);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CompressedWaterColumn->header, error);

  /* print Reson 7k sonar CompressedWaterColumn parameters (record 7042) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) CompressedWaterColumn->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, CompressedWaterColumn->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, CompressedWaterColumn->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, CompressedWaterColumn->number_beams);
  fprintf(stderr, "%s     samples:                    %u\n", first, CompressedWaterColumn->samples);
  fprintf(stderr, "%s     compressed_samples:         %u\n", first, CompressedWaterColumn->compressed_samples);
  fprintf(stderr, "%s     flags:                      %u\n", first, CompressedWaterColumn->flags);
  fprintf(stderr, "%s     first_sample:               %u\n", first, CompressedWaterColumn->first_sample);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, CompressedWaterColumn->sample_rate);
  fprintf(stderr, "%s     compression_factor:         %f\n", first, CompressedWaterColumn->compression_factor);
  fprintf(stderr, "%s     reserved:                   %u\n", first, CompressedWaterColumn->reserved);
  fprintf(stderr, "%s     magsamplesize:              %zu\n", first, CompressedWaterColumn->magsamplesize);
  fprintf(stderr, "%s     phasesamplesize:            %zu\n", first, CompressedWaterColumn->phasesamplesize);
  samplesize = CompressedWaterColumn->magsamplesize + CompressedWaterColumn->phasesamplesize;
  for (int i = 0;i<CompressedWaterColumn->number_beams;i++) {
    compressedwatercolumndata = (s7k3_compressedwatercolumndata *)&(CompressedWaterColumn->compressedwatercolumndata[i]);
    fprintf(stderr, "%s     beam_number:                %u\n", first, compressedwatercolumndata->beam_number);
    fprintf(stderr, "%s     segment_number:             %u\n", first, compressedwatercolumndata->segment_number);
    fprintf(stderr, "%s     samples:                    %u\n", first, compressedwatercolumndata->samples);
    for (unsigned int j = 0;j<compressedwatercolumndata->samples;j++) {
      const int k = j * samplesize;
      const int l = k + CompressedWaterColumn->magsamplesize;
      if (CompressedWaterColumn->magsamplesize == 1
          && CompressedWaterColumn->phasesamplesize == 0) {
        m1ptr = (char *)&compressedwatercolumndata->data[k];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d\n", first, i, j, *m1ptr);
      }
      else if (CompressedWaterColumn->magsamplesize == 1
          && CompressedWaterColumn->phasesamplesize == 1) {
        m1ptr = (char *)&compressedwatercolumndata->data[k];
        p1ptr = (char *)&compressedwatercolumndata->data[l];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d phase:%d\n", first, i, j, *m1ptr, *p1ptr);
      }
      else if (CompressedWaterColumn->magsamplesize == 2
          && CompressedWaterColumn->phasesamplesize == 0) {
        m2ptr = (short *)&compressedwatercolumndata->data[k];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d\n", first, i, j, *m2ptr);
      }
      else if (CompressedWaterColumn->magsamplesize == 2
          && CompressedWaterColumn->phasesamplesize == 2) {
        m2ptr = (short *)&compressedwatercolumndata->data[k];
        p2ptr = (short *)&compressedwatercolumndata->data[l];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d phase:%d\n", first, i, j, *m2ptr, *p2ptr);
      }
      else if (CompressedWaterColumn->magsamplesize == 4
          && CompressedWaterColumn->phasesamplesize == 0) {
        m4ptr = (int *)&compressedwatercolumndata->data[k];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d\n", first, i, j, *m4ptr);
      }
      else if (CompressedWaterColumn->magsamplesize == 4
          && CompressedWaterColumn->phasesamplesize == 1) {
        m4ptr = (int *)&compressedwatercolumndata->data[k];
        p1ptr = (char *)&compressedwatercolumndata->data[l];
        fprintf(stderr, "%s     beam %4d sample %5d mag:%d phase:%d\n", first, i, j, *m4ptr, *p1ptr);
      }
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
int mbsys_reson7k3_print_SegmentedRawDetection(int verbose, s7k3_SegmentedRawDetection *SegmentedRawDetection, int *error) {
  // s7k3_rawdetectiondata *rawdetectiondata;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SegmentedRawDetection:      %p\n", (void *)SegmentedRawDetection);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SegmentedRawDetection->header, error);

  /* print Reson 7k segmented raw detection (record 7047) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     record_header_size:         %u\n", first, SegmentedRawDetection->record_header_size);
  fprintf(stderr, "%s     n_segments:                 %u\n", first, SegmentedRawDetection->n_segments);
  fprintf(stderr, "%s     segment_field_size:         %u\n", first, SegmentedRawDetection->segment_field_size);
  fprintf(stderr, "%s     n_rx:                       %u\n", first, SegmentedRawDetection->n_rx);
  fprintf(stderr, "%s     rx_field_size:              %u\n", first, SegmentedRawDetection->rx_field_size);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SegmentedRawDetection->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, SegmentedRawDetection->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, SegmentedRawDetection->multi_ping);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, SegmentedRawDetection->sound_velocity);
  fprintf(stderr, "%s     rx_delay:                   %f\n", first, SegmentedRawDetection->rx_delay);
  fprintf(stderr, "\n%s   cnt seg_# txalong txacross txdelay freq type "
                  "bandwidth pulsewidth pulsewidthx pulsewidthl pulseenv "
                  "pulseenvpar srclevel rxbeamwidth algorithm flags samplerate "
                  "tvg rxbandwidth\n", first);
  fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
  for (unsigned int i = 0; i < SegmentedRawDetection->n_segments; i++) {
    s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata =
        (s7k3_segmentedrawdetectiontxdata *)&SegmentedRawDetection->segmentedrawdetectiontxdata[i];
    fprintf(stderr, "%s     %3d %3u %f %f %f %f %d %f %f %f %f %d %f %f %f %d %d %f %d %f\n",
              first, i,
              segmentedrawdetectiontxdata->segment_number,
              segmentedrawdetectiontxdata->tx_angle_along,
              segmentedrawdetectiontxdata->tx_angle_across,
              segmentedrawdetectiontxdata->tx_delay,
              segmentedrawdetectiontxdata->frequency,
              segmentedrawdetectiontxdata->pulse_type,
              segmentedrawdetectiontxdata->pulse_bandwidth,
              segmentedrawdetectiontxdata->tx_pulse_width,
              segmentedrawdetectiontxdata->tx_pulse_width_across,
              segmentedrawdetectiontxdata->tx_pulse_width_along,
              segmentedrawdetectiontxdata->tx_pulse_envelope,
              segmentedrawdetectiontxdata->tx_pulse_envelope_parameter,
              segmentedrawdetectiontxdata->tx_relative_src_level,
              segmentedrawdetectiontxdata->rx_beam_width,
              segmentedrawdetectiontxdata->detection_algorithm,
              segmentedrawdetectiontxdata->flags,
              segmentedrawdetectiontxdata->sampling_rate,
              segmentedrawdetectiontxdata->tvg,
              segmentedrawdetectiontxdata->rx_bandwidth);
  }
  fprintf(stderr, "\n%s   cnt bm_# seg detection rxacross flag quality uncert amp snrat\n", first);
  fprintf(stderr, "%s     ----------------------------------------------------------------------\n", first);
  for (unsigned int i = 0;i<SegmentedRawDetection->n_rx;i++) {
    s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata =
        (s7k3_segmentedrawdetectionrxdata *)&(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
    fprintf(stderr, "%s     %4d %4u %2u %f %f %6u %6u %f %f %f\n",
              first, i,
              segmentedrawdetectionrxdata->beam_number,
              segmentedrawdetectionrxdata->used_segment,
              segmentedrawdetectionrxdata->detection_point,
              segmentedrawdetectionrxdata->rx_angle_cross,
              segmentedrawdetectionrxdata->flags2,
              segmentedrawdetectionrxdata->quality,
              segmentedrawdetectionrxdata->uncertainty,
              segmentedrawdetectionrxdata->signal_strength,
              segmentedrawdetectionrxdata->sn_ratio);
  }
  fprintf(stderr, "\n%s     optionaldata:                %u\n", first, SegmentedRawDetection->optionaldata);
  if (SegmentedRawDetection->optionaldata != false) {
    fprintf(stderr, "%s     frequency:                   %f\n", first, SegmentedRawDetection->frequency);
    fprintf(stderr, "%s     latitude:                    %f\n", first, SegmentedRawDetection->latitude);
    fprintf(stderr, "%s     longitude:                   %f\n", first, SegmentedRawDetection->longitude);
    fprintf(stderr, "%s     heading:                     %f\n", first, SegmentedRawDetection->heading);
    fprintf(stderr, "%s     height_source:               %u\n", first, SegmentedRawDetection->height_source);
    fprintf(stderr, "%s     tide:                        %f\n", first, SegmentedRawDetection->tide);
    fprintf(stderr, "%s     roll:                        %f\n", first, SegmentedRawDetection->roll);
    fprintf(stderr, "%s     pitch:                       %f\n", first, SegmentedRawDetection->pitch);
    fprintf(stderr, "%s     heave:                       %f\n", first, SegmentedRawDetection->heave);
    fprintf(stderr, "%s     vehicle_depth:               %f\n", first, SegmentedRawDetection->vehicle_depth);
    fprintf(stderr, "\n%s   cnt bm_# depth ltrack xtrack ptgangle aziangle\n", first);
    fprintf(stderr, "%s     ------------------------------------------------\n", first);
    for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
      s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata =
          (s7k3_segmentedrawdetectionrxdata *)&(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
      s7k3_bathydata *bathydata = (s7k3_bathydata *)&SegmentedRawDetection->bathydata[i];
      fprintf(stderr, "%s     %4d %4u %12.3f %12.3f %12.3f   %11.6f %11.6f\n",
                      first, i, segmentedrawdetectionrxdata->beam_number,
                      bathydata->depth, bathydata->alongtrack, bathydata->acrosstrack,
                      bathydata->pointing_angle, bathydata->azimuth_angle);
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
int mbsys_reson7k3_print_CalibratedBeam(int verbose, s7k3_CalibratedBeam *CalibratedBeam, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       CalibratedBeam:      %p\n", (void *)CalibratedBeam);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CalibratedBeam->header, error);

  /* print Reson 7k sonar CalibratedBeam parameters (record 7042) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) CalibratedBeam->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, CalibratedBeam->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, CalibratedBeam->multi_ping);
  fprintf(stderr, "%s     first_beam:                 %u\n", first, CalibratedBeam->first_beam);
  fprintf(stderr, "%s     total_beams:                %u\n", first, CalibratedBeam->total_beams);
  fprintf(stderr, "%s     total_samples:              %u\n", first, CalibratedBeam->total_samples);
  fprintf(stderr, "%s     foward_looking_sonar:       %u\n", first, CalibratedBeam->foward_looking_sonar);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, CalibratedBeam->error_flag);
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, CalibratedBeam->reserved[i]);
  }
  fprintf(stderr, "%s     nalloc:         %u\n", first, CalibratedBeam->nalloc);
  for (unsigned int i = 0;i<CalibratedBeam->total_samples;i++) {
    fprintf(stderr, "%s     amplitude[%3d]:             %f\n", first, i, CalibratedBeam->samples[i]);
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
int mbsys_reson7k3_print_SystemEvents(int verbose, s7k3_SystemEvents *SystemEvents, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SystemEvents:%p\n", (void *)SystemEvents);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SystemEvents->header, error);

  /* print Reson 7k system event (record 7051) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SystemEvents->serial_number);
  fprintf(stderr, "%s     number_events:              %d\n", first, SystemEvents->number_events);
  fprintf(stderr, "%s     nalloc:                     %d\n", first, SystemEvents->nalloc);
  for (unsigned int i = 0; i < SystemEvents->number_events; i++) {
    s7k3_systemeventsdata *systemeventsdata = &(SystemEvents->systemeventsdata[i]);
    fprintf(stderr, "%s     event_type:                 %d\n", first, systemeventsdata->event_type);
    fprintf(stderr, "%s     event_id:                   %d\n", first, systemeventsdata->event_id);
    fprintf(stderr, "%s     system_enum:                %d\n", first, systemeventsdata->system_enum);
    fprintf(stderr, "%s     event_message_length:       %d\n", first, systemeventsdata->event_message_length);
    fprintf(stderr, "%s     s7kTime:                    %4u-%3.3u-%2.2u:%2.2u:%9.6f\n",
            first, systemeventsdata->s7kTime.Year, systemeventsdata->s7kTime.Day,
            systemeventsdata->s7kTime.Hours, systemeventsdata->s7kTime.Minutes,
            systemeventsdata->s7kTime.Seconds);
    fprintf(stderr, "%s     message:                    %s\n", first, systemeventsdata->event_message);
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
int mbsys_reson7k3_print_SystemEventMessage(int verbose, s7k3_SystemEventMessage *SystemEventMessage, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SystemEventMessage:%p\n", (void *)SystemEventMessage);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SystemEventMessage->header, error);

  /* print Reson 7k system event (record 7051) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SystemEventMessage->serial_number);
  fprintf(stderr, "%s     event_id:                   %d\n", first, SystemEventMessage->event_id);
  fprintf(stderr, "%s     message_length:             %d\n", first, SystemEventMessage->message_length);
  fprintf(stderr, "%s     event_identifier:           %d\n", first, SystemEventMessage->event_identifier);
  fprintf(stderr, "%s     message_alloc:              %d\n", first, SystemEventMessage->message_alloc);
  fprintf(stderr, "%s     message:                    %s\n", first, SystemEventMessage->message);

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
int mbsys_reson7k3_print_RDRRecordingStatus(int verbose, s7k3_RDRRecordingStatus *RDRRecordingStatus, int *error) {
  (void) verbose;  // Unused arg
  (void) RDRRecordingStatus;  // Unused arg
  (void) error;  // Unused arg
  // Notdone
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_Subscriptions(int verbose, s7k3_Subscriptions *Subscriptions, int *error) {
  (void) verbose;  // Unused arg
  (void) Subscriptions;  // Unused arg
  (void) error;  // Unused arg
  // Notdone
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_RDRStorageRecording(int verbose, s7k3_RDRStorageRecording *RDRStorageRecording, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       RDRStorageRecording:      %p\n", (void *)RDRStorageRecording);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &RDRStorageRecording->header, error);

  /* print Reson 7k Snippet Backscattering Strength (Record 7058) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     diskfree_percentage:        %u\n", first, RDRStorageRecording->diskfree_percentage);
  fprintf(stderr, "%s     number_records:             %u\n", first, RDRStorageRecording->number_records);
  fprintf(stderr, "%s     size:                       %llu\n", first, (long long unsigned) RDRStorageRecording->size);
  for (int i = 0; i < 4; i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, RDRStorageRecording->reserved[i]);
  }
  fprintf(stderr, "%s     mode:                       %u\n", first, RDRStorageRecording->mode);
  fprintf(stderr, "%s     file_name:                  %s\n", first, RDRStorageRecording->file_name);
  fprintf(stderr, "%s     RDR_error:                  %u\n", first, RDRStorageRecording->RDR_error);
  fprintf(stderr, "%s     data_rate:                  %llu\n", first, (long long unsigned) RDRStorageRecording->data_rate);
  fprintf(stderr, "%s     minutes_left:               %u\n", first, RDRStorageRecording->minutes_left);

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
int mbsys_reson7k3_print_CalibrationStatus(int verbose, s7k3_CalibrationStatus *CalibrationStatus, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       CalibrationStatus:      %p\n", (void *)CalibrationStatus);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CalibrationStatus->header, error);

  /* print Reson 7k Snippet Backscattering Strength (Record 7058) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) CalibrationStatus->serial_number);
  fprintf(stderr, "%s     calibration_status:         %u\n", first, CalibrationStatus->calibration_status);
  fprintf(stderr, "%s     percent_complete:           %u\n", first, CalibrationStatus->percent_complete);
  fprintf(stderr, "%s     s7kTime:                    %4u-%3.3u-%2.2u:%2.2u:%9.6f\n",
            first, CalibrationStatus->s7kTime.Year, CalibrationStatus->s7kTime.Day,
            CalibrationStatus->s7kTime.Hours, CalibrationStatus->s7kTime.Minutes,
            CalibrationStatus->s7kTime.Seconds);
  fprintf(stderr, "%s     status_message:             %s\n", first, CalibrationStatus->status_message);
  fprintf(stderr, "%s     sub_status:                 %u\n", first, CalibrationStatus->sub_status);
  fprintf(stderr, "%s     optionaldata:               %u\n", first, CalibrationStatus->optionaldata);
  fprintf(stderr, "%s     system_calibration:         %u\n", first, CalibrationStatus->system_calibration);
  fprintf(stderr, "%s     done_calibration:           %u\n", first, CalibrationStatus->done_calibration);
  fprintf(stderr, "%s     current_calibration:        %u\n", first, CalibrationStatus->current_calibration);
  fprintf(stderr, "%s     startup_calibration:        %u\n", first, CalibrationStatus->startup_calibration);
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%s     status[%d]:                  %u\n", first, i, CalibrationStatus->status[i]);
  }
  for (int i = 0; i < 2; i++) {
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, CalibrationStatus->reserved[i]);
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
int mbsys_reson7k3_print_CalibratedSideScan(int verbose, s7k3_CalibratedSideScan *CalibratedSideScan, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       CalibratedSideScan:      %p\n", (void *)CalibratedSideScan);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CalibratedSideScan->header, error);

  /* print Reson 7k Snippet Backscattering Strength (Record 7058) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) CalibratedSideScan->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, CalibratedSideScan->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, CalibratedSideScan->multi_ping);
  fprintf(stderr, "%s     beam_position:              %f\n", first, CalibratedSideScan->beam_position);
  fprintf(stderr, "%s     reserved:                   %u\n", first, CalibratedSideScan->reserved);
  fprintf(stderr, "%s     samples:                    %u\n", first, CalibratedSideScan->samples);
  fprintf(stderr, "%s     reserved2:                  %f\n", first, CalibratedSideScan->reserved2);
  fprintf(stderr, "%s     beams:                      %u\n", first, CalibratedSideScan->beams);
  fprintf(stderr, "%s     current_beam:               %u\n", first, CalibratedSideScan->current_beam);
  fprintf(stderr, "%s     bytes_persample:            %u\n", first, CalibratedSideScan->bytes_persample);
  fprintf(stderr, "%s     data_types:                 %u\n", first, CalibratedSideScan->data_types);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, CalibratedSideScan->error_flag);
  fprintf(stderr, "%s     nalloc:                     %u\n", first, CalibratedSideScan->nalloc);
  for (unsigned int i = 0; i < CalibratedSideScan->samples; i++) {
    fprintf(stderr, "%s     port[%d]: %10u from beam:%3u\n", first, i, CalibratedSideScan->port_data[i], CalibratedSideScan->port_beam[i]);
  }
  for (unsigned int i = 0; i < CalibratedSideScan->samples; i++) {
    fprintf(stderr, "%s     stbd[%d]: %10u from beam:%3u\n", first, i, CalibratedSideScan->stbd_data[i], CalibratedSideScan->stbd_beam[i]);
  }
  fprintf(stderr, "%s     optionaldata:               %u\n", first, CalibratedSideScan->optionaldata);
  fprintf(stderr, "%s     frequency:                  %f\n", first, CalibratedSideScan->frequency);
  fprintf(stderr, "%s     latitude:                   %f\n", first, CalibratedSideScan->latitude);
  fprintf(stderr, "%s     longitude:                  %f\n", first, CalibratedSideScan->longitude);
  fprintf(stderr, "%s     heading:                    %f\n", first, CalibratedSideScan->heading);
  fprintf(stderr, "%s     depth:                      %f\n", first, CalibratedSideScan->depth);

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
int mbsys_reson7k3_print_SnippetBackscatteringStrength(int verbose, s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SnippetBackscatteringStrength:      %p\n", (void *)SnippetBackscatteringStrength);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SnippetBackscatteringStrength->header, error);

  /* print Reson 7k Snippet Backscattering Strength (Record 7058) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) SnippetBackscatteringStrength->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, SnippetBackscatteringStrength->ping_number);
  fprintf(stderr, "%s     multi_ping:                 %u\n", first, SnippetBackscatteringStrength->multi_ping);
  fprintf(stderr, "%s     number_beams:               %u\n", first, SnippetBackscatteringStrength->number_beams);
  fprintf(stderr, "%s     error_flag:                 %u\n", first, SnippetBackscatteringStrength->error_flag);
  fprintf(stderr, "%s     control_flags:              %u\n", first, SnippetBackscatteringStrength->control_flags);
  fprintf(stderr, "%s     absorption:                 %f\n", first, SnippetBackscatteringStrength->absorption);
  for (int i = 0; i < 6; i++)
    fprintf(stderr, "%s     reserved[%d]:                %u\n", first, i, SnippetBackscatteringStrength->reserved[i]);
  for (int i = 0; i < SnippetBackscatteringStrength->number_beams; i++) {
    s7k3_snippetbackscatteringstrengthdata *snippetbackscatteringstrengthdata =
        &(SnippetBackscatteringStrength->snippetbackscatteringstrengthdata[i]);
    fprintf(stderr, "%s     beam: %u begin:%u bottom:%u end:%u\n", first, snippetbackscatteringstrengthdata->beam_number,
            snippetbackscatteringstrengthdata->begin_sample, snippetbackscatteringstrengthdata->bottom_sample,
            snippetbackscatteringstrengthdata->end_sample);
    for (unsigned int j = 0; j < snippetbackscatteringstrengthdata->end_sample - snippetbackscatteringstrengthdata->begin_sample + 1; j++)
      fprintf(stderr, "%s     bs[%d]:%f\n", first, snippetbackscatteringstrengthdata->begin_sample + j,
              snippetbackscatteringstrengthdata->bs[j]);
    if (SnippetBackscatteringStrength->control_flags & 0x40 && snippetbackscatteringstrengthdata->footprints != NULL) {
      for (unsigned int j = 0; j < snippetbackscatteringstrengthdata->end_sample - snippetbackscatteringstrengthdata->begin_sample + 1; j++)
        fprintf(stderr, "%s     footprints[%d]:%f\n", first, snippetbackscatteringstrengthdata->begin_sample + j,
                snippetbackscatteringstrengthdata->footprints[j]);
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

int mbsys_reson7k3_print_MB2Status(int verbose, s7k3_MB2Status *MB2Status, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       MB2Status:      %p\n", (void *)MB2Status);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &MB2Status->header, error);

  /* print Reson 7k Snippet Backscattering Strength (Record 7058) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     directory:                  %s\n", first, MB2Status->directory);
  fprintf(stderr, "%s     header_name:                %s\n", first, MB2Status->header_name);
  fprintf(stderr, "%s     trailer_name:               %s\n", first, MB2Status->trailer_name);
  fprintf(stderr, "%s     prepend_header:             %u\n", first, MB2Status->prepend_header);
  fprintf(stderr, "%s     storage:                    %u\n", first, MB2Status->storage);
  fprintf(stderr, "%s     playback_path:              %s\n", first, MB2Status->playback_path);
  fprintf(stderr, "%s     playback_file:              %s\n", first, MB2Status->playback_file);
  fprintf(stderr, "%s     playback_loopmode:          %u\n", first, MB2Status->playback_loopmode);
  fprintf(stderr, "%s     playback:                   %u\n", first, MB2Status->playback);
  fprintf(stderr, "%s     rrio_address1:              %s\n", first, MB2Status->rrio_address1);
  fprintf(stderr, "%s     rrio_address2:              %s\n", first, MB2Status->rrio_address2);
  fprintf(stderr, "%s     rrio_address3:              %s\n", first, MB2Status->rrio_address3);
  fprintf(stderr, "%s     build_hpr:                  %u\n", first, MB2Status->build_hpr);
  fprintf(stderr, "%s     attached_hpr:               %u\n", first, MB2Status->attached_hpr);
  fprintf(stderr, "%s     stacking:                   %u\n", first, MB2Status->stacking);
  fprintf(stderr, "%s     stacking_value:             %u\n", first, MB2Status->stacking_value);
  fprintf(stderr, "%s     zda_baudrate:               %u\n", first, MB2Status->zda_baudrate);
  fprintf(stderr, "%s     zda_parity:                 %u\n", first, MB2Status->zda_parity);
  fprintf(stderr, "%s     zda_databits:               %u\n", first, MB2Status->zda_databits);
  fprintf(stderr, "%s     zda_stopbits:               %u\n", first, MB2Status->zda_stopbits);
  fprintf(stderr, "%s     gga_baudrate:               %u\n", first, MB2Status->gga_baudrate);
  fprintf(stderr, "%s     gga_parity:                 %u\n", first, MB2Status->gga_parity);
  fprintf(stderr, "%s     gga_databits:               %u\n", first, MB2Status->gga_databits);
  fprintf(stderr, "%s     gga_stopbits:               %u\n", first, MB2Status->gga_stopbits);
  fprintf(stderr, "%s     svp_baudrate:               %u\n", first, MB2Status->svp_baudrate);
  fprintf(stderr, "%s     svp_parity:                 %u\n", first, MB2Status->svp_parity);
  fprintf(stderr, "%s     svp_databits:               %u\n", first, MB2Status->svp_databits);
  fprintf(stderr, "%s     svp_stopbits:               %u\n", first, MB2Status->svp_stopbits);
  fprintf(stderr, "%s     hpr_baudrate:               %u\n", first, MB2Status->hpr_baudrate);
  fprintf(stderr, "%s     hpr_parity:                 %u\n", first, MB2Status->hpr_parity);
  fprintf(stderr, "%s     hpr_databits:               %u\n", first, MB2Status->hpr_databits);
  fprintf(stderr, "%s     hpr_stopbits:               %u\n", first, MB2Status->hpr_stopbits);
  fprintf(stderr, "%s     hdt_baudrate:               %u\n", first, MB2Status->hdt_baudrate);
  fprintf(stderr, "%s     hdt_parity:                 %u\n", first, MB2Status->hdt_parity);
  fprintf(stderr, "%s     hdt_databits:               %u\n", first, MB2Status->hdt_databits);
  fprintf(stderr, "%s     hdt_stopbits:               %u\n", first, MB2Status->hdt_stopbits);
  fprintf(stderr, "%s     rrio:                       %u\n", first, MB2Status->rrio);
  fprintf(stderr, "%s     playback_timestamps:        %u\n", first, MB2Status->playback_timestamps);
  fprintf(stderr, "%s     reserved:                   %u\n", first, MB2Status->reserved);
  fprintf(stderr, "%s     reserved2:                  %u\n", first, MB2Status->reserved2);

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
int mbsys_reson7k3_print_FileHeader(int verbose, s7k3_FileHeader *FileHeader, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       FileHeader:        %p\n", (void *)FileHeader);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &FileHeader->header, error);

  /* print Reson 7k file header (record 7200) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     file_identifier:            0x", first);
  for (int i = 0; i < 2; i++)
    fprintf(stderr, "%llx", (long long unsigned) FileHeader->file_identifier[i]);
  fprintf(stderr, "\n");
  fprintf(stderr, "%s     version:                    %d\n", first, FileHeader->version);
  fprintf(stderr, "%s     reserved:                   %d\n", first, FileHeader->reserved);
  fprintf(stderr, "%s     session_identifier:         0x", first);
  for (int i = 0; i < 2; i++)
    fprintf(stderr, "%llx", (long long unsigned) FileHeader->session_identifier[i]);
  fprintf(stderr, "\n");
  fprintf(stderr, "%s     record_data_size:           %d\n", first, FileHeader->record_data_size);
  fprintf(stderr, "%s     number_subsystems:          %d\n", first, FileHeader->number_devices);
  fprintf(stderr, "%s     recording_name:             %s\n", first, FileHeader->recording_name);
  fprintf(stderr, "%s     recording_version:          %s\n", first, FileHeader->recording_version);
  fprintf(stderr, "%s     user_defined_name:          %s\n", first, FileHeader->user_defined_name);
  fprintf(stderr, "%s     notes:                      %s\n", first, FileHeader->notes);
  for (unsigned int i = 0; i < FileHeader->number_devices; i++) {
    s7k3_subsystem *subsystem = &FileHeader->subsystem[i];
    fprintf(stderr, "%s     device_identifier:          %d\n", first, subsystem->device_identifier);
    fprintf(stderr, "%s     system_enumerator:          %d\n", first, subsystem->system_enumerator);
  }
  fprintf(stderr, "%s     optionaldata:                 %d\n", first, FileHeader->optionaldata);
  fprintf(stderr, "%s     file_catalog_size:            %u\n", first, FileHeader->file_catalog_size);
  fprintf(stderr, "%s     file_catalog_offset:          %llu\n", first, (long long unsigned) FileHeader->file_catalog_offset);

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
int mbsys_reson7k3_print_FileCatalog(int verbose, s7k3_FileCatalog *FileCatalog, int *error) {
  s7k3_filecatalogdata *filecatalogdata;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       FileCatalog:       %p\n", (void *)FileCatalog);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &FileCatalog->header, error);

  /* print Reson 7k File Catalog (record 7300) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     size:                         %d\n", first, FileCatalog->size);
  fprintf(stderr, "%s     version:                      %d\n", first, FileCatalog->version);
  fprintf(stderr, "%s     n:                            %d\n", first, FileCatalog->n);
  fprintf(stderr, "%s     reserved:                     %d\n", first, FileCatalog->reserved);
  fprintf(stderr, "%s     list of data records (size offset type device system time count 8*reserved):\n", first);
  for (unsigned int i = 0; i < FileCatalog->n; i++) {
    filecatalogdata = &FileCatalog->filecatalogdata[i];
    fprintf(stderr, "%s     %7d %7d %8u %llu %5u %4u %2u %4u-%3.3u-%2.2u:%2.2u:%9.6f %.6f %u %u %u %u %u %u %u %u %u\n",
          first, i, filecatalogdata->sequence,
          filecatalogdata->size, (long long unsigned) filecatalogdata->offset,
          filecatalogdata->record_type, filecatalogdata->device_id,
          filecatalogdata->system_enumerator,
          filecatalogdata->s7kTime.Year,
          filecatalogdata->s7kTime.Day,
          filecatalogdata->s7kTime.Hours,
          filecatalogdata->s7kTime.Minutes,
          filecatalogdata->s7kTime.Seconds,
          filecatalogdata->time_d,
          filecatalogdata->record_count,
          filecatalogdata->reserved[0], filecatalogdata->reserved[1],
          filecatalogdata->reserved[2], filecatalogdata->reserved[3],
          filecatalogdata->reserved[4], filecatalogdata->reserved[5],
          filecatalogdata->reserved[6], filecatalogdata->reserved[7]);
  }

  // calculate record totals and print those out
  int nrec_read = 0;
  int nrec_write = 0;
  int nrec_ReferencePoint = 0;
  int nrec_UncalibratedSensorOffset = 0;
  int nrec_CalibratedSensorOffset = 0;
  int nrec_Position = 0;
  int nrec_CustomAttitude = 0;
  int nrec_Tide = 0;
  int nrec_Altitude = 0;
  int nrec_MotionOverGround = 0;
  int nrec_Depth = 0;
  int nrec_SoundVelocityProfile = 0;
  int nrec_CTD = 0;
  int nrec_Geodesy = 0;
  int nrec_RollPitchHeave = 0;
  int nrec_Heading = 0;
  int nrec_SurveyLine = 0;
  int nrec_Navigation = 0;
  int nrec_Attitude = 0;
  int nrec_PanTilt = 0;
  int nrec_SonarInstallationIDs = 0;
  int nrec_Mystery = 0;
  int nrec_SonarPipeEnvironment = 0;
  int nrec_ContactOutput = 0;
  int nrec_ProcessedSideScan = 0;
  int nrec_SonarSettings = 0;
  int nrec_Configuration = 0;
  int nrec_MatchFilter = 0;
  int nrec_FirmwareHardwareConfiguration = 0;
  int nrec_BeamGeometry = 0;
  int nrec_Bathymetry = 0;
  int nrec_SideScan = 0;
  int nrec_WaterColumn = 0;
  int nrec_VerticalDepth = 0;
  int nrec_TVG = 0;
  int nrec_Image = 0;
  int nrec_PingMotion = 0;
  int nrec_AdaptiveGate = 0;
  int nrec_DetectionDataSetup = 0;
  int nrec_Beamformed = 0;
  int nrec_VernierProcessingDataRaw = 0;
  int nrec_BITE = 0;
  int nrec_SonarSourceVersion = 0;
  int nrec_WetEndVersion8k = 0;
  int nrec_RawDetection = 0;
  int nrec_Snippet = 0;
  int nrec_VernierProcessingDataFiltered = 0;
  int nrec_InstallationParameters = 0;
  int nrec_BITESummary = 0;
  int nrec_CompressedBeamformedMagnitude = 0;
  int nrec_CompressedWaterColumn = 0;
  int nrec_SegmentedRawDetection = 0;
  int nrec_CalibratedBeam = 0;
  int nrec_SystemEvents = 0;
  int nrec_SystemEventMessage = 0;
  int nrec_RDRRecordingStatus = 0;
  int nrec_Subscriptions = 0;
  int nrec_RDRStorageRecording = 0;
  int nrec_CalibrationStatus = 0;
  int nrec_CalibratedSideScan = 0;
  int nrec_SnippetBackscatteringStrength = 0;
  int nrec_MB2Status = 0;
  int nrec_FileHeader = 0;
  int nrec_FileCatalog = 0;
  int nrec_TimeMessage = 0;
  int nrec_RemoteControl = 0;
  int nrec_RemoteControlAcknowledge = 0;
  int nrec_RemoteControlNotAcknowledge = 0;
  int nrec_RemoteControlSonarSettings = 0;
  int nrec_CommonSystemSettings = 0;
  int nrec_SVFiltering = 0;
  int nrec_SystemLockStatus = 0;
  int nrec_SoundVelocity = 0;
  int nrec_AbsorptionLoss = 0;
  int nrec_SpreadingLoss = 0;
  int nrec_ProfileAverageSalinity = 0;
  int nrec_ProfileAverageTemperature = 0;
  for (unsigned int i = 0; i < FileCatalog->n; i++) {
    filecatalogdata = &FileCatalog->filecatalogdata[i];

    switch (filecatalogdata->record_type) {
      case (R7KRECID_ReferencePoint):
        nrec_ReferencePoint++;
        break;
      case (R7KRECID_UncalibratedSensorOffset):
        nrec_UncalibratedSensorOffset++;
        break;
      case (R7KRECID_CalibratedSensorOffset):
        nrec_CalibratedSensorOffset++;
        break;
      case (R7KRECID_Position):
        nrec_Position++;
        break;
      case (R7KRECID_CustomAttitude):
        nrec_CustomAttitude++;
        break;
      case (R7KRECID_Tide):
        nrec_Tide++;
        break;
      case (R7KRECID_Altitude):
        nrec_Altitude++;
        break;
      case (R7KRECID_MotionOverGround):
        nrec_MotionOverGround++;
        break;
      case (R7KRECID_Depth):
        nrec_Depth++;
        break;
      case (R7KRECID_SoundVelocityProfile):
        nrec_SoundVelocityProfile++;
        break;
      case (R7KRECID_CTD):
        nrec_CTD++;
        break;
      case (R7KRECID_Geodesy):
        nrec_Geodesy++;
        break;
      case (R7KRECID_RollPitchHeave):
        nrec_RollPitchHeave++;
        break;
      case (R7KRECID_Heading):
        nrec_Heading++;
        break;
      case (R7KRECID_SurveyLine):
        nrec_SurveyLine++;
        break;
      case (R7KRECID_Navigation):
        nrec_Navigation++;
        break;
      case (R7KRECID_Attitude):
        nrec_Attitude++;
        break;
      case (R7KRECID_PanTilt):
        nrec_PanTilt++;
        break;
      case (R7KRECID_SonarInstallationIDs):
        nrec_SonarInstallationIDs++;
        break;
      case (R7KRECID_Mystery):
        nrec_Mystery++;
        break;
      case (R7KRECID_SonarPipeEnvironment):
        nrec_SonarPipeEnvironment++;
        break;
      case (R7KRECID_ContactOutput):
        nrec_ContactOutput++;
        break;
      case (R7KRECID_ProcessedSideScan):
        nrec_ProcessedSideScan++;
        break;
      case (R7KRECID_SonarSettings):
        nrec_SonarSettings++;
        break;
      case (R7KRECID_Configuration):
        nrec_Configuration++;
        break;
      case (R7KRECID_MatchFilter):
        nrec_MatchFilter++;
        break;
      case (R7KRECID_FirmwareHardwareConfiguration):
        nrec_FirmwareHardwareConfiguration++;
        break;
      case (R7KRECID_BeamGeometry):
        nrec_BeamGeometry++;
        break;
      case (R7KRECID_Bathymetry):
        nrec_Bathymetry++;
        break;
      case (R7KRECID_SideScan):
        nrec_SideScan++;
        break;
      case (R7KRECID_WaterColumn):
        nrec_WaterColumn++;
        break;
      case (R7KRECID_VerticalDepth):
        nrec_VerticalDepth++;
        break;
      case (R7KRECID_TVG):
        nrec_TVG++;
        break;
      case (R7KRECID_Image):
        nrec_Image++;
        break;
      case (R7KRECID_PingMotion):
        nrec_PingMotion++;
        break;
      case (R7KRECID_AdaptiveGate):
        nrec_AdaptiveGate++;
        break;
      case (R7KRECID_DetectionDataSetup):
        nrec_DetectionDataSetup++;
        break;
      case (R7KRECID_Beamformed):
        nrec_Beamformed++;
        break;
      case (R7KRECID_VernierProcessingDataRaw):
        nrec_VernierProcessingDataRaw++;
        break;
      case (R7KRECID_BITE):
        nrec_BITE++;
        break;
      case (R7KRECID_SonarSourceVersion):
        nrec_SonarSourceVersion++;
        break;
      case (R7KRECID_WetEndVersion8k):
        nrec_WetEndVersion8k++;
        break;
      case (R7KRECID_RawDetection):
        nrec_RawDetection++;
        break;
      case (R7KRECID_Snippet):
        nrec_Snippet++;
        break;
      case (R7KRECID_VernierProcessingDataFiltered):
        nrec_VernierProcessingDataFiltered++;
        break;
      case (R7KRECID_InstallationParameters):
        nrec_InstallationParameters++;
        break;
      case (R7KRECID_BITESummary):
        nrec_BITESummary++;
        break;
      case (R7KRECID_CompressedBeamformedMagnitude):
        nrec_CompressedBeamformedMagnitude++;
        break;
      case (R7KRECID_CompressedWaterColumn):
        nrec_CompressedWaterColumn++;
        break;
      case (R7KRECID_SegmentedRawDetection):
        nrec_SegmentedRawDetection++;
        break;
      case (R7KRECID_CalibratedBeam):
        nrec_CalibratedBeam++;
        break;
      case (R7KRECID_SystemEvents):
        nrec_SystemEvents++;
        break;
      case (R7KRECID_SystemEventMessage):
        nrec_SystemEventMessage++;
        break;
      case (R7KRECID_RDRRecordingStatus):
        nrec_RDRRecordingStatus++;
        break;
      case (R7KRECID_Subscriptions):
        nrec_Subscriptions++;
        break;
      case (R7KRECID_RDRStorageRecording):
        nrec_RDRStorageRecording++;
        break;
      case (R7KRECID_CalibrationStatus):
        nrec_CalibrationStatus++;
        break;
      case (R7KRECID_CalibratedSideScan):
        nrec_CalibratedSideScan++;
        break;
      case (R7KRECID_SnippetBackscatteringStrength):
        nrec_SnippetBackscatteringStrength++;
        break;
      case (R7KRECID_MB2Status):
        nrec_MB2Status++;
        break;
      case (R7KRECID_FileHeader):
        nrec_FileHeader++;
        break;
      case (R7KRECID_FileCatalog):
        nrec_FileCatalog++;
        break;
      case (R7KRECID_TimeMessage):
        nrec_TimeMessage++;
        break;
      case (R7KRECID_RemoteControl):
        nrec_RemoteControl++;
        break;
      case (R7KRECID_RemoteControlAcknowledge):
        nrec_RemoteControlAcknowledge++;
        break;
      case (R7KRECID_RemoteControlNotAcknowledge):
        nrec_RemoteControlNotAcknowledge++;
        break;
      case (R7KRECID_RemoteControlSonarSettings):
        nrec_RemoteControlSonarSettings++;
        break;
      case (R7KRECID_CommonSystemSettings):
        nrec_CommonSystemSettings++;
        break;
      case (R7KRECID_SVFiltering):
        nrec_SVFiltering++;
        break;
      case (R7KRECID_SystemLockStatus):
        nrec_SystemLockStatus++;
        break;
      case (R7KRECID_SoundVelocity):
        nrec_SoundVelocity++;
        break;
      case (R7KRECID_AbsorptionLoss):
        nrec_AbsorptionLoss++;
        break;
      case (R7KRECID_SpreadingLoss):
        nrec_SpreadingLoss++;
        break;
      case (R7KRECID_ProfileAverageSalinity):
        nrec_ProfileAverageSalinity++;
        break;
      case (R7KRECID_ProfileAverageTemperature):
        nrec_ProfileAverageTemperature++;
        break;
      default:
        break;
      }
    }
fprintf(stderr, "\nCounts of record types (total:%d):\n", FileCatalog->n);
fprintf(stderr, "nrec_ReferencePoint:                       %7d\n", nrec_ReferencePoint);
fprintf(stderr, "nrec_read:                                 %7d\n", nrec_read);
fprintf(stderr, "nrec_write:                                %7d\n", nrec_write);
fprintf(stderr, "nrec_ReferencePoint:                       %7d\n", nrec_ReferencePoint);
fprintf(stderr, "nrec_UncalibratedSensorOffset:             %7d\n", nrec_UncalibratedSensorOffset);
fprintf(stderr, "nrec_CalibratedSensorOffset:               %7d\n", nrec_CalibratedSensorOffset);
fprintf(stderr, "nrec_Position:                             %7d\n", nrec_Position);
fprintf(stderr, "nrec_CustomAttitude:                       %7d\n", nrec_CustomAttitude);
fprintf(stderr, "nrec_Tide:                                 %7d\n", nrec_Tide);
fprintf(stderr, "nrec_Altitude:                             %7d\n", nrec_Altitude);
fprintf(stderr, "nrec_MotionOverGround:                     %7d\n", nrec_MotionOverGround);
fprintf(stderr, "nrec_Depth:                                %7d\n", nrec_Depth);
fprintf(stderr, "nrec_SoundVelocityProfile:                 %7d\n", nrec_SoundVelocityProfile);
fprintf(stderr, "nrec_CTD:                                  %7d\n", nrec_CTD);
fprintf(stderr, "nrec_Geodesy:                              %7d\n", nrec_Geodesy);
fprintf(stderr, "nrec_RollPitchHeave:                       %7d\n", nrec_RollPitchHeave);
fprintf(stderr, "nrec_Heading:                              %7d\n", nrec_Heading);
fprintf(stderr, "nrec_SurveyLine:                           %7d\n", nrec_SurveyLine);
fprintf(stderr, "nrec_Navigation:                           %7d\n", nrec_Navigation);
fprintf(stderr, "nrec_Attitude:                             %7d\n", nrec_Attitude);
fprintf(stderr, "nrec_PanTilt:                              %7d\n", nrec_PanTilt);
fprintf(stderr, "nrec_SonarInstallationIDs:                 %7d\n", nrec_SonarInstallationIDs);
fprintf(stderr, "nrec_Mystery:                              %7d\n", nrec_Mystery);
fprintf(stderr, "nrec_SonarPipeEnvironment:                 %7d\n", nrec_SonarPipeEnvironment);
fprintf(stderr, "nrec_ContactOutput:                        %7d\n", nrec_ContactOutput);
fprintf(stderr, "nrec_ProcessedSideScan:                    %7d\n", nrec_ProcessedSideScan);
fprintf(stderr, "nrec_SonarSettings:                        %7d\n", nrec_SonarSettings);
fprintf(stderr, "nrec_Configuration:                        %7d\n", nrec_Configuration);
fprintf(stderr, "nrec_MatchFilter:                          %7d\n", nrec_MatchFilter);
fprintf(stderr, "nrec_FirmwareHardwareConfiguration:        %7d\n", nrec_FirmwareHardwareConfiguration);
fprintf(stderr, "nrec_BeamGeometry:                         %7d\n", nrec_BeamGeometry);
fprintf(stderr, "nrec_Bathymetry:                           %7d\n", nrec_Bathymetry);
fprintf(stderr, "nrec_SideScan:                             %7d\n", nrec_SideScan);
fprintf(stderr, "nrec_WaterColumn:                          %7d\n", nrec_WaterColumn);
fprintf(stderr, "nrec_VerticalDepth:                        %7d\n", nrec_VerticalDepth);
fprintf(stderr, "nrec_TVG:                                  %7d\n", nrec_TVG);
fprintf(stderr, "nrec_Image:                                %7d\n", nrec_Image);
fprintf(stderr, "nrec_PingMotion:                           %7d\n", nrec_PingMotion);
fprintf(stderr, "nrec_AdaptiveGate:                         %7d\n", nrec_AdaptiveGate);
fprintf(stderr, "nrec_DetectionDataSetup:                   %7d\n", nrec_DetectionDataSetup);
fprintf(stderr, "nrec_Beamformed:                           %7d\n", nrec_Beamformed);
fprintf(stderr, "nrec_VernierProcessingDataRaw:             %7d\n", nrec_VernierProcessingDataRaw);
fprintf(stderr, "nrec_BITE:                                 %7d\n", nrec_BITE);
fprintf(stderr, "nrec_SonarSourceVersion:                   %7d\n", nrec_SonarSourceVersion);
fprintf(stderr, "nrec_WetEndVersion8k:                      %7d\n", nrec_WetEndVersion8k);
fprintf(stderr, "nrec_RawDetection:                         %7d\n", nrec_RawDetection);
fprintf(stderr, "nrec_Snippet:                              %7d\n", nrec_Snippet);
fprintf(stderr, "nrec_VernierProcessingDataFiltered:        %7d\n", nrec_VernierProcessingDataFiltered);
fprintf(stderr, "nrec_InstallationParameters:               %7d\n", nrec_InstallationParameters);
fprintf(stderr, "nrec_BITESummary:                          %7d\n", nrec_BITESummary);
fprintf(stderr, "nrec_CompressedBeamformedMagnitude:        %7d\n", nrec_CompressedBeamformedMagnitude);
fprintf(stderr, "nrec_CompressedWaterColumn:                %7d\n", nrec_CompressedWaterColumn);
fprintf(stderr, "nrec_SegmentedRawDetection:                %7d\n", nrec_SegmentedRawDetection);
fprintf(stderr, "nrec_CalibratedBeam:                       %7d\n", nrec_CalibratedBeam);
fprintf(stderr, "nrec_SystemEvents:                         %7d\n", nrec_SystemEvents);
fprintf(stderr, "nrec_SystemEventMessage:                   %7d\n", nrec_SystemEventMessage);
fprintf(stderr, "nrec_RDRRecordingStatus:                   %7d\n", nrec_RDRRecordingStatus);
fprintf(stderr, "nrec_Subscriptions:                        %7d\n", nrec_Subscriptions);
fprintf(stderr, "nrec_RDRStorageRecording:                  %7d\n", nrec_RDRStorageRecording);
fprintf(stderr, "nrec_CalibrationStatus:                    %7d\n", nrec_CalibrationStatus);
fprintf(stderr, "nrec_CalibratedSideScan:                   %7d\n", nrec_CalibratedSideScan);
fprintf(stderr, "nrec_SnippetBackscatteringStrength:        %7d\n", nrec_SnippetBackscatteringStrength);
fprintf(stderr, "nrec_MB2Status:                            %7d\n", nrec_MB2Status);
fprintf(stderr, "nrec_FileHeader:                           %7d\n", nrec_FileHeader);
fprintf(stderr, "nrec_FileCatalog:                          %7d\n", nrec_FileCatalog);
fprintf(stderr, "nrec_TimeMessage:                          %7d\n", nrec_TimeMessage);
fprintf(stderr, "nrec_RemoteControl:                        %7d\n", nrec_RemoteControl);
fprintf(stderr, "nrec_RemoteControlAcknowledge:             %7d\n", nrec_RemoteControlAcknowledge);
fprintf(stderr, "nrec_RemoteControlNotAcknowledge:          %7d\n", nrec_RemoteControlNotAcknowledge);
fprintf(stderr, "nrec_RemoteControlSonarSettings:           %7d\n", nrec_RemoteControlSonarSettings);
fprintf(stderr, "nrec_CommonSystemSettings:                 %7d\n", nrec_CommonSystemSettings);
fprintf(stderr, "nrec_SVFiltering:                          %7d\n", nrec_SVFiltering);
fprintf(stderr, "nrec_SystemLockStatus:                     %7d\n", nrec_SystemLockStatus);
fprintf(stderr, "nrec_SoundVelocity:                        %7d\n", nrec_SoundVelocity);
fprintf(stderr, "nrec_AbsorptionLoss:                       %7d\n", nrec_AbsorptionLoss);
fprintf(stderr, "nrec_SpreadingLoss:                        %7d\n", nrec_SpreadingLoss);
fprintf(stderr, "nrec_ProfileAverageSalinity:               %7d\n", nrec_ProfileAverageSalinity);
fprintf(stderr, "nrec_ProfileAverageTemperature:            %7d\n", nrec_ProfileAverageTemperature);

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
int mbsys_reson7k3_print_TimeMessage(int verbose, s7k3_TimeMessage *TimeMessage, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       TimeMessage:       %p\n", (void *)TimeMessage);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &TimeMessage->header, error);

  /* print Reson 7k file header (record 7200) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     second_offset:              %d\n", first, TimeMessage->second_offset);
  fprintf(stderr, "%s     pulse_flag:                 %u\n", first, TimeMessage->pulse_flag);
  fprintf(stderr, "%s     port_id:                    %u\n", first, TimeMessage->port_id);
  fprintf(stderr, "%s     reserved:                   %u\n", first, TimeMessage->reserved);
  fprintf(stderr, "%s     reserved2:                  %llu\n", first, (long long unsigned) TimeMessage->reserved2);
  fprintf(stderr, "%s     optionaldata:               %d\n", first, TimeMessage->optionaldata);
  fprintf(stderr, "%s     utctime:                    %f\n", first, TimeMessage->utctime);
  fprintf(stderr, "%s     external_time:              %f\n", first, TimeMessage->external_time);
  fprintf(stderr, "%s     t0:                         %f\n", first, TimeMessage->t0);
  fprintf(stderr, "%s     t1:                         %f\n", first, TimeMessage->t1);
  fprintf(stderr, "%s     pulse_length:               %f\n", first, TimeMessage->pulse_length);
  fprintf(stderr, "%s     difference:                 %f\n", first, TimeMessage->difference);
  fprintf(stderr, "%s     io_status:                  %u\n", first, TimeMessage->io_status);

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
int mbsys_reson7k3_print_RemoteControl(int verbose, s7k3_RemoteControl *RemoteControl, int *error) {
  // Notdone
  (void) verbose;  // Unused arg
  (void) RemoteControl;  // Unused arg
  (void) error;  // Unused arg
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_RemoteControlAcknowledge(int verbose, s7k3_RemoteControlAcknowledge *RemoteControlAcknowledge, int *error) {
  (void) verbose;  // Unused arg
  (void) RemoteControlAcknowledge;  // Unused arg
  (void) error;  // Unused arg
  // Notdone
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_RemoteControlNotAcknowledge(int verbose, s7k3_RemoteControlNotAcknowledge *RemoteControlNotAcknowledge, int *error) {
  (void) verbose;  // Unused arg
  (void) RemoteControlNotAcknowledge;  // Unused arg
  (void) error;  // Unused arg
  // Notdone
  return (MB_SUCCESS);
}

/*--------------------------------------------------------------------*/
int mbsys_reson7k3_print_RemoteControlSonarSettings(int verbose, s7k3_RemoteControlSonarSettings *RemoteControlSonarSettings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       RemoteControlSonarSettings:  %p\n", (void *)RemoteControlSonarSettings);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &RemoteControlSonarSettings->header, error);

  /* print Reson 7k remote control sonar settings (record 7503) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) RemoteControlSonarSettings->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, RemoteControlSonarSettings->ping_number);
  fprintf(stderr, "%s     frequency:                  %f\n", first, RemoteControlSonarSettings->frequency);
  fprintf(stderr, "%s     sample_rate:                %f\n", first, RemoteControlSonarSettings->sample_rate);
  fprintf(stderr, "%s     receiver_bandwidth:         %f\n", first, RemoteControlSonarSettings->receiver_bandwidth);
  fprintf(stderr, "%s     tx_pulse_width:             %f\n", first, RemoteControlSonarSettings->tx_pulse_width);
  fprintf(stderr, "%s     tx_pulse_type:              %d\n", first, RemoteControlSonarSettings->tx_pulse_type);
  fprintf(stderr, "%s     tx_pulse_envelope:          %d\n", first, RemoteControlSonarSettings->tx_pulse_envelope);
  fprintf(stderr, "%s     tx_pulse_envelope_par:      %f\n", first, RemoteControlSonarSettings->tx_pulse_envelope_par);
  fprintf(stderr, "%s     tx_pulse_mode:              %d\n", first, RemoteControlSonarSettings->tx_pulse_mode);
  fprintf(stderr, "%s     tx_pulse_reserved:          %d\n", first, RemoteControlSonarSettings->tx_pulse_reserved);
  fprintf(stderr, "%s     max_ping_rate:              %f\n", first, RemoteControlSonarSettings->max_ping_rate);
  fprintf(stderr, "%s     ping_period:                %f\n", first, RemoteControlSonarSettings->ping_period);
  fprintf(stderr, "%s     range_selection:            %f\n", first, RemoteControlSonarSettings->range_selection);
  fprintf(stderr, "%s     power_selection:            %f\n", first, RemoteControlSonarSettings->power_selection);
  fprintf(stderr, "%s     gain_selection:             %f\n", first, RemoteControlSonarSettings->gain_selection);
  fprintf(stderr, "%s     control_flags:              %d\n", first, RemoteControlSonarSettings->control_flags);
  fprintf(stderr, "%s     projector_id:               %d\n", first, RemoteControlSonarSettings->projector_id);
  fprintf(stderr, "%s     steering_vertical:          %f\n", first, RemoteControlSonarSettings->steering_vertical);
  fprintf(stderr, "%s     steering_horizontal:        %f\n", first, RemoteControlSonarSettings->steering_horizontal);
  fprintf(stderr, "%s     beamwidth_vertical:         %f\n", first, RemoteControlSonarSettings->beamwidth_vertical);
  fprintf(stderr, "%s     beamwidth_horizontal:       %f\n", first, RemoteControlSonarSettings->beamwidth_horizontal);
  fprintf(stderr, "%s     focal_point:                %f\n", first, RemoteControlSonarSettings->focal_point);
  fprintf(stderr, "%s     projector_weighting:        %d\n", first, RemoteControlSonarSettings->projector_weighting);
  fprintf(stderr, "%s     projector_weighting_par:    %f\n", first, RemoteControlSonarSettings->projector_weighting_par);
  fprintf(stderr, "%s     transmit_flags:             %d\n", first, RemoteControlSonarSettings->transmit_flags);
  fprintf(stderr, "%s     hydrophone_id:              %d\n", first, RemoteControlSonarSettings->hydrophone_id);
  fprintf(stderr, "%s     rx_weighting:               %d\n", first, RemoteControlSonarSettings->rx_weighting);
  fprintf(stderr, "%s     rx_weighting_par:           %f\n", first, RemoteControlSonarSettings->rx_weighting_par);
  fprintf(stderr, "%s     rx_flags:                   %d\n", first, RemoteControlSonarSettings->rx_flags);
  fprintf(stderr, "%s     range_minimum:              %f\n", first, RemoteControlSonarSettings->range_minimum);
  fprintf(stderr, "%s     range_maximum:              %f\n", first, RemoteControlSonarSettings->range_maximum);
  fprintf(stderr, "%s     depth_minimum:              %f\n", first, RemoteControlSonarSettings->depth_minimum);
  fprintf(stderr, "%s     depth_maximum:              %f\n", first, RemoteControlSonarSettings->depth_maximum);
  fprintf(stderr, "%s     absorption:                 %f\n", first, RemoteControlSonarSettings->absorption);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, RemoteControlSonarSettings->sound_velocity);
  fprintf(stderr, "%s     spreading:                  %f\n", first, RemoteControlSonarSettings->spreading);
  fprintf(stderr, "%s     vernier_operation_mode:     %u\n", first, RemoteControlSonarSettings->vernier_operation_mode);
  fprintf(stderr, "%s     autofilter_window:          %u\n", first, RemoteControlSonarSettings->autofilter_window);
  fprintf(stderr, "%s     tx_offset_x:                %f\n", first, RemoteControlSonarSettings->tx_offset_x);
  fprintf(stderr, "%s     tx_offset_y:                %f\n", first, RemoteControlSonarSettings->tx_offset_y);
  fprintf(stderr, "%s     tx_offset_z:                %f\n", first, RemoteControlSonarSettings->tx_offset_z);
  fprintf(stderr, "%s     head_tilt_x:                %f\n", first, RemoteControlSonarSettings->head_tilt_x);
  fprintf(stderr, "%s     head_tilt_y:                %f\n", first, RemoteControlSonarSettings->head_tilt_y);
  fprintf(stderr, "%s     head_tilt_z:                %f\n", first, RemoteControlSonarSettings->head_tilt_z);
  fprintf(stderr, "%s     ping_state:                 %d\n", first, RemoteControlSonarSettings->ping_state);
  fprintf(stderr, "%s     beam_angle_mode:            %d\n", first, RemoteControlSonarSettings->beam_angle_mode);
  fprintf(stderr, "%s     s7kcenter_mode:             %d\n", first, RemoteControlSonarSettings->s7kcenter_mode);
  fprintf(stderr, "%s     gate_depth_min:             %f\n", first, RemoteControlSonarSettings->gate_depth_min);
  fprintf(stderr, "%s     gate_depth_max:             %f\n", first, RemoteControlSonarSettings->gate_depth_max);
  fprintf(stderr, "%s     trigger_width:              %f\n", first, RemoteControlSonarSettings->trigger_width);
  fprintf(stderr, "%s     trigger_offset:             %f\n", first, RemoteControlSonarSettings->trigger_offset);
  fprintf(stderr, "%s     projector_selection:        %d\n", first, RemoteControlSonarSettings->projector_selection);
  for (int i = 0; i < 2; i++)
    fprintf(stderr, "%s     reserved2[%d]:               %d\n", first, i, RemoteControlSonarSettings->reserved2[i]);
  fprintf(stderr, "%s     alternate_gain:             %f\n", first, RemoteControlSonarSettings->alternate_gain);
  fprintf(stderr, "%s     vernier_filter:             %u\n", first, RemoteControlSonarSettings->vernier_filter);
  fprintf(stderr, "%s     reserved3:                  %u\n", first, RemoteControlSonarSettings->reserved3);
  fprintf(stderr, "%s     custom_beams:               %d\n", first, RemoteControlSonarSettings->custom_beams);
  fprintf(stderr, "%s     coverage_angle:             %f\n", first, RemoteControlSonarSettings->coverage_angle);
  fprintf(stderr, "%s     coverage_mode:              %u\n", first, RemoteControlSonarSettings->coverage_mode);
  fprintf(stderr, "%s     quality_filter:             %u\n", first, RemoteControlSonarSettings->quality_filter);
  fprintf(stderr, "%s     received_steering:          %f\n", first, RemoteControlSonarSettings->received_steering);
  fprintf(stderr, "%s     flexmode_coverage:          %f\n", first, RemoteControlSonarSettings->flexmode_coverage);
  fprintf(stderr, "%s     flexmode_steering:          %f\n", first, RemoteControlSonarSettings->flexmode_steering);
  fprintf(stderr, "%s     constant_spacing:           %f\n", first, RemoteControlSonarSettings->constant_spacing);
  fprintf(stderr, "%s     beam_mode:                  %d\n", first, RemoteControlSonarSettings->beam_mode);
  fprintf(stderr, "%s     depth_gate_tilt:            %f\n", first, RemoteControlSonarSettings->depth_gate_tilt);
  fprintf(stderr, "%s     applied_frequency:          %f\n", first, RemoteControlSonarSettings->applied_frequency);
  fprintf(stderr, "%s     element_number:             %d\n", first, RemoteControlSonarSettings->element_number);

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

int mbsys_reson7k3_print_CommonSystemSettings(int verbose, s7k3_CommonSystemSettings *CommonSystemSettings, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       CommonSystemSettings:  %p\n", (void *)CommonSystemSettings);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &CommonSystemSettings->header, error);

  /* print Reson 7k Common System Settings (Record 7504) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     serial_number:              %llu\n", first, (long long unsigned) CommonSystemSettings->serial_number);
  fprintf(stderr, "%s     ping_number:                %u\n", first, CommonSystemSettings->ping_number);
  fprintf(stderr, "%s     sound_velocity:             %f\n", first, CommonSystemSettings->sound_velocity);
  fprintf(stderr, "%s     absorption:                 %f\n", first, CommonSystemSettings->absorption);
  fprintf(stderr, "%s     spreading_loss:             %f\n", first, CommonSystemSettings->spreading_loss);
  fprintf(stderr, "%s     sequencer_control:          %u\n", first, CommonSystemSettings->sequencer_control);
  fprintf(stderr, "%s     mru_format:                 %u\n", first, CommonSystemSettings->mru_format);
  fprintf(stderr, "%s     mru_baudrate:               %u\n", first, CommonSystemSettings->mru_baudrate);
  fprintf(stderr, "%s     mru_parity:                 %u\n", first, CommonSystemSettings->mru_parity);
  fprintf(stderr, "%s     mru_databits:               %u\n", first, CommonSystemSettings->mru_databits);
  fprintf(stderr, "%s     mru_stopbits:               %u\n", first, CommonSystemSettings->mru_stopbits);
  fprintf(stderr, "%s     orientation:                %u\n", first, CommonSystemSettings->orientation);
  fprintf(stderr, "%s     record_version:             %u\n", first, CommonSystemSettings->record_version);
  fprintf(stderr, "%s     motion_latency:             %f\n", first, CommonSystemSettings->motion_latency);
  fprintf(stderr, "%s     svp_filter:                 %u\n", first, CommonSystemSettings->svp_filter);
  fprintf(stderr, "%s     sv_override:                %u\n", first, CommonSystemSettings->sv_override);
  fprintf(stderr, "%s     activeenum:                 %u\n", first, CommonSystemSettings->activeenum);
  fprintf(stderr, "%s     active_id:                  %u\n", first, CommonSystemSettings->active_id);
  fprintf(stderr, "%s     system_mode:                %u\n", first, CommonSystemSettings->system_mode);
  fprintf(stderr, "%s     masterslave_mode:           %u\n", first, CommonSystemSettings->masterslave_mode);
  fprintf(stderr, "%s     tracker_flags:              %u\n", first, CommonSystemSettings->tracker_flags);
  fprintf(stderr, "%s     tracker_swathwidth:         %f\n", first, CommonSystemSettings->tracker_swathwidth);
  fprintf(stderr, "%s     multidetect_enable:         %u\n", first, CommonSystemSettings->multidetect_enable);
  fprintf(stderr, "%s     multidetect_obsize:         %u\n", first, CommonSystemSettings->multidetect_obsize);
  fprintf(stderr, "%s     multidetect_sensitivity:    %u\n", first, CommonSystemSettings->multidetect_sensitivity);
  fprintf(stderr, "%s     multidetect_detections:     %u\n", first, CommonSystemSettings->multidetect_detections);
  for (int i = 0;i<2;i++){
    fprintf(stderr, "%s     multidetect_reserved[%d]:    %u\n", first, i, CommonSystemSettings->multidetect_reserved[i]);
  }
  for (int i = 0;i<4;i++){
    fprintf(stderr, "%s     slave_ip[%d]:                %u\n", first, i, CommonSystemSettings->slave_ip[i]);
  }
  fprintf(stderr, "%s     snippet_controlflags:       %u\n", first, CommonSystemSettings->snippet_controlflags);
  fprintf(stderr, "%s     snippet_minwindow:          %u\n", first, CommonSystemSettings->snippet_minwindow);
  fprintf(stderr, "%s     snippet_maxwindow:          %u\n", first, CommonSystemSettings->snippet_maxwindow);
  fprintf(stderr, "%s     fullrange_dualhead:         %u\n", first, CommonSystemSettings->fullrange_dualhead);
  fprintf(stderr, "%s     delay_multiplier:           %f\n", first, CommonSystemSettings->delay_multiplier);
  fprintf(stderr, "%s     powersaving_mode:           %u\n", first, CommonSystemSettings->powersaving_mode);
  fprintf(stderr, "%s     flags:                      %u\n", first, CommonSystemSettings->flags);
  fprintf(stderr, "%s     range_blank:                %u\n", first, CommonSystemSettings->range_blank);
  fprintf(stderr, "%s     startup_normalization:      %u\n", first, CommonSystemSettings->startup_normalization);
  fprintf(stderr, "%s     restore_pingrate:           %u\n", first, CommonSystemSettings->restore_pingrate);
  fprintf(stderr, "%s     restore_power:              %u\n", first, CommonSystemSettings->restore_power);
  fprintf(stderr, "%s     sv_interlock:               %u\n", first, CommonSystemSettings->sv_interlock);
  fprintf(stderr, "%s     ignorepps_errors:           %u\n", first, CommonSystemSettings->ignorepps_errors);
  for (int i = 0;i<15;i++){
    fprintf(stderr, "%s     reserved1[%d]:              %u\n", first, i, CommonSystemSettings->reserved1[i]);
  }
  fprintf(stderr, "%s     compressed_wcflags:         %u\n", first, CommonSystemSettings->compressed_wcflags);
  fprintf(stderr, "%s     deckmode:                   %u\n", first, CommonSystemSettings->deckmode);
  fprintf(stderr, "%s     reserved2:                  %u\n", first, CommonSystemSettings->reserved2);
  fprintf(stderr, "%s     powermode_flags:            %u\n", first, CommonSystemSettings->powermode_flags);
  fprintf(stderr, "%s     powermode_max:              %u\n", first, CommonSystemSettings->powermode_max);
  fprintf(stderr, "%s     water_temperature:          %f\n", first, CommonSystemSettings->water_temperature);
  fprintf(stderr, "%s     sensor_override:            %u\n", first, CommonSystemSettings->sensor_override);
  fprintf(stderr, "%s     sensor_dataflags:           %u\n", first, CommonSystemSettings->sensor_dataflags);
  fprintf(stderr, "%s     sensor_active:              %u\n", first, CommonSystemSettings->sensor_active);
  fprintf(stderr, "%s     reserved3:                  %u\n", first, CommonSystemSettings->reserved3);
  fprintf(stderr, "%s     tracker_maxcoverage:        %f\n", first, CommonSystemSettings->tracker_maxcoverage);
  fprintf(stderr, "%s     dutycycle_mode:             %u\n", first, CommonSystemSettings->dutycycle_mode);
  fprintf(stderr, "%s     reserved4:                  %u\n", first, CommonSystemSettings->reserved4);
  for (int i = 0;i<99;i++){
    fprintf(stderr, "%s     reserved5[%d]:              %u\n", first, i, CommonSystemSettings->reserved5[i]);
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

int mbsys_reson7k3_print_SVFiltering(int verbose, s7k3_SVFiltering *SVFiltering, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SVFiltering:       %p\n", (void *)SVFiltering);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SVFiltering->header, error);

  /* print Reson 7k Sound Velocity (record 7610) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     sensor_sv:                %f\n", first, SVFiltering->sensor_sv);
  fprintf(stderr, "%s     filtered_sv:              %f\n", first, SVFiltering->filtered_sv);
  fprintf(stderr, "%s     filter:                   %u\n", first, SVFiltering->filter);

  /* Optional data */

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

int mbsys_reson7k3_print_SystemLockStatus(int verbose, s7k3_SystemLockStatus *SystemLockStatus, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SystemLockStatus:  %p\n", (void *)SystemLockStatus);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SystemLockStatus->header, error);

  /* print Reson 7k Sound Velocity (record 7610) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     systemlock:                    %u\n", first, SystemLockStatus->systemlock);
  fprintf(stderr, "%s     client_ip:                     %u\n", first, SystemLockStatus->client_ip);
  for (int i = 0; i < 8; i++) {
    fprintf(stderr, "%s     reserved[%d]:                   %u\n", first, i, SystemLockStatus->reserved[i]);
  }

  /* Optional data */

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
int mbsys_reson7k3_print_SoundVelocity(int verbose, s7k3_SoundVelocity *SoundVelocity, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SoundVelocity:     %p\n", (void *)SoundVelocity);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SoundVelocity->header, error);

  /* print Reson 7k Sound Velocity (record 7610) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     SoundVelocity:              %f\n", first, SoundVelocity->soundvelocity);

  /* Optional data */

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
int mbsys_reson7k3_print_AbsorptionLoss(int verbose, s7k3_AbsorptionLoss *AbsorptionLoss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       AbsorptionLoss:    %p\n", (void *)AbsorptionLoss);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &AbsorptionLoss->header, error);

  /* print Reson 7k Absorption Loss (record 7611) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     absorptionloss:             %f\n", first, AbsorptionLoss->absorptionloss);

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
int mbsys_reson7k3_print_SpreadingLoss(int verbose, s7k3_SpreadingLoss *SpreadingLoss, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       SpreadingLoss:     %p\n", (void *)SpreadingLoss);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &SpreadingLoss->header, error);

  /* print Reson 7k Spreading Loss (record 7612) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     SpreadingLoss:              %f\n", first, SpreadingLoss->spreadingloss);

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
int mbsys_reson7k3_print_ProfileAverageSalinity(int verbose, s7k3_ProfileAverageSalinity *ProfileAverageSalinity, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:                  %d\n", verbose);
    fprintf(stderr, "dbg2       ProfileAverageSalinity:   %p\n", (void *)ProfileAverageSalinity);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &ProfileAverageSalinity->header, error);

  /* print Reson 7k Spreading Loss (record 7612) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     salinity:              %f\n", first, ProfileAverageSalinity->salinity);

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
int mbsys_reson7k3_print_ProfileAverageTemperature(int verbose, s7k3_ProfileAverageTemperature *ProfileAverageTemperature, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:           %d\n", verbose);
    fprintf(stderr, "dbg2       ProfileAverageTemperature:     %p\n", (void *)ProfileAverageTemperature);
  }

  /* print Reson 7k data record header information */
  mbsys_reson7k3_print_header(verbose, &ProfileAverageTemperature->header, error);

  /* print Reson 7k Spreading Loss (record 7612) */
  const char *first;
  if (verbose >= 2)
    first = debug_str;
  else {
    first = nodebug_str;
    fprintf(stderr, "\n%sMBIO function <%s> called\n", first, __func__);
  }
  fprintf(stderr, "%sStructure Contents:\n", first);
  fprintf(stderr, "%s     temperature:            %f\n", first, ProfileAverageTemperature->temperature);

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
int mbsys_reson7k3_dimensions(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbath, int *namp, int *nss,
                             int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    /* get beam and pixel numbers */
    if (store->read_RawDetection) {
      s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
      *nbath = RawDetection->number_beams;
      *namp = *nbath;
      *nss = 0;
    }
    else if (store->read_SegmentedRawDetection) {
      s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;
      *nbath = SegmentedRawDetection->n_rx;
      *namp = *nbath;
      *nss = 0;
    }
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
int mbsys_reson7k3_pingnumber(int verbose, void *mbio_ptr, unsigned int *pingnumber, int *error) {
  s7k3_RawDetection *RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
  }

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)mb_io_ptr->store_data;

  /* extract data from structure */
  if (store->read_RawDetection) {
    RawDetection = (s7k3_RawDetection *)&store->RawDetection;
    *pingnumber = RawDetection->ping_number;
  }
  else if (store->read_SegmentedRawDetection) {
    SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;
    *pingnumber = SegmentedRawDetection->ping_number;
  }

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
int mbsys_reson7k3_sonartype(int verbose, void *mbio_ptr, void *store_ptr, int *sonartype, int *error) {
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
int mbsys_reson7k3_sidescantype(int verbose, void *mbio_ptr, void *store_ptr, int *ss_type, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get SideScan type */
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
int mbsys_reson7k3_preprocess(int verbose,     /* in: verbosity level set on command line 0..N */
                             void *mbio_ptr,  /* in: see mb_io.h:/^struct mb_io_struct/ */
                             void *store_ptr, /* in: see mbsys_reson7k.h:/^struct mbsys_reson7k3_struct/ */
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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  struct mb_platform_struct *platform = (struct mb_platform_struct *)platform_ptr;

  /* get saved values */
  double *pixel_size = (double *)&mb_io_ptr->saved1;
  double *swath_width = (double *)&mb_io_ptr->saved2;

  /* kluge parameters */
  double kluge_beampatternsnellfactor = 1.0;
  double kluge_soundspeedsnellfactor = 1.0;
  bool kluge_beampatternsnell = false;
  bool kluge_soundspeedsnell = false;
  bool kluge_zeroAttitudecorrection = false;
  bool kluge_zeroalongtrackangles = false;

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
      kluge_zeroAttitudecorrection = true;
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
        fprintf(stderr, "dbg2       kluge_zeroAttitudecorrection:  %d\n", kluge_zeroAttitudecorrection);
      }
      else if (pars->kluge_id[i] == MB_PR_KLUGE_ZEROALONGTRACKANGLES) {
        fprintf(stderr, "dbg2       kluge_zeroalongtrackangles:    %d\n", kluge_zeroalongtrackangles);
      }
    }
  }

  int status = MB_SUCCESS;

  s7k3_header *header = NULL;
  s7k3_SonarSettings *SonarSettings = NULL;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata = NULL;
  s7k3_bathydata *bathydata = NULL;
  s7k3_rawdetectiondata *rawdetectiondata = NULL;
  s7k3_VerticalDepth *VerticalDepth = NULL;

  /* variables for beam angle calculation */
  mb_3D_orientation tx_align;
  mb_3D_orientation tx_orientation;
  double tx_steer;
  mb_3D_orientation rx_align;
  mb_3D_orientation rx_orientation;
  double rx_steer;
  double reference_heading;
  double beamAzimuth;
  double beamDepression;

  s7k3_time s7kTime;
  int time_i[7];
  int time_j[5];
  double time_d = 0.0;
  double navlon = 0.0;
  double navlat = 0.0;
  double speed = 0.0;
  double altitude = 0.0;
  double sensordepth = 0.0;
  double heading = 0.0;
  double beamheading;
  double roll = 0.0;
  double beamroll;
  double pitch = 0.0;
  double beampitch;
  double heave = 0.0;
  double soundspeed;
  double soundspeednew;
  double soundspeedsnellfactor = 1.0;
  double mtodeglon, mtodeglat;
  double dx, dy, dt;
  int jnav = 0;
  int jsensordepth = 0;
  int jheading = 0;
  int jaltitude = 0;
  int jAttitude = 0;
  int jsoundspeed = 0;
  int interp_error = MB_ERROR_NO_ERROR;
  mb_u_char *qualitycharptr;
  mb_u_char beamflag;
  double ttime;

  /* if called with store_ptr == NULL then called after mb_read_init() but before
      any data are read - for some formats this allows kluge options to set special
      reading conditions/behaviors */
  if (store_ptr == NULL) {

  }

  /* deal with a survey record */
  else if (store->kind == MB_DATA_DATA) {
    s7k3_Navigation *Navigation = &(store->Navigation);
    SonarSettings = &(store->SonarSettings);
    s7k3_MatchFilter *MatchFilter = &(store->MatchFilter);
    s7k3_BeamGeometry *BeamGeometry = &(store->BeamGeometry);
    s7k3_SideScan *SideScan = &(store->SideScan);
    VerticalDepth = &(store->VerticalDepth);
    s7k3_TVG *TVG = &(store->TVG);
    s7k3_Image *Image = &(store->Image);
    s7k3_PingMotion *PingMotion = &(store->PingMotion);
    s7k3_Beamformed *Beamformed = &(store->Beamformed);
    s7k3_VernierProcessingDataRaw *VernierProcessingDataRaw = &(store->VernierProcessingDataRaw);
    s7k3_RawDetection *RawDetection = &(store->RawDetection);
    s7k3_Snippet *Snippet = &(store->Snippet);
    s7k3_VernierProcessingDataFiltered *VernierProcessingDataFiltered =
        &(store->VernierProcessingDataFiltered);
    s7k3_CompressedBeamformedMagnitude *CompressedBeamformedMagnitude =
        &(store->CompressedBeamformedMagnitude);
    s7k3_CompressedWaterColumn *CompressedWaterColumn =
        &(store->CompressedWaterColumn);
    s7k3_SegmentedRawDetection *SegmentedRawDetection = &(store->SegmentedRawDetection);
    s7k3_CalibratedBeam *CalibratedBeam = &(store->CalibratedBeam);
    s7k3_CalibratedSideScan *CalibratedSideScan = &(store->CalibratedSideScan);
    s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength =
        &(store->SnippetBackscatteringStrength);
    // s7k3_FileHeader *FileHeader = &(store->FileHeader);
    // s7k3_InstallationParameters *InstallationParameters = &(store->InstallationParameters);
    s7k3_RemoteControlSonarSettings *RemoteControlSonarSettings = &(store->RemoteControlSonarSettings);
    // s7k3_CommonSystemSettings *CommonSystemSettings = &(store->CommonSystemSettings);

    /* print out record headers */
    if (store->read_SonarSettings) {
      header = &(SonarSettings->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_SonarSettings:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], SonarSettings->ping_number);
    }
    if (store->read_MatchFilter) {
      header = &(MatchFilter->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_MatchFilter:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], MatchFilter->ping_number);
    }
    if (store->read_BeamGeometry) {
      header = &(BeamGeometry->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_BeamGeometry:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], BeamGeometry->number_beams);
    }
    if (store->read_SideScan) {
      header = &(SideScan->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_SideScan:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], SideScan->ping_number,
                SideScan->number_beams);
    }
    if (store->read_VerticalDepth) {
      header = &(VerticalDepth->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_VerticalDepth:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], VerticalDepth->ping_number);
    }
    if (store->read_TVG) {
      header = &(TVG->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_TVG:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], TVG->ping_number);
    }
    if (store->read_Image) {
      header = &(Image->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_Image:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "image w x h: %d x %d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], Image->ping_number,
                Image->width, Image->height);
    }
    if (store->read_PingMotion) {
      header = &(PingMotion->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_PingMotion:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], PingMotion->ping_number);
    }
    if (store->read_Beamformed) {
      header = &(Beamformed->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_Beamformed:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], Beamformed->ping_number,
                Beamformed->number_beams);
    }
    if (store->read_VernierProcessingDataRaw) {
      header = &(VernierProcessingDataRaw->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_VernierProcessingDataRaw:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], VernierProcessingDataRaw->ping_number);
    }
    if (store->read_RawDetection) {
      header = &(RawDetection->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_RawDetection:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], RawDetection->ping_number,
                RawDetection->number_beams);
    }

    if (store->read_Snippet) {
      header = &(Snippet->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_Snippet:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], Snippet->ping_number,
                Snippet->number_beams);
    }
    if (store->read_VernierProcessingDataFiltered) {
      header = &(VernierProcessingDataFiltered->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_VernierProcessingDataFiltered:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "soundings:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], VernierProcessingDataFiltered->ping_number,
                VernierProcessingDataFiltered->number_soundings);
    }
    if (store->read_CompressedBeamformedMagnitude) {
      header = &(CompressedBeamformedMagnitude->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_CompressedBeamformedMagnitude:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], CompressedBeamformedMagnitude->ping_number,
                CompressedBeamformedMagnitude->number_beams);
    }
    if (store->read_CompressedWaterColumn) {
      header = &(CompressedWaterColumn->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_CompressedWaterColumn:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], CompressedWaterColumn->ping_number,
                CompressedWaterColumn->number_beams);
    }
    if (store->read_SegmentedRawDetection) {
      header = &(SegmentedRawDetection->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_SegmentedRawDetection:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "n_segments:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], SegmentedRawDetection->ping_number,
                SegmentedRawDetection->n_segments);
    }
    if (store->read_CalibratedBeam) {
      header = &(CalibratedBeam->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_CalibratedBeam:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], CalibratedBeam->ping_number,
                CalibratedBeam->total_beams);
    }
    if (store->read_CalibratedSideScan) {
      header = &(CalibratedSideScan->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_CalibratedSideScan:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], CalibratedSideScan->ping_number);
    }
    if (store->read_SnippetBackscatteringStrength) {
      header = &(SnippetBackscatteringStrength->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_SnippetBackscatteringStrength:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d "
                "beams:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], SnippetBackscatteringStrength->ping_number,
                SnippetBackscatteringStrength->number_beams);
    }
    if (store->read_RemoteControlSonarSettings) {
      header = &(RemoteControlSonarSettings->header);
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, &time_d);
      if (verbose > 1)
        fprintf(stderr,
                "R7KRECID_RemoteControlSonarSettings:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping_number:%d\n",
                time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], RemoteControlSonarSettings->ping_number);
    }

    /* if requested ignore water column data
     * (will not be included in any output file) */
    if (pars->ignore_water_column) {
      store->read_WaterColumn = false;
      store->read_Image = false;
      store->read_Beamformed = false;
      store->read_CompressedBeamformedMagnitude = false;
      store->read_CompressedWaterColumn = false;
    }

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
              "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d | ping_number:%d\n",
              __func__, time_i[0], time_i[1], time_i[2], time_i[3],
              time_i[4], time_i[5], time_i[6], RawDetection->ping_number);

      /* apply the timestamp to all of the relevant data records */
      if (store->read_SonarSettings)
        store->SonarSettings.header.s7kTime = s7kTime;
      if (store->read_MatchFilter)
        store->MatchFilter.header.s7kTime = s7kTime;
      if (store->read_BeamGeometry)
        store->BeamGeometry.header.s7kTime = s7kTime;
      if (store->read_SideScan)
        store->SideScan.header.s7kTime = s7kTime;
      if (store->read_VerticalDepth)
        store->VerticalDepth.header.s7kTime = s7kTime;
      if (store->read_TVG)
        store->TVG.header.s7kTime = s7kTime;
      if (store->read_Image)
        store->Image.header.s7kTime = s7kTime;
      if (store->read_PingMotion)
        store->PingMotion.header.s7kTime = s7kTime;
      if (store->read_Beamformed)
        store->Beamformed.header.s7kTime = s7kTime;
      if (store->read_VernierProcessingDataRaw)
        store->VernierProcessingDataRaw.header.s7kTime = s7kTime;
      if (store->read_RawDetection)
        store->RawDetection.header.s7kTime = s7kTime;
      if (store->read_Snippet)
        store->Snippet.header.s7kTime = s7kTime;
      if (store->read_VernierProcessingDataFiltered)
        store->VernierProcessingDataFiltered.header.s7kTime = s7kTime;
      if (store->read_CompressedBeamformedMagnitude)
        store->CompressedBeamformedMagnitude.header.s7kTime = s7kTime;
      if (store->read_CompressedWaterColumn)
        store->CompressedWaterColumn.header.s7kTime = s7kTime;
      if (store->read_SegmentedRawDetection)
        store->SegmentedRawDetection.header.s7kTime = s7kTime;
      if (store->read_CalibratedBeam)
        store->CalibratedBeam.header.s7kTime = s7kTime;
      if (store->read_CalibratedSideScan)
        store->CalibratedSideScan.header.s7kTime = s7kTime;
      if (store->read_SnippetBackscatteringStrength)
        store->SnippetBackscatteringStrength.header.s7kTime = s7kTime;
      if (store->read_RemoteControlSonarSettings)
        store->RemoteControlSonarSettings.header.s7kTime = s7kTime;
    }

    /*--------------------------------------------------------------*/
    /* interpolate ancillary values  */
    /*--------------------------------------------------------------*/
bool dprint = false;
if (store->time_i[2]== 12 && store->time_i[3]==16 && store->time_i[4]==0 && store->time_i[5]==17)
	dprint = true;
if (store->time_i[2]== 12 && store->time_i[3]==16 && store->time_i[4]==3 && store->time_i[5]==45)
	dprint = true;

    int interp_status = mb_linear_interp_longitude(verbose, pars->nav_time_d - 1, pars->nav_lon - 1, pars->n_nav, time_d,
                                               &navlon, &jnav, &interp_error);
    interp_status &= mb_linear_interp_latitude(verbose, pars->nav_time_d - 1, pars->nav_lat - 1, pars->n_nav, time_d,
                                              &navlat, &jnav, &interp_error);
    if (pars->nav_speed != NULL) {
      interp_status &= mb_linear_interp(verbose, pars->nav_time_d - 1, pars->nav_speed - 1, pars->n_nav, time_d, &speed,
                                     &jnav, &interp_error);
    } else if (Navigation->speed > 0.0) {
      speed = 3.6 * Navigation->speed;
    } else {
      speed = 0.0;
    }

    /* interpolate sensordepth */
    interp_status &= mb_linear_interp(verbose, pars->sensordepth_time_d - 1, pars->sensordepth_sensordepth - 1,
                                     pars->n_sensordepth, time_d, &sensordepth, &jsensordepth, &interp_error);

    /* interpolate heading */
    interp_status &= mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                             pars->n_heading, time_d, &heading, &jheading, &interp_error);

    /* interpolate altitude */
    if (pars->n_altitude > 0) {
      interp_status &= mb_linear_interp(verbose, pars->altitude_time_d - 1, pars->altitude_altitude - 1, pars->n_altitude,
                                        time_d, &altitude, &jaltitude, &interp_error);
    }

    /* interpolate Attitude */
    interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                                     time_d, &roll, &jAttitude, &interp_error);
    interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                                     time_d, &pitch, &jAttitude, &interp_error);
    interp_status &= mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_heave - 1, pars->n_attitude,
                                     time_d, &heave, &jAttitude, &interp_error);

    /* interpolate soundspeed */
    if (pars->modify_soundspeed || kluge_soundspeedsnell) {
      interp_status &= mb_linear_interp(verbose, pars->soundspeed_time_d - 1, pars->soundspeed_soundspeed - 1, pars->n_soundspeed,
                                     time_d, &soundspeednew, &jsoundspeed, &interp_error);
    }

    /* do lever arm correction */
    if (platform != NULL) {
      /* calculate sonar Position */
      status = mb_platform_position(verbose, (void *)platform, pars->target_sensor, 0, navlon, navlat, sensordepth,
                                    heading, roll, pitch, &navlon, &navlat, &sensordepth, error);

      /* calculate sonar Attitude */
      status = mb_platform_orientation_target(verbose, (void *)platform, pars->target_sensor, 0, heading, roll, pitch,
                                              &heading, &roll, &pitch, error);
    }

    /* get local translation between lon lat degrees and meters */
    mb_coor_scale(verbose, navlat, &mtodeglon, &mtodeglat);
    // const double headingx = sin(DTR * heading);
    // const double headingy = cos(DTR * heading);

    /* if a valid speed is not available calculate it */
    if (interp_status == MB_SUCCESS && speed <= 0.0 && jnav > 0) {
      int j1;
      int j2;
      if (jnav > 1) {
        j1 = jnav - 2;
        j2 = jnav - 1;
      }
      else if (jnav == 1){
        j1 = jnav - 1;
        j2 = jnav;
      } else {
        assert(false);
      }
      dx = (pars->nav_lon[j2] - pars->nav_lon[j1]) / mtodeglon;
      dy = (pars->nav_lat[j2] - pars->nav_lat[j1]) / mtodeglat;
      dt = (pars->nav_time_d[j2] - pars->nav_time_d[j1]);
      if (dt > 0.0)
        speed = sqrt(dx * dx + dy * dy) / dt;
    }

    /* if the optional data are not all available, this ping
        is not useful, and is discarded by setting
        *error to MB_ERROR_MISSING_NAVATTITUDE */
    if (interp_status == MB_FAILURE) {
      status = MB_FAILURE;
      *error = MB_ERROR_MISSING_NAVATTITUDE;
    }

    /*--------------------------------------------------------------*/
    /* recalculate Bathymetry  */
    /*--------------------------------------------------------------*/
    if ((store->read_RawDetection && !RawDetection->optionaldata)
        || (store->read_SegmentedRawDetection && !SegmentedRawDetection->optionaldata)
        || pars->recalculate_bathymetry) {

      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2 Recalculating Bathymetry in %s: 7k ping records read:\n", __func__);
        fprintf(stderr, "dbg2      read_ProcessedSideScan:              %d\n",
                store->read_ProcessedSideScan);
        fprintf(stderr, "dbg2      read_SonarSettings:                  %d\n",
                store->read_SonarSettings);
        fprintf(stderr, "dbg2      read_MatchFilter:                    %d\n",
                store->read_MatchFilter);
        fprintf(stderr, "dbg2      read_BeamGeometry:                   %d\n",
                store->read_BeamGeometry);
        fprintf(stderr, "dbg2      read_Bathymetry:                     %d optionaldata:%d\n",
                store->read_Bathymetry, store->Bathymetry.optionaldata);
        fprintf(stderr, "dbg2      read_SideScan:                       %d optionaldata:%d\n",
                store->read_SideScan, store->SideScan.optionaldata);
        fprintf(stderr, "dbg2      read_WaterColumn:                    %d\n",
                store->read_WaterColumn);
        fprintf(stderr, "dbg2      read_VerticalDepth:                  %d\n",
                store->read_VerticalDepth);
        fprintf(stderr, "dbg2      read_TVG:                            %d\n",
                store->read_TVG);
        fprintf(stderr, "dbg2      read_Image:                          %d\n",
                store->read_Image);
        fprintf(stderr, "dbg2      read_PingMotion:                     %d\n",
                store->read_PingMotion);
        fprintf(stderr, "dbg2      read_DetectionDataSetup:             %d\n",
                store->read_DetectionDataSetup);
        fprintf(stderr, "dbg2      read_Beamformed:                     %d\n",
                store->read_Beamformed);
        fprintf(stderr, "dbg2      read_VernierProcessingDataRaw:       %d\n",
                store->read_VernierProcessingDataRaw);
        fprintf(stderr, "dbg2      read_RawDetection:                   %d optionaldata:%d\n",
                store->read_RawDetection, store->RawDetection.optionaldata);
        fprintf(stderr, "dbg2      read_Snippet:                        %d optionaldata:%d\n",
                store->read_Snippet, store->Snippet.optionaldata);
        fprintf(stderr, "dbg2      read_VernierProcessingDataFiltered:  %d\n",
                store->read_VernierProcessingDataFiltered);
        fprintf(stderr, "dbg2      read_CompressedBeamformedMagnitude:  %d\n",
                store->read_CompressedBeamformedMagnitude);
        fprintf(stderr, "dbg2      read_CompressedWaterColumn:          %d\n",
                store->read_CompressedWaterColumn);
        fprintf(stderr, "dbg2      read_SegmentedRawDetection:          %d optionaldata:%d\n",
                store->read_SegmentedRawDetection, store->SegmentedRawDetection.optionaldata);
        fprintf(stderr, "dbg2      read_CalibratedBeam:                 %d\n",
                store->read_CalibratedBeam);
        fprintf(stderr, "dbg2      read_CalibratedSideScan:             %d optionaldata:%d\n",
                store->read_CalibratedSideScan, store->CalibratedSideScan.optionaldata);
        fprintf(stderr, "dbg2      read_SnippetBackscatteringStrength:  %d optionaldata:%d\n",
                store->read_SnippetBackscatteringStrength, store->SnippetBackscatteringStrength.optionaldata);
        fprintf(stderr, "dbg2      read_RemoteControlSonarSettings:     %d\n",
                store->read_RemoteControlSonarSettings);
      }

      /* Deal with RawDetection record case */

      /* initialize all of the beams */
      if (store->read_RawDetection) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);
          qualitycharptr = (mb_u_char *)&(rawdetectiondata->quality);
          qualitycharptr[3] = 0;
          bathydata->depth = 0.0;
          bathydata->acrosstrack = 0.0;
          bathydata->alongtrack = 0.0;
          bathydata->pointing_angle = 0.0;
          bathydata->azimuth_angle = 0.0;
        }
      } else if (store->read_SegmentedRawDetection) {
        for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);
          qualitycharptr = (mb_u_char *)&(segmentedrawdetectionrxdata->quality);
          qualitycharptr[3] = 0;
          bathydata->depth = 0.0;
          bathydata->acrosstrack = 0.0;
          bathydata->alongtrack = 0.0;
          bathydata->pointing_angle = 0.0;
          bathydata->azimuth_angle = 0.0;
        }
      }

      /* set ping values */
      if (store->read_RawDetection) {
        RawDetection->frequency = SonarSettings->frequency;
        RawDetection->longitude = DTR * navlon;
        RawDetection->latitude = DTR * navlat;
        RawDetection->heading = DTR * heading;
        RawDetection->height_source = 1;
        RawDetection->tide = 0.0;
        RawDetection->roll = DTR * roll;
        RawDetection->pitch = DTR * pitch;
        RawDetection->heave = heave;
        if ((SonarSettings->rx_flags & 0x2) != 0) {
          RawDetection->vehicle_depth = sensordepth + heave;
        }
        else {
          RawDetection->vehicle_depth = sensordepth;
        }
      } else if (store->read_SegmentedRawDetection) {
        SegmentedRawDetection->frequency = SonarSettings->frequency;
        SegmentedRawDetection->longitude = DTR * navlon;
        SegmentedRawDetection->latitude = DTR * navlat;
        SegmentedRawDetection->heading = DTR * heading;
        SegmentedRawDetection->height_source = 1;
        SegmentedRawDetection->tide = 0.0;
        SegmentedRawDetection->roll = DTR * roll;
        SegmentedRawDetection->pitch = DTR * pitch;
        SegmentedRawDetection->heave = heave;
        if ((SonarSettings->rx_flags & 0x2) != 0) {
          SegmentedRawDetection->vehicle_depth = sensordepth + heave;
        }
        else {
          SegmentedRawDetection->vehicle_depth = sensordepth;
        }
      }

      /* get ready to calculate Bathymetry */
      if (SonarSettings->sound_velocity > 0.0)
        soundspeed = SonarSettings->sound_velocity;
      else
        soundspeed = 1500.0;
      // double rollr = DTR * roll;
      // double pitchr = DTR * pitch;

      /* zero atttitude correction if requested */
      // if (kluge_zeroAttitudecorrection) {
        // rollr = 0.0;
        // pitchr = 0.0;
      // }

      /* zero alongtrack angles if requested */
      if (kluge_zeroalongtrackangles) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          BeamGeometry->angle_alongtrack[i] = 0.0;
        }
      }

      /* if requested apply kluge scaling of rx beam angles */
      if (kluge_beampatternsnell) {
        /*
         * RawDetection record
         */
        if (store->read_RawDetection) {
          for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
            rawdetectiondata = &RawDetection->rawdetectiondata[i];
            rawdetectiondata->rx_angle
              = asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
                         * sin(rawdetectiondata->rx_angle))));
          }
        }
        else if (store->read_SegmentedRawDetection) {
          for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
            segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
            segmentedrawdetectionrxdata->rx_angle_cross
              = asin(MAX(-1.0, MIN(1.0, kluge_beampatternsnellfactor
                         * sin(segmentedrawdetectionrxdata->rx_angle_cross))));
          }
        }
      }

      /* Change the sound speed used to calculate Bathymetry */
      if (pars->modify_soundspeed) {
        soundspeedsnellfactor = soundspeednew / soundspeed;
        soundspeed = soundspeednew;
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

      if (pars->modify_soundspeed || kluge_soundspeedsnell) {
        /* change the sound speed recorded for the current ping and
         * then use it to alter the beam angles and recalculated the
         * Bathymetry
         */
        SonarSettings->sound_velocity = soundspeed;

        /*
         * RawDetection record
         */
        if (store->read_RawDetection) {
          for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
            rawdetectiondata = &RawDetection->rawdetectiondata[i];
            rawdetectiondata->rx_angle =
                asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                         * sin(rawdetectiondata->rx_angle))));
          }
        }
        else if (store->read_SegmentedRawDetection) {
          for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
            segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
            segmentedrawdetectionrxdata->rx_angle_cross =
                asin(MAX(-1.0, MIN(1.0, soundspeedsnellfactor
                         * sin(segmentedrawdetectionrxdata->rx_angle_cross))));
          }
        }
      }

      /* get transducer angular offsets */
      int tx_sign = 1.0;
      int rx_sign = 1.0;
      if (platform != NULL) {
        status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 0,
                                                &(tx_align.heading), &(tx_align.roll), &(tx_align.pitch), error);

        // handle reverse mounting of transmit array */
        if (tx_align.heading > 100.0 || tx_align.heading < -100.0) {
          tx_align.heading -= 180.0;
          if (tx_align.heading < 0.0)
            tx_align.heading += 360.0;
          //tx_align.heading *= -1;
          //tx_align.roll *= -1;
          //tx_align.pitch *= -1;
          tx_sign = -1.0;
        }

        status = mb_platform_orientation_offset(verbose, (void *)platform, pars->target_sensor, 1,
                                                &(rx_align.heading), &(rx_align.roll), &(rx_align.pitch), error);
        if (rx_align.heading > 100.0 || rx_align.heading < -100.0) {
          rx_align.heading -= 180.0;
          if (rx_align.heading < 0.0)
            rx_align.heading += 360.0;
          //rx_align.heading *= -1;
          //rx_align.roll *= -1;
          //rx_align.pitch *= -1;
          rx_sign = -1.0;
        }
      }

      /* calculate bathymetry from RawDetection record */
      if (store->read_RawDetection) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);

          /* get range */
          ttime = rawdetectiondata->detection_point / RawDetection->sampling_rate;

          /* set initial beamflag based on beam quality values */
          qualitycharptr = (mb_u_char *)&(rawdetectiondata->quality);

          // passed both quality metrics 0x01 and 0x02
          if ((qualitycharptr[0] & 0x03) == 0x03) {
            beamflag = MB_FLAG_NONE;
          }

          // multi-detect enabled, not primary pick, usually ignored
          else if ((RawDetection->flags & 0x10) && (rawdetectiondata->flags & 0x1E00)) {
            beamflag = MB_FLAG_FLAG + MB_FLAG_MULTIPICK;
          }

          // else primary pick but failed quality so treated as flagged by sonar
          else {
            beamflag = MB_FLAG_FLAG + MB_FLAG_SONAR;
          }
          qualitycharptr[3] = beamflag;

          /* get roll at bottom return time for this beam */
          /* interp_status &= */
              mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                               time_d + ttime, &beamroll, &jAttitude, error);
          // beamrollr = DTR * beamroll;

          /* get pitch at bottom return time for this beam */
          /* interp_status &= */
              mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                               time_d + ttime, &beampitch, &jAttitude, error);
          // const double beampitchr = DTR * beampitch;

          /* get heading at bottom return time for this beam */
          /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                   pars->n_heading, time_d + ttime, &beamheading,
                                                   &jheading, error);
          // beamheadingr = DTR * beamheading;

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
          tx_steer = RTD * tx_sign * RawDetection->tx_angle;
          tx_orientation.roll = roll;
          tx_orientation.pitch = pitch;
          tx_orientation.heading = heading;
          rx_steer = -RTD * rx_sign * RawDetection->rawdetectiondata[i].rx_angle;
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

          /* calculate Bathymetry */
          const double rr = 0.5 * soundspeed * ttime;
          const double xx = rr * sin(DTR * theta);
          const double zz = rr * cos(DTR * theta);
          bathydata->acrosstrack = xx * cos(DTR * phi);
          bathydata->alongtrack = xx * sin(DTR * phi);
          bathydata->depth = zz + sensordepth - heave;
          bathydata->pointing_angle = DTR * theta;
          bathydata->azimuth_angle = DTR * beamAzimuth;
if (dprint && i == RawDetection->number_beams/2) {
fprintf(stderr, "\nPing time: %d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n", 
store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5], store->time_i[6]);
fprintf(stderr, "Heading:%f Roll:%f Pitch:%f Heave:%f Sensordepth:%f\n",
heading, roll, pitch, heave, sensordepth);
fprintf(stderr, "Bathy calc: rx_sign:%d tx_sign:%d beamAzimuth:%f beamDepression:%f rr:%f zz:%f xt:%f lt:%f  depth:%f\n",
rx_sign, tx_sign, beamAzimuth, beamDepression, rr, zz, bathydata->acrosstrack, bathydata->alongtrack, bathydata->depth);
}
        }

        /* set flag */
        RawDetection->optionaldata = true;
        RawDetection->header.OptionalDataOffset =
            MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_RawDetection
            + RawDetection->number_beams * RawDetection->data_field_size;
      }
      /* calculate bathymetry from SegmentedRawDetection record */
      else if (store->read_SegmentedRawDetection) {
        for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata =
              &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment-1]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);

          /* get range */
          ttime = segmentedrawdetectionrxdata->detection_point / segmentedrawdetectiontxdata->sampling_rate;

          /* set initial beamflag based on beam quality values */
          qualitycharptr = (mb_u_char *)&(segmentedrawdetectionrxdata->quality);
          if ((qualitycharptr[0] & 0x03) == 0x03) {
            beamflag = MB_FLAG_NONE;
          } else {
            beamflag = MB_FLAG_FLAG + MB_FLAG_SONAR;
          }
          qualitycharptr[3] = beamflag;

          /* get roll at bottom return time for this beam */
          /* interp_status &= */
              mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_roll - 1, pars->n_attitude,
                               time_d + ttime, &beamroll, &jAttitude, error);
          //beamrollr = DTR * beamroll;

          /* get pitch at bottom return time for this beam */
          /* interp_status &= */
              mb_linear_interp(verbose, pars->attitude_time_d - 1, pars->attitude_pitch - 1, pars->n_attitude,
                               time_d + ttime, &beampitch, &jAttitude, error);
          // const double beampitchr = DTR * beampitch;

          /* get heading at bottom return time for this beam */
          /* interp_status &= */ mb_linear_interp_heading(verbose, pars->heading_time_d - 1, pars->heading_heading - 1,
                                                   pars->n_heading, time_d + ttime, &beamheading,
                                                   &jheading, error);
          // beamheadingr = DTR * beamheading;

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
          tx_steer = RTD * tx_sign * segmentedrawdetectiontxdata->tx_angle_along;
          tx_orientation.roll = roll;
          tx_orientation.pitch = pitch;
          tx_orientation.heading = heading;
          rx_steer = -RTD * rx_sign * segmentedrawdetectionrxdata->rx_angle_cross;
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

          /* calculate Bathymetry */
          const double rr = 0.5 * soundspeed * ttime;
          const double xx = rr * sin(DTR * theta);
          const double zz = rr * cos(DTR * theta);
          bathydata->acrosstrack = xx * cos(DTR * phi);
          bathydata->alongtrack = xx * sin(DTR * phi);
          bathydata->depth = zz + sensordepth - heave;
          bathydata->pointing_angle = DTR * theta;
          bathydata->azimuth_angle = DTR * beamAzimuth;
        }

        /* set flag */
        SegmentedRawDetection->optionaldata = true;
        SegmentedRawDetection->header.OptionalDataOffset =
            MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_SegmentedRawDetection
            + SegmentedRawDetection->n_segments * SegmentedRawDetection->segment_field_size
            + SegmentedRawDetection->n_rx * SegmentedRawDetection->rx_field_size;
      }
    }

    /* regenerate SideScan */
    if (!store->read_ProcessedSideScan
      || pars->recalculate_bathymetry
      || pars->multibeam_sidescan_source != MB_PR_SSSOURCE_UNKNOWN) {
      int ss_source = R7KRECID_None;
      if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_CALIBRATEDSNIPPET)
        ss_source = R7KRECID_SnippetBackscatteringStrength;
      else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_SNIPPET)
        ss_source = R7KRECID_Snippet;
      else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_CALIBRATEDWIDEBEAMBACKSCATTER)
        ss_source = R7KRECID_CalibratedSideScan;
      else if (pars->multibeam_sidescan_source == MB_PR_SSSOURCE_WIDEBEAMBACKSCATTER)
        ss_source = R7KRECID_SideScan;
      status = mbsys_reson7k3_makess_source(verbose, mbio_ptr, store_ptr, ss_source,
                                      false, pixel_size, false, swath_width,
                                      true, error);
    }
  //if (store->read_RawDetection) {
  //  mbsys_reson7k3_print_RawDetection(verbose, &store->RawDetection, error);
  //}
  //else if (store->read_SegmentedRawDetection) {
  //  mbsys_reson7k3_print_SegmentedRawDetection(verbose, &store->SegmentedRawDetection, error);
  //}
  //if (store->read_ProcessedSideScan)
  //mbsys_reson7k3_print_ProcessedSideScan(verbose, &store->ProcessedSideScan, error);

    /*--------------------------------------------------------------*/
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
int mbsys_reson7k3_extract_platform(int verbose, void *mbio_ptr, void *store_ptr, int *kind, void **platform_ptr, int *error) {
  int sensor_multibeam, sensor_position, sensor_Attitude;
  int ntimelag = 0;
  int isensor;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:         %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:      %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       platform_ptr:   %p\n", (void *)platform_ptr);
    fprintf(stderr, "dbg2       *platform_ptr:  %p\n", (void *)*platform_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_InstallationParameters *InstallationParameters = (s7k3_InstallationParameters *)&store->InstallationParameters;

  int status = MB_SUCCESS;

  /* if needed allocate a new platform structure */
  if (*platform_ptr == NULL) {
    status = mb_platform_init(verbose, (void **)platform_ptr, error);
  }

  /* extract sensor offsets from InstallationParameters record */
  if (*platform_ptr != NULL) {
    /* get pointer to platform structure */
    struct mb_platform_struct *platform = (struct mb_platform_struct *)(*platform_ptr);

    /* look for multibeam sensor, add it if necessary */
    sensor_multibeam = -1;
    for (isensor = 0; isensor < platform->num_sensors && sensor_multibeam < 0; isensor++) {
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
            verbose, (void *)platform, 0, 0, MB_SENSOR_POSITION_OFFSET_STATIC, (double)InstallationParameters->transmit_x,
            (double)InstallationParameters->transmit_y, (double)InstallationParameters->transmit_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
            (double)InstallationParameters->transmit_heading, (double)InstallationParameters->transmit_roll,
            (double)InstallationParameters->transmit_pitch, error);
      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 0, 1, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)InstallationParameters->receive_x, (double)InstallationParameters->receive_y,
                                               (double)InstallationParameters->receive_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
                                               (double)InstallationParameters->receive_heading, (double)InstallationParameters->receive_roll,
                                               (double)InstallationParameters->receive_pitch, error);
    }

    /* look for position sensor, add it if necessary */
    sensor_position = -1;
    if (platform->source_position1 >= 0)
      sensor_position = platform->source_position1;
    for (isensor = 0; isensor < platform->num_sensors && sensor_position < 0; isensor++) {
      if (platform->sensors[isensor].type == MB_SENSOR_TYPE_POSITION && platform->sensors[isensor].num_offsets == 1) {
        sensor_position = isensor;
      }
    }
    if (sensor_position < 0) {
      /* set sensor 1 (position) */
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
                                               (double)InstallationParameters->position_x, (double)InstallationParameters->position_y,
                                               (double)InstallationParameters->position_z, MB_SENSOR_ATTITUDE_OFFSET_NONE,
                                               (double)0.0, (double)0.0, (double)0.0, error);
      if (status == MB_SUCCESS && InstallationParameters->position_time_delay != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.001 * InstallationParameters->position_time_delay), 0, NULL, NULL, error);
      }
    }

    /* look for Attitude sensor, add it if necessary */
    sensor_Attitude = -1;
    if (platform->source_rollpitch1 >= 0)
      sensor_Attitude = platform->source_rollpitch1;
    for (isensor = 0; isensor < platform->num_sensors && sensor_Attitude < 0; isensor++) {
      if ((platform->sensors[isensor].type == MB_SENSOR_TYPE_VRU || platform->sensors[isensor].type == MB_SENSOR_TYPE_IMU ||
           platform->sensors[isensor].type == MB_SENSOR_TYPE_INS) &&
          platform->sensors[isensor].num_offsets == 1) {
        sensor_Attitude = isensor;
      }
    }
    if (sensor_Attitude < 0) {
      /* set sensor 2 (Attitude) */
      status =
          mb_platform_add_sensor(verbose, (void *)platform, MB_SENSOR_TYPE_VRU, NULL, NULL, NULL, 0, 0, 1, ntimelag, error);
      if (status == MB_SUCCESS) {
        sensor_Attitude = platform->num_sensors - 1;
      }
    }
    if (sensor_Attitude >= 0 && platform->sensors[sensor_Attitude].num_offsets == 1) {
      if (status == MB_SUCCESS) {
        platform->source_rollpitch1 = sensor_Attitude;
        platform->source_heading1 = sensor_Attitude;
        platform->source_rollpitch = sensor_Attitude;
        platform->source_heading = sensor_Attitude;
      }

      if (status == MB_SUCCESS)
        status = mb_platform_set_sensor_offset(verbose, (void *)platform, 2, 0, MB_SENSOR_POSITION_OFFSET_STATIC,
                                               (double)InstallationParameters->motion_x, (double)InstallationParameters->motion_y,
                                               (double)InstallationParameters->motion_z, MB_SENSOR_ATTITUDE_OFFSET_STATIC,
                                               (double)InstallationParameters->motion_heading, (double)InstallationParameters->motion_roll,
                                               (double)InstallationParameters->motion_pitch, error);
      if (status == MB_SUCCESS && InstallationParameters->motion_time_delay != 0) {
        status =
            mb_platform_set_sensor_timelatency(verbose, (void *)platform, 1, MB_SENSOR_TIME_LATENCY_STATIC,
                                               (double)(0.001 * InstallationParameters->motion_time_delay), 0, NULL, NULL, error);
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
int mbsys_reson7k3_extract(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d, double *navlon,
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

  s7k3_bathydata *bathydata = NULL;
  s7k3_rawdetectiondata *rawdetectiondata = NULL;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata = NULL;

  /* get mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get data structure pointer */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&(store->SonarSettings);
  s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&(store->BeamGeometry);
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;
  s7k3_ProcessedSideScan *ProcessedSideScan = (s7k3_ProcessedSideScan *)&store->ProcessedSideScan;
  s7k3_Position *Position = (s7k3_Position *)&store->Position;
  s7k3_Navigation *Navigation = (s7k3_Navigation *)&store->Navigation;
  s7k3_SystemEventMessage *SystemEventMessage = (s7k3_SystemEventMessage *)&store->SystemEventMessage;

  /* get saved values */
  // double *pixel_size = (double *)&mb_io_ptr->saved1;
  // double *swath_width = (double *)&mb_io_ptr->saved2;

  /* get data kind */
  *kind = store->kind;

#ifdef MBSYS_RESON7K3_DEBUG
  fprintf(stderr, "\nMBIO function <%s> called store->kind:%d\n", __func__, store->kind);
  if (store->kind == MB_DATA_DATA) {
    fprintf(stderr,  "Records read:\n");
  if (store->read_ProcessedSideScan > 0)
    fprintf(stderr, "  read_ProcessedSideScan\n");
  if (store->read_SonarSettings > 0)
    fprintf(stderr, "  read_SonarSettings\n");
  if (store->read_MatchFilter > 0)
    fprintf(stderr, "  read_MatchFilter\n");
  if (store->read_BeamGeometry > 0)
    fprintf(stderr, "  read_BeamGeometry\n");
  if (store->read_Bathymetry > 0)
    fprintf(stderr, "  read_Bathymetry optionaldata:%d\n", store->Bathymetry.optionaldata);
  if (store->read_SideScan > 0)
    fprintf(stderr, "  read_SideScan optionaldata:%d\n", store->SideScan.optionaldata);
  if (store->read_WaterColumn > 0)
    fprintf(stderr, "  read_WaterColumn\n");
  if (store->read_VerticalDepth > 0)
    fprintf(stderr, "  read_VerticalDepth\n");
  if (store->read_TVG > 0)
    fprintf(stderr, "  read_TVG\n");
  if (store->read_Image > 0)
    fprintf(stderr, "  read_Image\n");
  if (store->read_PingMotion > 0)
    fprintf(stderr, "  read_PingMotion\n");
  if (store->read_DetectionDataSetup > 0)
    fprintf(stderr, "  read_DetectionDataSetup\n");
  if (store->read_Beamformed > 0)
    fprintf(stderr, "  read_Beamformed\n");
  if (store->read_VernierProcessingDataRaw > 0)
    fprintf(stderr, "  read_VernierProcessingDataRaw\n");
  if (store->read_RawDetection > 0)
    fprintf(stderr, "  read_RawDetection optionaldata:%d\n", store->RawDetection.optionaldata);
  if (store->read_Snippet > 0)
    fprintf(stderr, "  read_Snippet optionaldata:%d\n", store->Snippet.optionaldata);
  if (store->read_VernierProcessingDataFiltered > 0)
    fprintf(stderr, "  read_VernierProcessingDataFiltered\n");
  if (store->read_CompressedBeamformedMagnitude > 0)
    fprintf(stderr, "  read_CompressedBeamformedMagnitude\n");
  if (store->read_CompressedWaterColumn > 0)
    fprintf(stderr, "  read_CompressedWaterColumn\n");
  if (store->read_SegmentedRawDetection > 0)
    fprintf(stderr, "  read_SegmentedRawDetection optionaldata:%d\n", store->SegmentedRawDetection.optionaldata);
  if (store->read_CalibratedBeam > 0)
    fprintf(stderr, "  read_CalibratedBeam\n");
  if (store->read_CalibratedSideScan > 0)
    fprintf(stderr, "  read_CalibratedSideScan optionaldata:%d\n", store->CalibratedSideScan.optionaldata);
  if (store->read_SnippetBackscatteringStrength > 0)
    fprintf(stderr, "  read_SnippetBackscatteringStrength optionaldata:%d\n", store->SnippetBackscatteringStrength.optionaldata);
  if (store->read_RemoteControlSonarSettings > 0)
    fprintf(stderr, "  read_RemoteControlSonarSettings\n");
}
#endif

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {

    // bathymetry in RawDetection 7027 records (e.g. Reson)
    if (store->read_RawDetection
        && RawDetection->optionaldata) {

      /* get the time */
      s7k3_header *header = &RawDetection->header;
      int time_j[5];
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, time_d);

      /* get heading */
      *heading = RTD * RawDetection->heading;

      /* get nav heading and speed  */
      *speed = 0.0;
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

      /* get Navigation */
      if (RawDetection->longitude != 0.0
          && RawDetection->latitude != 0.0) {
        *navlon = RTD * RawDetection->longitude;
        *navlat = RTD * RawDetection->latitude;
      }

      /* set beamwidths in mb_io structure */
      mb_io_ptr->beamwidth_xtrack = MIN(mb_io_ptr->beamwidth_xtrack, 2.0);
      mb_io_ptr->beamwidth_ltrack = MIN(mb_io_ptr->beamwidth_ltrack, 2.0);
      mb_io_ptr->beamwidth_xtrack = RTD * BeamGeometry->beamwidth_acrosstrack[BeamGeometry->number_beams / 2];
      //mb_io_ptr->beamwidth_ltrack = RTD * BeamGeometry->beamwidth_alongtrack[BeamGeometry->number_beams / 2];
      mb_io_ptr->beamwidth_ltrack = RTD * SonarSettings->beamwidth_vertical;

      /* read distance and depth values into storage arrays */
      /* the number of soundings reported is the number of actual detections -
          this is because the multi-detect function of Reson multibeams can produce up to five
          soundings per each formed beam that will be reported in this record */
      *nbath = RawDetection->number_beams;
      *namp = *nbath;
      *nss = 0;
      for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
        rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
        bathydata = &(RawDetection->bathydata[i]);
        bath[i] = bathydata->depth;
        bathacrosstrack[i] = bathydata->acrosstrack;
        bathalongtrack[i] = bathydata->alongtrack;
        const u32 quality = (rawdetectiondata->quality & 0xFF000000);
        beamflag[i] = (u8)(quality >> 24);
        amp[i] = rawdetectiondata->signal_strength;
      }

      // if multi-pick enabled make sure flagged secondary soundings are flagged for being secondary
      // do this check on insertion too
      if (RawDetection->flags & 0x10) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          if (mb_beam_check_flag_flagged(beamflag[i]) && rawdetectiondata->flags & 0x1E00) {
            beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MULTIPICK;
          }
        }
      }

    } // end bathymetry in RawDetection 7027 records (e.g. Reson)

    // bathymetry in SegmentedRawDetection records (e.g. Hydrosweep)
    else if (store->read_SegmentedRawDetection
        && SegmentedRawDetection->optionaldata) {
//mbsys_reson7k3_print_SegmentedRawDetection(verbose, SegmentedRawDetection, error);

      /* get the time */
      s7k3_header *header = &SegmentedRawDetection->header;
      int time_j[5];
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, time_d);

      /* get heading */
      *heading = RTD * SegmentedRawDetection->heading;

      /* get nav heading and speed  */
      *speed = 0.0;
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

      /* get Navigation */
      if (SegmentedRawDetection->longitude != 0.0
          && SegmentedRawDetection->latitude != 0.0) {
        *navlon = RTD * SegmentedRawDetection->longitude;
        *navlat = RTD * SegmentedRawDetection->latitude;
      }

      /* set beamwidths in mb_io structure */
      mb_io_ptr->beamwidth_xtrack = MIN(mb_io_ptr->beamwidth_xtrack, 2.0);
      mb_io_ptr->beamwidth_ltrack = MIN(mb_io_ptr->beamwidth_ltrack, 2.0);
      mb_io_ptr->beamwidth_xtrack = RTD * BeamGeometry->beamwidth_acrosstrack[BeamGeometry->number_beams / 2];
      //mb_io_ptr->beamwidth_ltrack = RTD * BeamGeometry->beamwidth_alongtrack[BeamGeometry->number_beams / 2];
      mb_io_ptr->beamwidth_ltrack = RTD * SonarSettings->beamwidth_vertical;

      /* read distance and depth values into storage arrays */
      /* the number of soundings reported is the number of actual detections -
          this is because the split beam Hydrosweep multibeams produce 960
          soundings from 320 formed beams that are reported in this record */
      *nbath = SegmentedRawDetection->n_rx;
      *namp = *nbath;
      *nss = 0;
      for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
        segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
        // s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata =
        //     &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment - 1]);
        bathydata = &(SegmentedRawDetection->bathydata[i]);
        bath[i] = bathydata->depth;
        bathacrosstrack[i] = bathydata->acrosstrack;
        bathalongtrack[i] = bathydata->alongtrack;
        const u32 quality = (segmentedrawdetectionrxdata->quality & 0xFF000000);
        beamflag[i] = (u8)(quality >> 24);
        amp[i] = segmentedrawdetectionrxdata->signal_strength;
      }
    } // end bathymetry in SegmentedRawDetection records (e.g. Hydrosweep)

    else {
      status = MB_FAILURE;
      *error = MB_ERROR_UNINTELLIGIBLE;
    }

    // extract processed multibeam sidescan
    if (status == MB_SUCCESS && store->read_ProcessedSideScan) {
			*nss = ProcessedSideScan->number_pixels;
			for (int i = 0; i < (int)ProcessedSideScan->number_pixels; i++) {
				ss[i] = ProcessedSideScan->sidescan[i];
				ssacrosstrack[i] = ProcessedSideScan->pixelwidth * (i - (int)ProcessedSideScan->number_pixels / 2);
				ssalongtrack[i] = ProcessedSideScan->alongtrack[i];
			}
			for (int i = ProcessedSideScan->number_pixels; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = 0.0;
				ssalongtrack[i] = 0.0;
			}
		}
		else {
      *nss = 0;
			for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
				ss[i] = MB_SIDESCAN_NULL;
				ssacrosstrack[i] = 0.0;
				ssalongtrack[i] = 0.0;
			}
    } // end extract processed multibeam sidescan

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
  else if (*kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get heading */
    *heading = RTD * Navigation->heading;

    /* get speed */
    *speed = 3.6 * Navigation->speed;

    /* get Navigation */
    *navlon = RTD * Navigation->longitude;
    *navlat = RTD * Navigation->latitude;

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

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get speed */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get Navigation */
    *navlon = RTD * Position->longitude_easting;
    *navlat = RTD * Position->latitude_northing;

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
    memset((void *)comment, 0, MB_COMMENT_MAXLINE);
    if (SystemEventMessage->message_length > 0) {
      strncpy(comment, SystemEventMessage->message, MIN(MB_COMMENT_MAXLINE - 1, SystemEventMessage->message_length));
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
  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k3_insert(int verbose, void *mbio_ptr, void *store_ptr, int kind, int time_i[7], double time_d, double navlon,
                         double navlat, double speed, double heading, int nbath, int namp, int nss, char *beamflag, double *bath,
                         double *amp, double *bathacrosstrack, double *bathalongtrack, double *ss, double *ssacrosstrack,
                         double *ssalongtrack, char *comment, int *error) {
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

  s7k3_rawdetectiondata *rawdetectiondata = NULL;
  s7k3_bathydata *bathydata = NULL;
  // s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata = NULL;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata = NULL;
  unsigned int msglen;
  u32 quality;

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  // s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&(store->SonarSettings);
  // s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&(store->BeamGeometry);
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;
  s7k3_ProcessedSideScan *ProcessedSideScan = (s7k3_ProcessedSideScan *)&store->ProcessedSideScan;
  s7k3_Position *Position = (s7k3_Position *)&store->Position;
  s7k3_Navigation *Navigation = (s7k3_Navigation *)&store->Navigation;
  s7k3_SystemEventMessage *SystemEventMessage = (s7k3_SystemEventMessage *)&store->SystemEventMessage;

  /* set data kind */
  store->kind = kind;

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_DATA) {

    // bathymetry in RawDetection 7027 records (e.g. Reson)
    if (store->read_RawDetection) {

      /* optional data must be set */
      RawDetection->optionaldata = true;

      /* get time */
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = time_d;

      /* get Navigation */
      RawDetection->longitude = DTR * navlon;
      RawDetection->latitude = DTR * navlat;

      /* get heading */
      RawDetection->heading = DTR * heading;

      /* get speed  */

      // if multi-pick enabled make sure flagged secondary soundings are flagged for being secondary
      // do this check on extraction too
      if (RawDetection->flags & 0x10) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          if (mb_beam_check_flag_flagged(beamflag[i]) && rawdetectiondata->flags & 0x1E00) {
            beamflag[i] = MB_FLAG_FLAG + MB_FLAG_MULTIPICK;
          }
        }
      }

      /* read distance and depth values into storage arrays */
      for (int i = 0; i < nbath; i++) {
        if ((unsigned int) i < RawDetection->number_beams) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);
          bathydata->depth = bath[i];
          bathydata->acrosstrack = bathacrosstrack[i];
          bathydata->alongtrack = bathalongtrack[i];
          quality = (rawdetectiondata->quality & 0x00FFFFFF) + (((u32)beamflag[i]) << 24);
          rawdetectiondata->quality = quality;
          rawdetectiondata->signal_strength = amp[i];
        }
      }
    } // end bathymetry in RawDetection 7027 records (e.g. Reson)

    // bathymetry in SegmentedRawDetection 7047 records (e.g. Hydrosweep)
    else if (store->read_SegmentedRawDetection) {

      /* optional data must be set */
      SegmentedRawDetection->optionaldata = true;

      /* get time */
      for (int i = 0; i < 7; i++)
        store->time_i[i] = time_i[i];
      store->time_d = time_d;

      /* get Navigation */
      SegmentedRawDetection->longitude = DTR * navlon;
      SegmentedRawDetection->latitude = DTR * navlat;

      /* get heading */
      SegmentedRawDetection->heading = DTR * heading;

      /* get speed  */

      /* read distance and depth values into storage arrays */
      for (int i = 0; i < nbath; i++) {
        if ((unsigned int) i < SegmentedRawDetection->n_rx) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);
          bathydata->depth = bath[i];
          bathydata->acrosstrack = bathacrosstrack[i];
          bathydata->alongtrack = bathalongtrack[i];
          quality = (segmentedrawdetectionrxdata->quality & 0x00FFFFFF) + (((u32)beamflag[i]) << 24);
          segmentedrawdetectionrxdata->quality = quality;
          segmentedrawdetectionrxdata->signal_strength = amp[i];
        }
      }
    } // end bathymetry in SegmentedRawDetection 7047 records (e.g. Hydrosweep)

    // insert sidescan into ProcessedSideScan record
    if (!store->read_ProcessedSideScan) {
      // Initialize ProcessedSideScan structure if necessary
      store->read_ProcessedSideScan = true;
      if (store->read_RawDetection) {
        ProcessedSideScan->header = RawDetection->header;
        ProcessedSideScan->serial_number = RawDetection->serial_number;
        ProcessedSideScan->ping_number = RawDetection->ping_number;
        ProcessedSideScan->multi_ping = RawDetection->multi_ping;
        ProcessedSideScan->sensordepth = RawDetection->vehicle_depth;
        ProcessedSideScan->altitude = bath[nbath/2] - ProcessedSideScan->sensordepth;
      }
      else if (store->read_SegmentedRawDetection) {
        ProcessedSideScan->header = SegmentedRawDetection->header;
        ProcessedSideScan->serial_number = SegmentedRawDetection->serial_number;
        ProcessedSideScan->ping_number = SegmentedRawDetection->ping_number;
        ProcessedSideScan->multi_ping = SegmentedRawDetection->multi_ping;
        ProcessedSideScan->sensordepth = SegmentedRawDetection->vehicle_depth;
        ProcessedSideScan->altitude = bath[nbath/2] - ProcessedSideScan->sensordepth;
      }
      ProcessedSideScan->header.Offset = 60;
      ProcessedSideScan->header.Size =
          MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE
          + R7KHDRSIZE_ProcessedSideScan + nss * 8;
      ProcessedSideScan->header.OptionalDataOffset = 0;
      ProcessedSideScan->header.OptionalDataIdentifier = 0;
      ProcessedSideScan->header.RecordType = R7KRECID_ProcessedSideScan;
      ProcessedSideScan->recordversion = 1;
      ProcessedSideScan->ss_source = MB_PR_SSSOURCE_UNKNOWN;
      ProcessedSideScan->number_pixels = nss;
      ProcessedSideScan->ss_type = MB_SIDESCAN_LINEAR;
      int ixmin = nss - 1;
      int ixmax = 0;
      for (int i = 0; i < nss; i++) {
        if (ss[i] != MB_SIDESCAN_NULL) {
          if (i < ixmin) ixmin = i;
          ixmax = i;
        }
      }
      if (ixmax > ixmin)
        ProcessedSideScan->pixelwidth = (ssacrosstrack[ixmax] - ssacrosstrack[ixmin]) / (ixmax - ixmin);
      else
        ProcessedSideScan->pixelwidth = 1.0;
    }

    // Insert the sidescan
    for (int i = 0; i < nss; i++) {
      ProcessedSideScan->sidescan[i] = ss[i];
      ProcessedSideScan->alongtrack[i] = ssalongtrack[i];
    }
    for (int i = nss; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      ProcessedSideScan->sidescan[i] = 0.0;
      ProcessedSideScan->alongtrack[i] = 0.0;
    }
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get Navigation */
    Navigation->longitude = DTR * navlon;
    Navigation->latitude = DTR * navlat;

    /* get heading */
    Navigation->heading = DTR * heading;

    /* get speed  */
    Navigation->speed = speed / 3.6;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get Navigation */
    Position->longitude_easting = DTR * navlon;
    Position->latitude_northing = DTR * navlat;

    /* get heading */

    /* get speed  */
  }

  /* insert comment in structure */
  else if (store->kind == MB_DATA_COMMENT) {
    /* make sure memory is allocated for comment */
    msglen = MIN(strlen(comment) + 1, MB_COMMENT_MAXLINE);
    if (msglen % 2 > 0)
      msglen++;
    if (SystemEventMessage->message_alloc < msglen) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, msglen, (void **)&(SystemEventMessage->message), error);
      if (status != MB_SUCCESS) {
        SystemEventMessage->message_alloc = 0;
        SystemEventMessage->message = NULL;
      }
      else {
        SystemEventMessage->message_alloc = msglen;
      }
    }

    /* copy comment */
    if (status == MB_SUCCESS) {
      store->type = R7KRECID_SystemEventMessage;
      SystemEventMessage->serial_number = 0;
      SystemEventMessage->event_id = 1;
      SystemEventMessage->message_length = msglen;
      SystemEventMessage->event_identifier = 0;
      memset((void *)SystemEventMessage->message, 0, SystemEventMessage->message_alloc);
      strncpy(SystemEventMessage->message, comment, msglen);
      SystemEventMessage->header.Version = 5;
      SystemEventMessage->header.Offset = 60;
      SystemEventMessage->header.SyncPattern = 0x0000FFFF;
      SystemEventMessage->header.Size =
          MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_SystemEventMessage + msglen + MBSYS_RESON7K_RECORDTAIL_SIZE;
      SystemEventMessage->header.OptionalDataOffset = 0;
      SystemEventMessage->header.OptionalDataIdentifier = 0;
      SystemEventMessage->header.Reserved = 0;
      SystemEventMessage->header.RecordType = R7KRECID_SystemEventMessage;
      SystemEventMessage->header.DeviceId = 0;
      SystemEventMessage->header.SystemEnumerator = 0;
      SystemEventMessage->header.Reserved2 = 0;
      SystemEventMessage->header.Flags = 0;
      SystemEventMessage->header.Reserved3 = 0;
      SystemEventMessage->header.Reserved4 = 0;
      SystemEventMessage->header.FragmentedTotal = 0;
      SystemEventMessage->header.FragmentNumber = 0;
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
int mbsys_reson7k3_ttimes(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, double *ttimes, double *angles,
                         double *angles_forward, double *angles_null, double *heave, double *alongtrack_offset, double *draft,
                         double *ssv, int *error) {
  s7k3_rawdetectiondata *rawdetectiondata;
  s7k3_bathydata *bathydata;
  s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata;

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

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&store->SonarSettings;
  // s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&store->BeamGeometry;
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = &store->SegmentedRawDetection;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    if (store->read_RawDetection
        && RawDetection->optionaldata) {

      /* get depth offset (heave + sonar depth) */
      *ssv = SonarSettings->sound_velocity;

      /* get draft */
      *draft = RawDetection->vehicle_depth;

      /* get travel times, angles */
      *nbeams = RawDetection->number_beams;
      for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
        rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
        bathydata = &(RawDetection->bathydata[i]);
        ttimes[i] = rawdetectiondata->detection_point / RawDetection->sampling_rate;
        angles[i] = RTD * bathydata->pointing_angle;
        angles_forward[i] = 90.0 - RTD * bathydata->azimuth_angle;
        angles_null[i] = 0.0;
        heave[i] =  RawDetection->heave;
        alongtrack_offset[i] = 0.0;
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }

    else if (store->read_SegmentedRawDetection
        && SegmentedRawDetection->optionaldata) {

      /* get depth offset (heave + sonar depth) */
      *ssv = SonarSettings->sound_velocity;

      /* get draft */
      *draft = SegmentedRawDetection->vehicle_depth;

      /* get travel times, angles */
      *nbeams = SegmentedRawDetection->n_rx;
      for (int i = 0; i < *nbeams; i++) {
        segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
        segmentedrawdetectiontxdata = &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment - 1]);
        bathydata = &(SegmentedRawDetection->bathydata[i]);
        ttimes[i] = segmentedrawdetectionrxdata->detection_point / segmentedrawdetectiontxdata->sampling_rate;
        angles[i] = RTD * bathydata->pointing_angle;
        angles_forward[i] = 90.0 - RTD * bathydata->azimuth_angle;
        angles_null[i] = 0.0;
        heave[i] =  SegmentedRawDetection->heave;
        alongtrack_offset[i] = 0.0;
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }

    /* deal with other record type */
    else {
      /* set status */
      *error = MB_ERROR_UNINTELLIGIBLE;
      status = MB_FAILURE;
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
int mbsys_reson7k3_detects(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nbeams, int *detects, int *error) {
  s7k3_rawdetectiondata *rawdetectiondata = NULL;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       detects:    %p\n", (void *)detects);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&store->BeamGeometry;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    if (store->read_BeamGeometry
        && store->read_RawDetection) {
      /* read beam detects into storage arrays */
      *nbeams = BeamGeometry->number_beams;
      for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
        rawdetectiondata = &(RawDetection->rawdetectiondata[i]);

        // Detect types are in bits 0-1
        if (rawdetectiondata->flags & 0x01)
          detects[i] = MB_DETECT_AMPLITUDE;
        else if (rawdetectiondata->flags & 0x02)
          detects[i] = MB_DETECT_PHASE;
        else
          detects[i] = MB_DETECT_UNKNOWN;

        // multidetect priority (0=highest) is in flags bits 9-12, shift to 8-11 in detects value
        if (RawDetection->flags & 0x10) {
          detects[i] = detects[i] | (int)((rawdetectiondata->flags & 0x1E00) >> 1);
        }
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }

    else if (store->read_SegmentedRawDetection
        && SegmentedRawDetection->optionaldata) {
      /* read beam detects into storage arrays */
      *nbeams = SegmentedRawDetection->n_rx;
      for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
        segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
        // s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata =
        //     &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment - 1]);
        // s7k3_bathydata *bathydata = &(SegmentedRawDetection->bathydata[i]);

        // Detect types are in bits 0-1
        if (segmentedrawdetectionrxdata->flags2 & 0x01)
          detects[i] = MB_DETECT_AMPLITUDE;
        else if (segmentedrawdetectionrxdata->flags2 & 0x02)
          detects[i] = MB_DETECT_PHASE;
        else
          detects[i] = MB_DETECT_UNKNOWN;

        // multidetect priority (0=highest) is in flags bits 9-12, shift to 8-11 in detects value
        if (RawDetection->flags & 0x10) {
          // TODO(schwehr): Bug?  rawdetectiondata->flags is Uninitialized according to cppcheck
          detects[i] = detects[i] | (int)((rawdetectiondata->flags & 0x1E00) >> 1);
        }
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }

    else {
      *error = MB_ERROR_UNINTELLIGIBLE;
      status = MB_FAILURE;
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
int mbsys_reson7k3_gains(int verbose, void *mbio_ptr, void *store_ptr, int *kind, double *transmit_gain, double *pulse_length,
                        double *receive_gain, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&store->SonarSettings;
  // s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {
    if (store->read_SonarSettings
        && store->read_RawDetection) {

      /* get transmit_gain (dB) */
      *transmit_gain = (double)SonarSettings->power_selection;

      /* get pulse_length (usec) */
      *pulse_length = (double)SonarSettings->tx_pulse_width;

      /* get receive_gain (dB) */
      *receive_gain = (double)SonarSettings->gain_selection;

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;

      /* done translating values */
    }
    else {
      *error = MB_ERROR_UNINTELLIGIBLE;
      status = MB_FAILURE;
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
int mbsys_reson7k3_extract_altitude(int verbose, void *mbio_ptr, void *store_ptr,
                                    int *kind, double *transducer_depth,
                                    double *altitudev, int *error) {
  s7k3_rawdetectiondata *rawdetectiondata;
  s7k3_bathydata *bathydata;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata;
  double xtrackmin;
  mb_u_char *qualitycharptr;
  mb_u_char beamflag;

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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  // s7k3_Navigation *Navigation = (s7k3_Navigation *)&store->Navigation;
  // s7k3_Attitude *Attitude = (s7k3_Attitude *)&store->Attitude;
  s7k3_Altitude *Altitude = (s7k3_Altitude *)&store->Altitude;
  // s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&(store->SonarSettings);
  // s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&(store->BeamGeometry);
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_DATA) {

    /* get altitude */
    bool altitude_found = false;
    if (mb_io_ptr->naltitude > 0) {
      mb_altint_interp(verbose, mbio_ptr, store->time_d, altitudev, error);
      altitude_found = true;
    }

    if (store->read_RawDetection
        && RawDetection->optionaldata) {

      /* get transducer depth and altitude */
      *transducer_depth = RawDetection->vehicle_depth + RawDetection->heave;
      if (!altitude_found) {
        /* get depth closest to nadir */
        xtrackmin = 999999.9;
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);
          qualitycharptr = (mb_u_char *)&(rawdetectiondata->quality);
          beamflag = qualitycharptr[3];
          if (mb_beam_ok(beamflag)) {
            if (fabs(bathydata->acrosstrack) < xtrackmin) {
              xtrackmin = fabs(bathydata->acrosstrack);
              *altitudev = bathydata->depth - *transducer_depth;
              altitude_found = true;
            }
          }
        }
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;

    }

    else if (store->read_SegmentedRawDetection
        && SegmentedRawDetection->optionaldata) {

      /* get transducer depth and altitude */
      *transducer_depth = SegmentedRawDetection->vehicle_depth + SegmentedRawDetection->heave;
      if (!altitude_found) {
        /* get depth closest to nadir */
        xtrackmin = 999999.9;
        for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);
          qualitycharptr = (mb_u_char *)&(segmentedrawdetectionrxdata->quality);
          beamflag = qualitycharptr[3];
          if (mb_beam_ok(beamflag)) {
            if (fabs(bathydata->acrosstrack) < xtrackmin) {
              xtrackmin = fabs(bathydata->acrosstrack);
              *altitudev = bathydata->depth - *transducer_depth;
              altitude_found = true;
            }
          }
        }
      }

      /* set status */
      *error = MB_ERROR_NO_ERROR;
      status = MB_SUCCESS;
    }

    else {
      *error = MB_ERROR_UNINTELLIGIBLE;
      status = MB_FAILURE;
    }

    if (!altitude_found && Altitude->altitude > 0.0) {
      *altitudev = Altitude->altitude;
    } else if (!altitude_found) {
      *altitudev = 0.0;
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
int mbsys_reson7k3_extract_nav(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int time_i[7], double *time_d,
                              double *navlon, double *navlat, double *speed, double *heading, double *draft, double *roll,
                              double *pitch, double *heave, int *error) {
  s7k3_header *header;
  int time_j[5];

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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Position *Position = (s7k3_Position *)&store->Position;
  s7k3_CustomAttitude *CustomAttitude = (s7k3_CustomAttitude *)&(store->CustomAttitude);
  // s7k3_Altitude *Altitude = (s7k3_Altitude *)&store->Altitude;
  s7k3_Depth *Depth = (s7k3_Depth *)&store->Depth;
  s7k3_RollPitchHeave *RollPitchHeave = (s7k3_RollPitchHeave *)&(store->RollPitchHeave);
  s7k3_Heading *Heading = (s7k3_Heading *)&store->Heading;
  s7k3_Navigation *Navigation = (s7k3_Navigation *)&store->Navigation;
  s7k3_Attitude *Attitude = (s7k3_Attitude *)&store->Attitude;
  // s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&(store->SonarSettings);
  // s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&(store->BeamGeometry);
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from ping structure */
  if (*kind == MB_DATA_DATA) {
    if (store->read_RawDetection
        && RawDetection->optionaldata) {

      /* get the time */
      header = &RawDetection->header;
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, time_d);

      /* get heading */
      *heading = RTD * RawDetection->heading;

      /* get interpolated nav and speed  */
      *speed = 0.0;
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

      /* get Navigation */
      if (RawDetection->longitude != 0.0
          && RawDetection->latitude != 0.0) {
        *navlon = RTD * RawDetection->longitude;
        *navlat = RTD * RawDetection->latitude;
      }

      /* get draft  */
      *draft = RawDetection->vehicle_depth;

      /* get Attitude  */
      *roll = RTD * RawDetection->roll;
      *pitch = RTD * RawDetection->pitch;
      *heave = RawDetection->heave;

      /* done translating values */
    }

    else if (store->read_SegmentedRawDetection
        && SegmentedRawDetection->optionaldata) {

      /* get the time */
      header = &SegmentedRawDetection->header;
      time_j[0] = header->s7kTime.Year;
      time_j[1] = header->s7kTime.Day;
      time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
      time_j[3] = (int)header->s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      mb_get_time(verbose, time_i, time_d);

      /* get heading */
      *heading = RTD * SegmentedRawDetection->heading;

      /* get nav heading and speed  */
      *speed = 0.0;
      if (mb_io_ptr->nfix > 0)
        mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

      /* get Navigation */
      if (SegmentedRawDetection->longitude != 0.0
          && SegmentedRawDetection->latitude != 0.0) {
        *navlon = RTD * SegmentedRawDetection->longitude;
        *navlat = RTD * SegmentedRawDetection->latitude;
      }

      /* get draft  */
      *draft = SegmentedRawDetection->vehicle_depth;

      /* get Attitude  */
      *roll = RTD * SegmentedRawDetection->roll;
      *pitch = RTD * SegmentedRawDetection->pitch;
      *heave = SegmentedRawDetection->heave;

      /* done translating values */
    }

    else {
      *error = MB_ERROR_UNINTELLIGIBLE;
      status = MB_FAILURE;
    }
  }

  /* extract data from nav structure */
  else if (*kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get Navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    *navlon = RTD * Navigation->longitude;
    *navlat = RTD * Navigation->latitude;

    /* get heading */
    *heading = RTD * Navigation->heading;

    /* get speed */
    *heading = 3.6 * Navigation->speed;

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }
  }

  /* extract data from nav structure */
  else if (*kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);
    *navlon = RTD * Position->longitude_easting;
    *navlat = RTD * Position->latitude_northing;

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }

    /* done translating values */
  }

  /* extract data from Attitude structure */
  else if (*kind == MB_DATA_ATTITUDE) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    *heading = (double)(RTD * Attitude->heading[0]);

    /* get roll pitch and heave */
    *roll = (double)(RTD * Attitude->roll[0]);
    *pitch = (double)(RTD * Attitude->pitch[0]);
    *heave = (double)(Attitude->heave[0]);

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }

    /* done translating values */
  }

  /* extract data from RollPitchHeave structure */
  else if (*kind == MB_DATA_ATTITUDE1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get roll pitch and heave */
    *roll = (double)(RTD * RollPitchHeave->roll);
    *pitch = (double)(RTD * RollPitchHeave->pitch);
    *heave = (double)(RollPitchHeave->heave);

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }

    /* done translating values */
  }

  /* extract data from CustomAttitude structure */
  else if (*kind == MB_DATA_ATTITUDE2) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    *heading = (double)(RTD * CustomAttitude->heading[0]);

    /* get roll pitch and heave */
    *roll = (double)(RTD * CustomAttitude->roll[0]);
    *pitch = (double)(RTD * CustomAttitude->pitch[0]);
    *heave = (double)(CustomAttitude->heave[0]);

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }

    /* done translating values */
  }

  /* extract data from attitude structure */
  else if (*kind == MB_DATA_HEADING) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    *heading = (double)(RTD * Heading->heading);

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    if (mb_io_ptr->nsensordepth > 0) {
      mb_depint_interp(verbose, mbio_ptr, store->time_d, draft, error);
      *heave = 0.0;
    }
    else {
      *draft = RawDetection->vehicle_depth;
      *heave = 0.0;
    }

    /* done translating values */
  }

  /* extract data from attitude structure */
  else if (*kind == MB_DATA_SENSORDEPTH) {
    /* get time */
    for (int i = 0; i < 7; i++)
      time_i[i] = store->time_i[i];
    *time_d = store->time_d;

    /* get navigation */
    *speed = 0.0;
    if (mb_io_ptr->nfix > 0)
      mb_navint_interp(verbose, mbio_ptr, store->time_d, *heading, *speed, navlon, navlat, speed, error);

    /* get heading */
    if (mb_io_ptr->nheading > 0)
      mb_hedint_interp(verbose, mbio_ptr, store->time_d, heading, error);

    /* get roll pitch and heave */
    if (mb_io_ptr->nattitude > 0) {
      mb_attint_interp(verbose, mbio_ptr, *time_d, heave, roll, pitch, error);
    }

    /* get draft  */
    *draft = Depth->depth;

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
int mbsys_reson7k3_extract_nnav(int verbose, void *mbio_ptr, void *store_ptr,
                                int nmax, int *kind, int *n, int *time_i,
                                double *time_d, double *navlon, double *navlat,
                                double *speed, double *heading, double *draft,
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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* extract data from Attitude structure (record 1016) */
  if (store->kind == MB_DATA_ATTITUDE) {
    s7k3_Attitude *Attitude = &store->Attitude;

    // get number of samples
    *n = Attitude->n;

    // loop over available data, up to the max that can be stored
    for (int inav = 0; inav < MIN(nmax, *n); inav++) {
      // get time - note time_i is dimensioned time_i[7*nmax]
      time_d[inav] = store->time_d + Attitude->delta_time[inav];
      mb_get_date(verbose, time_d[inav], &time_i[7*inav]);

      // get attitude from the Attitude record
      roll[inav] = (double)(RTD * Attitude->roll[inav]);
      pitch[inav] = (double)(RTD * Attitude->pitch[inav]);
      heave[inav] = (double)Attitude->heave[inav];
      heading[inav] = (double)(RTD * Attitude->heading[inav]);

      // get navigation from buffered time series
      speed[inav] = 0.0;
      if (mb_io_ptr->nfix > 0) {
        mb_navint_interp(verbose, mbio_ptr, time_d[inav], heading[inav], speed[inav],
                          &(navlon[inav]), &(navlat[inav]), &(speed[inav]), error);
      } else {
          navlon[inav] = 0.0;
          navlat[inav] = 0.0;
      }

      // get draft from buffered time series
      if (mb_io_ptr->nsensordepth > 0)
        mb_depint_interp(verbose, mbio_ptr, time_d[inav], &(draft[inav]), error);
      else
        draft[inav] = 0.0;
    }
  }

  /* extract data from CustomAttitude (record 1004) */
  else if (store->kind == MB_DATA_ATTITUDE2) {
    s7k3_CustomAttitude *CustomAttitude = &store->CustomAttitude;

    // get number of samples and delta_time
    *n = CustomAttitude->n;
    double delta_time = 0.0;
    if (CustomAttitude->frequency > 0.0)
      delta_time = 1 / CustomAttitude->frequency;

    // loop over available data, up to the max that can be stored
    for (int inav = 0; inav < MIN(nmax, *n); inav++) {
      // get time - note time_i is dimensioned time_i[7*nmax]
      time_d[inav] = store->time_d + inav * delta_time;
      mb_get_date(verbose, time_d[inav], &time_i[7*inav]);

      // get CustomAttitude from the CustomAttitude record
      roll[inav] = (double)(RTD * CustomAttitude->roll[inav]);
      pitch[inav] = (double)(RTD * CustomAttitude->pitch[inav]);
      heave[inav] = (double)CustomAttitude->heave[inav];
      heading[inav] = (double)(RTD * CustomAttitude->heading[inav]);

      // get navigation from buffered time series
      speed[inav] = 0.0;
      if (mb_io_ptr->nfix > 0) {
        mb_navint_interp(verbose, mbio_ptr, time_d[inav], heading[inav], speed[inav],
                          &(navlon[inav]), &(navlat[inav]), &(speed[inav]), error);
      } else {
          navlon[inav] = 0.0;
          navlat[inav] = 0.0;
      }

      // get draft from buffered time series
      if (mb_io_ptr->nsensordepth > 0)
        mb_depint_interp(verbose, mbio_ptr, time_d[inav], &(draft[inav]), error);
      else
        draft[inav] = 0.0;
    }
  }

  // All other records have single values so set *n=1 and call the extract_nav() function
  else {
    *n = 1;
    status = mbsys_reson7k3_extract_nav(verbose, mbio_ptr, store_ptr, kind, time_i, time_d,
                                  navlon, navlat, speed, heading, draft, roll,
                                  pitch, heave, error);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       kind:       %d\n", *kind);
    fprintf(stderr, "dbg2       n:          %d\n", *n);
    for (int inav = 0; inav < *n; inav++) {
      fprintf(stderr, "dbg2       %d time_i:        %4.4d/%2.2d/%2.2d-%2.2d:%2.2d:%2.2d.%6.6d\n",
                      inav, time_i[inav*7], time_i[inav*7+1], time_i[inav*7+2],
                      time_i[inav*7+3], time_i[inav*7+4], time_i[inav*7+5], time_i[inav*7+6]);
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
int mbsys_reson7k3_insert_nav(int verbose, void *mbio_ptr, void *store_ptr, int time_i[7], double time_d, double navlon,
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

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  // s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&(store->SonarSettings);
  // s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&(store->BeamGeometry);
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_Position *Position = (s7k3_Position *)&store->Position;
  s7k3_Navigation *Navigation = (s7k3_Navigation *)&store->Navigation;

  int status = MB_SUCCESS;

  /* insert data in ping structure */
  if (store->kind == MB_DATA_DATA) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get Navigation */
    RawDetection->longitude = DTR * navlon;
    RawDetection->latitude = DTR * navlat;

    /* get heading */
    RawDetection->heading = DTR * heading;

    /* get speed  */

    /* get draft  */
    RawDetection->vehicle_depth = draft;

    /* get roll pitch and heave */
    RawDetection->heave = heave;
    RawDetection->pitch = DTR * pitch;
    RawDetection->roll = DTR * roll;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get Navigation */
    Navigation->longitude = DTR * navlon;
    Navigation->latitude = DTR * navlat;

    /* get heading */
    Navigation->heading = DTR * heading;

    /* get speed  */
    Navigation->speed = speed / 3.6;

    /* get draft  */
    //Navigation->height = draft;
  }

  /* insert data in nav structure */
  else if (store->kind == MB_DATA_NAV1) {
    /* get time */
    for (int i = 0; i < 7; i++)
      store->time_i[i] = time_i[i];
    store->time_d = time_d;

    /* get Navigation */
    Position->longitude_easting = DTR * navlon;
    Position->latitude_northing = DTR * navlat;

    /* get heading */

    /* get speed  */

    /* get draft  */

    /* get roll pitch and heave */
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
int mbsys_reson7k3_extract_svp(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsvp, double *depth, double *velocity,
                              int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SoundVelocityProfile *SoundVelocityProfile = (s7k3_SoundVelocityProfile *)&(store->SoundVelocityProfile);
  s7k3_CTD *CTD = (s7k3_CTD *)&(store->CTD);
  double latitude = 0.0;

  /* get data kind */
  *kind = store->kind;

  int status = MB_SUCCESS;

  /* extract data from structure */
  if (*kind == MB_DATA_VELOCITY_PROFILE) {
    /* get number of depth-velocity pairs */
    *nsvp = SoundVelocityProfile->n;

    /* get profile */
    for (int i = 0; i < *nsvp; i++) {
      depth[i] = SoundVelocityProfile->depth[i];
      velocity[i] = SoundVelocityProfile->sound_velocity[i];
    }

    /* done translating values */
  }

  else if (*kind == MB_DATA_CTD) {
    /* get number of depth-velocity pairs */
    *nsvp = CTD->n;

    if (CTD->pressure_flag == 0) {
      if (CTD->latitude != 0.0) {
        latitude = RTD * CTD->latitude;
      }
      else if (store->read_RawDetection && store->RawDetection.optionaldata
          && store->RawDetection.latitude != 0.0) {
          latitude = RTD * store->RawDetection.latitude;
      }
      else if (store->Position.latitude_northing != 0.0 && store->Position.type == 0) {
        latitude = RTD * store->Position.latitude_northing;
      }
      else if (store->Navigation.latitude != 0.0) {
        latitude = RTD * store->Navigation.latitude;
      }
    }

    /* get profile */
    for (int i = 0; i < *nsvp; i++) {
      if (CTD->pressure_flag == 0) {
        status = mb_seabird_depth(verbose, (double)CTD->pressure_depth[i], latitude, &depth[i], error);
      }
      else {
        depth[i] = CTD->pressure_depth[i];
      }
      velocity[i] = CTD->sound_velocity[i];
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
int mbsys_reson7k3_insert_svp(int verbose, void *mbio_ptr, void *store_ptr, int nsvp, double *depth, double *velocity,
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

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SoundVelocityProfile *SoundVelocityProfile = (s7k3_SoundVelocityProfile *)&(store->SoundVelocityProfile);

  int status = MB_SUCCESS;

  /* insert data in structure */
  if (store->kind == MB_DATA_VELOCITY_PROFILE) {
    /* allocate memory if necessary */
    if (SoundVelocityProfile->nalloc < (unsigned int) nsvp) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(SoundVelocityProfile->depth), error);
      status = mb_reallocd(verbose, __FILE__, __LINE__, nsvp * sizeof(float), (void **)&(SoundVelocityProfile->sound_velocity), error);
      if (status == MB_SUCCESS) {
        SoundVelocityProfile->nalloc = nsvp;
      }
      else {
        SoundVelocityProfile->n = 0;
        SoundVelocityProfile->nalloc = 0;
      }
    }

    /* get profile */
    if (status == MB_SUCCESS) {
      SoundVelocityProfile->n = nsvp;
      for (unsigned int i = 0; i < SoundVelocityProfile->n; i++) {
        SoundVelocityProfile->depth[i] = depth[i];
        SoundVelocityProfile->sound_velocity[i] = velocity[i];
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
int mbsys_reson7k3_ctd(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nctd, double *time_d, double *conductivity,
                      double *temperature, double *depth, double *salinity, double *soundspeed, int *error) {
  int time_j[5];
  int time_i[7];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CTD *CTD = (s7k3_CTD *)&(store->CTD);

  /* get data kind */
  *kind = store->kind;

  /* extract ctd data from CTD record */
  if (*kind == MB_DATA_CTD) {
    /* get header */
    s7k3_header *header = &(CTD->header);

    /* get time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &time_d[0]);

    *nctd = MIN(CTD->n, MB_CTD_MAX);
    for (int i = 0; i < *nctd; i++) {
      time_d[i] = time_d[0] + i * (1.0 / CTD->sample_rate);
      if (CTD->conductivity_flag == 0)
        conductivity[i] = CTD->conductivity_salinity[i];
      else
        salinity[i] = CTD->conductivity_salinity[i];
      temperature[i] = CTD->temperature[i];
      depth[i] = CTD->pressure_depth[i];
      soundspeed[i] = CTD->sound_velocity[i];
    }
  }

  /* else failure */
  else {
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

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbsys_reson7k3_ancilliarysensor(int verbose, void *mbio_ptr, void *store_ptr, int *kind, int *nsamples, double *time_d,
                                   double *sensor1, double *sensor2, double *sensor3, double *sensor4, double *sensor5,
                                   double *sensor6, double *sensor7, double *sensor8, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mb_ptr:     %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  /* get data kind */
  *kind = store->kind;

  /* else failure */
  *nsamples = 0;

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
int mbsys_reson7k3_copy(int verbose, void *mbio_ptr, void *store_ptr, void *copy_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       copy_ptr:   %p\n", (void *)copy_ptr);
  }

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  struct mbsys_reson7k3_struct *copy = (struct mbsys_reson7k3_struct *)copy_ptr;

  /* copy over structures, allocating memory where necessary */

  /* Type of data record */
  copy->kind = store->kind; /* MB-System record ID */
  copy->type = store->type; /* Reson record ID */

  /* ping record read flags */
  copy->read_SonarSettings = store->read_SonarSettings;
  copy->read_MatchFilter = store->read_MatchFilter;
  copy->read_BeamGeometry = store->read_BeamGeometry;
  copy->read_Bathymetry = store->read_Bathymetry;
  copy->read_SideScan = store->read_SideScan;
  copy->read_WaterColumn = store->read_WaterColumn;
  copy->read_VerticalDepth = store->read_VerticalDepth;
  copy->read_TVG = store->read_TVG;
  copy->read_Image = store->read_Image;
  copy->read_PingMotion = store->read_PingMotion;
  copy->read_DetectionDataSetup = store->read_DetectionDataSetup;
  copy->read_Beamformed = store->read_Beamformed;
  copy->read_VernierProcessingDataRaw = store->read_VernierProcessingDataRaw;
  copy->read_RawDetection = store->read_RawDetection;
  copy->read_Snippet = store->read_Snippet;
  copy->read_VernierProcessingDataFiltered = store->read_VernierProcessingDataFiltered;
  copy->read_CompressedBeamformedMagnitude = store->read_CompressedBeamformedMagnitude;
  copy->read_CompressedWaterColumn = store->read_CompressedWaterColumn;
  copy->read_SegmentedRawDetection = store->read_SegmentedRawDetection;
  copy->read_CalibratedBeam = store->read_CalibratedBeam;
  copy->read_CalibratedSideScan = store->read_CalibratedSideScan;
  copy->read_SnippetBackscatteringStrength = store->read_SnippetBackscatteringStrength;
  copy->read_RemoteControlSonarSettings = store->read_RemoteControlSonarSettings;

  /* MB-System time stamp */
  copy->time_d = store->time_d;
  for (int i = 0; i < 7; i++)
    copy->time_i[i] = store->time_i[i];

  /* Reference point information (record 1000) */
  /*  Note: these offsets should be zero for submersible vehicles */
  copy->ReferencePoint = store->ReferencePoint;

  /* Sensor uncalibrated offset position information (record 1001) */
  copy->UncalibratedSensorOffset = store->UncalibratedSensorOffset;

  /* Sensor calibrated offset position information (record 1002) */
  copy->CalibratedSensorOffset = store->CalibratedSensorOffset;

  /* Position (record 1003) */
  copy->Position = store->Position;

  /* Custom attitude (record 1004) */
  //copy->CustomAttitude = store->CustomAttitude;
  copy->CustomAttitude.header = store->CustomAttitude.header;
  copy->CustomAttitude.fieldmask = store->CustomAttitude.fieldmask;
  copy->CustomAttitude.reserved = store->CustomAttitude.reserved;
  copy->CustomAttitude.n = store->CustomAttitude.n;
  copy->CustomAttitude.frequency = store->CustomAttitude.frequency;

  int status = MB_SUCCESS;

  if (status == MB_SUCCESS && (copy->CustomAttitude.nalloc < copy->CustomAttitude.n * sizeof(float))) {
    copy->CustomAttitude.nalloc = copy->CustomAttitude.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.heading), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.pitchrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.rollrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.headingrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CustomAttitude.nalloc, (void **)&(copy->CustomAttitude.heaverate), error);
    if (status != MB_SUCCESS) {
      copy->CustomAttitude.n = 0;
      copy->CustomAttitude.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0;i<copy->CustomAttitude.n;i++) {
      copy->CustomAttitude.pitch[i] = store->CustomAttitude.pitch[i];
      copy->CustomAttitude.roll[i] = store->CustomAttitude.roll[i];
      copy->CustomAttitude.heading[i] = store->CustomAttitude.heading[i];
      copy->CustomAttitude.heave[i] = store->CustomAttitude.heave[i];
      copy->CustomAttitude.pitchrate[i] = store->CustomAttitude.pitchrate[i];
      copy->CustomAttitude.rollrate[i] = store->CustomAttitude.rollrate[i];
      copy->CustomAttitude.headingrate[i] = store->CustomAttitude.headingrate[i];
      copy->CustomAttitude.heaverate[i] = store->CustomAttitude.heaverate[i];
    }
  }

  /* Tide (record 1005) */
  copy->Tide = store->Tide;

  /* Altitude (record 1006) */
  copy->Altitude = store->Altitude;

  /* Motion over ground (record 1007) */
  //copy->MotionOverGround = store->MotionOverGround;
  copy->MotionOverGround.header = store->MotionOverGround.header;
  copy->MotionOverGround.flags = store->MotionOverGround.flags;
  copy->MotionOverGround.reserved = store->MotionOverGround.reserved;
  copy->MotionOverGround.n = store->MotionOverGround.n;
  copy->MotionOverGround.frequency = store->MotionOverGround.frequency;
  if (status == MB_SUCCESS && (copy->MotionOverGround.nalloc < copy->MotionOverGround.n * sizeof(float))) {
    copy->MotionOverGround.nalloc = copy->MotionOverGround.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.x), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.y), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.z), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.xa), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.ya), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->MotionOverGround.nalloc, (void **)&(copy->MotionOverGround.za), error);
    if (status != MB_SUCCESS) {
      copy->MotionOverGround.n = 0;
      copy->MotionOverGround.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0;i<copy->MotionOverGround.n;i++) {
      copy->MotionOverGround.x[i] = store->MotionOverGround.x[i];
      copy->MotionOverGround.y[i] = store->MotionOverGround.y[i];
      copy->MotionOverGround.z[i] = store->MotionOverGround.z[i];
      copy->MotionOverGround.xa[i] = store->MotionOverGround.xa[i];
      copy->MotionOverGround.ya[i] = store->MotionOverGround.ya[i];
      copy->MotionOverGround.za[i] = store->MotionOverGround.za[i];
    }
  }

  /* Depth (record 1008) */
  copy->Depth = store->Depth;

  /* Sound velocity profile (record 1009) */
  //copy->SoundVelocityProfile = store->SoundVelocityProfile;
  copy->SoundVelocityProfile.header = store->SoundVelocityProfile.header;
  copy->SoundVelocityProfile.position_flag = store->SoundVelocityProfile.position_flag;
  copy->SoundVelocityProfile.reserved1 = store->SoundVelocityProfile.reserved1;
  copy->SoundVelocityProfile.reserved2 = store->SoundVelocityProfile.reserved2;
  copy->SoundVelocityProfile.latitude = store->SoundVelocityProfile.latitude;
  copy->SoundVelocityProfile.longitude = store->SoundVelocityProfile.longitude;
  copy->SoundVelocityProfile.n = store->SoundVelocityProfile.n;
  if (status == MB_SUCCESS && (copy->SoundVelocityProfile.nalloc < copy->SoundVelocityProfile.n * sizeof(float))) {
    copy->SoundVelocityProfile.nalloc = copy->SoundVelocityProfile.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->SoundVelocityProfile.nalloc, (void **)&(copy->SoundVelocityProfile.depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->SoundVelocityProfile.nalloc, (void **)&(copy->SoundVelocityProfile.sound_velocity), error);
    if (status != MB_SUCCESS) {
      copy->SoundVelocityProfile.n = 0;
      copy->SoundVelocityProfile.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0;i<copy->SoundVelocityProfile.n;i++) {
      copy->SoundVelocityProfile.depth[i] = store->SoundVelocityProfile.depth[i];
      copy->SoundVelocityProfile.sound_velocity[i] = store->SoundVelocityProfile.sound_velocity[i];
    }
  }

  /* CTD (record 1010) */
  //copy->CTD = store->CTD;
  copy->CTD.header = store->CTD.header;
  copy->CTD.frequency = store->CTD.frequency;
  copy->CTD.velocity_source_flag = store->CTD.velocity_source_flag;
  copy->CTD.velocity_algorithm = store->CTD.velocity_algorithm;
  copy->CTD.conductivity_flag = store->CTD.conductivity_flag;
  copy->CTD.pressure_flag = store->CTD.pressure_flag;
  copy->CTD.position_flag = store->CTD.position_flag;
  copy->CTD.validity = store->CTD.validity;
  copy->CTD.reserved = store->CTD.reserved;
  copy->CTD.latitude = store->CTD.latitude;
  copy->CTD.longitude = store->CTD.longitude;
  copy->CTD.sample_rate = store->CTD.sample_rate;
  copy->CTD.n = store->CTD.n;
  if (status == MB_SUCCESS && (copy->CTD.nalloc < copy->CTD.n * sizeof(float))) {
    copy->CTD.nalloc = copy->CTD.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CTD.nalloc, (void **)&(copy->CTD.conductivity_salinity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CTD.nalloc, (void **)&(copy->CTD.temperature), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CTD.nalloc, (void **)&(copy->CTD.pressure_depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CTD.nalloc, (void **)&(copy->CTD.sound_velocity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->CTD.nalloc, (void **)&(copy->CTD.absorption), error);
    if (status != MB_SUCCESS) {
      copy->CTD.n = 0;
      copy->CTD.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (unsigned int i = 0;i<copy->CTD.n;i++) {
      copy->CTD.conductivity_salinity[i] = store->CTD.conductivity_salinity[i];
      copy->CTD.temperature[i] = store->CTD.temperature[i];
      copy->CTD.pressure_depth[i] = store->CTD.pressure_depth[i];
      copy->CTD.sound_velocity[i] = store->CTD.sound_velocity[i];
      copy->CTD.absorption[i] = store->CTD.absorption[i];
    }
  }

  /* Geodesy (record 1011) */
  copy->Geodesy = store->Geodesy;

  /* Roll pitch heave (record 1012) */
  copy->RollPitchHeave = store->RollPitchHeave;

  /* Heading (record 1013) */
  copy->Heading = store->Heading;

  /* Survey line (record 1014) */
  //copy->SurveyLine = store->SurveyLine;
  copy->SurveyLine.header = store->SurveyLine.header;
  copy->SurveyLine.n = store->SurveyLine.n;
  copy->SurveyLine.type = store->SurveyLine.type;
  copy->SurveyLine.turnradius = store->SurveyLine.turnradius;
  strncpy(copy->SurveyLine.name, store->SurveyLine.name, 64);
  copy->SurveyLine.name[63] = '\0';
  if (status == MB_SUCCESS && (copy->SurveyLine.nalloc < copy->SurveyLine.n * sizeof(float))) {
    copy->SurveyLine.nalloc = copy->SurveyLine.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->SurveyLine.nalloc, (void **)&(copy->SurveyLine.latitude_northing), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->SurveyLine.nalloc, (void **)&(copy->SurveyLine.longitude_easting), error);
    if (status != MB_SUCCESS) {
      copy->SurveyLine.n = 0;
      copy->SurveyLine.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0;i<copy->SurveyLine.n;i++) {
      copy->SurveyLine.latitude_northing[i] = store->SurveyLine.latitude_northing[i];
      copy->SurveyLine.longitude_easting[i] = store->SurveyLine.longitude_easting[i];
    }
  }

  /* Navigation (record 1015) */
  copy->Navigation = store->Navigation;

  /* Attitude (record 1016) */
  //copy->Attitude = store->Attitude;
  copy->Attitude.header = store->Attitude.header;
  copy->Attitude.n = store->Attitude.n;
  if (status == MB_SUCCESS && (copy->Attitude.nalloc < copy->Attitude.n * sizeof(float))) {
    copy->Attitude.nalloc = copy->Attitude.n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Attitude.nalloc, (void **)&(copy->Attitude.delta_time), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Attitude.nalloc, (void **)&(copy->Attitude.roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Attitude.nalloc, (void **)&(copy->Attitude.pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Attitude.nalloc, (void **)&(copy->Attitude.heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Attitude.nalloc, (void **)&(copy->Attitude.heading), error);
    if (status != MB_SUCCESS) {
      copy->Attitude.n = 0;
      copy->Attitude.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    for (int i = 0;i<copy->Attitude.n;i++) {
      copy->Attitude.delta_time[i] = store->Attitude.delta_time[i];
      copy->Attitude.roll[i] = store->Attitude.roll[i];
      copy->Attitude.pitch[i] = store->Attitude.pitch[i];
      copy->Attitude.heave[i] = store->Attitude.heave[i];
      copy->Attitude.heading[i] = store->Attitude.heading[i];
    }
  }

  /* Pan Tilt (record 1017) */
  copy->PanTilt = store->PanTilt;

  /* Sonar Installation Identifiers (record 1020) */
  copy->SonarInstallationIDs = store->SonarInstallationIDs;

  /* Mystery (record 1022) */
  copy->Mystery = store->Mystery;

  /* Sonar Pipe Environment (record 2004) */
  //copy->SonarPipeEnvironment = store->SonarPipeEnvironment;
  copy->SonarPipeEnvironment.header = store->SonarPipeEnvironment.header;
  copy->SonarPipeEnvironment.pipe_number = store->SonarPipeEnvironment.pipe_number;
  copy->SonarPipeEnvironment.s7kTime = store->SonarPipeEnvironment.s7kTime;
  copy->SonarPipeEnvironment.multiping_number = store->SonarPipeEnvironment.multiping_number;
  copy->SonarPipeEnvironment.pipe_diameter = store->SonarPipeEnvironment.pipe_diameter;
  copy->SonarPipeEnvironment.sound_velocity = store->SonarPipeEnvironment.sound_velocity;
  copy->SonarPipeEnvironment.sample_rate = store->SonarPipeEnvironment.sample_rate;
  copy->SonarPipeEnvironment.finished = store->SonarPipeEnvironment.finished;
  copy->SonarPipeEnvironment.points_number = store->SonarPipeEnvironment.points_number;
  copy->SonarPipeEnvironment.n = store->SonarPipeEnvironment.n;
  for (int i = 0; i < MIN(copy->SonarPipeEnvironment.n, 5); i++) {
    copy->SonarPipeEnvironment.x[i] = store->SonarPipeEnvironment.x[i];
    copy->SonarPipeEnvironment.y[i] = store->SonarPipeEnvironment.y[i];
    copy->SonarPipeEnvironment.z[i] = store->SonarPipeEnvironment.z[i];
    copy->SonarPipeEnvironment.angle[i] = store->SonarPipeEnvironment.angle[i];
    copy->SonarPipeEnvironment.sample_number[i] = store->SonarPipeEnvironment.sample_number[i];
  }

  /* Contact Output (record 3001) */
  copy->ContactOutput = store->ContactOutput;

    /* Processed sidescan - MB-System extension to 7k format (record 3199) */
  copy->ProcessedSideScan = store->ProcessedSideScan;

  /* Reson 7k sonar settings (record 7000) */
  copy->SonarSettings = store->SonarSettings;

  /* Reson 7k configuration (record 7001) */
  //copy->Configuration = store->Configuration;
  copy->Configuration.header = store->Configuration.header;
  copy->Configuration.serial_number = store->Configuration.serial_number;
  copy->Configuration.number_devices = store->Configuration.number_devices;
  for (unsigned int i = 0;i<copy->Configuration.number_devices;i++) {
    copy->Configuration.device[i].magic_number = store->Configuration.device[i].magic_number;
    memcpy(copy->Configuration.device[i].description, store->Configuration.device[i].description, 60);
    copy->Configuration.device[i].description[60] = '\0';
    copy->Configuration.device[i].alphadata_card = store->Configuration.device[i].alphadata_card;
    copy->Configuration.device[i].serial_number = store->Configuration.device[i].serial_number;
    copy->Configuration.device[i].info_length = store->Configuration.device[i].info_length;
    if (status == MB_SUCCESS && (copy->Configuration.device[i].info_alloc < copy->Configuration.device[i].info_length)) {
      copy->Configuration.device[i].info_alloc = copy->Configuration.device[i].info_length + 1;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Configuration.device[i].info_alloc, (void **)&(copy->Configuration.device[i].info), error);
      if (status != MB_SUCCESS) {
        copy->Configuration.device[i].info_length = 0;
        copy->Configuration.device[i].info_alloc = 0;
      }
    }
    if (status == MB_SUCCESS) {
      strncpy(copy->Configuration.device[i].info, store->Configuration.device[i].info, copy->Configuration.device[i].info_length);
      copy->Configuration.device[i].info[copy->Configuration.device[i].info_length] = '\0';
    }
  }

  /* Reson 7k match filter (record 7002) */
  copy->MatchFilter = store->MatchFilter;

  /* Reson 7k firmware and hardware configuration (record 7003) */
  //copy->FirmwareHardwareConfiguration = store->FirmwareHardwareConfiguration;
  copy->FirmwareHardwareConfiguration.header = store->FirmwareHardwareConfiguration.header;
  copy->FirmwareHardwareConfiguration.device_count = store->FirmwareHardwareConfiguration.device_count;
  copy->FirmwareHardwareConfiguration.info_length = store->FirmwareHardwareConfiguration.info_length;
  if (status == MB_SUCCESS && (copy->FirmwareHardwareConfiguration.info_alloc < store->FirmwareHardwareConfiguration.info_length+1)) {
    copy->FirmwareHardwareConfiguration.info_alloc = store->FirmwareHardwareConfiguration.info_length + 1;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->FirmwareHardwareConfiguration.info_alloc, (void **)&(copy->FirmwareHardwareConfiguration.info), error);
    if (status != MB_SUCCESS) {
      copy->FirmwareHardwareConfiguration.device_count = 0;
      copy->FirmwareHardwareConfiguration.info_alloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    strncpy(copy->FirmwareHardwareConfiguration.info, store->FirmwareHardwareConfiguration.info, copy->FirmwareHardwareConfiguration.info_length);
    copy->FirmwareHardwareConfiguration.info[copy->FirmwareHardwareConfiguration.info_length] = '\0';
  }

  /* Reson 7k beam geometry (record 7004) */
  copy->BeamGeometry = store->BeamGeometry;

  /* Reson 7k bathymetry (record 7006) */
  copy->Bathymetry = store->Bathymetry;

  /* Reson 7k Side Scan Data (record 7007) */
  //copy->SideScan = store->SideScan;

  /* Reson 7k Generic Water Column data (record 7008) */
  //copy->WaterColumn = store->WaterColumn;

  /* Reson 7k Vertical Depth data (record 7009) */
  //copy->VerticalDepth = store->VerticalDepth;

  /* Reson 7k TVG data (record 7010) */
  //copy->TVG = store->TVG;

  /* Reson 7k image data (record 7011) */
  //copy->Image = store->Image;
  copy->Image.header = store->Image.header;
  copy->Image.ping_number = store->Image.ping_number;
  copy->Image.multi_ping = store->Image.multi_ping;
  copy->Image.width = store->Image.width;
  copy->Image.height = store->Image.height;
  copy->Image.color_depth = store->Image.color_depth;
  copy->Image.reserved = store->Image.reserved;
  copy->Image.compression = store->Image.compression;
  copy->Image.samples = store->Image.samples;
  copy->Image.flag = store->Image.flag;
  copy->Image.rx_delay = store->Image.rx_delay;
  memcpy(copy->Image.reserved2, store->Image.reserved2, 6 * sizeof(u32));
  if (status == MB_SUCCESS && (copy->Image.nalloc < copy->Image.width * copy->Image.height * copy->Image.color_depth)) {
    copy->Image.nalloc = copy->Image.width * copy->Image.height * copy->Image.color_depth;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, copy->Image.nalloc, (void **)&(copy->Image.image), error);
    if (status != MB_SUCCESS) {
      copy->Image.samples = 0;
      copy->Image.nalloc = 0;
    }
  }
  if (status == MB_SUCCESS) {
    memcpy(copy->Image.image, store->Image.image, copy->Image.width * copy->Image.height * copy->Image.color_depth);
  }

  /* Ping motion (record 7012) */
  //copy->PingMotion = store->PingMotion;

  /* Reson 7k Adaptive Gate (record 7014) */
  //copy->AdaptiveGate = store->AdaptiveGate;

  /* Detection setup (record 7017) */
  copy->DetectionDataSetup = store->DetectionDataSetup;

  /* Reson 7k Beamformed Data (record 7018) */
  //copy->Beamformed = store->Beamformed;

  /* Reson 7k Vernier Processing Data Raw (record 7019) */
  //copy->VernierProcessingDataRaw = store->VernierProcessingDataRaw;

  /* Reson 7k BITE (record 7021) */
  //copy->BITE = store->BITE;

  /* Reson 7k sonar source version (record 7022) */
  copy->SonarSourceVersion = store->SonarSourceVersion;

  /* Reson 7k 8k wet end version (record 7023) */
  copy->WetEndVersion8k = store->WetEndVersion8k;

  /* Reson 7k raw detection (record 7027) */
  copy->RawDetection = store->RawDetection;

  /* Reson 7k snippet (record 7028) */
  //copy->Snippet = store->Snippet;

  /* Reson 7k vernier Processing Data Filtered (Record 7029) */
  copy->VernierProcessingDataFiltered = store->VernierProcessingDataFiltered;

  /* Reson 7k sonar installation parameters (record 7030) */
  copy->InstallationParameters = store->InstallationParameters;

  /* Reson 7k BITE summary (Record 7031) */
  copy->BITESummary = store->BITESummary;

  /* Reson 7k Compressed Beamformed Magnitude Data (Record 7041) */
  //copy->CompressedBeamformedMagnitude = store->CompressedBeamformedMagnitude;

  /* Reson 7k Compressed Water Column Data (Record 7042) */
  //copy->CompressedWaterColumn = store->CompressedWaterColumn;

  /* Reson 7k Segmented Raw Detection Data (Record 7047) */
  //copy->SegmentedRawDetection = store->SegmentedRawDetection;

  /* Reson 7k Calibrated Beam Data (Record 7048) */
  //copy->CalibratedBeam = store->CalibratedBeam;

  /* Reson 7k System Events (part of Record 7050) */
  //copy->SystemEvents = store->SystemEvents;

  /* Reson 7k system event (record 7051) */
  //copy->SystemEventMessage = store->SystemEventMessage;

  /* Reson 7k RDR Recording Status (Record 7052) */
  //copy->RDRRecordingStatus = store->RDRRecordingStatus;

  /* Reson 7k Subscriptions (part of Record 7053) */
  //copy->Subscriptions = store->Subscriptions;

  /* Reson 7k System Events (Record 7054) */
  copy->RDRStorageRecording = store->RDRStorageRecording;

  /* Reson 7k Calibration Status (Record 7055) */
  copy->CalibrationStatus = store->CalibrationStatus;

  /* Reson 7k Calibrated Sidescan Data (record 7057) */
  //copy->CalibratedSideScan = store->CalibratedSideScan;

  /* Reson 7k Snippet Backscattering Strength (Record 7058) */
  //copy->SnippetBackscatteringStrength = store->SnippetBackscatteringStrength;

  /* Reson 7k MB2 Specific Status (Record 7059) */
  copy->MB2Status = store->MB2Status;

  /* Reson 7k file header (record 7200) */
  copy->FileHeader = store->FileHeader;

  /* Reson 7k File Catalog (Record 7300) */
  //copy->FileCatalog = store->FileCatalog;

  /* Reson 7k Time Message (Record 7400) */
  copy->TimeMessage = store->TimeMessage;

  /* Reson 7k Remote Control (Record 7500) */
  copy->RemoteControl = store->RemoteControl;

  /* Reson 7k Remote Control Acknowledge (Record 7501) */
  copy->RemoteControlAcknowledge = store->RemoteControlAcknowledge;

  /* Reson 7k Remote Control Not Acknowledge (Record 7502) */
  copy->RemoteControlNotAcknowledge = store->RemoteControlNotAcknowledge;

  /* Reson 7k remote control sonar settings (record 7503) */
  copy->RemoteControlSonarSettings = store->RemoteControlSonarSettings;

  /* Reson 7k Common System Settings (Record 7504) */
  copy->CommonSystemSettings = store->CommonSystemSettings;

  /* Reson 7k SV Filtering (record 7510) */
  copy->SVFiltering = store->SVFiltering;

  /* Reson 7k System Lock Status (record 7511) */
  copy->SystemLockStatus = store->SystemLockStatus;

  /* Reson 7k Sound Velocity (record 7610) */
  copy->SoundVelocity = store->SoundVelocity;

  /* Reson 7k Absorption Loss (record 7611) */
  copy->AbsorptionLoss = store->AbsorptionLoss;

  /* Reson 7k Spreading Loss (record 7612) */
  copy->SpreadingLoss = store->SpreadingLoss;

  /* Reson 7k Profile Average Salinity (record 7613) */
  copy->ProfileAverageSalinity = store->ProfileAverageSalinity;

  /* Reson 7k Profile Average Temperature (record 7614) */
  copy->ProfileAverageTemperature = store->ProfileAverageTemperature;

  /* record counting variables */
  copy->nrec_read = store->nrec_read;
  copy->nrec_write = store->nrec_write;
  copy->nrec_ReferencePoint = store->nrec_ReferencePoint;
  copy->nrec_UncalibratedSensorOffset = store->nrec_UncalibratedSensorOffset;
  copy->nrec_CalibratedSensorOffset = store->nrec_CalibratedSensorOffset;
  copy->nrec_Position = store->nrec_Position;
  copy->nrec_CustomAttitude = store->nrec_CustomAttitude;
  copy->nrec_Tide = store->nrec_Tide;
  copy->nrec_Altitude = store->nrec_Altitude;
  copy->nrec_MotionOverGround = store->nrec_MotionOverGround;
  copy->nrec_Depth = store->nrec_Depth;
  copy->nrec_SoundVelocityProfile = store->nrec_SoundVelocityProfile;
  copy->nrec_CTD = store->nrec_CTD;
  copy->nrec_Geodesy = store->nrec_Geodesy;
  copy->nrec_RollPitchHeave = store->nrec_RollPitchHeave;
  copy->nrec_Heading = store->nrec_Heading;
  copy->nrec_SurveyLine = store->nrec_SurveyLine;
  copy->nrec_Navigation = store->nrec_Navigation;
  copy->nrec_Attitude = store->nrec_Attitude;
  copy->nrec_PanTilt = store->nrec_PanTilt;
  copy->nrec_SonarInstallationIDs = store->nrec_SonarInstallationIDs;
  copy->nrec_SonarPipeEnvironment = store->nrec_SonarPipeEnvironment;
  copy->nrec_ContactOutput = store->nrec_ContactOutput;
  copy->nrec_ProcessedSideScan = store->nrec_ProcessedSideScan;
  copy->nrec_SonarSettings = store->nrec_SonarSettings;
  copy->nrec_Configuration = store->nrec_Configuration;
  copy->nrec_MatchFilter = store->nrec_MatchFilter;
  copy->nrec_FirmwareHardwareConfiguration = store->nrec_FirmwareHardwareConfiguration;
  copy->nrec_BeamGeometry = store->nrec_BeamGeometry;
  copy->nrec_Bathymetry = store->nrec_Bathymetry;
  copy->nrec_SideScan = store->nrec_SideScan;
  copy->nrec_WaterColumn = store->nrec_WaterColumn;
  copy->nrec_VerticalDepth = store->nrec_VerticalDepth;
  copy->nrec_TVG = store->nrec_TVG;
  copy->nrec_Image = store->nrec_Image;
  copy->nrec_PingMotion = store->nrec_PingMotion;
  copy->nrec_AdaptiveGate = store->nrec_AdaptiveGate;
  copy->nrec_DetectionDataSetup = store->nrec_DetectionDataSetup;
  copy->nrec_Beamformed = store->nrec_Beamformed;
  copy->nrec_VernierProcessingDataRaw = store->nrec_VernierProcessingDataRaw;
  copy->nrec_BITE = store->nrec_BITE;
  copy->nrec_SonarSourceVersion = store->nrec_SonarSourceVersion;
  copy->nrec_WetEndVersion8k = store->nrec_WetEndVersion8k;
  copy->nrec_RawDetection = store->nrec_RawDetection;
  copy->nrec_Snippet = store->nrec_Snippet;
  copy->nrec_VernierProcessingDataFiltered = store->nrec_VernierProcessingDataFiltered;
  copy->nrec_InstallationParameters = store->nrec_InstallationParameters;
  copy->nrec_BITESummary = store->nrec_BITESummary;
  copy->nrec_CompressedBeamformedMagnitude = store->nrec_CompressedBeamformedMagnitude;
  copy->nrec_CompressedWaterColumn = store->nrec_CompressedWaterColumn;
  copy->nrec_SegmentedRawDetection = store->nrec_SegmentedRawDetection;
  copy->nrec_CalibratedBeam = store->nrec_CalibratedBeam;
  copy->nrec_SystemEvents = store->nrec_SystemEvents;
  copy->nrec_SystemEventMessage = store->nrec_SystemEventMessage;
  copy->nrec_RDRRecordingStatus = store->nrec_RDRRecordingStatus;
  copy->nrec_Subscriptions = store->nrec_Subscriptions;
  copy->nrec_RDRStorageRecording = store->nrec_RDRStorageRecording;
  copy->nrec_CalibrationStatus = store->nrec_CalibrationStatus;
  copy->nrec_CalibratedSideScan = store->nrec_CalibratedSideScan;
  copy->nrec_SnippetBackscatteringStrength = store->nrec_SnippetBackscatteringStrength;
  copy->nrec_MB2Status = store->nrec_MB2Status;
  copy->nrec_FileHeader = store->nrec_FileHeader;
  copy->nrec_FileCatalog = store->nrec_FileCatalog;
  copy->nrec_TimeMessage = store->nrec_TimeMessage;
  copy->nrec_RemoteControl = store->nrec_RemoteControl;
  copy->nrec_RemoteControlAcknowledge = store->nrec_RemoteControlAcknowledge;
  copy->nrec_RemoteControlNotAcknowledge = store->nrec_RemoteControlNotAcknowledge;
  copy->nrec_RemoteControlSonarSettings = store->nrec_RemoteControlSonarSettings;
  copy->nrec_CommonSystemSettings = store->nrec_CommonSystemSettings;
  copy->nrec_SVFiltering = store->nrec_SVFiltering;
  copy->nrec_SystemLockStatus = store->nrec_SystemLockStatus;
  copy->nrec_SoundVelocity = store->nrec_SoundVelocity;
  copy->nrec_AbsorptionLoss = store->nrec_AbsorptionLoss;
  copy->nrec_SpreadingLoss = store->nrec_SpreadingLoss;
  copy->nrec_ProfileAverageSalinity = store->nrec_ProfileAverageSalinity;
  copy->nrec_ProfileAverageTemperature = store->nrec_ProfileAverageTemperature;

  /* Reson 7k file header (record 7200) */
  copy->FileHeader = store->FileHeader;

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
int mbsys_reson7k3_makess_source(
    int verbose, void *mbio_ptr, void *store_ptr, int source,
    int pixel_size_set,  // TODO(schwehr): bool
    double *pixel_size,
    int swath_width_set,  // TODO(schwehr): bool
    double *swath_width, int pixel_int, int *error) {
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

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  // This mbsys_reson7k3_makess() function generates a multibeam pseudo-sidescan
  // from the best available backscatter data. This sidescan is structured as
  // an array of pixels laid out with uniform acrosstrack spacing onto the
  // bathymetry. There are a variety of backscatter records that may be present
  // in a s7k data stream; if this function is called without the source record
  // specified, then the selection priority for which backscatter to use is:
  //        1) R7KRECID_SnippetBackscatteringStrength 7058
  //        2) R7KRECID_Snippet 7028
  //        3) R7KRECID_CalibratedSideScan 7057
  //        4) R7KRECID_SideScan 7007

  /* if necessary pick a source for the backscatter */
  if (store->kind == MB_DATA_DATA && source == R7KRECID_None) {
    if (store->read_SnippetBackscatteringStrength)
      source = R7KRECID_SnippetBackscatteringStrength;
    else if (store->read_Snippet)
      source = R7KRECID_Snippet;
    else if (store->read_CalibratedSideScan)
      source = R7KRECID_CalibratedSideScan;
    else if (store->read_SideScan)
      source = R7KRECID_SideScan;
  }

  s7k3_bathydata *bathydata = NULL;
  s7k3_rawdetectiondata *rawdetectiondata = NULL;
  s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata = NULL;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata = NULL;
  s7k3_snippetdata *snippetdata = NULL;
  s7k3_snippetbackscatteringstrengthdata *snippetbackscatteringstrengthdata = NULL;
  int nss = 0;
  int ss_cnt[MBSYS_RESON7K_MAX_PIXELS];
  double ss[MBSYS_RESON7K_MAX_PIXELS];
  double ssacrosstrack[MBSYS_RESON7K_MAX_PIXELS];
  double ssalongtrack[MBSYS_RESON7K_MAX_PIXELS];
  double bathsort[MBSYS_RESON7K_MAX_BEAMS];
  mb_u_char beamflag[MBSYS_RESON7K_MAX_BEAMS];
  double pixel_size_calc;
  double ss_spacing, ss_spacing_use;
  double soundspeed;
  int iminxtrack;
  double minxtrack;
  double maxxtrack;
  int nrangetable;
  double rangetable[MBSYS_RESON7K_MAX_SOUNDINGS];
  double acrosstracktable[MBSYS_RESON7K_MAX_SOUNDINGS], acrosstracktablemin;
  double alongtracktable[MBSYS_RESON7K_MAX_SOUNDINGS];
  int irangenadir, irange;
  // bool found;
  int pixel_int_use;
  int nsample_use, sample_start, sample_end;
  double angle, xtrack, xtrackss, ltrackss, factor;
  double range, beam_foot, beamwidth, sint;
  mb_u_char *data_uchar = NULL;
  unsigned short *data_ushort = NULL;
  unsigned int *data_uint = NULL;
  float *data_float = NULL;
  double *data_double = NULL;
  // int first, last;
  // k1, k2;
  mb_u_char *qualitycharptr;
  // int kk;

  s7k3_SonarSettings *SonarSettings = (s7k3_SonarSettings *)&store->SonarSettings;
  s7k3_BeamGeometry *BeamGeometry = (s7k3_BeamGeometry *)&store->BeamGeometry;
  s7k3_RawDetection *RawDetection = (s7k3_RawDetection *)&store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = (s7k3_SegmentedRawDetection *)&store->SegmentedRawDetection;
  s7k3_SideScan *SideScan = (s7k3_SideScan *)&store->SideScan;
  s7k3_Snippet *Snippet = (s7k3_Snippet *)&store->Snippet;
  s7k3_CalibratedSideScan *CalibratedSideScan = (s7k3_CalibratedSideScan *)&store->CalibratedSideScan;
  s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength = (s7k3_SnippetBackscatteringStrength *)&store->SnippetBackscatteringStrength;
  s7k3_ProcessedSideScan *ProcessedSideScan = (s7k3_ProcessedSideScan *)&store->ProcessedSideScan;
  s7k3_SoundVelocity *SoundVelocity = (s7k3_SoundVelocity *)&store->SoundVelocity;


  /* calculate SideScan from the desired source data if it is available */
  if (store->kind == MB_DATA_DATA
        && (store->read_RawDetection || store->read_SegmentedRawDetection)
        && ((source == R7KRECID_CalibratedSideScan && store->read_CalibratedSideScan) ||
            (source == R7KRECID_SnippetBackscatteringStrength && store->read_SnippetBackscatteringStrength) ||
            (source == R7KRECID_Snippet && store->read_Snippet) ||
            (source == R7KRECID_SideScan && store->read_SideScan))) {

    int nbathsort = 0;
    // Handle case of bathymetry in RawDetection 7027 records
    // - These records are output by Teledyne Reson multibeam sonars and
    //   are accompanied by BeamGeometry 7004 records
    // - The RawDetection records contain all valid soundings, not all beams
    // - The BeamGeometry defines the possible swath, the RawDetection contains
    //   the achieved swath.
    if (store->read_RawDetection) {
      // get acrosstract beam angle width from the center beam in the BeamGeometry record
      beamwidth = 2.0 * RTD * BeamGeometry->beamwidth_acrosstrack[BeamGeometry->number_beams / 2];

      /* get soundspeed */
      if (SonarSettings->sound_velocity > 0.0)
        soundspeed = SonarSettings->sound_velocity;
      else if (SoundVelocity->soundvelocity > 0.0)
        soundspeed = SoundVelocity->soundvelocity;
      else
        soundspeed = 1500.0;

      // get raw sample length in meters
      ss_spacing = 0.5 * soundspeed / SonarSettings->sample_rate;

      /* set number of pixels */
      nss = MIN(2 * BeamGeometry->number_beams, MBSYS_RESON7K_MAX_PIXELS);

      // get SideScan pixel size
      //if (!swath_width_set) {
      //  (*swath_width) = MAX(fabs(RTD * BeamGeometry->angle_acrosstrack[0]),
      //                       fabs(RTD * BeamGeometry->angle_acrosstrack[BeamGeometry->number_beams - 1]));
      //}
      if (!swath_width_set && RawDetection->number_beams > 0) {
        int ib1 = RawDetection->rawdetectiondata[0].beam_descriptor - 1;
        int ib2 = RawDetection->rawdetectiondata[RawDetection->number_beams - 1].beam_descriptor - 1;
        (*swath_width) = MAX(fabs(RTD * BeamGeometry->angle_acrosstrack[ib1]),
                             fabs(RTD * BeamGeometry->angle_acrosstrack[ib2]));
      }

      // get median depth relative to the sonar and check for min max xtrack
      nbathsort = 0;
      minxtrack = 0.0;
      maxxtrack = 0.0;
      iminxtrack = RawDetection->number_beams / 2;
      bool found = false;
      for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
        rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
        bathydata = &(RawDetection->bathydata[i]);
        qualitycharptr = (mb_u_char *)&(rawdetectiondata->quality);
        beamflag[i] = qualitycharptr[3];
        if (mb_beam_ok(beamflag[i])) {
          bathsort[nbathsort] = bathydata->depth - RawDetection->vehicle_depth;
          nbathsort++;
          if (!found || fabs(bathydata->acrosstrack) < minxtrack) {
            minxtrack = fabs(bathydata->acrosstrack);
            iminxtrack = i;
            found = true;
          }
          maxxtrack = MAX(fabs(bathydata->acrosstrack), maxxtrack);
        }
      }
    } // end handling bathymetry in RawDetection 7027 records

    // Handle case of bathymetry in SegmentedRawDetection 7047 records
    // - These records are output by Teledyne Atlas Hydrosweep multibeam sonars and
    //   are NOT accompanied by BeamGeometry 7004 records
    // - The RawDetection records contain all valid soundings from all transmit
    //   segments, not necessarily all formed beams
    else if (store->read_SegmentedRawDetection) {
      // get acrosstract beam angle width from the center beam in the BeamGeometry record
      beamwidth = 2.0 * RTD * SegmentedRawDetection->segmentedrawdetectiontxdata[0].rx_beam_width;

      /* get soundspeed */
      if (SegmentedRawDetection->sound_velocity > 0.0)
        soundspeed = SegmentedRawDetection->sound_velocity;
      else if (SonarSettings->sound_velocity > 0.0)
        soundspeed = SonarSettings->sound_velocity;
      else if (SoundVelocity->soundvelocity > 0.0)
        soundspeed = SoundVelocity->soundvelocity;
      else
        soundspeed = 1500.0;

      // get raw sample length in meters
      ss_spacing = 0.5 * soundspeed / SegmentedRawDetection->segmentedrawdetectiontxdata[0].sampling_rate;

      /* set number of pixels */
      nss = MBSYS_RESON7K_MAX_PIXELS / 2;

      // get SideScan pixel size
      if (!swath_width_set && SegmentedRawDetection->n_rx > 0) {
        double rx_angle1 = SegmentedRawDetection->segmentedrawdetectionrxdata[0].rx_angle_cross;
        double rx_angle2 = SegmentedRawDetection->segmentedrawdetectionrxdata[SegmentedRawDetection->n_rx - 1].rx_angle_cross;
        (*swath_width) = MAX(fabs(RTD * rx_angle1), fabs(RTD * rx_angle2));
      }

      // get median depth relative to the sonar and check for min max xtrack
      nbathsort = 0;
      minxtrack = 0.0;
      maxxtrack = 0.0;
      iminxtrack = SegmentedRawDetection->n_rx / 2;
      bool found = false;
      for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
        segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
        bathydata = &(SegmentedRawDetection->bathydata[i]);
        qualitycharptr = (mb_u_char *)&(segmentedrawdetectionrxdata->quality);
        beamflag[i] = qualitycharptr[3];
        if (mb_beam_ok(beamflag[i])) {
          bathsort[nbathsort] = bathydata->depth - SegmentedRawDetection->vehicle_depth;
          nbathsort++;
          if (!found || fabs(bathydata->acrosstrack) < minxtrack) {
            minxtrack = fabs(bathydata->acrosstrack);
            iminxtrack = i;
            found = true;
          }
          maxxtrack = MAX(fabs(bathydata->acrosstrack), maxxtrack);
        }
      }
    } // end handling bathymetry in SegmentedRawDetection 7047 records

    /* if bathymetry available calculate pixel size implied using swath width and nadir altitude */
    if (!pixel_size_set && nbathsort > 0) {
      qsort((void *)bathsort, nbathsort, sizeof(double), (void *)mb_double_compare);
      pixel_size_calc = 2.1 * tan(DTR * (*swath_width)) * bathsort[nbathsort / 2] / nss;

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

    /* zero the SideScan */
    for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      ss[i] = 0.0;
      ssacrosstrack[i] = 0.0;
      ssalongtrack[i] = 0.0;
      ss_cnt[i] = 0;
    }
    for (int i = 0; i < nss; i++) {
      ssacrosstrack[i] = (*pixel_size) * (double)(i - (nss / 2));
    }
    
    // Loop over raw backscatter or SideScan from the desired source,
    // putting each raw sample into the binning arrays. The possible
    // source records are:
    //        1) R7KRECID_SnippetBackscatteringStrength 7058
    //        2) R7KRECID_Snippet 7028
    //        3) R7KRECID_CalibratedSideScan 7057
    //        4) R7KRECID_SideScan 7007

    // use SnippetBackscatteringStrength 7058
    if (source == R7KRECID_SnippetBackscatteringStrength) {

      // loop over the snippets - we have to identify which sounding is associated
      // with each snippet - we only use snippets from non-null and unflagged
      // soundings - the snippet samples are located using the location of the
      // associated sounding
//if (store->read_RawDetection)
//mbsys_reson7k3_print_RawDetection(verbose, RawDetection, error);
//else if (store->read_SegmentedRawDetection)
//mbsys_reson7k3_print_SegmentedRawDetection(verbose, SegmentedRawDetection, error);
//mbsys_reson7k3_print_SnippetBackscatteringStrength(verbose, SnippetBackscatteringStrength, error);
      int ibeamdetectindex = 0;

      for (int i = 0; i < SnippetBackscatteringStrength->number_beams; i++) {
        snippetbackscatteringstrengthdata = (s7k3_snippetbackscatteringstrengthdata *) &(SnippetBackscatteringStrength->snippetbackscatteringstrengthdata[i]);
        bool processbeam = false;

        // Deal with case of RawDetection record
        if (store->read_RawDetection) {

          // search RawDetection record for the associated sounding
          bool found = false;
          for (unsigned int j = ibeamdetectindex; j < RawDetection->number_beams && !found; j++) {
            if ((RawDetection->rawdetectiondata[j].beam_descriptor == snippetbackscatteringstrengthdata->beam_number)
                && ((RawDetection->rawdetectiondata[j].flags & 0x40) == 0)) {
              ibeamdetectindex = j;
              found = true;
            }
          }
          // Now get altitude, xtrack, range, and angle from the sounding detection
          if (found && mb_beam_ok(beamflag[ibeamdetectindex])) {
            processbeam = true;
            rawdetectiondata = &(RawDetection->rawdetectiondata[ibeamdetectindex]);
            bathydata = &(RawDetection->bathydata[ibeamdetectindex]);
            // const double altitude = bathydata->depth - RawDetection->vehicle_depth;
            xtrack = bathydata->acrosstrack;
            range = 0.5 * soundspeed * rawdetectiondata->detection_point
                                      / RawDetection->sampling_rate;
            angle = RTD * rawdetectiondata->rx_angle;
          }
        }

        // Deal with case of SegmentedRawDetection record
        else if (store->read_SegmentedRawDetection) {

          // search SegmentedRawDetection record for the associated sounding
          bool found = false;
          for (unsigned int j = ibeamdetectindex; j < SegmentedRawDetection->n_rx && !found; j++) {
            if ((SegmentedRawDetection->segmentedrawdetectionrxdata[j].beam_number == snippetbackscatteringstrengthdata->beam_number)
                && ((SegmentedRawDetection->segmentedrawdetectionrxdata[j].flags2 & 0x4000) == 0)) {
              ibeamdetectindex = j;
              found = true;
            }
          }

          // Now get altitude, xtrack, range, and angle from the sounding detection
          if (found && mb_beam_ok(beamflag[ibeamdetectindex])) {
            processbeam = true;
            segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[ibeamdetectindex]);
            segmentedrawdetectiontxdata = &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment-1]);
            bathydata = &(SegmentedRawDetection->bathydata[ibeamdetectindex]);
            // const double altitude = bathydata->depth - SegmentedRawDetection->vehicle_depth;
            xtrack = bathydata->acrosstrack;
            range = 0.5 * soundspeed * segmentedrawdetectionrxdata->detection_point
                                      / segmentedrawdetectiontxdata->sampling_rate;
            angle = RTD * segmentedrawdetectionrxdata->rx_angle_cross;
          }
        }

        // Define the snippet samples to use and calculate the geometry
        if (processbeam) {
          // int nsample = snippetbackscatteringstrengthdata->end_sample - snippetbackscatteringstrengthdata->begin_sample + 1;
          beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
          sint = fabs(sin(DTR * angle));
          nsample_use = beam_foot / ss_spacing;
          if (sint < nsample_use * ss_spacing / beam_foot)
            ss_spacing_use = beam_foot / nsample_use;
          else
            ss_spacing_use = ss_spacing / sint;
          sample_start = MAX(((int)snippetbackscatteringstrengthdata->bottom_sample - (nsample_use / 2)),
                             (int)snippetbackscatteringstrengthdata->begin_sample);
          sample_end = MIN(((int)snippetbackscatteringstrengthdata->bottom_sample + (nsample_use / 2)),
                           (int)snippetbackscatteringstrengthdata->end_sample);

          for (int k = sample_start; k <= sample_end; k++) {
            if (xtrack < 0.0)
              xtrackss = xtrack - ss_spacing_use * (k - (int)snippetbackscatteringstrengthdata->bottom_sample);
            else
              xtrackss = xtrack + ss_spacing_use * (k - (int)snippetbackscatteringstrengthdata->bottom_sample);
            int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
            kk = MIN(MAX(0, kk), nss - 1);
            ss[kk] += (double)snippetbackscatteringstrengthdata->bs[k - (int)snippetbackscatteringstrengthdata->begin_sample];
            ssalongtrack[kk] += bathydata->alongtrack;
            ss_cnt[kk]++;
          }
        }
      }
    } // end use SnippetBackscatteringStrength 7058

    // use Snippet 7028
    else if (source == R7KRECID_Snippet) {

      // loop over the snippets - we have to identify which sounding is associated
      // with each snippet - we only use snippets from non-null and unflagged
      // soundings - the snippet samples are located using the location of the
      // associated sounding
//if (store->read_RawDetection)
//mbsys_reson7k3_print_RawDetection(verbose, RawDetection, error);
//else if (store->read_SegmentedRawDetection)
//mbsys_reson7k3_print_SegmentedRawDetection(verbose, SegmentedRawDetection, error);
//mbsys_reson7k3_print_Snippet(verbose, Snippet, error);
      int ibeamdetectindex = 0;
      for (int i = 0; i < Snippet->number_beams; i++) {
        snippetdata = (s7k3_snippetdata *) &(Snippet->snippetdata[i]);
        bool processbeam = false;

        // Deal with case of RawDetection record
        if (store->read_RawDetection) {

          // search RawDetection record for the associated sounding
          bool found = false;
          for (unsigned int j = ibeamdetectindex; j < RawDetection->number_beams && !found; j++) {
            if (RawDetection->rawdetectiondata[j].beam_descriptor == snippetdata->beam_number) {
              ibeamdetectindex = j;
              found = true;
            }
          }
          // Now get altitude, xtrack, range, and angle from the sounding detection
          if (found && ((RawDetection->rawdetectiondata[ibeamdetectindex].flags & 0x40) == 0)
              && mb_beam_ok(beamflag[ibeamdetectindex])) {
            processbeam = true;
            rawdetectiondata = &(RawDetection->rawdetectiondata[ibeamdetectindex]);
            bathydata = &(RawDetection->bathydata[ibeamdetectindex]);
            // const double altitude = bathydata->depth - RawDetection->vehicle_depth;
            xtrack = bathydata->acrosstrack;
            range = 0.5 * soundspeed * rawdetectiondata->detection_point
                                      / RawDetection->sampling_rate;
            angle = RTD * rawdetectiondata->rx_angle;
          }
        }

        // Deal with case of SegmentedRawDetection record
        else if (store->read_SegmentedRawDetection) {

          // search SegmentedRawDetection record for the associated sounding
          bool found = false;
          for (unsigned int j = ibeamdetectindex; j < SegmentedRawDetection->n_rx && !found; j++) {
            if (SegmentedRawDetection->segmentedrawdetectionrxdata[j].beam_number == snippetdata->beam_number
                && (SegmentedRawDetection->segmentedrawdetectionrxdata[j].flags2 & 0x4000) == 0) {
              ibeamdetectindex = j;
              found = true;
            }
          }

          // Now get altitude, xtrack, range, and angle from the sounding detection
          if (found && mb_beam_ok(beamflag[ibeamdetectindex])) {
            processbeam = true;
            segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[ibeamdetectindex]);
            segmentedrawdetectiontxdata = &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment-1]);
            bathydata = &(SegmentedRawDetection->bathydata[ibeamdetectindex]);
            // const double altitude = bathydata->depth - SegmentedRawDetection->vehicle_depth;
            xtrack = bathydata->acrosstrack;
            range = 0.5 * soundspeed * segmentedrawdetectionrxdata->detection_point
                                      / segmentedrawdetectiontxdata->sampling_rate;
            angle = RTD * segmentedrawdetectionrxdata->rx_angle_cross;
          }
        }

        // Define the snippet samples to use and calculate the geometry
        if (processbeam) {
          // int nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
          beam_foot = range * sin(DTR * beamwidth) / cos(DTR * angle);
          sint = fabs(sin(DTR * angle));
          nsample_use = beam_foot / ss_spacing;
          if (sint < nsample_use * ss_spacing / beam_foot)
            ss_spacing_use = beam_foot / nsample_use;
          else
            ss_spacing_use = ss_spacing / sint;
          sample_start = MAX(((int)snippetdata->detect_sample - (nsample_use / 2)),
                             (int)snippetdata->begin_sample);
          sample_end = MIN(((int)snippetdata->detect_sample + (nsample_use / 2)),
                           (int)snippetdata->end_sample);
          data_ushort = (unsigned short *)snippetdata->amplitude;
          data_uint = (unsigned int *)snippetdata->amplitude;

          for (int k = sample_start; k <= sample_end; k++) {
            if (xtrack < 0.0)
              xtrackss = xtrack - ss_spacing_use * (k - (int)snippetdata->detect_sample);
            else
              xtrackss = xtrack + ss_spacing_use * (k - (int)snippetdata->detect_sample);
            int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
            kk = MIN(MAX(0, kk), nss - 1);
            if ((Snippet->flags & 0x01) != 0) {
              ss[kk] += (double)data_uint[k - (int)snippetdata->begin_sample];
            }
            else {
              ss[kk] += (double)data_ushort[k - (int)snippetdata->begin_sample];
            }
            ssalongtrack[kk] += bathydata->alongtrack;
            ss_cnt[kk]++;
          }
        }
      }
    } // end use Snippet 7028

    // use CalibratedSideScan 7057
    else if (source == R7KRECID_CalibratedSideScan) {
      /* get acrosstrack distance versus range table from RawDetection */
      nrangetable = 0;
      irangenadir = 0;
      if (store->read_RawDetection) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);
          if (mb_beam_ok(beamflag[i])) {
            rangetable[nrangetable] = rawdetectiondata->detection_point
                                        / RawDetection->sampling_rate;
            acrosstracktable[nrangetable] = bathydata->acrosstrack;
            alongtracktable[nrangetable] = bathydata->alongtrack;
            if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
              irangenadir = nrangetable;
              acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
            }
            nrangetable++;
          }
        }
      } else if (store->read_SegmentedRawDetection) {
        for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          segmentedrawdetectiontxdata = &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment-1]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);
          if (mb_beam_ok(beamflag[i])) {
            rangetable[nrangetable] = segmentedrawdetectionrxdata->detection_point
                                        / segmentedrawdetectiontxdata->sampling_rate;
            acrosstracktable[nrangetable] = bathydata->acrosstrack;
            alongtracktable[nrangetable] = bathydata->alongtrack;
            if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
              irangenadir = nrangetable;
              acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
            }
            nrangetable++;
          }
        }
      }

      /* lay out port side */
      data_float = (float *)CalibratedSideScan->port_data;
      data_double = (double *)CalibratedSideScan->port_data;
      sample_start = rangetable[irangenadir] * SonarSettings->sample_rate;
      sample_end = MIN(rangetable[0] * SonarSettings->sample_rate, CalibratedSideScan->samples - 1);
      irange = irangenadir;
      for (int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)SonarSettings->sample_rate);
        bool found = false;
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
          if (CalibratedSideScan->bytes_persample == 4)
            ss[kk] += (double)data_float[i];
          else
            ss[kk] += (double)data_double[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }

      /* lay out starboard side */
      data_float = (float *)CalibratedSideScan->stbd_data;
      data_double = (double *)CalibratedSideScan->stbd_data;
      sample_start = rangetable[irangenadir] * SonarSettings->sample_rate;
      sample_end = MIN(rangetable[nrangetable - 1] * SonarSettings->sample_rate, CalibratedSideScan->samples - 1);
      irange = irangenadir;
      for (int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)SonarSettings->sample_rate);
        bool found = false;
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
          if (CalibratedSideScan->bytes_persample == 4)
            ss[kk] += (double)data_float[i];
          else
            ss[kk] += (double)data_double[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }
    } // end use CalibratedSideScan 7057

    // use SideScan 7007
    else if (source == R7KRECID_SideScan) {
      /* get acrosstrack distance versus range table from RawDetection */
      nrangetable = 0;
      irangenadir = 0;
      if (store->read_RawDetection) {
        for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
          rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
          bathydata = &(RawDetection->bathydata[i]);
          if (mb_beam_ok(beamflag[i])) {
            rangetable[nrangetable] = rawdetectiondata->detection_point
                                        / RawDetection->sampling_rate;
            acrosstracktable[nrangetable] = bathydata->acrosstrack;
            alongtracktable[nrangetable] = bathydata->alongtrack;
            if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
              irangenadir = nrangetable;
              acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
            }
            nrangetable++;
          }
        }
      } else if (store->read_SegmentedRawDetection) {
        for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
          segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
          segmentedrawdetectiontxdata = &(SegmentedRawDetection->segmentedrawdetectiontxdata[segmentedrawdetectionrxdata->used_segment-1]);
          bathydata = &(SegmentedRawDetection->bathydata[i]);
          if (mb_beam_ok(beamflag[i])) {
            rangetable[nrangetable] = segmentedrawdetectionrxdata->detection_point
                                        / segmentedrawdetectiontxdata->sampling_rate;
            acrosstracktable[nrangetable] = bathydata->acrosstrack;
            alongtracktable[nrangetable] = bathydata->alongtrack;
            if (nrangetable == 0 || fabs(acrosstracktable[nrangetable]) < acrosstracktablemin) {
              irangenadir = nrangetable;
              acrosstracktablemin = fabs(acrosstracktable[nrangetable]);
            }
            nrangetable++;
          }
        }
      }
//for (int i=0; i<nrangetable; i++){
//fprintf(stderr, "RangeTable: %d %11.6f %11.3f %11.3f\n", i, rangetable[i], acrosstracktable[i], alongtracktable[i]);
//}
      /* lay out port side */
      data_uchar = (mb_u_char *)SideScan->port_data;
      data_ushort = (unsigned short *)SideScan->port_data;
      data_uint = (unsigned int *)SideScan->port_data;
      sample_start = rangetable[irangenadir] * SonarSettings->sample_rate;
      sample_end = MIN(rangetable[0] * SonarSettings->sample_rate, SideScan->number_samples - 1);
      irange = irangenadir;
//fprintf(stderr, "\nPORT SIDE: samples: %d %d %d  sample_rate:%f pixel_size:%f\n",
//sample_start, sample_end, SideScan->number_samples, SonarSettings->sample_rate, *pixel_size);
      for (int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)SonarSettings->sample_rate);
        bool found = false;
        for (int j = irange; j > 0 && !found; j--) {
          if (range >= rangetable[j] && range < rangetable[j - 1]) {
            irange = j;
            found = true;
          }
        }
        factor = (range - rangetable[irange]) / (rangetable[irange - 1] - rangetable[irange]);
        xtrackss = acrosstracktable[irange] + factor * (acrosstracktable[irange - 1] - acrosstracktable[irange]);
        ltrackss = alongtracktable[irange] + factor * (alongtracktable[irange - 1] - alongtracktable[irange]);
//fprintf(stderr, "port %d: xtrack:%11.3f ltrack:%11.3f\n", i, xtrackss, ltrackss);
        const int kk = nss / 2 + (int)(xtrackss / (*pixel_size));
        if (kk >= 0 && kk < nss) {
          if (SideScan->sample_size == 1)
            ss[kk] += (double)data_uchar[i];
          else if (SideScan->sample_size == 2)
            ss[kk] += (double)data_ushort[i];
          else
            ss[kk] += (double)data_uint[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }

      /* lay out starboard side */
      data_uchar = (mb_u_char *)SideScan->stbd_data;
      data_ushort = (unsigned short *)SideScan->stbd_data;
      data_uint = (unsigned int *)SideScan->stbd_data;
      sample_start = rangetable[irangenadir] * SonarSettings->sample_rate;
      sample_end = MIN(rangetable[nrangetable - 1] * SonarSettings->sample_rate, SideScan->number_samples - 1);
      irange = irangenadir;
      for (int i = sample_start; i < sample_end; i++) {
        range = ((double)i) / ((double)SonarSettings->sample_rate);
        bool found = false;
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
          if (SideScan->sample_size == 1)
            ss[kk] += (double)data_uchar[i];
          else if (SideScan->sample_size == 2)
            ss[kk] += (double)data_ushort[i];
          else
            ss[kk] += (double)data_uint[i];
          ssalongtrack[kk] += ltrackss;
          ss_cnt[kk]++;
        }
      }
    } // end use SideScan 7007

    /* average the SideScan */
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

    /* interpolate the SideScan */
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

    /* embed the SideScan into the processed SideScan record */
    store->read_ProcessedSideScan = true;
    if (store->read_RawDetection) {
      ProcessedSideScan->header = RawDetection->header;
      ProcessedSideScan->serial_number = RawDetection->serial_number;
      ProcessedSideScan->ping_number = RawDetection->ping_number;
      ProcessedSideScan->multi_ping = RawDetection->multi_ping;
      ProcessedSideScan->pixelwidth = *pixel_size;
      ProcessedSideScan->sensordepth = RawDetection->vehicle_depth;
      ProcessedSideScan->altitude = RawDetection->bathydata[iminxtrack].depth
                                    - ProcessedSideScan->sensordepth;
    }
    else if (store->read_SegmentedRawDetection) {
      ProcessedSideScan->header = SegmentedRawDetection->header;
      ProcessedSideScan->serial_number = SegmentedRawDetection->serial_number;
      ProcessedSideScan->ping_number = SegmentedRawDetection->ping_number;
      ProcessedSideScan->multi_ping = SegmentedRawDetection->multi_ping;
      ProcessedSideScan->pixelwidth = *pixel_size;
      ProcessedSideScan->sensordepth = SegmentedRawDetection->vehicle_depth;
      ProcessedSideScan->altitude = SegmentedRawDetection->bathydata[iminxtrack].depth
                                    - ProcessedSideScan->sensordepth;
    }
    ProcessedSideScan->header.Offset = 60;
    ProcessedSideScan->header.Size = MBSYS_RESON7K_RECORDHEADER_SIZE
                                      + MBSYS_RESON7K_RECORDTAIL_SIZE
                                      + R7KHDRSIZE_ProcessedSideScan + nss * 8;
    ProcessedSideScan->header.OptionalDataOffset = 0;
    ProcessedSideScan->header.OptionalDataIdentifier = 0;
    ProcessedSideScan->header.RecordType = R7KRECID_ProcessedSideScan;
    ProcessedSideScan->recordversion = 1;
    ProcessedSideScan->ss_source = source;
    ProcessedSideScan->number_pixels = nss;
    ProcessedSideScan->ss_type = MB_SIDESCAN_LINEAR;
    ProcessedSideScan->pixelwidth = *pixel_size;
    for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      ProcessedSideScan->sidescan[i] = ss[i];
      ProcessedSideScan->alongtrack[i] = ssalongtrack[i];
    }

    if (verbose >= 2) {
      fprintf(stderr, "\ndbg2  SideScan regenerated in <%s>\n", __func__);
      fprintf(stderr, "dbg2       pixels_ss:  %d\n", nss);
      for (int i = 0; i < nss; i++)
        fprintf(stderr, "dbg2       pixel:%4d  cnt:%3d  ss:%10f  xtrack:%10f  ltrack:%10f\n", i, ss_cnt[i], ss[i],
                ssacrosstrack[i], ssalongtrack[i]);
    }
  }

  /* else unable to regenerate sidescan, output an empty record */
  else {
    /* embed the SideScan into the processed SideScan record */
    store->read_ProcessedSideScan = true;
    if (store->read_RawDetection) {
      minxtrack = 0.0;
      iminxtrack = RawDetection->number_beams / 2;
      bool found = false;
      for (unsigned int i = 0; i < RawDetection->number_beams; i++) {
        rawdetectiondata = &(RawDetection->rawdetectiondata[i]);
        bathydata = &(RawDetection->bathydata[i]);
        qualitycharptr = (mb_u_char *)&(rawdetectiondata->quality);
        beamflag[i] = qualitycharptr[3];
        if (mb_beam_ok(beamflag[i])) {
          if (!found || fabs(bathydata->acrosstrack) < minxtrack) {
            minxtrack = fabs(bathydata->acrosstrack);
            iminxtrack = i;
            found = true;
          }
        }
      }
      ProcessedSideScan->header = RawDetection->header;
      ProcessedSideScan->serial_number = RawDetection->serial_number;
      ProcessedSideScan->ping_number = RawDetection->ping_number;
      ProcessedSideScan->multi_ping = RawDetection->multi_ping;
      ProcessedSideScan->ss_source = source;
      ProcessedSideScan->number_pixels = MIN(2 * BeamGeometry->number_beams, MBSYS_RESON7K_MAX_PIXELS);
      ProcessedSideScan->ss_type = MB_SIDESCAN_LINEAR;
      ProcessedSideScan->pixelwidth = *pixel_size;
      ProcessedSideScan->sensordepth = RawDetection->vehicle_depth;
      ProcessedSideScan->altitude = RawDetection->bathydata[iminxtrack].depth
                                    - ProcessedSideScan->sensordepth;

    }
    else if (store->read_SegmentedRawDetection) {
      minxtrack = 0.0;
      iminxtrack = SegmentedRawDetection->n_rx / 2;
      bool found = false;
      for (unsigned int i = 0; i < SegmentedRawDetection->n_rx; i++) {
        segmentedrawdetectionrxdata = &(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
        bathydata = &(SegmentedRawDetection->bathydata[i]);
        qualitycharptr = (mb_u_char *)&(segmentedrawdetectionrxdata->quality);
        beamflag[i] = qualitycharptr[3];
        if (mb_beam_ok(beamflag[i])) {
          if (!found || fabs(bathydata->acrosstrack) < minxtrack) {
            minxtrack = fabs(bathydata->acrosstrack);
            iminxtrack = i;
            found = true;
          }
        }
      }
      ProcessedSideScan->header = SegmentedRawDetection->header;
      ProcessedSideScan->serial_number = SegmentedRawDetection->serial_number;
      ProcessedSideScan->ping_number = SegmentedRawDetection->ping_number;
      ProcessedSideScan->multi_ping = SegmentedRawDetection->multi_ping;
      ProcessedSideScan->ss_source = source;
      ProcessedSideScan->number_pixels = MBSYS_RESON7K_MAX_PIXELS / 2;
      ProcessedSideScan->ss_type = MB_SIDESCAN_LINEAR;
      ProcessedSideScan->pixelwidth = *pixel_size;
      ProcessedSideScan->sensordepth = SegmentedRawDetection->vehicle_depth;
      ProcessedSideScan->altitude = SegmentedRawDetection->bathydata[iminxtrack].depth
                                    - ProcessedSideScan->sensordepth;
    }
    ProcessedSideScan->header.Offset = 60;
    ProcessedSideScan->header.Size = MBSYS_RESON7K_RECORDHEADER_SIZE
                                      + MBSYS_RESON7K_RECORDTAIL_SIZE
                                      + R7KHDRSIZE_ProcessedSideScan + nss * 8;
    ProcessedSideScan->header.OptionalDataOffset = 0;
    ProcessedSideScan->header.OptionalDataIdentifier = 0;
    ProcessedSideScan->header.RecordType = R7KRECID_ProcessedSideScan;
    ProcessedSideScan->recordversion = 1;
    for (int i = 0; i < MBSYS_RESON7K_MAX_PIXELS; i++) {
      ProcessedSideScan->sidescan[i] = MB_SIDESCAN_NULL;
      ProcessedSideScan->alongtrack[i] = 0.0;
    }

    if (verbose >= 2) {
      fprintf(stderr, "\ndbg2  SideScan regenerated in <%s> - but all values null for lack of expected source data\n", __func__);
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
int mbsys_reson7k3_makess(int verbose, void *mbio_ptr, void *store_ptr, int pixel_size_set, double *pixel_size,
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

  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  // check for a prior sidescan source choice
  int source = store->ProcessedSideScan.ss_source;
  if (source != R7KRECID_SnippetBackscatteringStrength
      && source != R7KRECID_Snippet
      && source != R7KRECID_CalibratedSideScan
      && source != R7KRECID_SideScan) {
      source = R7KRECID_None;
  }

  int status = mbsys_reson7k3_makess_source(verbose, mbio_ptr, store_ptr, source,
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
