/*--------------------------------------------------------------------
 *    The MB-system:  mbr_reson7kr.c  4/4/2004
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
 * mbr_reson7kr.c contains the functions for reading and writing
 * multibeam data in the RESON7KR format.
 * These functions include:
 *   mbr_alm_reson7kr  - allocate read/write memory
 *   mbr_dem_reson7kr  - deallocate read/write memory
 *   mbr_rt_reson7kr  - read and translate data
 *   mbr_wt_reson7kr  - translate and write data
 *
 * Author:  D. W. Caress
 * Date:  April 4,2004
 *
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_reson7k.h"

#ifdef MBTRN_ENABLED
#include "r7k-reader.h"
#endif

/* turn on debug statements here */
//#define MBR_RESON7KR_DEBUG 1
//#define MBR_RESON7KR_DEBUG2 1
//#define MBR_RESON7KR_DEBUG3 1

/*--------------------------------------------------------------------*/
int mbr_info_reson7kr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* set format info parameters */
  *error = MB_ERROR_NO_ERROR;
  *system = MB_SYS_RESON7K;
  *beams_bath_max = MBSYS_RESON7K_MAX_BEAMS;
  *beams_amp_max = MBSYS_RESON7K_MAX_BEAMS;
  *pixels_ss_max = MBSYS_RESON7K_MAX_PIXELS;
  strncpy(format_name, "RESON7KR", MB_NAME_LENGTH);
  strncpy(system_name, "RESON7K", MB_NAME_LENGTH);
  strncpy(format_description,
          "Format name:          MBF_RESON7KR\nInformal Description: Reson 7K multibeam vendor format\nAttributes:           "
          "Reson 7K series multibeam sonars, \n                      bathymetry, amplitude, three channels sidescan, and "
          "subbottom\n                      up to 254 beams, variable pixels, binary, Reson.\n",
          MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_SINGLE;
  *variable_beams = true;
  *traveltime = true;
  *beam_flagging = true;
  *platform_source = MB_DATA_INSTALLATION;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_DATA;
  *svp_source = MB_DATA_VELOCITY_PROFILE;
  *beamwidth_xtrack = 1.0;
  *beamwidth_ltrack = 1.0;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", *system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", *beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", *beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", *pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", *numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", *filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", *variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", *traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", *beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", *platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", *nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", *sensordepth_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", *heading_source);
    fprintf(stderr, "dbg2       attitude_source:      %d\n", *attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", *svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", *beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", *beamwidth_ltrack);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_reson7kr(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  mb_io_ptr->structure_size = 0;
  mb_io_ptr->data_structure_size = 0;
  int status = mbsys_reson7k_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);
  int *save_flag = (int *)&mb_io_ptr->save_flag;
  int *current_ping = (int *)&mb_io_ptr->save14;
  int *last_ping = (int *)&mb_io_ptr->save1;
  int *recordid = (int *)&mb_io_ptr->save3;
  int *recordidlast = (int *)&mb_io_ptr->save4;
  char **bufferptr = (char **)&mb_io_ptr->saveptr1;
  // char *buffer = (char *)*bufferptr;
  unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
  char **buffersaveptr = (char **)&mb_io_ptr->saveptr2;
  // char *buffersave = (char *)*buffersaveptr;
  int *size = (int *)&mb_io_ptr->save8;
  int *nbadrec = (int *)&mb_io_ptr->save9;
  int *deviceid = (int *)&mb_io_ptr->save10;
  unsigned short *enumerator = (unsigned short *)&mb_io_ptr->save11;
  int *fileheaders = (int *)&mb_io_ptr->save12;
  double *pixel_size = (double *)&mb_io_ptr->saved1;
  double *swath_width = (double *)&mb_io_ptr->saved2;

  *current_ping = -1;
  *last_ping = -1;
  *save_flag = false;
  *recordid = R7KRECID_None;
  *recordidlast = R7KRECID_None;
  *bufferptr = NULL;
  *bufferalloc = 0;
  *size = 0;
  *nbadrec = 0;
  *deviceid = 0;
  *enumerator = 0;
  *fileheaders = 0;
  *pixel_size = 0.0;
  *swath_width = 0.0;

  /* allocate memory if necessary */
  if (status == MB_SUCCESS) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, MBSYS_RESON7K_BUFFER_STARTSIZE, (void **)bufferptr, error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, MBSYS_RESON7K_BUFFER_STARTSIZE, (void **)buffersaveptr, error);
    if (status == MB_SUCCESS)
      *bufferalloc = MBSYS_RESON7K_BUFFER_STARTSIZE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_reson7kr(int verbose, void *mbio_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* deallocate memory for data descriptor */
  int status = mbsys_reson7k_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* deallocate memory for reading/writing buffer */
  char **bufferptr = (char **)&mb_io_ptr->saveptr1;
  // char *buffer = (char *)*bufferptr;
  unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
  char **buffersaveptr = (char **)&mb_io_ptr->saveptr2;
  status = mb_freed(verbose, __FILE__, __LINE__, (void **)bufferptr, error);
  status = mb_freed(verbose, __FILE__, __LINE__, (void **)buffersaveptr, error);
  *bufferalloc = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_chk_header(int verbose, void *mbio_ptr, char *buffer, int *recordid, int *deviceid, unsigned short *enumerator,
                            int *size) {
  int status = MB_SUCCESS;
  unsigned short version = 0;
  unsigned short offset = 0;
  unsigned int sync = 0;
  unsigned short reserved = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:      %p\n", (void *)mbio_ptr);
  }

  /* get values to check */
  mb_get_binary_short(true, &buffer[0], &version);
  mb_get_binary_short(true, &buffer[2], &offset);
  mb_get_binary_int(true, &buffer[4], &sync);
  mb_get_binary_int(true, &buffer[8], size);
  mb_get_binary_int(true, &buffer[32], recordid);
  mb_get_binary_int(true, &buffer[36], deviceid);
  mb_get_binary_short(true, &buffer[40], &reserved);
  mb_get_binary_short(true, &buffer[42], enumerator);
#ifdef MBR_RESON7KR_DEBUG3
  fprintf(stderr, "\nChecking header in mbr_reson7kr_chk_header:\n");
  fprintf(stderr, "Version:      %4.4hX | %d\n", version, version);
  fprintf(stderr, "Offset:       %4.4hX | %d\n", offset, offset);
  fprintf(stderr, "Sync:         %4.4X | %d\n", sync, sync);
  fprintf(stderr, "Size:         %4.4X | %d\n", *size, *size);
  fprintf(stderr, "Record id:    %4.4X | %d\n", *recordid, *recordid);
  fprintf(stderr, "Device id:    %4.4X | %d\n", *deviceid, *deviceid);
  fprintf(stderr, "Reserved:     %4.4hX | %d\n", reserved, reserved);
  fprintf(stderr, "Enumerator:   %4.4hX | %d\n", *enumerator, *enumerator);
#endif

  /* reset enumerator if version 2 */
  if (version == 2)
    *enumerator = reserved;

  /* check sync */
  if (sync != 0x0000FFFF) {
    status = MB_FAILURE;
  }

  /* check recordid */
  else if (*recordid != R7KRECID_ReferencePoint && *recordid != R7KRECID_UncalibratedSensorOffset &&
           *recordid != R7KRECID_CalibratedSensorOffset && *recordid != R7KRECID_Position &&
           *recordid != R7KRECID_CustomAttitude && *recordid != R7KRECID_Tide && *recordid != R7KRECID_Altitude &&
           *recordid != R7KRECID_MotionOverGround && *recordid != R7KRECID_Depth &&
           *recordid != R7KRECID_SoundVelocityProfile && *recordid != R7KRECID_CTD && *recordid != R7KRECID_Geodesy &&
           *recordid != R7KRECID_RollPitchHeave && *recordid != R7KRECID_Heading && *recordid != R7KRECID_SurveyLine &&
           *recordid != R7KRECID_Navigation && *recordid != R7KRECID_Attitude && *recordid != R7KRECID_Rec1022 &&
           *recordid != R7KRECID_FSDWsidescan && *recordid != R7KRECID_FSDWsubbottom && *recordid != R7KRECID_Bluefin &&
           *recordid != R7KRECID_ProcessedSidescan && *recordid != R7KRECID_7kVolatileSonarSettings &&
           *recordid != R7KRECID_7kConfiguration && *recordid != R7KRECID_7kMatchFilter &&
           *recordid != R7KRECID_7kV2FirmwareHardwareConfiguration && *recordid != R7KRECID_7kBeamGeometry &&
           *recordid != R7KRECID_7kCalibrationData && *recordid != R7KRECID_7kBathymetricData &&
           *recordid != R7KRECID_7kBackscatterImageData && *recordid != R7KRECID_7kBeamData &&
           *recordid != R7KRECID_7kVerticalDepth && *recordid != R7KRECID_7kTVGData && *recordid != R7KRECID_7kImageData &&
           *recordid != R7KRECID_7kV2PingMotion && *recordid != R7KRECID_7kV2DetectionSetup &&
           *recordid != R7KRECID_7kV2BeamformedData && *recordid != R7KRECID_7kV2BITEData &&
           *recordid != R7KRECID_7kV27kCenterVersion && *recordid != R7KRECID_7kV28kWetEndVersion &&
           *recordid != R7KRECID_7kV2Detection && *recordid != R7KRECID_7kV2RawDetection &&
           *recordid != R7KRECID_7kV2SnippetData && *recordid != R7KRECID_7kCalibratedSnippetData &&
           *recordid != R7KRECID_7kInstallationParameters && *recordid != R7KRECID_7kSystemEventMessage &&
           *recordid != R7KRECID_7kDataStorageStatus && *recordid != R7KRECID_7kFileHeader && *recordid != R7KRECID_7kFileCatalog &&
           *recordid != R7KRECID_7kTriggerSequenceSetup && *recordid != R7KRECID_7kTriggerSequenceDone &&
           *recordid != R7KRECID_7kTimeMessage && *recordid != R7KRECID_7kRemoteControl &&
           *recordid != R7KRECID_7kRemoteControlAcknowledge && *recordid != R7KRECID_7kRemoteControlNotAcknowledge &&
           *recordid != R7KRECID_7kRemoteControlSonarSettings && *recordid != R7KRECID_7kReserved &&
           *recordid != R7KRECID_7kRoll && *recordid != R7KRECID_7kPitch && *recordid != R7KRECID_7kSoundVelocity &&
           *recordid != R7KRECID_7kAbsorptionLoss && *recordid != R7KRECID_7kSpreadingLoss &&
             *recordid != R7KRECID_7kFiller && *recordid != R7KRECID_8100SonarData) {
    status = MB_FAILURE;
  }
  else {
    status = MB_SUCCESS;

#ifdef MBR_RESON7KR_DEBUG2
    if (verbose > 0) {
      fprintf(stderr, "Good record id: %4.4X | %d", *recordid, *recordid);
      if (*recordid == R7KRECID_ReferencePoint)
        fprintf(stderr, " R7KRECID_ReferencePoint\n");
      if (*recordid == R7KRECID_UncalibratedSensorOffset)
        fprintf(stderr, " R7KRECID_UncalibratedSensorOffset\n");
      if (*recordid == R7KRECID_CalibratedSensorOffset)
        fprintf(stderr, " R7KRECID_CalibratedSensorOffset\n");
      if (*recordid == R7KRECID_Position)
        fprintf(stderr, " R7KRECID_Position\n");
      if (*recordid == R7KRECID_CustomAttitude)
        fprintf(stderr, " R7KRECID_CustomAttitude\n");
      if (*recordid == R7KRECID_Tide)
        fprintf(stderr, " R7KRECID_Tide\n");
      if (*recordid == R7KRECID_Altitude)
        fprintf(stderr, " R7KRECID_Altitude\n");
      if (*recordid == R7KRECID_MotionOverGround)
        fprintf(stderr, " R7KRECID_MotionOverGround\n");
      if (*recordid == R7KRECID_Depth)
        fprintf(stderr, " R7KRECID_Depth\n");
      if (*recordid == R7KRECID_SoundVelocityProfile)
        fprintf(stderr, " R7KRECID_SoundVelocityProfile\n");
      if (*recordid == R7KRECID_CTD)
        fprintf(stderr, " R7KRECID_CTD\n");
      if (*recordid == R7KRECID_Geodesy)
        fprintf(stderr, " R7KRECID_Geodesy\n");
      if (*recordid == R7KRECID_RollPitchHeave)
        fprintf(stderr, " R7KRECID_RollPitchHeave\n");
      if (*recordid == R7KRECID_Heading)
        fprintf(stderr, " R7KRECID_Heading\n");
      if (*recordid == R7KRECID_SurveyLine)
        fprintf(stderr, " R7KRECID_Heading\n");
      if (*recordid == R7KRECID_Navigation)
        fprintf(stderr, " R7KRECID_Heading\n");
      if (*recordid == R7KRECID_Attitude)
        fprintf(stderr, " R7KRECID_Attitude\n");
      if (*recordid == R7KRECID_Rec1022)
        fprintf(stderr, " R7KRECID_Rec1022\n");
      if (*recordid == R7KRECID_FSDWsidescan)
        fprintf(stderr, " R7KRECID_FSDWsidescan\n");
      if (*recordid == R7KRECID_FSDWsubbottom)
        fprintf(stderr, " R7KRECID_FSDWsubbottom\n");
      if (*recordid == R7KRECID_Bluefin)
        fprintf(stderr, " R7KRECID_Bluefin\n");
      if (*recordid == R7KRECID_ProcessedSidescan)
        fprintf(stderr, " R7KRECID_ProcessedSidescan\n");
      if (*recordid == R7KRECID_7kVolatileSonarSettings)
        fprintf(stderr, " R7KRECID_7kVolatileSonarSettings\n");
      if (*recordid == R7KRECID_7kConfiguration)
        fprintf(stderr, " R7KRECID_7kConfiguration\n");
      if (*recordid == R7KRECID_7kMatchFilter)
        fprintf(stderr, " R7KRECID_7kMatchFilter\n");
      if (*recordid == R7KRECID_7kV2FirmwareHardwareConfiguration)
        fprintf(stderr, " R7KRECID_7kV2FirmwareHardwareConfiguration\n");
      if (*recordid == R7KRECID_7kBeamGeometry)
        fprintf(stderr, " R7KRECID_7kBeamGeometry\n");
      if (*recordid == R7KRECID_7kCalibrationData)
        fprintf(stderr, " R7KRECID_7kCalibrationData\n");
      if (*recordid == R7KRECID_7kBathymetricData)
        fprintf(stderr, " R7KRECID_7kBathymetricData\n");
      if (*recordid == R7KRECID_7kBackscatterImageData)
        fprintf(stderr, " R7KRECID_7kBackscatterImageData\n");
      if (*recordid == R7KRECID_7kBeamData)
        fprintf(stderr, " R7KRECID_7kBeamData\n");
      if (*recordid == R7KRECID_7kVerticalDepth)
        fprintf(stderr, " R7KRECID_7kVerticalDepth\n");
      if (*recordid == R7KRECID_7kTVGData)
        fprintf(stderr, " R7KRECID_7kTVGData\n");
      if (*recordid == R7KRECID_7kImageData)
        fprintf(stderr, " R7KRECID_7kImageData\n");
      if (*recordid == R7KRECID_7kV2PingMotion)
        fprintf(stderr, " R7KRECID_7kV2PingMotion\n");
      if (*recordid == R7KRECID_7kV2DetectionSetup)
        fprintf(stderr, " R7KRECID_7kV2DetectionSetup\n");
      if (*recordid == R7KRECID_7kV2BeamformedData)
        fprintf(stderr, " R7KRECID_7kV2BeamformedData\n");
      if (*recordid == R7KRECID_7kV2BITEData)
        fprintf(stderr, " R7KRECID_7kV2BITEData\n");
      if (*recordid == R7KRECID_7kV27kCenterVersion)
        fprintf(stderr, " R7KRECID_7kV27kCenterVersion\n");
      if (*recordid == R7KRECID_7kV28kWetEndVersion)
        fprintf(stderr, " R7KRECID_7kV28kWetEndVersion\n");
      if (*recordid == R7KRECID_7kV2Detection)
        fprintf(stderr, " R7KRECID_7kV2Detection\n");
      if (*recordid == R7KRECID_7kV2RawDetection)
        fprintf(stderr, " R7KRECID_7kV2RawDetection\n");
      if (*recordid == R7KRECID_7kV2SnippetData)
        fprintf(stderr, " R7KRECID_7kV2SnippetData\n");
      if (*recordid == R7KRECID_7kCalibratedSnippetData)
        fprintf(stderr, " R&R7KRECID_7kCalibratedSnippetData\n");
      if (*recordid == R7KRECID_7kInstallationParameters)
        fprintf(stderr, " R7KRECID_7kInstallationParameters\n");
      if (*recordid == R7KRECID_7kSystemEventMessage)
        fprintf(stderr, "R7KRECID_7kSystemEventMessage\n");
      if (*recordid == R7KRECID_7kDataStorageStatus)
        fprintf(stderr, " R7KRECID_7kDataStorageStatus\n");
      if (*recordid == R7KRECID_7kFileHeader)
        fprintf(stderr, " R7KRECID_7kFileHeader\n");
      if (*recordid == R7KRECID_7kFileCatalog)
        fprintf(stderr, " R7KRECID_7kFileCatalog\n");
      if (*recordid == R7KRECID_7kTriggerSequenceSetup)
        fprintf(stderr, " R7KRECID_7kTriggerSequenceSetup\n");
      if (*recordid == R7KRECID_7kTriggerSequenceDone)
        fprintf(stderr, " R7KRECID_7kTriggerSequenceDone\n");
      if (*recordid == R7KRECID_7kTimeMessage)
        fprintf(stderr, " R7KRECID_7kTimeMessage\n");
      if (*recordid == R7KRECID_7kRemoteControl)
        fprintf(stderr, " R7KRECID_7kRemoteControl\n");
      if (*recordid == R7KRECID_7kRemoteControlAcknowledge)
        fprintf(stderr, " R7KRECID_7kRemoteControlAcknowledge\n");
      if (*recordid == R7KRECID_7kRemoteControlNotAcknowledge)
        fprintf(stderr, " R7KRECID_7kRemoteControlNotAcknowledge\n");
      if (*recordid == R7KRECID_7kRemoteControlSonarSettings)
        fprintf(stderr, " R7KRECID_7kRemoteControlSonarSettings\n");
      if (*recordid == R7KRECID_7kReserved)
        fprintf(stderr, " R7KRECID_7kReserved\n");
      if (*recordid == R7KRECID_7kRoll)
        fprintf(stderr, " R7KRECID_7kRoll\n");
      if (*recordid == R7KRECID_7kPitch)
        fprintf(stderr, " R7KRECID_7kPitch\n");
      if (*recordid == R7KRECID_7kSoundVelocity)
        fprintf(stderr, " R7KRECID_7kSoundVelocity\n");
      if (*recordid == R7KRECID_7kAbsorptionLoss)
        fprintf(stderr, " R7KRECID_7kAbsorptionLoss\n");
      if (*recordid == R7KRECID_7kSpreadingLoss)
        fprintf(stderr, " R7KRECID_7kSpreadingLoss\n");
      if (*recordid == R7KRECID_7kFiller)
        fprintf(stderr, " R7KRECID_7kFiller\n");
      if (*recordid == R7KRECID_8100SonarData)
        fprintf(stderr, " R7KRECID_8100SonarData\n");
    }
#endif
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Output arguments:\n");
    fprintf(stderr, "dbg2       recordid:      %d\n", *recordid);
    fprintf(stderr, "dbg2       deviceid:      %d\n", *deviceid);
    fprintf(stderr, "dbg2       enumerator:    %d\n", *enumerator);
    fprintf(stderr, "dbg2       size:          %d\n", *size);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_chk_pingnumber(int verbose, int recordid, char *buffer, int *ping_number) {
  int status = MB_SUCCESS;
  unsigned short offset = 0;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       recordid:      %d\n", recordid);
    fprintf(stderr, "dbg2       buffer:        %p\n", (void *)buffer);
  }

  /* get offset to data section */
  mb_get_binary_short(true, &buffer[2], &offset);

  /* check ping number if one of the ping records */
  if (recordid == R7KRECID_7kVolatileSonarSettings) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kMatchFilter) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kBathymetricData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kBackscatterImageData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kBeamData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kVerticalDepth) {
    index = offset + 8;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kTVGData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kImageData) {
    index = offset + 4;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2PingMotion) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2DetectionSetup) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2BeamformedData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2Detection) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2RawDetection) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kV2SnippetData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kCalibratedSnippetData) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_7kRemoteControlSonarSettings) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else if (recordid == R7KRECID_ProcessedSidescan) {
    index = offset + 12;
    mb_get_binary_int(true, &buffer[index], ping_number);
    status = MB_SUCCESS;
  }
  else {
    status = MB_FAILURE;
    *ping_number = 0;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Output arguments:\n");
    fprintf(stderr, "dbg2       ping_number:   %d\n", *ping_number);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_header(int verbose, char *buffer, unsigned int *index, s7k_header *header, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
  }

  /* extract the header */
  mb_get_binary_short(true, &buffer[*index], &(header->Version));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(header->Offset));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(header->SyncPattern));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(header->Size));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(header->OffsetToOptionalData));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(header->OptionalDataIdentifier));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(header->s7kTime.Year));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(header->s7kTime.Day));
  *index += 2;
  mb_get_binary_float(true, &buffer[*index], &(header->s7kTime.Seconds));
  *index += 4;
  header->s7kTime.Hours = (mb_u_char)buffer[*index];
  (*index)++;
  header->s7kTime.Minutes = (mb_u_char)buffer[*index];
  (*index)++;
  mb_get_binary_short(true, &buffer[*index], &(header->Reserved));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(header->RecordType));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(header->DeviceId));
  *index += 4;

    if (header->Version == 2) {
        mb_get_binary_short(true, &buffer[*index], &(header->SystemEnumerator));
        *index += 2;
    mb_get_binary_int(true, &buffer[*index], &(header->DataSetNumber));
        *index += 4;
    mb_get_binary_int(true, &buffer[*index], &(header->RecordNumber));
        *index += 4;
    for (int i = 0; i < 8; i++) {
      header->PreviousRecord[i] = buffer[*index];
      (*index)++;
    }
    for (int i = 0; i < 8; i++) {
      header->NextRecord[i] = buffer[*index];
      (*index)++;
    }
        mb_get_binary_short(true, &buffer[*index], &(header->Flags));
        *index += 2;
        mb_get_binary_short(true, &buffer[*index], &(header->Reserved3));
        *index += 2;
        header->Reserved2 = 0;
        header->Reserved4 = 0;
        header->FragmentedTotal = 0;
        header->FragmentNumber = 0;
    }

    else if (header->Version == 3) {
        mb_get_binary_short(true, &buffer[*index], &(header->Reserved2));
        *index += 2;
        mb_get_binary_short(true, &buffer[*index], &(header->SystemEnumerator));
        *index += 2;
        mb_get_binary_int(true, &buffer[*index], &(header->RecordNumber));
        *index += 4;
        mb_get_binary_short(true, &buffer[*index], &(header->Flags));
        *index += 2;
        mb_get_binary_short(true, &buffer[*index], &(header->Reserved3));
        *index += 2;
        for (int i = 0; i < 8; i++) {
            header->PreviousRecord[i] = 0;
            header->NextRecord[i] = 0;
        }
        header->Reserved4 = 0;
        header->FragmentedTotal = 0;
        header->FragmentNumber = 0;
    }

    else if (header->Version >= 4) {
        mb_get_binary_short(true, &buffer[*index], &(header->Reserved2));
        *index += 2;
        mb_get_binary_short(true, &buffer[*index], &(header->SystemEnumerator));
        *index += 2;
        mb_get_binary_int(true, &buffer[*index], &(header->RecordNumber));
        *index += 4;
        mb_get_binary_short(true, &buffer[*index], &(header->Flags));
        *index += 2;
        mb_get_binary_short(true, &buffer[*index], &(header->Reserved3));
        *index += 2;
    mb_get_binary_int(true, &buffer[*index], &(header->Reserved4));
    *index += 4;
    mb_get_binary_int(true, &buffer[*index], &(header->FragmentedTotal));
    *index += 4;
    mb_get_binary_int(true, &buffer[*index], &(header->FragmentNumber));
    *index += 4;
        for (int i = 0; i < 8; i++) {
            header->PreviousRecord[i] = 0;
            header->NextRecord[i] = 0;
        }
    }

  /* print out the results */
  /* mbsys_reson7k_print_header(verbose, header, error); */

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_reference(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_reference *reference;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  reference = &(store->reference);
  header = &(reference->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(reference->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(reference->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(reference->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(reference->water_z));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_ReferencePoint;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_ReferencePoint:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_reference(verbose, reference, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_sensoruncal(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_sensoruncal *sensoruncal;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  sensoruncal = &(store->sensoruncal);
  header = &(sensoruncal->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensoruncal->offset_yaw));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_UncalibratedSensorOffset;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_UncalibratedSensorOffset:      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_sensoruncal(verbose, sensoruncal, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_sensorcal(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_sensorcal *sensorcal;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  sensorcal = &(store->sensorcal);
  header = &(sensorcal->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sensorcal->offset_yaw));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_CalibratedSensorOffset;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_CalibratedSensorOffset:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_sensorcal(verbose, sensorcal, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_position(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_position *position;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  position = &(store->position);
  header = &(position->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(position->datum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(position->latency));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(position->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(position->longitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(position->height));
  index += 8;
  position->type = buffer[index];
  index++;
  position->utm_zone = buffer[index];
  index++;
  position->quality = buffer[index];
  index++;
  position->method = buffer[index];
  index++;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV1;
    store->type = R7KRECID_Position;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Position:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_position(verbose, position, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_customattitude(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_customattitude *customattitude;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  customattitude = &(store->customattitude);
  header = &(customattitude->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  customattitude->bitfield = (mb_u_char)buffer[index];
  index++;
  customattitude->reserved = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(customattitude->n));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(customattitude->frequency));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (customattitude->nalloc < customattitude->n) {
    data_size = customattitude->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->heading), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->pitchrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->rollrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->headingrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(customattitude->heaverate), error);
    if (status == MB_SUCCESS) {
      customattitude->nalloc = customattitude->n;
    }
    else {
      customattitude->nalloc = 0;
      customattitude->n = 0;
    }
  }

  if (customattitude->bitfield & 1)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->pitch[i]));
      index += 4;
    }
  if (customattitude->bitfield & 2)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->roll[i]));
      index += 4;
    }
  if (customattitude->bitfield & 4)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->heading[i]));
      index += 4;
    }
  if (customattitude->bitfield & 8)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->heave[i]));
      index += 4;
    }
  if (customattitude->bitfield & 16)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->pitchrate[i]));
      index += 4;
    }
  if (customattitude->bitfield & 32)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->rollrate[i]));
      index += 4;
    }
  if (customattitude->bitfield & 64)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->headingrate[i]));
      index += 4;
    }
  if (customattitude->bitfield & 128)
    for (int i = 0; i < customattitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(customattitude->heaverate[i]));
      index += 4;
    }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE;
    store->type = R7KRECID_CustomAttitude;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_CustomAttitude:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_customattitude(verbose, customattitude, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_tide(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_tide *tide;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  tide = &(store->tide);
  header = &(tide->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(tide->tide));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(tide->source));
  index += 2;
  tide->flags = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(tide->gauge));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(tide->datum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(tide->latency));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(tide->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(tide->longitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(tide->height));
  index += 8;
  tide->type = buffer[index];
  index++;
  tide->utm_zone = buffer[index];
  index++;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_TIDE;
    store->type = R7KRECID_Tide;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Tide:                          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_tide(verbose, tide, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_altitude(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_altitude *altitude;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  altitude = &(store->altitude);
  header = &(altitude->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(altitude->altitude));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ALTITUDE;
    store->type = R7KRECID_Altitude;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Altitude:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_altitude(verbose, altitude, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_motion(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_motion *motion;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  motion = &(store->motion);
  header = &(motion->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  motion->bitfield = (mb_u_char)buffer[index];
  index++;
  motion->reserved = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(motion->n));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(motion->frequency));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (motion->nalloc < motion->n) {
    data_size = motion->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->x), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->y), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->z), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->xa), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->ya), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(motion->za), error);
    if (status == MB_SUCCESS) {
      motion->nalloc = motion->n;
    }
    else {
      motion->nalloc = 0;
      motion->n = 0;
    }
  }

  if (motion->bitfield & 1) {
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->x[i]));
      index += 4;
    }
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->y[i]));
      index += 4;
    }
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->z[i]));
      index += 4;
    }
  }
  if (motion->bitfield & 2) {
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->xa[i]));
      index += 4;
    }
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->ya[i]));
      index += 4;
    }
    for (int i = 0; i < motion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(motion->za[i]));
      index += 4;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_MOTION;
    store->type = R7KRECID_MotionOverGround;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_MotionOverGround:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_motion(verbose, motion, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_depth(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_depth *depth;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  depth = &(store->depth);
  header = &(depth->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  depth->descriptor = (mb_u_char)buffer[index];
  index++;
  depth->correction = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(depth->reserved));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(depth->depth));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SENSORDEPTH;
    store->type = R7KRECID_Depth;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Depth:                         7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_depth(verbose, depth, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_svp(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_svp *svp;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  svp = &(store->svp);
  header = &(svp->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  svp->position_flag = (mb_u_char)buffer[index];
  index++;
  svp->reserved1 = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(svp->reserved2));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(svp->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(svp->longitude));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(svp->n));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (svp->nalloc < svp->n) {
    data_size = svp->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(svp->depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(svp->sound_velocity), error);
    if (status == MB_SUCCESS) {
      svp->nalloc = svp->n;
    }
    else {
      svp->nalloc = 0;
      svp->n = 0;
    }
  }

  for (unsigned int i = 0; i < svp->n; i++) {
    mb_get_binary_float(true, &buffer[index], &(svp->depth[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svp->sound_velocity[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_VELOCITY_PROFILE;
    store->type = R7KRECID_SoundVelocityProfile;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_SoundVelocityProfile:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_svp(verbose, svp, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_ctd(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_ctd *ctd;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  ctd = &(store->ctd);
  header = &(ctd->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(ctd->frequency));
  index += 4;
  ctd->velocity_source_flag = (mb_u_char)buffer[index];
  index++;
  ctd->velocity_algorithm = (mb_u_char)buffer[index];
  index++;
  ctd->conductivity_flag = (mb_u_char)buffer[index];
  index++;
  ctd->pressure_flag = (mb_u_char)buffer[index];
  index++;
  ctd->position_flag = (mb_u_char)buffer[index];
  index++;
  ctd->validity = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(ctd->reserved));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(ctd->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(ctd->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(ctd->sample_rate));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(ctd->n));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (ctd->nalloc < ctd->n) {
    data_size = ctd->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(ctd->conductivity_salinity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(ctd->temperature), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(ctd->pressure_depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(ctd->sound_velocity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(ctd->absorption), error);
    if (status == MB_SUCCESS) {
      ctd->nalloc = ctd->n;
    }
    else {
      ctd->nalloc = 0;
      ctd->n = 0;
    }
  }

  for (unsigned int i = 0; i < ctd->n; i++) {
    mb_get_binary_float(true, &buffer[index], &(ctd->conductivity_salinity[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(ctd->temperature[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(ctd->pressure_depth[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(ctd->sound_velocity[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(ctd->absorption[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_CTD;
    store->type = R7KRECID_CTD;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_CTD:                           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_ctd(verbose, ctd, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_geodesy(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_geodesy *geodesy;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  geodesy = &(store->geodesy);
  header = &(geodesy->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    geodesy->spheroid[i] = (mb_u_char)buffer[index];
    index++;
  }
  mb_get_binary_double(true, &buffer[index], &(geodesy->semimajoraxis));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->flattening));
  index += 8;
  for (int i = 0; i < 16; i++) {
    geodesy->reserved1[i] = (mb_u_char)buffer[index];
    index++;
  }
  for (int i = 0; i < 32; i++) {
    geodesy->datum[i] = (mb_u_char)buffer[index];
    index++;
  }
  mb_get_binary_int(true, &buffer[index], &(geodesy->calculation_method));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(geodesy->number_parameters));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(geodesy->dx));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->dy));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->dz));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->rx));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->ry));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->rz));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->scale));
  index += 8;
  for (int i = 0; i < 35; i++) {
    geodesy->reserved2[i] = (mb_u_char)buffer[index];
    index++;
  }
  for (int i = 0; i < 32; i++) {
    geodesy->grid_name[i] = (mb_u_char)buffer[index];
    index++;
  }
  geodesy->distance_units = (mb_u_char)buffer[index];
  index++;
  geodesy->angular_units = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_double(true, &buffer[index], &(geodesy->latitude_origin));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->central_meriidan));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->false_easting));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->false_northing));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(geodesy->central_scale_factor));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(geodesy->custum_identifier));
  index += 4;
  for (int i = 0; i < 50; i++) {
    geodesy->reserved3[i] = (mb_u_char)buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_Geodesy;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Geodesy:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_geodesy(verbose, geodesy, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_rollpitchheave(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_rollpitchheave *rollpitchheave;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  rollpitchheave = &(store->rollpitchheave);
  header = &(rollpitchheave->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(rollpitchheave->roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(rollpitchheave->pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(rollpitchheave->heave));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE;
    store->type = R7KRECID_RollPitchHeave;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_RollPitchHeave:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_rollpitchheave(verbose, rollpitchheave, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_heading(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_heading *heading;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  heading = &(store->heading);
  header = &(heading->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(heading->heading));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADING;
    store->type = R7KRECID_Heading;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Heading:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_heading(verbose, heading, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_surveyline(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_surveyline *surveyline;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  surveyline = &(store->surveyline);
  header = &(surveyline->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_short(true, &buffer[index], &(surveyline->n));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(surveyline->type));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(surveyline->turnradius));
  index += 4;
  for (int i = 0; i < 64; i++) {
    surveyline->name[i] = (char)buffer[index];
    index++;
  }

  /* make sure enough memory is allocated for channel data */
  if (surveyline->nalloc < surveyline->n) {
    data_size = surveyline->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(surveyline->latitude), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(surveyline->longitude), error);
    if (status == MB_SUCCESS) {
      surveyline->nalloc = surveyline->n;
    }
    else {
      surveyline->nalloc = 0;
      surveyline->n = 0;
    }
  }

  for (int i = 0; i < surveyline->n; i++) {
    mb_get_binary_double(true, &buffer[index], &(surveyline->latitude[i]));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(surveyline->longitude[i]));
    index += 8;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SURVEY_LINE;
    store->type = R7KRECID_SurveyLine;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_SurveyLine:                   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_surveyline(verbose, surveyline, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_navigation(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_navigation *navigation;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  navigation = &(store->navigation);
  header = &(navigation->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  navigation->vertical_reference = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_double(true, &buffer[index], &(navigation->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(navigation->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(navigation->position_accuracy));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(navigation->height));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(navigation->height_accuracy));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(navigation->speed));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(navigation->course));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(navigation->heading));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV3;
    store->type = R7KRECID_Navigation;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Navigation:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_navigation(verbose, navigation, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_attitude(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_attitude *attitude;
  unsigned int data_size;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  attitude = &(store->attitude);
  header = &(attitude->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  attitude->n = (mb_u_char)buffer[index];
  index++;

  /* make sure enough memory is allocated for channel data */
  if (attitude->nalloc < attitude->n) {
    data_size = attitude->n * sizeof(unsigned short);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(attitude->delta_time), error);
    data_size = attitude->n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(attitude->roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(attitude->pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(attitude->heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(attitude->heading), error);
    if (status == MB_SUCCESS) {
      attitude->nalloc = attitude->n;
    }
    else {
      attitude->nalloc = 0;
      attitude->n = 0;
    }
  }

  for (int i = 0; i < attitude->n; i++) {
    mb_get_binary_short(true, &buffer[index], &(attitude->delta_time[i]));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(attitude->roll[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(attitude->pitch[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(attitude->heave[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(attitude->heading[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE;
    store->type = R7KRECID_Attitude;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Attitude:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_attitude(verbose, attitude, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_rec1022(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_rec1022 *rec1022;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  rec1022 = &(store->rec1022);
  header = &(rec1022->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < R7KHDRSIZE_Rec1022; i++) {
    rec1022->data[i] = (mb_u_char)buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_RAW_LINE;
    store->type = R7KRECID_Rec1022;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_Rec1022:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_rec1022(verbose, rec1022, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwchannel(int verbose, int data_format, char *buffer, unsigned int *index, s7k_fsdwchannel *fsdwchannel,
                                int *error) {
  int status = MB_SUCCESS;
  unsigned int data_size = 0;
  short *shortptr = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       data_format:%d\n", data_format);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       fsdwchannel:%p\n", (void *)fsdwchannel);
  }

  /* extract the channel header */
  fsdwchannel->number = (mb_u_char)buffer[*index];
  (*index)++;
  fsdwchannel->type = (mb_u_char)buffer[*index];
  (*index)++;
  fsdwchannel->data_type = (mb_u_char)buffer[*index];
  (*index)++;
  fsdwchannel->polarity = (mb_u_char)buffer[*index];
  (*index)++;
  fsdwchannel->bytespersample = (mb_u_char)buffer[*index];
  (*index)++;
  for (int i = 0; i < 3; i++) {
    fsdwchannel->reserved1[i] = buffer[*index];
    (*index)++;
  }
  mb_get_binary_int(true, &buffer[*index], &(fsdwchannel->number_samples));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwchannel->start_time));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwchannel->sample_interval));
  *index += 4;
  mb_get_binary_float(true, &buffer[*index], &(fsdwchannel->range));
  *index += 4;
  mb_get_binary_float(true, &buffer[*index], &(fsdwchannel->voltage));
  *index += 4;
  for (int i = 0; i < 16; i++) {
    fsdwchannel->name[i] = buffer[*index];
    (*index)++;
  }
  for (int i = 0; i < 20; i++) {
    fsdwchannel->reserved2[i] = buffer[*index];
    (*index)++;
  }

  /* make sure enough memory is allocated for channel data */
  data_size = fsdwchannel->bytespersample * fsdwchannel->number_samples;
  if (fsdwchannel->data_alloc < data_size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(fsdwchannel->data), error);
    if (status != MB_SUCCESS)
      fsdwchannel->data_alloc = 0;
    else
      fsdwchannel->data_alloc = data_size;
  }

  /* copy over the data */
  if (status == MB_SUCCESS) {
    if (fsdwchannel->bytespersample == 1) {
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        fsdwchannel->data[i] = buffer[*index];
        (*index)++;
      }
    }
    else if (fsdwchannel->bytespersample == 2) {
      shortptr = (short *)fsdwchannel->data;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        /*srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_get_binary_short(true, &(buffer[*index]), &(shortptr[i]));
        *index += 2;
        /*ssptr = (short *) &(shortptr[i]);
        usptr = (unsigned short *) &(shortptr[i]);
        fprintf(stderr,"sample:%5d   raw:%6d %6d   swapped:%6d %6d\n",
        i,*srptr,*urptr,*ssptr,*usptr);*/
      }
    }
    else if (fsdwchannel->bytespersample == 4) {
      shortptr = (short *)fsdwchannel->data;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        /*srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_get_binary_short(true, &(buffer[*index]), &(shortptr[2 * i]));
        *index += 2;
        /*ssptr = (short *) &(shortptr[2*i]);
        usptr = (unsigned short *) &(shortptr[2*i]);
        fprintf(stderr,"sample:%5d   IMAGINARY: raw:%6d %6d   swapped:%6d %6d",
        i,*srptr,*urptr,*ssptr,*usptr);
        srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_get_binary_short(true, &(buffer[*index]), &(shortptr[2 * i + 1]));
        *index += 2;
        /*ssptr = (short *) &(shortptr[2*i+1]);
        usptr = (unsigned short *) &(shortptr[2*i+1]);
        fprintf(stderr,"    REAL: raw:%6d %6d   swapped:%6d %6d\n",
        *srptr,*urptr,*ssptr,*usptr);*/
      }
    }
  }

  /* print out the results */
  /*mbsys_reson7k_print_fsdwchannel(verbose, data_format, fsdwchannel, error);*/

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwssheader(int verbose, char *buffer, unsigned int *index, s7k_fsdwssheader *fsdwssheader, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:          %d\n", *index);
    fprintf(stderr, "dbg2       fsdwssheader:   %p\n", (void *)fsdwssheader);
  }

  /* extract the Edgetech sidescan header */
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->subsystem));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->channelNum));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->pingNum));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->packetNum));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->trigSource));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->samples));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->sampleInterval));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->startDepth));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->weightingFactor));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->ADCGain));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->ADCMax));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->rangeSetting));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->pulseID));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->markNumber));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->dataFormat));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->reserved));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->millisecondsToday));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->year));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->day));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->hour));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->minute));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->second));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->heading));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->pitch));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->roll));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->heave));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->yaw));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->depth));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwssheader->temperature));
  *index += 2;
  for (int i = 0; i < 2; i++) {
    fsdwssheader->reserved2[i] = buffer[*index];
    (*index)++;
  }
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->longitude));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwssheader->latitude));
  *index += 4;

  /* print out the results */
  /*mbsys_reson7k_print_fsdwssheader(verbose, fsdwssheader, error);*/

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsegyheader(int verbose, char *buffer, unsigned int *index, s7k_fsdwsegyheader *fsdwsegyheader, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:          %d\n", *index);
    fprintf(stderr, "dbg2       fsdwsegyheader: %p\n", (void *)fsdwsegyheader);
  }

  /* extract the Edgetech segy header */
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->sequenceNumber));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->startDepth));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->pingNum));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->channelNum));
  *index += 4;
  for (int i = 0; i < 6; i++) {
    mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unused1[i]));
    *index += 2;
  }
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->traceIDCode));
  *index += 2;
  for (int i = 0; i < 2; i++) {
    mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unused2[i]));
    *index += 2;
  }
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->dataFormat));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEAantennaeR));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEAantennaeO));
  *index += 2;
  for (int i = 0; i < 32; i++) {
    fsdwsegyheader->RS232[i] = buffer[*index];
    (*index)++;
  }
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->sourceCoordX));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->sourceCoordY));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->groupCoordX));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->groupCoordY));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->coordUnits));
  *index += 2;
  for (int i = 0; i < 24; i++) {
    fsdwsegyheader->annotation[i] = buffer[*index];
    (*index)++;
  }
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->samples));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->sampleInterval));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->ADCGain));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->pulsePower));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->correlated));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->startFreq));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->endFreq));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->sweepLength));
  *index += 2;
  for (int i = 0; i < 4; i++) {
    mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unused7[i]));
    *index += 2;
  }
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->aliasFreq));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->pulseID));
  *index += 2;
  for (int i = 0; i < 6; i++) {
    mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unused8[i]));
    *index += 2;
  }
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->year));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->day));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->hour));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->minute));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->second));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->timeBasis));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->weightingFactor));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unused9));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->heading));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->pitch));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->roll));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->temperature));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->heaveCompensation));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->trigSource));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->markNumber));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEAHour));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEAMinutes));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEASeconds));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEACourse));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEASpeed));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEADay));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->NMEAYear));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->millisecondsToday));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->ADCMax));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->calConst));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->vehicleID));
  *index += 2;
  for (int i = 0; i < 6; i++) {
    fsdwsegyheader->softwareVersion[i] = buffer[*index];
    (*index)++;
  }
  mb_get_binary_int(true, &buffer[*index], &(fsdwsegyheader->sphericalCorrection));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->packetNum));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->ADCDecimation));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->decimation));
  *index += 2;
  for (int i = 0; i < 7; i++) {
    mb_get_binary_short(true, &buffer[*index], &(fsdwsegyheader->unuseda[i]));
    *index += 2;
  }

  /* print out the results */
  /*mbsys_reson7k_print_fsdwsegyheader(verbose, fsdwsegyheader, error);*/

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsslo(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_i[7];
  double edgetech_time_d, s7k_time_d, bathy_time_d;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_fsdwss *fsdwsslo = &(store->fsdwsslo);
  s7k_header *header = &(fsdwsslo->header);
  s7kr_bathymetry *bathymetry = &(store->bathymetry);
  // s7kr_bluefin *bluefin = &(store->bluefin);

  /* extract the header */
  unsigned int index = 0;
  const int status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsslo->msec_timestamp));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsslo->ping_number));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsslo->number_channels));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsslo->total_bytes));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsslo->data_format));
  index += 4;
  index += 12;
  for (int i = 0; i < 2; i++) {
    s7k_fsdwchannel *fsdwchannel = &(fsdwsslo->channel[i]);
    mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsslo->data_format, buffer, &index, fsdwchannel, error);
  }
  s7k_fsdwssheader *fsdwssheader;
  for (int i = 0; i < 2; i++) {
    fsdwssheader = &(fsdwsslo->ssheader[i]);
    mbr_reson7kr_rd_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SIDESCAN2;
    store->type = R7KRECID_FSDWsidescan;
    store->sstype = R7KRECID_FSDWsidescanLo;

    /* get the time from the original Edgetech header */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = fsdwssheader->year;
    time_j[1] = fsdwssheader->day;
    time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
    time_j[3] = fsdwssheader->second;
    time_j[4] = 1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(edgetech_time_d));

    /* get the time from the 6046 datalogger header */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(s7k_time_d));

    /* get the time from the last bathymetry record */
    time_j[0] = bathymetry->header.s7kTime.Year;
    time_j[1] = bathymetry->header.s7kTime.Day;
    time_j[2] = 60 * bathymetry->header.s7kTime.Hours + bathymetry->header.s7kTime.Minutes;
    time_j[3] = (int)bathymetry->header.s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (bathymetry->header.s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(bathy_time_d));

/* figure out offset between 7k timestamp and Edgetech timestamp */
#ifdef MBR_RESON7KR_DEBUG2
    fprintf(stderr, "%s: 7k time offset: %f    7ktime:%f Edgetech:%f Bathymetry:%f\n", __func__,
            edgetech_time_d - bathy_time_d, s7k_time_d, edgetech_time_d, bathy_time_d);
#endif

    /* Use the Edgetech timestamp */
    store->time_d = edgetech_time_d;
    mb_get_date(verbose, store->time_d, store->time_i);

    /* use Edgetech time for early MBARI SBP missions with
        bad time synching, otherwise use 7K timestamp */
    /*if (header->s7kTime.Year == 2004)
        {
        /\* get the time from the original Edgetech header *\/
        time_j[0] = fsdwssheader->year;
        time_j[1] = fsdwssheader->day;
        time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
        time_j[3] = fsdwssheader->second;
        time_j[4] = 1000 * (fsdwssheader->millisecondsToday
                    - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
        }

    else
        {
        /\* get the time from the 6046 datalogger header *\/
        time_j[0] = header->s7kTime.Year;
        time_j[1] = header->s7kTime.Day;
        time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
        time_j[3] = (int) header->s7kTime.Seconds;
        time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
        }

    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));*/
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  for (int i = 0; i < fsdwsslo->number_channels; i++) {
    mb_get_date(verbose, s7k_time_d, time_i);
    s7k_fsdwchannel *fsdwchannel = &(fsdwsslo->channel[i]);
    s7k_fsdwssheader *fsdwssheader = &(fsdwsslo->ssheader[i]);
    fprintf(stderr,
            "R7KRECID_FSDWsidescanLo:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d "
            "%2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], fsdwssheader->year,
            fsdwssheader->day, fsdwssheader->hour, fsdwssheader->minute, fsdwssheader->second,
            fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday), fsdwsslo->ping_number,
            fsdwssheader->pingNum, fsdwchannel->number, fsdwssheader->channelNum, fsdwchannel->sample_interval,
            fsdwssheader->sampleInterval);
  }
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwss(verbose, fsdwsslo, error);

  /* get sonar_depth and altitude if possible */
  /*
      if (bluefin->nav[0].depth > 0.0)
          survey->sonar_depth = bluefin->nav[0].depth;
      if (bluefin->nav[0].altitude > 0.0)
          survey->sonar_altitude = bluefin->nav[0].altitude;
      if (bluefin->environmental[0].sound_speed > 0.0)
          sound_speed = bluefin->environmental[0].sound_speed;
      else
          sound_speed = 1500.0;
      half_sound_speed = 0.5 * sound_speed;
  */

  /* if necessary get first arrival and treat as altitude */
  /*
      if (survey->sonar_altitude == 0.0)
          {
          bottompick = fsdwchannel->number_samples;
          for (i=0;i<2;i++)
              {
              fsdwchannel = &(fsdwsslo->channel[i]);
              shortptr = (short *) fsdwchannel->data;
              ushortptr = (unsigned short *) fsdwchannel->data;
              if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
                  pickvalue = (double) ushortptr[0];
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
                  pickvalue = sqrt((double) (shortptr[0] * shortptr[0] + shortptr[1] * shortptr[1]));
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
                  pickvalue = fabs((double) ushortptr[0]);
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
                  pickvalue = fabs((double) ushortptr[0]);
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
                  pickvalue = (double) ushortptr[0];
              pickvalue = MAX(40 * pickvalue, 80.0);
              for (j=0;j<fsdwchannel->number_samples;j++)
                  {
                  if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
                      value = (double) ushortptr[j];
                  else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
                      value = sqrt((double) (shortptr[2*j] * shortptr[2*j] + shortptr[2*j+1] * shortptr[2*j+1]));
                  else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
                      value = (double) ushortptr[j];
                  else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
                      value = (double) ushortptr[j];
                  else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
                      value = (double) ushortptr[j];
                  if (bottompick > j && fabs(value) > pickvalue)
                      {
                      bottompick = j;
                      survey->sonar_altitude = half_sound_speed * bottompick * 0.000001 * fsdwchannel->sample_interval;
                      }
                  }
              }
          }
      else
          {
          bottompick = survey->sonar_altitude / (half_sound_speed * 0.000001 * fsdwchannel->sample_interval);
          }
  */

  /* insert sslo data into survey structure */
  /*
      tracemax = 0.0;
      pixelsize = (fsdwsslo->channel[0].range + fsdwsslo->channel[1].range) / 1024;
      for (j=0;j<1024;j++)
          {
          bin[j] = 0.0;
          nbin[j] = 0;
          }
      for (i=0;i<2;i++)
          {
          fsdwchannel = &(fsdwsslo->channel[i]);
          shortptr = (short *) fsdwchannel->data;
          ushortptr = (short *) fsdwchannel->data;
          survey->number_sslow_pixels = 1024;
          for (j=bottompick;j<fsdwchannel->number_samples;j++)
              {
              if (i == 0)
                  {
                  sign = -1.0;
                  k = fsdwchannel->number_samples - (j - bottompick);
                  }
              else
                  {
                  sign = 1.0;
                  k = fsdwchannel->number_samples + (j - bottompick);
                  }
              if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ENVELOPE)
                  value = (double) ushortptr[j];
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_ANALYTIC)
                  value = sqrt((double) (shortptr[2*j] * shortptr[2*j] + shortptr[2*j+1] * shortptr[2*j+1]));
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_RAW)
                  value = (double) ushortptr[j];
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_REALANALYTIC)
                  value = (double) ushortptr[j];
              else if (fsdwsslo->data_format == EDGETECH_TRACEFORMAT_PIXEL)
                  value = (double) ushortptr[j];
              tracemax = MAX(tracemax, value);
              range = half_sound_speed * j * 0.000001 * fsdwchannel->sample_interval;
              xtrack = sign * sqrt(fabs(range * range
                          - survey->sonar_altitude * survey->sonar_altitude));
              k = (xtrack / pixelsize) + 512;
              if (k < 0) k = 0;
              if (k > 1023) k = 1023;
              bin[k] += value * value;
              nbin[k]++;
              }
          }
      for (j=0;j<1024;j++)
          {
          if (nbin[j] > 0)
              survey->sslow[j] = sqrt(bin[j]) / nbin[j];
          else
              survey->sslow[j] = 0.0;
          survey->sslow_acrosstrack[j] = pixelsize * (j - 512);;
          survey->sslow_alongtrack[j] = 0.0;
          }
  */

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsshi(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fsdwss *fsdwsshi;
  s7k_fsdwchannel *fsdwchannel;
  s7k_fsdwssheader *fsdwssheader;
  s7kr_bathymetry *bathymetry;
  unsigned int index = 0;
  int time_i[7];
  double edgetech_time_d, s7k_time_d, bathy_time_d;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  fsdwsshi = &(store->fsdwsshi);
  header = &(fsdwsshi->header);
  bathymetry = &(store->bathymetry);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsshi->msec_timestamp));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsshi->ping_number));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsshi->number_channels));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsshi->total_bytes));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsshi->data_format));
  index += 4;
  index += 12;
  for (int i = 0; i < 2; i++) {
    fsdwchannel = &(fsdwsshi->channel[i]);
    mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsshi->data_format, buffer, &index, fsdwchannel, error);
  }
  for (int i = 0; i < 2; i++) {
    fsdwssheader = &(fsdwsshi->ssheader[i]);
    mbr_reson7kr_rd_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SIDESCAN3;
    store->type = R7KRECID_FSDWsidescan;
    store->sstype = R7KRECID_FSDWsidescanHi;

    /* get the time from the original Edgetech header */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = fsdwssheader->year;
    time_j[1] = fsdwssheader->day;
    time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
    time_j[3] = fsdwssheader->second;
    time_j[4] = 1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(edgetech_time_d));

    /* get the time from the 6046 datalogger header */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(s7k_time_d));

    /* get the time from the last bathymetry record */
    time_j[0] = bathymetry->header.s7kTime.Year;
    time_j[1] = bathymetry->header.s7kTime.Day;
    time_j[2] = 60 * bathymetry->header.s7kTime.Hours + bathymetry->header.s7kTime.Minutes;
    time_j[3] = (int)bathymetry->header.s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (bathymetry->header.s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(bathy_time_d));

/* figure out offset between 7k timestamp and Edgetech timestamp */
#ifdef MBR_RESON7KR_DEBUG2
    fprintf(stderr, "%s: 7k time offset: %f    7ktime:%f Edgetech:%f Bathymetry:%f\n", __func__,
            edgetech_time_d - bathy_time_d, s7k_time_d, edgetech_time_d, bathy_time_d);
#endif

    /* use Edgetech time */
    store->time_d = edgetech_time_d;
    mb_get_date(verbose, edgetech_time_d, store->time_i);

    /* use Edgetech time for early MBARI SBP missions with
        bad time synching, otherwise use 7K timestamp */
    /*if (header->s7kTime.Year == 2004)
        {
        /\* get the time from the original Edgetech header *\/
        time_j[0] = fsdwssheader->year;
        time_j[1] = fsdwssheader->day;
        time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
        time_j[3] = fsdwssheader->second;
        time_j[4] = 1000 * (fsdwssheader->millisecondsToday
                    - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
        }

    else
        {
        /\* get the time from the 6046 datalogger header *\/
        time_j[0] = header->s7kTime.Year;
        time_j[1] = header->s7kTime.Day;
        time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
        time_j[3] = (int) header->s7kTime.Seconds;
        time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
        }

    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));*/
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  for (int i = 0; i < fsdwsshi->number_channels; i++) {
    mb_get_date(verbose, s7k_time_d, time_i);
    fsdwchannel = &(fsdwsshi->channel[i]);
    fsdwssheader = &(fsdwsshi->ssheader[i]);
    fprintf(stderr,
            "R7KRECID_FSDWsidescanHi:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d "
            "%2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], fsdwssheader->year,
            fsdwssheader->day, fsdwssheader->hour, fsdwssheader->minute, fsdwssheader->second,
            fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday), fsdwsshi->ping_number,
            fsdwssheader->pingNum, fsdwchannel->number, fsdwssheader->channelNum, fsdwchannel->sample_interval,
            fsdwssheader->sampleInterval);
  }
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwss(verbose, fsdwsshi, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fsdwsb(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fsdwsb *fsdwsb;
  s7k_fsdwchannel *fsdwchannel;
  s7k_fsdwsegyheader *fsdwsegyheader;
  s7kr_bathymetry *bathymetry;
  unsigned int index = 0;
  int time_i[7];
  double edgetech_time_d, s7k_time_d, bathy_time_d;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  fsdwsb = &(store->fsdwsb);
  header = &(fsdwsb->header);
  bathymetry = &store->bathymetry;

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsb->msec_timestamp));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsb->ping_number));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsb->number_channels));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsb->total_bytes));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fsdwsb->data_format));
  index += 4;
  index += 12;
  fsdwchannel = &(fsdwsb->channel);
  mbr_reson7kr_rd_fsdwchannel(verbose, fsdwsb->data_format, buffer, &index, fsdwchannel, error);
  fsdwsegyheader = &(fsdwsb->segyheader);
  mbr_reson7kr_rd_fsdwsegyheader(verbose, buffer, &index, fsdwsegyheader, error);

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SUBBOTTOM_SUBBOTTOM;
    store->type = R7KRECID_FSDWsubbottom;

    /* get the time from the original Edgetech header */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = fsdwsegyheader->year;
    time_j[1] = fsdwsegyheader->day;
    time_j[2] = 60 * fsdwsegyheader->hour + fsdwsegyheader->minute;
    time_j[3] = fsdwsegyheader->second;
    time_j[4] = 1000 * (fsdwsegyheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwsegyheader->millisecondsToday)));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(edgetech_time_d));

    /* get the time from the 6046 datalogger header */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(s7k_time_d));

    /* get the time from the last bathymetry record */
    time_j[0] = bathymetry->header.s7kTime.Year;
    time_j[1] = bathymetry->header.s7kTime.Day;
    time_j[2] = 60 * bathymetry->header.s7kTime.Hours + bathymetry->header.s7kTime.Minutes;
    time_j[3] = (int)bathymetry->header.s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (bathymetry->header.s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &(bathy_time_d));

/* figure out offset between 7k timestamp and Edgetech timestamp */
#ifdef MBR_RESON7KR_DEBUG2
    fprintf(stderr, "%s: 7k time offset: %f    7ktime:%f Edgetech:%f Bathymetry:%f\n", __func__,
            edgetech_time_d - bathy_time_d, s7k_time_d, edgetech_time_d, bathy_time_d);
#endif

    /* Use the Edgetech timestamp */
    store->time_d = edgetech_time_d;
    mb_get_date(verbose, store->time_d, store->time_i);

    /* use Edgetech time */
    store->time_d = edgetech_time_d;
    mb_get_date(verbose, edgetech_time_d, store->time_i);

    /* use Edgetech time for early MBARI SBP missions with
        bad time synching, otherwise use 7K timestamp */
    /*if (header->s7kTime.Year == 2004)
        {
        /\* get the time from the original Edgetech header *\/
        time_j[0] = fsdwsegyheader->year;
        time_j[1] = fsdwsegyheader->day;
        time_j[2] = 60 * fsdwsegyheader->hour + fsdwsegyheader->minute;
        time_j[3] = fsdwsegyheader->second;
        time_j[4] = 1000 * (fsdwsegyheader->millisecondsToday
                    - 1000 * ((int)(0.001 * fsdwsegyheader->millisecondsToday)));
        }

    else
        {
        /\* get the time from the 6046 datalogger header *\/
        time_j[0] = header->s7kTime.Year;
        time_j[1] = header->s7kTime.Day;
        time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
        time_j[3] = (int) header->s7kTime.Seconds;
        time_j[4] = (int) (1000000 * (header->s7kTime.Seconds - time_j[3]));
        }

    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));*/
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  for (int i = 0; i < fsdwsb->number_channels; i++) {
    mb_get_date(verbose, s7k_time_d, time_i);
    fsdwchannel = &(fsdwsb->channel);
    fsdwsegyheader = &(fsdwsb->segyheader);
    fprintf(stderr,
            "R7KRECID_FSDWsubbottom:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d "
            "%2.2d:%2.2d:%2.2d.%3.3d) ping:%d %d chan:%d %d sampint:%d %d\n",
            time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], fsdwsegyheader->year,
            fsdwsegyheader->day, fsdwsegyheader->hour, fsdwsegyheader->minute, fsdwsegyheader->second,
            fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday), fsdwsb->ping_number,
            fsdwsegyheader->pingNum, fsdwchannel->number, fsdwsegyheader->channelNum, fsdwchannel->sample_interval,
            fsdwsegyheader->sampleInterval);
  }
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwsb(verbose, fsdwsb, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_bluefin(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_bluefin *bluefin;
  unsigned int index = 0;
  double time_d;
  bool timeproblem;
#ifdef MBR_RESON7KR_DEBUG2
  int time_i[7];
#endif
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  bluefin = &(store->bluefin);
  header = &(bluefin->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* get the first cut time from the 6046 datalogger header */
  int time_j[5] = {0, 0, 0, 0, 0};
  time_j[0] = header->s7kTime.Year;
  time_j[1] = header->s7kTime.Day;
  time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
  time_j[3] = (int)header->s7kTime.Seconds;
  time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
  mb_get_itime(verbose, time_j, store->time_i);
  mb_get_time(verbose, store->time_i, &(store->time_d));

  /* extract the start of the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(bluefin->msec_timestamp));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(bluefin->number_frames));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(bluefin->frame_size));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(bluefin->data_format));
  index += 4;
  for (int i = 0; i < 16; i++) {
    bluefin->reserved[i] = buffer[index];
    index++;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  if (bluefin->data_format == R7KRECID_BluefinNav)
    fprintf(stderr,
            "R7KRECID_BluefinNav:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
            "index:%d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], header->RecordNumber, header->Size, index);
  else
    fprintf(stderr,
            "R7KRECID_BluefinEnvironmental:     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
            "index:%d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], header->RecordNumber, header->Size, index);
#endif

  /* extract the nav or environmental data */
  if (bluefin->data_format == R7KRECID_BluefinNav) {
    for (int i = 0; i < bluefin->number_frames; i++) {
      mb_get_binary_int(true, &buffer[index], &(bluefin->nav[i].packet_size));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->nav[i].version));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->nav[i].offset));
      index += 2;
      mb_get_binary_int(true, &buffer[index], &(bluefin->nav[i].data_type));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(bluefin->nav[i].data_size));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->nav[i].s7kTime.Year));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->nav[i].s7kTime.Day));
      index += 2;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].s7kTime.Seconds));
      index += 4;
      bluefin->nav[i].s7kTime.Hours = (mb_u_char)buffer[index];
      (index)++;
      bluefin->nav[i].s7kTime.Minutes = (mb_u_char)buffer[index];
      (index)++;
      mb_get_binary_int(true, &buffer[index], &(bluefin->nav[i].checksum));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->nav[i].timedelay));
      index += 2;
      mb_get_binary_int(true, &buffer[index], &(bluefin->nav[i].quality));
      index += 4;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].latitude));
      index += 8;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].longitude));
      index += 8;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].speed));
      index += 4;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].depth));
      index += 8;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].altitude));
      index += 8;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].roll));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].pitch));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].yaw));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].northing_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].easting_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].depth_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].altitude_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].roll_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].pitch_rate));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->nav[i].yaw_rate));
      index += 4;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].position_time));
      index += 8;
      mb_get_binary_double(true, &buffer[index], &(bluefin->nav[i].depth_time));
      index += 8;
/*
fprintf(stderr,"Bluefin nav[%d].packet_size:        %d\n",i,bluefin->nav[i].packet_size);
fprintf(stderr,"Bluefin nav[%d].version:            %d\n",i,bluefin->nav[i].version);
fprintf(stderr,"Bluefin nav[%d].offset:             %d\n",i,bluefin->nav[i].offset);
fprintf(stderr,"Bluefin nav[%d].data_type:          %d\n",i,bluefin->nav[i].data_type);
fprintf(stderr,"Bluefin nav[%d].data_size:          %d\n",i,bluefin->nav[i].data_size);
fprintf(stderr,"Bluefin nav[%d].s7kTime.Year:       %d\n",i,bluefin->nav[i].s7kTime.Year);
fprintf(stderr,"Bluefin nav[%d].s7kTime.Day:        %d\n",i,bluefin->nav[i].s7kTime.Day);
fprintf(stderr,"Bluefin nav[%d].s7kTime.Seconds:    %f\n",i,bluefin->nav[i].s7kTime.Seconds);
fprintf(stderr,"Bluefin nav[%d].s7kTime.Hours:      %d\n",i,bluefin->nav[i].s7kTime.Hours);
fprintf(stderr,"Bluefin nav[%d].7kTime->Minutes:    %d\n",i,bluefin->nav[i].s7kTime.Minutes);
fprintf(stderr,"Bluefin nav[%d].checksum:           %d\n",i,bluefin->nav[i].checksum);
fprintf(stderr,"Bluefin nav[%d].timedelay:          %d\n",i,bluefin->nav[i].timedelay);
fprintf(stderr,"Bluefin nav[%d].quality:            %x\n",i,bluefin->nav[i].quality);
fprintf(stderr,"Bluefin nav[%d].latitude:           %f\n",i,bluefin->nav[i].latitude);
fprintf(stderr,"Bluefin nav[%d].longitude:          %f\n",i,bluefin->nav[i].longitude);
fprintf(stderr,"Bluefin nav[%d].speed:              %f\n",i,bluefin->nav[i].speed);
fprintf(stderr,"Bluefin nav[%d].depth:              %f\n",i,bluefin->nav[i].depth);
fprintf(stderr,"Bluefin nav[%d].altitude:           %f\n",i,bluefin->nav[i].altitude);
fprintf(stderr,"Bluefin nav[%d].roll:               %f\n",i,bluefin->nav[i].roll);
fprintf(stderr,"Bluefin nav[%d].pitch:              %f\n",i,bluefin->nav[i].pitch);
fprintf(stderr,"Bluefin nav[%d].yaw:                %f\n",i,bluefin->nav[i].yaw);
fprintf(stderr,"Bluefin nav[%d].northing_rate:      %f\n",i,bluefin->nav[i].northing_rate);
fprintf(stderr,"Bluefin nav[%d].easting_rate:       %f\n",i,bluefin->nav[i].easting_rate);
fprintf(stderr,"Bluefin nav[%d].depth_rate:         %f\n",i,bluefin->nav[i].depth_rate);
fprintf(stderr,"Bluefin nav[%d].altitude_rate:      %f\n",i,bluefin->nav[i].altitude_rate);
fprintf(stderr,"Bluefin nav[%d].roll_rate:          %f\n",i,bluefin->nav[i].roll_rate);
fprintf(stderr,"Bluefin nav[%d].pitch_rate:         %f\n",i,bluefin->nav[i].pitch_rate);
fprintf(stderr,"Bluefin nav[%d].yaw_rate:           %f\n",i,bluefin->nav[i].yaw_rate);
fprintf(stderr,"Bluefin nav[%d].position_time:      %f\n",i,bluefin->nav[i].position_time);
fprintf(stderr,"Bluefin nav[%d].depth_time:         %f\n",i,bluefin->nav[i].depth_time);
*/

/* print out the nav point time stamp */
#ifdef MBR_RESON7KR_DEBUG2
      time_j[0] = bluefin->nav[i].s7kTime.Year;
      time_j[1] = bluefin->nav[i].s7kTime.Day;
      time_j[2] = 60 * bluefin->nav[i].s7kTime.Hours + bluefin->nav[i].s7kTime.Minutes;
      time_j[3] = (int)bluefin->nav[i].s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (bluefin->nav[i].s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      fprintf(stderr,
              "                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) Pos_time:%f\n", i,
              time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6], bluefin->nav[i].position_time);
#endif
    }

    /* The Reson 6046 datalogger has been placing the same time tag in each of the frames
        - check if this is the case, if it is kluge new time tags spread over a one second interval */
    if (bluefin->number_frames > 1) {
      /* figure out if there is a time problem */
      timeproblem = false;
      for (int i = 1; i < bluefin->number_frames; i++) {
        if (bluefin->nav[i].position_time == bluefin->nav[i - 1].position_time)
          timeproblem = true;
      }
      timeproblem = false;

      /* figure out if the time changes anywhere */
      if (timeproblem) {
        /* change unix times to use 7k time */
        for (int i = 0; i < bluefin->number_frames; i++) {
/* get the time  */
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "CHANGE TIMESTAMP: %d %2.2d:%2.2d:%6.3f %12f", i, bluefin->nav[i].s7kTime.Hours,
                  bluefin->nav[i].s7kTime.Minutes, bluefin->nav[i].s7kTime.Seconds, time_d);
#endif
          time_j[0] = bluefin->nav[i].s7kTime.Year;
          time_j[1] = bluefin->nav[i].s7kTime.Day;
          time_j[2] = 60 * bluefin->nav[i].s7kTime.Hours + bluefin->nav[i].s7kTime.Minutes;
          time_j[3] = (int)bluefin->nav[i].s7kTime.Seconds;
          time_j[4] = (int)(1000000 * (bluefin->nav[i].s7kTime.Seconds - time_j[3]));
          mb_get_itime(verbose, time_j, store->time_i);
          mb_get_time(verbose, store->time_i, &time_d);
          bluefin->nav[i].position_time = time_d;
          bluefin->nav[i].depth_time = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "    %12f\n", time_d);
#endif
        }
      }
    }
  }
  else if (bluefin->data_format == R7KRECID_BluefinEnvironmental) {
    for (int i = 0; i < bluefin->number_frames; i++) {
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].packet_size));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].version));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].offset));
      index += 2;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].data_type));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].data_size));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].s7kTime.Year));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].s7kTime.Day));
      index += 2;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].s7kTime.Seconds));
      index += 4;
      bluefin->environmental[i].s7kTime.Hours = (mb_u_char)buffer[index];
      (index)++;
      bluefin->environmental[i].s7kTime.Minutes = (mb_u_char)buffer[index];
      (index)++;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].checksum));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].reserved1));
      index += 2;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].quality));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].sound_speed));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].conductivity));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].temperature));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].pressure));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].salinity));
      index += 4;
      mb_get_binary_double(true, &buffer[index], &(bluefin->environmental[i].ctd_time));
      index += 8;
      mb_get_binary_double(true, &buffer[index], &(bluefin->environmental[i].temperature_time));
      index += 8;
      mb_get_binary_double(true, &buffer[index], &(bluefin->environmental[i].surface_pressure));
      index += 8;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].temperature_counts));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].conductivity_frequency));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].pressure_counts));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bluefin->environmental[i].pressure_comp_voltage));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].sensor_time_sec));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(bluefin->environmental[i].sensor_time_nsec));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor1));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor2));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor3));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor4));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor5));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor6));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor7));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(bluefin->environmental[i].sensor8));
      index += 2;
      for (int j = 0; j < 8; j++) {
        bluefin->environmental[i].reserved2[j] = buffer[index];
        index++;
      }

/* print out the environmental point time stamp */
#ifdef MBR_RESON7KR_DEBUG2
      time_j[0] = bluefin->environmental[i].s7kTime.Year;
      time_j[1] = bluefin->environmental[i].s7kTime.Day;
      time_j[2] = 60 * bluefin->environmental[i].s7kTime.Hours + bluefin->environmental[i].s7kTime.Minutes;
      time_j[3] = (int)bluefin->environmental[i].s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (bluefin->environmental[i].s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, time_i);
      fprintf(stderr,
              "                       %2.2d          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) CTD_time:%f "
              "T_time:%f S_time:%d.%9.9d\n",
              i, time_i[0], time_i[1], time_i[2], time_i[3], time_i[4], time_i[5], time_i[6],
              bluefin->environmental[i].ctd_time, bluefin->environmental[i].temperature_time,
              bluefin->environmental[i].sensor_time_sec, bluefin->environmental[i].sensor_time_nsec);
#endif
    }

    /* The Reson 6046 datalogger has been placing the same time tag in each of the frames
        - check if this is the case, if it is kluge new time tags spread over a one second interval */
    if (bluefin->number_frames > 1) {
      /* figure out if there is a time problem */
      timeproblem = false;
      for (int i = 1; i < bluefin->number_frames; i++) {
        if (bluefin->environmental[i].ctd_time == bluefin->environmental[i - 1].ctd_time ||
            bluefin->environmental[i].ctd_time < 10000000.0)
          timeproblem = true;
      }
      timeproblem = false;

      /* figure out if the time changes anywhere */
      if (timeproblem) {
        /* change unix times to use 7k time */
        for (int i = 0; i < bluefin->number_frames; i++) {
/* get the time  */
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "CHANGE TIMESTAMP: %d %2.2d:%2.2d:%6.3f %12f", i, bluefin->environmental[i].s7kTime.Hours,
                  bluefin->environmental[i].s7kTime.Minutes, bluefin->environmental[i].s7kTime.Seconds, time_d);
#endif
          time_j[0] = bluefin->environmental[i].s7kTime.Year;
          time_j[1] = bluefin->environmental[i].s7kTime.Day;
          time_j[2] = 60 * bluefin->environmental[i].s7kTime.Hours + bluefin->environmental[i].s7kTime.Minutes;
          time_j[3] = (int)bluefin->environmental[i].s7kTime.Seconds;
          time_j[4] = (int)(1000000 * (bluefin->environmental[i].s7kTime.Seconds - time_j[3]));
          mb_get_itime(verbose, time_j, store->time_i);
          mb_get_time(verbose, store->time_i, &time_d);
          bluefin->environmental[i].ctd_time = time_d;
          bluefin->environmental[i].temperature_time = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "    %12f\n", time_d);
#endif
        }
      }
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    if (bluefin->data_format == R7KRECID_BluefinNav) {
      store->kind = MB_DATA_NAV2;
      store->type = R7KRECID_Bluefin;

      /* get the time from the 6046 datalogger header */
      time_j[0] = bluefin->nav[0].s7kTime.Year;
      time_j[1] = bluefin->nav[0].s7kTime.Day;
      time_j[2] = 60 * bluefin->nav[0].s7kTime.Hours + bluefin->nav[0].s7kTime.Minutes;
      time_j[3] = (int)bluefin->nav[0].s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (bluefin->nav[0].s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, store->time_i);
      mb_get_time(verbose, store->time_i, &(store->time_d));
    }
    else if (bluefin->data_format == R7KRECID_BluefinEnvironmental) {
      store->kind = MB_DATA_SSV;
      store->type = R7KRECID_Bluefin;

      /* get the time from the 6046 datalogger header */
      time_j[0] = bluefin->environmental[0].s7kTime.Year;
      time_j[1] = bluefin->environmental[0].s7kTime.Day;
      time_j[2] = 60 * bluefin->environmental[0].s7kTime.Hours + bluefin->environmental[0].s7kTime.Minutes;
      time_j[3] = (int)bluefin->environmental[0].s7kTime.Seconds;
      time_j[4] = (int)(1000000 * (bluefin->environmental[0].s7kTime.Seconds - time_j[3]));
      mb_get_itime(verbose, time_j, store->time_i);
      mb_get_time(verbose, store->time_i, &(store->time_d));
    }
    else {
      store->kind = MB_DATA_NONE;
      status = MB_FAILURE;
      *error = MB_ERROR_UNINTELLIGIBLE;
    }
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  if (bluefin->data_format == R7KRECID_BluefinNav)
    fprintf(stderr,
            "R7KRECID_BluefinNav:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
            "size:%d index:%d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], header->RecordNumber, header->Size, index);
  else
    fprintf(stderr,
            "R7KRECID_BluefinEnvironmental:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
            "size:%d index:%d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_bluefin(verbose, bluefin, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_processedsidescan(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_processedsidescan *processedsidescan;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  processedsidescan = &(store->processedsidescan);
  header = &(processedsidescan->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(processedsidescan->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(processedsidescan->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(processedsidescan->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(processedsidescan->recordversion));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(processedsidescan->ss_source));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(processedsidescan->number_pixels));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(processedsidescan->ss_type));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(processedsidescan->pixelwidth));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(processedsidescan->sensordepth));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(processedsidescan->altitude));
  index += 8;

  /* extract the data */
  for (unsigned int i = 0; i < processedsidescan->number_pixels; i++) {
    mb_get_binary_float(true, &buffer[index], &(processedsidescan->sidescan[i]));
    index += 4;
  }
  for (unsigned int i = 0; i < processedsidescan->number_pixels; i++) {
    mb_get_binary_float(true, &buffer[index], &(processedsidescan->alongtrack[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_ProcessedSidescan;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_ProcessedSidescan:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], processedsidescan->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_processedsidescan(verbose, processedsidescan, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_volatilesonarsettings(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_volatilesettings *volatilesettings;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  volatilesettings = &(store->volatilesettings);
  header = &(volatilesettings->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(volatilesettings->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(volatilesettings->multi_ping));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->sample_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->receiver_bandwidth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->pulse_width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->pulse_type));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->pulse_envelope));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->pulse_envelope_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->pulse_reserved));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->max_ping_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->ping_period));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->range_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->power_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->gain_selection));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->projector_magic_no));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->steering_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->steering_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->beamwidth_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->beamwidth_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->focal_point));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->projector_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->projector_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->transmit_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->hydrophone_magic_no));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->receive_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->receive_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(volatilesettings->receive_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->receive_width));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->range_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->range_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->depth_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->depth_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->absorption));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(volatilesettings->spreading));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(volatilesettings->reserved));
  index += 2;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kVolatileSonarSettings;

    /* get the time */
    int time_j[5] = {0, 0, 0, 0, 0};
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kVolatileSonarSettings:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], volatilesettings->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_volatilesettings(verbose, volatilesettings, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_configuration(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_configuration *configuration;
  s7k_device *device;
  unsigned int data_size;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  configuration = &(store->configuration);
  header = &(configuration->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(configuration->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(configuration->number_devices));
  index += 4;

  /* extract the data for each device */
  for (unsigned int i = 0; i < configuration->number_devices; i++) {
    device = &(configuration->device[i]);
    mb_get_binary_int(true, &buffer[index], &(device->magic_number));
    index += 4;
    for (int j = 0; j < 64; j++) {
      device->description[j] = buffer[index];
      index++;
    }
    mb_get_binary_long(true, &buffer[index], &(device->serial_number));
    index += 8;
    mb_get_binary_int(true, &buffer[index], &(device->info_length));
    index += 4;

    /* make sure enough memory is allocated for info data */
    if (device->info_alloc < device->info_length) {
      data_size = device->info_length + 1;
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(device->info), error);
      if (status == MB_SUCCESS) {
        device->info_alloc = device->info_length;
      }
      else {
        device->info_alloc = 0;
        device->info_length = 0;
      }
    }

    for (unsigned int j = 0; j < device->info_length; j++) {
      device->info[j] = buffer[index];
      index++;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kConfiguration;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kConfiguration:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_configuration(verbose, configuration, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_matchfilter(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_matchfilter *matchfilter;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  matchfilter = &(store->matchfilter);
  header = &(matchfilter->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(matchfilter->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(matchfilter->ping_number));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(matchfilter->operation));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(matchfilter->start_frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(matchfilter->end_frequency));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kMatchFilter;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kMatchFilter:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], matchfilter->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_matchfilter(verbose, matchfilter, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2firmwarehardwareconfiguration(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration;
  unsigned int index = 0;
  unsigned int data_size;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2firmwarehardwareconfiguration = &(store->v2firmwarehardwareconfiguration);
  header = &(v2firmwarehardwareconfiguration->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(v2firmwarehardwareconfiguration->device_count));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(v2firmwarehardwareconfiguration->info_length));
  index += 4;

  /* make sure enough memory is allocated for info data */
  if (v2firmwarehardwareconfiguration->info_alloc < v2firmwarehardwareconfiguration->info_length) {
    data_size = v2firmwarehardwareconfiguration->info_length + 1;
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(v2firmwarehardwareconfiguration->info), error);
    if (status == MB_SUCCESS) {
      v2firmwarehardwareconfiguration->info_alloc = v2firmwarehardwareconfiguration->info_length;
    }
    else {
      v2firmwarehardwareconfiguration->info_alloc = 0;
      v2firmwarehardwareconfiguration->info_length = 0;
    }
  }

  for (unsigned int j = 0; j < v2firmwarehardwareconfiguration->info_length; j++) {
    v2firmwarehardwareconfiguration->info[j] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kV2FirmwareHardwareConfiguration;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2FirmwareHardwareConfiguration: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
          "size:%d index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2firmwarehardwareconfiguration(verbose, v2firmwarehardwareconfiguration, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_beamgeometry(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_beamgeometry *beamgeometry;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  beamgeometry = &(store->beamgeometry);
  header = &(beamgeometry->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(beamgeometry->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(beamgeometry->number_beams));
  index += 4;

  /* extract the data */
  for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(beamgeometry->angle_alongtrack[i]));
    index += 4;
  }
  for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(beamgeometry->angle_acrosstrack[i]));
    index += 4;
  }
  for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(beamgeometry->beamwidth_alongtrack[i]));
    index += 4;
  }
  for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(beamgeometry->beamwidth_acrosstrack[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kBeamGeometry;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kBeamGeometry:                     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
          "size:%d index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_beamgeometry(verbose, beamgeometry, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_calibration(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_calibration *calibration;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  calibration = &(store->calibration);
  header = &(calibration->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(calibration->serial_number));
  index += 8;
  mb_get_binary_short(true, &buffer[index], &(calibration->number_channels));
  index += 2;

  /* extract the data */
  for (int i = 0; i < calibration->number_channels; i++) {
    mb_get_binary_float(true, &buffer[index], &(calibration->gain[i]));
    index += 4;
  }
  for (int i = 0; i < calibration->number_channels; i++) {
    mb_get_binary_float(true, &buffer[index], &(calibration->phase[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kCalibrationData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kCalibrationData:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_calibration(verbose, calibration, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_bathymetry(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_bathymetry *bathymetry;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};
  double acrosstrackmax, alongtrackmax;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  bathymetry = &(store->bathymetry);
  header = &(bathymetry->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(bathymetry->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(bathymetry->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(bathymetry->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(bathymetry->number_beams));
  index += 4;

  /* deal with version 5 records */
  if (header->Version >= 5) {
    bathymetry->layer_comp_flag = buffer[index];
    index++;
    bathymetry->sound_vel_flag = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->sound_velocity));
    index += 4;
  }
  else {
    bathymetry->layer_comp_flag = 0;
    bathymetry->sound_vel_flag = 0;
    bathymetry->sound_velocity = 0.0;
  }

  /* extract the data */
  for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(bathymetry->range[i]));
    index += 4;
  }
  for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
    bathymetry->quality[i] = buffer[index];
    index++;
  }
  for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(bathymetry->intensity[i]));
    index += 4;
  }
  if ((header->OffsetToOptionalData == 0 && header->Size >= 92 + 17 * bathymetry->number_beams) ||
      (header->OffsetToOptionalData > 0 && header->Size >= 137 + 37 * bathymetry->number_beams)) {
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(bathymetry->min_depth_gate[i]));
      index += 4;
    }
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(bathymetry->max_depth_gate[i]));
      index += 4;
    }
  }

  /* extract the optional data */
  if (header->OffsetToOptionalData > 0) {
    index = header->OffsetToOptionalData;
    bathymetry->optionaldata = true;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(bathymetry->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(bathymetry->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->heading));
    index += 4;
    bathymetry->height_source = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->tide));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->roll));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->pitch));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->heave));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(bathymetry->vehicle_height));
    index += 4;
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(bathymetry->depth[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathymetry->alongtrack[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathymetry->acrosstrack[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathymetry->pointing_angle[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathymetry->azimuth_angle[i]));
      index += 4;
    }

    /* now check to see if these data were written incorrectly with acrosstrack before alongtrack
     * (ashamedly, this was true for MB-System through 4.3.2000)
     * - if so, switch the arrays */
    if (bathymetry->acrossalongerror == MB_MAYBE) {
      if (header->s7kTime.Year > 2012) {
        bathymetry->acrossalongerror = MB_NO;
      }
      else {
        acrosstrackmax = 0.0;
        alongtrackmax = 0.0;
        for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
          acrosstrackmax = MAX(acrosstrackmax, fabs(bathymetry->acrosstrack[i]));
          alongtrackmax = MAX(alongtrackmax, fabs(bathymetry->alongtrack[i]));
        }
        if (alongtrackmax > acrosstrackmax) {
          bathymetry->nacrossalongerroryes++;
        }
        else {
          bathymetry->nacrossalongerrorno++;
        }
        if (bathymetry->nacrossalongerroryes > 10) {
          bathymetry->acrossalongerror = MB_YES;
        }
        else if (bathymetry->nacrossalongerrorno > 10) {
          bathymetry->acrossalongerror = MB_NO;
        }
      }
    }
    if (bathymetry->acrossalongerror == MB_YES ||
        (bathymetry->acrossalongerror == MB_MAYBE && alongtrackmax > acrosstrackmax)) {
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        acrosstrackmax = bathymetry->acrosstrack[i];
        bathymetry->acrosstrack[i] = bathymetry->alongtrack[i];
        bathymetry->alongtrack[i] = acrosstrackmax;
      }
    }
  }
  else {
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
      bathymetry->acrosstrack[i] = 0.0;
      bathymetry->alongtrack[i] = 0.0;
      bathymetry->pointing_angle[i] = 0.0;
      bathymetry->azimuth_angle[i] = 0.0;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kBathymetricData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kBathymetricData:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], bathymetry->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_bathymetry(verbose, bathymetry, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_backscatter(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header = NULL;
  s7kr_backscatter *backscatter = NULL;
  unsigned int data_size = 0;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};
  short *short_ptr = NULL;
  int *int_ptr = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  backscatter = &(store->backscatter);
  header = &(backscatter->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(backscatter->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(backscatter->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(backscatter->multi_ping));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(backscatter->beam_position));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(backscatter->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(backscatter->number_samples));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->port_beamwidth_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->port_beamwidth_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->stbd_beamwidth_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->stbd_beamwidth_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->port_steering_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->port_steering_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->stbd_steering_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(backscatter->stbd_steering_y));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(backscatter->number_beams));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(backscatter->current_beam));
  index += 2;
  backscatter->sample_size = buffer[index];
  index++;
  backscatter->data_type = buffer[index];
  index++;

  /* allocate memory if required */
  data_size = backscatter->number_samples * backscatter->sample_size;
  if (backscatter->nalloc < data_size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(backscatter->port_data), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(backscatter->stbd_data), error);
    if (status == MB_SUCCESS) {
      backscatter->nalloc = data_size;
    }
    else {
      backscatter->nalloc = 0;
      backscatter->number_samples = 0;
    }
  }

  /* extract backscatter data */
  if (backscatter->sample_size == 1) {
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      backscatter->port_data[i] = buffer[index];
      index++;
    }
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      backscatter->stbd_data[i] = buffer[index];
      index++;
    }
  }
  else if (backscatter->sample_size == 2) {
    short_ptr = (short *)backscatter->port_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      mb_get_binary_short(true, &buffer[index], &(short_ptr[i]));
      index += 2;
    }
    short_ptr = (short *)backscatter->stbd_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      mb_get_binary_short(true, &buffer[index], &(short_ptr[i]));
      index += 2;
    }
  }
  else if (backscatter->sample_size == 4) {
    int_ptr = (int *)backscatter->port_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      mb_get_binary_int(true, &buffer[index], &(int_ptr[i]));
      index += 4;
    }
    int_ptr = (int *)backscatter->stbd_data;
    for (unsigned int i = 0; i < backscatter->number_samples; i++) {
      mb_get_binary_int(true, &buffer[index], &(int_ptr[i]));
      index += 4;
    }
  }

  /* extract the optional data */
  if (header->OffsetToOptionalData > 0) {
    index = header->OffsetToOptionalData;
    backscatter->optionaldata = true;
    mb_get_binary_float(true, &buffer[index], &(backscatter->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(backscatter->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(backscatter->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(backscatter->heading));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(backscatter->altitude));
    index += 4;
  }
  else {
    backscatter->optionaldata = false;
    backscatter->frequency = 0.0;
    backscatter->latitude = 0.0;
    backscatter->longitude = 0.0;
    backscatter->heading = 0.0;
    backscatter->altitude = 0.0;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kBackscatterImageData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kBackscatterImageData:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], backscatter->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_backscatter(verbose, backscatter, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_beam(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header = NULL;
  s7kr_beam *beam = NULL;
  s7kr_snippet *snippet = 0;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};
  unsigned int nalloc_amp = 0;
  unsigned int nalloc_phase = 0;
  unsigned int nsamples = 0;
  int sample_type_amp = 0;
  int sample_type_phase = 0;
  int sample_type_iandq = 0;
  char *charptr = NULL;
  unsigned short *ushortptr = NULL;
  unsigned int *uintptr = NULL;
  short *shortptramp = NULL;
  short *shortptrphase = NULL;
  int *intptramp = NULL;
  int *intptrphase = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  beam = &(store->beam);
  header = &(beam->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(beam->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(beam->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(beam->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(beam->number_beams));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(beam->reserved));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(beam->number_samples));
  index += 4;
  beam->record_subset_flag = buffer[index];
  index++;
  beam->row_column_flag = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(beam->sample_header_id));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(beam->sample_type));
  index += 4;
  sample_type_amp = beam->sample_type & 15;
  sample_type_phase = (beam->sample_type >> 4) & 15;
  sample_type_iandq = (beam->sample_type >> 8) & 15;
  for (int i = 0; i < beam->number_beams; i++) {
    snippet = &beam->snippets[i];
    mb_get_binary_short(true, &buffer[index], &(snippet->beam_number));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(snippet->begin_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippet->end_sample));
    index += 4;
  }

  for (int i = 0; i < beam->number_beams; i++) {
    /* allocate memory for snippet if needed */
    snippet = &beam->snippets[i];
    nalloc_amp = 0;
    nalloc_phase = 0;
    if (sample_type_amp == 1)
      nalloc_amp += 1;
    else if (sample_type_amp == 2)
      nalloc_amp += 2;
    else if (sample_type_amp == 3)
      nalloc_amp += 4;
    if (sample_type_phase == 1)
      nalloc_phase += 1;
    else if (sample_type_phase == 2)
      nalloc_phase += 2;
    else if (sample_type_phase == 3)
      nalloc_phase += 4;
    if (sample_type_iandq == 1) {
      nalloc_amp += 2;
      nalloc_phase += 2;
    }
    else if (sample_type_iandq == 2) {
      nalloc_amp += 4;
      nalloc_phase += 4;
    }
    nalloc_amp *= (snippet->end_sample - snippet->begin_sample + 1);
    nalloc_phase *= (snippet->end_sample - snippet->begin_sample + 1);
    if (status == MB_SUCCESS && (snippet->nalloc_amp < nalloc_amp || snippet->nalloc_phase < nalloc_phase)) {
      snippet->nalloc_amp = nalloc_amp;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, snippet->nalloc_amp, (void **)&(snippet->amplitude), error);
      snippet->nalloc_phase = nalloc_phase;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, snippet->nalloc_phase, (void **)&(snippet->phase), error);
      if (status != MB_SUCCESS) {
        snippet->nalloc_amp = 0;
        snippet->nalloc_phase = 0;
      }
    }

    /* extract snippet or beam data */
    if (status == MB_SUCCESS) {
      nsamples = snippet->end_sample - snippet->begin_sample + 1;
      for (unsigned int j = 0; j < nsamples; j++) {
        if (sample_type_amp == 1) {
          charptr = (char *)snippet->amplitude;
          charptr[j] = buffer[index];
          index++;
        }
        else if (sample_type_amp == 2) {
          ushortptr = (unsigned short *)snippet->amplitude;
          mb_get_binary_short(true, &buffer[index], &(ushortptr[j]));
          index += 2;
        }
        else if (sample_type_amp == 3) {
          uintptr = (unsigned int *)snippet->amplitude;
          mb_get_binary_int(true, &buffer[index], &(uintptr[j]));
          index += 4;
        }
        if (sample_type_phase == 1) {
          charptr = (char *)snippet->phase;
          charptr[j] = buffer[index];
          index++;
        }
        else if (sample_type_phase == 2) {
          ushortptr = (unsigned short *)snippet->phase;
          mb_get_binary_short(true, &buffer[index], &(ushortptr[j]));
          index += 2;
        }
        else if (sample_type_phase == 3) {
          uintptr = (unsigned int *)snippet->phase;
          mb_get_binary_int(true, &buffer[index], &(uintptr[j]));
          index += 4;
        }
        if (sample_type_iandq == 1) {
          shortptramp = (short *)snippet->amplitude;
          shortptrphase = (short *)snippet->phase;
          mb_get_binary_short(true, &buffer[index], &(shortptramp[j]));
          index += 2;
          mb_get_binary_short(true, &buffer[index], &(shortptrphase[j]));
          index += 2;
        }
        else if (sample_type_iandq == 2) {
          intptramp = (int *)snippet->amplitude;
          intptrphase = (int *)snippet->phase;
          mb_get_binary_int(true, &buffer[index], &(intptramp[j]));
          index += 4;
          mb_get_binary_int(true, &buffer[index], &(intptrphase[j]));
          index += 4;
        }
      }
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kBeamData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KHDRSIZE_7kBeamData:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], beam->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_beam(verbose, beam, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_verticaldepth(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_verticaldepth *verticaldepth;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  verticaldepth = &(store->verticaldepth);
  header = &(verticaldepth->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(verticaldepth->frequency));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(verticaldepth->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(verticaldepth->multi_ping));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(verticaldepth->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(verticaldepth->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(verticaldepth->heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(verticaldepth->alongtrack));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(verticaldepth->acrosstrack));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(verticaldepth->vertical_depth));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kVerticalDepth;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kVerticalDepth:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], verticaldepth->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_verticaldepth(verbose, verticaldepth, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_tvg(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_tvg *tvg;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};
  unsigned int nalloc = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  tvg = &(store->tvg);
  header = &(tvg->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(tvg->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(tvg->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(tvg->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(tvg->n));
  index += 4;
  for (int i = 0; i < 8; i++) {
    mb_get_binary_int(true, &buffer[index], &(tvg->reserved[i]));
    index += 4;
  }

  /* allocate memory for tvg if needed */
  nalloc = tvg->n * sizeof(float);
  if (status == MB_SUCCESS && tvg->nalloc < nalloc) {
    tvg->nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, tvg->nalloc, (void **)&(tvg->tvg), error);
    if (status != MB_SUCCESS) {
      tvg->nalloc = 0;
    }
  }

  /* extract tvg data */
  memcpy((void *)tvg->tvg, (const void *)&buffer[index], (size_t)(tvg->n * sizeof(float)));

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kTVGData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kTVGData:                          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], tvg->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_tvg(verbose, tvg, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_image(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_image *image;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};
  unsigned int nalloc = 0;
  char *charptr;
  unsigned short *ushortptr;
  unsigned int *uintptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  image = &(store->image);
  header = &(image->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(image->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(image->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(image->width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(image->height));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(image->color_depth));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(image->width_height_flag));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(image->compression));
  index += 2;

  /* allocate memory for image if needed */
  nalloc = image->width * image->height * image->color_depth;
  if (status == MB_SUCCESS && image->nalloc < nalloc) {
    image->nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, image->nalloc, (void **)&(image->image), error);
    if (status != MB_SUCCESS) {
      image->nalloc = 0;
      image->width = 0;
      image->height = 0;
    }
  }

  /* extract image data */
  if (image->color_depth == 1) {
    charptr = (char *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++) {
      charptr[i] = buffer[index];
      index++;
    }
  }
  else if (image->color_depth == 2) {
    ushortptr = (unsigned short *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++) {
      mb_get_binary_short(true, &buffer[index], &(ushortptr[i]));
      index += 2;
    }
  }
  else if (image->color_depth == 4) {
    uintptr = (unsigned int *)image->image;
    for (unsigned int i = 0; i < image->width * image->height; i++) {
      mb_get_binary_int(true, &buffer[index], &(uintptr[i]));
      index += 4;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kImageData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kImageData:                        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], image->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_image(verbose, image, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2pingmotion(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2pingmotion *v2pingmotion;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2pingmotion = &(store->v2pingmotion);
  header = &(v2pingmotion->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2pingmotion->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2pingmotion->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2pingmotion->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2pingmotion->n));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2pingmotion->flags));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2pingmotion->error_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2pingmotion->frequency));
  index += 4;
  if (v2pingmotion->flags & 1) {
    mb_get_binary_float(true, &buffer[index], &(v2pingmotion->pitch));
    index += 4;
  }

  /* allocate memory for v2pingmotion if needed */
  if (status == MB_SUCCESS && v2pingmotion->nalloc < v2pingmotion->n) {
    size_t size = sizeof(float) * v2pingmotion->n;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&(v2pingmotion->roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&(v2pingmotion->heading), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, size, (void **)&(v2pingmotion->heave), error);
    if (status == MB_SUCCESS) {
      v2pingmotion->nalloc = v2pingmotion->n;

      /* extract v2pingmotion data */
      if (v2pingmotion->flags & 2) {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          mb_get_binary_float(true, &buffer[index], &(v2pingmotion->roll[i]));
          index += 4;
        }
      }
      else {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          v2pingmotion->roll[i] = 0.0;
        }
      }
      if (v2pingmotion->flags & 4) {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          mb_get_binary_float(true, &buffer[index], &(v2pingmotion->heading[i]));
          index += 4;
        }
      }
      else {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          v2pingmotion->heading[i] = 0.0;
        }
      }
      if (v2pingmotion->flags & 8) {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          mb_get_binary_float(true, &buffer[index], &(v2pingmotion->heave[i]));
          index += 4;
        }
      }
      else {
        for (unsigned int i = 0; i < v2pingmotion->n; i++) {
          v2pingmotion->heave[i] = 0.0;
        }
      }

    } else {
      v2pingmotion->nalloc = 0;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2PingMotion;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2PingMotion:                     7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2pingmotion->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2pingmotion(verbose, v2pingmotion, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2detectionsetup(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2detectionsetup *v2detectionsetup;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2detectionsetup = &(store->v2detectionsetup);
  header = &(v2detectionsetup->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2detectionsetup->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2detectionsetup->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->number_beams));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->data_field_size));
  index += 4;
  v2detectionsetup->detection_algorithm = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->detection_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->minimum_depth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->maximum_depth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->minimum_range));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->maximum_range));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->minimum_nadir_search));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->maximum_nadir_search));
  index += 4;
  v2detectionsetup->automatic_filter_window = buffer[index];
  index++;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->applied_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->depth_gate_tilt));
  index += 4;
  for (int i = 0; i < 14; i++) {
    mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->reserved[i]));
    index += 4;
  }

  /* extract v2detectionsetup data */
  for (unsigned int i = 0; i < v2detectionsetup->number_beams; i++) {
    mb_get_binary_short(true, &buffer[index], &(v2detectionsetup->beam_descriptor[i]));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->detection_point[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->flags[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->auto_limits_min_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->auto_limits_max_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->user_limits_min_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->user_limits_max_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2detectionsetup->quality[i]));
    index += 4;
    if (v2detectionsetup->data_field_size >= R7KRDTSIZE_7kV2DetectionSetup + 4) {
      mb_get_binary_float(true, &buffer[index], &(v2detectionsetup->uncertainty[i]));
      index += 4;
    }
    else {
      v2detectionsetup->uncertainty[i] = 0.0;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2DetectionSetup;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2DetectionSetup:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2detectionsetup->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2detectionsetup(verbose, v2detectionsetup, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2beamformed(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2beamformed *v2beamformed;
  s7kr_v2amplitudephase *v2amplitudephase;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2beamformed = &(store->v2beamformed);
  header = &(v2beamformed->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2beamformed->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2beamformed->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2beamformed->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(v2beamformed->number_beams));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2beamformed->number_samples));
  index += 4;
  for (int i = 0; i < 32; i++) {
    v2beamformed->reserved[i] = buffer[index];
    index++;
  }

  /* loop over all beams */
  for (int i = 0; i < v2beamformed->number_beams; i++) {
    v2amplitudephase = &(v2beamformed->amplitudephase[i]);

    /* allocate memory for v2beamformed if needed */
    if (status == MB_SUCCESS && v2amplitudephase->nalloc < sizeof(short) * v2beamformed->number_samples) {
      v2amplitudephase->nalloc = sizeof(short) * v2beamformed->number_samples;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, v2amplitudephase->nalloc,
                             (void **)&(v2amplitudephase->amplitude), error);
      if (status != MB_SUCCESS) {
        v2amplitudephase->nalloc = 0;
      }
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, v2amplitudephase->nalloc, (void **)&(v2amplitudephase->phase),
                             error);
      if (status != MB_SUCCESS) {
        v2amplitudephase->nalloc = 0;
      }
    }

    /* extract v2beamformed data */
    for (unsigned int j = 0; j < v2beamformed->number_samples; j++) {
      mb_get_binary_short(true, &buffer[index], &(v2amplitudephase->amplitude[j]));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(v2amplitudephase->phase[j]));
      index += 2;
    }
    v2amplitudephase->beam_number = i;
    v2amplitudephase->number_samples = v2beamformed->number_samples;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2BeamformedData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2BeamformedData:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2beamformed->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2beamformed(verbose, v2beamformed, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2bite(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2bite *v2bite;
  s7kr_v2bitereport *report;
  s7k_time *s7ktime;
  s7kr_v2bitefield *bitefield;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2bite = &(store->v2bite);
  header = &(v2bite->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_short(true, &buffer[index], &(v2bite->number_reports));
  index += 2;

  /* allocate memory for v2bite->reports if needed */
  if (status == MB_SUCCESS && v2bite->nalloc < v2bite->number_reports * sizeof(s7kr_v2bitereport)) {
    v2bite->nalloc = v2bite->number_reports * sizeof(s7kr_v2bitereport);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, v2bite->nalloc, (void **)&(v2bite->reports), error);
    if (status != MB_SUCCESS) {
      v2bite->nalloc = 0;
    }
  }

  /* loop over all bite reports */
  for (int i = 0; i < v2bite->number_reports; i++) {
    report = &(v2bite->reports[i]);

    for (int j = 0; j < 64; j++) {
      report->source_name[j] = buffer[index];
      index++;
    }
    report->source_address = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(report->frequency));
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(report->enumerator));
    index += 2;

    s7ktime = &(report->downlink_time);
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7ktime->Seconds));
    index += 4;
    s7ktime->Hours = (mb_u_char)buffer[index];
    index++;
    s7ktime->Minutes = (mb_u_char)buffer[index];
    index++;

    s7ktime = &(report->uplink_time);
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7ktime->Seconds));
    index += 4;
    s7ktime->Hours = (mb_u_char)buffer[index];
    index++;
    s7ktime->Minutes = (mb_u_char)buffer[index];
    index++;

    s7ktime = &(report->bite_time);
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7ktime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7ktime->Seconds));
    index += 4;
    s7ktime->Hours = (mb_u_char)buffer[index];
    index++;
    s7ktime->Minutes = (mb_u_char)buffer[index];
    index++;

    report->status = buffer[index];
    index++;
    mb_get_binary_short(true, &buffer[index], &(report->number_bite));
    index += 2;
    for (int j = 0; j < 32; j++) {
      report->bite_status[j] = buffer[index];
      index++;
    }

    /* loop over all bite fields */
    for (int j = 0; j < report->number_bite; j++) {
      bitefield = &(report->bitefield[j]);

      mb_get_binary_short(true, &buffer[index], &(bitefield->reserved));
      index += 2;
      for (int k = 0; k < 64; k++) {
        bitefield->name[k] = buffer[index];
        index++;
      }
      bitefield->device_type = buffer[index];
      index++;
      mb_get_binary_float(true, &buffer[index], &(bitefield->minimum));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bitefield->maximum));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bitefield->value));
      index += 4;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kV2BITEData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2BITEData:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2bite(verbose, v2bite, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v27kcenterversion(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v27kcenterversion *v27kcenterversion;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v27kcenterversion = &(store->v27kcenterversion);
  header = &(v27kcenterversion->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    v27kcenterversion->version[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kV27kCenterVersion;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV27kCenterVersion:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v27kcenterversion(verbose, v27kcenterversion, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v28kwetendversion(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v28kwetendversion *v28kwetendversion;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v28kwetendversion = &(store->v28kwetendversion);
  header = &(v28kwetendversion->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    v28kwetendversion->version[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_7kV28kWetEndVersion;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV28kWetEndVersion:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v28kwetendversion(verbose, v28kwetendversion, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2detection(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2detection *v2detection;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2detection = &(store->v2detection);
  header = &(v2detection->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2detection->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2detection->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2detection->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2detection->number_beams));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(v2detection->data_field_size));
  index += 4;
  mb_get_binary_long(true, &buffer[index], &(v2detection->corrections));
  index += 8;
  v2detection->detection_algorithm = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(v2detection->flags));
  index += 4;
  for (int i = 0; i < 64; i++) {
    v2detection->reserved[i] = buffer[index];
    index++;
  }

  /* extract the data */
  for (unsigned int i = 0; i < v2detection->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(v2detection->range[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2detection->angle_x[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2detection->angle_y[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2detection->range_error[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2detection->angle_x_error[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2detection->angle_y_error[i]));
    index += 4;

    /* skip extra data if it exists */
    if (v2detection->data_field_size > 24)
      index += v2detection->data_field_size - 24;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2Detection;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2Detection:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2detection->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2detection(verbose, v2detection, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2rawdetection(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2rawdetection *v2rawdetection;
  s7kr_bathymetry *bathymetry;
    s7kr_beamgeometry *beamgeometry;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2rawdetection = &(store->v2rawdetection);
  header = &(v2rawdetection->header);
  bathymetry = &(store->bathymetry);
    beamgeometry = &(store->beamgeometry);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2rawdetection->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2rawdetection->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2rawdetection->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(v2rawdetection->number_beams));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(v2rawdetection->data_field_size));
  index += 4;
  v2rawdetection->detection_algorithm = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(v2rawdetection->detection_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2rawdetection->sampling_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(v2rawdetection->tx_angle));
  index += 4;
  for (int i = 0; i < 64; i++) {
    v2rawdetection->reserved[i] = buffer[index];
    index++;
  }

  /* extract the data */
  for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
    mb_get_binary_short(true, &buffer[index], &(v2rawdetection->beam_descriptor[i]));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(v2rawdetection->detection_point[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2rawdetection->rx_angle[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2rawdetection->flags[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(v2rawdetection->quality[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(v2rawdetection->uncertainty[i]));
    index += 4;

    /* skip extra data if it exists */
    if (v2rawdetection->data_field_size > 22)
      index += v2rawdetection->data_field_size - 22;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2RawDetection;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  /* check for broken record */
  for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
    if ((v2rawdetection->beam_descriptor[i] > MBSYS_RESON7K_MAX_BEAMS) ||
        (store->read_bathymetry && v2rawdetection->beam_descriptor[i] > bathymetry->number_beams) ||
        (store->read_beamgeometry && v2rawdetection->beam_descriptor[i] > beamgeometry->number_beams)) {
      status = MB_FAILURE;
      *error = MB_ERROR_UNINTELLIGIBLE;
    }
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2RawDetection:                   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2rawdetection->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2rawdetection(verbose, v2rawdetection, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_v2snippet(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2snippet *v2snippet;
  s7kr_v2snippettimeseries *snippettimeseries;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2snippet = &(store->v2snippet);
  header = &(v2snippet->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(v2snippet->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(v2snippet->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(v2snippet->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(v2snippet->number_beams));
  index += 2;
  v2snippet->error_flag = buffer[index];
  index++;
  v2snippet->control_flags = buffer[index];
  index++;
  for (int i = 0; i < 28; i++) {
    v2snippet->reserved[i] = buffer[index];
    index++;
  }

  /* loop over all beams to get snippet parameters */
  for (int i = 0; i < v2snippet->number_beams; i++) {
    snippettimeseries = &(v2snippet->snippettimeseries[i]);

    /* extract snippettimeseries data */
    mb_get_binary_short(true, &buffer[index], &(snippettimeseries->beam_number));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->begin_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->detect_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->end_sample));
    index += 4;

    /* allocate memory for snippettimeseries if needed */
    if (status == MB_SUCCESS &&
        snippettimeseries->nalloc < 2 * (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1)) {
      snippettimeseries->nalloc = 2 * (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1);
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, snippettimeseries->nalloc,
                             (void **)&(snippettimeseries->amplitude), error);
      if (status != MB_SUCCESS) {
        snippettimeseries->nalloc = 0;
      }
    }
  }

  /* loop over all beams to get snippet data */
  if (status == MB_SUCCESS)
    for (int i = 0; i < v2snippet->number_beams; i++) {
      snippettimeseries = &(v2snippet->snippettimeseries[i]);
      for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
        mb_get_binary_short(true, &buffer[index], &(snippettimeseries->amplitude[j]));
        index += 2;
      }
    }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kV2SnippetData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kV2SnippetData:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], v2snippet->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2snippet(verbose, v2snippet, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_calibratedsnippet(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_calibratedsnippet *calibratedsnippet;
  s7kr_calibratedsnippettimeseries *snippettimeseries;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  calibratedsnippet = &(store->calibratedsnippet);
  header = &(calibratedsnippet->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(calibratedsnippet->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(calibratedsnippet->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(calibratedsnippet->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(calibratedsnippet->number_beams));
  index += 2;
  calibratedsnippet->error_flag = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(calibratedsnippet->control_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(calibratedsnippet->absorption));
  index += 4;
  for (int i = 0; i < 6; i++) {
    mb_get_binary_int(true, &buffer[index], &(calibratedsnippet->reserved[i]));
    index += 4;
  }

  /* loop over all beams to get snippet parameters */
  for (int i = 0; i < calibratedsnippet->number_beams; i++) {
    snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);

    /* extract snippettimeseries data */
    mb_get_binary_short(true, &buffer[index], &(snippettimeseries->beam_number));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->begin_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->detect_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippettimeseries->end_sample));
    index += 4;

    /* allocate memory for snippettimeseries if needed */
    unsigned int nalloc = sizeof(float)
                  * (snippettimeseries->end_sample
                    - snippettimeseries->begin_sample + 1);
    if (status == MB_SUCCESS && snippettimeseries->nalloc < nalloc) {
      snippettimeseries->nalloc = nalloc;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, snippettimeseries->nalloc,
                             (void **)&(snippettimeseries->amplitude), error);
      if (status == MB_SUCCESS && calibratedsnippet->control_flags & 0x40)
        status = mb_reallocd(verbose, __FILE__, __LINE__, snippettimeseries->nalloc,
                             (void **)&(snippettimeseries->footprints), error);
      if (status != MB_SUCCESS) {
        snippettimeseries->nalloc = 0;
      }
    }
  }

  /* loop over all beams to get snippet amplitude data */
  if (status == MB_SUCCESS) {
    for (int i = 0; i < calibratedsnippet->number_beams; i++) {
      snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);
      for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
        mb_get_binary_float(true, &buffer[index], &(snippettimeseries->amplitude[j]));
        index += 4;
      }
    }

    /* loop over all beams to get snippet footprint data */
    if (calibratedsnippet->control_flags & 0x40) {
      for (int i = 0; i < calibratedsnippet->number_beams; i++) {
        snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);
        for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
          mb_get_binary_float(true, &buffer[index], &(snippettimeseries->footprints[j]));
          index += 4;
        }
      }
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kCalibratedSnippetData;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kCalibratedSnippetData:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], calibratedsnippet->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_calibratedsnippet(verbose, calibratedsnippet, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_installation(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_installation *installation;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  installation = &(store->installation);
  header = &(installation->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(installation->frequency));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(installation->firmware_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    installation->firmware_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(installation->software_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    installation->software_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(installation->s7k_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    installation->s7k_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(installation->protocal_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    installation->protocal_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->transmit_heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->receive_heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->motion_heading));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(installation->motion_time_delay));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(installation->position_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->position_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(installation->position_z));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(installation->position_time_delay));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(installation->waterline_z));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_INSTALLATION;
    store->type = R7KRECID_7kInstallationParameters;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kInstallationParameters:      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_installation(verbose, installation, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_fileheader(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fileheader *fileheader;
  s7kr_subsystem *subsystem;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  fileheader = &(store->fileheader);
  header = &(fileheader->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 16; i++) {
    fileheader->file_identifier[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(fileheader->version));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(fileheader->reserved));
  index += 2;
  for (int i = 0; i < 16; i++) {
    fileheader->session_identifier[i] = buffer[index];
    index++;
  }
  mb_get_binary_int(true, &buffer[index], &(fileheader->record_data_size));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(fileheader->number_subsystems));
  index += 4;
  for (int i = 0; i < 64; i++) {
    fileheader->recording_name[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 16; i++) {
    fileheader->recording_version[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 64; i++) {
    fileheader->user_defined_name[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 128; i++) {
    fileheader->notes[i] = buffer[index];
    index++;
  }
  for (unsigned int i = 0; i < fileheader->number_subsystems; i++) {
    subsystem = &(fileheader->subsystem[i]);
    mb_get_binary_int(true, &buffer[index], &(subsystem->device_identifier));
    index += 4;
    if (header->Version == 2)
      mb_get_binary_short(true, &buffer[index], &(subsystem->system_enumerator));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(subsystem->system_enumerator));
    index += 2;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADER;
    store->type = R7KRECID_7kFileHeader;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kFileHeader:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fileheader(verbose, fileheader, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_systemeventmessage(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_systemeventmessage *systemeventmessage;
  unsigned int data_size;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  systemeventmessage = &(store->systemeventmessage);
  header = &(systemeventmessage->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(systemeventmessage->serial_number));
  index += 8;
  mb_get_binary_short(true, &buffer[index], &(systemeventmessage->event_id));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(systemeventmessage->message_length));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(systemeventmessage->event_identifier));
  index += 2;

  /* make sure enough memory is allocated for channel data */
  if (systemeventmessage->message_alloc < systemeventmessage->message_length) {
    data_size = systemeventmessage->message_length + 1;
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(systemeventmessage->message), error);
    if (status == MB_SUCCESS) {
      systemeventmessage->message_alloc = systemeventmessage->message_length;
    }
    else {
      systemeventmessage->message_alloc = 0;
      systemeventmessage->message_length = 0;
    }
  }

  /* extract the data */
  for (int i = 0; i < systemeventmessage->message_length; i++) {
    systemeventmessage->message[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_COMMENT;
    store->type = R7KRECID_7kSystemEventMessage;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kSystemEventMessage:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_systemeventmessage(verbose, systemeventmessage, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_remotecontrolsettings(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_remotecontrolsettings *remotecontrolsettings;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  remotecontrolsettings = &(store->remotecontrolsettings);
  header = &(remotecontrolsettings->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(remotecontrolsettings->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->ping_number));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->sample_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->receiver_bandwidth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->pulse_width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->pulse_type));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->pulse_envelope));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->pulse_envelope_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->pulse_reserved));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->max_ping_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->ping_period));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->range_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->power_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->gain_selection));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->projector_magic_no));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->steering_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->steering_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->beamwidth_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->beamwidth_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->focal_point));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->projector_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->projector_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->transmit_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->hydrophone_magic_no));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->receive_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->receive_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(remotecontrolsettings->receive_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->range_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->range_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->depth_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->depth_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->absorption));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->spreading));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(remotecontrolsettings->reserved));
  index += 2;
  if (header->Size >=
      MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE + R7KHDRSIZE_7kRemoteControlSonarSettings) {
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->tx_offset_x));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->tx_offset_y));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->tx_offset_z));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->head_tilt_x));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->head_tilt_y));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->head_tilt_z));
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(remotecontrolsettings->ping_on_off));
    index += 2;
    remotecontrolsettings->data_sample_types = buffer[index];
    index++;
    remotecontrolsettings->projector_orientation = buffer[index];
    index++;
    mb_get_binary_short(true, &buffer[index], &(remotecontrolsettings->beam_angle_mode));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(remotecontrolsettings->r7kcenter_mode));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->gate_depth_min));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(remotecontrolsettings->gate_depth_max));
    index += 4;
    for (int i = 0; i < 35; i++) {
      mb_get_binary_short(true, &buffer[index], &(remotecontrolsettings->reserved2[i]));
      index += 2;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_7kRemoteControlSonarSettings;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kRemoteControlSonarSettings:       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], remotecontrolsettings->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_remotecontrolsettings(verbose, remotecontrolsettings, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_reserved(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_reserved *reserved;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  reserved = &(store->reserved);
  header = &(reserved->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < R7KHDRSIZE_7kReserved; i++) {
    reserved->reserved[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_RAW_LINE;
    store->type = R7KRECID_7kReserved;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kReserved:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_reserved(verbose, reserved, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_roll(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_roll *roll;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  roll = &(store->roll);
  header = &(roll->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(roll->roll));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ROLL;
    store->type = R7KRECID_7kRoll;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kRoll:                        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_roll(verbose, roll, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_pitch(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_pitch *pitch;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  pitch = &(store->pitch);
  header = &(pitch->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(pitch->pitch));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PITCH;
    store->type = R7KRECID_7kPitch;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kPitch:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_pitch(verbose, pitch, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_soundvelocity(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_soundvelocity *soundvelocity;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  soundvelocity = &(store->soundvelocity);
  header = &(soundvelocity->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(soundvelocity->soundvelocity));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SSV;
    store->type = R7KRECID_7kSoundVelocity;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kSoundVelocity:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_soundvelocity(verbose, soundvelocity, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_absorptionloss(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_absorptionloss *absorptionloss;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  absorptionloss = &(store->absorptionloss);
  header = &(absorptionloss->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(absorptionloss->absorptionloss));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ABSORPTIONLOSS;
    store->type = R7KRECID_7kAbsorptionLoss;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kAbsorptionLoss:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_absorptionloss(verbose, absorptionloss, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_spreadingloss(int verbose, char *buffer, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_spreadingloss *spreadingloss;
  unsigned int index = 0;
  int time_j[5] = {0, 0, 0, 0, 0};

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  spreadingloss = &(store->spreadingloss);
  header = &(spreadingloss->header);

  /* extract the header */
  index = 0;
  status = mbr_reson7kr_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(spreadingloss->spreadingloss));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SPREADINGLOSS;
    store->type = R7KRECID_7kSpreadingLoss;

    /* get the time */
    time_j[0] = header->s7kTime.Year;
    time_j[1] = header->s7kTime.Day;
    time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
    time_j[3] = (int)header->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, store->time_i);
    mb_get_time(verbose, store->time_i, &(store->time_d));
  }
  else {
    store->kind = MB_DATA_NONE;
  }

/* print out the results */
#ifdef MBR_RESON7KR_DEBUG
  fprintf(stderr,
          "R7KRECID_7kSpreadingLoss:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordNumber, header->Size, index);
#endif
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_spreadingloss(verbose, spreadingloss, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fsdwss *fsdwsslo;
  s7kr_fsdwss *fsdwsshi;
  s7kr_fsdwsb *fsdwsb;
  s7k_fsdwssheader *fsdwssheader;
  s7k_fsdwsegyheader *fsdwsegyheader;
  s7kr_bathymetry *bathymetry;
  s7kr_backscatter *backscatter;
  s7kr_beam *beam;
  s7kr_image *image;
  s7kr_beamgeometry *beamgeometry;
  s7kr_v2detection *v2detection;
  s7kr_v2rawdetection *v2rawdetection;
  double *edgetech_time_d;
  double *edgetech_dt;
  double *last_7k_time_d;
  FILE *mbfp;
  size_t read_len;
  int *current_ping;
  int *last_ping;
  int *new_ping;
  int *save_flag;
  int *recordid;
  int *recordidlast;
  int *deviceid;
  unsigned short *enumerator;
  int *fileheaders;
  char **bufferptr;
  char *buffer = NULL;
  int *bufferalloc;
  char **buffersaveptr;
  char *buffersave;
  int *size;
  int *nbadrec;
  int skip;
  bool ping_record;
  int time_j[5], time_i[7];
  double time_d;
  int nscan;
  int version_major, version_minor, version_svn;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  mbfp = mb_io_ptr->mbfp;

  /* get saved values */
  save_flag = (int *)&mb_io_ptr->save_flag;
  current_ping = (int *)&mb_io_ptr->save14;
  last_ping = (int *)&mb_io_ptr->save1;
  new_ping = (int *)&mb_io_ptr->save2;
  recordid = (int *)&mb_io_ptr->save3;
  recordidlast = (int *)&mb_io_ptr->save4;
  bufferptr = (char **)&mb_io_ptr->saveptr1;
  buffer = (char *)*bufferptr;
  bufferalloc = (int *)&mb_io_ptr->save6;
  buffersaveptr = (char **)&mb_io_ptr->saveptr2;
  buffersave = (char *)*buffersaveptr;
  size = (int *)&mb_io_ptr->save8;
  nbadrec = (int *)&mb_io_ptr->save9;
  deviceid = (int *)&mb_io_ptr->save10;
  enumerator = (unsigned short *)&mb_io_ptr->save11;
  fileheaders = (int *)&mb_io_ptr->save12;
  edgetech_time_d = (double *)&mb_io_ptr->saved3;
  edgetech_dt = (double *)&mb_io_ptr->saved4;
  last_7k_time_d = (double *)&mb_io_ptr->saved5;

  /* set file position */
  mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

  /* loop over reading data until a record is ready for return */
  bool done = false;
  *error = MB_ERROR_NO_ERROR;
  while (!done) {

    /* if previously read record stored use it first */
    if (*save_flag) {
      *save_flag = false;
      mbr_reson7kr_chk_header(verbose, mbio_ptr, buffersave, recordid, deviceid, enumerator, size);
      for (int i = 0; i < *size; i++)
        buffer[i] = buffersave[i];
    }

#ifdef MBTRN_ENABLED
        /* if reading from a socket ask for the entire next record
         * - the buffer is allocated to
         *     MBSYS_RESON7K_BUFFER_STARTSIZE = 65536 bytes (64 kB)
         *   at stream initialization
         *   which should be large enough for any single 7k record */
    else if (mb_io_ptr->mbsp != NULL) {
      read_len = (size_t)MBSYS_RESON7K_BUFFER_STARTSIZE;
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
            mbr_reson7kr_chk_header(verbose, mbio_ptr, buffer, recordid, deviceid, enumerator, size);
    }
#endif

    /* else if reading from a file deal with possibility of corruption by
     * first finding the next sync block, then reading the heading, and then
     * finally reading the rest of the record */
    else {
      /* read next record header into buffer */
      read_len = (size_t)MBSYS_RESON7K_VERSIONSYNCSIZE;
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);

      /* check header - if not a good header read a byte
          at a time until a good header is found */
      skip = 0;
      while (status == MB_SUCCESS &&
             mbr_reson7kr_chk_header(verbose, mbio_ptr, buffer, recordid, deviceid, enumerator, size) != MB_SUCCESS) {
        /* get next byte */
        for (int i = 0; i < MBSYS_RESON7K_VERSIONSYNCSIZE - 1; i++)
          buffer[i] = buffer[i + 1];
        read_len = (size_t)1;
        status = mb_fileio_get(verbose, mbio_ptr, &buffer[MBSYS_RESON7K_VERSIONSYNCSIZE - 1], &read_len, error);
        skip++;
      }

      /* report problem */
      if (skip > 0 && verbose >= 0) {
        if (*nbadrec == 0)
          fprintf(stderr, "\nThe MBF_RESON7KR module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...\n");
        fprintf(stderr, "MBF_RESON7KR skipped %d bytes between records %4.4X:%d and %4.4X:%d\n", skip, *recordidlast,
                *recordidlast, *recordid, *recordid);
        (*nbadrec)++;
      }
      *recordidlast = *recordid;
      store->type = *recordid;

      /* allocate memory to read rest of record if necessary */
      if (*bufferalloc < *size) {
        status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
        if (status == MB_SUCCESS)
          status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)buffersaveptr, error);
        if (status != MB_SUCCESS) {
          *bufferalloc = 0;
          done = true;
        }
        else {
          *bufferalloc = *size;
          buffer = (char *)*bufferptr;
          buffersave = (char *)*buffersaveptr;
        }
      }

      /* read the rest of the record */
      if (status == MB_SUCCESS) {
        read_len = (size_t)(*size - MBSYS_RESON7K_VERSIONSYNCSIZE);
        status = mb_fileio_get(verbose, mbio_ptr, &buffer[MBSYS_RESON7K_VERSIONSYNCSIZE], &read_len, error);
      }

#ifndef MBR_RESON7KR_DEBUG2
      if (skip > 0)
        fprintf(stderr, "RESON7KR record:skip:%d recordid:%x %d deviceid:%x %d enumerator:%x %d size:%d done:%d\n", skip,
                *recordid, *recordid, *deviceid, *deviceid, *enumerator, *enumerator, *size, done);
#endif
    }

    /* check for ping record and ping number */
    ping_record = false;
    if (status == MB_SUCCESS) {
      if (*recordid == R7KRECID_7kVolatileSonarSettings || *recordid == R7KRECID_7kMatchFilter ||
          *recordid == R7KRECID_7kBeamGeometry || *recordid == R7KRECID_7kRemoteControlSonarSettings ||
          *recordid == R7KRECID_7kBathymetricData || *recordid == R7KRECID_ProcessedSidescan ||
          *recordid == R7KRECID_7kBackscatterImageData || *recordid == R7KRECID_7kBeamData ||
          *recordid == R7KRECID_7kVerticalDepth || *recordid == R7KRECID_7kTVGData || *recordid == R7KRECID_7kImageData ||
          *recordid == R7KRECID_7kV2PingMotion || *recordid == R7KRECID_7kV2DetectionSetup ||
          *recordid == R7KRECID_7kV2BeamformedData || *recordid == R7KRECID_7kV2Detection ||
          *recordid == R7KRECID_7kV2RawDetection || *recordid == R7KRECID_7kV2SnippetData ||
          *recordid == R7KRECID_7kCalibratedSnippetData) {
        /* check for ping number */
        ping_record = true;
        mbr_reson7kr_chk_pingnumber(verbose, *recordid, buffer, new_ping);

        /* fix lack of ping number for backscatter and beam geometry records */
        if (*recordid == R7KRECID_7kBackscatterImageData && *new_ping <= 0)
          *new_ping = *last_ping;
        else if (*recordid == R7KRECID_7kBeamGeometry && *new_ping <= 0)
          *new_ping = *last_ping;

        /* set current ping */
        store->current_ping_number = *new_ping;

#ifdef MBR_RESON7KR_DEBUG2
        fprintf(stderr, "called mbr_reson7kr_chk_pingnumber recordid:%d last_ping:%d new_ping:%d\n", *recordid,
                *last_ping, *new_ping);
        fprintf(stderr, "current ping:%d records read: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                store->current_ping_number, store->read_volatilesettings, store->read_matchfilter,
                store->read_beamgeometry, store->read_remotecontrolsettings, store->read_bathymetry,
                store->read_backscatter, store->read_beam, store->read_verticaldepth, store->read_tvg, store->read_image,
                store->read_v2pingmotion, store->read_v2detectionsetup, store->read_v2beamformed, store->read_v2detection,
                store->read_v2rawdetection, store->read_v2snippet, store->read_processedsidescan);
#endif

        /* determine if record is continuation of the last ping
            or a new ping - if new ping and last ping not yet
            output then save the new record and output the
            last ping as fully read */
        if (*last_ping >= 0 && *new_ping >= 0 && *last_ping != *new_ping) {
          /* good ping if bathymetry record is read */
          if (store->read_bathymetry) {
            done = true;
            store->kind = MB_DATA_DATA;
            *save_flag = true;
            *current_ping = *last_ping;
            *last_ping = -1;
            for (int i = 0; i < *size; i++)
              buffersave[i] = buffer[i];

            /* get the time */
            bathymetry = &(store->bathymetry);
            header = &(bathymetry->header);
            time_j[0] = header->s7kTime.Year;
            time_j[1] = header->s7kTime.Day;
            time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
            time_j[3] = (int)header->s7kTime.Seconds;
            time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
            mb_get_itime(verbose, time_j, store->time_i);
            mb_get_time(verbose, store->time_i, &(store->time_d));
          }

          /* good ping if at least the detects are available */
          else if (store->read_v2detection) {
            done = true;
            store->kind = MB_DATA_DATA;
            *save_flag = true;
            *current_ping = *last_ping;
            *last_ping = -1;
            for (int i = 0; i < *size; i++)
              buffersave[i] = buffer[i];

            /* get the time */
            v2detection = &(store->v2detection);
            header = &(v2detection->header);
            time_j[0] = header->s7kTime.Year;
            time_j[1] = header->s7kTime.Day;
            time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
            time_j[3] = (int)header->s7kTime.Seconds;
            time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
            mb_get_itime(verbose, time_j, store->time_i);
            mb_get_time(verbose, store->time_i, &(store->time_d));
          }

          /* good ping if at least the raw detects are available */
          else if (store->read_v2rawdetection) {
            done = true;
            store->kind = MB_DATA_DATA;
            *save_flag = true;
            *current_ping = *last_ping;
            *last_ping = -1;
            for (int i = 0; i < *size; i++)
              buffersave[i] = buffer[i];

            /* get the time */
            v2rawdetection = &(store->v2rawdetection);
            header = &(v2rawdetection->header);
            time_j[0] = header->s7kTime.Year;
            time_j[1] = header->s7kTime.Day;
            time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
            time_j[3] = (int)header->s7kTime.Seconds;
            time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
            mb_get_itime(verbose, time_j, store->time_i);
            mb_get_time(verbose, store->time_i, &(store->time_d));
          }

          /* not a complete record unless there is bathymetry, drop the partial ping */
          else {
            done = false;
            *last_ping = -1;
            *last_ping = *new_ping;
            *save_flag = false;
          }
        }
        else if (*last_ping >= 0 && *new_ping >= 0 && *last_ping == *new_ping) {
          done = false;
        }
        else if (*last_ping == -1 && *new_ping >= 0) {
          done = false;
          *current_ping = -1;
          *last_ping = *new_ping;
          store->read_volatilesettings = false;
          store->read_matchfilter = false;
          store->read_beamgeometry = false;
          store->read_bathymetry = false;
          store->read_remotecontrolsettings = false;
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
        }
      }
    }

    /* check for ping data already read in read error case */
    if (status == MB_FAILURE && *last_ping >= 0
      && (store->read_bathymetry || store->read_v2detection || store->read_v2rawdetection)) {
      status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
      done = true;
      *save_flag = false;
      *last_ping = -1;
      store->kind = MB_DATA_DATA;
      store->time_d = *last_7k_time_d;
      mb_get_date(verbose, store->time_d, store->time_i);
    }

#ifdef MBR_RESON7KR_DEBUG2
    if (status == MB_SUCCESS && !done && !*save_flag) {
      fprintf(stderr, "Reading record id: %4.4X  %4.4d | %4.4X  %4.4d | %4.4hX  %4.4d |", *recordid, *recordid, *deviceid,
              *deviceid, *enumerator, *enumerator);
      if (*recordid == R7KRECID_ReferencePoint)
        fprintf(stderr, " R7KRECID_ReferencePoint %d\n", *recordid);
      if (*recordid == R7KRECID_UncalibratedSensorOffset)
        fprintf(stderr, " R7KRECID_UncalibratedSensorOffset %d\n", *recordid);
      if (*recordid == R7KRECID_CalibratedSensorOffset)
        fprintf(stderr, " R7KRECID_CalibratedSensorOffset %d\n", *recordid);
      if (*recordid == R7KRECID_Position)
        fprintf(stderr, " R7KRECID_Position %d\n", *recordid);
      if (*recordid == R7KRECID_CustomAttitude)
        fprintf(stderr, " R7KRECID_CustomAttitude %d\n", *recordid);
      if (*recordid == R7KRECID_Tide)
        fprintf(stderr, " R7KRECID_Tide %d\n", *recordid);
      if (*recordid == R7KRECID_Altitude)
        fprintf(stderr, " R7KRECID_Altitude %d\n", *recordid);
      if (*recordid == R7KRECID_MotionOverGround)
        fprintf(stderr, " R7KRECID_MotionOverGround %d\n", *recordid);
      if (*recordid == R7KRECID_Depth)
        fprintf(stderr, " R7KRECID_Depth %d\n", *recordid);
      if (*recordid == R7KRECID_SoundVelocityProfile)
        fprintf(stderr, " R7KRECID_SoundVelocityProfile %d\n", *recordid);
      if (*recordid == R7KRECID_CTD)
        fprintf(stderr, " R7KRECID_CTD %d\n", *recordid);
      if (*recordid == R7KRECID_Geodesy)
        fprintf(stderr, " R7KRECID_Geodesy %d\n", *recordid);
      if (*recordid == R7KRECID_RollPitchHeave)
        fprintf(stderr, " R7KRECID_RollPitchHeave %d\n", *recordid);
      if (*recordid == R7KRECID_Heading)
        fprintf(stderr, " R7KRECID_Heading %d\n", *recordid);
      if (*recordid == R7KRECID_SurveyLine)
        fprintf(stderr, " R7KRECID_SurveyLine %d\n", *recordid);
      if (*recordid == R7KRECID_Navigation)
        fprintf(stderr, " R7KRECID_Navigation %d\n", *recordid);
      if (*recordid == R7KRECID_Attitude)
        fprintf(stderr, " R7KRECID_Attitude %d\n", *recordid);
      if (*recordid == R7KRECID_Rec1022)
        fprintf(stderr, " R7KRECID_Rec1022 %d\n", *recordid);
      if (*recordid == R7KRECID_GenericSensorCalibration)
        fprintf(stderr, " R7KRECID_GenericSensorCalibration %d\n", *recordid);
      if (*recordid == R7KRECID_GenericSidescan)
        fprintf(stderr, " R7KRECID_GenericSidescan %d\n", *recordid);
      if (*recordid == R7KRECID_FSDWsidescan)
        fprintf(stderr, " R7KRECID_FSDWsidescan %d\n", *recordid);
      if (*recordid == R7KRECID_FSDWsubbottom)
        fprintf(stderr, " R7KRECID_FSDWsubbottom %d\n", *recordid);
      if (*recordid == R7KRECID_Bluefin)
        fprintf(stderr, " R7KRECID_Bluefin %d\n", *recordid);
      if (*recordid == R7KRECID_ProcessedSidescan)
        fprintf(stderr, " R7KRECID_ProcessedSidescan %d\n", *recordid);
      if (*recordid == R7KRECID_7kVolatileSonarSettings)
        fprintf(stderr, " R7KRECID_7kVolatileSonarSettings %d\n", *recordid);
      if (*recordid == R7KRECID_7kConfiguration)
        fprintf(stderr, " R7KRECID_7kConfiguration %d\n", *recordid);
      if (*recordid == R7KRECID_7kMatchFilter)
        fprintf(stderr, " R7KRECID_7kMatchFilter %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2FirmwareHardwareConfiguration)
        fprintf(stderr, " R7KRECID_7kV2FirmwareHardwareConfiguration %d\n", *recordid);
      if (*recordid == R7KRECID_7kBeamGeometry)
        fprintf(stderr, " R7KRECID_7kBeamGeometry %d\n", *recordid);
      if (*recordid == R7KRECID_7kCalibrationData)
        fprintf(stderr, " R7KRECID_7kCalibrationData %d\n", *recordid);
      if (*recordid == R7KRECID_7kBathymetricData)
        fprintf(stderr, " R7KRECID_7kBathymetricData %d\n", *recordid);
      if (*recordid == R7KRECID_7kBackscatterImageData)
        fprintf(stderr, " R7KRECID_7kBackscatterImageData %d\n", *recordid);
      if (*recordid == R7KRECID_7kBeamData)
        fprintf(stderr, " R7KRECID_7kBeamData %d\n", *recordid);
      if (*recordid == R7KRECID_7kVerticalDepth)
        fprintf(stderr, " R7KRECID_7kVerticalDepth %d\n", *recordid);
      if (*recordid == R7KRECID_7kTVGData)
        fprintf(stderr, " R7KRECID_7kTVGData %d\n", *recordid);
      if (*recordid == R7KRECID_7kImageData)
        fprintf(stderr, " R7KRECID_7kImageData %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2PingMotion)
        fprintf(stderr, " R7KRECID_7kV2PingMotion %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2DetectionSetup)
        fprintf(stderr, " R7KRECID_7kV2DetectionSetup %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2BeamformedData)
        fprintf(stderr, " R7KRECID_7kV2BeamformedData %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2BITEData)
        fprintf(stderr, " R7KRECID_7kV2BITEData %d\n", *recordid);
      if (*recordid == R7KRECID_7kV27kCenterVersion)
        fprintf(stderr, " R7KRECID_7kV27kCenterVersion %d\n", *recordid);
      if (*recordid == R7KRECID_7kV28kWetEndVersion)
        fprintf(stderr, " R7KRECID_7kV28kWetEndVersion %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2Detection)
        fprintf(stderr, " R7KRECID_7kV2Detection %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2RawDetection)
        fprintf(stderr, " R7KRECID_7kV2RawDetection %d\n", *recordid);
      if (*recordid == R7KRECID_7kV2SnippetData)
        fprintf(stderr, " R7KRECID_7kV2SnippetData %d\n", *recordid);
      if (*recordid == R7KRECID_7kCalibrationData)
        fprintf(stderr, " R7KRECID_7kCalibratedSnippetData %d\n", *recordid);
      if (*recordid == R7KRECID_7kInstallationParameters)
        fprintf(stderr, " R7KRECID_7kInstallationParameters %d\n", *recordid);
      if (*recordid == R7KRECID_7kSystemEventMessage)
        fprintf(stderr, "R7KRECID_7kSystemEventMessage %d\n", *recordid);
      if (*recordid == R7KRECID_7kDataStorageStatus)
        fprintf(stderr, " R7KRECID_7kDataStorageStatus %d\n", *recordid);
      if (*recordid == R7KRECID_7kFileHeader)
        fprintf(stderr, " R7KRECID_7kFileHeader %d\n", *recordid);
      if (*recordid == R7KRECID_7kFileCatalog)
        fprintf(stderr, " R7KRECID_7kFileCatalog %d\n", *recordid);
      if (*recordid == R7KRECID_7kTriggerSequenceSetup)
        fprintf(stderr, " R7KRECID_7kTriggerSequenceSetup %d\n", *recordid);
      if (*recordid == R7KRECID_7kTriggerSequenceDone)
        fprintf(stderr, " R7KRECID_7kTriggerSequenceDone %d\n", *recordid);
      if (*recordid == R7KRECID_7kTimeMessage)
        fprintf(stderr, " R7KRECID_7kTimeMessage %d\n", *recordid);
      if (*recordid == R7KRECID_7kRemoteControl)
        fprintf(stderr, " R7KRECID_7kRemoteControl %d\n", *recordid);
      if (*recordid == R7KRECID_7kRemoteControlAcknowledge)
        fprintf(stderr, " R7KRECID_7kRemoteControlAcknowledge %d\n", *recordid);
      if (*recordid == R7KRECID_7kRemoteControlNotAcknowledge)
        fprintf(stderr, " R7KRECID_7kRemoteControlNotAcknowledge %d\n", *recordid);
      if (*recordid == R7KRECID_7kRemoteControlSonarSettings)
        fprintf(stderr, " R7KRECID_7kRemoteControlSonarSettings %d\n", *recordid);
      if (*recordid == R7KRECID_7kReserved)
        fprintf(stderr, " R7KRECID_7kReserved %d\n", *recordid);
      if (*recordid == R7KRECID_7kRoll)
        fprintf(stderr, " R7KRECID_7kRoll %d\n", *recordid);
      if (*recordid == R7KRECID_7kPitch)
        fprintf(stderr, " R7KRECID_7kPitch %d\n", *recordid);
      if (*recordid == R7KRECID_7kSoundVelocity)
        fprintf(stderr, " R7KRECID_7kSoundVelocity %d\n", *recordid);
      if (*recordid == R7KRECID_7kAbsorptionLoss)
        fprintf(stderr, " R7KRECID_7kAbsorptionLoss %d\n", *recordid);
      if (*recordid == R7KRECID_7kSpreadingLoss)
        fprintf(stderr, " R7KRECID_7kSpreadingLoss %d\n", *recordid);
      if (*recordid == R7KRECID_7kFiller)
        fprintf(stderr, " R7KRECID_7kFiller %d\n", *recordid);
      if (*recordid == R7KRECID_8100SonarData)
        fprintf(stderr, " R7KRECID_8100SonarData %d\n", *recordid);
    }
#endif

    /* set done if read failure */
    if (status == MB_FAILURE) {
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "call nothing, read failure\n");
#endif
      done = true;
    }

    /* if needed parse the data record */
    if (status == MB_SUCCESS && !done) {
      if (*recordid == R7KRECID_7kFileHeader) {
        status = mbr_reson7kr_rd_fileheader(verbose, buffer, store_ptr, error);
        (*fileheaders)++;
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fileheader++;
        }

        /* kluge to set bogus background navigation */
        /*klugelon = -121.0;
        klugelat = 36.0;
        mb_navint_add(verbose, mbio_ptr,
                store->time_d,
                klugelon,
                klugelat,
                error);
        klugelon = -121.0;
        klugelat = 37.168;
        mb_navint_add(verbose, mbio_ptr,
                store->time_d + 86400.0,
                klugelon,
                klugelat,
                error);*/
      }
      else if (*recordid == R7KRECID_ReferencePoint) {
        status = mbr_reson7kr_rd_reference(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_reference++;
        }
      }
      else if (*recordid == R7KRECID_UncalibratedSensorOffset) {
        status = mbr_reson7kr_rd_sensoruncal(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_sensoruncal++;
        }
      }
      else if (*recordid == R7KRECID_CalibratedSensorOffset) {
        status = mbr_reson7kr_rd_sensorcal(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_sensorcal++;
        }
      }
      else if (*recordid == R7KRECID_Position) {
        status = mbr_reson7kr_rd_position(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS)
          done = true;
      }
      else if (*recordid == R7KRECID_CustomAttitude) {
        status = mbr_reson7kr_rd_customattitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_customattitude++;
        }
      }
      else if (*recordid == R7KRECID_Tide) {
        status = mbr_reson7kr_rd_tide(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_tide++;
        }
      }
      else if (*recordid == R7KRECID_Altitude) {
        status = mbr_reson7kr_rd_altitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_altitude++;
        }
      }
      else if (*recordid == R7KRECID_MotionOverGround) {
        status = mbr_reson7kr_rd_motion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_motion++;
        }
      }
      else if (*recordid == R7KRECID_Depth) {
        status = mbr_reson7kr_rd_depth(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_depth++;
        }
      }
      else if (*recordid == R7KRECID_SoundVelocityProfile) {
        status = mbr_reson7kr_rd_svp(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_svp++;
        }
      }
      else if (*recordid == R7KRECID_CTD) {
        status = mbr_reson7kr_rd_ctd(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_ctd++;
        }
      }
      else if (*recordid == R7KRECID_Geodesy) {
        status = mbr_reson7kr_rd_geodesy(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_geodesy++;
        }
      }
      else if (*recordid == R7KRECID_RollPitchHeave) {
        status = mbr_reson7kr_rd_rollpitchheave(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_rollpitchheave++;
        }
      }
      else if (*recordid == R7KRECID_Heading) {
        status = mbr_reson7kr_rd_heading(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_heading++;
        }
      }
      else if (*recordid == R7KRECID_SurveyLine) {
        status = mbr_reson7kr_rd_surveyline(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_surveyline++;
        }
      }
      else if (*recordid == R7KRECID_Navigation) {
        status = mbr_reson7kr_rd_navigation(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_navigation++;
        }
      }
      else if (*recordid == R7KRECID_Attitude) {
        status = mbr_reson7kr_rd_attitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_attitude++;
        }
      }
      else if (*recordid == R7KRECID_Rec1022) {
        status = mbr_reson7kr_rd_rec1022(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_rec1022++;
        }
      }
      else if (*recordid == R7KRECID_FSDWsidescan && *deviceid == R7KDEVID_EdgetechFSDW && *enumerator == 20) {
        status = mbr_reson7kr_rd_fsdwsslo(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fsdwsslo++;

          /* store the Edgetech timestamp for possible use in fixing 7k time */
          fsdwsslo = &(store->fsdwsslo);
          header = &(fsdwsslo->header);
          fsdwssheader = &(fsdwsslo->ssheader[0]);
          time_j[0] = fsdwssheader->year;
          time_j[1] = fsdwssheader->day;
          time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
          time_j[3] = fsdwssheader->second;
          time_j[4] =
              1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          if (*edgetech_time_d > 0.0 && time_d - *edgetech_time_d > 0.002)
            *edgetech_dt = time_d - *edgetech_time_d;
          *edgetech_time_d = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "mbr_reson7kr_rd_fsdwsslo: EDGETECH TIME: %f %f\n", *edgetech_time_d, *edgetech_dt);
#endif
        }
      }
      else if (*recordid == R7KRECID_FSDWsidescan && *deviceid == R7KDEVID_EdgetechFSDWSSLF) {
        status = mbr_reson7kr_rd_fsdwsslo(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fsdwsslo++;

          /* store the Edgetech timestamp for possible use in fixing 7k time */
          fsdwsslo = &(store->fsdwsslo);
          header = &(fsdwsslo->header);
          fsdwssheader = &(fsdwsslo->ssheader[0]);
          time_j[0] = fsdwssheader->year;
          time_j[1] = fsdwssheader->day;
          time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
          time_j[3] = fsdwssheader->second;
          time_j[4] =
              1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          if (*edgetech_time_d > 0.0 && time_d - *edgetech_time_d > 0.002)
            *edgetech_dt = time_d - *edgetech_time_d;
          *edgetech_time_d = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "mbr_reson7kr_rd_fsdwsslo: EDGETECH TIME: %f %f\n", *edgetech_time_d, *edgetech_dt);
#endif
        }
      }
      else if (*recordid == R7KRECID_FSDWsidescan && *deviceid == R7KDEVID_EdgetechFSDW && *enumerator == 21) {
        status = mbr_reson7kr_rd_fsdwsshi(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fsdwsshi++;

          /* store the Edgetech timestamp for possible use in fixing 7k time */
          fsdwsshi = &(store->fsdwsshi);
          header = &(fsdwsshi->header);
          fsdwssheader = &(fsdwsshi->ssheader[0]);
          time_j[0] = fsdwssheader->year;
          time_j[1] = fsdwssheader->day;
          time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
          time_j[3] = fsdwssheader->second;
          time_j[4] =
              1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          if (*edgetech_time_d > 0.0 && time_d - *edgetech_time_d > 0.002)
            *edgetech_dt = time_d - *edgetech_time_d;
          *edgetech_time_d = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "mbr_reson7kr_rd_fsdwsshi: EDGETECH TIME: %f %f\n", *edgetech_time_d, *edgetech_dt);
#endif
        }
      }
      else if (*recordid == R7KRECID_FSDWsidescan && *deviceid == R7KDEVID_EdgetechFSDWSSHF) {
        status = mbr_reson7kr_rd_fsdwsshi(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fsdwsshi++;

          /* store the Edgetech timestamp for possible use in fixing 7k time */
          fsdwsshi = &(store->fsdwsshi);
          header = &(fsdwsshi->header);
          fsdwssheader = &(fsdwsshi->ssheader[0]);
          time_j[0] = fsdwssheader->year;
          time_j[1] = fsdwssheader->day;
          time_j[2] = 60 * fsdwssheader->hour + fsdwssheader->minute;
          time_j[3] = fsdwssheader->second;
          time_j[4] =
              1000 * (fsdwssheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwssheader->millisecondsToday)));
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          if (*edgetech_time_d > 0.0 && time_d - *edgetech_time_d > 0.002)
            *edgetech_dt = time_d - *edgetech_time_d;
          *edgetech_time_d = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "mbr_reson7kr_rd_fsdwsshi: EDGETECH TIME: %f %f\n", *edgetech_time_d, *edgetech_dt);
#endif
        }
      }
      else if (*recordid == R7KRECID_FSDWsubbottom) {
        status = mbr_reson7kr_rd_fsdwsb(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_fsdwsb++;

          /* store the Edgetech timestamp for possible use in fixing 7k time */
          fsdwsb = &(store->fsdwsb);
          header = &(fsdwsb->header);
          fsdwsegyheader = &(fsdwsb->segyheader);
          time_j[0] = fsdwsegyheader->year;
          time_j[1] = fsdwsegyheader->day;
          time_j[2] = 60 * fsdwsegyheader->hour + fsdwsegyheader->minute;
          time_j[3] = fsdwsegyheader->second;
          time_j[4] =
              1000 * (fsdwsegyheader->millisecondsToday - 1000 * ((int)(0.001 * fsdwsegyheader->millisecondsToday)));
          mb_get_itime(verbose, time_j, time_i);
          mb_get_time(verbose, time_i, &time_d);
          if (*edgetech_time_d > 0.0 && time_d - *edgetech_time_d > 0.002)
            *edgetech_dt = time_d - *edgetech_time_d;
          *edgetech_time_d = time_d;
#ifdef MBR_RESON7KR_DEBUG2
          fprintf(stderr, "mbr_reson7kr_rd_fsdwsb:    EDGETECH TIME: %f %f\n", *edgetech_time_d, *edgetech_dt);
#endif
        }
      }
      else if (*recordid == R7KRECID_Bluefin) {
        status = mbr_reson7kr_rd_bluefin(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          if (store->bluefin.data_format == R7KRECID_BluefinNav)
            store->nrec_bluefinnav++;
          else if (store->bluefin.data_format == R7KRECID_BluefinEnvironmental)
            store->nrec_bluefinenv++;
        }
      }
      else if (*recordid == R7KRECID_ProcessedSidescan) {
        status = mbr_reson7kr_rd_processedsidescan(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_processedsidescan = true;
          store->nrec_processedsidescan++;
        }
      }
      else if (*recordid == R7KRECID_7kVolatileSonarSettings) {
        status = mbr_reson7kr_rd_volatilesonarsettings(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_volatilesettings = true;
          store->nrec_volatilesonarsettings++;
        }
      }
      else if (*recordid == R7KRECID_7kConfiguration) {
        status = mbr_reson7kr_rd_configuration(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_configuration++;
        }
      }
      else if (*recordid == R7KRECID_7kMatchFilter) {
        status = mbr_reson7kr_rd_matchfilter(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_matchfilter = true;
          store->nrec_matchfilter++;
        }
      }
      else if (*recordid == R7KRECID_7kV2FirmwareHardwareConfiguration) {
        status = mbr_reson7kr_rd_v2firmwarehardwareconfiguration(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_v2firmwarehardwareconfiguration++;
        }
      }
      else if (*recordid == R7KRECID_7kBeamGeometry) {
        status = mbr_reson7kr_rd_beamgeometry(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_beamgeometry = true;
          done = false;
          store->nrec_beamgeometry++;

          /* set beam widths */
          beamgeometry = &(store->beamgeometry);
          mb_io_ptr->beamwidth_xtrack = RTD * beamgeometry->beamwidth_acrosstrack[beamgeometry->number_beams / 2];
          mb_io_ptr->beamwidth_ltrack = RTD * beamgeometry->beamwidth_alongtrack[beamgeometry->number_beams / 2];
        }
      }
      else if (*recordid == R7KRECID_7kCalibrationData) {
        status = mbr_reson7kr_rd_calibration(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_calibration++;
        }
      }
      else if (*recordid == R7KRECID_7kBathymetricData) {
        status = mbr_reson7kr_rd_bathymetry(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_bathymetry = true;
          store->nrec_bathymetry++;

          /* if needed use most recent Edgetech timestamp to fix 7k time */
          bathymetry = &(store->bathymetry);
          header = &(bathymetry->header);
          *last_7k_time_d = store->time_d;
          if (header->s7kTime.Year < 2004 && *edgetech_time_d > 0.0 && *edgetech_dt > 0.0 && *edgetech_dt < 2.0) {
            if (*edgetech_time_d + *edgetech_dt > *last_7k_time_d + 0.002) {
              store->time_d = *edgetech_time_d + *edgetech_dt;
            }
            else {
              store->time_d = *edgetech_time_d + 2 * (*edgetech_dt);
            }
            mb_get_date(verbose, store->time_d, store->time_i);
            mb_get_jtime(verbose, store->time_i, time_j);
            header->s7kTime.Year = store->time_i[0];
            header->s7kTime.Day = time_j[1];
            header->s7kTime.Hours = store->time_i[3];
            header->s7kTime.Minutes = store->time_i[4];
            header->s7kTime.Seconds = store->time_i[5] + 0.000001 * store->time_i[6];
#ifdef MBR_RESON7KR_DEBUG2
            fprintf(stderr,
                    "TIME CORRECTION: R7KRECID_7kBathymetricData:        7Ktime(%4.4d/%2.2d/%2.2d "
                    "%2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
                    store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4],
                    store->time_i[5], store->time_i[6], header->RecordNumber, bathymetry->ping_number);
#endif
          }
        }

        /*mbsys_reson7k_print_bathymetry(verbose, &(store->bathymetry), error);*/
      }
      else if (*recordid == R7KRECID_7kBackscatterImageData) {
        status = mbr_reson7kr_rd_backscatter(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_backscatter = true;
          store->nrec_backscatter++;

          /* if needed use most recent Edgetech timestamp to fix 7k time */
          backscatter = &(store->backscatter);
          header = &(backscatter->header);
          *last_7k_time_d = store->time_d;
          if (header->s7kTime.Year < 2004 && *edgetech_time_d > 0.0 && *edgetech_dt > 0.0 && *edgetech_dt < 2.0) {
            if (*edgetech_time_d + *edgetech_dt > *last_7k_time_d + 0.002) {
              store->time_d = *edgetech_time_d + *edgetech_dt;
            }
            else {
              store->time_d = *edgetech_time_d + 2 * (*edgetech_dt);
            }
            mb_get_date(verbose, store->time_d, store->time_i);
            mb_get_jtime(verbose, store->time_i, time_j);
            header->s7kTime.Year = store->time_i[0];
            header->s7kTime.Day = time_j[1];
            header->s7kTime.Hours = store->time_i[3];
            header->s7kTime.Minutes = store->time_i[4];
            header->s7kTime.Seconds = store->time_i[5] + 0.000001 * store->time_i[6];
#ifdef MBR_RESON7KR_DEBUG2
            fprintf(stderr,
                    "TIME CORRECTION: R7KRECID_7kBackscatterImageData:   7Ktime(%4.4d/%2.2d/%2.2d "
                    "%2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
                    store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4],
                    store->time_i[5], store->time_i[6], header->RecordNumber, backscatter->ping_number);
#endif
          }
        }
      }
      else if (*recordid == R7KRECID_7kBeamData) {
        status = mbr_reson7kr_rd_beam(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_beam = true;
          store->nrec_beam++;

          /* if needed use most recent Edgetech timestamp to fix 7k time */
          beam = &(store->beam);
          header = &(beam->header);
          *last_7k_time_d = store->time_d;
          if (header->s7kTime.Year < 2004 && *edgetech_time_d > 0.0 && *edgetech_dt > 0.0 && *edgetech_dt < 2.0) {
            if (*edgetech_time_d + *edgetech_dt > *last_7k_time_d + 0.002) {
              store->time_d = *edgetech_time_d + *edgetech_dt;
            }
            else {
              store->time_d = *edgetech_time_d + 2 * (*edgetech_dt);
            }
            mb_get_date(verbose, store->time_d, store->time_i);
            mb_get_jtime(verbose, store->time_i, time_j);
            header->s7kTime.Year = store->time_i[0];
            header->s7kTime.Day = time_j[1];
            header->s7kTime.Hours = store->time_i[3];
            header->s7kTime.Minutes = store->time_i[4];
            header->s7kTime.Seconds = store->time_i[5] + 0.000001 * store->time_i[6];
#ifdef MBR_RESON7KR_DEBUG2
            fprintf(stderr,
                    "TIME CORRECTION: R7KRECID_7kBeamData: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) "
                    "record_number:%d ping:%d\n",
                    store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4],
                    store->time_i[5], store->time_i[6], header->RecordNumber, beam->ping_number);
#endif
          }
        }
      }
      else if (*recordid == R7KRECID_7kVerticalDepth) {
        status = mbr_reson7kr_rd_verticaldepth(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_verticaldepth = true;
          store->nrec_verticaldepth++;
        }
      }
      else if (*recordid == R7KRECID_7kTVGData) {
        status = mbr_reson7kr_rd_tvg(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_tvg = true;
          store->nrec_tvg++;
        }
      }
      else if (*recordid == R7KRECID_7kImageData) {
        status = mbr_reson7kr_rd_image(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_image = true;
          store->nrec_image++;

          /* if needed use most recent Edgetech timestamp to fix 7k time */
          image = &(store->image);
          header = &(image->header);
          *last_7k_time_d = store->time_d;
          if (header->s7kTime.Year < 2004 && *edgetech_time_d > 0.0 && *edgetech_dt > 0.0 && *edgetech_dt < 2.0) {
            if (*edgetech_time_d + *edgetech_dt > *last_7k_time_d + 0.002) {
              store->time_d = *edgetech_time_d + *edgetech_dt;
            }
            else {
              store->time_d = *edgetech_time_d + 2 * (*edgetech_dt);
            }
            mb_get_date(verbose, store->time_d, store->time_i);
            mb_get_jtime(verbose, store->time_i, time_j);
            header->s7kTime.Year = store->time_i[0];
            header->s7kTime.Day = time_j[1];
            header->s7kTime.Hours = store->time_i[3];
            header->s7kTime.Minutes = store->time_i[4];
            header->s7kTime.Seconds = store->time_i[5] + 0.000001 * store->time_i[6];
#ifdef MBR_RESON7KR_DEBUG2
            fprintf(stderr,
                    "TIME CORRECTION: R7KRECID_7kImageData:              7Ktime(%4.4d/%2.2d/%2.2d "
                    "%2.2d:%2.2d:%2.2d.%6.6d) record_number:%d ping:%d\n",
                    store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4],
                    store->time_i[5], store->time_i[6], header->RecordNumber, image->ping_number);
#endif
          }
        }
      }
      else if (*recordid == R7KRECID_7kV2PingMotion) {
        status = mbr_reson7kr_rd_v2pingmotion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2pingmotion = true;
          store->nrec_v2pingmotion++;
        }
      }
      else if (*recordid == R7KRECID_7kV2DetectionSetup) {
        status = mbr_reson7kr_rd_v2detectionsetup(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2detectionsetup = true;
          store->nrec_v2detectionsetup++;
        }
      }
      else if (*recordid == R7KRECID_7kV2BeamformedData) {
        status = mbr_reson7kr_rd_v2beamformed(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2beamformed = true;
          store->nrec_v2beamformed++;
        }
      }
      else if (*recordid == R7KRECID_7kV2BITEData) {
        status = mbr_reson7kr_rd_v2bite(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_v2bite++;
        }
      }
      else if (*recordid == R7KRECID_7kV27kCenterVersion) {
        status = mbr_reson7kr_rd_v27kcenterversion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_v27kcenterversion++;
        }
      }
      else if (*recordid == R7KRECID_7kV28kWetEndVersion) {
        status = mbr_reson7kr_rd_v28kwetendversion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_v28kwetendversion++;
        }
      }
      else if (*recordid == R7KRECID_7kV2Detection) {
        status = mbr_reson7kr_rd_v2detection(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2detection = true;
          store->nrec_v2detection++;
        }
      }
      else if (*recordid == R7KRECID_7kV2RawDetection) {
        status = mbr_reson7kr_rd_v2rawdetection(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2rawdetection = true;
          store->nrec_v2rawdetection++;
        }
      }
      else if (*recordid == R7KRECID_7kV2SnippetData) {
        status = mbr_reson7kr_rd_v2snippet(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_v2snippet = true;
          store->nrec_v2snippet++;
        }
      }
      else if (*recordid == R7KRECID_7kCalibratedSnippetData) {
        status = mbr_reson7kr_rd_calibratedsnippet(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_calibratedsnippet = true;
          store->nrec_calibratedsnippet++;
        }
      }
      else if (*recordid == R7KRECID_7kInstallationParameters) {
        status = mbr_reson7kr_rd_installation(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_installation++;
        }
      }
      else if (*recordid == R7KRECID_7kSystemEventMessage) {
        status = mbr_reson7kr_rd_systemeventmessage(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_systemeventmessage++;
        }
      }
      else if (*recordid == R7KRECID_7kRemoteControlSonarSettings) {
        status = mbr_reson7kr_rd_remotecontrolsettings(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->read_remotecontrolsettings = true;
          done = false;
          store->nrec_remotecontrolsettings++;
        }
      }
      else if (*recordid == R7KRECID_7kReserved) {
        status = mbr_reson7kr_rd_reserved(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_reserved++;
        }
      }
      else if (*recordid == R7KRECID_7kRoll) {
        status = mbr_reson7kr_rd_roll(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_roll++;
        }
      }
      else if (*recordid == R7KRECID_7kPitch) {
        status = mbr_reson7kr_rd_pitch(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_pitch++;
        }
      }
      else if (*recordid == R7KRECID_7kSoundVelocity) {
        status = mbr_reson7kr_rd_soundvelocity(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_soundvelocity++;
        }
      }
      else if (*recordid == R7KRECID_7kAbsorptionLoss) {
        status = mbr_reson7kr_rd_absorptionloss(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_absorptionloss++;
        }
      }
      else if (*recordid == R7KRECID_7kSpreadingLoss) {
        status = mbr_reson7kr_rd_spreadingloss(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_spreadingloss++;
        }
      }
      else {

#ifdef MBR_RESON7KR_DEBUG2
        fprintf(stderr, "Record type %d - recognized but not supported.\n", *recordid);
#endif
        done = false;
        store->nrec_other++;
      }

      /* check if ping record is known to be done */
      if (status == MB_SUCCESS && ping_record && store->read_v2detectionsetup) {
        if (status == MB_SUCCESS && ping_record && store->read_volatilesettings &&
            store->read_matchfilter && store->read_beamgeometry && store->read_bathymetry &&
            store->read_remotecontrolsettings && store->read_backscatter &&
            store->read_beam &&
            store->read_verticaldepth
            /* && store->read_tvg */
            && store->read_image && store->read_v2pingmotion &&
            store->read_v2detectionsetup && store->read_v2beamformed &&
            store->read_v2detection && store->read_v2rawdetection &&
            store->read_v2snippet) {
          done = true;
          *current_ping = *last_ping;
          *last_ping = -1;
        }
      }

      /* check for MB-System format error in bathymetry records by checking comments
       * for MB-System distributions earlier than 4.3.2004 */
      if (status == MB_SUCCESS && *recordid == R7KRECID_7kSystemEventMessage
                && store->systemeventmessage.message_length > 0
                && store->bathymetry.acrossalongerror == MB_MAYBE) {
        nscan = sscanf(store->systemeventmessage.message, "MB-System Version %d.%d.%d", &version_major, &version_minor,
                       &version_svn);
        if (nscan == 0)
          nscan = sscanf(store->systemeventmessage.message, "MB-system Version %d.%d.%d", &version_major,
                         &version_minor, &version_svn);
        if (nscan == 3 && (version_major < 5 || (version_major == 5 && version_minor < 3) ||
                           (version_major == 5 && version_minor == 3 && version_svn < 2004))) {
          store->bathymetry.acrossalongerror = MB_YES;
        }
        else if (nscan == 2 && (version_major < 5 || (version_major == 5 && version_minor < 3))) {
          store->bathymetry.acrossalongerror = MB_NO;
                }
        else if (nscan >= 2) {
          store->bathymetry.acrossalongerror = MB_NO;
        }
      }
    }

    /* bail out if there is a parsing error */
    if (status == MB_FAILURE)
      done = true;
#ifdef MBR_RESON7KR_DEBUG2
    if (verbose >= 0) {
      fprintf(stderr, "done:%d kind:%d recordid:%x size:%d status:%d error:%d\n", done, store->kind, *recordid, *size,
              status, *error);
      fprintf(stderr, "end of mbr_reson7kr_rd_data loop:\n\n");
    }
/*if (status == MB_SUCCESS)
{
if (*save_flag)
fprintf(stderr,"RECORD SAVED\n");
else
fprintf(stderr,"status:%d recordid:%d ping_record:%d current:%d last:%d new:%d  done:%d recs:%d %d %d %d %d %d %d %d %d\n",
status,*recordid,ping_record,*current_ping,*last_ping,*new_ping,done,
store->read_volatilesettings,store->read_matchfilter,
store->read_beamgeometry,store->read_bathymetry,store->read_backscatter,
store->read_beam,store->read_verticaldepth,store->read_tvg,store->read_image);
}*/
#endif
  }
#ifdef MBR_RESON7KR_DEBUG2
  if (status == MB_SUCCESS)
    fprintf(stderr, "RESON7KR DATA READ: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

  /* get file position - check file and socket, use appropriate ftelln */
    if (mb_io_ptr->mbfp != NULL) {
         if (*save_flag)
            mb_io_ptr->file_bytes = ftell(mbfp) - *size;
        else
            mb_io_ptr->file_bytes = ftell(mbfp);
    }
#ifdef MBTRN_ENABLED
    else if (mb_io_ptr->mbsp != NULL) {
        if (*save_flag)
            mb_io_ptr->file_bytes = r7kr_reader_tell(mb_io_ptr->mbsp) - *size;
        else
            mb_io_ptr->file_bytes = r7kr_reader_tell(mb_io_ptr->mbsp);
    }else{
        fprintf(stderr,"ERROR - both file and socket input pointers are NULL\n");
    }
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  int interp_status;
  int interp_error = MB_ERROR_NO_ERROR;
  s7kr_position *position;
  s7kr_navigation *navigation;
  s7kr_attitude *attitude;
  s7kr_heading *dheading;
  s7kr_rollpitchheave *rollpitchheave;
  s7kr_customattitude *customattitude;
  s7kr_altitude *altitude;
  s7kr_depth *depth;
  s7kr_volatilesettings *volatilesettings;
  s7kr_beamgeometry *beamgeometry;
  s7kr_bathymetry *bathymetry;
  s7kr_v2detection *v2detection;
  s7kr_v2detectionsetup *v2detectionsetup;
  s7kr_v2rawdetection *v2rawdetection;
  s7kr_bluefin *bluefin;
  int ss_source;
  double speed, heading, longitude, latitude;
  double roll, pitch, heave;
  double sonar_depth, sonar_altitude;
  double theta, phi;
  double *pixel_size, *swath_width;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* read next data from file */
  status = mbr_reson7kr_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* get pointers to data structures */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  position = &store->position;
  attitude = &store->attitude;
  volatilesettings = &store->volatilesettings;
  beamgeometry = &store->beamgeometry;
  bathymetry = &store->bathymetry;
  // s7kr_backscatter *backscatter = &store->backscatter;
  // s7kr_beam *beam = &store->beam;
  // s7kr_image *image = &store->image;
  v2detectionsetup = &store->v2detectionsetup;
  v2detection = &store->v2detection;
  v2rawdetection = &store->v2rawdetection;
  bluefin = &store->bluefin;
  // s7kr_processedsidescan *processedsidescan = &store->processedsidescan;
  // int *current_ping = (int *)&mb_io_ptr->save14;
  pixel_size = (double *)&mb_io_ptr->saved1;
  swath_width = (double *)&mb_io_ptr->saved2;

  /* throw away multibeam data if the time stamp makes no sense */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->time_i[0] < 2004) {
    status = MB_FAILURE;
    *error = MB_ERROR_UNINTELLIGIBLE;
  }

  /* save fix if nav data */
  if (status == MB_SUCCESS && store->kind == MB_DATA_NAV1) {
    /* add latest fix */
    position = &(store->position);
    mb_navint_add(verbose, mbio_ptr, store->time_d, (double)(RTD * position->longitude), (double)(RTD * position->latitude),
                  error);
  }

  /* save nav and attitude if bluefin data */
  if (status == MB_SUCCESS && store->kind == MB_DATA_NAV2) {
    /* add latest fix */
    bluefin = &(store->bluefin);
    for (int i = 0; i < bluefin->number_frames; i++) {
      /* if (bluefin->nav[i].timedelay != 0)
      fprintf(stderr,"NAV TIME DIFF: %f %d\n", bluefin->nav[i].position_time,bluefin->nav[i].timedelay);*/
      mb_navint_add(verbose, mbio_ptr, (double)(bluefin->nav[i].position_time), (double)(RTD * bluefin->nav[i].longitude),
                    (double)(RTD * bluefin->nav[i].latitude), error);
      mb_attint_add(verbose, mbio_ptr, (double)(bluefin->nav[i].position_time), (double)(0.0),
                    (double)(RTD * bluefin->nav[i].roll), (double)(RTD * bluefin->nav[i].pitch), error);
      mb_hedint_add(verbose, mbio_ptr, (double)(bluefin->nav[i].position_time), (double)(RTD * bluefin->nav[i].yaw), error);
      if (mb_io_ptr->nsensordepth == 0 ||
          (bluefin->nav[i].depth != mb_io_ptr->sensordepth_sensordepth[mb_io_ptr->nsensordepth - 1])) {
        if (bluefin->nav[i].depth_time <= 0.0)
          bluefin->nav[i].depth_time = bluefin->nav[i].position_time;
        mb_depint_add(verbose, mbio_ptr, (double)(bluefin->nav[i].depth_time), (double)(bluefin->nav[i].depth), error);
      }
      if (bluefin->nav[i].altitude > 0.0 && bluefin->nav[i].altitude < 250.0 &&
          (i == 0 || bluefin->nav[i].altitude != bluefin->nav[i - 1].altitude)) {
        mb_altint_add(verbose, mbio_ptr, (double)(bluefin->nav[i].position_time), (double)(bluefin->nav[i].altitude),
                      error);
      }
    }
  }

  /* save nav and heading if navigation data */
  if (status == MB_SUCCESS && store->kind == MB_DATA_NAV3) {
    /* add latest fix */
    navigation = &(store->navigation);
    mb_navint_add(verbose, mbio_ptr, store->time_d, (double)(RTD * navigation->longitude),
                  (double)(RTD * navigation->latitude), error);
    mb_hedint_add(verbose, mbio_ptr, store->time_d, (double)(RTD * navigation->heading), error);
  }

  /* save attitude if attitude record */
  if (status == MB_SUCCESS && store->kind == MB_DATA_ATTITUDE && store->type == R7KRECID_Attitude) {
    /* get attitude structure */
    attitude = &(store->attitude);

    /* add latest attitude samples */
    for (int i = 0; i < attitude->n; i++) {
      mb_attint_add(verbose, mbio_ptr, (double)(store->time_d + 0.001 * ((double)attitude->delta_time[i])),
                    (double)(attitude->heave[i]), (double)(RTD * attitude->roll[i]), (double)(RTD * attitude->pitch[i]),
                    error);
      mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d + 0.001 * ((double)attitude->delta_time[i])),
                    (double)(RTD * attitude->heading[i]), error);
    }
  }

  /* else save attitude if rollpitchheave record */
  else if (status == MB_SUCCESS && store->kind == MB_DATA_ATTITUDE && store->type == R7KRECID_RollPitchHeave) {
    /* get attitude structure */
    rollpitchheave = &(store->rollpitchheave);

    /* add latest attitude samples */
    mb_attint_add(verbose, mbio_ptr, (double)(store->time_d), (double)(rollpitchheave->heave),
                  (double)(RTD * rollpitchheave->roll), (double)(RTD * rollpitchheave->pitch), error);
  }

  /* else save attitude if customattitude record */
  else if (status == MB_SUCCESS && store->kind == MB_DATA_ATTITUDE && store->type == R7KRECID_CustomAttitude) {
    /* get attitude structure */
    customattitude = &(store->customattitude);

    /* add latest attitude samples */
    for (int i = 0; i < customattitude->n; i++) {
      mb_attint_add(verbose, mbio_ptr, (double)(store->time_d + ((double)i) / ((double)customattitude->frequency)),
                    (double)(customattitude->heave[i]), (double)(RTD * customattitude->roll[i]),
                    (double)(RTD * customattitude->pitch[i]), error);
      mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d + ((double)i) / ((double)customattitude->frequency)),
                    (double)(RTD * customattitude->heading[i]), error);
    }
  }

  /* save heading if heading record */
  if (status == MB_SUCCESS && store->kind == MB_DATA_HEADING && store->type == R7KRECID_Heading) {
    /* get attitude structure */
    dheading = &(store->heading);

    /* add latest heading sample */
    mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d), (double)(RTD * dheading->heading), error);
  }

  /* save altitude if altitude record */
  if (status == MB_SUCCESS && store->kind == MB_DATA_ALTITUDE && store->type == R7KRECID_Altitude) {
    /* get attitude structure */
    altitude = &(store->altitude);

    /* add latest altitude sample */
    mb_altint_add(verbose, mbio_ptr, (double)(store->time_d), (double)(altitude->altitude), error);
  }

  /* save sensordepth if depth record */
  if (status == MB_SUCCESS && store->kind == MB_DATA_SENSORDEPTH && store->type == R7KRECID_Depth) {
    /* get attitude structure */
    depth = &(store->depth);

    /* add latest depth sample if sensor depth, not water depth */
    if (depth->descriptor == 0 && depth->depth != 0.0)
      mb_depint_add(verbose, mbio_ptr, (double)(store->time_d), (double)(depth->depth), error);
  }

#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
    fprintf(stderr, "Record returned: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

/* kluge to reset quality flags */
/*  if (status == MB_SUCCESS
    && store->kind == MB_DATA_DATA
    && bathymetry->header.Version < 5)
    {
    for (i=0;i<bathymetry->number_beams;i++)
        {
        if ((bathymetry->quality[i] & 15) < 2)
            {
            if (bathymetry->range[i] > 0.007)
                {
                bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 15;
                }
            else
                {
                bathymetry->quality[i] = (bathymetry->quality[i] & 240) + 3;
                }
            }
        }
    }
*/

#ifdef MBR_RESON7KR_DEBUG
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA)
    fprintf(stderr, "\nPING: store->read_bathymetry:%d ping_number:%d\n\n", store->read_bathymetry,
            v2rawdetection->ping_number);
#endif

  /* calculate bathymetry if only raw detects are available */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !store->read_bathymetry &&
      store->read_v2rawdetection) {
    bathymetry->header = v2rawdetection->header;
    bathymetry->header.RecordType = R7KRECID_7kBathymetricData;
    bathymetry->serial_number = v2rawdetection->serial_number;
    bathymetry->ping_number = v2rawdetection->ping_number;
    bathymetry->multi_ping = v2rawdetection->multi_ping;
        if (v2rawdetection->number_beams > 0) {
            bathymetry->number_beams = v2rawdetection->beam_descriptor[v2rawdetection->number_beams-1] + 1;
        }
        else {
            bathymetry->number_beams = 0;
        }
    bathymetry->layer_comp_flag = 0;
    bathymetry->sound_vel_flag = 0;
    if (volatilesettings->sound_velocity > 0.0) {
      bathymetry->sound_velocity = volatilesettings->sound_velocity;
    }
    else if (bluefin->environmental[0].sound_speed > 0.0) {
      bathymetry->sound_velocity = bluefin->environmental[0].sound_speed;
    }
    else {
      bathymetry->sound_velocity = 1500.0;
    }
    bathymetry->optionaldata = false;
    store->read_bathymetry = true;
  }

  /* else calculate bathymetry if only detects are available */
  else if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !store->read_bathymetry &&
           store->read_v2detection) {
    bathymetry->header = v2detection->header;
    bathymetry->header.RecordType = R7KRECID_7kBathymetricData;
    bathymetry->serial_number = v2detection->serial_number;
    bathymetry->ping_number = v2detection->ping_number;
    bathymetry->multi_ping = v2detection->multi_ping;
    bathymetry->number_beams = v2detection->number_beams;
    bathymetry->layer_comp_flag = 0;
    bathymetry->sound_vel_flag = 0;
    if (volatilesettings->sound_velocity > 0.0) {
      bathymetry->sound_velocity = volatilesettings->sound_velocity;
    }
    else if (bluefin->environmental[0].sound_speed > 0.0) {
      bathymetry->sound_velocity = bluefin->environmental[0].sound_speed;
    }
    else {
      bathymetry->sound_velocity = 1500.0;
    }
    bathymetry->optionaldata = false;
    store->read_bathymetry = true;
  }

  /* get optional values in bathymetry record if needed */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && !bathymetry->optionaldata) {
    /* get navigation */
    speed = 0.0;
    longitude = 0.0;
    latitude = 0.0;
    heading = 0.0;
    sonar_depth = 0.0;
    interp_status = mb_hedint_interp(verbose, mbio_ptr, store->time_d, &heading, &interp_error);
    if (interp_status == MB_SUCCESS)
      interp_status =
          mb_navint_interp(verbose, mbio_ptr, store->time_d, heading, speed, &longitude, &latitude, &speed, &interp_error);
    if (interp_status == MB_SUCCESS)
      interp_status = mb_depint_interp(verbose, mbio_ptr, store->time_d, &sonar_depth, &interp_error);

    /* if the optional data are not all available, this ping
        is not useful. Just use null values here and catch
        this condition with mb7kpreprocess */
    /* if (interp_status == MB_FAILURE)
        {
        longitude = 0.0;
        latitude = 0.0;
        heading = 0.0;
        sonar_depth = 0.0;
        } */

    /* get altitude */
    interp_status = mb_altint_interp(verbose, mbio_ptr, store->time_d, &sonar_altitude, &interp_error);
    if (interp_status == MB_FAILURE) {
      /* set altitude data to zero */
      sonar_altitude = 0.0;
    }

    /* get attitude */
    interp_status = mb_attint_interp(verbose, mbio_ptr, store->time_d, &heave, &roll, &pitch, &interp_error);
    if (interp_status == MB_FAILURE) {
      /* set nav & attitude data to zero */
      roll = 0.0;
      pitch = 0.0;
      heave = 0.0;
    }

    /* calculate the optional values in the bathymetry record */
    bathymetry->longitude = DTR * longitude;
    bathymetry->latitude = DTR * latitude;
    bathymetry->heading = DTR * heading;
    bathymetry->height_source = 1;
    bathymetry->tide = 0.0;
    bathymetry->roll = DTR * roll;
    bathymetry->pitch = DTR * pitch;
    bathymetry->heave = heave;
    bathymetry->vehicle_height = -sonar_depth;

    /* get ready to calculate bathymetry */
    double soundspeed;
    if (volatilesettings->sound_velocity > 0.0)
      soundspeed = volatilesettings->sound_velocity;
    else if (bluefin->environmental[0].sound_speed > 0.0)
      soundspeed = bluefin->environmental[0].sound_speed;
    else
      soundspeed = 1500.0;

    /* loop over detections as available - the 7k format has used several
       different records over the years, so there are several different
       cases that must be handled */

    /* case of v2rawdetection record */
    if (store->read_v2rawdetection) {
      /* initialize all of the beams */
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        bathymetry->quality[i] = 0;
        bathymetry->depth[i] = 0.0;
        bathymetry->acrosstrack[i] = 0.0;
        bathymetry->alongtrack[i] = 0.0;
        bathymetry->pointing_angle[i] = 0.0;
        bathymetry->azimuth_angle[i] = 0.0;
      }

      /* now loop over the detects */
      for (unsigned int j = 0; j < v2rawdetection->number_beams; j++) {
        const int i = v2rawdetection->beam_descriptor[j];
        bathymetry->range[i] = v2rawdetection->detection_point[j] / v2rawdetection->sampling_rate;
        bathymetry->quality[i] = v2rawdetection->quality[j];
        const double alpha = RTD * (bathymetry->pitch + v2rawdetection->tx_angle);
        const double beta = 90.0 - RTD * (v2rawdetection->rx_angle[j] - bathymetry->roll);
        mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
        const double rr = 0.5 * soundspeed * bathymetry->range[i];
        const double xx = rr * sin(DTR * theta);
        const double zz = rr * cos(DTR * theta);
        bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
        bathymetry->alongtrack[i] = xx * sin(DTR * phi);
        bathymetry->depth[i] = zz + sonar_depth - heave;
        bathymetry->pointing_angle[i] = DTR * theta;
        bathymetry->azimuth_angle[i] = DTR * phi;
      }
    }

    /* case of v2detection record with v2detectionsetup */
    else if (store->read_v2detection && store->read_v2detectionsetup) {
      /* now loop over the detects */
      for (unsigned int j = 0; j < v2detection->number_beams; j++) {
        const int i = v2detectionsetup->beam_descriptor[j];

        bathymetry->range[i] = v2detection->range[j];
        const double alpha = RTD * (v2detection->angle_y[j] + bathymetry->pitch + volatilesettings->steering_vertical);
        const double beta = 90.0 - RTD * (v2detection->angle_x[j] - bathymetry->roll);
        mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
        const double rr = 0.5 * soundspeed * bathymetry->range[i];
        const double xx = rr * sin(DTR * theta);
        const double zz = rr * cos(DTR * theta);
        bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
        bathymetry->alongtrack[i] = xx * sin(DTR * phi);
        bathymetry->depth[i] = zz + sonar_depth - heave;
        bathymetry->pointing_angle[i] = DTR * theta;
        bathymetry->azimuth_angle[i] = DTR * phi;
      }
    }

    /* case of v2detection record alone */
    else if (store->read_v2detection) {
      /* now loop over the detects */
      for (unsigned int j = 0; j < v2detection->number_beams; j++) {
        const int i = j;

        bathymetry->range[i] = v2detection->range[j];
        const double alpha = RTD * (v2detection->angle_y[j] + bathymetry->pitch + volatilesettings->steering_vertical);
        const double beta = 90.0 - RTD * (v2detection->angle_x[j] - bathymetry->roll);
        mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
        const double rr = 0.5 * soundspeed * bathymetry->range[i];
        const double xx = rr * sin(DTR * theta);
        const double zz = rr * cos(DTR * theta);
        bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
        bathymetry->alongtrack[i] = xx * sin(DTR * phi);
        bathymetry->depth[i] = zz + sonar_depth - heave;
        bathymetry->pointing_angle[i] = DTR * theta;
        bathymetry->azimuth_angle[i] = DTR * phi;
      }
    }

    /* else default case of beamgeometry record */
    else {
      /* loop over all beams */
            bathymetry->number_beams = beamgeometry->number_beams;
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        if ((bathymetry->quality[i] & 15) > 0) {
          const double alpha = RTD * (beamgeometry->angle_alongtrack[i] + bathymetry->pitch + volatilesettings->steering_vertical);
          const double beta = 90.0 - RTD * (beamgeometry->angle_acrosstrack[i] - bathymetry->roll);
          mb_rollpitch_to_takeoff(verbose, alpha, beta, &theta, &phi, error);
          const double rr = 0.5 * soundspeed * bathymetry->range[i];
          const double xx = rr * sin(DTR * theta);
          const double zz = rr * cos(DTR * theta);
          bathymetry->acrosstrack[i] = xx * cos(DTR * phi);
          bathymetry->alongtrack[i] = xx * sin(DTR * phi);
          bathymetry->depth[i] = zz + sonar_depth - heave;
          bathymetry->pointing_angle[i] = DTR * theta;
          bathymetry->azimuth_angle[i] = DTR * phi;
        }
        else {
          bathymetry->quality[i] = 0;
          bathymetry->depth[i] = 0.0;
          bathymetry->acrosstrack[i] = 0.0;
          bathymetry->alongtrack[i] = 0.0;
          bathymetry->pointing_angle[i] = 0.0;
          bathymetry->azimuth_angle[i] = 0.0;
        }
      }
    }

    /* set flag */
    bathymetry->optionaldata = true;
    bathymetry->header.OffsetToOptionalData =
        MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kBathymetricData + bathymetry->number_beams * 9;

    /* mbsys_reson7k_print_bathymetry(verbose, bathymetry, error);*/
  }

  /* generate processed sidescan if needed */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->read_bathymetry &&
      !store->read_processedsidescan) {
    /* set source of processed sidescan to be best available data */
    if (store->read_calibratedsnippet)
      ss_source = R7KRECID_7kCalibratedSnippetData;
    else if (store->read_v2snippet)
      ss_source = R7KRECID_7kV2SnippetData;
    else if (store->read_beam)
      ss_source = R7KRECID_7kBeamData;
    else if (store->read_backscatter)
      ss_source = R7KRECID_7kBackscatterImageData;
    else
      ss_source = 0;
    status = mbsys_reson7k_makess_source(verbose, mbio_ptr, store_ptr, ss_source, false, pixel_size, false, swath_width, true, error);
  }

  /* set error and kind in mb_io_ptr */
  mb_io_ptr->new_error = *error;
  mb_io_ptr->new_kind = store->kind;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_header(int verbose, char *buffer, unsigned int *index, s7k_header *header, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       header:     %p\n", (void *)header);
  }

  /* set some important values */
  header->Version = 5;
  header->Offset = 60;
  header->SyncPattern = 0x0000ffff;
  header->Reserved = 0;
  for (int i = 0; i < 8; i++) {
    header->PreviousRecord[i] = -1;
    header->NextRecord[i] = -1;
  }
  header->Flags = 0;
  header->Reserved2 = 0;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_header(verbose, header, error);

  /* insert the header */
  mb_put_binary_short(true, header->Version, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, header->Offset, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, header->SyncPattern, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->Size, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->OffsetToOptionalData, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->OptionalDataIdentifier, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, header->s7kTime.Year, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, header->s7kTime.Day, &buffer[*index]);
  *index += 2;
  mb_put_binary_float(true, header->s7kTime.Seconds, &buffer[*index]);
  *index += 4;
  buffer[*index] = header->s7kTime.Hours;
  (*index)++;
  buffer[*index] = header->s7kTime.Minutes;
  (*index)++;
  mb_put_binary_short(true, header->Reserved, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, header->RecordType, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->DeviceId, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, header->Reserved2, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, header->SystemEnumerator, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, header->RecordNumber, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, header->Flags, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, header->Reserved3, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, header->Reserved4, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->FragmentedTotal, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, header->FragmentNumber, &buffer[*index]);
  *index += 4;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_reference(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_reference *reference;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  reference = &(store->reference);
  header = &(reference->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_reference(verbose, reference, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_ReferencePoint;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    mb_put_binary_float(true, reference->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, reference->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, reference->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, reference->water_z, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensoruncal(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_sensoruncal *sensoruncal;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  sensoruncal = &(store->sensoruncal);
  header = &(sensoruncal->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_sensoruncal(verbose, sensoruncal, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_UncalibratedSensorOffset;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, sensoruncal->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensoruncal->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensoruncal->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensoruncal->offset_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensoruncal->offset_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensoruncal->offset_yaw, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_sensorcal(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_sensorcal *sensorcal;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  sensorcal = &(store->sensorcal);
  header = &(sensorcal->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_sensorcal(verbose, sensorcal, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CalibratedSensorOffset;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, sensorcal->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensorcal->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensorcal->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensorcal->offset_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensorcal->offset_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sensorcal->offset_yaw, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_position(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_position *position;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  position = &(store->position);
  header = &(position->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_position(verbose, position, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Position;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, position->datum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, position->latency, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, position->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, position->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, position->height, &buffer[index]);
    index += 8;
    buffer[index] = position->type;
    index++;
    buffer[index] = position->utm_zone;
    index++;
    buffer[index] = position->quality;
    index++;
    buffer[index] = position->method;
    index++;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_customattitude(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_customattitude *customattitude;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  customattitude = &(store->customattitude);
  header = &(customattitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_customattitude(verbose, customattitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CustomAttitude;
  if (customattitude->bitfield & 1)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 2)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 4)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 8)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 16)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 32)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 64)
    *size += customattitude->n * sizeof(float);
  if (customattitude->bitfield & 128)
    *size += customattitude->n * sizeof(float);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    customattitude->bitfield = (mb_u_char)buffer[index];
    index++;
    customattitude->reserved = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(true, customattitude->n, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, customattitude->frequency, &buffer[index]);
    index += 4;

    if (customattitude->bitfield & 1)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->pitch[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 2)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->roll[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 4)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->heading[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 8)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->heave[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 16)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->pitchrate[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 32)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->rollrate[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 64)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->headingrate[i], &buffer[index]);
        index += 4;
      }
    if (customattitude->bitfield & 128)
      for (int i = 0; i < customattitude->n; i++) {
        mb_put_binary_float(true, customattitude->heaverate[i], &buffer[index]);
        index += 4;
      }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_tide(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_tide *tide;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  tide = &(store->tide);
  header = &(tide->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_tide(verbose, tide, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Tide;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, tide->tide, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, tide->source, &buffer[index]);
    index += 2;
    buffer[index] = tide->flags;
    index++;
    mb_get_binary_short(true, &buffer[index], &(tide->gauge));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(tide->datum));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(tide->latency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(tide->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(tide->longitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(tide->height));
    index += 8;
    buffer[index] = tide->type;
    index++;
    buffer[index] = tide->utm_zone;
    index++;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_altitude(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_altitude *altitude;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  altitude = &(store->altitude);
  header = &(altitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_altitude(verbose, altitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Altitude;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, altitude->altitude, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_motion(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_motion *motion;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  motion = &(store->motion);
  header = &(motion->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_motion(verbose, motion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_MotionOverGround;
  if (motion->bitfield & 1)
    *size += 3 * motion->n * sizeof(float);
  if (motion->bitfield & 2)
    *size += 3 * motion->n * sizeof(float);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    motion->bitfield = (mb_u_char)buffer[index];
    index++;
    motion->reserved = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(true, motion->n, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, motion->frequency, &buffer[index]);
    index += 4;

    if (motion->bitfield & 1) {
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->x[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->y[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->z[i], &buffer[index]);
        index += 4;
      }
    }
    if (motion->bitfield & 2) {
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->xa[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->ya[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < motion->n; i++) {
        mb_put_binary_float(true, motion->za[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_depth(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_depth *depth;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  depth = &(store->depth);
  header = &(depth->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_depth(verbose, depth, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Depth;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    depth->descriptor = (mb_u_char)buffer[index];
    index++;
    depth->correction = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(true, depth->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, depth->depth, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_svp(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_svp *svp;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  svp = &(store->svp);
  header = &(svp->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_svp(verbose, svp, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SoundVelocityProfile;
  *size += R7KRDTSIZE_SoundVelocityProfile * svp->n;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    svp->position_flag = (mb_u_char)buffer[index];
    index++;
    svp->reserved1 = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(true, svp->reserved2, &buffer[index]);
    index += 2;
    mb_put_binary_double(true, svp->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, svp->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, svp->n, &buffer[index]);
    index += 4;

    for (unsigned int i = 0; i < svp->n; i++) {
      mb_put_binary_float(true, svp->depth[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svp->sound_velocity[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_ctd(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_ctd *ctd;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  ctd = &(store->ctd);
  header = &(ctd->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_ctd(verbose, ctd, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CTD;
  *size += ctd->n * R7KRDTSIZE_CTD;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, ctd->frequency, &buffer[index]);
    index += 4;
    buffer[index] = ctd->velocity_source_flag;
    index++;
    buffer[index] = ctd->velocity_algorithm;
    index++;
    buffer[index] = ctd->conductivity_flag;
    index++;
    buffer[index] = ctd->pressure_flag;
    index++;
    buffer[index] = ctd->position_flag;
    index++;
    buffer[index] = ctd->validity;
    index++;
    mb_put_binary_short(true, ctd->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_double(true, ctd->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, ctd->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, ctd->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, ctd->n, &buffer[index]);
    index += 4;

    for (unsigned int i = 0; i < ctd->n; i++) {
      mb_put_binary_float(true, ctd->conductivity_salinity[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, ctd->temperature[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, ctd->pressure_depth[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, ctd->sound_velocity[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, ctd->absorption[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_geodesy(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_geodesy *geodesy;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  geodesy = &(store->geodesy);
  header = &(geodesy->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_geodesy(verbose, geodesy, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Geodesy;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      geodesy->spheroid[i] = (mb_u_char)buffer[index];
      index++;
    }
    mb_put_binary_double(true, geodesy->semimajoraxis, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->flattening, &buffer[index]);
    index += 8;
    for (int i = 0; i < 16; i++) {
      geodesy->reserved1[i] = (mb_u_char)buffer[index];
      index++;
    }
    for (int i = 0; i < 32; i++) {
      geodesy->datum[i] = (mb_u_char)buffer[index];
      index++;
    }
    mb_put_binary_int(true, geodesy->calculation_method, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, geodesy->number_parameters, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, geodesy->dx, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->dy, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->dz, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->rx, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->ry, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->rz, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->scale, &buffer[index]);
    index += 8;
    for (int i = 0; i < 35; i++) {
      geodesy->reserved2[i] = (mb_u_char)buffer[index];
      index++;
    }
    for (int i = 0; i < 32; i++) {
      geodesy->grid_name[i] = (mb_u_char)buffer[index];
      index++;
    }
    geodesy->distance_units = (mb_u_char)buffer[index];
    index++;
    geodesy->angular_units = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_double(true, geodesy->latitude_origin, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->central_meriidan, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->false_easting, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->false_northing, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, geodesy->central_scale_factor, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, geodesy->custum_identifier, &buffer[index]);
    index += 4;
    for (int i = 0; i < 50; i++) {
      geodesy->reserved3[i] = (mb_u_char)buffer[index];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_rollpitchheave(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_rollpitchheave *rollpitchheave;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  rollpitchheave = &(store->rollpitchheave);
  header = &(rollpitchheave->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_rollpitchheave(verbose, rollpitchheave, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RollPitchHeave;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, rollpitchheave->roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, rollpitchheave->pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, rollpitchheave->heave, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_heading(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_heading *heading;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  heading = &(store->heading);
  header = &(heading->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_heading(verbose, heading, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Heading;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, heading->heading, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_surveyline(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_surveyline *surveyline;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  surveyline = &(store->surveyline);
  header = &(surveyline->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_surveyline(verbose, surveyline, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SurveyLine;
  *size += surveyline->n * R7KRDTSIZE_SurveyLine;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_short(true, surveyline->n, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, surveyline->type, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, surveyline->turnradius, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = (char)surveyline->name[i];
      index++;
    }
    for (int i = 0; i < surveyline->n; i++) {
      mb_put_binary_double(true, surveyline->latitude[i], &buffer[index]);
      index += 8;
      mb_put_binary_double(true, surveyline->longitude[i], &buffer[index]);
      index += 8;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_navigation(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_navigation *navigation;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  navigation = &(store->navigation);
  header = &(navigation->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_navigation(verbose, navigation, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Navigation;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    buffer[index] = (mb_u_char)navigation->vertical_reference;
    index++;
    mb_put_binary_double(true, navigation->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, navigation->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, navigation->position_accuracy, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, navigation->height, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, navigation->height_accuracy, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, navigation->speed, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, navigation->course, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, navigation->heading, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_attitude(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_attitude *attitude;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  attitude = &(store->attitude);
  header = &(attitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_attitude(verbose, attitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Attitude;
  *size += attitude->n * R7KRDTSIZE_Attitude;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    buffer[index] = attitude->n;
    index++;
    for (int i = 0; i < attitude->n; i++) {
      mb_put_binary_short(true, attitude->delta_time[i], &buffer[index]);
      index += 2;
      mb_put_binary_float(true, attitude->roll[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, attitude->pitch[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, attitude->heave[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, attitude->heading[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_rec1022(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_rec1022 *rec1022;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  rec1022 = &(store->rec1022);
  header = &(rec1022->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_rec1022(verbose, rec1022, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Rec1022;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < R7KHDRSIZE_Rec1022; i++) {
      buffer[index] = rec1022->data[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwchannel(int verbose, int data_format, char *buffer, unsigned int *index, s7k_fsdwchannel *fsdwchannel,
                                int *error) {
  int status = MB_SUCCESS;
  short *shortptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       data_format:%d\n", data_format);
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       fsdwchannel:%p\n", (void *)fsdwchannel);
  }

  /* print out the data to be output */
  /*mbsys_reson7k_print_fsdwchannel(verbose, data_format, fsdwchannel, error);*/

  /* extract the channel header */
  buffer[*index] = fsdwchannel->number;
  (*index)++;
  buffer[*index] = fsdwchannel->type;
  (*index)++;
  buffer[*index] = fsdwchannel->data_type;
  (*index)++;
  buffer[*index] = fsdwchannel->polarity;
  (*index)++;
  buffer[*index] = fsdwchannel->bytespersample;
  (*index)++;
  for (int i = 0; i < 3; i++) {
    buffer[*index] = fsdwchannel->reserved1[i];
    (*index)++;
  }
  mb_put_binary_int(true, fsdwchannel->number_samples, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwchannel->start_time, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwchannel->sample_interval, &buffer[*index]);
  *index += 4;
  mb_put_binary_float(true, fsdwchannel->range, &buffer[*index]);
  *index += 4;
  mb_put_binary_float(true, fsdwchannel->voltage, &buffer[*index]);
  *index += 4;
  for (int i = 0; i < 16; i++) {
    buffer[*index] = fsdwchannel->name[i];
    (*index)++;
  }
  for (int i = 0; i < 20; i++) {
    buffer[*index] = fsdwchannel->reserved2[i];
    (*index)++;
  }

  /* copy over the data */
  if (status == MB_SUCCESS) {
    if (fsdwchannel->bytespersample == 1) {
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        buffer[*index] = fsdwchannel->data[i];
        (*index)++;
      }
    }
    else if (fsdwchannel->bytespersample == 2) {
      shortptr = (short *)fsdwchannel->data;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        /*srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_put_binary_short(true, shortptr[i], &buffer[*index]);
        *index += 2;
        /*ssptr = (short *) &(shortptr[i]);
        usptr = (unsigned short *) &(shortptr[i]);
        fprintf(stderr,"sample:%5d   raw:%6d %6d   swapped:%6d %6d\n",
        i,*srptr,*urptr,*ssptr,*usptr);*/
      }
    }
    else if (fsdwchannel->bytespersample == 4) {
      shortptr = (short *)fsdwchannel->data;
      for (unsigned int i = 0; i < fsdwchannel->number_samples; i++) {
        /*srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_put_binary_short(true, shortptr[2 * i], &buffer[*index]);
        *index += 2;
        /*ssptr = (short *) &(shortptr[2*i]);
        usptr = (unsigned short *) &(shortptr[2*i]);
        fprintf(stderr,"sample:%5d   IMAGINARY: raw:%6d %6d   swapped:%6d %6d",
        i,*srptr,*urptr,*ssptr,*usptr);
        srptr = (short *) &(buffer[*index]);
        urptr = (unsigned short *) &(buffer[*index]);*/
        mb_put_binary_short(true, shortptr[2 * i + 1], &buffer[*index]);
        *index += 2;
        /*ssptr = (short *) &(shortptr[2*i+1]);
        usptr = (unsigned short *) &(shortptr[2*i+1]);
        fprintf(stderr,"    REAL: raw:%6d %6d   swapped:%6d %6d\n",
        *srptr,*urptr,*ssptr,*usptr);*/
      }
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwssheader(int verbose, char *buffer, unsigned int *index, s7k_fsdwssheader *fsdwssheader, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:          %d\n", *index);
    fprintf(stderr, "dbg2       fsdwssheader:   %p\n", (void *)fsdwssheader);
  }

  /* print out the data to be output */
  /*mbsys_reson7k_print_fsdwssheader(verbose, fsdwssheader, error);*/

  /* extract the Edgetech sidescan header */
  mb_put_binary_short(true, fsdwssheader->subsystem, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->channelNum, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwssheader->pingNum, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwssheader->packetNum, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->trigSource, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwssheader->samples, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwssheader->sampleInterval, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwssheader->startDepth, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwssheader->weightingFactor, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->ADCGain, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->ADCMax, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->rangeSetting, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->pulseID, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->markNumber, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->dataFormat, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->reserved, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwssheader->millisecondsToday, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwssheader->year, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->day, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->hour, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->minute, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->second, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->heading, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->pitch, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->roll, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->heave, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwssheader->yaw, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwssheader->depth, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwssheader->temperature, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 2; i++) {
    buffer[*index] = fsdwssheader->reserved2[i];
    (*index)++;
  }
  mb_put_binary_int(true, fsdwssheader->longitude, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwssheader->latitude, &buffer[*index]);
  *index += 4;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsegyheader(int verbose, char *buffer, unsigned int *index, s7k_fsdwsegyheader *fsdwsegyheader, int *error) {
  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       index:          %d\n", *index);
    fprintf(stderr, "dbg2       fsdwsegyheader: %p\n", (void *)fsdwsegyheader);
  }

  /* print out the data to be output */
  /*mbsys_reson7k_print_fsdwsegyheader(verbose, fsdwsegyheader, error);*/

  /* extract the Edgetech segy header */
  mb_put_binary_int(true, fsdwsegyheader->sequenceNumber, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->startDepth, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->pingNum, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->channelNum, &buffer[*index]);
  *index += 4;
  for (int i = 0; i < 6; i++) {
    mb_put_binary_short(true, fsdwsegyheader->unused1[i], &buffer[*index]);
    *index += 2;
  }
  mb_put_binary_short(true, fsdwsegyheader->traceIDCode, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 2; i++) {
    mb_put_binary_short(true, fsdwsegyheader->unused2[i], &buffer[*index]);
    *index += 2;
  }
  mb_put_binary_short(true, fsdwsegyheader->dataFormat, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEAantennaeR, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEAantennaeO, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 32; i++) {
    buffer[*index] = fsdwsegyheader->RS232[i];
    (*index)++;
  }
  mb_put_binary_int(true, fsdwsegyheader->sourceCoordX, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->sourceCoordY, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->groupCoordX, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(true, fsdwsegyheader->groupCoordY, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwsegyheader->coordUnits, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 24; i++) {
    buffer[*index] = fsdwsegyheader->annotation[i];
    (*index)++;
  }
  mb_put_binary_short(true, fsdwsegyheader->samples, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwsegyheader->sampleInterval, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwsegyheader->ADCGain, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->pulsePower, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->correlated, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->startFreq, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->endFreq, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->sweepLength, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 4; i++) {
    mb_put_binary_short(true, fsdwsegyheader->unused7[i], &buffer[*index]);
    *index += 2;
  }
  mb_put_binary_short(true, fsdwsegyheader->aliasFreq, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->pulseID, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 6; i++) {
    mb_put_binary_short(true, fsdwsegyheader->unused8[i], &buffer[*index]);
    *index += 2;
  }
  mb_put_binary_short(true, fsdwsegyheader->year, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->day, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->hour, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->minute, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->second, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->timeBasis, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->weightingFactor, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->unused9, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->heading, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->pitch, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->roll, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->temperature, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->heaveCompensation, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->trigSource, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->markNumber, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEAHour, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEAMinutes, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEASeconds, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEACourse, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEASpeed, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEADay, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->NMEAYear, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(true, fsdwsegyheader->millisecondsToday, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwsegyheader->ADCMax, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->calConst, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->vehicleID, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 6; i++) {
    buffer[*index] = fsdwsegyheader->softwareVersion[i];
    (*index)++;
  }
  mb_put_binary_int(true, fsdwsegyheader->sphericalCorrection, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(true, fsdwsegyheader->packetNum, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->ADCDecimation, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(true, fsdwsegyheader->decimation, &buffer[*index]);
  *index += 2;
  for (int i = 0; i < 7; i++) {
    mb_put_binary_short(true, fsdwsegyheader->unuseda[i], &buffer[*index]);
    *index += 2;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       index:      %d\n", *index);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsslo(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_fsdwchannel *fsdwchannel;
  s7k_fsdwssheader *fsdwssheader;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_fsdwss *fsdwsslo = &(store->fsdwsslo);
  s7k_header *header = &(fsdwsslo->header);
  // s7kr_bathymetry *bathymetry = &(store->bathymetry);
  // s7kr_bluefin *bluefin = &(store->bluefin);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwss(verbose, fsdwsslo, error);
#ifdef MBR_RESON7KR_DEBUG2
  for (int i = 0; i < fsdwsslo->number_channels; i++) {
    fsdwchannel = &(fsdwsslo->channel[i]);
    fsdwssheader = &(fsdwsslo->ssheader[i]);
    fprintf(stderr,
            "SSLO: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d "
            "%d chan:%d %d sampint:%d %d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], fsdwssheader->year, fsdwssheader->day, fsdwssheader->hour, fsdwssheader->minute,
            fsdwssheader->second, fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
            fsdwsslo->ping_number, fsdwssheader->pingNum, fsdwchannel->number, fsdwssheader->channelNum,
            fsdwchannel->sample_interval, fsdwssheader->sampleInterval);
  }
#endif

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FSDWsidescan;
  for (int i = 0; i < fsdwsslo->number_channels; i++) {
    *size += R7KHDRSIZE_FSDWchannelinfo;
    *size += R7KHDRSIZE_FSDWssheader;
    fsdwchannel = &(fsdwsslo->channel[i]);
    *size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, fsdwsslo->msec_timestamp, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsslo->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsslo->number_channels, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsslo->total_bytes, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsslo->data_format, &buffer[index]);
    index += 4;
    index += 12;
    for (int i = 0; i < 2; i++) {
      fsdwchannel = &(fsdwsslo->channel[i]);
      mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsslo->data_format, buffer, &index, fsdwchannel, error);
    }
    for (int i = 0; i < 2; i++) {
      fsdwssheader = &(fsdwsslo->ssheader[i]);
      mbr_reson7kr_wr_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsshi(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fsdwss *fsdwsshi;
  s7k_fsdwchannel *fsdwchannel;
  s7k_fsdwssheader *fsdwssheader;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  fsdwsshi = &(store->fsdwsshi);
  header = &(fsdwsshi->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwss(verbose, fsdwsshi, error);
#ifdef MBR_RESON7KR_DEBUG2
  for (int i = 0; i < fsdwsshi->number_channels; i++) {
    fsdwchannel = &(fsdwsshi->channel[i]);
    fsdwssheader = &(fsdwsshi->ssheader[i]);
    fprintf(stderr,
            "SSHI: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d "
            "%d chan:%d %d sampint:%d %d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], fsdwssheader->year, fsdwssheader->day, fsdwssheader->hour, fsdwssheader->minute,
            fsdwssheader->second, fsdwssheader->millisecondsToday - 1000 * (int)(0.001 * fsdwssheader->millisecondsToday),
            fsdwsshi->ping_number, fsdwssheader->pingNum, fsdwchannel->number, fsdwssheader->channelNum,
            fsdwchannel->sample_interval, fsdwssheader->sampleInterval);
  }
#endif

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FSDWsidescan;
  for (int i = 0; i < fsdwsshi->number_channels; i++) {
    *size += R7KHDRSIZE_FSDWchannelinfo;
    *size += R7KHDRSIZE_FSDWssheader;
    fsdwchannel = &(fsdwsshi->channel[i]);
    *size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, fsdwsshi->msec_timestamp, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsshi->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsshi->number_channels, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsshi->total_bytes, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsshi->data_format, &buffer[index]);
    index += 4;
    index += 12;
    for (int i = 0; i < 2; i++) {
      fsdwchannel = &(fsdwsshi->channel[i]);
      mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsshi->data_format, buffer, &index, fsdwchannel, error);
    }
    for (int i = 0; i < 2; i++) {
      fsdwssheader = &(fsdwsshi->ssheader[i]);
      mbr_reson7kr_wr_fsdwssheader(verbose, buffer, &index, fsdwssheader, error);
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fsdwsb(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_fsdwsb *fsdwsb;
  s7k_fsdwchannel *fsdwchannel;
  s7k_fsdwsegyheader *fsdwsegyheader;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  fsdwsb = &(store->fsdwsb);
  header = &(fsdwsb->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fsdwsb(verbose, fsdwsb, error);
#ifdef MBR_RESON7KR_DEBUG2
  for (int i = 0; i < fsdwsb->number_channels; i++) {
    fsdwchannel = &(fsdwsb->channel);
    fsdwsegyheader = &(fsdwsb->segyheader);
    fprintf(stderr,
            "SBP:  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) FSDWtime(%4.4d-%3.3d %2.2d:%2.2d:%2.2d.%3.3d) ping:%d "
            "%d chan:%d %d sampint:%d %d\n",
            store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
            store->time_i[6], fsdwsegyheader->year, fsdwsegyheader->day, fsdwsegyheader->hour, fsdwsegyheader->minute,
            fsdwsegyheader->second,
            fsdwsegyheader->millisecondsToday - 1000 * (int)(0.001 * fsdwsegyheader->millisecondsToday), fsdwsb->ping_number,
            fsdwsegyheader->pingNum, fsdwchannel->number, fsdwsegyheader->channelNum, fsdwchannel->sample_interval,
            fsdwsegyheader->sampleInterval);
  }
#endif

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FSDWsubbottom;

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FSDWsubbottom;
  for (int i = 0; i < fsdwsb->number_channels; i++) {
    *size += R7KHDRSIZE_FSDWchannelinfo;
    *size += R7KHDRSIZE_FSDWsbheader;
    fsdwchannel = &(fsdwsb->channel);
    *size += fsdwchannel->bytespersample * fsdwchannel->number_samples;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, fsdwsb->msec_timestamp, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsb->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsb->number_channels, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsb->total_bytes, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fsdwsb->data_format, &buffer[index]);
    index += 4;
    index += 12;
    fsdwchannel = &(fsdwsb->channel);
    mbr_reson7kr_wr_fsdwchannel(verbose, fsdwsb->data_format, buffer, &index, fsdwchannel, error);
    fsdwsegyheader = &(fsdwsb->segyheader);
    mbr_reson7kr_wr_fsdwsegyheader(verbose, buffer, &index, fsdwsegyheader, error);

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_bluefin(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_bluefin *bluefin;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  bluefin = &(store->bluefin);
  header = &(bluefin->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_bluefin(verbose, bluefin, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_BluefinDataFrame;
  *size += bluefin->number_frames * bluefin->frame_size;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, bluefin->msec_timestamp, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, bluefin->number_frames, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, bluefin->frame_size, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, bluefin->data_format, &buffer[index]);
    index += 4;
    for (int i = 0; i < 16; i++) {
      buffer[index] = bluefin->reserved[i];
      index++;
    }
    if (bluefin->data_format == R7KRECID_BluefinNav) {
      for (int i = 0; i < bluefin->number_frames; i++) {
        mb_put_binary_int(true, bluefin->nav[i].packet_size, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->nav[i].version, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->nav[i].offset, &buffer[index]);
        index += 2;
        mb_put_binary_int(true, bluefin->nav[i].data_type, &buffer[index]);
        index += 4;
        mb_put_binary_int(true, bluefin->nav[i].data_size, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->nav[i].s7kTime.Year, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->nav[i].s7kTime.Day, &buffer[index]);
        index += 2;
        mb_put_binary_float(true, bluefin->nav[i].s7kTime.Seconds, &buffer[index]);
        index += 4;
        buffer[index] = bluefin->nav[i].s7kTime.Hours;
        (index)++;
        buffer[index] = bluefin->nav[i].s7kTime.Minutes;
        (index)++;
        mb_put_binary_int(true, bluefin->nav[i].checksum, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->nav[i].timedelay, &buffer[index]);
        index += 2;
        mb_put_binary_int(true, bluefin->nav[i].quality, &buffer[index]);
        index += 4;
        mb_put_binary_double(true, bluefin->nav[i].latitude, &buffer[index]);
        index += 8;
        mb_put_binary_double(true, bluefin->nav[i].longitude, &buffer[index]);
        index += 8;
        mb_put_binary_float(true, bluefin->nav[i].speed, &buffer[index]);
        index += 4;
        mb_put_binary_double(true, bluefin->nav[i].depth, &buffer[index]);
        index += 8;
        mb_put_binary_double(true, bluefin->nav[i].altitude, &buffer[index]);
        index += 8;
        mb_put_binary_float(true, bluefin->nav[i].roll, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].pitch, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].yaw, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].northing_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].easting_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].depth_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].altitude_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].roll_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].pitch_rate, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->nav[i].yaw_rate, &buffer[index]);
        index += 4;
        mb_put_binary_double(true, bluefin->nav[i].position_time, &buffer[index]);
        index += 8;
        mb_put_binary_double(true, bluefin->nav[i].depth_time, &buffer[index]);
        index += 8;
      }
    }
    else if (bluefin->data_format == R7KRECID_BluefinEnvironmental) {
      for (int i = 0; i < bluefin->number_frames; i++) {
        mb_put_binary_int(true, bluefin->environmental[i].packet_size, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->environmental[i].version, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].offset, &buffer[index]);
        index += 2;
        mb_put_binary_int(true, bluefin->environmental[i].data_type, &buffer[index]);
        index += 4;
        mb_put_binary_int(true, bluefin->environmental[i].data_size, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->environmental[i].s7kTime.Year, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].s7kTime.Day, &buffer[index]);
        index += 2;
        mb_put_binary_float(true, bluefin->environmental[i].s7kTime.Seconds, &buffer[index]);
        index += 4;
        buffer[index] = bluefin->environmental[i].s7kTime.Hours;
        (index)++;
        buffer[index] = bluefin->environmental[i].s7kTime.Minutes;
        (index)++;
        mb_put_binary_int(true, bluefin->environmental[i].checksum, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->environmental[i].reserved1, &buffer[index]);
        index += 2;
        mb_put_binary_int(true, bluefin->environmental[i].quality, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].sound_speed, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].conductivity, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].temperature, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].pressure, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].salinity, &buffer[index]);
        index += 4;
        mb_put_binary_double(true, bluefin->environmental[i].ctd_time, &buffer[index]);
        index += 8;
        mb_put_binary_double(true, bluefin->environmental[i].temperature_time, &buffer[index]);
        index += 8;
        mb_put_binary_double(true, bluefin->environmental[i].surface_pressure, &buffer[index]);
        index += 8;
        mb_put_binary_int(true, bluefin->environmental[i].temperature_counts, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].conductivity_frequency, &buffer[index]);
        index += 4;
        mb_put_binary_int(true, bluefin->environmental[i].pressure_counts, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bluefin->environmental[i].pressure_comp_voltage, &buffer[index]);
        index += 4;
        mb_put_binary_int(true, bluefin->environmental[i].sensor_time_sec, &buffer[index]);
        index += 4;
        mb_put_binary_int(true, bluefin->environmental[i].sensor_time_nsec, &buffer[index]);
        index += 4;
        mb_put_binary_short(true, bluefin->environmental[i].sensor1, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor2, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor3, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor4, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor5, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor6, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor7, &buffer[index]);
        index += 2;
        mb_put_binary_short(true, bluefin->environmental[i].sensor8, &buffer[index]);
        index += 2;
        for (int j = 0; j < 8; j++) {
          buffer[index] = bluefin->environmental[i].reserved2[j];
          index++;
        }
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_processedsidescan(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_processedsidescan *processedsidescan;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  processedsidescan = &(store->processedsidescan);
  header = &(processedsidescan->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_ProcessedSidescan;
  *size += processedsidescan->number_pixels * 8;
  header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_processedsidescan(verbose, processedsidescan, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, processedsidescan->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, processedsidescan->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, processedsidescan->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, processedsidescan->recordversion, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, processedsidescan->ss_source, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, processedsidescan->number_pixels, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, processedsidescan->ss_type, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, processedsidescan->pixelwidth, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, processedsidescan->sensordepth, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, processedsidescan->altitude, &buffer[index]);
    index += 8;

    /* insert the data */
    for (unsigned int i = 0; i < processedsidescan->number_pixels; i++) {
      mb_put_binary_float(true, processedsidescan->sidescan[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < processedsidescan->number_pixels; i++) {
      mb_put_binary_float(true, processedsidescan->alongtrack[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_volatilesonarsettings(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size,
                                          int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_volatilesettings *volatilesettings;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  volatilesettings = &(store->volatilesettings);
  header = &(volatilesettings->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_volatilesettings(verbose, volatilesettings, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kVolatileSonarSettings;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, volatilesettings->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, volatilesettings->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, volatilesettings->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, volatilesettings->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->receiver_bandwidth, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->pulse_width, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->pulse_type, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->pulse_envelope, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->pulse_envelope_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->pulse_reserved, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->max_ping_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->ping_period, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->range_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->power_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->gain_selection, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->projector_magic_no, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->steering_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->steering_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->beamwidth_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->beamwidth_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->focal_point, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->projector_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->projector_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->transmit_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->hydrophone_magic_no, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->receive_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->receive_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, volatilesettings->receive_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->receive_width, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->range_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->range_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->depth_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->depth_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->absorption, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, volatilesettings->spreading, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, volatilesettings->reserved, &buffer[index]);
    index += 2;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_configuration(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_configuration *configuration;
  s7k_device *device;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  configuration = &(store->configuration);
  header = &(configuration->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_configuration(verbose, configuration, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kConfiguration;
  for (unsigned int i = 0; i < configuration->number_devices; i++) {
    *size += 80;
    device = &(configuration->device[i]);
    *size += device->info_length;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, configuration->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, configuration->number_devices, &buffer[index]);
    index += 4;

    /* extract the data for each device */
    for (unsigned int i = 0; i < configuration->number_devices; i++) {
      device = &(configuration->device[i]);
      mb_put_binary_int(true, device->magic_number, &buffer[index]);
      index += 4;
      for (int j = 0; j < 64; j++) {
        buffer[index] = device->description[j];
        index++;
      }
      mb_put_binary_long(true, device->serial_number, &buffer[index]);
      index += 8;
      mb_put_binary_int(true, device->info_length, &buffer[index]);
      index += 4;

      for (unsigned int j = 0; j < device->info_length; j++) {
        buffer[index] = device->info[j];
        index++;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_matchfilter(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_matchfilter *matchfilter;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  matchfilter = &(store->matchfilter);
  header = &(matchfilter->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_matchfilter(verbose, matchfilter, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kMatchFilter;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, matchfilter->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, matchfilter->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, matchfilter->operation, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, matchfilter->start_frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, matchfilter->end_frequency, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2firmwarehardwareconfiguration(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size,
                                                    int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2firmwarehardwareconfiguration *v2firmwarehardwareconfiguration;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2firmwarehardwareconfiguration = &(store->v2firmwarehardwareconfiguration);
  header = &(v2firmwarehardwareconfiguration->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2firmwarehardwareconfiguration(verbose, v2firmwarehardwareconfiguration, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2FirmwareHardwareConfiguration;
  *size += v2firmwarehardwareconfiguration->info_length;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, v2firmwarehardwareconfiguration->device_count, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, v2firmwarehardwareconfiguration->info_length, &buffer[index]);
    index += 4;

    /* extract the info */
    for (unsigned int i = 0; i < v2firmwarehardwareconfiguration->info_length; i++) {
      buffer[index] = v2firmwarehardwareconfiguration->info[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_beamgeometry(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_beamgeometry *beamgeometry;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  beamgeometry = &(store->beamgeometry);
  header = &(beamgeometry->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_beamgeometry(verbose, beamgeometry, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kBeamGeometry;
  *size += beamgeometry->number_beams * 16;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, beamgeometry->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, beamgeometry->number_beams, &buffer[index]);
    index += 4;

    /* insert the data */
    for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
      mb_put_binary_float(true, beamgeometry->angle_alongtrack[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
      mb_put_binary_float(true, beamgeometry->angle_acrosstrack[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
      mb_put_binary_float(true, beamgeometry->beamwidth_alongtrack[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < beamgeometry->number_beams; i++) {
      mb_put_binary_float(true, beamgeometry->beamwidth_acrosstrack[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_calibration(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_calibration *calibration;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  calibration = &(store->calibration);
  header = &(calibration->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_calibration(verbose, calibration, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kCalibrationData;
  *size += calibration->number_channels * 8;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, calibration->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_short(true, calibration->number_channels, &buffer[index]);
    index += 2;

    /* insert the data */
    for (int i = 0; i < calibration->number_channels; i++) {
      mb_put_binary_float(true, calibration->gain[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < calibration->number_channels; i++) {
      mb_put_binary_float(true, calibration->phase[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_bathymetry(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_bathymetry *bathymetry;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  bathymetry = &(store->bathymetry);
  header = &(bathymetry->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kBathymetricData;
  *size += bathymetry->number_beams * 17;
  if (bathymetry->optionaldata) {
    *size += 45 + bathymetry->number_beams * 20;
    header->OffsetToOptionalData =
        MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_7kBathymetricData + bathymetry->number_beams * 17;
  }
  else
    header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_bathymetry(verbose, bathymetry, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* make sure the version number is right */
    if (header->Version < 5)
      header->Version = 5;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, bathymetry->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, bathymetry->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, bathymetry->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, bathymetry->number_beams, &buffer[index]);
    index += 4;
    buffer[index] = bathymetry->layer_comp_flag;
    index++;
    buffer[index] = bathymetry->sound_vel_flag;
    index++;
    mb_put_binary_float(true, bathymetry->sound_velocity, &buffer[index]);
    index += 4;

    /* insert the data */
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_put_binary_float(true, bathymetry->range[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      buffer[index] = bathymetry->quality[i];
      index++;
    }
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_put_binary_float(true, bathymetry->intensity[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_put_binary_float(true, bathymetry->min_depth_gate[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
      mb_put_binary_float(true, bathymetry->max_depth_gate[i], &buffer[index]);
      index += 4;
    }

    /* insert the optional data */
    if (bathymetry->optionaldata) {
      mb_put_binary_float(true, bathymetry->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(true, bathymetry->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(true, bathymetry->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(true, bathymetry->heading, &buffer[index]);
      index += 4;
      buffer[index] = bathymetry->height_source;
      index++;
      mb_put_binary_float(true, bathymetry->tide, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, bathymetry->roll, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, bathymetry->pitch, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, bathymetry->heave, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, bathymetry->vehicle_height, &buffer[index]);
      index += 4;
      for (unsigned int i = 0; i < bathymetry->number_beams; i++) {
        mb_put_binary_float(true, bathymetry->depth[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bathymetry->alongtrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bathymetry->acrosstrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bathymetry->pointing_angle[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bathymetry->azimuth_angle[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_backscatter(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_backscatter *backscatter;
  unsigned int data_size;
  unsigned int index = 0;
  char *buffer = NULL;
  short *short_ptr = NULL;
  int *int_ptr = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  backscatter = &(store->backscatter);
  header = &(backscatter->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_backscatter(verbose, backscatter, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kBackscatterImageData;
  *size += 2 * backscatter->number_samples * backscatter->sample_size;
  if (header->OffsetToOptionalData > 0) {
    *size += 28;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, backscatter->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, backscatter->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, backscatter->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, backscatter->beam_position, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, backscatter->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, backscatter->number_samples, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->port_beamwidth_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->port_beamwidth_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->stbd_beamwidth_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->stbd_beamwidth_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->port_steering_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->port_steering_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->stbd_steering_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, backscatter->stbd_steering_y, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, backscatter->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, backscatter->current_beam, &buffer[index]);
    index += 2;
    buffer[index] = backscatter->sample_size;
    index++;
    buffer[index] = backscatter->data_type;
    index++;

    /* allocate memory if required */
    data_size = backscatter->number_samples * backscatter->sample_size;
    if (backscatter->nalloc < data_size) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(backscatter->port_data), error);
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(backscatter->stbd_data), error);
      if (status == MB_SUCCESS) {
        backscatter->nalloc = data_size;
      }
      else {
        backscatter->nalloc = 0;
        backscatter->number_samples = 0;
      }
    }

    /* extract backscatter data */
    if (backscatter->sample_size == 1) {
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        buffer[index] = backscatter->port_data[i];
        index++;
      }
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        buffer[index] = backscatter->stbd_data[i];
        index++;
      }
    }
    else if (backscatter->sample_size == 2) {
      short_ptr = (short *)backscatter->port_data;
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        mb_put_binary_short(true, short_ptr[i], &buffer[index]);
        index += 2;
      }
      short_ptr = (short *)backscatter->stbd_data;
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        mb_put_binary_short(true, short_ptr[i], &buffer[index]);
        index += 2;
      }
    }
    else if (backscatter->sample_size == 4) {
      int_ptr = (int *)backscatter->port_data;
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        mb_put_binary_int(true, int_ptr[i], &buffer[index]);
        index += 4;
      }
      int_ptr = (int *)backscatter->stbd_data;
      for (unsigned int i = 0; i < backscatter->number_samples; i++) {
        mb_put_binary_int(true, int_ptr[i], &buffer[index]);
        index += 4;
      }
    }

    /* extract the optional data */
    if (header->OffsetToOptionalData > 0) {
      index = header->OffsetToOptionalData;
      backscatter->optionaldata = true;
      mb_put_binary_float(true, backscatter->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(true, backscatter->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(true, backscatter->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(true, backscatter->heading, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, backscatter->altitude, &buffer[index]);
      index += 4;
    }
    else {
      backscatter->optionaldata = false;
      backscatter->frequency = 0.0;
      backscatter->latitude = 0.0;
      backscatter->longitude = 0.0;
      backscatter->heading = 0.0;
      backscatter->altitude = 0.0;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_beam(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_beam *beam;
  s7kr_snippet *snippet;
  unsigned int index = 0;
  char *buffer = NULL;
  int nsamples;
  int sample_type_amp;
  int sample_type_phase;
  int sample_type_iandq;
  int sample_size;
  char *charptr;
  unsigned short *ushortptr;
  unsigned int *uintptr;
  short *shortptramp;
  short *shortptrphase;
  int *intptramp;
  int *intptrphase;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  beam = &(store->beam);
  header = &(beam->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_beam(verbose, beam, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kBeamData;
  sample_type_amp = beam->sample_type & 15;
  sample_type_phase = (beam->sample_type >> 4) & 15;
  sample_type_iandq = (beam->sample_type >> 8) & 15;
  sample_size = 0;
  if (sample_type_amp == 1)
    sample_size += 1;
  else if (sample_type_amp == 2)
    sample_size += 2;
  else if (sample_type_amp == 3)
    sample_size += 4;
  if (sample_type_phase == 1)
    sample_size += 1;
  else if (sample_type_phase == 2)
    sample_size += 2;
  else if (sample_type_phase == 3)
    sample_size += 4;
  if (sample_type_iandq == 1)
    sample_size += 4;
  else if (sample_type_iandq == 2)
    sample_size += 8;
  for (int i = 0; i < beam->number_beams; i++) {
    snippet = &beam->snippets[i];
    *size += 10 + sample_size * (snippet->end_sample - snippet->begin_sample + 1);
  }
  if (header->OffsetToOptionalData > 0) {
    *size += 24 + beam->number_beams * 12;
  }

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, beam->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, beam->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, beam->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, beam->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, beam->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, beam->number_samples, &buffer[index]);
    index += 4;
    buffer[index] = beam->record_subset_flag;
    index++;
    buffer[index] = beam->row_column_flag;
    index++;
    mb_put_binary_short(true, beam->sample_header_id, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, beam->sample_type, &buffer[index]);
    index += 4;
    for (int i = 0; i < beam->number_beams; i++) {
      snippet = &beam->snippets[i];
      mb_put_binary_short(true, snippet->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_int(true, snippet->begin_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, snippet->end_sample, &buffer[index]);
      index += 4;
    }

    for (int i = 0; i < beam->number_beams; i++) {
      /* extract snippet or beam data data */
      if (status == MB_SUCCESS) {
        nsamples = snippet->end_sample - snippet->begin_sample + 1;
        for (int j = 0; j < nsamples; j++) {
          if (sample_type_amp == 1) {
            charptr = (char *)snippet->amplitude;
            buffer[index] = charptr[j];
            index++;
          }
          else if (sample_type_amp == 2) {
            ushortptr = (unsigned short *)snippet->amplitude;
            mb_put_binary_short(true, ushortptr[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_amp == 3) {
            uintptr = (unsigned int *)snippet->amplitude;
            mb_put_binary_int(true, uintptr[j], &buffer[index]);
            index += 4;
          }
          if (sample_type_phase == 1) {
            charptr = (char *)snippet->phase;
            buffer[index] = charptr[j];
            index++;
          }
          else if (sample_type_phase == 2) {
            ushortptr = (unsigned short *)snippet->phase;
            mb_put_binary_short(true, ushortptr[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_phase == 3) {
            uintptr = (unsigned int *)snippet->phase;
            mb_put_binary_int(true, uintptr[j], &buffer[index]);
            index += 4;
          }
          if (sample_type_iandq == 1) {
            shortptramp = (short *)snippet->amplitude;
            shortptrphase = (short *)snippet->phase;
            mb_put_binary_short(true, shortptramp[j], &buffer[index]);
            index += 2;
            mb_put_binary_short(true, shortptrphase[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_iandq == 2) {
            intptramp = (int *)snippet->amplitude;
            intptrphase = (int *)snippet->phase;
            mb_put_binary_int(true, intptramp[j], &buffer[index]);
            index += 4;
            mb_put_binary_int(true, intptrphase[j], &buffer[index]);
            index += 4;
          }
        }
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_verticaldepth(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_verticaldepth *verticaldepth;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  verticaldepth = &(store->verticaldepth);
  header = &(verticaldepth->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_verticaldepth(verbose, verticaldepth, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kVerticalDepth;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, verticaldepth->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, verticaldepth->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, verticaldepth->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_double(true, verticaldepth->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, verticaldepth->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, verticaldepth->heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, verticaldepth->alongtrack, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, verticaldepth->acrosstrack, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, verticaldepth->vertical_depth, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_tvg(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_tvg *tvg;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  tvg = &(store->tvg);
  header = &(tvg->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_tvg(verbose, tvg, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kTVGData;
  *size += tvg->n * sizeof(float);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, tvg->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, tvg->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, tvg->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, tvg->n, &buffer[index]);
    index += 4;
    for (int i = 0; i < 8; i++) {
      mb_put_binary_int(true, tvg->reserved[i], &buffer[index]);
      index += 4;
    }

    /* insert tvg data */
    memcpy((void *)&buffer[index], (const void *)tvg->tvg, (size_t)(tvg->n * sizeof(float)));
    index += tvg->n * sizeof(float);

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_image(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_image *image;
  unsigned int index = 0;
  char *buffer = NULL;
  unsigned int nalloc = 0;
  char *charptr;
  unsigned short *ushortptr;
  unsigned int *uintptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  image = &(store->image);
  header = &(image->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_image(verbose, image, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kImageData;
  *size += image->width * image->height * image->color_depth;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(true, image->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, image->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, image->width, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, image->height, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, image->color_depth, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, image->width_height_flag, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, image->compression, &buffer[index]);
    index += 2;

    /* allocate memory for image if needed */
    nalloc = image->width * image->height * image->color_depth;
    if (status == MB_SUCCESS && image->nalloc < nalloc) {
      image->nalloc = nalloc;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, image->nalloc, (void **)&(image->image), error);
      if (status != MB_SUCCESS) {
        image->nalloc = 0;
        image->width = 0;
        image->height = 0;
      }
    }

    /* extract image data */
    if (image->color_depth == 1) {
      charptr = (char *)image->image;
      for (unsigned int i = 0; i < image->width * image->height; i++) {
        buffer[index] = charptr[i];
        index++;
      }
    }
    else if (image->color_depth == 2) {
      ushortptr = (unsigned short *)image->image;
      for (unsigned int i = 0; i < image->width * image->height; i++) {
        mb_put_binary_short(true, ushortptr[i], &buffer[index]);
        index += 2;
      }
    }
    else if (image->color_depth == 4) {
      uintptr = (unsigned int *)image->image;
      for (unsigned int i = 0; i < image->width * image->height; i++) {
        mb_put_binary_int(true, uintptr[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2pingmotion(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2pingmotion *v2pingmotion;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2pingmotion = &(store->v2pingmotion);
  header = &(v2pingmotion->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2pingmotion(verbose, v2pingmotion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2PingMotion;
  if (v2pingmotion->flags & 1)
    *size += sizeof(float);
  if (v2pingmotion->flags & 2)
    *size += sizeof(float) * v2pingmotion->n;
  if (v2pingmotion->flags & 4)
    *size += sizeof(float) * v2pingmotion->n;
  if (v2pingmotion->flags & 8)
    *size += sizeof(float) * v2pingmotion->n;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2pingmotion->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2pingmotion->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2pingmotion->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2pingmotion->n, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2pingmotion->flags, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2pingmotion->error_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2pingmotion->frequency, &buffer[index]);
    index += 4;
    if (v2pingmotion->flags & 1) {
      mb_put_binary_float(true, v2pingmotion->pitch, &buffer[index]);
      index += 4;
    }
    if (v2pingmotion->flags & 2) {
      for (unsigned int i = 0; i < v2pingmotion->n; i++) {
        mb_put_binary_float(true, v2pingmotion->roll[i], &buffer[index]);
        index += 4;
      }
    }
    if (v2pingmotion->flags & 4) {
      for (unsigned int i = 0; i < v2pingmotion->n; i++) {
        mb_put_binary_float(true, v2pingmotion->heading[i], &buffer[index]);
        index += 4;
      }
    }
    if (v2pingmotion->flags & 8) {
      for (unsigned int i = 0; i < v2pingmotion->n; i++) {
        mb_put_binary_float(true, v2pingmotion->heave[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2detectionsetup(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2detectionsetup *v2detectionsetup;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2detectionsetup = &(store->v2detectionsetup);
  header = &(v2detectionsetup->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2detectionsetup(verbose, v2detectionsetup, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2DetectionSetup;
  *size += v2detectionsetup->number_beams * v2detectionsetup->data_field_size;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2detectionsetup->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2detectionsetup->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2detectionsetup->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2detectionsetup->number_beams, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, v2detectionsetup->data_field_size, &buffer[index]);
    index += 4;
    buffer[index] = v2detectionsetup->detection_algorithm;
    index++;
    mb_put_binary_int(true, v2detectionsetup->detection_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->minimum_depth, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->maximum_depth, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->minimum_range, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->maximum_range, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->minimum_nadir_search, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->maximum_nadir_search, &buffer[index]);
    index += 4;
    buffer[index] = v2detectionsetup->automatic_filter_window;
    index++;
    mb_put_binary_float(true, v2detectionsetup->applied_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2detectionsetup->depth_gate_tilt, &buffer[index]);
    index += 4;
    for (int i = 0; i < 14; i++) {
      mb_put_binary_float(true, v2detectionsetup->reserved[i], &buffer[index]);
      index += 4;
    }
    for (unsigned int i = 0; i < v2detectionsetup->number_beams; i++) {
      mb_put_binary_short(true, v2detectionsetup->beam_descriptor[i], &buffer[index]);
      index += 2;
      mb_put_binary_float(true, v2detectionsetup->detection_point[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->flags[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->auto_limits_min_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->auto_limits_max_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->user_limits_min_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->user_limits_max_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2detectionsetup->quality[i], &buffer[index]);
      index += 4;
      if (v2detectionsetup->data_field_size >= R7KRDTSIZE_7kV2DetectionSetup + 4) {
        mb_put_binary_int(true, v2detectionsetup->uncertainty[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2beamformed(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2beamformed *v2beamformed;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2beamformed = &(store->v2beamformed);
  header = &(v2beamformed->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2beamformed(verbose, v2beamformed, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2BeamformedData;
  *size += 2 * sizeof(short) * v2beamformed->number_beams * v2beamformed->number_samples;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2beamformed->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2beamformed->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2beamformed->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, v2beamformed->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2beamformed->number_samples, &buffer[index]);
    index += 4;
    for (int i = 0; i < 32; i++) {
      buffer[index] = v2beamformed->reserved[i];
      index++;
    }
    for (int i = 0; i < v2beamformed->number_beams; i++) {
      s7kr_v2amplitudephase *v2amplitudephase = &(v2beamformed->amplitudephase[i]);

      /* insert v2beamformed data */
      for (unsigned int j = 0; j < v2beamformed->number_samples; j++) {
        mb_put_binary_short(true, v2amplitudephase->amplitude[j], &buffer[index]);
        index += 2;
        mb_put_binary_short(true, v2amplitudephase->phase[j], &buffer[index]);
        index += 2;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2bite(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2bite *v2bite;
  s7kr_v2bitereport *report;
  s7k_time *s7ktime;
  s7kr_v2bitefield *bitefield;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2bite = &(store->v2bite);
  header = &(v2bite->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2bite(verbose, v2bite, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2BITEData;
  for (int i = 0; i < v2bite->number_reports; i++) {
    report = &(v2bite->reports[i]);
    *size += R7KRDTSIZE_7kV2BITERecordData + report->number_bite * R7KRDTSIZE_7kV2BITEFieldData;
  }

  /* allocate memory to store rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_short(true, v2bite->number_reports, &buffer[index]);
    index += 2;
    for (int i = 0; i < v2bite->number_reports; i++) {
      report = &(v2bite->reports[i]);

      for (int j = 0; j < 64; j++) {
        buffer[index] = report->source_name[j];
        index++;
      }
      buffer[index] = report->source_address;
      index++;
      mb_put_binary_float(true, report->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_short(true, report->enumerator, &buffer[index]);
      index += 2;

      s7ktime = &(report->downlink_time);
      mb_put_binary_short(true, s7ktime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, s7ktime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, s7ktime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7ktime->Hours;
      index++;
      buffer[index] = s7ktime->Minutes;
      index++;

      s7ktime = &(report->uplink_time);
      mb_put_binary_short(true, s7ktime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, s7ktime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, s7ktime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7ktime->Hours;
      index++;
      buffer[index] = s7ktime->Minutes;
      index++;

      s7ktime = &(report->bite_time);
      mb_put_binary_short(true, s7ktime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, s7ktime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, s7ktime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7ktime->Hours;
      index++;
      buffer[index] = s7ktime->Minutes;
      index++;

      buffer[index] = report->status;
      index++;
      mb_put_binary_short(true, report->number_bite, &buffer[index]);
      index += 2;
      for (int j = 0; j < 32; j++) {
        buffer[index] = report->bite_status[j];
        index++;
      }

      /* loop over all bite fields */
      for (int j = 0; j < report->number_bite; j++) {
        bitefield = &(report->bitefield[j]);

        mb_put_binary_short(true, bitefield->reserved, &buffer[index]);
        index += 2;
        for (int k = 0; k < 64; k++) {
          buffer[index] = bitefield->name[k];
          index++;
        }
        buffer[index] = bitefield->device_type;
        index++;
        mb_put_binary_float(true, bitefield->minimum, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bitefield->maximum, &buffer[index]);
        index += 4;
        mb_put_binary_float(true, bitefield->value, &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v27kcenterversion(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v27kcenterversion *v27kcenterversion;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v27kcenterversion = &(store->v27kcenterversion);
  header = &(v27kcenterversion->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v27kcenterversion(verbose, v27kcenterversion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV27kCenterVersion;

  /* allocate memory to store rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      buffer[index] = v27kcenterversion->version[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v28kwetendversion(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v28kwetendversion *v28kwetendversion;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v28kwetendversion = &(store->v28kwetendversion);
  header = &(v28kwetendversion->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v28kwetendversion(verbose, v28kwetendversion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV28kWetEndVersion;

  /* allocate memory to store rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      buffer[index] = v28kwetendversion->version[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2detection(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2detection *v2detection;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2detection = &(store->v2detection);
  header = &(v2detection->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2Detection;
  *size += v2detection->number_beams * v2detection->data_field_size;
  header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2detection(verbose, v2detection, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2detection->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2detection->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2detection->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2detection->number_beams, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, v2detection->data_field_size, &buffer[index]);
    index += 4;
    mb_put_binary_long(true, v2detection->corrections, &buffer[index]);
    index += 8;
    buffer[index] = v2detection->detection_algorithm;
    index++;
    mb_put_binary_int(true, v2detection->flags, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = v2detection->reserved[i];
      index++;
    }

    /* insert the data */
    for (unsigned int i = 0; i < v2detection->number_beams; i++) {
      mb_put_binary_float(true, v2detection->range[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2detection->angle_x[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2detection->angle_y[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2detection->range_error[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2detection->angle_x_error[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2detection->angle_y_error[i], &buffer[index]);
      index += 4;
      if (v2detection->data_field_size > 24)
        for (unsigned int j = 0; j < v2detection->data_field_size - 24; j++) {
          buffer[index] = 0;
          index++;
        }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2rawdetection(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2rawdetection *v2rawdetection;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2rawdetection = &(store->v2rawdetection);
  header = &(v2rawdetection->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2RawDetection;
  *size += v2rawdetection->number_beams * v2rawdetection->data_field_size;
  header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2rawdetection(verbose, v2rawdetection, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2rawdetection->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2rawdetection->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2rawdetection->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, v2rawdetection->number_beams, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, v2rawdetection->data_field_size, &buffer[index]);
    index += 4;
    buffer[index] = v2rawdetection->detection_algorithm;
    index++;
    mb_put_binary_int(true, v2rawdetection->detection_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2rawdetection->sampling_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, v2rawdetection->tx_angle, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = v2rawdetection->reserved[i];
      index++;
    }

    /* insert the data */
    for (unsigned int i = 0; i < v2rawdetection->number_beams; i++) {
      mb_put_binary_short(true, v2rawdetection->beam_descriptor[i], &buffer[index]);
      index += 2;
      mb_put_binary_float(true, v2rawdetection->detection_point[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2rawdetection->rx_angle[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2rawdetection->flags[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(true, v2rawdetection->quality[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(true, v2rawdetection->uncertainty[i], &buffer[index]);
      index += 4;
      if (v2rawdetection->data_field_size > 22)
        for (unsigned int j = 0; j < v2rawdetection->data_field_size - 22; j++) {
          buffer[index] = 0;
          index++;
        }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_v2snippet(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_v2snippet *v2snippet;
  s7kr_v2snippettimeseries *snippettimeseries;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  v2snippet = &(store->v2snippet);
  header = &(v2snippet->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kV2SnippetData;
  for (int i = 0; i < v2snippet->number_beams; i++) {
    snippettimeseries = &(v2snippet->snippettimeseries[i]);

    *size += R7KRDTSIZE_7kV2SnippetTimeseries +
             sizeof(short) * (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1);
  }
  header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_v2snippet(verbose, v2snippet, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, v2snippet->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, v2snippet->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, v2snippet->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, v2snippet->number_beams, &buffer[index]);
    index += 2;
    buffer[index] = v2snippet->error_flag;
    index++;
    buffer[index] = v2snippet->control_flags;
    index++;
    for (int i = 0; i < 28; i++) {
      buffer[index] = v2snippet->reserved[i];
      index++;
    }

    /* insert the snippet parameters */
    for (int i = 0; i < v2snippet->number_beams; i++) {
      snippettimeseries = &(v2snippet->snippettimeseries[i]);

      /* extract snippettimeseries data */
      mb_put_binary_short(true, snippettimeseries->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_int(true, snippettimeseries->begin_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, snippettimeseries->detect_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, snippettimeseries->end_sample, &buffer[index]);
      index += 4;
    }

    /* loop over all beams to insert snippet data */
    for (int i = 0; i < v2snippet->number_beams; i++) {
      snippettimeseries = &(v2snippet->snippettimeseries[i]);
      for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
        mb_put_binary_short(true, snippettimeseries->amplitude[j], &buffer[index]);
        index += 2;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_calibratedsnippet(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_calibratedsnippet *calibratedsnippet;
  s7kr_calibratedsnippettimeseries *snippettimeseries;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  calibratedsnippet = &(store->calibratedsnippet);
  header = &(calibratedsnippet->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kCalibratedSnippetData;
  for (int i = 0; i < calibratedsnippet->number_beams; i++) {
    snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);

    *size += R7KRDTSIZE_7kCalibratedSnippetTimeseries;
    *size += sizeof(float) * (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1);
    if (calibratedsnippet->control_flags & 0x40) {
      *size += sizeof(float) * (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1);
    }
  }
  header->OffsetToOptionalData = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_calibratedsnippet(verbose, calibratedsnippet, error);

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, calibratedsnippet->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, calibratedsnippet->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, calibratedsnippet->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, calibratedsnippet->number_beams, &buffer[index]);
    index += 2;
    buffer[index] = calibratedsnippet->error_flag;
    index++;
    mb_put_binary_int(true, calibratedsnippet->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, calibratedsnippet->absorption, &buffer[index]);
    index += 4;
    for (int i = 0; i < 6; i++) {
      mb_put_binary_int(true, calibratedsnippet->reserved[i], &buffer[index]);
      index += 4;
    }

    /* insert the snippet parameters */
    for (int i = 0; i < calibratedsnippet->number_beams; i++) {
      snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);

      /* insert snippettimeseries data */
      mb_put_binary_short(true, snippettimeseries->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_int(true, snippettimeseries->begin_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, snippettimeseries->detect_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, snippettimeseries->end_sample, &buffer[index]);
      index += 4;
    }

    /* loop over all beams to insert snippet amplitude data */
    for (int i = 0; i < calibratedsnippet->number_beams; i++) {
      snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);
      for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
        mb_put_binary_float(true, snippettimeseries->amplitude[j], &buffer[index]);
        index += 4;
      }
    }

    /* loop over all beams to insert snippet footprint data */
    if (calibratedsnippet->control_flags & 0x40) {
      for (int i = 0; i < calibratedsnippet->number_beams; i++) {
        snippettimeseries = &(calibratedsnippet->calibratedsnippettimeseries[i]);
        for (unsigned int j = 0; j < (snippettimeseries->end_sample - snippettimeseries->begin_sample + 1); j++) {
          mb_put_binary_float(true, snippettimeseries->amplitude[j], &buffer[index]);
          index += 4;
        }
      }
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
} /*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_installation(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  int status = MB_SUCCESS;
  s7k_header *header;
  s7kr_installation *installation;
  unsigned int index = 0;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  installation = &(store->installation);
  header = &(installation->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_installation(verbose, installation, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kInstallationParameters;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, installation->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, installation->firmware_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = installation->firmware_version[i];
      index++;
    }
    mb_put_binary_short(true, installation->software_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = installation->software_version[i];
      index++;
    }
    mb_put_binary_short(true, installation->s7k_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = installation->s7k_version[i];
      index++;
    }
    mb_put_binary_short(true, installation->protocal_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = installation->protocal_version[i];
      index++;
    }
    mb_put_binary_float(true, installation->transmit_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->transmit_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->transmit_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->transmit_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->transmit_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->transmit_heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->receive_heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->motion_heading, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, installation->motion_time_delay, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, installation->position_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->position_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, installation->position_z, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, installation->position_time_delay, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, installation->waterline_z, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_fileheader(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_fileheader *fileheader = &(store->fileheader);
  s7k_header *header = &(fileheader->header);

  /* make sure data are defined properly */
  if (header->RecordType != R7KRECID_7kFileHeader) {
    /* Reson 7k data record header information */
    header->Version = 4;
    header->Offset = 60;
    header->SyncPattern = 0x0000ffff;
    header->OffsetToOptionalData = 0;
    header->OptionalDataIdentifier = 0;
    header->s7kTime.Year = 0;
    header->s7kTime.Day = 0;
    header->s7kTime.Seconds = 0.0;
    header->s7kTime.Hours = 0;
    header->s7kTime.Minutes = 0;
    header->Reserved = 0;
    header->RecordType = R7KRECID_7kFileHeader;
    header->DeviceId = 0;
    header->Reserved2 = 0;
    header->SystemEnumerator = 0;
    header->DataSetNumber = 0;
    header->RecordNumber = 0;
    for (int i = 0; i < 8; i++) {
      header->PreviousRecord[i] = -1;
      header->NextRecord[i] = -1;
    }
    header->Flags = 0;
    header->Reserved3 = 0;
    header->Reserved4 = 0;
    header->FragmentedTotal = 0;
    header->FragmentNumber = 0;
  }

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_fileheader(verbose, fileheader, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kFileHeader + R7KRDTSIZE_7kFileHeader;
  for (unsigned int i = 0; i < fileheader->number_subsystems; i++)
    *size += 6;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 16; i++) {
      buffer[index] = fileheader->file_identifier[i];
      index++;
    }
    mb_put_binary_short(true, fileheader->version, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, fileheader->reserved, &buffer[index]);
    index += 2;
    for (int i = 0; i < 16; i++) {
      buffer[index] = fileheader->session_identifier[i];
      index++;
    }
    mb_put_binary_int(true, fileheader->record_data_size, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, fileheader->number_subsystems, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = fileheader->recording_name[i];
      index++;
    }
    for (int i = 0; i < 16; i++) {
      buffer[index] = fileheader->recording_version[i];
      index++;
    }
    for (int i = 0; i < 64; i++) {
      buffer[index] = fileheader->user_defined_name[i];
      index++;
    }
    for (int i = 0; i < 128; i++) {
      buffer[index] = fileheader->notes[i];
      index++;
    }
    for (unsigned int i = 0; i < fileheader->number_subsystems; i++) {
      s7kr_subsystem *subsystem = &(fileheader->subsystem[i]);
      mb_put_binary_int(true, subsystem->device_identifier, &buffer[index]);
      index += 4;
      mb_put_binary_short(true, subsystem->system_enumerator, &buffer[index]);
      index += 2;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_systemeventmessage(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_systemeventmessage *systemeventmessage = &(store->systemeventmessage);
  s7k_header *header = &(systemeventmessage->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_systemeventmessage(verbose, systemeventmessage, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kSystemEventMessage;
  *size += systemeventmessage->message_length;

  int status = MB_SUCCESS;
  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, systemeventmessage->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_short(true, systemeventmessage->event_id, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, systemeventmessage->message_length, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, systemeventmessage->event_identifier, &buffer[index]);
    index += 2;

    /* insert the data */
    for (int i = 0; i < systemeventmessage->message_length; i++) {
      buffer[index] = systemeventmessage->message[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_remotecontrolsettings(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size,
                                          int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_remotecontrolsettings *remotecontrolsettings = &(store->remotecontrolsettings);
  s7k_header *header = &(remotecontrolsettings->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_remotecontrolsettings(verbose, remotecontrolsettings, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kRemoteControlSonarSettings;

  int status = MB_SUCCESS;
  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(true, remotecontrolsettings->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, remotecontrolsettings->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->receiver_bandwidth, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->pulse_width, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->pulse_type, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->pulse_envelope, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->pulse_envelope_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->pulse_reserved, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->max_ping_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->ping_period, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->range_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->power_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->gain_selection, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->projector_magic_no, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->steering_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->steering_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->beamwidth_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->beamwidth_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->focal_point, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->projector_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->projector_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->transmit_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->hydrophone_magic_no, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->receive_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->receive_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, remotecontrolsettings->receive_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->range_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->range_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->depth_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->depth_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->absorption, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->spreading, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, remotecontrolsettings->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, remotecontrolsettings->tx_offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->tx_offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->tx_offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->head_tilt_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->head_tilt_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->head_tilt_z, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, remotecontrolsettings->ping_on_off, &buffer[index]);
    index += 2;
    buffer[index] = remotecontrolsettings->data_sample_types;
    index++;
    buffer[index] = remotecontrolsettings->projector_orientation;
    index++;
    mb_put_binary_short(true, remotecontrolsettings->beam_angle_mode, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, remotecontrolsettings->r7kcenter_mode, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, remotecontrolsettings->gate_depth_min, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, remotecontrolsettings->gate_depth_max, &buffer[index]);
    index += 4;
    for (int i = 0; i < 35; i++) {
      mb_put_binary_short(true, remotecontrolsettings->reserved2[i], &buffer[index]);
      index += 2;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_reserved(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_reserved *reserved = &(store->reserved);
  s7k_header *header = &(reserved->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_reserved(verbose, reserved, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kReserved;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < R7KHDRSIZE_7kReserved; i++) {
      buffer[index] = reserved->reserved[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_roll(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_roll *roll = &(store->roll);
  s7k_header *header = &(roll->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_roll(verbose, roll, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kRoll;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, roll->roll, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_pitch(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_pitch *pitch = &(store->pitch);
  s7k_header *header = &(pitch->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_pitch(verbose, pitch, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kPitch;

  int status = MB_SUCCESS;
  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, pitch->pitch, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_soundvelocity(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_soundvelocity *soundvelocity = &(store->soundvelocity);
  s7k_header *header = &(soundvelocity->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_soundvelocity(verbose, soundvelocity, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kSoundVelocity;

  int status = MB_SUCCESS;
  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, soundvelocity->soundvelocity, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_absorptionloss(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_absorptionloss *absorptionloss = &(store->absorptionloss);
  s7k_header *header = &(absorptionloss->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_absorptionloss(verbose, absorptionloss, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kAbsorptionLoss;

  int status = MB_SUCCESS;
  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, absorptionloss->absorptionloss, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_spreadingloss(int verbose, unsigned int *bufferalloc, char **bufferptr, void *store_ptr, unsigned int *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  s7kr_spreadingloss *spreadingloss = &(store->spreadingloss);
  s7k_header *header = &(spreadingloss->header);

/* print out the data to be output */
#ifdef MBR_RESON7KR_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k_print_spreadingloss(verbose, spreadingloss, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_7kSpreadingLoss;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS) {
      *bufferalloc = 0;
    }
    else {
      *bufferalloc = *size;
    }
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    char *buffer = (char *)*bufferptr;

    /* insert the header */
    unsigned int index = 0;
    status = mbr_reson7kr_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(true, spreadingloss->spreadingloss, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(true, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (unsigned int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(true, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_DATA;
      *size = 0;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       size:       %d\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_reson7kr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;
  // FILE *mbfp = mb_io_ptr->mbfp;

  /* get saved values */
  char **bufferptr = (char **)&mb_io_ptr->saveptr1;
  char *buffer = (char *)*bufferptr;
  unsigned int *bufferalloc = (unsigned int *)&mb_io_ptr->save6;
  int *fileheaders = (int *)&mb_io_ptr->save12;
  unsigned int size = 0;
  size_t write_len = 0;

  int status = MB_SUCCESS;

  /* write fileheader if needed */
  if (status == MB_SUCCESS && (store->type == R7KRECID_7kFileHeader || *fileheaders == 0)) {
#ifdef MBR_RESON7KR_DEBUG2
    fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kFileHeader, R7KRECID_7kFileHeader);
    fprintf(stderr, " R7KRECID_7kFileHeader\n");
#endif
    status = mbr_reson7kr_wr_fileheader(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    (*fileheaders)++;

    /* write the record to the output file */
    if (status == MB_SUCCESS) {
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
      store->nrec_fileheader++;
    }
  }

    /* if flag mb_io_ptr->save15 has been set, then only raw bathymetry,
     * navigation, heading, and attitude data records should be written */
    if (mb_io_ptr->save15) {
        store->read_matchfilter = false;
        //store->read_volatilesettings;
        store->read_matchfilter = false;
        //store->read_beamgeometry;
        store->read_remotecontrolsettings = false;
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
        //store->read_v2rawdetection;
        store->read_v2snippet = false;
        store->read_calibratedsnippet = false;
        store->read_processedsidescan = false;
    }

  /* call appropriate writing routines for ping data */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA) {
    /* Reson 7k volatile sonar settings (record 7000) */
    if (status == MB_SUCCESS && store->read_volatilesettings) {
      store->type = R7KRECID_7kVolatileSonarSettings;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kVolatileSonarSettings, R7KRECID_7kVolatileSonarSettings);
      fprintf(stderr, " R7KRECID_7kVolatileSonarSettings\n");
#endif
      status = mbr_reson7kr_wr_volatilesonarsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }
    /* Reson 7k match filter (record 7002) */
    if (status == MB_SUCCESS && store->read_matchfilter) {
      store->type = R7KRECID_7kMatchFilter;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kMatchFilter, R7KRECID_7kMatchFilter);
      fprintf(stderr, " R7KRECID_7kMatchFilter\n");
#endif
      status = mbr_reson7kr_wr_matchfilter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k beam geometry (record 7004) */
    if (status == MB_SUCCESS && store->read_beamgeometry) {
      store->type = R7KRECID_7kBeamGeometry;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kBeamGeometry, R7KRECID_7kBeamGeometry);
      fprintf(stderr, " R7KRECID_7kBeamGeometry\n");
#endif
      status = mbr_reson7kr_wr_beamgeometry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k remote control settings (record 7503) */
    if (status == MB_SUCCESS && store->read_remotecontrolsettings) {
      store->type = R7KRECID_7kRemoteControlSonarSettings;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kRemoteControlSonarSettings,
              R7KRECID_7kRemoteControlSonarSettings);
      fprintf(stderr, " R7KRECID_7kRemoteControlSonarSettings\n");
#endif
      status = mbr_reson7kr_wr_remotecontrolsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k bathymetry (record 7006) */
    if (status == MB_SUCCESS && store->read_bathymetry) {
      store->type = R7KRECID_7kBathymetricData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kBathymetricData, R7KRECID_7kBathymetricData);
      fprintf(stderr, " R7KRECID_7kBathymetricData\n");
#endif

      /*mbsys_reson7k_print_bathymetry(verbose, &(store->bathymetry), error);*/

      status = mbr_reson7kr_wr_bathymetry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Processed sidescan - MB-System extension to 7k format (record 3199) */
    if (status == MB_SUCCESS && store->read_processedsidescan) {
      store->type = R7KRECID_ProcessedSidescan;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_ProcessedSidescan, R7KRECID_ProcessedSidescan);
      fprintf(stderr, " R7KRECID_ProcessedSidescan\n");
#endif

      /*mbsys_reson7k_print_processedsidescan(verbose, &(store->processedsidescan), error);*/

      status = mbr_reson7kr_wr_processedsidescan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k backscatter imagery data (record 7007) */
    if (status == MB_SUCCESS && store->read_backscatter) {
      store->type = R7KRECID_7kBackscatterImageData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kBackscatterImageData, R7KRECID_7kBackscatterImageData);
      fprintf(stderr, " R7KRECID_7kBackscatterImageData\n");
#endif
      status = mbr_reson7kr_wr_backscatter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k beam data (record 7008) */
    if (status == MB_SUCCESS && store->read_beam) {
      store->type = R7KRECID_7kBeamData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kBeamData, R7KRECID_7kBeamData);
      fprintf(stderr, " R7KRECID_7kBeamData\n");
#endif
      status = mbr_reson7kr_wr_beam(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k vertical depth (record 7009) */
    if (status == MB_SUCCESS && store->read_verticaldepth) {
      store->type = R7KRECID_7kVerticalDepth;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kVerticalDepth, R7KRECID_7kVerticalDepth);
      fprintf(stderr, " R7KRECID_7kVerticalDepth\n");
#endif
      status = mbr_reson7kr_wr_verticaldepth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k tvg data (record 7010) */
    if (status == MB_SUCCESS && store->read_tvg) {
      store->type = R7KRECID_7kTVGData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kTVGData, R7KRECID_7kTVGData);
      fprintf(stderr, " R7KRECID_7kTVGData\n");
#endif
      status = mbr_reson7kr_wr_tvg(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k image data (record 7011) */
    if (status == MB_SUCCESS && store->read_image) {
      store->type = R7KRECID_7kImageData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kImageData, R7KRECID_7kImageData);
      fprintf(stderr, " R7KRECID_7kImageData\n");
#endif
      status = mbr_reson7kr_wr_image(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k Ping motion (record 7012) */
    if (status == MB_SUCCESS && store->read_v2pingmotion) {
      store->type = R7KRECID_7kV2PingMotion;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2PingMotion, R7KRECID_7kV2PingMotion);
      fprintf(stderr, " R7KRECID_7kV2PingMotion\n");
#endif
      status = mbr_reson7kr_wr_v2pingmotion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k detection setup (record 7017) */
    if (status == MB_SUCCESS && store->read_v2detectionsetup) {
      store->type = R7KRECID_7kV2DetectionSetup;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2DetectionSetup, R7KRECID_7kV2DetectionSetup);
      fprintf(stderr, " R7KRECID_7kV2DetectionSetup\n");
#endif
      status = mbr_reson7kr_wr_v2detectionsetup(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k beamformed magnitude and phase data (record 7018) */
    if (status == MB_SUCCESS && store->read_v2beamformed) {
      store->type = R7KRECID_7kV2BeamformedData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2BeamformedData, R7KRECID_7kV2BeamformedData);
      fprintf(stderr, " R7KRECID_7kV2BeamformedData\n");
#endif
      status = mbr_reson7kr_wr_v2beamformed(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k version 2 detection (record 7026) */
    if (status == MB_SUCCESS && store->read_v2detection) {
      store->type = R7KRECID_7kV2Detection;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2Detection, R7KRECID_7kV2Detection);
      fprintf(stderr, " R7KRECID_7kV2Detection\n");
#endif
      status = mbr_reson7kr_wr_v2detection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k version 2 raw detection (record 7027) */
    if (status == MB_SUCCESS && store->read_v2rawdetection) {
      store->type = R7KRECID_7kV2RawDetection;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2RawDetection, R7KRECID_7kV2RawDetection);
      fprintf(stderr, " R7KRECID_7kV2RawDetection\n");
#endif
      status = mbr_reson7kr_wr_v2rawdetection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k version 2 snippet (record 7028) */
    if (status == MB_SUCCESS && store->read_v2snippet) {
      store->type = R7KRECID_7kV2SnippetData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kV2SnippetData, R7KRECID_7kV2SnippetData);
      fprintf(stderr, " R7KRECID_7kV2SnippetData\n");
#endif
      status = mbr_reson7kr_wr_v2snippet(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k calibrated snippet (record 7058) */
    if (status == MB_SUCCESS && store->read_calibratedsnippet) {
      store->type = R7KRECID_7kCalibratedSnippetData;
#ifdef MBR_RESON7KR_DEBUG2
      fprintf(stderr, "Writing record id: %4.4X | %d", R7KRECID_7kCalibratedSnippetData, R7KRECID_7kCalibratedSnippetData);
      fprintf(stderr, " R7KRECID_7kCalibratedSnippetData\n");
#endif
      status = mbr_reson7kr_wr_calibratedsnippet(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }
  }

  /* call appropriate writing routine for other records */
  else if (status == MB_SUCCESS && store->type != R7KRECID_7kFileHeader) {
#ifdef MBR_RESON7KR_DEBUG2
    fprintf(stderr, "Writing record id: %4.4X | %d", store->type, store->type);
    if (store->type == R7KRECID_ReferencePoint)
      fprintf(stderr, " R7KRECID_ReferencePoint\n");
    if (store->type == R7KRECID_UncalibratedSensorOffset)
      fprintf(stderr, " R7KRECID_UncalibratedSensorOffset\n");
    if (store->type == R7KRECID_CalibratedSensorOffset)
      fprintf(stderr, " R7KRECID_CalibratedSensorOffset\n");
    if (store->type == R7KRECID_Position)
      fprintf(stderr, " R7KRECID_Position\n");
    if (store->type == R7KRECID_CustomAttitude)
      fprintf(stderr, " R7KRECID_CustomAttitude\n");
    if (store->type == R7KRECID_Tide)
      fprintf(stderr, " R7KRECID_Tide\n");
    if (store->type == R7KRECID_Altitude)
      fprintf(stderr, " R7KRECID_Altitude\n");
    if (store->type == R7KRECID_MotionOverGround)
      fprintf(stderr, " R7KRECID_MotionOverGround\n");
    if (store->type == R7KRECID_Depth)
      fprintf(stderr, " R7KRECID_Depth\n");
    if (store->type == R7KRECID_SoundVelocityProfile)
      fprintf(stderr, " R7KRECID_SoundVelocityProfile\n");
    if (store->type == R7KRECID_CTD)
      fprintf(stderr, " R7KRECID_CTD\n");
    if (store->type == R7KRECID_Geodesy)
      fprintf(stderr, " R7KRECID_Geodesy\n");
    if (store->type == R7KRECID_RollPitchHeave)
      fprintf(stderr, " R7KRECID_RollPitchHeave\n");
    if (store->type == R7KRECID_Heading)
      fprintf(stderr, " R7KRECID_Heading\n");
    if (store->type == R7KRECID_SurveyLine)
      fprintf(stderr, " R7KRECID_SurveyLine\n");
    if (store->type == R7KRECID_Navigation)
      fprintf(stderr, " R7KRECID_Navigation\n");
    if (store->type == R7KRECID_Attitude)
      fprintf(stderr, " R7KRECID_Attitude\n");
    if (store->type == R7KRECID_Rec1022)
      fprintf(stderr, " R7KRECID_Rec1022\n");
    if (store->type == R7KRECID_GenericSensorCalibration)
      fprintf(stderr, " R7KRECID_GenericSensorCalibration\n");
    if (store->type == R7KRECID_GenericSidescan)
      fprintf(stderr, " R7KRECID_GenericSidescan\n");
    if (store->type == R7KRECID_FSDWsidescan) {
      if (store->sstype == R7KRECID_FSDWsidescanLo)
        fprintf(stderr, " R7KRECID_FSDWsidescan R7KRECID_FSDWsidescanLo\n");
      if (store->sstype == R7KRECID_FSDWsidescanHi)
        fprintf(stderr, " R7KRECID_FSDWsidescan R7KRECID_FSDWsidescanHi\n");
    }
    if (store->type == R7KRECID_FSDWsubbottom)
      fprintf(stderr, " R7KRECID_FSDWsubbottom\n");
    if (store->type == R7KRECID_Bluefin)
      fprintf(stderr, " R7KRECID_Bluefin\n");
    if (store->type == R7KRECID_7kVolatileSonarSettings)
      fprintf(stderr, " R7KRECID_7kVolatileSonarSettings\n");
    if (store->type == R7KRECID_7kConfiguration)
      fprintf(stderr, " R7KRECID_7kConfiguration\n");
    if (store->type == R7KRECID_7kMatchFilter)
      fprintf(stderr, " R7KRECID_7kMatchFilter\n");
    if (store->type == R7KRECID_7kV2FirmwareHardwareConfiguration)
      fprintf(stderr, " R7KRECID_7kV2FirmwareHardwareConfiguration\n");
    if (store->type == R7KRECID_7kBeamGeometry)
      fprintf(stderr, " R7KRECID_7kBeamGeometry\n");
    if (store->type == R7KRECID_7kCalibrationData)
      fprintf(stderr, " R7KRECID_7kCalibrationData\n");
    if (store->type == R7KRECID_7kBathymetricData)
      fprintf(stderr, " R7KRECID_7kBathymetricData\n");
    if (store->type == R7KRECID_7kBackscatterImageData)
      fprintf(stderr, " R7KRECID_7kBackscatterImageData\n");
    if (store->type == R7KRECID_7kBeamData)
      fprintf(stderr, " R7KRECID_7kBeamData\n");
    if (store->type == R7KRECID_7kVerticalDepth)
      fprintf(stderr, " R7KRECID_7kVerticalDepth\n");
    if (store->type == R7KRECID_7kTVGData)
      fprintf(stderr, " R7KRECID_7kTVGData\n");
    if (store->type == R7KRECID_7kImageData)
      fprintf(stderr, " R7KRECID_7kImageData\n");
    if (store->type == R7KRECID_7kV2PingMotion)
      fprintf(stderr, " R7KRECID_7kV2PingMotion\n");
    if (store->type == R7KRECID_7kV2DetectionSetup)
      fprintf(stderr, " R7KRECID_7kV2DetectionSetup\n");
    if (store->type == R7KRECID_7kV2BeamformedData)
      fprintf(stderr, " R7KRECID_7kV2BeamformedData\n");
    if (store->type == R7KRECID_7kV2BITEData)
      fprintf(stderr, " R7KRECID_7kV2BITEData\n");
    if (store->type == R7KRECID_7kV27kCenterVersion)
      fprintf(stderr, " R7KRECID_7kV27kCenterVersion\n");
    if (store->type == R7KRECID_7kV28kWetEndVersion)
      fprintf(stderr, " R7KRECID_7kV28kWetEndVersion\n");
    if (store->type == R7KRECID_7kV2Detection)
      fprintf(stderr, " R7KRECID_7kV2Detection\n");
    if (store->type == R7KRECID_7kV2RawDetection)
      fprintf(stderr, " R7KRECID_7kV2RawDetection\n");
    if (store->type == R7KRECID_7kV2SnippetData)
      fprintf(stderr, " R7KRECID_7kV2SnippetData\n");
    if (store->type == R7KRECID_7kCalibratedSnippetData)
      fprintf(stderr, " R7KRECID_7kCalibratedSnippetData\n");
    if (store->type == R7KRECID_7kInstallationParameters)
      fprintf(stderr, " R7KRECID_7kInstallationParameters\n");
    if (store->type == R7KRECID_7kSystemEventMessage)
      fprintf(stderr, "R7KRECID_7kSystemEventMessage\n");
    if (store->type == R7KRECID_7kDataStorageStatus)
      fprintf(stderr, " R7KRECID_7kDataStorageStatus\n");
    if (store->type == R7KRECID_7kFileHeader)
      fprintf(stderr, " R7KRECID_7kFileHeader\n");
    if (store->type == R7KRECID_7kFileCatalog)
      fprintf(stderr, " R7KRECID_7kFileCatalog\n");
    if (store->type == R7KRECID_7kTriggerSequenceSetup)
      fprintf(stderr, " R7KRECID_7kTriggerSequenceSetup\n");
    if (store->type == R7KRECID_7kTriggerSequenceDone)
      fprintf(stderr, " R7KRECID_7kTriggerSequenceDone\n");
    if (store->type == R7KRECID_7kTimeMessage)
      fprintf(stderr, " R7KRECID_7kTimeMessage\n");
    if (store->type == R7KRECID_7kRemoteControl)
      fprintf(stderr, " R7KRECID_7kRemoteControl\n");
    if (store->type == R7KRECID_7kRemoteControlAcknowledge)
      fprintf(stderr, " R7KRECID_7kRemoteControlAcknowledge\n");
    if (store->type == R7KRECID_7kRemoteControlNotAcknowledge)
      fprintf(stderr, " R7KRECID_7kRemoteControlNotAcknowledge\n");
    if (store->type == R7KRECID_7kRemoteControlSonarSettings)
      fprintf(stderr, " R7KRECID_7kRemoteControlSonarSettings\n");
    if (store->type == R7KRECID_7kReserved)
      fprintf(stderr, " R7KRECID_7kReserved\n");
    if (store->type == R7KRECID_7kRoll)
      fprintf(stderr, " R7KRECID_7kRoll\n");
    if (store->type == R7KRECID_7kPitch)
      fprintf(stderr, " R7KRECID_7kPitch\n");
    if (store->type == R7KRECID_7kSoundVelocity)
      fprintf(stderr, " R7KRECID_7kSoundVelocity\n");
    if (store->type == R7KRECID_7kAbsorptionLoss)
      fprintf(stderr, " R7KRECID_7kAbsorptionLoss\n");
    if (store->type == R7KRECID_7kSpreadingLoss)
      fprintf(stderr, " R7KRECID_7kSpreadingLoss\n");
    if (store->type == R7KRECID_7kFiller)
      fprintf(stderr, " R7KRECID_7kFiller\n");
    if (store->type == R7KRECID_8100SonarData)
      fprintf(stderr, " R7KRECID_8100SonarData\n");
#endif

    /* if flag mb_io_ptr->save15 has been set, then only raw bathymetry,
     * navigation, heading, and attitude data records should be written */
        if (mb_io_ptr->save15) {
            if (store->type == R7KRECID_Position
                || store->type == R7KRECID_Altitude
                || store->type == R7KRECID_Depth
                || store->type == R7KRECID_CTD
                || store->type == R7KRECID_RollPitchHeave
                || store->type == R7KRECID_Heading
                || store->type == R7KRECID_Navigation
                || store->type == R7KRECID_Attitude) {

            }
            else {
                store->type = R7KRECID_None;
            }
        }

    if (store->type == R7KRECID_ReferencePoint) {
      status = mbr_reson7kr_wr_reference(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_UncalibratedSensorOffset) {
      status = mbr_reson7kr_wr_sensoruncal(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_CalibratedSensorOffset) {
      status = mbr_reson7kr_wr_sensorcal(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Position) {
      status = mbr_reson7kr_wr_position(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_CustomAttitude) {
      status = mbr_reson7kr_wr_customattitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Tide) {
      status = mbr_reson7kr_wr_tide(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Altitude) {
      status = mbr_reson7kr_wr_altitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_MotionOverGround) {
      status = mbr_reson7kr_wr_motion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Depth) {
      status = mbr_reson7kr_wr_depth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_SoundVelocityProfile) {
      status = mbr_reson7kr_wr_svp(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_CTD) {
      status = mbr_reson7kr_wr_ctd(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Geodesy) {
      status = mbr_reson7kr_wr_geodesy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_RollPitchHeave) {
      status = mbr_reson7kr_wr_rollpitchheave(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Heading) {
      status = mbr_reson7kr_wr_heading(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_SurveyLine) {
      status = mbr_reson7kr_wr_surveyline(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Navigation) {
      status = mbr_reson7kr_wr_navigation(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Attitude) {
      status = mbr_reson7kr_wr_attitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Rec1022) {
      status = mbr_reson7kr_wr_rec1022(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_FSDWsidescan && store->sstype == R7KRECID_FSDWsidescanLo) {
      status = mbr_reson7kr_wr_fsdwsslo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_FSDWsidescan && store->sstype == R7KRECID_FSDWsidescanHi) {
      status = mbr_reson7kr_wr_fsdwsshi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_FSDWsubbottom) {
      status = mbr_reson7kr_wr_fsdwsb(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_Bluefin) {
      status = mbr_reson7kr_wr_bluefin(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_ProcessedSidescan) {
      status = mbr_reson7kr_wr_processedsidescan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kVolatileSonarSettings) {
      status = mbr_reson7kr_wr_volatilesonarsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kConfiguration) {
      status = mbr_reson7kr_wr_configuration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kV2FirmwareHardwareConfiguration) {
      status = mbr_reson7kr_wr_v2firmwarehardwareconfiguration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kCalibrationData) {
      status = mbr_reson7kr_wr_calibration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kV2BITEData) {
      status = mbr_reson7kr_wr_v2bite(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kV27kCenterVersion) {
      status = mbr_reson7kr_wr_v27kcenterversion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kV28kWetEndVersion) {
      status = mbr_reson7kr_wr_v28kwetendversion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kInstallationParameters) {
      status = mbr_reson7kr_wr_installation(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kSystemEventMessage) {
      status = mbr_reson7kr_wr_systemeventmessage(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kRemoteControlSonarSettings) {
      status = mbr_reson7kr_wr_remotecontrolsettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kReserved) {
      status = mbr_reson7kr_wr_reserved(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kRoll) {
      status = mbr_reson7kr_wr_roll(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kPitch) {
      status = mbr_reson7kr_wr_pitch(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kSoundVelocity) {
      status = mbr_reson7kr_wr_soundvelocity(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kAbsorptionLoss) {
      status = mbr_reson7kr_wr_absorptionloss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
    else if (store->type == R7KRECID_7kSpreadingLoss) {
      status = mbr_reson7kr_wr_spreadingloss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    }
        else if (store->type == R7KRECID_None) {
            /* do nothing, including do not set an error */
        }
    else {
      fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_KIND;
    }

    /* finally write the record to the output file */
    if (status == MB_SUCCESS) {
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }
  }

#ifdef MBR_RESON7KR_DEBUG2
  fprintf(stderr, "RESON7KR DATA WRITTEN: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_reson7kr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k_struct *store = (struct mbsys_reson7k_struct *)store_ptr;

  /* write next data to file */
  const int status = mbr_reson7kr_wr_data(verbose, mb_io_ptr, store, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  return (status);
}

/*--------------------------------------------------------------------*/
int mbr_register_reson7kr(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_reson7kr(
      verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
      mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
      &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
      &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
      &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_reson7kr;
  mb_io_ptr->mb_io_format_free = &mbr_dem_reson7kr;
  mb_io_ptr->mb_io_store_alloc = &mbsys_reson7k_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_reson7k_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_reson7kr;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_reson7kr;
  mb_io_ptr->mb_io_dimensions = &mbsys_reson7k_dimensions;
  mb_io_ptr->mb_io_pingnumber = &mbsys_reson7k_pingnumber;
  mb_io_ptr->mb_io_sonartype = &mbsys_reson7k_sonartype;
  mb_io_ptr->mb_io_sidescantype = &mbsys_reson7k_sidescantype;
  mb_io_ptr->mb_io_preprocess = &mbsys_reson7k_preprocess;
  mb_io_ptr->mb_io_extract_platform = &mbsys_reson7k_extract_platform;
  mb_io_ptr->mb_io_extract = &mbsys_reson7k_extract;
  mb_io_ptr->mb_io_insert = &mbsys_reson7k_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_reson7k_extract_nav;
  mb_io_ptr->mb_io_extract_nnav = &mbsys_reson7k_extract_nnav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_reson7k_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_reson7k_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_reson7k_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_reson7k_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_reson7k_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_reson7k_detects;
  mb_io_ptr->mb_io_gains = &mbsys_reson7k_gains;
  mb_io_ptr->mb_io_copyrecord = &mbsys_reson7k_copy;
  mb_io_ptr->mb_io_makess = &mbsys_reson7k_makess;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;
  mb_io_ptr->mb_io_extract_segytraceheader = &mbsys_reson7k_extract_segytraceheader;
  mb_io_ptr->mb_io_extract_segy = &mbsys_reson7k_extract_segy;
  mb_io_ptr->mb_io_insert_segy = &mbsys_reson7k_insert_segy;
  mb_io_ptr->mb_io_ctd = &mbsys_reson7k_ctd;
  mb_io_ptr->mb_io_ancilliarysensor = &mbsys_reson7k_ancilliarysensor;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       system:             %d\n", mb_io_ptr->system);
    fprintf(stderr, "dbg2       beams_bath_max:     %d\n", mb_io_ptr->beams_bath_max);
    fprintf(stderr, "dbg2       beams_amp_max:      %d\n", mb_io_ptr->beams_amp_max);
    fprintf(stderr, "dbg2       pixels_ss_max:      %d\n", mb_io_ptr->pixels_ss_max);
    fprintf(stderr, "dbg2       format_name:        %s\n", mb_io_ptr->format_name);
    fprintf(stderr, "dbg2       system_name:        %s\n", mb_io_ptr->system_name);
    fprintf(stderr, "dbg2       format_description: %s\n", mb_io_ptr->format_description);
    fprintf(stderr, "dbg2       numfile:            %d\n", mb_io_ptr->numfile);
    fprintf(stderr, "dbg2       filetype:           %d\n", mb_io_ptr->filetype);
    fprintf(stderr, "dbg2       variable_beams:     %d\n", mb_io_ptr->variable_beams);
    fprintf(stderr, "dbg2       traveltime:         %d\n", mb_io_ptr->traveltime);
    fprintf(stderr, "dbg2       beam_flagging:      %d\n", mb_io_ptr->beam_flagging);
    fprintf(stderr, "dbg2       platform_source:    %d\n", mb_io_ptr->platform_source);
    fprintf(stderr, "dbg2       nav_source:         %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       sensordepth_source: %d\n", mb_io_ptr->nav_source);
    fprintf(stderr, "dbg2       heading_source:     %d\n", mb_io_ptr->heading_source);
    fprintf(stderr, "dbg2       attitude_source:    %d\n", mb_io_ptr->attitude_source);
    fprintf(stderr, "dbg2       svp_source:         %d\n", mb_io_ptr->svp_source);
    fprintf(stderr, "dbg2       beamwidth_xtrack:   %f\n", mb_io_ptr->beamwidth_xtrack);
    fprintf(stderr, "dbg2       beamwidth_ltrack:   %f\n", mb_io_ptr->beamwidth_ltrack);
    fprintf(stderr, "dbg2       format_alloc:       %p\n", (void *)mb_io_ptr->mb_io_format_alloc);
    fprintf(stderr, "dbg2       format_free:        %p\n", (void *)mb_io_ptr->mb_io_format_free);
    fprintf(stderr, "dbg2       store_alloc:        %p\n", (void *)mb_io_ptr->mb_io_store_alloc);
    fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
    fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
    fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
    fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
    fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
    fprintf(stderr, "dbg2       extract_nav:        %p\n", (void *)mb_io_ptr->mb_io_extract_nav);
    fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
    fprintf(stderr, "dbg2       extract_altitude:   %p\n", (void *)mb_io_ptr->mb_io_extract_altitude);
    fprintf(stderr, "dbg2       insert_altitude:    %p\n", (void *)mb_io_ptr->mb_io_insert_altitude);
    fprintf(stderr, "dbg2       extract_svp:        %p\n", (void *)mb_io_ptr->mb_io_extract_svp);
    fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
    fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
    fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
    fprintf(stderr, "dbg2       makess:             %p\n", (void *)mb_io_ptr->mb_io_makess);
    fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       extract_segytraceheader: %p\n", (void *)mb_io_ptr->mb_io_extract_segytraceheader);
    fprintf(stderr, "dbg2       extract_segy:       %p\n", (void *)mb_io_ptr->mb_io_extract_segy);
    fprintf(stderr, "dbg2       insert_segy:        %p\n", (void *)mb_io_ptr->mb_io_insert_segy);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
