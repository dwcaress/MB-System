/*--------------------------------------------------------------------
 *    The MB-system:  mbr_3dwisslp.c  2/11/93
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
 * mbr_3dwisslp.c contains the functions for reading and writing
 * multibeam data in the MBF_3DWISSLP format.
 * These functions include:
 *   mbr_alm_3dwisslp  - allocate read/write memory
 *   mbr_dem_3dwisslp  - deallocate read/write memory
 *   mbr_rt_3dwisslp  - read and translate data
 *   mbr_wt_3dwisslp  - translate and write data
 *
 * Author:  D. W. Caress
 * Date:  December 27, 2013
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_3ddwissl.h"

/*#define MBF_3DWISSLP_DEBUG 1 */

/*--------------------------------------------------------------------*/
int mbr_info_3dwisslp
(
  int verbose,
  int *system,
  int *beams_bath_max,
  int *beams_amp_max,
  int *pixels_ss_max,
  char *format_name,
  char *system_name,
  char *format_description,
  int *numfile,
  int *filetype,
  int *variable_beams,
  int *traveltime,
  int *beam_flagging,
  int *platform_source,
  int *nav_source,
  int *sensordepth_source,
  int *heading_source,
  int *attitude_source,
  int *svp_source,
  double *beamwidth_xtrack,
  double *beamwidth_ltrack,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* set format info parameters */
  *error = MB_ERROR_NO_ERROR;
  *system = MB_SYS_3DDWISSL;
  *beams_bath_max = 0;
  *beams_amp_max = 0;
  *pixels_ss_max = 0;
  strncpy(format_name, "3DWISSLP", MB_NAME_LENGTH);
  strncpy(system_name, "3DWISSLP", MB_NAME_LENGTH);
  strncpy(format_description,
    "Format name:          MBF_3DWISSLP\nInformal Description: 3D at Depth " "Wide Swath Subsea Lidar (WiSSL) processing format\n" "           Attributes: 3D at Depth lidar, variable pulses, bathymetry and amplitude, \n" "                      binary, MBARI.\n",
    MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_NORMAL;
  *variable_beams = true;
  *traveltime = false;
  *beam_flagging = true;
  *platform_source = MB_DATA_NONE;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_DATA;
  *svp_source = MB_DATA_NONE;
  *beamwidth_xtrack = 0.02;
  *beamwidth_ltrack = 0.02;

  const int status = MB_SUCCESS;

  if (verbose >= 2)
    {
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

  return status;
} /* mbr_info_3dwisslp */
/*--------------------------------------------------------------------*/
int mbr_alm_3dwisslp
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  const int status = mbsys_3ddwissl_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* get pointer to mbio descriptor */
  mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set file header read flag
     TODO(schwehr): Why is file_header_readwritten immediately overwritten?*/
  int *file_header_readwritten = (int *)&mb_io_ptr->save1;
  *file_header_readwritten = MB_NO;

  /* set saved bytes flag */
  mb_io_ptr->save2 = false;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_alm_3dwisslp */
/*--------------------------------------------------------------------*/
int mbr_dem_3dwisslp
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  /* deallocate reading/writing buffer */
  if ((mb_io_ptr->data_structure_size > 0) && (mb_io_ptr->raw_data != NULL))
    {
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
    mb_io_ptr->raw_data = NULL;
    mb_io_ptr->data_structure_size = 0;
    }

  /* deallocate memory  */
  status = mbsys_3ddwissl_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_dem_3dwisslp */
/*--------------------------------------------------------------------*/
int mbr_3dwisslp_rd_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl_struct *store = (struct mbsys_3ddwissl_struct *)store_ptr;

  /* get saved values */
  int *file_header_readwritten = (int *)&mb_io_ptr->save1;

  /* set file position */
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  /* set status */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  size_t index;  // TODO(schwehr): Localize index to each block.
  size_t read_len;

  /* if first read then read the fileheader, which is returned as a parameter record */
  if (*file_header_readwritten == MB_NO)
    {
    /* calculate size of file header and allocate read buffer */
    /* - the size of V1S1 and V1S3 parameter records is the same at 450 bytes */
    read_len =
      (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
    if (mb_io_ptr->data_structure_size < read_len)
      {
      status =
        mb_reallocd(verbose,
        __FILE__,
        __LINE__,
        read_len,
        (void **)(&mb_io_ptr->raw_data),
        error);
      if (status == MB_SUCCESS)
        mb_io_ptr->data_structure_size = read_len;
      }

    /* read file header and check the first two bytes */
    char *buffer = mb_io_ptr->raw_data;
    read_len = (size_t)MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE;
    if (status == MB_SUCCESS)
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
    if (status == MB_SUCCESS)
      {
      index = 0;
      mb_get_binary_short(true, (void *)&buffer[index], &(store->parameter_id)); index += 2;
      mb_get_binary_short(true, (void *)&buffer[index], &(store->magic_number)); index += 2;

      /* if ok and parameter_id is for the fileheader and the magic number is correct
       * then parse the rest of the file header */
      if (( store->parameter_id == MBSYS_3DDWISSL_RECORD_FILEHEADER) &&
        ( store->magic_number == MBF_3DWISSLP_MAGICNUMBER) )
        {
        /* set read flag */
        *file_header_readwritten = MB_YES;

        /* get scan information */
        mb_get_binary_short(true, (void *)&buffer[index], &(store->file_version));
        index += 2;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->sub_version));
        index += 2;
        mb_get_binary_float(true,
          (void *)&buffer[index],
          &(store->cross_track_angle_start)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(store->cross_track_angle_end)); index += 4;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->pulses_per_scan));
        index += 2;
        store->soundings_per_pulse = buffer[index]; index += 1;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->heada_scans_per_file));
        index += 2;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->headb_scans_per_file));
        // index += 2;

        /* calculate size of a processed scan record and allocate read buffer and pulses
           array */
        if (( store->file_version == 1) && ( store->sub_version == 1) )
          store->size_pulse_record_raw= MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE+
            store->pulses_per_scan*
            (MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE+ store->soundings_per_pulse *
            MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE);
        else if (( store->file_version == 1) && ( store->sub_version == 2) )
          store->size_pulse_record_raw= MBSYS_3DDWISSL_V1S2_RAW_SCAN_HEADER_SIZE+
            store->pulses_per_scan*
            (MBSYS_3DDWISSL_V1S2_RAW_PULSE_HEADER_SIZE+ store->soundings_per_pulse *
            MBSYS_3DDWISSL_V1S2_RAW_SOUNDING_SIZE);
        else/*if (store->file_version == 1 && store->sub_version == 3)*/
          store->size_pulse_record_raw= MBSYS_3DDWISSL_V1S3_RAW_SCAN_HEADER_SIZE+
            store->pulses_per_scan*
            (MBSYS_3DDWISSL_V1S3_RAW_PULSE_HEADER_SIZE+ store->soundings_per_pulse *
            MBSYS_3DDWISSL_V1S3_RAW_SOUNDING_SIZE);
        if (( store->file_version == 1) && ( store->sub_version == 1) )
          store->size_pulse_record_processed= MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE+
            store->pulses_per_scan*
            (MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE+ store->soundings_per_pulse *
            MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE);
        else/*if (store->file_version == 1 && store->sub_version == 3)*/
          store->size_pulse_record_processed= MBSYS_3DDWISSL_V1S3_PRO_SCAN_HEADER_SIZE+
            store->pulses_per_scan*
            (MBSYS_3DDWISSL_V1S3_PRO_PULSE_HEADER_SIZE+ store->soundings_per_pulse *
            MBSYS_3DDWISSL_V1S3_PRO_SOUNDING_SIZE);
        if (mb_io_ptr->data_structure_size < store->size_pulse_record_processed)
          {
          status = mb_reallocd(verbose,
            __FILE__,
            __LINE__,
            store->size_pulse_record_processed,
            (void **)(&mb_io_ptr->raw_data),
            error);
          if (status == MB_SUCCESS)
            {
            mb_io_ptr->data_structure_size = store->size_pulse_record_processed;
            buffer = mb_io_ptr->raw_data;
            }
          }
        if (store->num_pulses_alloc < store->pulses_per_scan)
          {
          read_len = store->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct);
          status =
            mb_reallocd(verbose,
            __FILE__,
            __LINE__,
            read_len,
            (void **)(&store->pulses),
            error);
          if (status == MB_SUCCESS)
            store->num_pulses_alloc = store->pulses_per_scan;
          }
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d INDEX at end of scan information: %zu  size_pulse_record_raw:%d size_pulse_record_processed:%d data_structure_size:%d\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          index,
          store->size_pulse_record_raw,
          store->size_pulse_record_processed,
          mb_io_ptr->data_structure_size);
        fprintf(stderr,
          "FILE_VERSION:%d FILE_SUBVERSION:%d\n",
          store->file_version,
          store->sub_version);
#endif

        /* set the WiSSL two optical head geometry using predefined values */
        store->heada_offset_x_m = MBSYS_3DDWISSL_HEADA_OFFSET_X_M;
        store->heada_offset_y_m = MBSYS_3DDWISSL_HEADA_OFFSET_Y_M;
        store->heada_offset_z_m = MBSYS_3DDWISSL_HEADA_OFFSET_Z_M;
        store->heada_offset_heading_deg = MBSYS_3DDWISSL_HEADA_OFFSET_HEADING_DEG;
        store->heada_offset_roll_deg = MBSYS_3DDWISSL_HEADA_OFFSET_ROLL_DEG;
        store->heada_offset_pitch_deg = MBSYS_3DDWISSL_HEADA_OFFSET_PITCH_DEG;
        store->headb_offset_x_m = MBSYS_3DDWISSL_HEADB_OFFSET_X_M;
        store->headb_offset_y_m = MBSYS_3DDWISSL_HEADB_OFFSET_Y_M;
        store->headb_offset_z_m = MBSYS_3DDWISSL_HEADB_OFFSET_Z_M;
        store->headb_offset_heading_deg = MBSYS_3DDWISSL_HEADB_OFFSET_HEADING_DEG;
        store->headb_offset_roll_deg = MBSYS_3DDWISSL_HEADB_OFFSET_ROLL_DEG;
        store->headb_offset_pitch_deg = MBSYS_3DDWISSL_HEADB_OFFSET_PITCH_DEG;
        }
      }

    /* now read the calibration information */

    /* format V1S1 has a 450-byte calibration structure */
    if (( status == MB_SUCCESS) && ( store->file_version == 1) && ( store->sub_version == 1) )
      {
      /* get calibration information for head a */
      read_len = (size_t)(2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
      if (status == MB_SUCCESS)
        {
        index = 0;

        /* get calibration information for head a */
        struct mbsys_3ddwissl_calibration_v1s1_struct *calibration_v1s1 =
          &store->calibration_v1s1_a;
        memcpy(calibration_v1s1->cfg_path, &buffer[index], 64); index +=64;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s1->laser_head_no)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s1->process_for_air)); index += 4;
        calibration_v1s1->temperature_compensation = buffer[index]; index += 1;
        calibration_v1s1->emergency_shutdown = buffer[index]; index += 1;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ocb_temperature_limit_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ocb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_temperature_limit_1_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_temperature_limit_2_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->dig_temperature_limit_c)); index += 4;
        memcpy(calibration_v1s1->l_d_cable_set, &buffer[index], 24); index +=24;
        memcpy(calibration_v1s1->ocb_comm_port, &buffer[index], 24); index += 24;
        memcpy(calibration_v1s1->ocb_comm_cfg, &buffer[index], 24); index += 24;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ao_deg_to_volt)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ai_neg_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ai_pos_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s1->t1_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s1->ff_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g300)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly)); index += 8;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->laser_start_time_sec)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->scanner_shift_cts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_lrg_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_med_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_sml_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->el_angle_fixed_deg)); index += 4;
        memcpy(calibration_v1s1->unused, &buffer[index], 116); index += 116;
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d INDEX at end of calibration a: %zu\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          index);
#endif
        /* get calibration information for head b */
        calibration_v1s1 = &store->calibration_v1s1_b;
        memcpy(calibration_v1s1->cfg_path, &buffer[index], 64); index +=64;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s1->laser_head_no)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s1->process_for_air)); index += 4;
        calibration_v1s1->temperature_compensation = buffer[index]; index += 1;
        calibration_v1s1->emergency_shutdown = buffer[index]; index += 1;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ocb_temperature_limit_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ocb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_temperature_limit_1_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_temperature_limit_2_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->pb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->dig_temperature_limit_c)); index += 4;
        memcpy(calibration_v1s1->l_d_cable_set, &buffer[index], 24); index +=24;
        memcpy(calibration_v1s1->ocb_comm_port, &buffer[index], 24); index += 24;
        memcpy(calibration_v1s1->ocb_comm_cfg, &buffer[index], 24); index += 24;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ao_deg_to_volt)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ai_neg_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->az_ai_pos_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s1->t1_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s1->ff_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->t1_water_secondary_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->ff_water_secondary_g300)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s1->temp_comp_poly)); index += 8;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->laser_start_time_sec)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->scanner_shift_cts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_lrg_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_med_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->factory_scanner_sml_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s1->el_angle_fixed_deg)); index += 4;
        memcpy(calibration_v1s1->unused, &buffer[index], 116); // index += 116;
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d INDEX at end of calibration b: %zu\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          index);
#endif
        }
      }

    /* format V1S3 has a different 450-byte calibration structure */
    /* - this is the same as the raw format V1S2 except that 43 unused bytes have */
    /*   been added to match the V1S1 calibration structure size */
    else if (( status == MB_SUCCESS) && ( store->file_version == 1) &&
      (( store->sub_version == 2) || ( store->sub_version == 3) ) )
      {
      /* get calibration information for head a */
      read_len = (size_t)(2 * MBSYS_3DDWISSL_V1S3_CALIBRATION_SIZE);
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
      if (status == MB_SUCCESS)
        {
        index = 0;

        /* get calibration information for head a */
        struct mbsys_3ddwissl_calibration_v1s3_struct *calibration_v1s3=
          &store->calibration_v1s3_a;
        memcpy(calibration_v1s3->cfg_path, &buffer[index], 64); index +=64;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->laser_head_no)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->process_for_air)); index += 4;
        calibration_v1s3->temperature_compensation = buffer[index]; index += 1;
        calibration_v1s3->emergency_shutdown = buffer[index]; index += 1;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ocb_temperature_limit_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ocb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_temperature_limit_1_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_temperature_limit_2_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->dig_temperature_limit_c)); index += 4;
        memcpy(calibration_v1s3->ocb_comm_port, &buffer[index], 24); index += 24;
        memcpy(calibration_v1s3->ocb_comm_cfg, &buffer[index], 24); index += 24;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ao_deg_to_volt)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ai_neg_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ai_pos_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s3->t1_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s3->ff_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g300)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly)); index += 8;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->laser_start_time_sec)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_shift_cts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_lrg_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_med_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_sml_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_dig_cnt_to_volts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->el_angle_fixed_deg)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->zda_to_pps_max_msec)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->zda_udp_port)); index += 4;
        calibration_v1s3->show_time_sync_errors = buffer[index]; index += 1;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->min_time_diff_update_msec)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->ctd_tcp_port)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->trigger_level_volt)); index += 8;
        mb_get_binary_int(true,
          (void *)&buffer[index],
          &(calibration_v1s3->mf_t0_position)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->mf_start_proc)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->dig_ref_pos_t0_cnts)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index], &(calibration_v1s3->dummy));
        index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->t0_min_height_raw_cts)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_0)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_3)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_4)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_5)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_0)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_3)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_4)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_5)); index += 8;
        if (( store->file_version == 1) && ( store->sub_version == 3) )
          {
          mb_get_binary_short(true, (void *)&buffer[index],
            &(calibration_v1s3->trigger_coupling_type)); index += 2;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(calibration_v1s3->digitizer_voltage_range_v)); index += 4;
          mb_get_binary_int(true, (void *)&buffer[index],
            &(calibration_v1s3->prf_tune_wait_ms)); index += 4;
          memcpy(calibration_v1s3->unused, &buffer[index], 33); index += 33;
          }
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d INDEX at end of calibration a: %zu\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          index);
#endif

        /* get calibration information for head b */
        calibration_v1s3 = &store->calibration_v1s3_b;
        memcpy(calibration_v1s3->cfg_path, &buffer[index], 64); index +=64;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->laser_head_no)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->process_for_air)); index += 4;
        calibration_v1s3->temperature_compensation = buffer[index]; index += 1;
        calibration_v1s3->emergency_shutdown = buffer[index]; index += 1;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ocb_temperature_limit_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ocb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_temperature_limit_1_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_temperature_limit_2_c)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->pb_humidity_limit)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->dig_temperature_limit_c)); index += 4;
        memcpy(calibration_v1s3->ocb_comm_port, &buffer[index], 24); index += 24;
        memcpy(calibration_v1s3->ocb_comm_cfg, &buffer[index], 24); index += 24;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ao_deg_to_volt)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ai_neg_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->az_ai_pos_v_to_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s3->t1_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(calibration_v1s3->ff_air));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g4000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g3000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g2000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g1000)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g400)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->t1_water_g300)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->ff_water_g300)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->temp_comp_poly)); index += 8;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->laser_start_time_sec)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_shift_cts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_lrg_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_med_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_scanner_sml_deg)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->factory_dig_cnt_to_volts)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(calibration_v1s3->el_angle_fixed_deg)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->zda_to_pps_max_msec)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->zda_udp_port)); index += 4;
        calibration_v1s3->show_time_sync_errors = buffer[index]; index += 1;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->min_time_diff_update_msec)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->ctd_tcp_port)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->trigger_level_volt)); index += 8;
        mb_get_binary_int(true,
          (void *)&buffer[index],
          &(calibration_v1s3->mf_t0_position)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->mf_start_proc)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->dig_ref_pos_t0_cnts)); index += 4;
        mb_get_binary_int(true, (void *)&buffer[index], &(calibration_v1s3->dummy));
        index += 4;
        mb_get_binary_int(true, (void *)&buffer[index],
          &(calibration_v1s3->t0_min_height_raw_cts)); index += 4;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_0)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_3)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_4)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_neg_polynom_5)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_0)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_1)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_2)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_3)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_4)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index],
          &(calibration_v1s3->scanner_pos_polynom_5)); index += 8;
        if (( store->file_version == 1) && ( store->sub_version == 3) )
          {
          mb_get_binary_short(true, (void *)&buffer[index],
            &(calibration_v1s3->trigger_coupling_type)); index += 2;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(calibration_v1s3->digitizer_voltage_range_v)); index += 4;
          mb_get_binary_int(true, (void *)&buffer[index],
            &(calibration_v1s3->prf_tune_wait_ms)); index += 4;
          memcpy(calibration_v1s3->unused, &buffer[index], 33); // index += 33;
          }
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d INDEX at end of calibration b: %zu\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          index);
#endif
        }
      }

    if (status == MB_SUCCESS)
      {
      store->kind = MB_DATA_PARAMETER;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d File header read, location in file: %ld\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        ftell(mb_io_ptr->mbfp));
      if (( store->file_version == 1) && ( store->sub_version == 1) )
        fprintf(stderr,
          "SCAN_HEADER_SIZE:%d pulses_per_scan:%d PULSE_HEADER_SIZE:%d soundings_per_pulse:%d SOUNDING_SIZE:%d\n",
          MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE,
          store->pulses_per_scan,
          MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE,
          store->soundings_per_pulse,
          MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE);
      else if (( store->file_version == 1) && ( store->sub_version == 2) )
        fprintf(stderr,
          "SCAN_HEADER_SIZE:%d pulses_per_scan:%d PULSE_HEADER_SIZE:%d soundings_per_pulse:%d SOUNDING_SIZE:%d\n",
          MBSYS_3DDWISSL_V1S2_RAW_SCAN_HEADER_SIZE,
          store->pulses_per_scan,
          MBSYS_3DDWISSL_V1S2_RAW_PULSE_HEADER_SIZE,
          store->soundings_per_pulse,
          MBSYS_3DDWISSL_V1S2_RAW_SOUNDING_SIZE);
      else/* if (store->file_version == 1 && store->sub_version == 3) */
        fprintf(stderr,
          "SCAN_HEADER_SIZE:%d pulses_per_scan:%d PULSE_HEADER_SIZE:%d soundings_per_pulse:%d SOUNDING_SIZE:%d\n",
          MBSYS_3DDWISSL_V1S3_RAW_SCAN_HEADER_SIZE,
          store->pulses_per_scan,
          MBSYS_3DDWISSL_V1S3_RAW_PULSE_HEADER_SIZE,
          store->soundings_per_pulse,
          MBSYS_3DDWISSL_V1S3_RAW_SOUNDING_SIZE);
#endif
      }
    else
      {
      status = MB_FAILURE;
      *error = MB_ERROR_BAD_FORMAT;
      store->kind = MB_DATA_NONE;
      }
    }

  /* else read subsequent data records */
  else
    {
    /* read and check two bytes until a valid record_id is found */
    char *buffer = mb_io_ptr->raw_data;
    read_len = (size_t)sizeof(short);
    bool valid_id = false;
#ifdef MBF_3DWISSLP_DEBUG
    int skip = 0;
    fprintf(stderr,
      "%s:%s():%d About to read next record, location in file: %ld\n",
      __FILE__,
      __FUNCTION__,
      __LINE__,
      ftell(mb_io_ptr->mbfp));
#endif
    status = mb_fileio_get(verbose, mbio_ptr, (void *)buffer, &read_len, error);
    do
      {
      if (status == MB_SUCCESS)
        {
        memcpy(&store->record_id, buffer, sizeof(short));
        if (( store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA) ||
          ( store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADB) ||
          ( store->record_id == MBSYS_3DDWISSL_RECORD_COMMENT) )
          {
          valid_id = true;
          }
        else
          {
#ifdef MBF_3DWISSLP_DEBUG
          fprintf(stderr,
            "%s:%s():%d SKIP BAD RECORD ID: %x %x %x %d skip:%d valid_id:%d status:%d error:%d\n",
            __FILE__,
            __FUNCTION__,
            __LINE__,
            (mb_u_char)buffer[0],
            (mb_u_char)buffer[1],
            store->record_id,
            store->record_id,
            skip,
            valid_id,
            status,
            *error);
          skip++;
#endif
          buffer[0] = buffer[1];
          read_len = (size_t)sizeof(char);
          status =
            mb_fileio_get(verbose, mbio_ptr, (void *)&(buffer[1]), &read_len, error);
          }
        }
      else
        {
        store->record_id = 0;
        }
      }
    while (status == MB_SUCCESS && !valid_id);
#ifdef MBF_3DWISSLP_DEBUG
    fprintf(stderr,
      "%s:%s():%d RECORD ID: %x %d skip:%d valid_id:%d status:%d error:%d\n",
      __FILE__,
      __FUNCTION__,
      __LINE__,
      store->record_id,
      store->record_id,
      skip,
      valid_id,
      status,
      *error);
#endif

    /* read MBSYS_3DDWISSL_RECORD_PROHEADA or MBSYS_3DDWISSL_RECORD_PROHEADB record */
    if (( status == MB_SUCCESS) &&
      (( store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA) ||
      ( store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADB) ))
      {
#ifdef MBF_3DWISSLP_DEBUG
      if (store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA)
        fprintf(stderr,
          "%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_PROHEADA\n",
          __FILE__,
          __FUNCTION__,
          __LINE__);
      else
        fprintf(stderr,
          "%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_PROHEADB\n",
          __FILE__,
          __FUNCTION__,
          __LINE__);
#endif
      read_len = (size_t)sizeof(unsigned int);
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
      mb_get_binary_int(true, (void *)&buffer[0], &(store->scan_size));
      read_len = (size_t)(store->scan_size);
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "read_len:%zu last 8 bytes: %x %x %x %x %x %x %x %x\n",
        read_len,
        buffer[read_len-8],
        buffer[read_len-7],
        buffer[read_len-6],
        buffer[read_len-5],
        buffer[read_len-4],
        buffer[read_len-3],
        buffer[read_len-2],
        buffer[read_len-1]);
#endif
      if (status == MB_SUCCESS)
        {
        index = 0;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->year)); index += 2;
        store->month = buffer[index]; index += 1;
        store->day = buffer[index]; index += 1;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->jday)); index += 2;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->hour)); index += 2;
        store->minutes = buffer[index]; index += 1;
        store->seconds = buffer[index]; index += 1;
        mb_get_binary_int(true, (void *)&buffer[index], &(store->nanoseconds));
        index += 4;

#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "%s:%s():%d Time: %d %d %d %d %d %d %d %d\n",
          __FILE__,
          __FUNCTION__,
          __LINE__,
          store->year,
          store->month,
          store->day,
          store->jday,
          store->hour,
          store->minutes,
          store->seconds,
          store->nanoseconds);
#endif
        store->gain = buffer[index]; index += 1;
        store->unused = buffer[index]; index += 1;
        mb_get_binary_float(true, (void *)&buffer[index],
          &(store->digitizer_temperature)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->ctd_temperature));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->ctd_salinity));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->ctd_pressure));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->index)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->range_start));
        index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->range_end));
        index += 4;
        mb_get_binary_int(true, (void *)&buffer[index], &(store->pulse_count));
        index += 4;
#ifdef MBF_3DWISSLP_DEBUG
        fprintf(stderr,
          "read %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%9.9d pulse_count:%d\n",
          store->year,
          store->month,
          store->day,
          store->hour,
          store->minutes,
          store->seconds,
          store->nanoseconds,
          store->pulse_count);
#endif
        mb_get_binary_double(true, (void *)&buffer[index], &(store->time_d)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index], &(store->navlon)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index], &(store->navlat)); index += 8;
        mb_get_binary_double(true, (void *)&buffer[index], &(store->sensordepth));
        index += 8;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->speed)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->heading)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->roll)); index += 4;
        mb_get_binary_float(true, (void *)&buffer[index], &(store->pitch)); index += 4;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->validpulse_count));
        index += 2;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->validsounding_count));
        index += 2;

        /* initialize all of the pulses with zero values excepting for null beamflags */
        memset(store->pulses, 0,
          (size_t)(store->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct)));
        for (int ipulse=0; ipulse<store->pulses_per_scan; ipulse++)
          {
          struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
          for (int isounding = 0; isounding < store->soundings_per_pulse; isounding++)
            pulse->soundings[isounding].beamflag = MB_FLAG_NULL;
          }

        /* parse the list of pulses - note that for this format the list of valid soundings
           follows separately */
        for (int ivalidpulse=0; ivalidpulse<store->validpulse_count; ivalidpulse++)
          {
          unsigned short ushort_val = 0;
          mb_get_binary_short(true, (void *)&buffer[index], &ushort_val); index += 2;
          int ipulse = (int) ushort_val;
          struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->angle_az));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->angle_el));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->offset_az));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->offset_el));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->time_offset));
          index += 4;
          mb_get_binary_double(true, (void *)&buffer[index], &(pulse->time_d));
          index += 8;
          mb_get_binary_double(true,
            (void *)&buffer[index],
            &(pulse->acrosstrack_offset)); index += 8;
          mb_get_binary_double(true, (void *)&buffer[index],
            &(pulse->alongtrack_offset)); index += 8;
          mb_get_binary_double(true,
            (void *)&buffer[index],
            &(pulse->sensordepth_offset)); index += 8;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->heading_offset));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->roll_offset));
          index += 4;
          mb_get_binary_float(true, (void *)&buffer[index], &(pulse->pitch_offset));
          index += 4;
          }

        /* parse the list of valid soundings */
        for (int ivalidsounding = 0; ivalidsounding < store->validsounding_count;
          ivalidsounding++)
          {
          unsigned short ushort_val = 0;
          mb_get_binary_short(true, (void *)&buffer[index], &ushort_val); index += 2;
          int ipulse = (int) ushort_val;
          int isounding = (int) buffer[index]; index += 1;
          struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
          pulse->validsounding_count += 1;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(pulse->soundings[isounding].range)); index += 4;
          mb_get_binary_short(true, (void *)&buffer[index],
            &(pulse->soundings[isounding].amplitude)); index += 2;
          if (store->sub_version >= 2)
            {
            pulse->soundings[isounding].diagnostic = buffer[index]; index += 1;
            }
          else
            {
            pulse->soundings[isounding].diagnostic = 0;
            }
          pulse->soundings[isounding].beamflag = buffer[index]; index += 1;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(pulse->soundings[isounding].acrosstrack)); index += 4;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(pulse->soundings[isounding].alongtrack)); index += 4;
          mb_get_binary_float(true, (void *)&buffer[index],
            &(pulse->soundings[isounding].depth)); index += 4;
          }

        store->bathymetry_calculated = true;
        store->kind = MB_DATA_DATA;
        }
      }

    /* read comment record */
    else if (( status == MB_SUCCESS) && ( store->record_id == MBSYS_3DDWISSL_RECORD_COMMENT) )
      {
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_COMMENT\n",
        __FILE__,
        __FUNCTION__,
        __LINE__);
#endif
      read_len = (size_t)sizeof(short);
      buffer = mb_io_ptr->raw_data;
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
      if (status == MB_SUCCESS)
        {
        index = 0;
        mb_get_binary_short(true, (void *)&buffer[index], &(store->comment_len));
        // index += 2;
        read_len = (size_t)(MIN(store->comment_len, MB_COMMENT_MAXLINE-1));
        memset(store->comment, 0, MB_COMMENT_MAXLINE);
        status = mb_fileio_get(verbose, mbio_ptr, store->comment, &read_len, error);
        }
      if (status == MB_SUCCESS)
        store->kind = MB_DATA_COMMENT;
      }
    }
#ifdef MBF_3DWISSLP_DEBUG
  fprintf(stderr,
    "%s:%s():%d END of mbr_3dwisslp_rd_data: status:%d error:%d kind:%d\n",
    __FILE__,
    __FUNCTION__,
    __LINE__,
    status,
    *error,
    store->kind);
#endif

  /* print out status info */
  if (( verbose >= 3) && ( status == MB_SUCCESS) )
    mbsys_3ddwissl_print_store(verbose, store_ptr, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:     %d\n", status);
    }

  return status;
} /* mbr_3dwisslp_rd_data */
/*--------------------------------------------------------------------*/
int mbr_rt_3dwisslp
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* get pointers to mbio descriptor and data structure */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl_struct *store = (struct mbsys_3ddwissl_struct *)store_ptr;

  /* read next data from file */
  const int status = mbr_3dwisslp_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* if needed calculate bathymetry */
  if (( status == MB_SUCCESS) && ( store->kind == MB_DATA_DATA) &&
      (!store->bathymetry_calculated))
    mbsys_3ddwissl_calculatebathymetry(verbose,
      mbio_ptr,
      store_ptr,
      MBSYS_3DDWISSL_DEFAULT_AMPLITUDE_THRESHOLD,
      MBSYS_3DDWISSL_DEFAULT_TARGET_ALTITUDE,
      error);

  /* print out status info */
  if (verbose > 1)
    mbsys_3ddwissl_print_store(verbose, store_ptr, error);

  /* set error and kind in mb_io_ptr */
  mb_io_ptr->new_error = *error;
  mb_io_ptr->new_kind = store->kind;

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_rt_3dwisslp */
/*--------------------------------------------------------------------*/
int mbr_3dwisslp_wr_data
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_3ddwissl_struct *store = (struct mbsys_3ddwissl_struct *)store_ptr;

  /* get saved values */
  int *file_header_readwritten = (int *)&mb_io_ptr->save1;

  /* set file position */
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  if (verbose >= 4)
    {
    fprintf(stderr, "\ndbg4  Data record kind in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg4       kind:       %d\n", store->kind);
    }

  /* set status */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
#ifdef MBF_3DWISSLP_DEBUG
  fprintf(stderr, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__);
  fprintf(stderr,
    "mb_io_ptr->raw_data:%p mb_io_ptr->data_structure_size:%d\n",
    mb_io_ptr->raw_data,
    mb_io_ptr->data_structure_size);
#endif

  char *buffer;
  size_t write_len;
  size_t index;

  /* if first write then write the magic number file header */
  if (( store->kind == MB_DATA_PARAMETER) ||
    (( store->kind == MB_DATA_DATA) && ( *file_header_readwritten != MB_YES) ))
    {
    /* if comments have been written then reset file position to start of file */
    if (mb_io_ptr->file_pos > 0)
      fseek(mb_io_ptr->mbfp, 0, SEEK_SET);

    /* calculate maximum size of output lidar record and allocate write buffer to handle that */
    if (store->sub_version == 1)
      write_len =
        (size_t)MAX((MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE+ store->pulses_per_scan*
        (MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE+ store->soundings_per_pulse*
        MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE)),
        (MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE));
    else/* if (store->sub_version >= 2) */
      write_len =
        (size_t)MAX((MBSYS_3DDWISSL_V1S3_PRO_SCAN_HEADER_SIZE+ store->pulses_per_scan*
        (MBSYS_3DDWISSL_V1S3_PRO_PULSE_HEADER_SIZE+ store->soundings_per_pulse*
        MBSYS_3DDWISSL_V1S3_PRO_SOUNDING_SIZE)),
        (MBSYS_3DDWISSL_V1S3_PARAMETER_SIZE+ 2 * MBSYS_3DDWISSL_V1S3_CALIBRATION_SIZE));
    if (mb_io_ptr->data_structure_size < write_len)
      {
      status =
        mb_reallocd(verbose,
        __FILE__,
        __LINE__,
        write_len,
        (void **)(&mb_io_ptr->raw_data),
        error);
      if (status == MB_SUCCESS)
        mb_io_ptr->data_structure_size = write_len;
      }

    /* write file header which is also the parameter record */
    if (( status == MB_SUCCESS) && ( store->file_version == 1) && ( store->sub_version == 1) )
      {
      write_len =
        (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 *
        MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
      index = 0;
      buffer = mb_io_ptr->raw_data;

      /* start of parameter record (and file ) */
      store->parameter_id = MBSYS_3DDWISSL_RECORD_FILEHEADER;
      store->magic_number = MBF_3DWISSLP_MAGICNUMBER;
      mb_put_binary_short(true, store->parameter_id, (void **)&buffer[index]); index += 2;
      mb_put_binary_short(true, store->magic_number, (void **)&buffer[index]); index += 2;

      /* put scan information */
      mb_put_binary_short(true, store->file_version, (void **)&buffer[index]); index += 2;
      mb_put_binary_short(true, store->sub_version, (void **)&buffer[index]); index += 2;
      mb_put_binary_float(true, store->cross_track_angle_start, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, store->cross_track_angle_end, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_short(true, store->pulses_per_scan, (void **)&buffer[index]);
      index += 2;
      buffer[index] = store->soundings_per_pulse; index += 1;
      mb_put_binary_short(true, store->heada_scans_per_file, (void **)&buffer[index]);
      index += 2;
      mb_put_binary_short(true, store->headb_scans_per_file, (void **)&buffer[index]);
      index += 2;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of scan information: %zu  size_pulse_record_raw:%d size_pulse_record_processed:%d data_structure_size:%d\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index,
        store->size_pulse_record_raw,
        store->size_pulse_record_processed,
        mb_io_ptr->data_structure_size);
      fprintf(stderr,
        "    file_version:%d sub_version:%d pulses_per_scan:%d soundings_per_pulse:%d\n",
        store->file_version,
        store->sub_version,
        store->pulses_per_scan,
        store->soundings_per_pulse);
#endif

      /* put calibration information for head a */
      struct mbsys_3ddwissl_calibration_v1s1_struct *calibration_v1s1;
      calibration_v1s1 = &store->calibration_v1s1_a;
      memcpy((void **)&buffer[index], calibration_v1s1->cfg_path, 64); index +=64;
      mb_put_binary_int(true, calibration_v1s1->laser_head_no, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s1->process_for_air, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s1->temperature_compensation; index += 1;
      buffer[index] = calibration_v1s1->emergency_shutdown; index += 1;
      mb_put_binary_float(true, calibration_v1s1->ocb_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ocb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->pb_temperature_limit_1_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->pb_temperature_limit_2_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->pb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->dig_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s1->l_d_cable_set, 24); index +=24;
      memcpy((void **)&buffer[index], calibration_v1s1->ocb_comm_port, 24); index += 24;
      memcpy((void **)&buffer[index], calibration_v1s1->ocb_comm_cfg, 24); index += 24;
      mb_put_binary_float(true, calibration_v1s1->az_ao_deg_to_volt,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->az_ai_neg_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->az_ai_pos_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g4000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g4000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g3000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g3000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g2000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g2000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g1000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g1000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_secondary_g400,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_secondary_g400,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_secondary_g300,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_secondary_g300,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly, (void **)&buffer[index]);
      index += 8;
      mb_put_binary_float(true, calibration_v1s1->laser_start_time_sec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->scanner_shift_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_lrg_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_med_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_sml_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->el_angle_fixed_deg,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s1->unused, 116); index +=116;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of calibration a: %zu\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index);
#endif

      /* get calibration information for head b */
      calibration_v1s1 = &store->calibration_v1s1_b;
      memcpy((void **)&buffer[index], calibration_v1s1->cfg_path, 64); index +=64;
      mb_put_binary_int(true, calibration_v1s1->laser_head_no, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s1->process_for_air, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s1->temperature_compensation; index += 1;
      buffer[index] = calibration_v1s1->emergency_shutdown; index += 1;
      mb_put_binary_float(true, calibration_v1s1->ocb_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ocb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->pb_temperature_limit_1_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->pb_temperature_limit_2_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->pb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->dig_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s1->l_d_cable_set, 24); index +=24;
      memcpy((void **)&buffer[index], calibration_v1s1->ocb_comm_port, 24); index += 24;
      memcpy((void **)&buffer[index], calibration_v1s1->ocb_comm_cfg, 24); index += 24;
      mb_put_binary_float(true, calibration_v1s1->az_ao_deg_to_volt,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->az_ai_neg_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->az_ai_pos_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g4000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g4000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g3000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g3000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g2000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g2000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->t1_water_secondary_g1000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->ff_water_secondary_g1000,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_secondary_g400,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_secondary_g400,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->t1_water_secondary_g300,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->ff_water_secondary_g300,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s1->temp_comp_poly, (void **)&buffer[index]);
      index += 8;
      mb_put_binary_float(true, calibration_v1s1->laser_start_time_sec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->scanner_shift_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_lrg_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_med_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s1->factory_scanner_sml_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s1->el_angle_fixed_deg,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s1->unused, 116); // index +=116;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of calibration b: %zu\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index);
#endif

      /* write file header from buffer */
      status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d Wrote file header %zu bytes\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        write_len);
#endif

      /* reset file position to end of file in case comments have been written */
      fseek(mb_io_ptr->mbfp, 0, SEEK_END);

      *file_header_readwritten = MB_YES;
      }
    else if (( status == MB_SUCCESS) && ( store->file_version == 1) &&
      ( store->sub_version >= 2) )
      {
      write_len =
        (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 *
        MBSYS_3DDWISSL_V1S3_CALIBRATION_SIZE);
      index = 0;
      buffer = mb_io_ptr->raw_data;
      unsigned short sub_version = 3;

      /* start of parameter record (and file ) */
      store->parameter_id = MBSYS_3DDWISSL_RECORD_FILEHEADER;
      store->magic_number = MBF_3DWISSLP_MAGICNUMBER;
      mb_put_binary_short(true, store->parameter_id, (void **)&buffer[index]); index += 2;
      mb_put_binary_short(true, store->magic_number, (void **)&buffer[index]); index += 2;

      /* get scan information */
      mb_put_binary_short(true, store->file_version, (void **)&buffer[index]); index += 2;
      mb_put_binary_short(true, sub_version, (void **)&buffer[index]); index += 2;
      mb_put_binary_float(true, store->cross_track_angle_start, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, store->cross_track_angle_end, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_short(true, store->pulses_per_scan, (void **)&buffer[index]);
      index += 2;
      buffer[index] = store->soundings_per_pulse; index += 1;
      mb_put_binary_short(true, store->heada_scans_per_file, (void **)&buffer[index]);
      index += 2;
      mb_put_binary_short(true, store->headb_scans_per_file, (void **)&buffer[index]);
      index += 2;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of scan information: %zu  size_pulse_record_raw:%d size_pulse_record_processed:%d data_structure_size:%d\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index,
        store->size_pulse_record_raw,
        store->size_pulse_record_processed,
        mb_io_ptr->data_structure_size);
      fprintf(stderr,
        "    file_version:%d sub_version:%d pulses_per_scan:%d soundings_per_pulse:%d\n",
        store->file_version,
        store->sub_version,
        store->pulses_per_scan,
        store->soundings_per_pulse);
#endif

      /* get calibration information for head a */
      struct mbsys_3ddwissl_calibration_v1s3_struct *calibration_v1s3;
      calibration_v1s3 = &store->calibration_v1s3_a;
      memcpy((void **)&buffer[index], calibration_v1s3->cfg_path, 64); index += 64;
      mb_put_binary_int(true, calibration_v1s3->laser_head_no, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->process_for_air, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s3->temperature_compensation; index += 1;
      buffer[index] = calibration_v1s3->emergency_shutdown; index += 1;
      mb_put_binary_float(true, calibration_v1s3->ocb_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->ocb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->pb_temperature_limit_1_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->pb_temperature_limit_2_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->pb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->dig_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s3->ocb_comm_port, 24); index += 24;
      memcpy((void **)&buffer[index], calibration_v1s3->ocb_comm_cfg, 24); index += 24;
      mb_put_binary_float(true, calibration_v1s3->az_ao_deg_to_volt,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->az_ai_neg_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->az_ai_pos_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly, (void **)&buffer[index]);
      index += 8;
      mb_put_binary_float(true, calibration_v1s3->laser_start_time_sec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->scanner_shift_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_lrg_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_med_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_sml_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->factory_dig_cnt_to_volts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->el_angle_fixed_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->zda_to_pps_max_msec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->zda_udp_port, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s3->show_time_sync_errors; index += 1;
      mb_put_binary_int(true, calibration_v1s3->min_time_diff_update_msec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->ctd_tcp_port, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_double(true, calibration_v1s3->trigger_level_volt,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_int(true, calibration_v1s3->mf_t0_position, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->mf_start_proc, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->dig_ref_pos_t0_cnts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->dummy, (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->t0_min_height_raw_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_0,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_3,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_4,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_5,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_0,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_3,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_4,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_5,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_short(true, calibration_v1s3->trigger_coupling_type,
      (void **)&buffer[index]); index += 2;
      mb_put_binary_float(true, calibration_v1s3->digitizer_voltage_range_v,
      (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->prf_tune_wait_ms,
      (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s3->unused, 33); index +=33;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of calibration a: %zu\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index);
#endif

      /* get calibration information for head b */
      calibration_v1s3 = &store->calibration_v1s3_b;
      memcpy((void **)&buffer[index], calibration_v1s3->cfg_path, 64); index +=64;
      mb_put_binary_int(true, calibration_v1s3->laser_head_no, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->process_for_air, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s3->temperature_compensation; index += 1;
      buffer[index] = calibration_v1s3->emergency_shutdown; index += 1;
      mb_put_binary_float(true, calibration_v1s3->ocb_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->ocb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->pb_temperature_limit_1_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->pb_temperature_limit_2_c,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->pb_humidity_limit,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->dig_temperature_limit_c,
        (void **)&buffer[index]); index += 4;
      memcpy((void **)&buffer[index], calibration_v1s3->ocb_comm_port, 24); index += 24;
      memcpy((void **)&buffer[index], calibration_v1s3->ocb_comm_cfg, 24); index += 24;
      mb_put_binary_float(true, calibration_v1s3->az_ao_deg_to_volt,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->az_ai_neg_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->az_ai_pos_v_to_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_air, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g4000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g3000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g2000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g1000, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g400, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->t1_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_float(true, calibration_v1s3->ff_water_g300, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->temp_comp_poly, (void **)&buffer[index]);
      index += 8;
      mb_put_binary_float(true, calibration_v1s3->laser_start_time_sec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->scanner_shift_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_lrg_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_med_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true, calibration_v1s3->factory_scanner_sml_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->factory_dig_cnt_to_volts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_float(true,
        calibration_v1s3->el_angle_fixed_deg,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->zda_to_pps_max_msec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->zda_udp_port, (void **)&buffer[index]);
      index += 4;
      buffer[index] = calibration_v1s3->show_time_sync_errors; index += 1;
      mb_put_binary_int(true, calibration_v1s3->min_time_diff_update_msec,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->ctd_tcp_port, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_double(true, calibration_v1s3->trigger_level_volt,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_int(true, calibration_v1s3->mf_t0_position, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->mf_start_proc, (void **)&buffer[index]);
      index += 4;
      mb_put_binary_int(true, calibration_v1s3->dig_ref_pos_t0_cnts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->dummy, (void **)&buffer[index]); index += 4;
      mb_put_binary_int(true, calibration_v1s3->t0_min_height_raw_cts,
        (void **)&buffer[index]); index += 4;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_0,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_3,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_4,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_neg_polynom_5,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_0,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_1,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_2,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_3,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_4,
        (void **)&buffer[index]); index += 8;
      mb_put_binary_double(true, calibration_v1s3->scanner_pos_polynom_5,
        (void **)&buffer[index]); index += 8;
        mb_put_binary_short(true, calibration_v1s3->trigger_coupling_type,
        (void **)&buffer[index]); index += 2;
        mb_put_binary_float(true, calibration_v1s3->digitizer_voltage_range_v,
        (void **)&buffer[index]); index += 4;
        mb_put_binary_int(true, calibration_v1s3->prf_tune_wait_ms,
        (void **)&buffer[index]); index += 4;
        memcpy((void **)&buffer[index], calibration_v1s3->unused, 33); // index +=33;
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d INDEX at end of calibration b: %zu\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        index);
#endif

      /* write file header from buffer */
      status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d Wrote file header %zu bytes\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        write_len);
#endif

      /* reset file position to end of file in case comments have been written */
      fseek(mb_io_ptr->mbfp, 0, SEEK_END);

      *file_header_readwritten = MB_YES;
      }
    }

  /* write comment record */
  if (( status == MB_SUCCESS) && ( store->kind == MB_DATA_COMMENT) )
    {
    /* calculate size of output comment record and parameter record and
            allocate write buffer to handle the larger of the two */
    write_len =
      MAX(
      (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE),
      MB_COMMENT_MAXLINE + 4);
    if (mb_io_ptr->data_structure_size < write_len)
      {
      status =
        mb_reallocd(verbose,
        __FILE__,
        __LINE__,
        write_len,
        (void **)(&mb_io_ptr->raw_data),
        error);
      if (status == MB_SUCCESS)
        mb_io_ptr->data_structure_size = write_len;
      }

    /* write dummy file header / parameter record if one hasn't already been written */
    if (*file_header_readwritten == MB_NO)
      {
      /* calculate size of parameter record to be written here */
      write_len =
        (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE+ 2 *
        MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);

      /* write file header which is also the parameter record */
      index = 0;
      buffer = mb_io_ptr->raw_data;
      memset(buffer, 0, write_len);

      /* start of parameter record (and file ) */
      mb_put_binary_short(true, store->parameter_id, (void **)&buffer[index]); index += 2;
      mb_put_binary_short(true, store->magic_number, (void **)&buffer[index]); index += 2;

      /* write file header from buffer */
      status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
#ifdef MBF_3DWISSLP_DEBUG
      fprintf(stderr,
        "%s:%s():%d Wrote dummy file header %zu bytes\n",
        __FILE__,
        __FUNCTION__,
        __LINE__,
        write_len);
#endif

      /* reset file position to end of file in case comments have been written */
      fseek(mb_io_ptr->mbfp, 0, SEEK_END);

      *file_header_readwritten = MB_MAYBE;
      }

    /* encode the comment */
    index = 0;
    buffer = mb_io_ptr->raw_data;

    store->record_id = MBSYS_3DDWISSL_RECORD_COMMENT;
    store->comment_len = MIN(strlen(store->comment), MB_COMMENT_MAXLINE-1);
    mb_put_binary_short(true, store->record_id, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, store->comment_len, &buffer[index]);
    index += 2;
    memcpy(&buffer[index], store->comment, store->comment_len);
    index += store->comment_len;

    /* write comment record */
    write_len = (size_t)index;
    status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
#ifdef MBF_3DWISSLP_DEBUG
    fprintf(stderr,
      "%s:%s():%d Wrote comment %zu bytes\n",
      __FILE__,
      __FUNCTION__,
      __LINE__,
      write_len);
#endif
    }

  /* write LIDAR scan record */
  else if (( status == MB_SUCCESS) && ( store->kind == MB_DATA_DATA) )
    {
    /* count valid (non-null) pulses and soundings */
    store->validpulse_count = 0;
    store->validsounding_count = 0;
    for (unsigned int ipulse = 0; ipulse < store->pulse_count; ipulse++)
      {
      struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
      pulse->validsounding_count = 0;
      for (int isounding = 0; isounding < store->soundings_per_pulse; isounding++)
        if (pulse->soundings[isounding].beamflag != MB_FLAG_NULL)
          {
          pulse->validsounding_count++;
          store->validsounding_count++;
          }
      if (pulse->validsounding_count > 0)
        store->validpulse_count++;
      }

    /* calculate size of output lidar record and allocate write buffer to handle that */
    if (( store->file_version == 1) && ( store->sub_version == 1) )
      write_len =
        (size_t)(MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE+ store->validpulse_count*
        MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE+ store->validsounding_count*
        MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE);
    else/* if (store->file_version == 1 && store->sub_version >= 2) */
      write_len =
        (size_t)(MBSYS_3DDWISSL_V1S3_PRO_SCAN_HEADER_SIZE+ store->validpulse_count*
        MBSYS_3DDWISSL_V1S3_PRO_PULSE_HEADER_SIZE+ store->validsounding_count*
        MBSYS_3DDWISSL_V1S3_PRO_SOUNDING_SIZE);
    store->scan_size = write_len;
#ifdef MBF_3DWISSLP_DEBUG
    fprintf(stderr,
      "%s:%s():%d write_len %zu bytes from validpulse_count:%d validsoundingcount:%d\n",
      __FILE__,
      __FUNCTION__,
      __LINE__,
      write_len,
      store->validpulse_count,
      store->validsounding_count);
#endif

    /* encode the data */
    index = 0;
    buffer = mb_io_ptr->raw_data;

    if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA)
      store->record_id = MBSYS_3DDWISSL_RECORD_PROHEADA;
    if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADB)
      store->record_id = MBSYS_3DDWISSL_RECORD_PROHEADB;
    mb_put_binary_short(true, store->record_id, &buffer[index]); index += 2;
    mb_put_binary_int(true, store->scan_size, &buffer[index]); index += 4;
    mb_put_binary_short(true, store->year, &buffer[index]); index += 2;
    buffer[index] = (mb_u_char)store->month; index += 1;
    buffer[index] = (mb_u_char)store->day; index += 1;
    mb_put_binary_short(true, store->jday, &buffer[index]); index += 2;
    mb_put_binary_short(true, store->hour, &buffer[index]); index += 2;
    buffer[index] = (mb_u_char)store->minutes; index++;
    buffer[index] = (mb_u_char)store->seconds; index++;
    mb_put_binary_int(true, store->nanoseconds, &buffer[index]); index += 4;

    buffer[index] = store->gain; index += 1;
    buffer[index] = store->unused; index += 1;
    mb_put_binary_float(true, store->digitizer_temperature, (void **)&buffer[index]);
    index += 4;
    mb_put_binary_float(true, store->ctd_temperature, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->ctd_salinity, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->ctd_pressure, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->index, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->range_start, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->range_end, (void **)&buffer[index]); index += 4;
    mb_put_binary_int(true, store->pulse_count, (void **)&buffer[index]); index += 4;

    mb_put_binary_double(true, store->time_d, (void **)&buffer[index]); index += 8;
    mb_put_binary_double(true, store->navlon, (void **)&buffer[index]); index += 8;
    mb_put_binary_double(true, store->navlat, (void **)&buffer[index]); index += 8;
    mb_put_binary_double(true, store->sensordepth, (void **)&buffer[index]); index += 8;
    mb_put_binary_float(true, store->speed, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->heading, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->roll, (void **)&buffer[index]); index += 4;
    mb_put_binary_float(true, store->pitch, (void **)&buffer[index]); index += 4;
    mb_put_binary_short(true, store->validpulse_count, &buffer[index]); index += 2;
    mb_put_binary_short(true, store->validsounding_count, &buffer[index]); index += 2;

    /* write only the valid (non-null) scan pulses */
    for (int ipulse=0; ipulse<store->pulses_per_scan; ipulse++)
      {
      struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
      if (pulse->validsounding_count > 0)
        {
        mb_put_binary_short(true, (unsigned short)ipulse, &buffer[index]); index += 2;
        mb_put_binary_float(true, pulse->angle_az, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(true, pulse->angle_el, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(true, pulse->offset_az, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(true, pulse->offset_el, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(true, pulse->time_offset, (void **)&buffer[index]);
        index += 4;
        mb_put_binary_double(true, pulse->time_d, (void **)&buffer[index]); index += 8;
        mb_put_binary_double(true, pulse->acrosstrack_offset, (void **)&buffer[index]);
        index += 8;
        mb_put_binary_double(true, pulse->alongtrack_offset, (void **)&buffer[index]);
        index += 8;
        mb_put_binary_double(true, pulse->sensordepth_offset, (void **)&buffer[index]);
        index += 8;
        mb_put_binary_float(true, pulse->heading_offset, (void **)&buffer[index]);
        index += 4;
        mb_put_binary_float(true, pulse->roll_offset, (void **)&buffer[index]);
        index += 4;
        mb_put_binary_float(true, pulse->pitch_offset, (void **)&buffer[index]);
        index += 4;
        }
      }

    /* write only the valid (non-null) soundings */
    for (int ipulse=0; ipulse<store->pulses_per_scan; ipulse++)
      {
      struct mbsys_3ddwissl_pulse_struct *pulse = &store->pulses[ipulse];
      if (pulse->validsounding_count > 0)
        {
        for (int isounding=0; isounding<store->soundings_per_pulse; isounding++)
          if (pulse->soundings[isounding].beamflag != MB_FLAG_NULL)
            {
            mb_put_binary_short(true, (unsigned short)ipulse,
              (void **)&buffer[index]); index += 2;
            buffer[index] = (mb_u_char)isounding; index += 1;
            mb_put_binary_float(true,
              pulse->soundings[isounding].range,
              (void **)&buffer[index]); index += 4;
            mb_put_binary_short(true,
              pulse->soundings[isounding].amplitude,
              (void **)&buffer[index]); index += 2;
            if (( store->file_version == 1) && ( store->sub_version >= 2) )
              {
              buffer[index] = pulse->soundings[isounding].diagnostic; index += 1;
              }
            buffer[index] = pulse->soundings[isounding].beamflag; index += 1;
            mb_put_binary_float(true, pulse->soundings[isounding].acrosstrack,
              (void **)&buffer[index]); index += 4;
            mb_put_binary_float(true,
              pulse->soundings[isounding].alongtrack,
              (void **)&buffer[index]); index += 4;
            mb_put_binary_float(true,
              pulse->soundings[isounding].depth,
              (void **)&buffer[index]); index += 4;
            }
        }
      }

    /* write LIDAR scan record */
#ifdef MBF_3DWISSLP_DEBUG
    fprintf(stderr,
      "%s:%s():%d Writing MBF_3DWISSLP scan record %zu %zu bytes from buffer:%p  pulse_count:%d time_d:%f\n",
      __FILE__,
      __FUNCTION__,
      __LINE__,
      write_len,
      index,
      buffer,
      store->pulse_count,
      store->time_d);
#endif
    write_len = (size_t)index;
    status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
    }

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_3dwisslp_wr_data */
/*--------------------------------------------------------------------*/
int mbr_wt_3dwisslp
(
  int verbose,
  void *mbio_ptr,
  void *store_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* write next data to file */
  const int status = mbr_3dwisslp_wr_data(verbose, mbio_ptr, store_ptr, error);

  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  return status;
} /* mbr_wt_3dwisslp */
/*--------------------------------------------------------------------*/
int mbr_register_3dwisslp
(
  int verbose,
  void *mbio_ptr,
  int *error
)
{
  if (verbose >= 2)
    {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    }

  /* check for non-null structure pointers */
  assert(mbio_ptr != NULL);

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_3dwisslp(verbose,
    &mb_io_ptr->system,
    &mb_io_ptr->beams_bath_max,
    &mb_io_ptr->beams_amp_max,
    &mb_io_ptr->pixels_ss_max,
    mb_io_ptr->format_name,
    mb_io_ptr->system_name,
    mb_io_ptr->format_description,
    &mb_io_ptr->numfile,
    &mb_io_ptr->filetype,
    &mb_io_ptr->variable_beams,
    &mb_io_ptr->traveltime,
    &mb_io_ptr->beam_flagging,
    &mb_io_ptr->platform_source,
    &mb_io_ptr->nav_source,
    &mb_io_ptr->sensordepth_source,
    &mb_io_ptr->heading_source,
    &mb_io_ptr->attitude_source,
    &mb_io_ptr->svp_source,
    &mb_io_ptr->beamwidth_xtrack,
    &mb_io_ptr->beamwidth_ltrack,
    error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_3dwisslp;
  mb_io_ptr->mb_io_format_free = &mbr_dem_3dwisslp;
  mb_io_ptr->mb_io_store_alloc = &mbsys_3ddwissl_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_3ddwissl_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_3dwisslp;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_3dwisslp;
  mb_io_ptr->mb_io_dimensions = &mbsys_3ddwissl_dimensions;
  mb_io_ptr->mb_io_preprocess = &mbsys_3ddwissl_preprocess;
  mb_io_ptr->mb_io_sensorhead = &mbsys_3ddwissl_sensorhead;
  mb_io_ptr->mb_io_extract = &mbsys_3ddwissl_extract;
  mb_io_ptr->mb_io_insert = &mbsys_3ddwissl_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_3ddwissl_extract_nav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_3ddwissl_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_3ddwissl_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_3ddwissl_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_3ddwissl_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_3ddwissl_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_3ddwissl_detects;
  mb_io_ptr->mb_io_copyrecord = &mbsys_3ddwissl_copy;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;

  if (verbose >= 2)
    {
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
    fprintf(stderr, "dbg2       format_alloc:       %p\n",
      (void *)mb_io_ptr->mb_io_format_alloc);
    fprintf(stderr, "dbg2       format_free:        %p\n",
      (void *)mb_io_ptr->mb_io_format_free);
    fprintf(stderr, "dbg2       store_alloc:        %p\n",
      (void *)mb_io_ptr->mb_io_store_alloc);
    fprintf(stderr, "dbg2       store_free:         %p\n", (void *)mb_io_ptr->mb_io_store_free);
    fprintf(stderr, "dbg2       read_ping:          %p\n", (void *)mb_io_ptr->mb_io_read_ping);
    fprintf(stderr, "dbg2       write_ping:         %p\n", (void *)mb_io_ptr->mb_io_write_ping);
    fprintf(stderr, "dbg2       preprocess:         %p\n", (void *)mb_io_ptr->mb_io_preprocess);
    fprintf(stderr, "dbg2       extract:            %p\n", (void *)mb_io_ptr->mb_io_extract);
    fprintf(stderr, "dbg2       insert:             %p\n", (void *)mb_io_ptr->mb_io_insert);
    fprintf(stderr, "dbg2       extract_nav:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_nav);
    fprintf(stderr, "dbg2       insert_nav:         %p\n", (void *)mb_io_ptr->mb_io_insert_nav);
    fprintf(stderr,
      "dbg2       extract_altitude:   %p\n",
      (void *)mb_io_ptr->mb_io_extract_altitude);
    fprintf(stderr,
      "dbg2       insert_altitude:    %p\n",
      (void *)mb_io_ptr->mb_io_insert_altitude);
    fprintf(stderr, "dbg2       extract_svp:        %p\n",
      (void *)mb_io_ptr->mb_io_extract_svp);
    fprintf(stderr, "dbg2       insert_svp:         %p\n", (void *)mb_io_ptr->mb_io_insert_svp);
    fprintf(stderr, "dbg2       ttimes:             %p\n", (void *)mb_io_ptr->mb_io_ttimes);
    fprintf(stderr, "dbg2       detects:            %p\n", (void *)mb_io_ptr->mb_io_detects);
    fprintf(stderr,
      "dbg2       extract_rawss:      %p\n",
      (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n",
      (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
    }

  return status;
} /* mbr_register_3dwisslp */
/*--------------------------------------------------------------------*/
