/*--------------------------------------------------------------------
 *    The MB-system:  mbr_mbarimb1.c  2/2/93
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
 * mbr_mbarimb1.c contains the functions for reading and writing
 * multibeam data in the MBF_MBARIMB1 format.
 * These functions include:
 *   mbr_alm_mbarimb1  - allocate read/write memory
 *   mbr_dem_mbarimb1  - deallocate read/write memory
 *   mbr_rt_mbarimb1  - read and translate data
 *   mbr_wt_mbarimb1  - translate and write data
 *
 * Author:  D. W. Caress
 * Date: October 14, 2019
 *
 */
/*
 * Notes on the MBF_MBARIMB1 data format:
 *   1. This data format is used to store swath bathymetry
 *      data with arbitrary numbers of beams. This format was
 *      created by the Monterey Bay Aquarium Research Institute to
 *      serve as a structure to pass filtered bathymetry data into
 *      software used for terrain relative navigation (TRN). Data files
 *      are created in the format as logs of the records passed
 *      purpose archive formats for processed swath data.
 *   2. The format stores bathymetry data only, no backscatter.
 *   3. Each data record has a 56 byte header including a four-byte
 *      sync pattern ("MB1\0"), the record size, timestamp,
 *      navigation, and number of beams.
 *        offset     type      size         name
 *        ------   -------  --------    ---------------
 *          0       char[4]   4         sync field, always "MB1\0"
 *          4       int       4         data record size, including sync + checksum
 *          8       double    8         timestamp, epoch seconds since 1970
 *         16       double    8         latitude, decimal degrees
 *         24       double    8         longitude, decimal degrees
 *         32       double    8         sonar depth, meters, positive down
 *         40       double    8         heading, decimal degrees
 *         48       int       4         ping number from sonar
 *         52       int       4         number of beams
 *   4. The header is followed by an arbitrary number of beams,
 *      all presumed valid, each consisting of 28 bytes:
 *        offset     type      size         name
 *        ------   -------  --------    ---------------
 *          0       int       4         original beam index
 *          4       double    8         acrosstrack distance, meters, postive starboard
 *         12       double    8         alongtrack distance, meters, postive forward
 *         20       double    8         bathymetry relative to sonar, meters, positive down
 *   5. The beam section is followed by a 4-byte checksum calculated over the
 *      preceding bytes starting with the sync field.
 *          0       int       4         checksum
 *
 * The kind value in the mbsys_ldeoih_struct indicates whether the
 * structure holds data (kind = 1) or an
 * ascii comment record (kind = 0).
 *
 * The structures used to represent the binary data in the MBF_MBARIMB1 format
 * are documented in the mbsys_ldeoih.h file.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_ldeoih.h"

/* define header sizes */
static const int MBF_MBARIMB1_HEADERSIZE = 56;
static const int MBF_MBARIMB1_BEAMSIZE = 28;
static const int MBF_MBARIMB1_CHECKSUMSIZE = 4;
static const int MBF_MBARIMB1_ID = 0x4D423100;    // 'M','B','1','\0'
                                                  // = 3228237 (little endian()
                                                  // = 1296183552 (big endian)
static const int MBF_MBARIMB1_START_BUFFER_SIZE = 4096;

/*--------------------------------------------------------------------*/
int mbr_info_mbarimb1(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
  *system = MB_SYS_LDEOIH;
  *beams_bath_max = 0;
  *beams_amp_max = 0;
  *pixels_ss_max = 0;
  strncpy(format_name, "MBARIMB1", MB_NAME_LENGTH);
  strncpy(system_name, "LDEOIH", MB_NAME_LENGTH);
  strncpy(format_description, "Format name:          MBF_MBARIMB1\n"
          "Informal Description: MBARI TRN swath bathymetry\n"
          "Attributes:           Downsampled bathymetry from multibeam sonars, \n"
          "                      bathymetry only, variable beams, binary, MBARI\n",
          MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_NORMAL;
  *variable_beams = true;
  *traveltime = false;
  *beam_flagging = false;
  *platform_source = MB_DATA_NONE;
  *nav_source = MB_DATA_DATA;
  *sensordepth_source = MB_DATA_DATA;
  *heading_source = MB_DATA_DATA;
  *attitude_source = MB_DATA_DATA;
  *svp_source = MB_DATA_NONE;
  *beamwidth_xtrack = 0.0;
  *beamwidth_ltrack = 0.0;

  int status = MB_SUCCESS;

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
int mbr_alm_mbarimb1(int verbose, void *mbio_ptr, int *error) {
  char **bufferptr = NULL;
  int *bufferalloc = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  int status = mbsys_ldeoih_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* allocate starting memory for data record buffer */
  bufferptr = (char **)&mb_io_ptr->raw_data;
  bufferalloc = (int *)&mb_io_ptr->structure_size;
  *bufferptr = NULL;
  *bufferalloc = 0;
  if (status == MB_SUCCESS) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, MBF_MBARIMB1_START_BUFFER_SIZE, (void **)bufferptr, error);
    if (status == MB_SUCCESS)
      *bufferalloc = MBF_MBARIMB1_START_BUFFER_SIZE;
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
int mbr_dem_mbarimb1(int verbose, void *mbio_ptr, int *error) {

  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* deallocate reading/writing buffer */
  if (mb_io_ptr->raw_data != NULL && mb_io_ptr->structure_size > 0) {
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
    mb_io_ptr->raw_data = NULL;
    mb_io_ptr->data_structure_size = 0;
  }

  /* deallocate memory for data descriptor */
  status = mbsys_ldeoih_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

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
int mbr_rt_mbarimb1(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

  int status = MB_SUCCESS;
  char **bufferptr = NULL;
  size_t *bufferalloc = NULL;
  char *buffer = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor and data structure */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* set file position */
  mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

  /* get reading buffer */
  bufferptr = (char **)&mb_io_ptr->raw_data;
  bufferalloc = (size_t *)&mb_io_ptr->structure_size;
  buffer = *bufferptr;

  /* read next header from file, skipping bytes to find sync if necessary */
  size_t read_len = MBF_MBARIMB1_HEADERSIZE;
  int skip = 0;
  status = mb_fileio_get(verbose, mbio_ptr, (void *)buffer, &read_len, error);
  mb_io_ptr->file_bytes += read_len;
  while (status == MB_SUCCESS && strncmp(buffer, "MB1", 4) != 0) {
    for (int i=0;i<MBF_MBARIMB1_HEADERSIZE-1;i++) {
      buffer[i] = buffer[i+1];
    }
    read_len = 1;
    status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[MBF_MBARIMB1_HEADERSIZE-1], &read_len, error);
    skip++;
  }
  mb_io_ptr->file_bytes += skip;

  /* parse the header */
  double time_d = 0.0;;
  double navlon = 0.0;;
  double navlat = 0.0;;
  double sensordepth = 0.0;;
  double heading = 0.0;
  int ping_number = 0;
  int beams_bath = 0;
  if (status == MB_SUCCESS) {
    int record_size;
    int index = 4;
    mb_get_binary_int(true, &buffer[index], &record_size);
    index += 4;
    mb_get_binary_double(true, &buffer[index], &time_d);
    index += 8;
    mb_get_binary_double(true, &buffer[index], &navlat);
    index += 8;
    mb_get_binary_double(true, &buffer[index], &navlon);
    index += 8;
    mb_get_binary_double(true, &buffer[index], &sensordepth);
    index += 8;
    mb_get_binary_double(true, &buffer[index], &heading);
    index += 8;
    mb_get_binary_int(true, &buffer[index], &ping_number);
    index += 4;
    mb_get_binary_int(true, &buffer[index], &beams_bath);
    index += 4;

    store->kind = MB_DATA_DATA;
    store->time_d = time_d;
    store->longitude = navlon;
    store->latitude = navlat;
    store->sensordepth = sensordepth;
    store->altitude = 0.0;
    store->heading = RTD * heading;
    store->speed = 0.0;
    store->roll = 0.0;
    store->pitch = 0.0;
    store->heave = 0.0;
    store->beam_xwidth = 0.0;
    store->beam_lwidth = 0.0;
    store->beams_bath = beams_bath;
    store->beams_amp = 0;
    store->pixels_ss = 0;
    store->sensorhead = 0;

  }

  // make sure buffer is large enough to read the record
  if (status == MB_SUCCESS) {
    read_len = MBF_MBARIMB1_BEAMSIZE * beams_bath + 1;
    if (*bufferalloc < read_len) {
      *bufferalloc = read_len;
      status = mb_reallocd(verbose, __FILE__, __LINE__, read_len, (void **)bufferptr, error);
      if (status != MB_SUCCESS) {
        *bufferalloc = 0;
      }
      else {
        buffer = (char *)*bufferptr;
        *bufferalloc = read_len;
      }
    }
  }

  /* if needed reset numbers of beams and allocate memory for store arrays */
  if (status == MB_SUCCESS && beams_bath > store->beams_bath_alloc) {
    store->beams_bath_alloc = beams_bath;
    status = mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(char),
                        (void **)&store->beamflag, error);
    status = mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                        (void **)&store->bath, error);
    status = mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                        (void **)&store->bath_acrosstrack, error);
    status = mb_reallocd(verbose, __FILE__, __LINE__, store->beams_bath_alloc * sizeof(short),
                        (void **)&store->bath_alongtrack, error);

    /* deal with a memory allocation failure */
    if (status == MB_FAILURE) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&store->beamflag, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_acrosstrack, error);
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)&store->bath_alongtrack, error);
      status = MB_FAILURE;
      *error = MB_ERROR_MEMORY_FAIL;
      if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBcopy function <%s> terminated with error\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:      %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:  %d\n", status);
      }
      return (status);
    }
  }

  // read the rest of the record
  if (status == MB_SUCCESS) {
    status = mb_fileio_get(verbose, mbio_ptr, (void *)buffer, &read_len, error);
    mb_io_ptr->file_bytes += read_len;
  }

  /* loop over beams to get scaling then read beams into storage */
  if (status == MB_SUCCESS) {
    double depthmax = 0.0;
    double distmax = 0.0;
    double bath = 0.0;
    double bathacrosstrack = 0.0;
    double bathalongtrack = 0.0;
    // int index = 0;
    for (int i = 0; i < beams_bath; i++) {
      int beam_id;
      int index = i * MBF_MBARIMB1_BEAMSIZE;
      mb_get_binary_int(true, &buffer[index], &beam_id);
      index += 4;
      mb_get_binary_double(true, &buffer[index], &bathalongtrack);
      index += 8;
      mb_get_binary_double(true, &buffer[index], &bathacrosstrack);
      index += 8;
      mb_get_binary_double(true, &buffer[index], &bath);
      index += 8;
      depthmax = MAX(depthmax, fabs(bath));
      distmax = MAX(distmax, fabs(bathacrosstrack));
      distmax = MAX(distmax, fabs(bathalongtrack));
    }
    if (depthmax > 0.0)
      store->depth_scale = 0.001 * (float)(MAX((depthmax / 30.0), 1.0));
    if (distmax > 0.0)
      store->distance_scale = 0.001 * (float)(MAX((distmax / 30.0), 1.0));
    // double ss_scale = 0.0;

    for (int i = 0; i < beams_bath; i++) {
      int beam_id;
      int index = i * MBF_MBARIMB1_BEAMSIZE;
      mb_get_binary_int(true, &buffer[index], &beam_id);
      index += 4;
      mb_get_binary_double(true, &buffer[index], &bathalongtrack);
      index += 8;
      mb_get_binary_double(true, &buffer[index], &bathacrosstrack);
      index += 8;
      mb_get_binary_double(true, &buffer[index], &bath);
      index += 8;

      store->beamflag[i] = MB_FLAG_NONE;
      store->bath[i] = bath / store->depth_scale;
      store->bath_acrosstrack[i] = bathacrosstrack / store->distance_scale;
      store->bath_alongtrack[i] = bathalongtrack / store->distance_scale;
    }
  }

  /* set kind and error in mb_io_ptr */
  mb_io_ptr->new_kind = store->kind;
  mb_io_ptr->new_error = *error;

  if (verbose >= 5 && store->kind == MB_DATA_DATA) {
    fprintf(stderr, "\ndbg5  Current version header values in function <%s>\n", __func__);
    fprintf(stderr, "dbg5       time_d:           %f\n", store->time_d);
    fprintf(stderr, "dbg5       longitude:        %f\n", store->longitude);
    fprintf(stderr, "dbg5       latitude:         %f\n", store->latitude);
    fprintf(stderr, "dbg5       sensordepth:       %f\n", store->sensordepth);
    fprintf(stderr, "dbg5       altitude:         %f\n", store->altitude);
    fprintf(stderr, "dbg5       heading:          %f\n", store->heading);
    fprintf(stderr, "dbg5       speed:            %f\n", store->speed);
    fprintf(stderr, "dbg5       roll:             %f\n", store->roll);
    fprintf(stderr, "dbg5       pitch:            %f\n", store->pitch);
    fprintf(stderr, "dbg5       heave:            %f\n", store->heave);
    fprintf(stderr, "dbg5       beam_xwidth:      %f\n", store->beam_xwidth);
    fprintf(stderr, "dbg5       beam_lwidth:      %f\n", store->beam_lwidth);
    fprintf(stderr, "dbg5       beams_bath:       %d\n", store->beams_bath);
    fprintf(stderr, "dbg5       beams_amp:        %d\n", store->beams_amp);
    fprintf(stderr, "dbg5       pixels_ss:        %d\n", store->pixels_ss);
    fprintf(stderr, "dbg5       sensorhead:       %d\n", store->sensorhead);
    fprintf(stderr, "dbg5       depth_scale:      %f\n", store->depth_scale);
    fprintf(stderr, "dbg5       distance_scale:   %f\n", store->distance_scale);
    fprintf(stderr, "dbg5       ss_scalepower:    %d\n", store->ss_scalepower);
    fprintf(stderr, "dbg5       ss_type:          %d\n", store->ss_type);
    fprintf(stderr, "dbg5       imagery_type:     %d\n", store->imagery_type);
    fprintf(stderr, "dbg5       topo_type:        %d\n", store->topo_type);
    fprintf(stderr, "dbg5       status:           %d\n", status);
    fprintf(stderr, "dbg5       error:            %d\n", *error);
  }

  /* update maximum numbers of beams and pixels */
  if (status == MB_SUCCESS) {
    mb_io_ptr->beams_bath_max = MAX(mb_io_ptr->beams_bath_max, store->beams_bath);
    mb_io_ptr->beams_amp_max = MAX(mb_io_ptr->beams_amp_max, store->beams_amp);
    mb_io_ptr->pixels_ss_max = MAX(mb_io_ptr->pixels_ss_max, store->pixels_ss);
  }

  /* print debug messages */
  if (verbose >= 5 && status == MB_SUCCESS) {
    fprintf(stderr, "\ndbg5  New data read in function <%s>\n", __func__);
    fprintf(stderr, "dbg5       beams_bath: %d\n", store->beams_bath);
    for (int i = 0; i < store->beams_bath; i++)
      fprintf(stderr, "dbg5       beam:%d  flag:%d  bath:%d  acrosstrack:%d  alongtrack:%d\n", i, store->beamflag[i],
              store->bath[i], store->bath_acrosstrack[i], store->bath_alongtrack[i]);
    fprintf(stderr, "dbg5       beams_amp:  %d\n", store->beams_amp);
    for (int i = 0; i < store->beams_amp; i++)
      fprintf(stderr, "dbg5       beam:%d  flag:%d  amp:%d  acrosstrack:%d  alongtrack:%d\n", i, store->beamflag[i],
              store->amp[i], store->bath_acrosstrack[i], store->bath_alongtrack[i]);
    fprintf(stderr, "dbg5       pixels_ss:  %d\n", store->pixels_ss);
    for (int i = 0; i < store->pixels_ss; i++)
      fprintf(stderr, "dbg5       pixel:%d  ss:%d acrosstrack:%d  alongtrack:%d\n", i, store->ss[i],
              store->ss_acrosstrack[i], store->ss_alongtrack[i]);
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
int mbr_wt_mbarimb1(int verbose, void *mbio_ptr, void *store_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor and data storage */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_ldeoih_struct *store = (struct mbsys_ldeoih_struct *)store_ptr;

  /* get reading buffer */
  char **bufferptr = NULL;
  size_t *bufferalloc = NULL;
  char *buffer = NULL;
  bufferptr = (char **)&mb_io_ptr->raw_data;
  bufferalloc = (size_t *)&mb_io_ptr->structure_size;
  buffer = *bufferptr;

  int status = MB_SUCCESS;
  if (store->kind == MB_DATA_DATA) {

    // write mb1 format
    int record_size = MBF_MBARIMB1_HEADERSIZE + store->beams_bath * MBF_MBARIMB1_BEAMSIZE + MBF_MBARIMB1_CHECKSUMSIZE;
    double time_d = store->time_d;
    double navlon = store->longitude;
    double navlat = store->latitude;
    double sensordepth = store->sensordepth;
    double heading = DTR * store->heading;
    int ping_number = 0;
    int beams_bath = store->beams_bath;

    // make sure buffer is large enough to read the record
    size_t write_len = record_size;
    if (*bufferalloc < write_len) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, write_len, (void **)bufferptr, error);
      if (status != MB_SUCCESS) {
        *bufferalloc = 0;
      }
      else {
        buffer = (char *)*bufferptr;
        *bufferalloc = write_len;
      }
    }

    int index = 0;
    buffer[0] = 'M';
    buffer[1] = 'B';
    buffer[2] = '1';
    buffer[3] = 0;
    index += 4;
    mb_put_binary_int(true, record_size, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, time_d, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, navlat, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, navlon, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, sensordepth, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, heading, &buffer[index]);
    index += 8;
    mb_put_binary_int(true, ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, beams_bath, &buffer[index]);
    index += 4;

    for (int i=0; i<beams_bath; i++) {
      int beam_id = 0;
      mb_put_binary_int(true, beam_id, &buffer[index]);
      index += 4;
      double bathalongtrack = store->bath_alongtrack[i] * store->distance_scale;
      mb_put_binary_double(true, bathalongtrack, &buffer[index]);
      index += 8;
      double bathacrosstrack = store->bath_acrosstrack[i] * store->distance_scale;
      mb_put_binary_double(true, bathacrosstrack, &buffer[index]);
      index += 8;
      double bath = store->bath[i] * store->depth_scale;
      mb_put_binary_double(true, bath, &buffer[index]);
      index += 8;
    }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Writing data in function <%s>\n", __func__);
    fprintf(stderr, "dbg5       kind:       %d\n", store->kind);
    fprintf(stderr, "dbg5       status:     %d\n", status);
    fprintf(stderr, "dbg5       error:      %d\n", *error);
    fprintf(stderr, "dbg5  Header values:\n");
    fprintf(stderr, "dbg5       record_id:        %d\n", MBF_MBARIMB1_ID);
    fprintf(stderr, "dbg5       record_size:      %d\n", record_size);
    fprintf(stderr, "dbg5       time_d:           %f\n", time_d);
    fprintf(stderr, "dbg5       latitude:         %f\n", navlat);
    fprintf(stderr, "dbg5       longitude:        %f\n", navlon);
    fprintf(stderr, "dbg5       sensordepth:       %f\n", sensordepth);
    fprintf(stderr, "dbg5       heading:          %f\n", heading);
    fprintf(stderr, "dbg5       ping_number:      %d\n", ping_number);
    fprintf(stderr, "dbg5       beams_bath:       %d\n", beams_bath);
    for (int i=0; i<beams_bath; i++) {
      fprintf(stderr, "dbg5       beam_id:          %d\n", 0);
      fprintf(stderr, "dbg5       bathalongtrack:   %f\n", store->bath_alongtrack[i] * store->distance_scale);
      fprintf(stderr, "dbg5       bathacrosstrack:  %f\n", store->bath_acrosstrack[i] * store->distance_scale);
      fprintf(stderr, "dbg5       bath:             %f\n", (store->bath[i] * store->depth_scale) + sensordepth);
    }
  }

    /* write to file */
    if ((status = fwrite(buffer, 1, write_len, mb_io_ptr->mbfp)) == record_size) {
      status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
    }
    else {
      status = MB_FAILURE;
      *error = MB_ERROR_EOF;
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
int mbr_register_mbarimb1(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_mbarimb1(
      verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
      mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
      &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
      &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
      &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_mbarimb1;
  mb_io_ptr->mb_io_format_free = &mbr_dem_mbarimb1;
  mb_io_ptr->mb_io_store_alloc = &mbsys_ldeoih_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_ldeoih_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_mbarimb1;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_mbarimb1;
  mb_io_ptr->mb_io_dimensions = &mbsys_ldeoih_dimensions;
  mb_io_ptr->mb_io_sonartype = &mbsys_ldeoih_sonartype;
  mb_io_ptr->mb_io_sidescantype = &mbsys_ldeoih_sidescantype;
  mb_io_ptr->mb_io_sensorhead = &mbsys_ldeoih_sensorhead;
  mb_io_ptr->mb_io_extract = &mbsys_ldeoih_extract;
  mb_io_ptr->mb_io_insert = &mbsys_ldeoih_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_ldeoih_extract_nav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_ldeoih_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_ldeoih_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = &mbsys_ldeoih_insert_altitude;
  mb_io_ptr->mb_io_extract_svp = NULL;
  mb_io_ptr->mb_io_insert_svp = NULL;
  mb_io_ptr->mb_io_ttimes = &mbsys_ldeoih_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_ldeoih_detects;
  mb_io_ptr->mb_io_copyrecord = &mbsys_ldeoih_copy;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;

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
    fprintf(stderr, "dbg2       dimensions:         %p\n", (void *)mb_io_ptr->mb_io_dimensions);
    fprintf(stderr, "dbg2       sidescantype:       %p\n", (void *)mb_io_ptr->mb_io_sidescantype);
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
    fprintf(stderr, "dbg2       extract_rawss:      %p\n", (void *)mb_io_ptr->mb_io_extract_rawss);
    fprintf(stderr, "dbg2       insert_rawss:       %p\n", (void *)mb_io_ptr->mb_io_insert_rawss);
    fprintf(stderr, "dbg2       copyrecord:         %p\n", (void *)mb_io_ptr->mb_io_copyrecord);
    fprintf(stderr, "dbg2       error:              %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:         %d\n", status);
  }

  return (status);
}
/*--------------------------------------------------------------------*/
