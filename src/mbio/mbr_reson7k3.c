/*--------------------------------------------------------------------
 *    The MB-system:  mbr_reson7k3.c  1/8/2019
 *
 *    Copyright (c) 2019-2019 by
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
 * mbr_reson7k3.c contains the functions for reading and writing
 * multibeam data in the MBF_RESON7K3 format version 3.
 * These functions include:
 *   mbr_alm_reson7k3  - allocate read/write memory
 *   mbr_dem_reson7k3  - deallocate read/write memory
 *   mbr_rt_reson7k3  - read and translate data
 *   mbr_wt_reson7k3  - translate and write data
 *
 * Authors:  C. S. Ferreira & D. W. Caress
 * Date:  March 2019
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_process.h"
#include "mb_status.h"
#include "mb_swap.h"
#include "mbsys_reson7k3.h"

/* turn on debug statements here */
//#define MBR_RESON7K3_DEBUG 1
//#define MBR_RESON7K3_DEBUG2 1
//#define MBR_RESON7K3_DEBUG3 1

/*--------------------------------------------------------------------*/
int mbr_info_reson7k3(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
  strncpy(format_name, "RESON7K3", MB_NAME_LENGTH);
  strncpy(system_name, "RESON7K", MB_NAME_LENGTH);
  strncpy(format_description,
          "Format name:          MBF_RESON7K3\nInformal Description: Reson 7K multibeam vendor format\nAttributes:           "
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
int mbr_alm_reson7k3(int verbose, void *mbio_ptr, int *error) {
  int *current_ping;
  int *last_ping;
  int *save_flag;
  int *recordid;
  int *recordidlast;
  char **bufferptr;
  char *buffer;
  int *bufferalloc;
  char **buffersaveptr;
  char *buffersave;
  int *size;
  int *nbadrec;
  int *deviceid;
  unsigned short *enumerator;
  int *fileheaders;
  double *pixel_size;
  double *swath_width;
  int *preprocess_pars_set;
  int *platform_set;
	struct mb_platform_struct **platform_ptr = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* allocate memory for data structure */
  mb_io_ptr->structure_size = 0;
  mb_io_ptr->data_structure_size = 0;
  int status = mbsys_reson7k3_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);
  save_flag = (int *)&mb_io_ptr->save_flag;
  current_ping = (int *)&mb_io_ptr->save14;
  last_ping = (int *)&mb_io_ptr->save1;
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
  pixel_size = (double *)&mb_io_ptr->saved1;
  swath_width = (double *)&mb_io_ptr->saved2;
  preprocess_pars_set = (int *)&mb_io_ptr->save13;
  platform_set = (int *)&mb_io_ptr->save7;
  platform_ptr = (struct mb_platform_struct **)&mb_io_ptr->saveptr3;

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
int mbr_reson7k3_wr_header(int verbose, char *buffer, int *index, s7k3_header *header, int *error) {

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

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_header(verbose, header, error);

  /* insert the header */
  mb_put_binary_short(false, header->Version, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(false, header->Offset, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(false, header->SyncPattern, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->Size, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->OptionalDataOffset, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->OptionalDataIdentifier, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(false, header->s7kTime.Year, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(false, header->s7kTime.Day, &buffer[*index]);
  *index += 2;
  mb_put_binary_float(false, header->s7kTime.Seconds, &buffer[*index]);
  *index += 4;
  buffer[*index] = header->s7kTime.Hours;
  (*index)++;
  buffer[*index] = header->s7kTime.Minutes;
  (*index)++;
  mb_put_binary_short(false, header->RecordVersion, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(false, header->RecordType, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->DeviceId, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(false, header->Reserved, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(false, header->SystemEnumerator, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(false, header->Reserved2, &buffer[*index]);
  *index += 4;
  mb_put_binary_short(false, header->Flags, &buffer[*index]);
  *index += 2;
  mb_put_binary_short(false, header->Reserved3, &buffer[*index]);
  *index += 2;
  mb_put_binary_int(false, header->Reserved4, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->FragmentedTotal, &buffer[*index]);
  *index += 4;
  mb_put_binary_int(false, header->FragmentNumber, &buffer[*index]);
  *index += 4;

  const int status = MB_SUCCESS;

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
int mbr_reson7k3_wr_FileCatalog(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  s7k3_filecatalogdata *filecatalogdata;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_FileCatalog *FileCatalog = &(store->FileCatalog_write);
  s7k3_header *header = &(FileCatalog->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FileCatalog(verbose, FileCatalog, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FileCatalog;
  *size += FileCatalog->n * R7KRDTSIZE_FileCatalog;

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
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(false, FileCatalog->size, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, FileCatalog->version, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, FileCatalog->n, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, FileCatalog->reserved, &buffer[index]);
    index += 4;
    for (int i = 0; i < FileCatalog->n; i++) {
      filecatalogdata = &(FileCatalog->filecatalogdata[i]);
      mb_put_binary_int(false, filecatalogdata->size, &buffer[index]);
      index += 4;
      mb_put_binary_long(false, filecatalogdata->offset, &buffer[index]);
      index += 8;
      mb_put_binary_short(false, filecatalogdata->record_type, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, filecatalogdata->device_id, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, filecatalogdata->system_enumerator, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, filecatalogdata->s7kTime.Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, filecatalogdata->s7kTime.Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, filecatalogdata->s7kTime.Seconds, &buffer[index]);
      index += 4;
      buffer[index] = filecatalogdata->s7kTime.Hours;
      index++;
      buffer[index] = filecatalogdata->s7kTime.Minutes;
      index++;
      mb_put_binary_int(false, filecatalogdata->record_count, &buffer[index]);
      index += 4;
      for (int j = 0;j<8;j++) {
        mb_put_binary_short(false, filecatalogdata->reserved[j], &buffer[index]);
        index += 2;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_dem_reson7k3(int verbose, void *mbio_ptr, int *error) {
  char **bufferptr = NULL;
  char *buffer = NULL;
  int *bufferalloc = NULL;
  char **buffersaveptr = NULL;
  char *buffersave = NULL;
  int *filecatalogoffsetoffset = NULL;
  int *platform_set = NULL;
	struct mb_platform_struct **platform_ptr = NULL;
  int size;
  long offset;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointers to buffers */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *) &mb_io_ptr->store_data;
  bufferptr = (char **)&mb_io_ptr->saveptr1;
  buffer = (char *)*bufferptr;
  bufferalloc = (int *)&mb_io_ptr->save6;
  buffersaveptr = (char **)&mb_io_ptr->saveptr2;
  buffersave = (char *)*buffersaveptr;
  filecatalogoffsetoffset = (int *)&mb_io_ptr->save5;
  platform_ptr = (struct mb_platform_struct **)&mb_io_ptr->saveptr3;

  int status = MB_SUCCESS;

  // if this is ordinary file i/o then write the FileCatalog record before
  // deallocating memory
  if (mb_io_ptr->filemode == MB_FILEMODE_WRITE) {
    // get file offset before writing FileCatalog
    offset = ftell(mb_io_ptr->mbfp);

    // set the FileCatalog header values
    store->FileCatalog_write.header.Version = 5;
    store->FileCatalog_write.header.Offset = 60;
    store->FileCatalog_write.header.SyncPattern = 65535;
    store->FileCatalog_write.header.Size
      = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE
          + R7KHDRSIZE_FileCatalog + store->FileCatalog_write.n * R7KRDTSIZE_FileCatalog;
    store->FileCatalog_write.header.OptionalDataOffset = 0;
    store->FileCatalog_write.header.OptionalDataIdentifier = 0;
    int time_j[5], time_i[7];
    mb_get_date(verbose, (double) time((time_t *)0), time_i);
    mb_get_jtime(verbose, time_i, time_j);
    store->FileCatalog_write.header.s7kTime.Year = time_i[0];
    store->FileCatalog_write.header.s7kTime.Day = time_j[1];
    store->FileCatalog_write.header.s7kTime.Hours = time_i[3];
    store->FileCatalog_write.header.s7kTime.Minutes = time_i[4];
    store->FileCatalog_write.header.s7kTime.Seconds = time_i[5] + 0.000001 * time_i[6];
    store->FileCatalog_write.header.RecordVersion = 1;
    store->FileCatalog_write.header.RecordType = R7KRECID_FileCatalog;
    store->FileCatalog_write.header.DeviceId = 7000;
    store->FileCatalog_write.header.Reserved = 0;
    store->FileCatalog_write.header.SystemEnumerator = 0;
    store->FileCatalog_write.header.Reserved2 = 0;
    store->FileCatalog_write.header.Flags = 0;
    store->FileCatalog_write.header.Reserved3 = 0;
    store->FileCatalog_write.header.Reserved4 = 0;
    store->FileCatalog_write.header.FragmentedTotal = 0;
    store->FileCatalog_write.header.FragmentNumber = 0;
    store->FileCatalog_write.size = 14;
    store->FileCatalog_write.version = 1;
    store->FileCatalog_write.reserved = 0;

    // write the FileCatalog record at the end of the file
    status = mbr_reson7k3_wr_FileCatalog(verbose, bufferalloc, bufferptr, store, &size, error);
    buffer = (char *)*bufferptr;
    size_t write_len = (size_t)size;
    status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
//mbsys_reson7k3_print_FileCatalog(verbose, &store->FileCatalog_write, error);

    // now reset the size and offset of the FileCatalog record in the FileHeader
    // record at the start of the file
    fseek(mb_io_ptr->mbfp, (long)*filecatalogoffsetoffset, SEEK_SET);
    int index = 0;
    mb_put_binary_int(false, write_len, &buffer[index]);
    index += 4;
    mb_put_binary_long(false, offset, &buffer[index]);
    index += 8;
    write_len = index;
    status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    fseek(mb_io_ptr->mbfp, 0L, SEEK_END);
  }

  /* deallocate memory for preprocessing parameters */
  platform_set = (int *)&mb_io_ptr->save7;
  platform_ptr = (struct mb_platform_struct **)&mb_io_ptr->saveptr3;
  if (*platform_set == true) {
    status = mb_platform_deall(verbose, (void **)platform_ptr, error);
    *platform_set = false;
    *platform_ptr = NULL;
  }

  /* deallocate memory for data descriptor */
  status = mbsys_reson7k3_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* deallocate memory for reading/writing buffer */
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
int mbr_reson7k3_chk_header(int verbose, void *mbio_ptr, char *buffer, int *recordid,
                            int *deviceid, unsigned short *enumerator, int *size) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:      %p\n", (void *)mbio_ptr);
  }

  /* get values to check */
  unsigned short version;
  unsigned short offset;
  unsigned int sync;
  unsigned short reserved;
  mb_get_binary_short(true, &buffer[0], &version);
  mb_get_binary_short(true, &buffer[2], &offset);
  mb_get_binary_int(true, &buffer[4], &sync);
  mb_get_binary_int(true, &buffer[8], size);
  mb_get_binary_int(true, &buffer[32], recordid);
  mb_get_binary_int(true, &buffer[36], deviceid);
  mb_get_binary_short(true, &buffer[40], &reserved);
  mb_get_binary_short(true, &buffer[42], enumerator);
#ifdef MBR_RESON7K3_DEBUG3
  fprintf(stderr, "\nChecking header in mbr_reson7k3_chk_header:\n");
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

  int status = MB_SUCCESS;

  /* check sync */
  if (sync != 0x0000FFFF) {
    status = MB_FAILURE;
  }

  /* check recordid */
  else if (*recordid != R7KRECID_ReferencePoint &&
            *recordid != R7KRECID_UncalibratedSensorOffset &&
            *recordid != R7KRECID_CalibratedSensorOffset &&
            *recordid != R7KRECID_Position &&
            *recordid != R7KRECID_CustomAttitude &&
            *recordid != R7KRECID_Tide &&
            *recordid != R7KRECID_Altitude &&
            *recordid != R7KRECID_MotionOverGround &&
            *recordid != R7KRECID_Depth &&
            *recordid != R7KRECID_SoundVelocityProfile &&
            *recordid != R7KRECID_CTD &&
            *recordid != R7KRECID_Geodesy &&
            *recordid != R7KRECID_RollPitchHeave &&
            *recordid != R7KRECID_Heading &&
            *recordid != R7KRECID_SurveyLine &&
            *recordid != R7KRECID_Navigation &&
            *recordid != R7KRECID_Attitude &&
            *recordid != R7KRECID_PanTilt &&
            *recordid != R7KRECID_SonarInstallationIDs &&
            *recordid != R7KRECID_Mystery &&
            *recordid != R7KRECID_SonarPipeEnvironment &&
            *recordid != R7KRECID_ContactOutput &&
            *recordid != R7KRECID_ProcessedSideScan &&
            *recordid != R7KRECID_SonarSettings &&
            *recordid != R7KRECID_Configuration &&
            *recordid != R7KRECID_MatchFilter &&
            *recordid != R7KRECID_FirmwareHardwareConfiguration &&
            *recordid != R7KRECID_BeamGeometry &&
            *recordid != R7KRECID_Bathymetry &&
            *recordid != R7KRECID_SideScan &&
            *recordid != R7KRECID_WaterColumn &&
            *recordid != R7KRECID_VerticalDepth &&
            *recordid != R7KRECID_TVG &&
            *recordid != R7KRECID_Image &&
            *recordid != R7KRECID_PingMotion &&
            *recordid != R7KRECID_AdaptiveGate &&
            *recordid != R7KRECID_DetectionDataSetup &&
            *recordid != R7KRECID_Beamformed &&
            *recordid != R7KRECID_VernierProcessingDataRaw &&
            *recordid != R7KRECID_BITE &&
            *recordid != R7KRECID_SonarSourceVersion &&
            *recordid != R7KRECID_WetEndVersion8k &&
            *recordid != R7KRECID_RawDetection &&
            *recordid != R7KRECID_Snippet &&
            *recordid != R7KRECID_VernierProcessingDataFiltered &&
            *recordid != R7KRECID_InstallationParameters &&
            *recordid != R7KRECID_BITESummary &&
            *recordid != R7KRECID_CompressedBeamformedMagnitude &&
            *recordid != R7KRECID_CompressedWaterColumn &&
            *recordid != R7KRECID_SegmentedRawDetection &&
            *recordid != R7KRECID_CalibratedBeam &&
            *recordid != R7KRECID_SystemEvents &&
            *recordid != R7KRECID_SystemEventMessage &&
            *recordid != R7KRECID_RDRRecordingStatus &&
            *recordid != R7KRECID_Subscriptions &&
            *recordid != R7KRECID_RDRStorageRecording &&
            *recordid != R7KRECID_CalibrationStatus &&
            *recordid != R7KRECID_CalibratedSideScan &&
            *recordid != R7KRECID_SnippetBackscatteringStrength &&
            *recordid != R7KRECID_MB2Status &&
            *recordid != R7KRECID_FileHeader &&
            *recordid != R7KRECID_FileCatalog &&
            *recordid != R7KRECID_TimeMessage &&
            *recordid != R7KRECID_RemoteControl &&
            *recordid != R7KRECID_RemoteControlAcknowledge &&
            *recordid != R7KRECID_RemoteControlNotAcknowledge &&
            *recordid != R7KRECID_RemoteControlSonarSettings &&
            *recordid != R7KRECID_CommonSystemSettings &&
            *recordid != R7KRECID_SVFiltering &&
            *recordid != R7KRECID_SystemLockStatus &&
            *recordid != R7KRECID_SoundVelocity &&
            *recordid != R7KRECID_AbsorptionLoss &&
            *recordid != R7KRECID_SpreadingLoss) {
    status = MB_FAILURE;
  }
  else {
    status = MB_SUCCESS;

#ifdef MBR_RESON7K3_DEBUG2
    if (verbose > 0) {
      fprintf(stderr, "Good record id: %4.4X | %d", *recordid, *recordid);
    if (*recordid ==R7KRECID_None)
      fprintf(stderr, " R7KRECID_None:                                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_ReferencePoint)
      fprintf(stderr, " R7KRECID_ReferencePoint:                      %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_UncalibratedSensorOffset)
      fprintf(stderr, " R7KRECID_UncalibratedSensorOffset:            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CalibratedSensorOffset)
      fprintf(stderr, " R7KRECID_CalibratedSensorOffset:              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Position)
      fprintf(stderr, " R7KRECID_Position:                            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CustomAttitude)
      fprintf(stderr, " R7KRECID_CustomAttitude:                      %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Tide)
      fprintf(stderr, " R7KRECID_Tide:                                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Altitude)
      fprintf(stderr, " R7KRECID_Altitude:                            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_MotionOverGround)
      fprintf(stderr, " R7KRECID_MotionOverGround:                    %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Depth)
      fprintf(stderr, " R7KRECID_Depth:                               %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SoundVelocityProfile)
      fprintf(stderr, " R7KRECID_SoundVelocityProfile:                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CTD)
      fprintf(stderr, " R7KRECID_CTD:                                 %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Geodesy)
      fprintf(stderr, " R7KRECID_Geodesy:                             %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RollPitchHeave)
      fprintf(stderr, " R7KRECID_RollPitchHeave:                      %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Heading)
      fprintf(stderr, " R7KRECID_Heading:                             %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SurveyLine)
      fprintf(stderr, " R7KRECID_SurveyLine:                          %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Navigation)
      fprintf(stderr, " R7KRECID_Navigation:                          %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Attitude)
      fprintf(stderr, " R7KRECID_Attitude:                            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_PanTilt)
      fprintf(stderr, " R7KRECID_PanTilt:                             %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SonarInstallationIDs)
      fprintf(stderr, " R7KRECID_SonarInstallationIDs:                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Mystery)
      fprintf(stderr, " R7KRECID_Mystery:                             %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SonarPipeEnvironment)
      fprintf(stderr, " R7KRECID_SonarPipeEnvironment:                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_ContactOutput)
      fprintf(stderr, " R7KRECID_ContactOutput:                       %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_ProcessedSideScan)
      fprintf(stderr, " R7KRECID_ProcessedSideScan:                   %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SonarSettings)
      fprintf(stderr, " R7KRECID_SonarSettings:                       %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Configuration)
      fprintf(stderr, " R7KRECID_Configuration:                       %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_MatchFilter)
      fprintf(stderr, " R7KRECID_MatchFilter:                         %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_FirmwareHardwareConfiguration)
      fprintf(stderr, " R7KRECID_FirmwareHardwareConfiguration:       %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_BeamGeometry)
      fprintf(stderr, " R7KRECID_BeamGeometry:                        %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Bathymetry)
      fprintf(stderr, " R7KRECID_Bathymetry:                          %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SideScan)
      fprintf(stderr, " R7KRECID_SideScan:                            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_WaterColumn)
      fprintf(stderr, " R7KRECID_WaterColumn:                         %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_VerticalDepth)
      fprintf(stderr, " R7KRECID_VerticalDepth:                       %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_TVG)
      fprintf(stderr, " R7KRECID_TVG:                                 %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Image)
      fprintf(stderr, " R7KRECID_Image:                               %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_PingMotion)
      fprintf(stderr, " R7KRECID_PingMotion:                          %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_AdaptiveGate)
      fprintf(stderr, " R7KRECID_AdaptiveGate:                        %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_DetectionDataSetup)
      fprintf(stderr, " R7KRECID_DetectionDataSetup:                  %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Beamformed)
      fprintf(stderr, " R7KRECID_Beamformed:                          %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_VernierProcessingDataRaw)
      fprintf(stderr, " R7KRECID_VernierProcessingDataRaw:            %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_BITE)
      fprintf(stderr, " R7KRECID_BITE:                                %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SonarSourceVersion)
      fprintf(stderr, " R7KRECID_SonarSourceVersion:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_WetEndVersion8k)
      fprintf(stderr, " R7KRECID_WetEndVersion8k:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RawDetection)
      fprintf(stderr, " R7KRECID_RawDetection:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Snippet)
      fprintf(stderr, " R7KRECID_Snippet:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_VernierProcessingDataFiltered)
      fprintf(stderr, " R7KRECID_VernierProcessingDataFiltered:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_InstallationParameters)
      fprintf(stderr, " R7KRECID_InstallationParameters:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_BITESummary)
      fprintf(stderr, " R7KRECID_BITESummary:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CompressedBeamformedMagnitude)
      fprintf(stderr, " R7KRECID_CompressedBeamformedMagnitude:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CompressedWaterColumn)
      fprintf(stderr, " R7KRECID_CompressedWaterColumn:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SegmentedRawDetection)
      fprintf(stderr, " R7KRECID_SegmentedRawDetection:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CalibratedBeam)
      fprintf(stderr, " R7KRECID_CalibratedBeam:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SystemEvents)
      fprintf(stderr, " R7KRECID_SystemEvents:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SystemEventMessage)
      fprintf(stderr, " R7KRECID_SystemEventMessage:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RDRRecordingStatus)
      fprintf(stderr, " R7KRECID_RDRRecordingStatus:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_Subscriptions)
      fprintf(stderr, " R7KRECID_Subscriptions:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RDRStorageRecording)
      fprintf(stderr, " R7KRECID_RDRStorageRecording:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CalibrationStatus)
      fprintf(stderr, " R7KRECID_CalibrationStatus:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CalibratedSideScan)
      fprintf(stderr, " R7KRECID_CalibratedSideScan:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SnippetBackscatteringStrength)
      fprintf(stderr, " R7KRECID_SnippetBackscatteringStrength:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_MB2Status)
      fprintf(stderr, " R7KRECID_MB2Status:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_FileHeader)
      fprintf(stderr, " R7KRECID_FileHeader:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_FileCatalog)
      fprintf(stderr, " R7KRECID_FileCatalog:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_TimeMessage)
      fprintf(stderr, " R7KRECID_TimeMessage:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RemoteControl)
      fprintf(stderr, " R7KRECID_RemoteControl:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RemoteControlAcknowledge)
      fprintf(stderr, " R7KRECID_RemoteControlAcknowledge:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RemoteControlNotAcknowledge)
      fprintf(stderr, " R7KRECID_RemoteControlNotAcknowledge:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_RemoteControlSonarSettings)
      fprintf(stderr, " R7KRECID_RemoteControlSonarSettings:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_CommonSystemSettings)
      fprintf(stderr, " R7KRECID_CommonSystemSettings:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SVFiltering)
      fprintf(stderr, " R7KRECID_SVFiltering:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SystemLockStatus)
      fprintf(stderr, " R7KRECID_SystemLockStatus:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SoundVelocity)
      fprintf(stderr, " R7KRECID_SoundVelocity:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_AbsorptionLoss)
      fprintf(stderr, " R7KRECID_AbsorptionLoss:                              %4.4X | %d\n", *recordid, *recordid);
    if (*recordid ==R7KRECID_SpreadingLoss)
      fprintf(stderr, " R7KRECID_SpreadingLoss:                              %4.4X | %d\n", *recordid, *recordid);
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
int mbr_reson7k3_chk_pingnumber(int verbose, int recordid, char *buffer, int *ping_number) {
  unsigned short offset;
  int index;

  assert(buffer != NULL);
  assert(ping_number != NULL);

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
  switch (recordid) {

    case R7KRECID_ProcessedSideScan:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_SonarSettings:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_MatchFilter:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_Bathymetry:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_SideScan:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_WaterColumn:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_VerticalDepth:
      index = offset + 8;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_TVG:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_Image:
      index = offset + 4;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_PingMotion:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_AdaptiveGate:
      index = offset + 14;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_DetectionDataSetup:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_Beamformed:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_VernierProcessingDataRaw:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_RawDetection:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_Snippet:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_VernierProcessingDataFiltered:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_CompressedBeamformedMagnitude:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_CompressedWaterColumn:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_SegmentedRawDetection:
      index = offset + 26;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_CalibratedBeam:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_CalibratedSideScan:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_SnippetBackscatteringStrength:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

    case R7KRECID_RemoteControlSonarSettings:
      index = offset + 12;
      mb_get_binary_int(true, &buffer[index], ping_number);
      break;

      default:
      *ping_number = 0;
    }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Output arguments:\n");
    fprintf(stderr, "dbg2       ping_number:   %d\n", *ping_number);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", MB_SUCCESS);
  }

  return (MB_SUCCESS);
}
/*--------------------------------------------------------------------*/
int mbr_reson7k3_rd_header(int verbose, char *buffer, int *index, s7k3_header *header, int *error) {
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
  mb_get_binary_int(true, &buffer[*index], &(header->OptionalDataOffset));
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
  mb_get_binary_short(true, &buffer[*index], &(header->RecordVersion));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(header->RecordType));
  *index += 4;
  mb_get_binary_int(true, &buffer[*index], &(header->DeviceId));
  *index += 4;
  mb_get_binary_short(true, &buffer[*index], &(header->Reserved));
  *index += 2;
  mb_get_binary_short(true, &buffer[*index], &(header->SystemEnumerator));
  *index += 2;
  mb_get_binary_int(true, &buffer[*index], &(header->Reserved2));
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

  /* print out the results */
  /* mbsys_reson7k3_print_header(verbose, header, error); */

  const int status = MB_SUCCESS;

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
int mbr_reson7k3_rd_ReferencePoint(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_ReferencePoint *ReferencePoint;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  ReferencePoint = &(store->ReferencePoint);
  header = &(ReferencePoint->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(ReferencePoint->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(ReferencePoint->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(ReferencePoint->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(ReferencePoint->water_z));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_ReferencePoint;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_ReferencePoint:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_ReferencePoint(verbose, ReferencePoint, error);

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
int mbr_reson7k3_rd_UncalibratedSensorOffset(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_UncalibratedSensorOffset *UncalibratedSensorOffset;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  UncalibratedSensorOffset = &(store->UncalibratedSensorOffset);
  header = &(UncalibratedSensorOffset->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(UncalibratedSensorOffset->offset_yaw));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_UncalibratedSensorOffset;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_UncalibratedSensorOffset:      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_UncalibratedSensorOffset(verbose, UncalibratedSensorOffset, error);

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
int mbr_reson7k3_rd_CalibratedSensorOffset(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_CalibratedSensorOffset *CalibratedSensorOffset;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  CalibratedSensorOffset = &(store->CalibratedSensorOffset);
  header = &(CalibratedSensorOffset->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CalibratedSensorOffset->offset_yaw));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_CalibratedSensorOffset;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_CalibratedSensorOffset:        7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CalibratedSensorOffset(verbose, CalibratedSensorOffset, error);

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
int mbr_reson7k3_rd_Position(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Position *Position;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Position = &(store->Position);
  header = &(Position->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(Position->datum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Position->latency));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(Position->latitude_northing));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Position->longitude_easting));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Position->height));
  index += 8;
  Position->type = buffer[index];
  index++;
  Position->utm_zone = buffer[index];
  index++;
  Position->quality = buffer[index];
  index++;
  Position->method = buffer[index];
  index++;
  Position->nsat = buffer[index];
  index++;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV1;
    store->type = R7KRECID_Position;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Position:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Position(verbose, Position, error);

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
int mbr_reson7k3_rd_CustomAttitude(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_CustomAttitude *CustomAttitude;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  CustomAttitude = &(store->CustomAttitude);
  header = &(CustomAttitude->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  CustomAttitude->fieldmask = (mb_u_char)buffer[index];
  index++;
  CustomAttitude->reserved = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(CustomAttitude->n));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(CustomAttitude->frequency));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (CustomAttitude->nalloc < CustomAttitude->n) {
    data_size = CustomAttitude->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->heading), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->pitchrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->rollrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->headingrate), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CustomAttitude->heaverate), error);
    if (status == MB_SUCCESS) {
      CustomAttitude->nalloc = CustomAttitude->n;
    }
    else {
      CustomAttitude->nalloc = 0;
      CustomAttitude->n = 0;
    }
  }

  if (CustomAttitude->fieldmask & 1)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->pitch[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 2)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->roll[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 4)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->heading[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 8)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->heave[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 16)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->pitchrate[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 32)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->rollrate[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 64)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->headingrate[i]));
      index += 4;
    }
  if (CustomAttitude->fieldmask & 128)
    for (int i = 0; i < CustomAttitude->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(CustomAttitude->heaverate[i]));
      index += 4;
    }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE2;
    store->type = R7KRECID_CustomAttitude;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_CustomAttitude:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CustomAttitude(verbose, CustomAttitude, error);

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
int mbr_reson7k3_rd_Tide(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Tide *Tide;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Tide = &(store->Tide);
  header = &(Tide->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(Tide->tide));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Tide->source));
  index += 2;
  Tide->flags = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(Tide->gauge));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(Tide->datum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Tide->latency));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(Tide->latitude_northing));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Tide->longitude_easting));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Tide->height));
  index += 8;
  Tide->type = buffer[index];
  index++;
  Tide->utm_zone = buffer[index];
  index++;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_TIDE;
    store->type = R7KRECID_Tide;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Tide:                          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Tide(verbose, Tide, error);

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
int mbr_reson7k3_rd_Altitude(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Altitude *Altitude;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Altitude = &(store->Altitude);
  header = &(Altitude->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(Altitude->altitude));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ALTITUDE;
    store->type = R7KRECID_Altitude;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Altitude:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Altitude(verbose, Altitude, error);

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
int mbr_reson7k3_rd_MotionOverGround(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_MotionOverGround *MotionOverGround;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  MotionOverGround = &(store->MotionOverGround);
  header = &(MotionOverGround->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  MotionOverGround->flags = (mb_u_char)buffer[index];
  index++;
  MotionOverGround->reserved = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(MotionOverGround->n));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(MotionOverGround->frequency));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (MotionOverGround->nalloc < MotionOverGround->n) {
    data_size = MotionOverGround->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->x), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->y), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->z), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->xa), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->ya), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(MotionOverGround->za), error);
    if (status == MB_SUCCESS) {
      MotionOverGround->nalloc = MotionOverGround->n;
    }
    else {
      MotionOverGround->nalloc = 0;
      MotionOverGround->n = 0;
    }
  }

  if (MotionOverGround->flags & 1) {
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->x[i]));
      index += 4;
    }
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->y[i]));
      index += 4;
    }
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->z[i]));
      index += 4;
    }
  }
  if (MotionOverGround->flags & 2) {
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->xa[i]));
      index += 4;
    }
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->ya[i]));
      index += 4;
    }
    for (int i = 0; i < MotionOverGround->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(MotionOverGround->za[i]));
      index += 4;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_MOTION;
    store->type = R7KRECID_MotionOverGround;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_MotionOverGround:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_MotionOverGround(verbose, MotionOverGround, error);

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
int mbr_reson7k3_rd_Depth(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Depth *Depth;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Depth = &(store->Depth);
  header = &(Depth->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  Depth->descriptor = (mb_u_char)buffer[index];
  index++;
  Depth->correction = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(Depth->reserved));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(Depth->depth));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SONARDEPTH;
    store->type = R7KRECID_Depth;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Depth:                         7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Depth(verbose, Depth, error);

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
int mbr_reson7k3_rd_SoundVelocityProfile(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_SoundVelocityProfile *SoundVelocityProfile;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  SoundVelocityProfile = &(store->SoundVelocityProfile);
  header = &(SoundVelocityProfile->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  SoundVelocityProfile->position_flag = (mb_u_char)buffer[index];
  index++;
  SoundVelocityProfile->reserved1 = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(SoundVelocityProfile->reserved2));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(SoundVelocityProfile->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(SoundVelocityProfile->longitude));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(SoundVelocityProfile->n));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (SoundVelocityProfile->nalloc < SoundVelocityProfile->n) {
    data_size = SoundVelocityProfile->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SoundVelocityProfile->depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SoundVelocityProfile->sound_velocity), error);
    if (status == MB_SUCCESS) {
      SoundVelocityProfile->nalloc = SoundVelocityProfile->n;
    }
    else {
      SoundVelocityProfile->nalloc = 0;
      SoundVelocityProfile->n = 0;
    }
  }

  for (int i = 0; i < SoundVelocityProfile->n; i++) {
    mb_get_binary_float(true, &buffer[index], &(SoundVelocityProfile->depth[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SoundVelocityProfile->sound_velocity[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_VELOCITY_PROFILE;
    store->type = R7KRECID_SoundVelocityProfile;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SoundVelocityProfile:          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SoundVelocityProfile(verbose, SoundVelocityProfile, error);

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
int mbr_reson7k3_rd_CTD(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_CTD *CTD;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  CTD = &(store->CTD);
  header = &(CTD->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(CTD->frequency));
  index += 4;
  CTD->velocity_source_flag = (mb_u_char)buffer[index];
  index++;
  CTD->velocity_algorithm = (mb_u_char)buffer[index];
  index++;
  CTD->conductivity_flag = (mb_u_char)buffer[index];
  index++;
  CTD->pressure_flag = (mb_u_char)buffer[index];
  index++;
  CTD->position_flag = (mb_u_char)buffer[index];
  index++;
  CTD->validity = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(CTD->reserved));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(CTD->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(CTD->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(CTD->sample_rate));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CTD->n));
  index += 4;

  /* make sure enough memory is allocated for channel data */
  if (CTD->nalloc < CTD->n) {
    data_size = CTD->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CTD->conductivity_salinity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CTD->temperature), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CTD->pressure_depth), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CTD->sound_velocity), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(CTD->absorption), error);
    if (status == MB_SUCCESS) {
      CTD->nalloc = CTD->n;
    }
    else {
      CTD->nalloc = 0;
      CTD->n = 0;
    }
  }

  for (int i = 0; i < CTD->n; i++) {
    mb_get_binary_float(true, &buffer[index], &(CTD->conductivity_salinity[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(CTD->temperature[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(CTD->pressure_depth[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(CTD->sound_velocity[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(CTD->absorption[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_CTD;
    store->type = R7KRECID_CTD;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_CTD:                           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CTD(verbose, CTD, error);

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
int mbr_reson7k3_rd_Geodesy(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Geodesy *Geodesy;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Geodesy = &(store->Geodesy);
  header = &(Geodesy->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    Geodesy->spheroid[i] = (mb_u_char)buffer[index];
    index++;
  }
  mb_get_binary_double(true, &buffer[index], &(Geodesy->semimajoraxis));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->flattening));
  index += 8;
  for (int i = 0; i < 16; i++) {
    Geodesy->reserved1[i] = (mb_u_char)buffer[index];
    index++;
  }
  for (int i = 0; i < 32; i++) {
    Geodesy->datum[i] = (mb_u_char)buffer[index];
    index++;
  }
  mb_get_binary_int(true, &buffer[index], &(Geodesy->calculation_method));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(Geodesy->number_parameters));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->dx));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->dy));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->dz));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->rx));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->ry));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->rz));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->scale));
  index += 8;
  for (int i = 0; i < 35; i++) {
    Geodesy->reserved2[i] = (mb_u_char)buffer[index];
    index++;
  }
  for (int i = 0; i < 32; i++) {
    Geodesy->grid_name[i] = (mb_u_char)buffer[index];
    index++;
  }
  Geodesy->distance_units = (mb_u_char)buffer[index];
  index++;
  Geodesy->angular_units = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->latitude_origin));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->central_meridian));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->false_easting));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->false_northing));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Geodesy->central_scale_factor));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(Geodesy->custom_identifier));
  index += 4;
  for (int i = 0; i < 50; i++) {
    Geodesy->reserved3[i] = (mb_u_char)buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_Geodesy;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Geodesy:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Geodesy(verbose, Geodesy, error);

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
int mbr_reson7k3_rd_RollPitchHeave(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_RollPitchHeave *RollPitchHeave;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  RollPitchHeave = &(store->RollPitchHeave);
  header = &(RollPitchHeave->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(RollPitchHeave->roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RollPitchHeave->pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RollPitchHeave->heave));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE1;
    store->type = R7KRECID_RollPitchHeave;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_RollPitchHeave:                7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RollPitchHeave(verbose, RollPitchHeave, error);

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
int mbr_reson7k3_rd_Heading(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Heading *Heading;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Heading = &(store->Heading);
  header = &(Heading->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(Heading->heading));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADING;
    store->type = R7KRECID_Heading;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Heading:                       7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Heading(verbose, Heading, error);

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
int mbr_reson7k3_rd_SurveyLine(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_SurveyLine *SurveyLine;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  SurveyLine = &(store->SurveyLine);
  header = &(SurveyLine->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_short(true, &buffer[index], &(SurveyLine->n));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(SurveyLine->type));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(SurveyLine->turnradius));
  index += 4;
  for (int i = 0; i < 64; i++) {
    SurveyLine->name[i] = (char)buffer[index];
    index++;
  }

  /* make sure enough memory is allocated for channel data */
  if (SurveyLine->nalloc < SurveyLine->n) {
    data_size = SurveyLine->n * sizeof(float);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SurveyLine->latitude_northing), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SurveyLine->longitude_easting), error);
    if (status == MB_SUCCESS) {
      SurveyLine->nalloc = SurveyLine->n;
    }
    else {
      SurveyLine->nalloc = 0;
      SurveyLine->n = 0;
    }
  }

  for (int i = 0; i < SurveyLine->n; i++) {
    mb_get_binary_double(true, &buffer[index], &(SurveyLine->latitude_northing[i]));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(SurveyLine->longitude_easting[i]));
    index += 8;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SURVEY_LINE;
    store->type = R7KRECID_SurveyLine;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SurveyLine:                   7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SurveyLine(verbose, SurveyLine, error);

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
int mbr_reson7k3_rd_Navigation(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Navigation *Navigation;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Navigation = &(store->Navigation);
  header = &(Navigation->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  Navigation->vertical_reference = (mb_u_char)buffer[index];
  index++;
  mb_get_binary_double(true, &buffer[index], &(Navigation->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(Navigation->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(Navigation->position_accuracy));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Navigation->height));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Navigation->height_accuracy));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Navigation->speed));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Navigation->course));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Navigation->heading));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV;
    store->type = R7KRECID_Navigation;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Navigation:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Navigation(verbose, Navigation, error);

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
int mbr_reson7k3_rd_Attitude(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Attitude *Attitude;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Attitude = &(store->Attitude);
  header = &(Attitude->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  Attitude->n = (mb_u_char)buffer[index];
  index++;

  /* make sure enough memory is allocated for channel data */
  if (Attitude->nalloc < Attitude->n) {
    data_size = Attitude->n * sizeof(unsigned short);
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(Attitude->delta_time), error);
    data_size = Attitude->n * sizeof(float);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(Attitude->roll), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(Attitude->pitch), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(Attitude->heave), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(Attitude->heading), error);
    if (status == MB_SUCCESS) {
      Attitude->nalloc = Attitude->n;
    }
    else {
      Attitude->nalloc = 0;
      Attitude->n = 0;
    }
  }

  for (int i = 0; i < Attitude->n; i++) {
    mb_get_binary_short(true, &buffer[index], &(Attitude->delta_time[i]));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(Attitude->roll[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Attitude->pitch[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Attitude->heave[i]));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Attitude->heading[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ATTITUDE;
    store->type = R7KRECID_Attitude;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Attitude:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Attitude(verbose, Attitude, error);

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
int mbr_reson7k3_rd_PanTilt(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_PanTilt *PanTilt;
  int index;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  PanTilt = &(store->PanTilt);
  header = &(PanTilt->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SonarInstallationIDs(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_header *header = NULL;
  s7k3_SonarInstallationIDs *SonarInstallationIDs;
  int index;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  SonarInstallationIDs = &(store->SonarInstallationIDs);
  header = &(SonarInstallationIDs->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_Mystery(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Mystery *Mystery;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Mystery = &(store->Mystery);
  header = &(Mystery->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < R7KHDRSIZE_Mystery; i++) {
    Mystery->data[i] = (mb_u_char)buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_RAW_LINE;
    store->type = R7KRECID_Mystery;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Mystery:                      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Mystery(verbose, Mystery, error);

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
int mbr_reson7k3_rd_SonarPipeEnvironment(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_header *header = NULL;
  s7k3_SonarPipeEnvironment *SonarPipeEnvironment;
  int index;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  SonarPipeEnvironment = &(store->SonarPipeEnvironment);
  header = &(SonarPipeEnvironment->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_ContactOutput(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_header *header = NULL;
  s7k3_ContactOutput *ContactOutput;
  int index;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  ContactOutput = &(store->ContactOutput);
  header = &(ContactOutput->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_ProcessedSideScan(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_ProcessedSideScan *ProcessedSideScan;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  ProcessedSideScan = &(store->ProcessedSideScan);
  header = &(ProcessedSideScan->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(ProcessedSideScan->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(ProcessedSideScan->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(ProcessedSideScan->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(ProcessedSideScan->recordversion));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(ProcessedSideScan->ss_source));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(ProcessedSideScan->number_pixels));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(ProcessedSideScan->ss_type));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(ProcessedSideScan->pixelwidth));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(ProcessedSideScan->sonardepth));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(ProcessedSideScan->altitude));
  index += 8;

  /* extract the data */
  for (int i = 0; i < ProcessedSideScan->number_pixels; i++) {
    mb_get_binary_float(true, &buffer[index], &(ProcessedSideScan->sidescan[i]));
    index += 4;
  }
  for (int i = 0; i < ProcessedSideScan->number_pixels; i++) {
    mb_get_binary_float(true, &buffer[index], &(ProcessedSideScan->alongtrack[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_ProcessedSideScan;

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
#ifdef MBR_reson7k3_DEBUG
  fprintf(stderr,
          "R7KRECID_ProcessedSideScan:                --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], ProcessedSideScan->ping_number, header->Size, index);
#endif
#ifdef MBR_reson7k3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_ProcessedSideScan(verbose, ProcessedSideScan, error);

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
int mbr_reson7k3_rd_SonarSettings(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_SonarSettings *SonarSettings;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  SonarSettings = &(store->SonarSettings);
  header = &(SonarSettings->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(SonarSettings->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SonarSettings->multi_ping));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->sample_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->receiver_bandwidth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->tx_pulse_width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->tx_pulse_type));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->tx_pulse_envelope));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->tx_pulse_envelope_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->tx_pulse_mode));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->max_ping_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->ping_period));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->range_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->power_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->gain_selection));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->projector_id));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->steering_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->steering_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->beamwidth_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->beamwidth_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->focal_point));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->projector_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->projector_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->transmit_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->hydrophone_id));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->rx_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->rx_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SonarSettings->rx_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->rx_width));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->range_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->range_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->depth_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->depth_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->absorption));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SonarSettings->spreading));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SonarSettings->reserved));
  index += 2;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_SonarSettings;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SonarSettings:               --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], SonarSettings->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarSettings(verbose, SonarSettings, error);

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
int mbr_reson7k3_rd_Configuration(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_Configuration *Configuration;
  s7k3_device *device;
  int data_size;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  Configuration = &(store->Configuration);
  header = &(Configuration->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(Configuration->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(Configuration->number_devices));
  index += 4;

  /* extract the data for each device */
  for (int i = 0; i < Configuration->number_devices; i++) {
    device = &(Configuration->device[i]);
    mb_get_binary_int(true, &buffer[index], &(device->magic_number));
    index += 4;
    for (int j = 0; j < 60; j++) {
      device->description[j] = buffer[index];
      index++;
    }
    mb_get_binary_int(true, &buffer[index], &(device->alphadata_card));
    index += 4;
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

    for (int j = 0; j < device->info_length; j++) {
      device->info[j] = buffer[index];
      index++;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_Configuration;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Configuration:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Configuration(verbose, Configuration, error);

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
int mbr_reson7k3_rd_MatchFilter(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_MatchFilter *MatchFilter;
  int index;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  MatchFilter = &(store->MatchFilter);
  header = &(MatchFilter->header);

  /* extract the header */
  index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(MatchFilter->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(MatchFilter->ping_number));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(MatchFilter->operation));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(MatchFilter->start_frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(MatchFilter->end_frequency));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(MatchFilter->window_type));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(MatchFilter->shading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(MatchFilter->pulse_width));
  index += 4;
  for (int i = 0;i<13;i++) {
    mb_get_binary_int(true, &buffer[index], &(MatchFilter->reserved[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_MatchFilter;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_MatchFilter:                 --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], MatchFilter->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_MatchFilter(verbose, MatchFilter, error);

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
int mbr_reson7k3_rd_FirmwareHardwareConfiguration(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_header *header = NULL;
  s7k3_FirmwareHardwareConfiguration *FirmwareHardwareConfiguration;
  int index;
  int data_size;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  FirmwareHardwareConfiguration = &(store->FirmwareHardwareConfiguration);
  header = &(FirmwareHardwareConfiguration->header);

  /* extract the header */
  index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(FirmwareHardwareConfiguration->device_count));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(FirmwareHardwareConfiguration->info_length));
  index += 4;

  /* make sure enough memory is allocated for info data */
  if (FirmwareHardwareConfiguration->info_alloc < FirmwareHardwareConfiguration->info_length) {
    data_size = FirmwareHardwareConfiguration->info_length + 1;
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(FirmwareHardwareConfiguration->info), error);
    if (status == MB_SUCCESS) {
      FirmwareHardwareConfiguration->info_alloc = FirmwareHardwareConfiguration->info_length;
    }
    else {
      FirmwareHardwareConfiguration->info_alloc = 0;
      FirmwareHardwareConfiguration->info_length = 0;
    }
  }

  for (int j = 0; j < FirmwareHardwareConfiguration->info_length; j++) {
    FirmwareHardwareConfiguration->info[j] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_FirmwareHardwareConfiguration;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_FirmwareHardwareConfiguration: 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
          "size:%d index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FirmwareHardwareConfiguration(verbose, FirmwareHardwareConfiguration, error);

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
int mbr_reson7k3_rd_BeamGeometry(int verbose, char *buffer, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BeamGeometry *BeamGeometry = &(store->BeamGeometry);
  s7k3_header *header = &(BeamGeometry->header);

  /* extract the header */
  int index = 0;
  const int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(BeamGeometry->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(BeamGeometry->number_beams));
  index += 4;

  /* extract the data */
  for (int i = 0; i < BeamGeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(BeamGeometry->angle_alongtrack[i]));
    index += 4;
  }
  for (int i = 0; i < BeamGeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(BeamGeometry->angle_acrosstrack[i]));
    index += 4;
  }
  for (int i = 0; i < BeamGeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(BeamGeometry->beamwidth_alongtrack[i]));
    index += 4;
  }
  for (int i = 0; i < BeamGeometry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(BeamGeometry->beamwidth_acrosstrack[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_BeamGeometry;

    /* get the time */
    int time_j[5];
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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_BeamGeometry:                  7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) record_number:%d "
          "size:%d index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_BeamGeometry(verbose, BeamGeometry, error);

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
int mbr_reson7k3_rd_Bathymetry(int verbose, char *buffer, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Bathymetry *Bathymetry = &(store->Bathymetry);
  s7k3_header *header = &(Bathymetry->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(Bathymetry->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(Bathymetry->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Bathymetry->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(Bathymetry->number_beams));
  index += 4;

  /* deal with version 5 records */
  if (header->Version >= 5) {
    Bathymetry->layer_comp_flag = buffer[index];
    index++;
    Bathymetry->sound_vel_flag = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->sound_velocity));
    index += 4;
  }
  else {
    Bathymetry->layer_comp_flag = 0;
    Bathymetry->sound_vel_flag = 0;
    Bathymetry->sound_velocity = 0.0;
  }

  /* extract the data */
  for (int i = 0; i < Bathymetry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->range[i]));
    index += 4;
  }
  for (int i = 0; i < Bathymetry->number_beams; i++) {
    Bathymetry->quality[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < Bathymetry->number_beams; i++) {
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->intensity[i]));
    index += 4;
  }
  if ((header->OptionalDataOffset == 0 && header->Size >= 92 + 17 * Bathymetry->number_beams) ||
      (header->OptionalDataOffset > 0 && header->Size >= 137 + 37 * Bathymetry->number_beams)) {
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->min_depth_gate[i]));
      index += 4;
    }
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->max_depth_gate[i]));
      index += 4;
    }
  }

  /* extract the optional data */
  if (header->OptionalDataOffset > 0) {
    index = header->OptionalDataOffset;
    Bathymetry->optionaldata = true;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(Bathymetry->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(Bathymetry->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->heading));
    index += 4;
    Bathymetry->height_source = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->tide));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->roll));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->pitch));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->heave));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Bathymetry->vehicle_depth));
    index += 4;
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->depth[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->alongtrack[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->acrosstrack[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->pointing_angle[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(Bathymetry->azimuth_angle[i]));
      index += 4;
    }
  }
  else {
    Bathymetry->optionaldata = false;
    Bathymetry->frequency = 0.0;
    Bathymetry->latitude = 0.0;
    Bathymetry->longitude = 0.0;
    Bathymetry->heading = 0.0;
    Bathymetry->height_source = 0;
    Bathymetry->tide = 0.0;
    Bathymetry->roll = 0.0;
    Bathymetry->pitch = 0.0;
    Bathymetry->heave = 0.0;
    Bathymetry->vehicle_depth = 0.0;
    for (int i = 0; i < MBSYS_RESON7K_MAX_BEAMS; i++) {
      Bathymetry->depth[i] = 0.0;
      Bathymetry->acrosstrack[i] = 0.0;
      Bathymetry->alongtrack[i] = 0.0;
      Bathymetry->pointing_angle[i] = 0.0;
      Bathymetry->azimuth_angle[i] = 0.0;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_Bathymetry;

    /* get the time */
    int time_j[5];
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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Bathymetry:                  --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], Bathymetry->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Bathymetry(verbose, Bathymetry, error);

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
int mbr_reson7k3_rd_SideScan(int verbose, char *buffer, void *store_ptr, int *error) {
  int data_size;
  int time_j[5];
  short *short_ptr;
  int *int_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SideScan *SideScan = &(store->SideScan);
  s7k3_header *header = &(SideScan->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(SideScan->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(SideScan->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SideScan->multi_ping));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(SideScan->beam_position));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SideScan->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SideScan->number_samples));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(SideScan->nadir_depth));
  index += 4;
  for (int i = 0; i<7; i++) {
     mb_get_binary_int(true, &buffer[index], &(SideScan->reserved[i]));
      index += 4;
  }
  mb_get_binary_short(true, &buffer[index], &(SideScan->number_beams));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(SideScan->current_beam));
  index += 2;
  SideScan->sample_size = buffer[index];
  index++;
  SideScan->data_type = buffer[index];
  index++;

  /* allocate memory if required */
  data_size = SideScan->number_samples * SideScan->sample_size;
  if (SideScan->nalloc < data_size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SideScan->port_data), error);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SideScan->stbd_data), error);
    if (status == MB_SUCCESS) {
      SideScan->nalloc = data_size;
    }
    else {
      SideScan->nalloc = 0;
      SideScan->number_samples = 0;
    }
  }

  /* extract SideScan data */
  if (SideScan->sample_size == 1) {
    for (int i = 0; i < SideScan->number_samples; i++) {
      SideScan->port_data[i] = buffer[index];
      index++;
    }
    for (int i = 0; i < SideScan->number_samples; i++) {
      SideScan->stbd_data[i] = buffer[index];
      index++;
    }
  }
  else if (SideScan->sample_size == 2) {
    short_ptr = (short *)SideScan->port_data;
    for (int i = 0; i < SideScan->number_samples; i++) {
      mb_get_binary_short(true, &buffer[index], &(short_ptr[i]));
      index += 2;
    }
    short_ptr = (short *)SideScan->stbd_data;
    for (int i = 0; i < SideScan->number_samples; i++) {
      mb_get_binary_short(true, &buffer[index], &(short_ptr[i]));
      index += 2;
    }
  }
  else if (SideScan->sample_size == 4) {
    int_ptr = (int *)SideScan->port_data;
    for (int i = 0; i < SideScan->number_samples; i++) {
      mb_get_binary_int(true, &buffer[index], &(int_ptr[i]));
      index += 4;
    }
    int_ptr = (int *)SideScan->stbd_data;
    for (int i = 0; i < SideScan->number_samples; i++) {
      mb_get_binary_int(true, &buffer[index], &(int_ptr[i]));
      index += 4;
    }
  }

  /* extract the optional data */
  if (header->OptionalDataOffset > 0) {
    index = header->OptionalDataOffset;
    SideScan->optionaldata = true;
    mb_get_binary_float(true, &buffer[index], &(SideScan->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(SideScan->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(SideScan->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(SideScan->heading));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SideScan->altitude));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SideScan->depth));
    index += 4;
  }
  else {
    SideScan->optionaldata = false;
    SideScan->frequency = 0.0;
    SideScan->latitude = 0.0;
    SideScan->longitude = 0.0;
    SideScan->heading = 0.0;
    SideScan->altitude = 0.0;
    SideScan->depth = 0.0;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_SideScan;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SideScan:                    --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], SideScan->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SideScan(verbose, SideScan, error);

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
int mbr_reson7k3_rd_WaterColumn(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_wcd *wcd;
  int time_j[5];
  int nalloc_amp;
  int nalloc_phase;
  int nsamples;
  int sample_type_amp;
  int sample_type_phase;
  int sample_type_iandq;
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
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_WaterColumn *WaterColumn = &(store->WaterColumn);
  s7k3_header *header = &(WaterColumn->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(WaterColumn->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(WaterColumn->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(WaterColumn->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(WaterColumn->number_beams));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(WaterColumn->reserved));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(WaterColumn->samples));
  index += 4;
  WaterColumn->subset_flag = buffer[index];
  index++;
  WaterColumn->column_flag = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(WaterColumn->reserved2));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(WaterColumn->sample_type));
  index += 4;
  sample_type_amp = WaterColumn->sample_type & 15;
  sample_type_phase = (WaterColumn->sample_type >> 4) & 15;
  sample_type_iandq = (WaterColumn->sample_type >> 8) & 15;
  for (int i = 0; i < WaterColumn->number_beams; i++) {
    wcd = &WaterColumn->wcd[i];
    mb_get_binary_short(true, &buffer[index], &(wcd->beam_number));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(wcd->begin_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(wcd->end_sample));
    index += 4;
  }

  for (int i = 0; i < WaterColumn->number_beams; i++) {
    /* allocate memory for Snippet if needed */
    wcd = &WaterColumn->wcd[i];
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
    nalloc_amp *= (wcd->end_sample - wcd->begin_sample + 1);
    nalloc_phase *= (wcd->end_sample - wcd->begin_sample + 1);
    if (status == MB_SUCCESS && (wcd->nalloc_amp < nalloc_amp || wcd->nalloc_phase < nalloc_phase)) {
      wcd->nalloc_amp = nalloc_amp;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, wcd->nalloc_amp, (void **)&(wcd->amplitude), error);
      wcd->nalloc_phase = nalloc_phase;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, wcd->nalloc_phase, (void **)&(wcd->phase), error);
      if (status != MB_SUCCESS) {
        wcd->nalloc_amp = 0;
        wcd->nalloc_phase = 0;
      }
    }

    /* extract wcd or beam data */
    if (status == MB_SUCCESS) {
      nsamples = wcd->end_sample - wcd->begin_sample + 1;
      for (int j = 0; j < nsamples; j++) {
        if (sample_type_amp == 1) {
          charptr = (char *)wcd->amplitude;
          charptr[j] = buffer[index];
          index++;
        }
        else if (sample_type_amp == 2) {
          ushortptr = (unsigned short *)wcd->amplitude;
          mb_get_binary_short(true, &buffer[index], &(ushortptr[j]));
          index += 2;
        }
        else if (sample_type_amp == 3) {
          uintptr = (unsigned int *)wcd->amplitude;
          mb_get_binary_int(true, &buffer[index], &(uintptr[j]));
          index += 4;
        }
        if (sample_type_phase == 1) {
          charptr = (char *)wcd->phase;
          charptr[j] = buffer[index];
          index++;
        }
        else if (sample_type_phase == 2) {
          ushortptr = (unsigned short *)wcd->phase;
          mb_get_binary_short(true, &buffer[index], &(ushortptr[j]));
          index += 2;
        }
        else if (sample_type_phase == 3) {
          uintptr = (unsigned int *)wcd->phase;
          mb_get_binary_int(true, &buffer[index], &(uintptr[j]));
          index += 4;
        }
        if (sample_type_iandq == 1) {
          shortptramp = (short *)wcd->amplitude;
          shortptrphase = (short *)wcd->phase;
          mb_get_binary_short(true, &buffer[index], &(shortptramp[j]));
          index += 2;
          mb_get_binary_short(true, &buffer[index], &(shortptrphase[j]));
          index += 2;
        }
        else if (sample_type_iandq == 2) {
          intptramp = (int *)wcd->amplitude;
          intptrphase = (int *)wcd->phase;
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
    store->type = R7KRECID_WaterColumn;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KHDRSIZE_WaterColumn:                     --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], WaterColumn->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_WaterColumn(verbose, WaterColumn, error);

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
int mbr_reson7k3_rd_VerticalDepth(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VerticalDepth *VerticalDepth = &(store->VerticalDepth);
  s7k3_header *header = &(VerticalDepth->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(VerticalDepth->frequency));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(VerticalDepth->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(VerticalDepth->multi_ping));
  index += 2;
  mb_get_binary_double(true, &buffer[index], &(VerticalDepth->latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(VerticalDepth->longitude));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(VerticalDepth->heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(VerticalDepth->alongtrack));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(VerticalDepth->acrosstrack));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(VerticalDepth->vertical_depth));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_VerticalDepth;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_VerticalDepth:               --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], VerticalDepth->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_VerticalDepth(verbose, VerticalDepth, error);

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
int mbr_reson7k3_rd_TVG(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];
  int nalloc;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_TVG *TVG = &(store->TVG);
  s7k3_header *header = &(TVG->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(TVG->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(TVG->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(TVG->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(TVG->n));
  index += 4;
  for (int i = 0; i < 8; i++) {
    mb_get_binary_int(true, &buffer[index], &(TVG->reserved[i]));
    index += 4;
  }

  /* allocate memory for TVG if needed */
  nalloc = TVG->n * sizeof(float);
  if (status == MB_SUCCESS && TVG->nalloc < nalloc) {
    TVG->nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, TVG->nalloc, (void **)&(TVG->tvg), error);
    if (status != MB_SUCCESS) {
      TVG->nalloc = 0;
    }
  }

  /* extract TVG data */
  memcpy((void *)TVG->tvg, (const void *)&buffer[index], (size_t)(TVG->n * sizeof(float)));

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_TVG;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_TVG:                         --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], TVG->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_TVG(verbose, TVG, error);

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
int mbr_reson7k3_rd_Image(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];
  int nalloc;
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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Image *Image = &(store->Image);
  s7k3_header *header = &(Image->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(Image->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Image->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(Image->width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(Image->height));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Image->color_depth));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(Image->reserved));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(Image->compression));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(Image->samples));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(Image->flag));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(Image->rx_delay));
  index += 4;
  for (int i = 0; i < 6; i++) {
    mb_get_binary_int(true, &buffer[index], &(Image->reserved2[i]));
    index += 4;
  }

  /* allocate memory for Image if needed */
  nalloc = Image->width * Image->height * Image->color_depth;
  if (status == MB_SUCCESS && Image->nalloc < nalloc) {
    Image->nalloc = nalloc;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, Image->nalloc, (void **)&(Image->image), error);
    if (status != MB_SUCCESS) {
      Image->nalloc = 0;
      Image->width = 0;
      Image->height = 0;
    }
  }

  /* extract image data */
  if (Image->color_depth == 1) {
    charptr = (char *)Image->image;
    for (int i = 0; i < Image->width * Image->height; i++) {
      charptr[i] = buffer[index];
      index++;
    }
  }
  else if (Image->color_depth == 2) {
    ushortptr = (unsigned short *)Image->image;
    for (int i = 0; i < Image->width * Image->height; i++) {
      mb_get_binary_short(true, &buffer[index], &(ushortptr[i]));
      index += 2;
    }
  }
  else if (Image->color_depth == 4) {
    uintptr = (unsigned int *)Image->image;
    for (int i = 0; i < Image->width * Image->height; i++) {
      mb_get_binary_int(true, &buffer[index], &(uintptr[i]));
      index += 4;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_Image;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Image:                       --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], Image->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Image(verbose, Image, error);

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
int mbr_reson7k3_rd_PingMotion(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_PingMotion *PingMotion = &(store->PingMotion);
  s7k3_header *header = &(PingMotion->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(PingMotion->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(PingMotion->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(PingMotion->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(PingMotion->n));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(PingMotion->flags));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(PingMotion->error_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(PingMotion->frequency));
  index += 4;
  if (PingMotion->flags & 1) {
    mb_get_binary_float(true, &buffer[index], &(PingMotion->pitch));
    index += 4;
  }

  /* allocate memory for PingMotion if needed */
  if (status == MB_SUCCESS && PingMotion->nalloc < PingMotion->n) {
    PingMotion->nalloc = sizeof(float) * PingMotion->n;
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, PingMotion->nalloc, (void **)&(PingMotion->roll), error);
    if (status != MB_SUCCESS) {
      PingMotion->nalloc = 0;
    }
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, PingMotion->nalloc, (void **)&(PingMotion->heading), error);
    if (status != MB_SUCCESS) {
      PingMotion->nalloc = 0;
    }
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, PingMotion->nalloc, (void **)&(PingMotion->heave), error);
    if (status != MB_SUCCESS) {
      PingMotion->nalloc = 0;
    }
  }

  /* extract PingMotion data */
  if (PingMotion->flags & 2) {
    for (int i = 0; i < PingMotion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(PingMotion->roll[i]));
      index += 4;
    }
  }
  else {
    for (int i = 0; i < PingMotion->n; i++) {
      PingMotion->roll[i] = 0.0;
    }
  }
  if (PingMotion->flags & 4) {
    for (int i = 0; i < PingMotion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(PingMotion->heading[i]));
      index += 4;
    }
  }
  else {
    for (int i = 0; i < PingMotion->n; i++) {
      PingMotion->heading[i] = 0.0;
    }
  }
  if (PingMotion->flags & 8) {
    for (int i = 0; i < PingMotion->n; i++) {
      mb_get_binary_float(true, &buffer[index], &(PingMotion->heave[i]));
      index += 4;
    }
  }
  else {
    for (int i = 0; i < PingMotion->n; i++) {
      PingMotion->heave[i] = 0.0;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_PingMotion;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_PingMotion:                   --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], PingMotion->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_PingMotion(verbose, PingMotion, error);

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
int mbr_reson7k3_rd_AdaptiveGate(int verbose, char *buffer, void *store_ptr, int *error){

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_AdaptiveGate *AdaptiveGate = &(store->AdaptiveGate);
  s7k3_header *header = &(AdaptiveGate->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_DetectionDataSetup(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_DetectionDataSetup *DetectionDataSetup = &(store->DetectionDataSetup);
  s7k3_header *header = &(DetectionDataSetup->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(DetectionDataSetup->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(DetectionDataSetup->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->number_beams));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->data_block_size));
  index += 4;
  DetectionDataSetup->detection_algorithm = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->detection_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->minimum_depth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->maximum_depth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->minimum_range));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->maximum_range));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->minimum_nadir_search));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->maximum_nadir_search));
  index += 4;
  DetectionDataSetup->automatic_filter_window = buffer[index];
  index++;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->applied_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->depth_gate_tilt));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->nadir_depth));
  index += 4;
  for (int i = 0; i < 13; i++) {
    mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->reserved[i]));
    index += 4;
  }

  /* extract DetectionDataSetup data */
  for (int i = 0; i < DetectionDataSetup->number_beams; i++) {
    mb_get_binary_short(true, &buffer[index], &(DetectionDataSetup->beam_descriptor[i]));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->detection_point[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->flags[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->auto_limits_min_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->auto_limits_max_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->user_limits_min_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->user_limits_max_sample[i]));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(DetectionDataSetup->quality[i]));
    index += 4;
    if (DetectionDataSetup->data_block_size >= R7KRDTSIZE_DetectionDataSetup) {
      mb_get_binary_float(true, &buffer[index], &(DetectionDataSetup->uncertainty[i]));
      index += 4;
    } else {
      DetectionDataSetup->uncertainty[i] = 0.0;
    }
    if (DetectionDataSetup->data_block_size > R7KRDTSIZE_DetectionDataSetup)
      index += DetectionDataSetup->data_block_size - R7KRDTSIZE_DetectionDataSetup;
  }
  if (DetectionDataSetup->data_block_size > R7KRDTSIZE_DetectionDataSetup)
    DetectionDataSetup->data_block_size = R7KRDTSIZE_DetectionDataSetup;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_DetectionDataSetup;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_DetectionDataSetup:          --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], DetectionDataSetup->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_DetectionDataSetup(verbose, DetectionDataSetup, error);

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
int mbr_reson7k3_rd_Beamformed(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_amplitudephase *amplitudephase;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Beamformed *Beamformed = &(store->Beamformed);
  s7k3_header *header = &(Beamformed->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(Beamformed->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(Beamformed->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Beamformed->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(Beamformed->number_beams));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(Beamformed->number_samples));
  index += 4;
  for (int i = 0; i < 8; i++) {
    Beamformed->reserved[i] = buffer[index];
    index++;
  }

  /* loop over all beams */
  for (int i = 0; i < Beamformed->number_beams; i++) {
    amplitudephase = &(Beamformed->amplitudephase[i]);

    /* allocate memory for Beamformed if needed */
    if (status == MB_SUCCESS && amplitudephase->nalloc < sizeof(short) * Beamformed->number_samples) {
      amplitudephase->nalloc = sizeof(short) * Beamformed->number_samples;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, amplitudephase->nalloc,
                             (void **)&(amplitudephase->amplitude), error);
      if (status != MB_SUCCESS) {
        amplitudephase->nalloc = 0;
      }
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, amplitudephase->nalloc, (void **)&(amplitudephase->phase),
                             error);
      if (status != MB_SUCCESS) {
        amplitudephase->nalloc = 0;
      }
    }

    /* extract Beamformed data */
    for (int j = 0; j < Beamformed->number_samples; j++) {
      mb_get_binary_short(true, &buffer[index], &(amplitudephase->amplitude[j]));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(amplitudephase->phase[j]));
      index += 2;
    }
    amplitudephase->beam_number = i;
    amplitudephase->number_samples = Beamformed->number_samples;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_Beamformed;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_BeamformedData:               --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], Beamformed->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Beamformed(verbose, Beamformed, error);

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
int mbr_reson7k3_rd_VernierProcessingDataRaw(int verbose, char *buffer, void *store_ptr, int *error){

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VernierProcessingDataRaw *VernierProcessingDataRaw = &(store->VernierProcessingDataRaw);
  s7k3_header *header = &(VernierProcessingDataRaw->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_BITE(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_bitereport *bitereport;
  s7k3_time *s7kTime;
  s7k3_bitefield *bitefield;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BITE *BITE = &(store->BITE);
  s7k3_header *header = &(BITE->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_short(true, &buffer[index], &(BITE->number_reports));
  index += 2;

  /* allocate memory for BITE->reports if needed */
  const size_t nalloc = BITE->number_reports * (R7KRDTSIZE_BITERecordData + 256 * R7KRDTSIZE_BITEFieldData);
  if (status == MB_SUCCESS && BITE->nalloc < nalloc) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc, (void **)&(BITE->bitereports), error);
    if (status == MB_SUCCESS) {
      BITE->nalloc = nalloc;
    } else {
      BITE->nalloc = 0;
    }
  }

  /* loop over all bite reports */
  for (int i = 0; i < BITE->number_reports; i++) {
    bitereport = &(BITE->bitereports[i]);
    for (int j = 0; j < 64; j++) {
      bitereport->source_name[j] = buffer[index];
      index++;
    }
    bitereport->source_address = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(bitereport->reserved));
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(bitereport->reserved2));
    index += 2;

    s7kTime = &(bitereport->downlink_time);
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7kTime->Seconds));
    index += 4;
    s7kTime->Hours = (mb_u_char)buffer[index];
    index++;
    s7kTime->Minutes = (mb_u_char)buffer[index];
    index++;

    s7kTime = &(bitereport->uplink_time);
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7kTime->Seconds));
    index += 4;
    s7kTime->Hours = (mb_u_char)buffer[index];
    index++;
    s7kTime->Minutes = (mb_u_char)buffer[index];
    index++;

    s7kTime = &(bitereport->bite_time);
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(s7kTime->Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(s7kTime->Seconds));
    index += 4;
    s7kTime->Hours = (mb_u_char)buffer[index];
    index++;
    s7kTime->Minutes = (mb_u_char)buffer[index];
    index++;

    bitereport->status = buffer[index];
    index++;
    mb_get_binary_short(true, &buffer[index], &(bitereport->number_bite));
    index += 2;
    for (int j = 0; j < 4; j++) {
      mb_get_binary_long(true, &buffer[index], &(bitereport->bite_status[j]));
      index += 8;
    }

    /* loop over all bite fields */
    for (int j = 0; j < bitereport->number_bite; j++) {
      bitefield = &(bitereport->bitefield[j]);

      mb_get_binary_short(true, &buffer[index], &(bitefield->field));
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
    store->type = R7KRECID_BITE;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_BITE:                          7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_BITE(verbose, BITE, error);

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
int mbr_reson7k3_rd_SonarSourceVersion(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSourceVersion *SonarSourceVersion = &(store->SonarSourceVersion);
  s7k3_header *header = &(SonarSourceVersion->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    SonarSourceVersion->version[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_SonarSourceVersion;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SonarSourceVersion:            7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarSourceVersion(verbose, SonarSourceVersion, error);

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
int mbr_reson7k3_rd_WetEndVersion8k(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_WetEndVersion8k *WetEndVersion8k = &(store->WetEndVersion8k);
  s7k3_header *header = &(WetEndVersion8k->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 32; i++) {
    WetEndVersion8k->version[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_WetEndVersion8k;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_WetEndVersion8k:           7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_WetEndVersion8k(verbose, WetEndVersion8k, error);

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
int mbr_reson7k3_rd_RawDetection(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_rawdetectiondata *rawdetectiondata;
  s7k3_bathydata *bathydata;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RawDetection *RawDetection = &(store->RawDetection);
  s7k3_header *header = &(RawDetection->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(RawDetection->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(RawDetection->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(RawDetection->multi_ping));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(RawDetection->number_beams));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RawDetection->data_field_size));
  index += 4;
  RawDetection->detection_algorithm = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(RawDetection->flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RawDetection->sampling_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RawDetection->tx_angle));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RawDetection->applied_roll));
  index += 4;
  for (int i = 0; i < 15; i++) {
    mb_get_binary_int(true, &buffer[index], &(RawDetection->reserved[i]));
    index += 4;
  }

  /* extract the data */
  for (int i = 0; i < RawDetection->number_beams; i++) {
    rawdetectiondata = (s7k3_rawdetectiondata *)&RawDetection->rawdetectiondata[i];
    mb_get_binary_short(true, &buffer[index], &(rawdetectiondata->beam_descriptor));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->detection_point));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->rx_angle));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(rawdetectiondata->flags));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(rawdetectiondata->quality));
    index += 4;
    if (RawDetection->data_field_size >= 22) {
      mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->uncertainty));
      index += 4;
    }
    if (RawDetection->data_field_size >= 26) {
      mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->signal_strength));
      index += 4;
    }
    if (RawDetection->data_field_size >= 30) {
      mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->min_limit));
      index += 4;
    }
    if (RawDetection->data_field_size >= 34) {
      mb_get_binary_float(true, &buffer[index], &(rawdetectiondata->max_limit));
      index += 4;
    }

    /* skip extra data if it exists */
    if (RawDetection->data_field_size > 34)
      index += RawDetection->data_field_size - 34;
  }

  /* get optional data - calculated bathymetry */
  if (header->OptionalDataOffset != 0) {
    RawDetection->optionaldata = true;
    index = header->OptionalDataOffset;

    mb_get_binary_float(true, &buffer[index], &(RawDetection->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(RawDetection->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(RawDetection->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->heading));
    index += 4;
    RawDetection->height_source = buffer[index];
    index += 1;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->tide));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->roll));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->pitch));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->heave));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(RawDetection->vehicle_depth));
    index += 4;
    for (int i = 0; i < RawDetection->number_beams; i++) {
      bathydata = (s7k3_bathydata *)&RawDetection->bathydata[i];
      mb_get_binary_float(true, &buffer[index], &(bathydata->depth));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->alongtrack));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->acrosstrack));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->pointing_angle));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->azimuth_angle));
      index += 4;
    }
  }
  else {
    RawDetection->optionaldata = false;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_RawDetection;

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
  for (int i = 0; i < RawDetection->number_beams; i++) {
    rawdetectiondata = (s7k3_rawdetectiondata *)&RawDetection->rawdetectiondata[i];
    if (rawdetectiondata->beam_descriptor > MBSYS_RESON7K_MAX_BEAMS) {
      status = MB_FAILURE;
      *error = MB_ERROR_UNINTELLIGIBLE;
    }
  }

/* print out the results */
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_RawDetection:                --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], RawDetection->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RawDetection(verbose, RawDetection, error);

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
int mbr_reson7k3_rd_Snippet(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_snippetdata *snippetdata;
  int time_j[5];
  int nsample;
  u32 nalloc;
  u16 *u16_ptr;
  u32 *u32_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Snippet *Snippet = &(store->Snippet);
  s7k3_header *header = &(Snippet->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(Snippet->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(Snippet->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(Snippet->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(Snippet->number_beams));
  index += 2;
  Snippet->error_flag = buffer[index];
  index++;
  Snippet->control_flags = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &(Snippet->flags));
  index += 4;
  for (int i = 0; i < 6; i++) {
    mb_get_binary_int(true, &buffer[index], &(Snippet->reserved[i]));
    index += 4;
  }

  /* loop over all beams to get Snippet parameters */
  for (int i = 0; i < Snippet->number_beams; i++) {
    snippetdata = (s7k3_snippetdata *)&(Snippet->snippetdata[i]);

    /* extract snippet data */
    mb_get_binary_short(true, &buffer[index], &(snippetdata->beam_number));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(snippetdata->begin_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippetdata->detect_sample));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(snippetdata->end_sample));
    index += 4;

    /* allocate memory for snippet data if needed */
    nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
    if ((Snippet->flags & 0x01) != 0) {
      nalloc = 4 * nsample;
    } else {
      nalloc = 2 * nsample;
    }
    if (status == MB_SUCCESS &&
        snippetdata->nalloc < nalloc) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, nalloc,
                             (void **)&(snippetdata->amplitude), error);
      if (status == MB_SUCCESS) {
        snippetdata->nalloc = nalloc;
      } else {
        snippetdata->nalloc = 0;
      }
    }
  }

  /* loop over all beams to get Snippet data */
  if (status == MB_SUCCESS) {
    if ((Snippet->flags & 0x01) != 0) {
      for (int i = 0; i < Snippet->number_beams; i++) {
        snippetdata = (s7k3_snippetdata *)&(Snippet->snippetdata[i]);
        u32_ptr = (u32 *)snippetdata->amplitude;
        nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
        for (int j = 0; j < nsample; j++) {
          mb_get_binary_int(true, &buffer[index], &(u32_ptr[j]));
          index += 4;
        }
      }
    }
    else {
      for (int i = 0; i < Snippet->number_beams; i++) {
        snippetdata = (s7k3_snippetdata *)&(Snippet->snippetdata[i]);
        u16_ptr = (u16 *)snippetdata->amplitude;
        nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
        for (int j = 0; j < nsample; j++) {
          mb_get_binary_short(true, &buffer[index], &(u16_ptr[j]));
          index += 2;
        }
      }
    }
  }

  /* get optional data - calculated bathymetry */
  if (header->OptionalDataOffset != 0) {
    Snippet->optionaldata = true;
    index = header->OptionalDataOffset;

    mb_get_binary_float(true, &buffer[index], &(Snippet->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(Snippet->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(Snippet->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(Snippet->heading));
    index += 4;
    for (int i = 0; i < Snippet->number_beams; i++) {
      mb_get_binary_float(true, &buffer[index], &(Snippet->beam_alongtrack[i]));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(Snippet->beam_acrosstrack[i]));
      index += 4;
      mb_get_binary_int(true, &buffer[index], &(Snippet->center_sample[i]));
      index += 4;
    }
  }
  else {
    Snippet->optionaldata = false;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_Snippet;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_Snippet:                     --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], Snippet->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Snippet(verbose, Snippet, error);

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
int mbr_reson7k3_rd_VernierProcessingDataFiltered(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VernierProcessingDataFiltered *VernierProcessingDataFiltered = &(store->VernierProcessingDataFiltered);
  s7k3_header *header = &(VernierProcessingDataFiltered->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_InstallationParameters(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_InstallationParameters *InstallationParameters = &(store->InstallationParameters);
  s7k3_header *header = &(InstallationParameters->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->frequency));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->firmware_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    InstallationParameters->firmware_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->software_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    InstallationParameters->software_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->s7k3_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    InstallationParameters->s7k3_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->protocal_version_len));
  index += 2;
  for (int i = 0; i < 128; i++) {
    InstallationParameters->protocal_version[i] = buffer[index];
    index++;
  }
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->transmit_heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->receive_heading));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->motion_heading));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->motion_time_delay));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->position_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->position_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->position_z));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(InstallationParameters->position_time_delay));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(InstallationParameters->waterline_z));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_INSTALLATION;
    store->type = R7KRECID_InstallationParameters;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_InstallationParameters:      7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_InstallationParameters(verbose, InstallationParameters, error);

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
int mbr_reson7k3_rd_BITESummary(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BITESummary *BITESummary = &(store->BITESummary);
  s7k3_header *header = &(BITESummary->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_CompressedBeamformedMagnitude(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CompressedBeamformedMagnitude *CompressedBeamformedMagnitude = &(store->CompressedBeamformedMagnitude);
  s7k3_header *header = &(CompressedBeamformedMagnitude->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_CompressedWaterColumn(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_compressedwatercolumndata *compressedwatercolumndata;
  size_t nread;
  int truncatebeams;
  int magnitudeonly;
  int eightbitmagphase;
  int downsampling;
  int downsamplingtype;
  int thirtytwobitdata;
  int compressionfactorvalid;
  int segmentnumbersvalid;
  int firstsamplerxdelay;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CompressedWaterColumn *CompressedWaterColumn = &(store->CompressedWaterColumn);
  s7k3_header *header = &(CompressedWaterColumn->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(CompressedWaterColumn->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(CompressedWaterColumn->multi_ping));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CompressedWaterColumn->number_beams));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->samples));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->compressed_samples));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->first_sample));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CompressedWaterColumn->sample_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CompressedWaterColumn->compression_factor));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CompressedWaterColumn->reserved));
  index += 4;

  /* calculate bytes per sample and allocate memory for time series if needed */
  /* Flags bit 0 : Use maximum bottom detection point in each beam to
            limit data. Data is included up to the bottom detection
            point + 10%. This flag has no effect on systems which
            do not perform bottom detection. */
  if (CompressedWaterColumn->flags & 0x0001)
    truncatebeams = true;
  else
    truncatebeams = false;

  /* Flags bit 1 : Include magnitude data only (strip phase) */
  if (CompressedWaterColumn->flags & 0x0002)
    magnitudeonly = true;
  else
    magnitudeonly = false;

  /* Flags bit 2 : Convert mag to dB, then compress from 16 bit to
            8 bit by truncation of 8 lower bits. Phase compression simply
            truncates lower (least significant) byte of phase data. */
  if (CompressedWaterColumn->flags & 0x0004)
    eightbitmagphase = true;
  else
    eightbitmagphase = false;

  /* Flags bit 3 : Reserved. 0x08) */

  /* Flags bit 4-7 : Downsampling divisor. Value = ((flags | 0xF0) >> 4). Only
          values 2-16 are valid. This field is ignored if downsampling
          is not enabled (type = “none”). */
  downsampling = ((CompressedWaterColumn->flags & 0x00F0) >> 4);

  /* Bit 8-11 : Downsampling type:
          0 (0x000) = None
          1 (0x100) = Middle value
          2 (0x200) = Peak value
          3 (0x300) = Average value */
  downsamplingtype = ((CompressedWaterColumn->flags & 0x0F00) >> 8);

  /* Bit 12 : 32 Bits data */
  if (CompressedWaterColumn->flags & 0x1000)
    thirtytwobitdata = true;
  else
    thirtytwobitdata = false;

  /* Bit 13 : Compression factor available */
  if (CompressedWaterColumn->flags & 0x2000)
    compressionfactorvalid = true;
  else
    compressionfactorvalid = false;

  /* Bit 14 : Segment numbers available */
  if (CompressedWaterColumn->flags & 0x4000)
    segmentnumbersvalid = true;
  else
    segmentnumbersvalid = false;

  /* Bit 15 : First sample contains RxDelay value. */
  if (CompressedWaterColumn->flags & 0x8000)
    firstsamplerxdelay = true;
  else
    firstsamplerxdelay = false;

  /* now calculate samplesize */
  if (thirtytwobitdata == true) {
    CompressedWaterColumn->magsamplesize = 4;
    if (magnitudeonly == true)
      CompressedWaterColumn->phasesamplesize = 0;
    else
      CompressedWaterColumn->phasesamplesize = 1;
  }
  else {
    if (eightbitmagphase == true)
      CompressedWaterColumn->magsamplesize = 1;
    else
      CompressedWaterColumn->magsamplesize = 2;
    if (magnitudeonly == true)
      CompressedWaterColumn->phasesamplesize = 0;
    else
      CompressedWaterColumn->phasesamplesize = CompressedWaterColumn->magsamplesize;
  }

  /* loop over all beams to get CompressedWaterColumn parameters */
  for (int i = 0; i < CompressedWaterColumn->number_beams; i++) {
    compressedwatercolumndata = (s7k3_compressedwatercolumndata *)&(CompressedWaterColumn->compressedwatercolumndata[i]);

    /* extract CompressedWaterColumn data */
    mb_get_binary_short(true, &buffer[index], &(compressedwatercolumndata->beam_number));
    index += 2;
    if (segmentnumbersvalid == true) {
      compressedwatercolumndata->segment_number = buffer[index];
      index += 1;
    }
    mb_get_binary_int(true, &buffer[index], &(compressedwatercolumndata->samples));
    index += 4;

    /* allocate memory for compressedwatercolumndata data if needed */
    nread = (CompressedWaterColumn->magsamplesize + CompressedWaterColumn->phasesamplesize)
              * compressedwatercolumndata->samples;
    if (status == MB_SUCCESS && compressedwatercolumndata->nalloc < nread) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, nread,
                             (void **)&(compressedwatercolumndata->data), error);
      if (status == MB_SUCCESS)
        compressedwatercolumndata->nalloc = nread;
      else
        compressedwatercolumndata->nalloc = 0;
    }

    /* get the time series */
    if (status == MB_SUCCESS) {
      memcpy(compressedwatercolumndata->data, &buffer[index], nread);
      index += nread;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_CompressedWaterColumn;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_CompressedWaterColumn:       --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], CompressedWaterColumn->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CompressedWaterColumn(verbose, CompressedWaterColumn, error);

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
int mbr_reson7k3_rd_SegmentedRawDetection(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata;
  s7k3_bathydata *bathydata;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = &(store->SegmentedRawDetection);
  s7k3_header *header = &(SegmentedRawDetection->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;

  mb_get_binary_short(true, &buffer[index], &(SegmentedRawDetection->record_header_size));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(SegmentedRawDetection->n_segments));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SegmentedRawDetection->segment_field_size));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(SegmentedRawDetection->n_rx));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SegmentedRawDetection->rx_field_size));
  index += 2;
  mb_get_binary_long(true, &buffer[index], &(SegmentedRawDetection->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(SegmentedRawDetection->ping_number));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(SegmentedRawDetection->multi_ping));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->rx_delay));
  index += 4;

  /* extract the data */
  for (int i = 0; i < SegmentedRawDetection->n_segments; i++) {
    segmentedrawdetectiontxdata = (s7k3_segmentedrawdetectiontxdata *)&SegmentedRawDetection->segmentedrawdetectiontxdata[i];
    mb_get_binary_short(true, &buffer[index], &(segmentedrawdetectiontxdata->segment_number));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_angle_along));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_angle_across));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_delay));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->frequency));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(segmentedrawdetectiontxdata->pulse_type));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->pulse_bandwidth));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_pulse_width));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_pulse_width_across));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_pulse_width_along));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_pulse_envelope));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_pulse_envelope_parameter));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->tx_relative_src_level));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->rx_beam_width));
    index += 4;
    segmentedrawdetectiontxdata->detection_algorithm = buffer[index];
    index += 1;
    mb_get_binary_int(true, &buffer[index], &(segmentedrawdetectiontxdata->flags));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->sampling_rate));
    index += 4;
    segmentedrawdetectiontxdata->tvg = buffer[index];
    index += 1;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectiontxdata->rx_bandwidth));
    index += 4;

    /* skip extra data if it exists */
    if (SegmentedRawDetection->segment_field_size > 68)
      index += SegmentedRawDetection->segment_field_size - 68;
  }

  for (int i = 0;i<SegmentedRawDetection->n_rx;i++) {
    segmentedrawdetectionrxdata = (s7k3_segmentedrawdetectionrxdata *)&(SegmentedRawDetection->segmentedrawdetectionrxdata[i]);
    mb_get_binary_short(true, &buffer[index], &(segmentedrawdetectionrxdata->beam_number));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(segmentedrawdetectionrxdata->used_segment));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectionrxdata->detection_point));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectionrxdata->rx_angle_cross));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(segmentedrawdetectionrxdata->flags2));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(segmentedrawdetectionrxdata->quality));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectionrxdata->uncertainty));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectionrxdata->signal_strength));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(segmentedrawdetectionrxdata->sn_ratio));
    index += 4;

    /* skip extra data if it exists */
    if (SegmentedRawDetection->rx_field_size > 32)
      index += SegmentedRawDetection->rx_field_size - 32;
  }

  /* get optional data - calculated bathymetry */
  if (header->OptionalDataOffset != 0) {
    SegmentedRawDetection->optionaldata = true;
    index = header->OptionalDataOffset;

    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->frequency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(SegmentedRawDetection->latitude));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(SegmentedRawDetection->longitude));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->heading));
    index += 4;
    SegmentedRawDetection->height_source = buffer[index];
    index += 1;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->tide));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->roll));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->pitch));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->heave));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(SegmentedRawDetection->vehicle_depth));
    index += 4;
    for (int i = 0; i < SegmentedRawDetection->n_rx; i++) {
      bathydata = (s7k3_bathydata *)&SegmentedRawDetection->bathydata[i];
      mb_get_binary_float(true, &buffer[index], &(bathydata->depth));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->alongtrack));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->acrosstrack));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->pointing_angle));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(bathydata->azimuth_angle));
      index += 4;
    }
  }
  else {
    SegmentedRawDetection->optionaldata = false;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_SegmentedRawDetection;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SegmentedRawDetection:       --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], SegmentedRawDetection->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SegmentedRawDetection(verbose, SegmentedRawDetection, error);

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
int mbr_reson7k3_rd_CalibratedBeam(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibratedBeam *CalibratedBeam = &(store->CalibratedBeam);
  s7k3_header *header = &(CalibratedBeam->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SystemEvents(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemEvents *SystemEvents = &(store->SystemEvents);
  s7k3_header *header = &(SystemEvents->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SystemEventMessage(int verbose, char *buffer, void *store_ptr, int *error) {
  int data_size;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemEventMessage *SystemEventMessage = &(store->SystemEventMessage);
  s7k3_header *header = &(SystemEventMessage->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(SystemEventMessage->serial_number));
  index += 8;
  mb_get_binary_short(true, &buffer[index], &(SystemEventMessage->event_id));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(SystemEventMessage->message_length));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(SystemEventMessage->event_identifier));
  index += 2;

  /* make sure enough memory is allocated for channel data */
  if (SystemEventMessage->message_alloc < SystemEventMessage->message_length) {
    data_size = SystemEventMessage->message_length + 1;
    status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SystemEventMessage->message), error);
    if (status == MB_SUCCESS) {
      SystemEventMessage->message_alloc = SystemEventMessage->message_length;
    }
    else {
      SystemEventMessage->message_alloc = 0;
      SystemEventMessage->message_length = 0;
    }
  }

  /* extract the data */
  for (int i = 0; i < SystemEventMessage->message_length; i++) {
    SystemEventMessage->message[i] = buffer[index];
    index++;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_COMMENT;
    store->type = R7KRECID_SystemEventMessage;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SystemEventMessage:  -comment- 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SystemEventMessage(verbose, SystemEventMessage, error);

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
int mbr_reson7k3_rd_RDRRecordingStatus(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RDRRecordingStatus *RDRRecordingStatus = &(store->RDRRecordingStatus);
  s7k3_header *header = &(RDRRecordingStatus->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_Subscriptions(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Subscriptions *Subscriptions = &(store->Subscriptions);
  s7k3_header *header = &(Subscriptions->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_RDRStorageRecording(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RDRStorageRecording *RDRStorageRecording = &(store->RDRStorageRecording);
  s7k3_header *header = &(RDRStorageRecording->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_CalibrationStatus(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibrationStatus *CalibrationStatus = &(store->CalibrationStatus);
  s7k3_header *header = &(CalibrationStatus->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_CalibratedSideScan(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibratedSideScan *CalibratedSideScan = &(store->CalibratedSideScan);
  s7k3_header *header = &(CalibratedSideScan->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SnippetBackscatteringStrength(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength = &(store->SnippetBackscatteringStrength);
  s7k3_header *header = &(SnippetBackscatteringStrength->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_MB2Status(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_MB2Status *MB2Status = &(store->MB2Status);
  s7k3_header *header = &(MB2Status->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_FileHeader(int verbose, char *buffer, void *store_ptr, int *error) {
  s7k3_subsystem *subsystem;
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_FileHeader *FileHeader = &(store->FileHeader);
  s7k3_header *header = &(FileHeader->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  for (int i = 0; i < 2; i++) {
    mb_get_binary_long(true, &buffer[index], &(FileHeader->file_identifier[i]));
    index += 8;
  }
  mb_get_binary_short(true, &buffer[index], &(FileHeader->version));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(FileHeader->reserved));
  index += 2;
  for (int i = 0; i < 2; i++) {
    mb_get_binary_long(true, &buffer[index], &(FileHeader->session_identifier[i]));
    index += 8;
  }
  mb_get_binary_int(true, &buffer[index], &(FileHeader->record_data_size));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(FileHeader->number_devices));
  index += 4;
  for (int i = 0; i < 64; i++) {
    FileHeader->recording_name[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 16; i++) {
    FileHeader->recording_version[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 64; i++) {
    FileHeader->user_defined_name[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < 128; i++) {
    FileHeader->notes[i] = buffer[index];
    index++;
  }
  for (int i = 0; i < FileHeader->number_devices; i++) {
    subsystem = &(FileHeader->subsystem[i]);
    mb_get_binary_int(true, &buffer[index], &(subsystem->device_identifier));
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(subsystem->system_enumerator));
    index += 2;
  }

  /* extract the optional data */
  if (header->OptionalDataOffset > 0) {
    index = header->OptionalDataOffset;
    FileHeader->optionaldata = true;
    mb_get_binary_int(true, &buffer[index], &(FileHeader->file_catalog_size));
    index += 4;
    mb_get_binary_long(true, &buffer[index], &(FileHeader->file_catalog_offset));
    index += 8;
  }
  else {
    FileHeader->optionaldata = false;
    FileHeader->file_catalog_size = 0;
    FileHeader->file_catalog_offset = 0;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADER;
    store->type = R7KRECID_FileHeader;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_FileHeader:                    7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FileHeader(verbose, FileHeader, error);

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
int mbr_reson7k3_chk_pingrecord(int verbose, int recordid, int *pingrecord) {
  assert(pingrecord != NULL);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:       %d\n", verbose);
    fprintf(stderr, "dbg2       recordid:      %d\n", recordid);
  }

  /* check ping number if one of the ping records */
  switch (recordid) {
    case R7KRECID_ProcessedSideScan:
    case R7KRECID_SonarSettings:
    case R7KRECID_MatchFilter:
    case R7KRECID_BeamGeometry:
    case R7KRECID_Bathymetry:
    case R7KRECID_SideScan:
    case R7KRECID_WaterColumn:
    case R7KRECID_VerticalDepth:
    case R7KRECID_TVG:
    case R7KRECID_Image:
    case R7KRECID_PingMotion:
    case R7KRECID_AdaptiveGate:
    case R7KRECID_DetectionDataSetup:
    case R7KRECID_Beamformed:
    case R7KRECID_VernierProcessingDataRaw:
    case R7KRECID_RawDetection:
    case R7KRECID_Snippet:
    case R7KRECID_VernierProcessingDataFiltered:
    case R7KRECID_CompressedBeamformedMagnitude:
    case R7KRECID_CompressedWaterColumn:
    case R7KRECID_SegmentedRawDetection:
    case R7KRECID_CalibratedBeam:
    case R7KRECID_CalibratedSideScan:
    case R7KRECID_SnippetBackscatteringStrength:
    case R7KRECID_RemoteControlSonarSettings:
      *pingrecord = true;
      break;
    default:
      *pingrecord = false;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Output arguments:\n");
    fprintf(stderr, "dbg2       pingrecord:    %d\n", *pingrecord);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:        %d\n", MB_SUCCESS);
  }

  return (MB_SUCCESS);
}
/*--------------------------------------------------------------------*/

int mbr_reson7k3_FileCatalog_compare(const void *a, const void *b) {
  int result = 0;

  s7k3_filecatalogdata *aa = (s7k3_filecatalogdata *) a;
  s7k3_filecatalogdata *bb = (s7k3_filecatalogdata *) b;

  // compare so that index table of data records is ordered correctly
  //  - The first record should be the 7200 FileHeader
  //  - Any comment records 7051 SystemEventMessage should be immediately after
  //    the FileHeader in reverse time order
  //  - The next records in order should be
  //      7022 SonarSourceVersion
  //      7001 Configuration
  //      7030 InstallationParameters
  //  - All other data records should be in time order, excepting that all records
  //    associated with a ping should be grouped together
  //  - Within a ping record group, the order is:
  //      7000 SonarSettings
  //      7503 RemoteControlSonarSettings
  //      7002 MatchFilter
  //      7004 BeamGeometry
  //      7027 RawDetection or 7047 SegmentedRawDetection
  //      7007 SideScan
  //      7057 R7KRECID_CalibratedSideScan
  //      7028 Snippet
  //      7058 R7KRECID_SnippetBackscatteringStrength
  //      7018 Beamformed
  //      7041 R7KRECID_CompressedBeamformedMagnitude
  //      7048 R7KRECID_CalibratedBeam
  //      7042 R7KRECID_CompressedWaterColumn
  //      3199 R7KRECID_ProcessedSideScan
  //
  //  - records associated with pings have nonzero ping_number values

  // deal with FileHeader
  if (aa->record_type == R7KRECID_FileHeader) {
    result = -1;
  }
  else if (bb->record_type == R7KRECID_FileHeader) {
    result = 1;
  }

  // deal with comments
  else if (aa->record_type == R7KRECID_SystemEventMessage
    && bb->record_type == R7KRECID_SystemEventMessage) {
    if (aa->time_d < bb->time_d)
      result = -1;
    else if (aa->time_d > bb->time_d)
      result = 1;
    else
      result = 0;
  }
  else if (aa->record_type == R7KRECID_SystemEventMessage) {
    result = -1;
  }
  else if (bb->record_type == R7KRECID_SystemEventMessage) {
    result = 1;
  }

  // deal with 7022 SonarSourceVersion
  else if (aa->record_type == R7KRECID_SonarSourceVersion
    && bb->record_type == R7KRECID_SonarSourceVersion) {
    if (aa->time_d < bb->time_d)
      result = -1;
    else if (aa->time_d > bb->time_d)
      result = 1;
    else
      result = 0;
  }
  else if (aa->record_type == R7KRECID_SonarSourceVersion) {
    result = -1;
  }
  else if (bb->record_type == R7KRECID_SonarSourceVersion) {
    result = 1;
  }

  // deal with 7001 Configuration
  else if (aa->record_type == R7KRECID_Configuration
    && bb->record_type == R7KRECID_Configuration) {
    if (aa->time_d < bb->time_d)
      result = -1;
    else if (aa->time_d > bb->time_d)
      result = 1;
    else
      result = 0;
  }
  else if (aa->record_type == R7KRECID_Configuration) {
    result = -1;
  }
  else if (bb->record_type == R7KRECID_Configuration) {
    result = 1;
  }

  // deal with two ping records
  else if (aa->pingrecord == true && bb->pingrecord == true) {
    // case of records from different pings
    if (aa->time_d < bb->time_d) {
      result = -1;
    }
    else if (aa->time_d > bb->time_d) {
      result = 1;
    }

    // case of records from the same ping
    else {
      if (aa->record_type == R7KRECID_SonarSettings)
        result = -1;
      else if (bb->record_type == R7KRECID_SonarSettings)
        result = 1;

      else if (aa->record_type == R7KRECID_RemoteControlSonarSettings)
        result = -1;
      else if (bb->record_type == R7KRECID_RemoteControlSonarSettings)
        result = 1;

      else if (aa->record_type == R7KRECID_MatchFilter)
        result = -1;
      else if (bb->record_type == R7KRECID_MatchFilter)
        result = 1;

      else if (aa->record_type == R7KRECID_BeamGeometry)
        result = -1;
      else if (bb->record_type == R7KRECID_BeamGeometry)
        result = 1;

      else if (aa->record_type == R7KRECID_RawDetection)
        result = -1;
      else if (bb->record_type == R7KRECID_RawDetection)
        result = 1;

      else if (aa->record_type == R7KRECID_SegmentedRawDetection)
        result = -1;
      else if (bb->record_type == R7KRECID_SegmentedRawDetection)
        result = 1;

      else if (aa->record_type == R7KRECID_SideScan)
        result = -1;
      else if (bb->record_type == R7KRECID_SideScan)
        result = 1;

      else if (aa->record_type == R7KRECID_CalibratedSideScan)
        result = -1;
      else if (bb->record_type == R7KRECID_CalibratedSideScan)
        result = 1;

      else if (aa->record_type == R7KRECID_Snippet)
        result = -1;
      else if (bb->record_type == R7KRECID_Snippet)
        result = 1;

      else if (aa->record_type == R7KRECID_SnippetBackscatteringStrength)
        result = -1;
      else if (bb->record_type == R7KRECID_SnippetBackscatteringStrength)
        result = 1;

      else if (aa->record_type == R7KRECID_Beamformed)
        result = -1;
      else if (bb->record_type == R7KRECID_Beamformed)
        result = 1;

      else if (aa->record_type == R7KRECID_CompressedBeamformedMagnitude)
        result = -1;
      else if (bb->record_type == R7KRECID_CompressedBeamformedMagnitude)
        result = 1;

      else if (aa->record_type == R7KRECID_CalibratedBeam)
        result = -1;
      else if (bb->record_type == R7KRECID_CalibratedBeam)
        result = 1;

      else if (aa->record_type == R7KRECID_CompressedWaterColumn)
        result = -1;
      else if (bb->record_type == R7KRECID_CompressedWaterColumn)
        result = 1;

      else if (aa->record_type == R7KRECID_ProcessedSideScan)
        result = -1;
      else if (bb->record_type == R7KRECID_ProcessedSideScan)
        result = 1;

      else
        result = 0;
    }
  }

  /* deal with all other pairs of data records - order by timestamp */
  else if (aa->time_d < bb->time_d)
    result = -1;
  else if (aa->time_d > bb->time_d)
    result = 1;
  else
    result = 0;

  return(result);
};
/*--------------------------------------------------------------------*/
int mbr_reson7k3_rd_FileCatalog(int verbose, char *buffer, void *store_ptr, int *error){
  s7k3_filecatalogdata *filecatalogdata;
  int time_j[5], time_i[7];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_FileCatalog *FileCatalog = &(store->FileCatalog_read);
  s7k3_header *header = &(FileCatalog->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_int(true, &buffer[index], &(FileCatalog->size));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(FileCatalog->version));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(FileCatalog->n));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(FileCatalog->reserved));
  index += 4;

  /* allocate memory for data record catalog if needed */
  if (status == MB_SUCCESS && FileCatalog->nalloc
      < FileCatalog->n * sizeof(s7k3_filecatalogdata)) {
    FileCatalog->nalloc = FileCatalog->n * sizeof(s7k3_filecatalogdata);
    if (status == MB_SUCCESS)
      status = mb_reallocd(verbose, __FILE__, __LINE__, FileCatalog->nalloc, (void **)&(FileCatalog->filecatalogdata), error);
    if (status != MB_SUCCESS) {
      FileCatalog->nalloc = 0;
    }
  }

  for (int i = 0; i < FileCatalog->n; i++) {
    filecatalogdata = &(FileCatalog->filecatalogdata[i]);
    filecatalogdata->sequence = i;
    mb_get_binary_int(true, &buffer[index], &(filecatalogdata->size));
    index += 4;
    mb_get_binary_long(true, &buffer[index], &(filecatalogdata->offset));
    index += 8;
    mb_get_binary_short(true, &buffer[index], &(filecatalogdata->record_type));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(filecatalogdata->device_id));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(filecatalogdata->system_enumerator));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(filecatalogdata->s7kTime.Year));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(filecatalogdata->s7kTime.Day));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(filecatalogdata->s7kTime.Seconds));
    index += 4;
    filecatalogdata->s7kTime.Hours = (mb_u_char)buffer[index];
    index++;
    filecatalogdata->s7kTime.Minutes = (mb_u_char)buffer[index];
    index++;
    mb_get_binary_int(true, &buffer[index], &(filecatalogdata->record_count));
    index += 4;
    for (int j = 0;j<8;j++) {
      mb_get_binary_short(true, &buffer[index], &(filecatalogdata->reserved[j]));
      index += 2;
    }

    // store time_d for sorting
    time_j[0] = filecatalogdata->s7kTime.Year;
    time_j[1] = filecatalogdata->s7kTime.Day;
    time_j[2] = 60 * filecatalogdata->s7kTime.Hours + filecatalogdata->s7kTime.Minutes;
    time_j[3] = (int)filecatalogdata->s7kTime.Seconds;
    time_j[4] = (int)(1000000 * (filecatalogdata->s7kTime.Seconds - time_j[3]));
    mb_get_itime(verbose, time_j, time_i);
    mb_get_time(verbose, time_i, &filecatalogdata->time_d);

    // store ping_number for sorting
    status = mbr_reson7k3_chk_pingrecord(verbose, filecatalogdata->record_type, &filecatalogdata->pingrecord);
  }

  // sort the data records, leaving the FileHeader record in place at the start
  // of the file, any comments just after the FileHeadeer, and then ordering by
  // timestamp while keeping ping related records together for each ping

  qsort((void *)FileCatalog->filecatalogdata, FileCatalog->n,
        sizeof(s7k3_filecatalogdata), (void *)mbr_reson7k3_FileCatalog_compare);

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADER;
    store->type = R7KRECID_FileCatalog;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_FileCatalog:             7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FileCatalog(verbose, FileCatalog, error);

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
int mbr_reson7k3_rd_TimeMessage(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_TimeMessage *TimeMessage = &(store->TimeMessage);
  s7k3_header *header = &(TimeMessage->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_RemoteControl(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControl *RemoteControl = &(store->RemoteControl);
  s7k3_header *header = &(RemoteControl->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_RemoteControlAcknowledge(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlAcknowledge *RemoteControlAcknowledge = &(store->RemoteControlAcknowledge);
  s7k3_header *header = &(RemoteControlAcknowledge->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_RemoteControlNotAcknowledge(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlNotAcknowledge *RemoteControlNotAcknowledge = &(store->RemoteControlNotAcknowledge);
  s7k3_header *header = &(RemoteControlNotAcknowledge->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_RemoteControlSonarSettings(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlSonarSettings *RemoteControlSonarSettings = &(store->RemoteControlSonarSettings);
  s7k3_header *header = &(RemoteControlSonarSettings->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(RemoteControlSonarSettings->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->ping_number));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->frequency));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->sample_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->receiver_bandwidth));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_width));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_type));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_envelope));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_envelope_par));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_mode));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->tx_pulse_reserved));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->max_ping_rate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->ping_period));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->range_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->power_selection));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->gain_selection));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->control_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->projector_id));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->steering_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->steering_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->beamwidth_vertical));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->beamwidth_horizontal));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->focal_point));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->projector_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->projector_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->transmit_flags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->hydrophone_id));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->rx_weighting));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->rx_weighting_par));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->rx_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->range_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->range_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->depth_minimum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->depth_maximum));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->absorption));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->spreading));
  index += 4;
  buffer[index] = RemoteControlSonarSettings->vernier_operation_mode;
  index ++;
  buffer[index] = RemoteControlSonarSettings->autofilter_window;
  index ++;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->tx_offset_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->tx_offset_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->tx_offset_z));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->head_tilt_x));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->head_tilt_y));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->head_tilt_z));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->ping_state));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->beam_angle_mode));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->s7kcenter_mode));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->gate_depth_min));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->gate_depth_max));
  index += 4;

  mb_get_binary_double(true, &buffer[index], &(RemoteControlSonarSettings->trigger_width));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(RemoteControlSonarSettings->trigger_offset));
  index += 8;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->projector_selection));
  index += 2;
  for (int i = 0;i<2;i++) {
    mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->reserved2[i]));
    index += 4;
  }
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->alternate_gain));
  index += 4;
  RemoteControlSonarSettings->vernier_filter = buffer[index];
  index ++;
  RemoteControlSonarSettings->reserved3 = buffer[index];
  index ++;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->custom_beams));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->coverage_angle));
  index += 4;
  RemoteControlSonarSettings->coverage_mode = buffer[index];
  index ++;
  RemoteControlSonarSettings->quality_filter = buffer[index];
  index ++;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->received_steering));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->flexmode_coverage));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->flexmode_steering));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->constant_spacing));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(RemoteControlSonarSettings->beam_mode));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->depth_gate_tilt));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(RemoteControlSonarSettings->applied_frequency));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(RemoteControlSonarSettings->element_number));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
    store->type = R7KRECID_RemoteControlSonarSettings;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_RemoteControlSonarSettings:  --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], RemoteControlSonarSettings->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RemoteControlSonarSettings(verbose, RemoteControlSonarSettings, error);

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
int mbr_reson7k3_rd_CommonSystemSettings(int verbose, char *buffer, void *store_ptr, int *error){
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CommonSystemSettings *CommonSystemSettings = &(store->CommonSystemSettings);
  s7k3_header *header = &(CommonSystemSettings->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_long(true, &buffer[index], &(CommonSystemSettings->serial_number));
  index += 8;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->ping_number));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->sound_velocity));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->absorption));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->spreading_loss));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->sequencer_control));
  index += 4;
  CommonSystemSettings->mru_format = buffer[index];
  index++;
  CommonSystemSettings->mru_baudrate = buffer[index];
  index++;
  CommonSystemSettings->mru_parity = buffer[index];
  index++;
  CommonSystemSettings->mru_databits = buffer[index];
  index++;
  CommonSystemSettings->mru_stopbits = buffer[index];
  index++;
  CommonSystemSettings->orientation = buffer[index];
  index++;
  CommonSystemSettings->record_version = buffer[index];
  index++;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->motion_latency));
  index += 4;
  CommonSystemSettings->svp_filter = buffer[index];
  index++;
  CommonSystemSettings->sv_override = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->activeenum));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->active_id));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->system_mode));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->masterslave_mode));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->tracker_flags));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->tracker_swathwidth));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_enable));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_obsize));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_sensitivity));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_detections));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_reserved[0]));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->multidetect_reserved[1]));
  index += 2;
  for (int i = 0;i<4;i++) {
    CommonSystemSettings->slave_ip[i] = buffer[index];
    index++;
  }
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->snippet_controlflags));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->snippet_minwindow));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->snippet_maxwindow));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->fullrange_dualhead));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->delay_multiplier));
  index += 4;
  CommonSystemSettings->powersaving_mode = buffer[index];
  index++;
  CommonSystemSettings->flags = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->range_blank));
  index += 2;
  CommonSystemSettings->startup_normalization = buffer[index];
  index++;
  CommonSystemSettings->restore_pingrate = buffer[index];
  index++;
  CommonSystemSettings->restore_power = buffer[index];
  index++;
  CommonSystemSettings->sv_interlock = buffer[index];
  index++;
  CommonSystemSettings->ignorepps_errors = buffer[index];
  index++;
  for (int i = 0;i<15;i++) {
    CommonSystemSettings->reserved1[i] = buffer[index];
    index++;
  }
  mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->compressed_wcflags));
  index += 4;
  CommonSystemSettings->deckmode = buffer[index];
  index++;
  CommonSystemSettings->reserved2 = buffer[index];
  index++;
  CommonSystemSettings->powermode_flags = buffer[index];
  index++;
  CommonSystemSettings->powermode_max = buffer[index];
  index++;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->water_temperature));
  index += 4;
  CommonSystemSettings->sensor_override = buffer[index];
  index++;
  CommonSystemSettings->sensor_dataflags = buffer[index];
  index++;
  CommonSystemSettings->sensor_active = buffer[index];
  index++;
  CommonSystemSettings->reserved3 = buffer[index];
  index++;
  mb_get_binary_float(true, &buffer[index], &(CommonSystemSettings->tracker_maxcoverage));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->dutycycle_mode));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(CommonSystemSettings->reserved4));
  index += 2;
  for (int i = 0;i<99;i++) {
    mb_get_binary_int(true, &buffer[index], &(CommonSystemSettings->reserved5[i]));
    index += 4;
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_PARAMETER;
    store->type = R7KRECID_CommonSystemSettings;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_CommonSystemSettings:  --7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) ping:%d size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], CommonSystemSettings->ping_number, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CommonSystemSettings(verbose, CommonSystemSettings, error);

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
int mbr_reson7k3_rd_SVFiltering(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SVFiltering *SVFiltering = &(store->SVFiltering);
  s7k3_header *header = &(SVFiltering->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SystemLockStatus(int verbose, char *buffer, void *store_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemLockStatus *SystemLockStatus = &(store->SystemLockStatus);
  s7k3_header *header = &(SystemLockStatus->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  //Notdone

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
int mbr_reson7k3_rd_SoundVelocity(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SoundVelocity *SoundVelocity = &(store->SoundVelocity);
  s7k3_header *header = &(SoundVelocity->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(SoundVelocity->soundvelocity));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SSV;
    store->type = R7KRECID_SoundVelocity;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SoundVelocity:                 7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SoundVelocity(verbose, SoundVelocity, error);

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
int mbr_reson7k3_rd_AbsorptionLoss(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_AbsorptionLoss *AbsorptionLoss = &(store->AbsorptionLoss);
  s7k3_header *header = &(AbsorptionLoss->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(AbsorptionLoss->absorptionloss));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_ABSORPTIONLOSS;
    store->type = R7KRECID_AbsorptionLoss;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_AbsorptionLoss:              7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_AbsorptionLoss(verbose, AbsorptionLoss, error);

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
int mbr_reson7k3_rd_SpreadingLoss(int verbose, char *buffer, void *store_ptr, int *error) {
  int time_j[5];

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SpreadingLoss *SpreadingLoss = &(store->SpreadingLoss);
  s7k3_header *header = &(SpreadingLoss->header);

  /* extract the header */
  int index = 0;
  int status = mbr_reson7k3_rd_header(verbose, buffer, &index, header, error);

  /* extract the data */
  index = header->Offset + 4;
  mb_get_binary_float(true, &buffer[index], &(SpreadingLoss->spreadingloss));
  index += 4;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SPREADINGLOSS;
    store->type = R7KRECID_SpreadingLoss;

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
#ifdef MBR_RESON7K3_DEBUG
  fprintf(stderr,
          "R7KRECID_SpreadingLoss:               7Ktime(%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d) RecordType:%d Size:%d "
          "index:%d\n",
          store->time_i[0], store->time_i[1], store->time_i[2], store->time_i[3], store->time_i[4], store->time_i[5],
          store->time_i[6], header->RecordType, header->Size, index);
#endif
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SpreadingLoss(verbose, SpreadingLoss, error);

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
int mbr_reson7k3_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  s7k3_header *header = NULL;
  s7k3_RawDetection *RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection;
  int skip;
  int ping_record;
  int time_j[5];
  size_t read_len;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  /* get saved values */
  int *save_flag = (int *)&mb_io_ptr->save_flag;
  int *current_ping = (int *)&mb_io_ptr->save14;
  int *last_ping = (int *)&mb_io_ptr->save1;
  int *new_ping = (int *)&mb_io_ptr->save2;
  int *recordid = (int *)&mb_io_ptr->save3;
  int *recordidlast = (int *)&mb_io_ptr->save4;
  char **bufferptr = (char **)&mb_io_ptr->saveptr1;
  char *buffer = (char *)*bufferptr;
  int *bufferalloc = (int *)&mb_io_ptr->save6;
  char **buffersaveptr = (char **)&mb_io_ptr->saveptr2;
  char *buffersave = (char *)*buffersaveptr;
  int *size = (int *)&mb_io_ptr->save8;
  int *nbadrec = (int *)&mb_io_ptr->save9;
  int *deviceid = (int *)&mb_io_ptr->save10;
  unsigned short *enumerator = (unsigned short *)&mb_io_ptr->save11;
  int *fileheaders = (int *)&mb_io_ptr->save12;
  double *last_7k_time_d = (double *)&mb_io_ptr->saved5;
  int *icatalog = (int *)&mb_io_ptr->save15;

  /* set file position */
  mb_io_ptr->file_pos = mb_io_ptr->file_bytes;

  /* loop over reading data until a record is ready for return */
  bool done = false;
  *error = MB_ERROR_NO_ERROR;
  while (!done) {
    /* if previously read record stored use it first */
    if (*save_flag == true) {
      *save_flag = false;
      mbr_reson7k3_chk_header(verbose, mbio_ptr, buffersave, recordid, deviceid, enumerator, size);
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
      mbr_reson7k3_chk_header(verbose, mbio_ptr, buffer, recordid, deviceid, enumerator, size);
    }
#endif

    /* else if reading from a file deal with possibility of corruption by
     * first finding the next sync block, then reading the heading, and then
     * finally reading the rest of the record */
    else {

      /* if FileCatalog has been read then set file pointer to read the next
          record header on the sorted list of records */
      if (store->FileCatalog_read.n > 0 && *icatalog < store->FileCatalog_read.n) {
        fseek(mb_io_ptr->mbfp, store->FileCatalog_read.filecatalogdata[*icatalog].offset, SEEK_SET);
        (*icatalog)++;
      }

      /* read next record header into buffer */
      read_len = (size_t)MBSYS_RESON7K_VERSIONSYNCSIZE;
      status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);

      /* check header - if not a good header read a byte
          at a time until a good header is found */
      skip = 0;
      while (status == MB_SUCCESS &&
             mbr_reson7k3_chk_header(verbose, mbio_ptr, buffer, recordid,
                                    deviceid, enumerator, size) != MB_SUCCESS) {
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
          fprintf(stderr, "\nThe MBF_reson7k3 module skipped data between identified\n\
data records. Something is broken, most probably the data...\n\
However, the data may include a data record type that we\n\
haven't seen yet, or there could be an error in the code.\n\
If skipped data are reported multiple times, \n\
we recommend you send a data sample and problem \n\
description to the MB-System team \n\
(caress@mbari.org and dale@ldeo.columbia.edu)\n\
Have a nice day...:                              %4.4X | %d\n", store->type, store->type);
        fprintf(stderr, "MBF_reson7k3 skipped %d bytes between records %4.4X:%d and %4.4X:%d\n", skip, *recordidlast,
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

#ifndef MBR_RESON7K3_DEBUG2
      if (skip > 0)
        fprintf(stderr, "reson7k3 record:skip:%d recordid:%x %d deviceid:%x %d enumerator:%x %d size:%d done:%d\n", skip,
                *recordid, *recordid, *deviceid, *deviceid, *enumerator, *enumerator, *size, done);
#endif
    }

    /* check for ping record and ping number */
    ping_record = false;
    if (status == MB_SUCCESS) {
      if (*recordid == R7KRECID_ProcessedSideScan
          || *recordid == R7KRECID_SonarSettings
          || *recordid == R7KRECID_MatchFilter
          || *recordid == R7KRECID_BeamGeometry
          || *recordid == R7KRECID_Bathymetry
          || *recordid == R7KRECID_SideScan
          || *recordid == R7KRECID_WaterColumn
          || *recordid == R7KRECID_VerticalDepth
          || *recordid == R7KRECID_TVG
          || *recordid == R7KRECID_Image
          || *recordid == R7KRECID_PingMotion
          || *recordid == R7KRECID_AdaptiveGate
          || *recordid == R7KRECID_DetectionDataSetup
          || *recordid == R7KRECID_Beamformed
          || *recordid == R7KRECID_VernierProcessingDataRaw
          || *recordid == R7KRECID_RawDetection
          || *recordid == R7KRECID_Snippet
          || *recordid == R7KRECID_VernierProcessingDataFiltered
          || *recordid == R7KRECID_CompressedBeamformedMagnitude
          || *recordid == R7KRECID_CompressedWaterColumn
          || *recordid == R7KRECID_SegmentedRawDetection
          || *recordid == R7KRECID_CalibratedBeam
          || *recordid == R7KRECID_CalibratedSideScan
          || *recordid == R7KRECID_SnippetBackscatteringStrength) {

        /* check for ping number */
        ping_record = true;
        mbr_reson7k3_chk_pingnumber(verbose, *recordid, buffer, new_ping);

        /* fix lack of ping number for beam geometry records */
        if (*recordid == R7KRECID_BeamGeometry && *new_ping <= 0)
          *new_ping = *last_ping;

        /* determine if record is continuation of the last ping
            or a new ping - if new ping and last ping not yet
            output then save the new record and output the
            last ping as fully read */
        if (*last_ping >= 0 && *new_ping >= 0 && *last_ping != *new_ping) {
          /* good ping if bathymetry record is read */
          if (store->read_RawDetection == true
              || store->read_SegmentedRawDetection == true) {
            done = true;
            store->kind = MB_DATA_DATA;
            *save_flag = true;
            *current_ping = *last_ping;
            *last_ping = -1;
            for (int i = 0; i < *size; i++)
              buffersave[i] = buffer[i];

            /* get the time */
            if (store->read_RawDetection == true) {
              RawDetection = &(store->RawDetection);
              header = &(RawDetection->header);
            }
            else if (store->read_SegmentedRawDetection == true) {
              SegmentedRawDetection = &(store->SegmentedRawDetection);
              header = &(SegmentedRawDetection->header);
            }
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
          store->read_ProcessedSideScan = false;
          store->read_SonarSettings = false;
          store->read_MatchFilter = false;
          store->read_BeamGeometry = false;
          store->read_Bathymetry = false;
          store->read_SideScan = false;
          store->read_WaterColumn = false;
          store->read_VerticalDepth = false;
          store->read_TVG = false;
          store->read_Image = false;
          store->read_PingMotion = false;
          store->read_DetectionDataSetup = false;
          store->read_Beamformed = false;
          store->read_VernierProcessingDataRaw = false;
          store->read_RawDetection = false;
          store->read_Snippet = false;
          store->read_VernierProcessingDataFiltered = false;
          store->read_CompressedBeamformedMagnitude = false;
          store->read_CompressedWaterColumn = false;
          store->read_SegmentedRawDetection = false;
          store->read_CalibratedBeam = false;
          store->read_CalibratedSideScan = false;
          store->read_SnippetBackscatteringStrength = false;
          store->read_RemoteControlSonarSettings = false;
        }
      }
    }

    /* check for ping data already read if FileCatalog encountered
        or if no FileCatalog read at start and any non-ping record encountered */
    if (status == MB_SUCCESS && *last_ping >= 0
        && (*recordid == R7KRECID_FileCatalog
            || (ping_record == false && store->FileCatalog_read.n > 0))) {
      /* good ping if bathymetry record is read */
      if (store->read_RawDetection == true
          || store->read_SegmentedRawDetection == true) {
        done = true;
        store->kind = MB_DATA_DATA;
        *save_flag = true;
        *current_ping = *last_ping;
        *last_ping = -1;
        for (int i = 0; i < *size; i++)
          buffersave[i] = buffer[i];

        /* get the time */
        if (store->read_RawDetection == true) {
          RawDetection = &(store->RawDetection);
          header = &(RawDetection->header);
        }
        else if (store->read_SegmentedRawDetection == true) {
          SegmentedRawDetection = &(store->SegmentedRawDetection);
          header = &(SegmentedRawDetection->header);
        }
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

    /* check for ping data already read in read error case */
    else if (status == MB_FAILURE && *last_ping >= 0) {
      status = MB_SUCCESS;
      *error = MB_ERROR_NO_ERROR;
      done = true;
      *save_flag = false;
      *last_ping = -1;
      store->kind = MB_DATA_DATA;
      store->time_d = *last_7k_time_d;
      mb_get_date(verbose, store->time_d, store->time_i);
    }

#ifdef MBR_RESON7K3_DEBUG2
    if (status == MB_SUCCESS && !done && *save_flag == false) {
      fprintf(stderr, "Reading record id: %4.4X  %4.4d | %4.4X  %4.4d | %4.4hX  %4.4d |", *recordid, *recordid, *deviceid,
              *deviceid, *enumerator, *enumerator);
      if (*recordid == R7KRECID_None)
        fprintf(stderr, " R7KRECID_None %d\n", *recordid);
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
      if (*recordid == R7KRECID_PanTilt)
        fprintf(stderr, " R7KRECID_PanTilt %d\n", *recordid);
      if (*recordid == R7KRECID_SonarInstallationIDs)
        fprintf(stderr, " R7KRECID_SonarInstallationIDs %d\n", *recordid);
      if (*recordid == R7KRECID_Mystery)
        fprintf(stderr, " R7KRECID_Mystery %d\n", *recordid);
      if (*recordid == R7KRECID_SonarPipeEnvironment)
        fprintf(stderr, " R7KRECID_SonarPipeEnvironment %d\n", *recordid);
      if (*recordid == R7KRECID_ContactOutput)
        fprintf(stderr, " R7KRECID_ContactOutput %d\n", *recordid);
      if (*recordid == R7KRECID_ProcessedSideScan)
        fprintf(stderr, " R7KRECID_ProcessedSideScan %d\n", *recordid);
      if (*recordid == R7KRECID_SonarSettings)
        fprintf(stderr, " R7KRECID_SonarSettings %d\n", *recordid);
      if (*recordid == R7KRECID_Configuration)
        fprintf(stderr, " R7KRECID_Configuration %d\n", *recordid);
      if (*recordid == R7KRECID_MatchFilter)
        fprintf(stderr, " R7KRECID_MatchFilter %d\n", *recordid);
      if (*recordid == R7KRECID_FirmwareHardwareConfiguration)
        fprintf(stderr, " R7KRECID_FirmwareHardwareConfiguration %d\n", *recordid);
      if (*recordid == R7KRECID_BeamGeometry)
        fprintf(stderr, " R7KRECID_BeamGeometry %d\n", *recordid);
      if (*recordid == R7KRECID_Bathymetry)
        fprintf(stderr, " R7KRECID_Bathymetry %d\n", *recordid);
      if (*recordid == R7KRECID_SideScan)
        fprintf(stderr, " R7KRECID_SideScan %d\n", *recordid);
      if (*recordid == R7KRECID_WaterColumn)
        fprintf(stderr, " R7KRECID_WaterColumn %d\n", *recordid);
      if (*recordid == R7KRECID_VerticalDepth)
        fprintf(stderr, " R7KRECID_VerticalDepth %d\n", *recordid);
      if (*recordid == R7KRECID_TVG)
        fprintf(stderr, " R7KRECID_TVG %d\n", *recordid);
      if (*recordid == R7KRECID_Image)
        fprintf(stderr, " R7KRECID_Image %d\n", *recordid);
      if (*recordid == R7KRECID_PingMotion)
        fprintf(stderr, " R7KRECID_PingMotion %d\n", *recordid);
      if (*recordid == R7KRECID_AdaptiveGate)
        fprintf(stderr, " R7KRECID_AdaptiveGate %d\n", *recordid);
      if (*recordid == R7KRECID_DetectionDataSetup)
        fprintf(stderr, " R7KRECID_DetectionDataSetup %d\n", *recordid);
      if (*recordid == R7KRECID_Beamformed)
        fprintf(stderr, " R7KRECID_Beamformed %d\n", *recordid);
      if (*recordid == R7KRECID_VernierProcessingDataRaw)
        fprintf(stderr, " R7KRECID_VernierProcessingDataRaw %d\n", *recordid);
      if (*recordid == R7KRECID_BITE)
        fprintf(stderr, " R7KRECID_BITE %d\n", *recordid);
      if (*recordid == R7KRECID_SonarSourceVersion)
        fprintf(stderr, " R7KRECID_SonarSourceVersion %d\n", *recordid);
      if (*recordid == R7KRECID_WetEndVersion8k)
        fprintf(stderr, " R7KRECID_WetEndVersion8k %d\n", *recordid);
      if (*recordid == R7KRECID_RawDetection)
        fprintf(stderr, " R7KRECID_RawDetection %d\n", *recordid);
      if (*recordid == R7KRECID_Snippet)
        fprintf(stderr, " R7KRECID_Snippet %d\n", *recordid);
      if (*recordid == R7KRECID_VernierProcessingDataFiltered)
        fprintf(stderr, " R7KRECID_VernierProcessingDataFiltered %d\n", *recordid);
      if (*recordid == R7KRECID_InstallationParameters)
        fprintf(stderr, " R7KRECID_InstallationParameters %d\n", *recordid);
      if (*recordid == R7KRECID_BITESummary)
        fprintf(stderr, " R7KRECID_BITESummary %d\n", *recordid);
      if (*recordid == R7KRECID_CompressedBeamformedMagnitude)
        fprintf(stderr, " R7KRECID_CompressedBeamformedMagnitude %d\n", *recordid);
      if (*recordid == R7KRECID_CompressedWaterColumn)
        fprintf(stderr, " R7KRECID_CompressedWaterColumn %d\n", *recordid);
      if (*recordid == R7KRECID_SegmentedRawDetection)
        fprintf(stderr, " R7KRECID_SegmentedRawDetection %d\n", *recordid);
      if (*recordid == R7KRECID_CalibratedBeam)
        fprintf(stderr, " R7KRECID_CalibratedBeam %d\n", *recordid);
      if (*recordid == R7KRECID_SystemEvents)
        fprintf(stderr, " R7KRECID_SystemEvents %d\n", *recordid);
      if (*recordid == R7KRECID_SystemEventMessage)
        fprintf(stderr, " R7KRECID_SystemEventMessage %d\n", *recordid);
      if (*recordid == R7KRECID_RDRRecordingStatus)
        fprintf(stderr, " R7KRECID_RDRRecordingStatus %d\n", *recordid);
      if (*recordid == R7KRECID_Subscriptions)
        fprintf(stderr, " R7KRECID_Subscriptions %d\n", *recordid);
      if (*recordid == R7KRECID_RDRStorageRecording)
        fprintf(stderr, " R7KRECID_RDRStorageRecording %d\n", *recordid);
      if (*recordid == R7KRECID_CalibrationStatus)
        fprintf(stderr, " R7KRECID_CalibrationStatus %d\n", *recordid);
      if (*recordid == R7KRECID_CalibratedSideScan)
        fprintf(stderr, " R7KRECID_CalibratedSideScan %d\n", *recordid);
      if (*recordid == R7KRECID_SnippetBackscatteringStrength)
        fprintf(stderr, " R7KRECID_SnippetBackscatteringStrength %d\n", *recordid);
      if (*recordid == R7KRECID_MB2Status)
        fprintf(stderr, " R7KRECID_MB2Status %d\n", *recordid);
      if (*recordid == R7KRECID_FileHeader)
        fprintf(stderr, " R7KRECID_FileHeader %d\n", *recordid);
      if (*recordid == R7KRECID_FileCatalog)
        fprintf(stderr, " R7KRECID_FileCatalog %d\n", *recordid);
      if (*recordid == R7KRECID_TimeMessage)
        fprintf(stderr, " R7KRECID_TimeMessage %d\n", *recordid);
      if (*recordid == R7KRECID_RemoteControl)
        fprintf(stderr, " R7KRECID_RemoteControl %d\n", *recordid);
      if (*recordid == R7KRECID_RemoteControlAcknowledge)
        fprintf(stderr, " R7KRECID_RemoteControlAcknowledge %d\n", *recordid);
      if (*recordid == R7KRECID_RemoteControlNotAcknowledge)
        fprintf(stderr, " R7KRECID_RemoteControlNotAcknowledge %d\n", *recordid);
      if (*recordid == R7KRECID_RemoteControlSonarSettings)
        fprintf(stderr, " R7KRECID_RemoteControlSonarSettings %d\n", *recordid);
      if (*recordid == R7KRECID_CommonSystemSettings)
        fprintf(stderr, " R7KRECID_CommonSystemSettings %d\n", *recordid);
      if (*recordid == R7KRECID_SVFiltering)
        fprintf(stderr, " R7KRECID_SVFiltering %d\n", *recordid);
      if (*recordid == R7KRECID_SystemLockStatus)
        fprintf(stderr, " R7KRECID_SystemLockStatus %d\n", *recordid);
      if (*recordid == R7KRECID_SoundVelocity)
        fprintf(stderr, " R7KRECID_SoundVelocity %d\n", *recordid);
      if (*recordid == R7KRECID_AbsorptionLoss)
        fprintf(stderr, " R7KRECID_AbsorptionLoss %d\n", *recordid);
      if (*recordid == R7KRECID_SpreadingLoss)
        fprintf(stderr, " R7KRECID_SpreadingLoss %d\n", *recordid);
    }
#endif

    /* set done if read failure */
    if (status == MB_FAILURE) {
#ifdef MBR_RESON7K3_DEBUG2
      fprintf(stderr, "call nothing, read failure:                              %4.4X | %d\n", store->type, store->type);
#endif
      done = true;
    }

    /* if possible and needed parse the data record now */
    if (status == MB_SUCCESS && !done) {

      if (*recordid == R7KRECID_ReferencePoint) {
        status = mbr_reson7k3_rd_ReferencePoint(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_ReferencePoint++;
        }
      }
      else if (*recordid == R7KRECID_UncalibratedSensorOffset) {
        status = mbr_reson7k3_rd_UncalibratedSensorOffset(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_UncalibratedSensorOffset++;
        }
      }
      else if (*recordid == R7KRECID_CalibratedSensorOffset) {
        status = mbr_reson7k3_rd_CalibratedSensorOffset(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_CalibratedSensorOffset++;
        }
      }
      else if (*recordid == R7KRECID_Position) {
        status = mbr_reson7k3_rd_Position(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Position++;
        }
      }
      else if (*recordid == R7KRECID_CustomAttitude) {
        status = mbr_reson7k3_rd_CustomAttitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_CustomAttitude++;
        }
      }
      else if (*recordid == R7KRECID_Tide) {
        status = mbr_reson7k3_rd_Tide(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Tide++;
        }
      }
      else if (*recordid == R7KRECID_Altitude) {
        status = mbr_reson7k3_rd_Altitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Altitude++;
        }
      }
      else if (*recordid == R7KRECID_MotionOverGround) {
        status = mbr_reson7k3_rd_MotionOverGround(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_MotionOverGround++;
        }
      }
      else if (*recordid == R7KRECID_Depth) {
        status = mbr_reson7k3_rd_Depth(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Depth++;
        }
      }
      else if (*recordid == R7KRECID_SoundVelocityProfile) {
        status = mbr_reson7k3_rd_SoundVelocityProfile(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SoundVelocityProfile++;
        }
      }
      else if (*recordid == R7KRECID_CTD) {
        status = mbr_reson7k3_rd_CTD(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_CTD++;
        }
      }
      else if (*recordid == R7KRECID_Geodesy) {
        status = mbr_reson7k3_rd_Geodesy(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Geodesy++;
        }
      }
      else if (*recordid == R7KRECID_RollPitchHeave) {
        status = mbr_reson7k3_rd_RollPitchHeave(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RollPitchHeave++;
        }
      }
      else if (*recordid == R7KRECID_Heading) {
        status = mbr_reson7k3_rd_Heading(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Heading++;
        }
      }
      else if (*recordid == R7KRECID_SurveyLine) {
        status = mbr_reson7k3_rd_SurveyLine(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SurveyLine++;
        }
      }
      else if (*recordid == R7KRECID_Navigation) {
        status = mbr_reson7k3_rd_Navigation(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Navigation++;
        }
      }
      else if (*recordid == R7KRECID_Attitude) {
        status = mbr_reson7k3_rd_Attitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Attitude++;
        }
      }
      else if (*recordid == R7KRECID_PanTilt) {
        status = mbr_reson7k3_rd_PanTilt(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_PanTilt++;
        }
      }
      else if (*recordid == R7KRECID_SonarInstallationIDs) {
        status = mbr_reson7k3_rd_SonarInstallationIDs(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SonarInstallationIDs++;
        }
      }
      else if (*recordid == R7KRECID_Mystery) {
        status = mbr_reson7k3_rd_Mystery(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Mystery++;
        }
      }
      else if (*recordid == R7KRECID_SonarPipeEnvironment) {
        status = mbr_reson7k3_rd_SonarPipeEnvironment(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SonarPipeEnvironment++;
        }
      }
      else if (*recordid == R7KRECID_ContactOutput) {
        status = mbr_reson7k3_rd_ContactOutput(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_ContactOutput++;
        }
      }
      else if (*recordid == R7KRECID_ProcessedSideScan) {
        status = mbr_reson7k3_rd_ProcessedSideScan(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_ProcessedSideScan++;
          store->read_ProcessedSideScan = true;
        }
      }
      else if (*recordid == R7KRECID_SonarSettings) {
        status = mbr_reson7k3_rd_SonarSettings(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_SonarSettings++;
          store->read_SonarSettings = true;
        }
      }
      else if (*recordid == R7KRECID_Configuration) {
        status = mbr_reson7k3_rd_Configuration(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Configuration++;
        }
      }
      else if (*recordid == R7KRECID_MatchFilter) {
        status = mbr_reson7k3_rd_MatchFilter(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_MatchFilter++;
          store->read_MatchFilter = true;
        }
      }
      else if (*recordid == R7KRECID_FirmwareHardwareConfiguration) {
        status = mbr_reson7k3_rd_FirmwareHardwareConfiguration(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_FirmwareHardwareConfiguration++;
        }
      }
      else if (*recordid == R7KRECID_BeamGeometry) {
        status = mbr_reson7k3_rd_BeamGeometry(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_BeamGeometry++;
          store->read_BeamGeometry = true;
        }
      }
      else if (*recordid == R7KRECID_Bathymetry) {
        status = mbr_reson7k3_rd_Bathymetry(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_Bathymetry++;
          store->read_Bathymetry = true;
        }
      }
      else if (*recordid == R7KRECID_SideScan) {
        status = mbr_reson7k3_rd_SideScan(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_SideScan++;
          store->read_SideScan = true;
        }
      }
      else if (*recordid == R7KRECID_WaterColumn) {
        status = mbr_reson7k3_rd_WaterColumn(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_WaterColumn++;
          store->read_WaterColumn = true;
        }
      }
      else if (*recordid == R7KRECID_VerticalDepth) {
        status = mbr_reson7k3_rd_VerticalDepth(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_VerticalDepth++;
          store->read_VerticalDepth = true;
        }
      }
      else if (*recordid == R7KRECID_TVG) {
        status = mbr_reson7k3_rd_TVG(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_TVG++;
          store->read_TVG = true;
        }
      }
      else if (*recordid == R7KRECID_Image) {
        status = mbr_reson7k3_rd_Image(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_Image++;
          store->read_Image = true;
        }
      }
      else if (*recordid == R7KRECID_PingMotion) {
        status = mbr_reson7k3_rd_PingMotion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_PingMotion++;
          store->read_PingMotion = true;
        }
      }
      else if (*recordid == R7KRECID_AdaptiveGate) {
        status = mbr_reson7k3_rd_AdaptiveGate(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_AdaptiveGate++;
        }
      }
      else if (*recordid == R7KRECID_DetectionDataSetup) {
        status = mbr_reson7k3_rd_DetectionDataSetup(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_DetectionDataSetup++;
          store->read_DetectionDataSetup = true;
        }
      }
      else if (*recordid == R7KRECID_Beamformed) {
        status = mbr_reson7k3_rd_Beamformed(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_Beamformed++;
          store->read_Beamformed = true;
        }
      }
      else if (*recordid == R7KRECID_VernierProcessingDataRaw) {
        status = mbr_reson7k3_rd_VernierProcessingDataRaw(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_VernierProcessingDataRaw++;
          store->read_VernierProcessingDataRaw = true;
        }
      }
      else if (*recordid == R7KRECID_BITE) {
        status = mbr_reson7k3_rd_BITE(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_BITE++;
        }
      }
      else if (*recordid == R7KRECID_SonarSourceVersion) {
        status = mbr_reson7k3_rd_SonarSourceVersion(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SonarSourceVersion++;
        }
      }
      else if (*recordid == R7KRECID_WetEndVersion8k) {
        status = mbr_reson7k3_rd_WetEndVersion8k(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_WetEndVersion8k++;
        }
      }
      else if (*recordid == R7KRECID_RawDetection) {
        status = mbr_reson7k3_rd_RawDetection(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_RawDetection++;
          store->read_RawDetection = true;
        }
      }
      else if (*recordid == R7KRECID_Snippet) {
      status = mbr_reson7k3_rd_Snippet(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_Snippet++;
          store->read_Snippet = true;
        }
      }
      else if (*recordid == R7KRECID_VernierProcessingDataFiltered) {
        status = mbr_reson7k3_rd_VernierProcessingDataFiltered(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_VernierProcessingDataFiltered++;
          store->read_VernierProcessingDataFiltered = true;
        }
      }
      else if (*recordid == R7KRECID_InstallationParameters) {
        status = mbr_reson7k3_rd_InstallationParameters(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_InstallationParameters++;
        }
      }
      else if (*recordid == R7KRECID_BITESummary) {
        status = mbr_reson7k3_rd_BITESummary(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_BITESummary++;
        }
      }
      else if (*recordid == R7KRECID_CompressedBeamformedMagnitude) {
        status = mbr_reson7k3_rd_CompressedBeamformedMagnitude(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_CompressedBeamformedMagnitude++;
          store->read_CompressedBeamformedMagnitude = true;
        }
      }
      else if (*recordid == R7KRECID_CompressedWaterColumn) {
        status = mbr_reson7k3_rd_CompressedWaterColumn(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_CompressedWaterColumn++;
          store->read_CompressedWaterColumn = true;
        }
      }
      else if (*recordid == R7KRECID_SegmentedRawDetection) {
        status = mbr_reson7k3_rd_SegmentedRawDetection(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_SegmentedRawDetection++;
          store->read_SegmentedRawDetection = true;
        }
      }
      else if (*recordid == R7KRECID_CalibratedBeam) {
        status = mbr_reson7k3_rd_CalibratedBeam(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_CalibratedBeam++;
          store->read_CalibratedBeam = true;
        }
      }
      else if (*recordid == R7KRECID_SystemEvents) {
        status = mbr_reson7k3_rd_SystemEvents(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SystemEvents++;
        }
      }
      else if (*recordid == R7KRECID_SystemEventMessage) {
        status = mbr_reson7k3_rd_SystemEventMessage(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SystemEventMessage++;
        }
      }
      else if (*recordid == R7KRECID_RDRRecordingStatus) {
        status = mbr_reson7k3_rd_RDRRecordingStatus(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RDRRecordingStatus++;
        }
      }
      else if (*recordid == R7KRECID_Subscriptions) {
        status = mbr_reson7k3_rd_Subscriptions(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_Subscriptions++;
        }
      }
      else if (*recordid == R7KRECID_RDRStorageRecording) {
        status = mbr_reson7k3_rd_RDRStorageRecording(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RDRStorageRecording++;
        }
      }
      else if (*recordid == R7KRECID_CalibrationStatus) {
        status = mbr_reson7k3_rd_CalibrationStatus(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_CalibrationStatus++;
        }
      }
      else if (*recordid == R7KRECID_CalibratedSideScan) {
        status = mbr_reson7k3_rd_CalibratedSideScan(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_CalibratedSideScan++;
          store->read_CalibratedSideScan = true;
        }
      }
      else if (*recordid == R7KRECID_SnippetBackscatteringStrength) {
        status = mbr_reson7k3_rd_SnippetBackscatteringStrength(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_SnippetBackscatteringStrength++;
          store->read_SnippetBackscatteringStrength = true;
        }
      }
      else if (*recordid == R7KRECID_MB2Status) {
        status = mbr_reson7k3_rd_MB2Status(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_MB2Status++;
        }
      }
      else if (*recordid == R7KRECID_FileHeader) {
        status = mbr_reson7k3_rd_FileHeader(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          (*fileheaders)++;
          done = true;
          store->nrec_FileHeader++;
        }
//mbsys_reson7k3_print_FileHeader(verbose, &store->FileHeader, error);

        // If the FileHeader record indicates the file ends with a FileCatalog
        // record, then jump to the end of file, read the FileCatalog, and jump
        // back to the current location.
        if (status == MB_SUCCESS && store->FileHeader.optionaldata == true
            && store->FileHeader.file_catalog_size > 0
            && store->FileHeader.file_catalog_offset > 0
            && mb_io_ptr->mbfp != NULL) {
          // save current file location
          int fpos_current = ftell(mb_io_ptr->mbfp);

          // move to start of FileCatalog record
          int fstatus = fseek(mb_io_ptr->mbfp, store->FileHeader.file_catalog_offset, SEEK_SET);

          // Most of the time the FileHeader.file_catalog_size value is the size
          // of the entire FileCatalog record as per the format spec, but sometimes
          // it is just the size of the catalog list at 48 bytes per entry.
          // This has been documented in sample Hydrosweep data from R/V Polarstern.
          // Check for cases where the size is an even multiple of 48 - in these
          // cases add 82 bytes so the entire record is read
          if (store->FileHeader.file_catalog_size % 48 == 0) {
            store->FileHeader.file_catalog_size += MBSYS_RESON7K_RECORDHEADER_SIZE
                                                  + R7KHDRSIZE_FileCatalog
                                                  + MBSYS_RESON7K_RECORDTAIL_SIZE;
          }

          /* allocate memory to read record if necessary */
          if (*bufferalloc < store->FileHeader.file_catalog_size) {
            status = mb_reallocd(verbose, __FILE__, __LINE__, store->FileHeader.file_catalog_size, (void **)bufferptr, error);
            if (status == MB_SUCCESS)
              status = mb_reallocd(verbose, __FILE__, __LINE__, store->FileHeader.file_catalog_size, (void **)buffersaveptr, error);
            if (status != MB_SUCCESS) {
              *bufferalloc = 0;
              done = true;
            }
            else {
              *bufferalloc = store->FileHeader.file_catalog_size;
              buffer = (char *)*bufferptr;
              buffersave = (char *)*buffersaveptr;
            }
          }

          // read the entire record into the buffer
          if (status == MB_SUCCESS) {
            read_len = (size_t)(store->FileHeader.file_catalog_size);
            status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
          }

          // parse the FileCatalog record
          if (status == MB_SUCCESS) {
//mbsys_reson7k3_print_FileHeader(verbose, &store->FileHeader, error);
            status = mbr_reson7k3_rd_FileCatalog(verbose, buffer, store_ptr, error);
            if (status == MB_SUCCESS) {
              store->nrec_FileCatalog = 1;
            }
//mbsys_reson7k3_print_FileCatalog(verbose, &store->FileCatalog_read, error);
          }

          // reset kind and type to FileHeader
          store->kind = MB_DATA_HEADER;
          store->type = R7KRECID_FileHeader;

          // reset file position
          fstatus = fseek(mb_io_ptr->mbfp, fpos_current, SEEK_SET);
          *icatalog = 1;

        }
      }
      else if (*recordid == R7KRECID_FileCatalog) {
        //status = mbr_reson7k3_rd_FileCatalog(verbose, buffer, store_ptr, error);
        //if (status == MB_SUCCESS) {
        //  done = true;
        //  store->nrec_FileCatalog = 1;
        //}
//mbsys_reson7k3_print_FileCatalog(verbose, &store->FileCatalog_read, error);
      }
      else if (*recordid == R7KRECID_TimeMessage) {
        status = mbr_reson7k3_rd_TimeMessage(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_TimeMessage++;
        }
      }
      else if (*recordid == R7KRECID_RemoteControl) {
        status = mbr_reson7k3_rd_RemoteControl(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RemoteControl++;
        }
      }
      else if (*recordid == R7KRECID_RemoteControlAcknowledge) {
        status = mbr_reson7k3_rd_RemoteControlAcknowledge(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RemoteControlAcknowledge++;
        }
      }
      else if (*recordid == R7KRECID_RemoteControlNotAcknowledge) {
        status = mbr_reson7k3_rd_RemoteControlNotAcknowledge(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_RemoteControlNotAcknowledge++;
        }
      }
      else if (*recordid == R7KRECID_RemoteControlSonarSettings) {
        status = mbr_reson7k3_rd_RemoteControlSonarSettings(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          store->nrec_RemoteControlSonarSettings++;
          store->read_RemoteControlSonarSettings = true;
        }
      }
      else if (*recordid == R7KRECID_CommonSystemSettings) {
        status = mbr_reson7k3_rd_CommonSystemSettings(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_CommonSystemSettings++;
        }
      }
      else if (*recordid == R7KRECID_SVFiltering) {
        status = mbr_reson7k3_rd_SVFiltering(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SVFiltering++;
        }
      }
      else if (*recordid == R7KRECID_SystemLockStatus) {
        status = mbr_reson7k3_rd_SystemLockStatus(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SystemLockStatus++;
        }
      }
      else if (*recordid == R7KRECID_SoundVelocity) {
        status = mbr_reson7k3_rd_SoundVelocity(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SoundVelocity++;
        }
      }
      else if (*recordid == R7KRECID_AbsorptionLoss) {
        status = mbr_reson7k3_rd_AbsorptionLoss(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_AbsorptionLoss++;
        }
      }
      else if (*recordid == R7KRECID_SpreadingLoss) {
      status = mbr_reson7k3_rd_SpreadingLoss(verbose, buffer, store_ptr, error);
        if (status == MB_SUCCESS) {
          done = true;
          store->nrec_SpreadingLoss++;
        }
      }
    }

#ifdef MBR_RESON7K3_DEBUG2
    if (status == MB_SUCCESS && ping_record == true) {
      fprintf(stderr,"recordid:%d ping_record:%d last_ping:%d new_ping:%d current_ping:%d done:%d status:%d error:%d\n",
          *recordid, ping_record,*last_ping,*new_ping,*current_ping,done,status,*error);
      fprintf(stderr, "current ping:%d records read: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
              *new_ping, store->read_ProcessedSideScan, store->read_SonarSettings,
              store->read_MatchFilter, store->read_BeamGeometry,
              store->read_Bathymetry, store->read_SideScan,
              store->read_WaterColumn, store->read_VerticalDepth,
              store->read_TVG, store->read_Image,
              store->read_PingMotion, store->read_DetectionDataSetup,
              store->read_Beamformed, store->read_VernierProcessingDataRaw,
              store->read_RawDetection, store->read_Snippet,
              store->read_VernierProcessingDataFiltered, store->read_CompressedBeamformedMagnitude,
              store->read_CompressedWaterColumn, store->read_SegmentedRawDetection,
              store->read_CalibratedBeam, store->read_CalibratedSideScan,
              store->read_SnippetBackscatteringStrength,
              store->read_RemoteControlSonarSettings);
    }
#endif

    /* bail out if there is a parsing error */
    if (status == MB_FAILURE)
      done = true;
#ifdef MBR_RESON7K3_DEBUG2
    if (verbose >= 0) {
      fprintf(stderr, "---Read record id: %4.4X  %4.4d | recordid:%x size:%d\n", store->type, store->type, *recordid, *size);
      fprintf(stderr, "end of mbr_reson7k3_rd_data loop: done:%d kind:%d status:%d error:%d\n", done, store->kind, status, *error);
    }
#endif
  }
#ifdef MBR_RESON7K3_DEBUG2
  if (status == MB_SUCCESS)
    fprintf(stderr, "RESON7K3 DATA READ: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

  /* get file position - check file and socket, use appropriate ftell */
  if (mb_io_ptr->mbfp != NULL) {
       if (*save_flag == true)
          mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp) - *size;
      else
          mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);
  }
#ifdef MBTRN_ENABLED
  else if (mb_io_ptr->mbsp != NULL) {
      if (*save_flag == true)
          mb_io_ptr->file_bytes = r7kr_reader_tell(mb_io_ptr->mbsp) - *size;
      else
          mb_io_ptr->file_bytes = r7kr_reader_tell(mb_io_ptr->mbsp);
  } else {
      fprintf(stderr,"ERROR - both file and socket input pointers are NULL:                              %4.4X | %d\n", store->type, store->type);
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
int mbr_rt_reson7k3(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  int *preprocess_pars_set;
  struct mb_preprocess_struct *preprocess_pars;
  int *platform_set;
	struct mb_platform_struct **platform_ptr = NULL;
  double soundspeed;
  int *asynch_source_nav = NULL;
  int *asynch_source_sensordepth = NULL;
  int *asynch_source_heading = NULL;
  int *asynch_source_attitude = NULL;
  int *asynch_source_altitude = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  /* read next data from file */
  int status = mbr_reson7k3_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* get pointers to data structures */
  s7k3_Position *Position = &store->Position;
  s7k3_CustomAttitude *CustomAttitude = &store->CustomAttitude;
  s7k3_Altitude *Altitude = &store->Altitude;
  s7k3_Depth *Depth = &store->Depth;
  s7k3_RollPitchHeave *RollPitchHeave = &store->RollPitchHeave;
  s7k3_Heading *Heading = &store->Heading;
  s7k3_Navigation *Navigation = &store->Navigation;
  s7k3_Attitude *Attitude = &store->Attitude;
  s7k3_SonarSettings *SonarSettings = &store->SonarSettings;
  s7k3_RawDetection *RawDetection = &store->RawDetection;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = &store->SegmentedRawDetection;
  preprocess_pars_set = (int *)&mb_io_ptr->save13;
  preprocess_pars = (struct mb_preprocess_struct *)&mb_io_ptr->preprocess_pars;
  platform_set = (int *)&mb_io_ptr->save7;
  platform_ptr = (struct mb_platform_struct **)&mb_io_ptr->saveptr3;
  asynch_source_nav = (int *)&mb_io_ptr->save16;
  asynch_source_sensordepth = (int *)&mb_io_ptr->save17;
  asynch_source_heading = (int *)&mb_io_ptr->save18;
  asynch_source_attitude = (int *)&mb_io_ptr->save19;
  asynch_source_altitude = (int *)&mb_io_ptr->save20;

  // Use the following asynchronous data source priority order:
  //    Position lon lat -
  //      Navigation      1015 MB_DATA_NAV
  //      Position        1003 MB_DATA_NAV1
  //    Sensor depth -
  //      Navigation      1015 MB_DATA_NAV - IF height_accuracy is reasonable
  //      Depth           1008 MB_DATA_SONARDEPTH - IF depth_descriptor=0 ==> depth to sensor value
  //      Position        1003 MB_DATA_NAV1 - IF height_accuracy is reasonable
  //    Heading -
  //      Navigation      1015 MB_DATA_NAV
  //      Heading         1013 MB_DATA_HEADING
  //      CustomAttitude  1004 MB_DATA_ATTITUDE2
  //    Roll pitch heave -
  //      Attitude        1016 MB_DATA_ATTITUDE - also includes heading
  //      RollPitchHeave  1012 MB_DATA_ATTITUDE1
  //      CustomAttitude  1004 MB_DATA_ATTITUDE2 - also includes heading
  //    Altitude:
  //      Altitude        1006 MB_DATA_ALTITUDE
  //

  // deal with buffering asynchronous data if status == MB_SUCCESS
  if (status == MB_SUCCESS) {

    // save position, sensordepth, heading if Navigation record
    if (store->kind == MB_DATA_NAV) {
      Navigation = &(store->Navigation);

      // add position (clear old data from other sources if needed)
      if (*asynch_source_nav != MB_DATA_NAV) {
        *asynch_source_nav = MB_DATA_NAV;
        mb_io_ptr->nfix = 0;
      }
      mb_navint_add(verbose, mbio_ptr, store->time_d,
                    (double)(RTD * Navigation->longitude),
                    (double)(RTD * Navigation->latitude), error);

      // add heading (clear old data from other sources if needed)
      if (*asynch_source_heading != MB_DATA_NAV) {
        *asynch_source_heading = MB_DATA_NAV;
        mb_io_ptr->nheading = 0;
      }
      mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d),
                    (double)(RTD * Navigation->heading), error);

      // add sensordepth (clear old data from other sources if needed)
      if (*asynch_source_sensordepth != MB_DATA_NAV) {
        *asynch_source_sensordepth = MB_DATA_NAV;
        mb_io_ptr->nsonardepth = 0;
      }
      mb_depint_add(verbose, mbio_ptr, (double)(store->time_d),
                      (double)(-Navigation->height), error);
    }

    /* save Attitude if Attitude record */
    else if (store->kind == MB_DATA_ATTITUDE) {
      Attitude = &(store->Attitude);

      // add attitude (clear old data from other sources if needed)
      if (*asynch_source_attitude != MB_DATA_ATTITUDE) {
        *asynch_source_attitude = MB_DATA_ATTITUDE;
        mb_io_ptr->nattitude = 0;
      }
      for (int i = 0; i < Attitude->n; i++) {
        mb_attint_add(verbose, mbio_ptr, (double)(store->time_d + 0.001 * ((double)Attitude->delta_time[i])),
                      (double)(Attitude->heave[i]), (double)(RTD * Attitude->roll[i]), (double)(RTD * Attitude->pitch[i]),
                      error);
      }

      // add heading (clear old data from other sources if needed)
      if (*asynch_source_heading != MB_DATA_ATTITUDE) {
        *asynch_source_heading = MB_DATA_ATTITUDE;
        mb_io_ptr->nheading = 0;
      }
      for (int i = 0; i < Attitude->n; i++) {
        mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d + 0.001 * ((double)Attitude->delta_time[i])),
                      (double)(RTD * Attitude->heading[i]), error);
      }
    }

    // save position if Position record and no higher priority source already encountered
    else if (store->kind == MB_DATA_NAV1) {
      Position = &(store->Position);

      // add position (clear old data from other sources if needed)
      if (*asynch_source_nav == MB_DATA_NONE) {
        *asynch_source_nav = MB_DATA_NAV1;
        mb_io_ptr->nfix = 0;
      }
      if (*asynch_source_nav == MB_DATA_NAV1) {
        mb_navint_add(verbose, mbio_ptr, store->time_d,
                      (double)(RTD * Position->longitude_easting),
                      (double)(RTD * Position->latitude_northing), error);
      }
    }

    // save heading if Heading record and no higher priority source already encountered
    else if (store->kind == MB_DATA_HEADING) {
      Heading = &(store->Heading);

      // add heading (clear old data from other sources if needed)
      if (*asynch_source_heading == MB_DATA_NONE
          || *asynch_source_heading == MB_DATA_ATTITUDE2) {
        *asynch_source_heading = MB_DATA_HEADING;
        mb_io_ptr->nheading = 0;
      }
      if (*asynch_source_heading == MB_DATA_HEADING) {
        mb_hedint_add(verbose, mbio_ptr, (double)(store->time_d),
                      (double)(RTD * Heading->heading), error);
      }
    }

    /* save attitude if RollPitchHeave record */
    else if (store->kind == MB_DATA_ATTITUDE1) {
      RollPitchHeave = &(store->RollPitchHeave);

      // add attitude (clear old data from other sources if needed)
      if (*asynch_source_attitude == MB_DATA_NONE
        || *asynch_source_attitude == MB_DATA_ATTITUDE2) {
        *asynch_source_attitude = MB_DATA_ATTITUDE1;
        mb_io_ptr->nattitude = 0;
      }
      if (*asynch_source_attitude == MB_DATA_ATTITUDE1) {
        mb_attint_add(verbose, mbio_ptr, (double)(store->time_d),
                      (double)(RollPitchHeave->heave),
                      (double)(RTD * RollPitchHeave->roll),
                      (double)(RTD * RollPitchHeave->pitch), error);
      }
    }

    /* save attitude if CustomAttitude record */
    else if (store->kind == MB_DATA_ATTITUDE2) {
      CustomAttitude = &(store->CustomAttitude);

      // add attitude (clear old data from other sources if needed)
      if (*asynch_source_attitude == MB_DATA_NONE) {
        *asynch_source_attitude = MB_DATA_ATTITUDE2;
        mb_io_ptr->nattitude = 0;
      }
      if (*asynch_source_attitude == MB_DATA_ATTITUDE2) {
        for (int i = 0; i < CustomAttitude->n; i++) {
          mb_attint_add(verbose, mbio_ptr,
                    (double)(store->time_d + ((double)i)
                              / ((double)CustomAttitude->frequency)),
                    (double)(CustomAttitude->heave[i]),
                    (double)(RTD * CustomAttitude->roll[i]),
                    (double)(RTD * CustomAttitude->pitch[i]), error);
        }
      }

      // add heading (clear old data from other sources if needed)
      if (*asynch_source_heading == MB_DATA_NONE) {
        *asynch_source_heading = MB_DATA_ATTITUDE2;
        mb_io_ptr->nheading = 0;
      }
      if (*asynch_source_heading == MB_DATA_ATTITUDE2) {
        for (int i = 0; i < CustomAttitude->n; i++) {
          mb_hedint_add(verbose, mbio_ptr,
                    (double)(store->time_d + ((double)i) / ((double)CustomAttitude->frequency)),
                    (double)(RTD * CustomAttitude->heading[i]), error);
        }
      }
    }

    /* save sonardepth if Depth record showing depth of sensor */
    else if (store->kind == MB_DATA_SONARDEPTH) {
      Depth = &(store->Depth);

      // add sensordepth (clear old data from other sources if needed)
      if (*asynch_source_sensordepth == MB_DATA_NONE) {
        *asynch_source_sensordepth = MB_DATA_SONARDEPTH;
        mb_io_ptr->nsonardepth = 0;
      }
      if (*asynch_source_sensordepth == MB_DATA_SONARDEPTH) {
        mb_depint_add(verbose, mbio_ptr, (double)(store->time_d),
                      (double)(Depth->depth), error);
      }
    }

    /* save altitude if Altitude record */
    else if (store->kind == MB_DATA_ALTITUDE) {
      Altitude = &(store->Altitude);

      // add altitude (clear old data from other sources if needed)
      if (*asynch_source_altitude == MB_DATA_NONE) {
        *asynch_source_altitude = MB_DATA_ALTITUDE;
        mb_io_ptr->naltitude = 0;
      }
      if (*asynch_source_altitude == MB_DATA_ALTITUDE) {
        mb_altint_add(verbose, mbio_ptr, (double)(store->time_d),
                      (double)(Altitude->altitude), error);
      }
    }

  } // end of dealing with asynchronous data

#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
    fprintf(stderr, "Record returned: type:%d status:%d error:%d\n\n", store->kind, status, *error);
#endif

  /* if needed calculate bathymetry using preprocess function */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA
      && (  (store->read_RawDetection == true
              && RawDetection->optionaldata == false)
            || (store->read_SegmentedRawDetection == true
              && SegmentedRawDetection->optionaldata == false))) {
    /* get platform model if needed */
    if (*platform_set == false) {
      status = mbsys_reson7k3_extract_platform(verbose, mbio_ptr, store_ptr, &store->kind, (void **)platform_ptr, error);
      *platform_set = true;
    }

    /* set preprocess parameters if needed - have to update counts of ancilliary data arrays each time */
    if (*preprocess_pars_set == false) {
      preprocess_pars->target_sensor = 0;

      preprocess_pars->timestamp_changed = false;
      preprocess_pars->time_d = 0.0;

      preprocess_pars->n_nav = mb_io_ptr->nfix;
      preprocess_pars->nav_time_d = mb_io_ptr->fix_time_d;
      preprocess_pars->nav_lon = mb_io_ptr->fix_lon;
      preprocess_pars->nav_lat = mb_io_ptr->fix_lat;
      preprocess_pars->nav_speed = NULL;

      preprocess_pars->n_sensordepth = mb_io_ptr->nsonardepth;
      preprocess_pars->sensordepth_time_d = mb_io_ptr->sonardepth_time_d;
      preprocess_pars->sensordepth_sensordepth = mb_io_ptr->sonardepth_sonardepth;

      preprocess_pars->n_heading = mb_io_ptr->nheading;
      preprocess_pars->heading_time_d = mb_io_ptr->heading_time_d;
      preprocess_pars->heading_heading = mb_io_ptr->heading_heading;

      preprocess_pars->n_altitude = mb_io_ptr->naltitude;
      preprocess_pars->altitude_time_d = mb_io_ptr->altitude_time_d;
      preprocess_pars->altitude_altitude = mb_io_ptr->altitude_altitude;

      preprocess_pars->n_attitude = mb_io_ptr->nattitude;
      preprocess_pars->attitude_time_d = mb_io_ptr->attitude_time_d;
      preprocess_pars->attitude_roll = mb_io_ptr->attitude_roll;
      preprocess_pars->attitude_pitch = mb_io_ptr->attitude_pitch;
      preprocess_pars->attitude_heave = mb_io_ptr->attitude_heave;

      preprocess_pars->n_soundspeed = 1;
      soundspeed = SonarSettings->sound_velocity;
      preprocess_pars->soundspeed_time_d = &store->time_d;
      preprocess_pars->soundspeed_soundspeed = &soundspeed;

      preprocess_pars->no_change_survey = false;
      preprocess_pars->multibeam_sidescan_source = MB_PR_SSSOURCE_SNIPPET;
      preprocess_pars->modify_soundspeed = false;
      preprocess_pars->recalculate_bathymetry = true;
      preprocess_pars->sounding_amplitude_filter = false;
      preprocess_pars->sounding_amplitude_threshold = 0.0;
      preprocess_pars->sounding_altitude_filter = false;
      preprocess_pars->sounding_target_altitude = 0.0;
      preprocess_pars->ignore_water_column = false;
      preprocess_pars->head1_offsets = false;
      preprocess_pars->head1_offsets_x = 0.0;
      preprocess_pars->head1_offsets_y = 0.0;
      preprocess_pars->head1_offsets_z = 0.0;
      preprocess_pars->head1_offsets_heading = 0.0;
      preprocess_pars->head1_offsets_roll = 0.0;
      preprocess_pars->head1_offsets_pitch = 0.0;
      preprocess_pars->head2_offsets = false;
      preprocess_pars->head2_offsets_x = 0.0;
      preprocess_pars->head2_offsets_y = 0.0;
      preprocess_pars->head2_offsets_z = 0.0;
      preprocess_pars->head2_offsets_heading = 0.0;
      preprocess_pars->head2_offsets_roll = 0.0;
      preprocess_pars->head2_offsets_pitch = 0.0;

      preprocess_pars->n_kluge = 0;
    } else {
      preprocess_pars->n_nav = mb_io_ptr->nfix;
      preprocess_pars->n_sensordepth = mb_io_ptr->nsonardepth;
      preprocess_pars->n_heading = mb_io_ptr->nheading;
      preprocess_pars->n_altitude = mb_io_ptr->naltitude;
      preprocess_pars->n_attitude = mb_io_ptr->nattitude;
    }

    status = mbsys_reson7k3_preprocess(verbose, mbio_ptr, store_ptr,
                *platform_ptr, preprocess_pars, error);
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
int mbr_reson7k3_FileCatalog_update(int verbose, void *mbio_ptr, void *store_ptr, int size, void *header_ptr, int *error) {
  s7k3_filecatalogdata *filecatalogdata = NULL;

  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);
  assert(size > 0);
  assert(header_ptr != NULL);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:      %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:     %p\n", mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:    %p\n", store_ptr);
    fprintf(stderr, "dbg2       size:         %d\n", size);
    fprintf(stderr, "dbg2       header_ptr:   %p\n", header_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)mb_io_ptr->store_data;

  /* get pointers to data structures */
  s7k3_header *header = (s7k3_header *)header_ptr;
  s7k3_FileCatalog *FileCatalog = (s7k3_FileCatalog *)&store->FileCatalog_write;

  int status = MB_SUCCESS;

  /* allocate memory for data record catalog if needed */
  if (FileCatalog->nalloc < (FileCatalog->n + 1) * sizeof(s7k3_filecatalogdata)) {
    FileCatalog->nalloc = (FileCatalog->n + 1000) * sizeof(s7k3_filecatalogdata);
    status = mb_reallocd(verbose, __FILE__, __LINE__, FileCatalog->nalloc, (void **)&(FileCatalog->filecatalogdata), error);
    if (status != MB_SUCCESS) {
      FileCatalog->nalloc = 0;
    }
  }
  // Add a new entry for a data record about to be written to the output file
  filecatalogdata = &FileCatalog->filecatalogdata[FileCatalog->n];
  filecatalogdata->sequence = FileCatalog->n;
  int time_j[5];
  time_j[0] = header->s7kTime.Year;
  time_j[1] = header->s7kTime.Day;
  time_j[2] = 60 * header->s7kTime.Hours + header->s7kTime.Minutes;
  time_j[3] = (int)header->s7kTime.Seconds;
  time_j[4] = (int)(1000000 * (header->s7kTime.Seconds - time_j[3]));
  int time_i[7];
  mb_get_itime(verbose, time_j, time_i);
  mb_get_time(verbose, time_i, &(filecatalogdata->time_d));
  mbr_reson7k3_chk_pingrecord(verbose, header->RecordType, &filecatalogdata->pingrecord);
  filecatalogdata->size = size;
  filecatalogdata->offset = ftell(mb_io_ptr->mbfp);
  filecatalogdata->record_type = header->RecordType;
  filecatalogdata->device_id = header->DeviceId;
  filecatalogdata->system_enumerator = header->SystemEnumerator;
  filecatalogdata->s7kTime.Year = header->s7kTime.Year;
  filecatalogdata->s7kTime.Day = header->s7kTime.Day;
  filecatalogdata->s7kTime.Seconds = header->s7kTime.Seconds;
  filecatalogdata->s7kTime.Hours = header->s7kTime.Hours;
  filecatalogdata->s7kTime.Minutes = header->s7kTime.Minutes;
  if (filecatalogdata->pingrecord == true)
    filecatalogdata->record_count = 1;
  else
    filecatalogdata->record_count = 0;
  for (int i=0; i<8; i++) {
    filecatalogdata->reserved[i] = 0;
  }
  FileCatalog->n++;

#ifdef MBR_RESON7K3_DEBUG
fprintf(stderr, "^^>Update FileCatalog list: File %s Line %d type:%d n:%d\n", __FILE__, __LINE__, header->RecordType, FileCatalog->n);
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
int mbr_reson7k3_wr_ReferencePoint(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_ReferencePoint *ReferencePoint = &(store->ReferencePoint);
  s7k3_header *header = &(ReferencePoint->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_ReferencePoint(verbose, ReferencePoint, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_ReferencePoint;

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
    buffer = (char *)*bufferptr;

    /* insert the header */
    index = 0;
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    mb_put_binary_float(false, ReferencePoint->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, ReferencePoint->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, ReferencePoint->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, ReferencePoint->water_z, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_UncalibratedSensorOffset(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_UncalibratedSensorOffset *UncalibratedSensorOffset = &(store->UncalibratedSensorOffset);
  s7k3_header *header = &(UncalibratedSensorOffset->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_UncalibratedSensorOffset(verbose, UncalibratedSensorOffset, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, UncalibratedSensorOffset->offset_yaw, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CalibratedSensorOffset(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibratedSensorOffset *CalibratedSensorOffset = &(store->CalibratedSensorOffset);
  s7k3_header *header = &(CalibratedSensorOffset->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CalibratedSensorOffset(verbose, CalibratedSensorOffset, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CalibratedSensorOffset->offset_yaw, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Position(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Position *Position = &(store->Position);
  s7k3_header *header = &(Position->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Position(verbose, Position, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(false, Position->datum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Position->latency, &buffer[index]);
    index += 4;
    mb_put_binary_double(false, Position->latitude_northing, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Position->longitude_easting, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Position->height, &buffer[index]);
    index += 8;
    buffer[index] = Position->type;
    index++;
    buffer[index] = Position->utm_zone;
    index++;
    buffer[index] = Position->quality;
    index++;
    buffer[index] = Position->method;
    index++;
    buffer[index] = Position->nsat;
    index++;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CustomAttitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CustomAttitude *CustomAttitude = &(store->CustomAttitude);
  s7k3_header *header = &(CustomAttitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CustomAttitude(verbose, CustomAttitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CustomAttitude;
  if (CustomAttitude->fieldmask & 1)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 2)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 4)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 8)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 16)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 32)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 64)
    *size += CustomAttitude->n * sizeof(float);
  if (CustomAttitude->fieldmask & 128)
    *size += CustomAttitude->n * sizeof(float);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    CustomAttitude->fieldmask = (mb_u_char)buffer[index];
    index++;
    CustomAttitude->reserved = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(false, CustomAttitude->n, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, CustomAttitude->frequency, &buffer[index]);
    index += 4;

    if (CustomAttitude->fieldmask & 1)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->pitch[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 2)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->roll[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 4)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->heading[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 8)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->heave[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 16)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->pitchrate[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 32)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->rollrate[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 64)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->headingrate[i], &buffer[index]);
        index += 4;
      }
    if (CustomAttitude->fieldmask & 128)
      for (int i = 0; i < CustomAttitude->n; i++) {
        mb_put_binary_float(false, CustomAttitude->heaverate[i], &buffer[index]);
        index += 4;
      }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Tide(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Tide *Tide = &(store->Tide);
  s7k3_header *header = &(Tide->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Tide(verbose, Tide, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, Tide->tide, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Tide->source, &buffer[index]);
    index += 2;
    buffer[index] = Tide->flags;
    index++;
    mb_get_binary_short(true, &buffer[index], &(Tide->gauge));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(Tide->datum));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(Tide->latency));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(Tide->latitude_northing));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(Tide->longitude_easting));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(Tide->height));
    index += 8;
    buffer[index] = Tide->type;
    index++;
    buffer[index] = Tide->utm_zone;
    index++;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Altitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Altitude *Altitude = &(store->Altitude);
  s7k3_header *header = &(Altitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Altitude(verbose, Altitude, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, Altitude->altitude, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_MotionOverGround(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_MotionOverGround *MotionOverGround = &(store->MotionOverGround);
  s7k3_header *header = &(MotionOverGround->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_MotionOverGround(verbose, MotionOverGround, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_MotionOverGround;
  if (MotionOverGround->flags & 1)
    *size += 3 * MotionOverGround->n * sizeof(float);
  if (MotionOverGround->flags & 2)
    *size += 3 * MotionOverGround->n * sizeof(float);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    MotionOverGround->flags = (mb_u_char)buffer[index];
    index++;
    MotionOverGround->reserved = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(false, MotionOverGround->n, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, MotionOverGround->frequency, &buffer[index]);
    index += 4;

    if (MotionOverGround->flags & 1) {
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->x[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->y[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->z[i], &buffer[index]);
        index += 4;
      }
    }
    if (MotionOverGround->flags & 2) {
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->xa[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->ya[i], &buffer[index]);
        index += 4;
      }
      for (int i = 0; i < MotionOverGround->n; i++) {
        mb_put_binary_float(false, MotionOverGround->za[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Depth(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Depth *Depth = &(store->Depth);
  s7k3_header *header = &(Depth->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Depth(verbose, Depth, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    Depth->descriptor = (mb_u_char)buffer[index];
    index++;
    Depth->correction = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(false, Depth->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, Depth->depth, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SoundVelocityProfile(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SoundVelocityProfile *SoundVelocityProfile = &(store->SoundVelocityProfile);
  s7k3_header *header = &(SoundVelocityProfile->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SoundVelocityProfile(verbose, SoundVelocityProfile, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SoundVelocityProfile;
  *size += R7KRDTSIZE_SoundVelocityProfile * SoundVelocityProfile->n;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    SoundVelocityProfile->position_flag = (mb_u_char)buffer[index];
    index++;
    SoundVelocityProfile->reserved1 = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_short(false, SoundVelocityProfile->reserved2, &buffer[index]);
    index += 2;
    mb_put_binary_double(false, SoundVelocityProfile->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, SoundVelocityProfile->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, SoundVelocityProfile->n, &buffer[index]);
    index += 4;

    for (int i = 0; i < SoundVelocityProfile->n; i++) {
      mb_put_binary_float(false, SoundVelocityProfile->depth[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SoundVelocityProfile->sound_velocity[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CTD(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CTD *CTD = &(store->CTD);
  s7k3_header *header = &(CTD->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CTD(verbose, CTD, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CTD;
  *size += CTD->n * R7KRDTSIZE_CTD;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, CTD->frequency, &buffer[index]);
    index += 4;
    buffer[index] = CTD->velocity_source_flag;
    index++;
    buffer[index] = CTD->velocity_algorithm;
    index++;
    buffer[index] = CTD->conductivity_flag;
    index++;
    buffer[index] = CTD->pressure_flag;
    index++;
    buffer[index] = CTD->position_flag;
    index++;
    buffer[index] = CTD->validity;
    index++;
    mb_put_binary_short(false, CTD->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_double(false, CTD->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, CTD->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(false, CTD->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CTD->n, &buffer[index]);
    index += 4;

    for (int i = 0; i < CTD->n; i++) {
      mb_put_binary_float(false, CTD->conductivity_salinity[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, CTD->temperature[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, CTD->pressure_depth[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, CTD->sound_velocity[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, CTD->absorption[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Geodesy(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Geodesy *Geodesy = &(store->Geodesy);
  s7k3_header *header = &(Geodesy->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Geodesy(verbose, Geodesy, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      Geodesy->spheroid[i] = (mb_u_char)buffer[index];
      index++;
    }
    mb_put_binary_double(false, Geodesy->semimajoraxis, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->flattening, &buffer[index]);
    index += 8;
    for (int i = 0; i < 16; i++) {
      Geodesy->reserved1[i] = (mb_u_char)buffer[index];
      index++;
    }
    for (int i = 0; i < 32; i++) {
      Geodesy->datum[i] = (mb_u_char)buffer[index];
      index++;
    }
    mb_put_binary_int(false, Geodesy->calculation_method, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, Geodesy->number_parameters, &buffer[index]);
    index += 4;
    mb_put_binary_double(false, Geodesy->dx, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->dy, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->dz, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->rx, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->ry, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->rz, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->scale, &buffer[index]);
    index += 8;
    for (int i = 0; i < 35; i++) {
      Geodesy->reserved2[i] = (mb_u_char)buffer[index];
      index++;
    }
    for (int i = 0; i < 32; i++) {
      Geodesy->grid_name[i] = (mb_u_char)buffer[index];
      index++;
    }
    Geodesy->distance_units = (mb_u_char)buffer[index];
    index++;
    Geodesy->angular_units = (mb_u_char)buffer[index];
    index++;
    mb_put_binary_double(false, Geodesy->latitude_origin, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->central_meridian, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->false_easting, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->false_northing, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Geodesy->central_scale_factor, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, Geodesy->custom_identifier, &buffer[index]);
    index += 4;
    for (int i = 0; i < 50; i++) {
      Geodesy->reserved3[i] = (mb_u_char)buffer[index];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RollPitchHeave(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RollPitchHeave *RollPitchHeave = &(store->RollPitchHeave);
  s7k3_header *header = &(RollPitchHeave->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RollPitchHeave(verbose, RollPitchHeave, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, RollPitchHeave->roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RollPitchHeave->pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RollPitchHeave->heave, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Heading(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Heading *Heading = &(store->Heading);
  s7k3_header *header = &(Heading->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Heading(verbose, Heading, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, Heading->heading, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SurveyLine(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SurveyLine *SurveyLine = &(store->SurveyLine);
  s7k3_header *header = &(SurveyLine->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SurveyLine(verbose, SurveyLine, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SurveyLine;
  *size += SurveyLine->n * R7KRDTSIZE_SurveyLine;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_short(false, SurveyLine->n, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, SurveyLine->type, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, SurveyLine->turnradius, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = (char)SurveyLine->name[i];
      index++;
    }
    for (int i = 0; i < SurveyLine->n; i++) {
      mb_put_binary_double(false, SurveyLine->latitude_northing[i], &buffer[index]);
      index += 8;
      mb_put_binary_double(false, SurveyLine->longitude_easting[i], &buffer[index]);
      index += 8;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Navigation(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Navigation *Navigation = &(store->Navigation);
  s7k3_header *header = &(Navigation->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Navigation(verbose, Navigation, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    buffer[index] = (mb_u_char)Navigation->vertical_reference;
    index++;
    mb_put_binary_double(false, Navigation->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, Navigation->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(false, Navigation->position_accuracy, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Navigation->height, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Navigation->height_accuracy, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Navigation->speed, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Navigation->course, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Navigation->heading, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Attitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Attitude *Attitude = &(store->Attitude);
  s7k3_header *header = &(Attitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Attitude(verbose, Attitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Attitude;
  *size += Attitude->n * R7KRDTSIZE_Attitude;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    buffer[index] = Attitude->n;
    index++;
    for (int i = 0; i < Attitude->n; i++) {
      mb_put_binary_short(false, Attitude->delta_time[i], &buffer[index]);
      index += 2;
      mb_put_binary_float(false, Attitude->roll[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Attitude->pitch[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Attitude->heave[i], &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Attitude->heading[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_PanTilt(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_PanTilt *PanTilt = &(store->PanTilt);
  s7k3_header *header = &(PanTilt->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_PanTilt(verbose, PanTilt, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_PanTilt;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SonarInstallationIDs(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarInstallationIDs *SonarInstallationIDs = &(store->SonarInstallationIDs);
  s7k3_header *header = &(SonarInstallationIDs->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarInstallationIDs(verbose, SonarInstallationIDs, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SonarInstallationIDs;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Mystery(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Mystery *Mystery = &(store->Mystery);
  s7k3_header *header = &(Mystery->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Mystery(verbose, Mystery, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Mystery;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < R7KHDRSIZE_Mystery; i++) {
      buffer[index] = Mystery->data[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SonarPipeEnvironment(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarPipeEnvironment *SonarPipeEnvironment = &(store->SonarPipeEnvironment);
  s7k3_header *header = &(SonarPipeEnvironment->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarPipeEnvironment(verbose, SonarPipeEnvironment, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SonarPipeEnvironment;
  *size += SonarPipeEnvironment->n * R7KRDTSIZE_SonarPipeEnvironment;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_ContactOutput(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_ContactOutput *ContactOutput = &(store->ContactOutput);
  s7k3_header *header = &(ContactOutput->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_ContactOutput(verbose, ContactOutput, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_ContactOutput;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_ProcessedSideScan(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_ProcessedSideScan *ProcessedSideScan = &(store->ProcessedSideScan);
  s7k3_header *header = &(ProcessedSideScan->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_ProcessedSideScan(verbose, ProcessedSideScan, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_ProcessedSideScan;
  *size +=  2 * sizeof(float) * ProcessedSideScan->number_pixels;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, ProcessedSideScan->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, ProcessedSideScan->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, ProcessedSideScan->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, ProcessedSideScan->recordversion, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, ProcessedSideScan->ss_source, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, ProcessedSideScan->number_pixels, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, ProcessedSideScan->ss_type, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, ProcessedSideScan->pixelwidth, &buffer[index]);
    index += 4;
    mb_put_binary_double(false, ProcessedSideScan->sonardepth, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, ProcessedSideScan->altitude, &buffer[index]);
    index += 8;

    /* extract the data */
    for (int i = 0; i < ProcessedSideScan->number_pixels; i++) {
      mb_put_binary_float(false, ProcessedSideScan->sidescan[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < ProcessedSideScan->number_pixels; i++) {
      mb_put_binary_float(false, ProcessedSideScan->alongtrack[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SonarSettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size,
                                          int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSettings *SonarSettings = &(store->SonarSettings);
  s7k3_header *header = &(SonarSettings->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarSettings(verbose, SonarSettings, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SonarSettings;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, SonarSettings->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, SonarSettings->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SonarSettings->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, SonarSettings->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->receiver_bandwidth, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->tx_pulse_width, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->tx_pulse_type, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->tx_pulse_envelope, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->tx_pulse_envelope_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->tx_pulse_mode, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->max_ping_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->ping_period, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->range_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->power_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->gain_selection, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->projector_id, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->steering_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->steering_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->beamwidth_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->beamwidth_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->focal_point, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->projector_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->projector_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->transmit_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->hydrophone_id, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->rx_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->rx_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SonarSettings->rx_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->rx_width, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->range_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->range_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->depth_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->depth_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->absorption, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SonarSettings->spreading, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SonarSettings->reserved, &buffer[index]);
    index += 2;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Configuration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_device *device;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Configuration *Configuration = &(store->Configuration);
  s7k3_header *header = &(Configuration->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Configuration(verbose, Configuration, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Configuration;
  for (int i = 0; i < Configuration->number_devices; i++) {
    *size += 80;
    device = &(Configuration->device[i]);
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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, Configuration->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, Configuration->number_devices, &buffer[index]);
    index += 4;

    /* extract the data for each device */
    for (int i = 0; i < Configuration->number_devices; i++) {
      device = &(Configuration->device[i]);
      mb_put_binary_int(false, device->magic_number, &buffer[index]);
      index += 4;
      for (int j = 0; j < 60; j++) {
        buffer[index] = device->description[j];
        index++;
      }
      mb_put_binary_int(false, device->alphadata_card, &buffer[index]);
      index += 4;
      mb_put_binary_long(false, device->serial_number, &buffer[index]);
      index += 8;
      mb_put_binary_int(false, device->info_length, &buffer[index]);
      index += 4;

      for (int j = 0; j < device->info_length; j++) {
        buffer[index] = device->info[j];
        index++;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_MatchFilter(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_MatchFilter *MatchFilter = &(store->MatchFilter);
  s7k3_header *header = &(MatchFilter->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_MatchFilter(verbose, MatchFilter, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_MatchFilter;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, MatchFilter->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, MatchFilter->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, MatchFilter->operation, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, MatchFilter->start_frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, MatchFilter->end_frequency, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, MatchFilter->window_type, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, MatchFilter->shading, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, MatchFilter->pulse_width, &buffer[index]);
    index += 4;
    for (int i = 0;i<13;i++) {
      mb_put_binary_int(false, MatchFilter->reserved[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_FirmwareHardwareConfiguration(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size,
                                                    int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_FirmwareHardwareConfiguration *FirmwareHardwareConfiguration = &(store->FirmwareHardwareConfiguration);
  s7k3_header *header = &(FirmwareHardwareConfiguration->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FirmwareHardwareConfiguration(verbose, FirmwareHardwareConfiguration, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FirmwareHardwareConfiguration;
  *size += FirmwareHardwareConfiguration->info_length;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(false, FirmwareHardwareConfiguration->device_count, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, FirmwareHardwareConfiguration->info_length, &buffer[index]);
    index += 4;

    /* extract the info */
    for (int i = 0; i < FirmwareHardwareConfiguration->info_length; i++) {
      buffer[index] = FirmwareHardwareConfiguration->info[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_BeamGeometry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BeamGeometry *BeamGeometry = &(store->BeamGeometry);
  s7k3_header *header = &(BeamGeometry->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_BeamGeometry(verbose, BeamGeometry, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_BeamGeometry;
  *size += BeamGeometry->number_beams * 16;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, BeamGeometry->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, BeamGeometry->number_beams, &buffer[index]);
    index += 4;

    /* insert the data */
    for (int i = 0; i < BeamGeometry->number_beams; i++) {
      mb_put_binary_float(false, BeamGeometry->angle_alongtrack[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < BeamGeometry->number_beams; i++) {
      mb_put_binary_float(false, BeamGeometry->angle_acrosstrack[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < BeamGeometry->number_beams; i++) {
      mb_put_binary_float(false, BeamGeometry->beamwidth_alongtrack[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < BeamGeometry->number_beams; i++) {
      mb_put_binary_float(false, BeamGeometry->beamwidth_acrosstrack[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Bathymetry(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Bathymetry *Bathymetry = &(store->Bathymetry);
  s7k3_header *header = &(Bathymetry->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Bathymetric;
  *size += Bathymetry->number_beams * 17;
  if (Bathymetry->optionaldata == true) {
    *size += 45 + Bathymetry->number_beams * 20;
    header->OptionalDataOffset =
        MBSYS_RESON7K_RECORDHEADER_SIZE + R7KHDRSIZE_Bathymetric + Bathymetry->number_beams * 17;
  }
  else
    header->OptionalDataOffset = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Bathymetry(verbose, Bathymetry, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, Bathymetry->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, Bathymetry->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Bathymetry->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, Bathymetry->number_beams, &buffer[index]);
    index += 4;
    buffer[index] = Bathymetry->layer_comp_flag;
    index++;
    buffer[index] = Bathymetry->sound_vel_flag;
    index++;
    mb_put_binary_float(false, Bathymetry->sound_velocity, &buffer[index]);
    index += 4;

    /* insert the data */
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_put_binary_float(false, Bathymetry->range[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      buffer[index] = Bathymetry->quality[i];
      index++;
    }
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_put_binary_float(false, Bathymetry->intensity[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_put_binary_float(false, Bathymetry->min_depth_gate[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < Bathymetry->number_beams; i++) {
      mb_put_binary_float(false, Bathymetry->max_depth_gate[i], &buffer[index]);
      index += 4;
    }

    /* insert the optional data */
    if (Bathymetry->optionaldata == true) {
      mb_put_binary_float(false, Bathymetry->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(false, Bathymetry->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(false, Bathymetry->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(false, Bathymetry->heading, &buffer[index]);
      index += 4;
      buffer[index] = Bathymetry->height_source;
      index++;
      mb_put_binary_float(false, Bathymetry->tide, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Bathymetry->roll, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Bathymetry->pitch, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Bathymetry->heave, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, Bathymetry->vehicle_depth, &buffer[index]);
      index += 4;
      for (int i = 0; i < Bathymetry->number_beams; i++) {
        mb_put_binary_float(false, Bathymetry->depth[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(false, Bathymetry->alongtrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(false, Bathymetry->acrosstrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(false, Bathymetry->pointing_angle[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(false, Bathymetry->azimuth_angle[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SideScan(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int data_size;
  int index;
  char *buffer;
  short *short_ptr;
  int *int_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SideScan *SideScan = &(store->SideScan);
  s7k3_header *header = &(SideScan->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SideScan(verbose, SideScan, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SideScan;
  *size += 2 * SideScan->number_samples * SideScan->sample_size;
  if (SideScan->optionaldata == true) {
    header->OptionalDataOffset = *size - MBSYS_RESON7K_RECORDTAIL_SIZE;
    *size += 32;
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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, SideScan->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, SideScan->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SideScan->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, SideScan->beam_position, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SideScan->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SideScan->number_samples, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, SideScan->nadir_depth, &buffer[index]);
    index += 4;
    for (int i = 0; i< 7; i++) {
        mb_put_binary_int(false, SideScan->reserved[i], &buffer[index]);
        index += 4;
    }
    mb_put_binary_short(false, SideScan->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, SideScan->current_beam, &buffer[index]);
    index += 2;
    buffer[index] = SideScan->sample_size;
    index++;
    buffer[index] = SideScan->data_type;
    index++;

    /* allocate memory if required */
    data_size = SideScan->number_samples * SideScan->sample_size;
    if (SideScan->nalloc < data_size) {
      status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SideScan->port_data), error);
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, data_size, (void **)&(SideScan->stbd_data), error);
      if (status == MB_SUCCESS) {
        SideScan->nalloc = data_size;
      }
      else {
        SideScan->nalloc = 0;
        SideScan->number_samples = 0;
      }
    }

    /* extract SideScan data */
    if (SideScan->sample_size == 1) {
      for (int i = 0; i < SideScan->number_samples; i++) {
        buffer[index] = SideScan->port_data[i];
        index++;
      }
      for (int i = 0; i < SideScan->number_samples; i++) {
        buffer[index] = SideScan->stbd_data[i];
        index++;
      }
    }
    else if (SideScan->sample_size == 2) {
      short_ptr = (short *)SideScan->port_data;
      for (int i = 0; i < SideScan->number_samples; i++) {
        mb_put_binary_short(false, short_ptr[i], &buffer[index]);
        index += 2;
      }
      short_ptr = (short *)SideScan->stbd_data;
      for (int i = 0; i < SideScan->number_samples; i++) {
        mb_put_binary_short(false, short_ptr[i], &buffer[index]);
        index += 2;
      }
    }
    else if (SideScan->sample_size == 4) {
      int_ptr = (int *)SideScan->port_data;
      for (int i = 0; i < SideScan->number_samples; i++) {
        mb_put_binary_int(false, int_ptr[i], &buffer[index]);
        index += 4;
      }
      int_ptr = (int *)SideScan->stbd_data;
      for (int i = 0; i < SideScan->number_samples; i++) {
        mb_put_binary_int(false, int_ptr[i], &buffer[index]);
        index += 4;
      }
    }

    /* extract the optional data */
    if (header->OptionalDataOffset > 0) {
      index = header->OptionalDataOffset;
      SideScan->optionaldata = true;
      mb_put_binary_float(false, SideScan->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(false, SideScan->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(false, SideScan->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(false, SideScan->heading, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SideScan->altitude, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SideScan->depth, &buffer[index]);
      index += 4;
    }
    else {
      SideScan->optionaldata = false;
      SideScan->frequency = 0.0;
      SideScan->latitude = 0.0;
      SideScan->longitude = 0.0;
      SideScan->heading = 0.0;
      SideScan->altitude = 0.0;
      SideScan->depth = 0.0;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_WaterColumn(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_wcd *wcd;
  int index;
  char *buffer;
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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_WaterColumn *WaterColumn = &(store->WaterColumn);
  s7k3_header *header = &(WaterColumn->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_WaterColumn(verbose, WaterColumn, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_WaterColumn;
  sample_type_amp = WaterColumn->sample_type & 15;
  sample_type_phase = (WaterColumn->sample_type >> 4) & 15;
  sample_type_iandq = (WaterColumn->sample_type >> 8) & 15;
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
  for (int i = 0; i < WaterColumn->number_beams; i++) {
    wcd = &WaterColumn->wcd[i];
    *size += 10 + sample_size * (wcd->end_sample - wcd->begin_sample + 1);
  }
  if (header->OptionalDataOffset > 0) {
    *size += 24 + WaterColumn->number_beams * 12;
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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, WaterColumn->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, WaterColumn->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, WaterColumn->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, WaterColumn->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, WaterColumn->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, WaterColumn->samples, &buffer[index]);
    index += 4;
    buffer[index] = WaterColumn->subset_flag;
    index++;
    buffer[index] = WaterColumn->column_flag;
    index++;
    mb_put_binary_short(false, WaterColumn->reserved2, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, WaterColumn->sample_type, &buffer[index]);
    index += 4;
    for (int i = 0; i < WaterColumn->number_beams; i++) {
      wcd = &WaterColumn->wcd[i];
      mb_put_binary_short(false, wcd->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_int(false, wcd->begin_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, wcd->end_sample, &buffer[index]);
      index += 4;
    }

    for (int i = 0; i < WaterColumn->number_beams; i++) {
      /* extract WaterColumn data */
      if (status == MB_SUCCESS) {
        wcd = &WaterColumn->wcd[i];
        nsamples = wcd->end_sample - wcd->begin_sample + 1;
        for (int j = 0; j < nsamples; j++) {
          if (sample_type_amp == 1) {
            charptr = (char *)wcd->amplitude;
            buffer[index] = charptr[j];
            index++;
          }
          else if (sample_type_amp == 2) {
            ushortptr = (unsigned short *)wcd->amplitude;
            mb_put_binary_short(false, ushortptr[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_amp == 3) {
            uintptr = (unsigned int *)wcd->amplitude;
            mb_put_binary_int(false, uintptr[j], &buffer[index]);
            index += 4;
          }
          if (sample_type_phase == 1) {
            charptr = (char *)wcd->phase;
            buffer[index] = charptr[j];
            index++;
          }
          else if (sample_type_phase == 2) {
            ushortptr = (unsigned short *)wcd->phase;
            mb_put_binary_short(false, ushortptr[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_phase == 3) {
            uintptr = (unsigned int *)wcd->phase;
            mb_put_binary_int(false, uintptr[j], &buffer[index]);
            index += 4;
          }
          if (sample_type_iandq == 1) {
            shortptramp = (short *)wcd->amplitude;
            shortptrphase = (short *)wcd->phase;
            mb_put_binary_short(false, shortptramp[j], &buffer[index]);
            index += 2;
            mb_put_binary_short(false, shortptrphase[j], &buffer[index]);
            index += 2;
          }
          else if (sample_type_iandq == 2) {
            intptramp = (int *)wcd->amplitude;
            intptrphase = (int *)wcd->phase;
            mb_put_binary_int(false, intptramp[j], &buffer[index]);
            index += 4;
            mb_put_binary_int(false, intptrphase[j], &buffer[index]);
            index += 4;
          }
        }
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_VerticalDepth(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VerticalDepth *VerticalDepth = &(store->VerticalDepth);
  s7k3_header *header = &(VerticalDepth->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_VerticalDepth(verbose, VerticalDepth, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_VerticalDepth;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, VerticalDepth->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, VerticalDepth->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, VerticalDepth->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_double(false, VerticalDepth->latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, VerticalDepth->longitude, &buffer[index]);
    index += 8;
    mb_put_binary_float(false, VerticalDepth->heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, VerticalDepth->alongtrack, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, VerticalDepth->acrosstrack, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, VerticalDepth->vertical_depth, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_TVG(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_TVG *TVG = &(store->TVG);
  s7k3_header *header = &(TVG->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_TVG(verbose, TVG, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_TVG;
  *size += TVG->n * sizeof(float);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, TVG->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, TVG->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, TVG->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, TVG->n, &buffer[index]);
    index += 4;
    for (int i = 0; i < 8; i++) {
      mb_put_binary_int(false, TVG->reserved[i], &buffer[index]);
      index += 4;
    }

    /* insert TVG data */
    memcpy((void *)&buffer[index], (const void *)TVG->tvg, (size_t)(TVG->n * sizeof(float)));
    index += TVG->n * sizeof(float);

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Image(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;
  int nalloc;
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
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Image *Image = &(store->Image);
  s7k3_header *header = &(Image->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Image(verbose, Image, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Image;
  *size += Image->width * Image->height * Image->color_depth;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_int(false, Image->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Image->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, Image->width, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, Image->height, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Image->color_depth, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, Image->reserved, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, Image->compression, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, Image->samples, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, Image->flag, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, Image->rx_delay, &buffer[index]);
    index += 4;
    for (int i = 0; i < 6; i++) {
      mb_put_binary_int(false, Image->reserved2[i], &buffer[index]);
      index += 4;
    }

    /* allocate memory for image if needed */
    nalloc = Image->width * Image->height * Image->color_depth;
    if (status == MB_SUCCESS && Image->nalloc < nalloc) {
      Image->nalloc = nalloc;
      if (status == MB_SUCCESS)
        status = mb_reallocd(verbose, __FILE__, __LINE__, Image->nalloc, (void **)&(Image->image), error);
      if (status != MB_SUCCESS) {
        Image->nalloc = 0;
        Image->width = 0;
        Image->height = 0;
      }
    }

    /* extract image data */
    if (Image->color_depth == 1) {
      charptr = (char *)Image->image;
      for (int i = 0; i < Image->width * Image->height; i++) {
        buffer[index] = charptr[i];
        index++;
      }
    }
    else if (Image->color_depth == 2) {
      ushortptr = (unsigned short *)Image->image;
      for (int i = 0; i < Image->width * Image->height; i++) {
        mb_put_binary_short(false, ushortptr[i], &buffer[index]);
        index += 2;
      }
    }
    else if (Image->color_depth == 4) {
      uintptr = (unsigned int *)Image->image;
      for (int i = 0; i < Image->width * Image->height; i++) {
        mb_put_binary_int(false, uintptr[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_PingMotion(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_PingMotion *PingMotion = &(store->PingMotion);
  s7k3_header *header = &(PingMotion->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_PingMotion(verbose, PingMotion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_PingMotion;
  if (PingMotion->flags & 1)
    *size += sizeof(float);
  if (PingMotion->flags & 2)
    *size += sizeof(float) * PingMotion->n;
  if (PingMotion->flags & 4)
    *size += sizeof(float) * PingMotion->n;
  if (PingMotion->flags & 8)
    *size += sizeof(float) * PingMotion->n;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, PingMotion->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, PingMotion->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, PingMotion->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, PingMotion->n, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, PingMotion->flags, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, PingMotion->error_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, PingMotion->frequency, &buffer[index]);
    index += 4;
    if (PingMotion->flags & 1) {
      mb_put_binary_float(false, PingMotion->pitch, &buffer[index]);
      index += 4;
    }
    if (PingMotion->flags & 2) {
      for (int i = 0; i < PingMotion->n; i++) {
        mb_put_binary_float(false, PingMotion->roll[i], &buffer[index]);
        index += 4;
      }
    }
    if (PingMotion->flags & 4) {
      for (int i = 0; i < PingMotion->n; i++) {
        mb_put_binary_float(false, PingMotion->heading[i], &buffer[index]);
        index += 4;
      }
    }
    if (PingMotion->flags & 8) {
      for (int i = 0; i < PingMotion->n; i++) {
        mb_put_binary_float(false, PingMotion->heave[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_AdaptiveGate(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_AdaptiveGate *AdaptiveGate = &(store->AdaptiveGate);
  s7k3_header *header = &(AdaptiveGate->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_AdaptiveGate(verbose, AdaptiveGate, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_AdaptiveGate;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_DetectionDataSetup(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_DetectionDataSetup *DetectionDataSetup = &(store->DetectionDataSetup);
  s7k3_header *header = &(DetectionDataSetup->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_DetectionDataSetup(verbose, DetectionDataSetup, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_DetectionDataSetup;
  if (DetectionDataSetup->data_block_size > R7KRDTSIZE_DetectionDataSetup)
    DetectionDataSetup->data_block_size = R7KRDTSIZE_DetectionDataSetup;
  *size += DetectionDataSetup->number_beams * DetectionDataSetup->data_block_size;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, DetectionDataSetup->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, DetectionDataSetup->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, DetectionDataSetup->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, DetectionDataSetup->number_beams, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, DetectionDataSetup->data_block_size, &buffer[index]);
    index += 4;
    buffer[index] = DetectionDataSetup->detection_algorithm;
    index++;
    mb_put_binary_int(false, DetectionDataSetup->detection_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->minimum_depth, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->maximum_depth, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->minimum_range, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->maximum_range, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->minimum_nadir_search, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->maximum_nadir_search, &buffer[index]);
    index += 4;
    buffer[index] = DetectionDataSetup->automatic_filter_window;
    index++;
    mb_put_binary_float(false, DetectionDataSetup->applied_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->depth_gate_tilt, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, DetectionDataSetup->nadir_depth, &buffer[index]);
    index += 4;
    for (int i = 0; i < 13; i++) {
      mb_put_binary_float(false, DetectionDataSetup->reserved[i], &buffer[index]);
      index += 4;
    }
    for (int i = 0; i < DetectionDataSetup->number_beams; i++) {
      mb_put_binary_short(false, DetectionDataSetup->beam_descriptor[i], &buffer[index]);
      index += 2;
      mb_put_binary_float(false, DetectionDataSetup->detection_point[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->flags[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->auto_limits_min_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->auto_limits_max_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->user_limits_min_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->user_limits_max_sample[i], &buffer[index]);
      index += 4;
      mb_put_binary_int(false, DetectionDataSetup->quality[i], &buffer[index]);
      index += 4;
      if (DetectionDataSetup->data_block_size >= R7KRDTSIZE_DetectionDataSetup) {
        mb_put_binary_int(false, DetectionDataSetup->uncertainty[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Beamformed(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_amplitudephase *amplitudephase;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Beamformed *Beamformed = &(store->Beamformed);
  s7k3_header *header = &(Beamformed->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Beamformed(verbose, Beamformed, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Beamformed;
  *size += 2 * sizeof(short) * Beamformed->number_beams * Beamformed->number_samples;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, Beamformed->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, Beamformed->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Beamformed->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, Beamformed->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, Beamformed->number_samples, &buffer[index]);
    index += 4;
    for (int i = 0; i < 8; i++) {
      buffer[index] = Beamformed->reserved[i];
      index++;
    }
    for (int i = 0; i < Beamformed->number_beams; i++) {
      amplitudephase = &(Beamformed->amplitudephase[i]);

      /* insert Beamformed data */
      for (int j = 0; j < Beamformed->number_samples; j++) {
        mb_put_binary_short(false, amplitudephase->amplitude[j], &buffer[index]);
        index += 2;
        mb_put_binary_short(false, amplitudephase->phase[j], &buffer[index]);
        index += 2;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_VernierProcessingDataRaw(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VernierProcessingDataRaw *VernierProcessingDataRaw = &(store->VernierProcessingDataRaw);
  s7k3_header *header = &(VernierProcessingDataRaw->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_VernierProcessingDataRaw(verbose, VernierProcessingDataRaw, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_VernierProcessingDataRaw;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_BITE(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_bitereport *bitereport;
  s7k3_time *s7kTime;
  s7k3_bitefield *bitefield;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BITE *BITE = &(store->BITE);
  s7k3_header *header = &(BITE->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_BITE(verbose, BITE, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_BITE;
  for (int i = 0; i < BITE->number_reports; i++) {
    bitereport = &(BITE->bitereports[i]);
    *size += R7KRDTSIZE_BITERecordData + bitereport->number_bite * R7KRDTSIZE_BITEFieldData;
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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_short(false, BITE->number_reports, &buffer[index]);
    index += 2;
    for (int i = 0; i < BITE->number_reports; i++) {
      bitereport = &(BITE->bitereports[i]);

      for (int j = 0; j < 64; j++) {
        buffer[index] = bitereport->source_name[j];
        index++;
      }
      buffer[index] = bitereport->source_address;
      index++;
      mb_put_binary_float(false, bitereport->reserved, &buffer[index]);
      index += 4;
      mb_put_binary_short(false, bitereport->reserved2, &buffer[index]);
      index += 2;

      s7kTime = &(bitereport->downlink_time);
      mb_put_binary_short(false, s7kTime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, s7kTime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, s7kTime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7kTime->Hours;
      index++;
      buffer[index] = s7kTime->Minutes;
      index++;

      s7kTime = &(bitereport->uplink_time);
      mb_put_binary_short(false, s7kTime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, s7kTime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, s7kTime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7kTime->Hours;
      index++;
      buffer[index] = s7kTime->Minutes;
      index++;

      s7kTime = &(bitereport->bite_time);
      mb_put_binary_short(false, s7kTime->Year, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, s7kTime->Day, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, s7kTime->Seconds, &buffer[index]);
      index += 4;
      buffer[index] = s7kTime->Hours;
      index++;
      buffer[index] = s7kTime->Minutes;
      index++;

      buffer[index] = bitereport->status;
      index++;
      mb_put_binary_short(false, bitereport->number_bite, &buffer[index]);
      index += 2;
      for (int j = 0; j < 4; j++) {
        mb_put_binary_long(false, bitereport->bite_status[j], &buffer[index]);
        index += 8;
      }

      /* loop over all bite fields */
      for (int j = 0; j < bitereport->number_bite; j++) {
        bitefield = &(bitereport->bitefield[j]);

        mb_put_binary_short(false, bitefield->field, &buffer[index]);
        index += 2;
        for (int k = 0; k < 64; k++) {
          buffer[index] = bitefield->name[k];
          index++;
        }
        buffer[index] = bitefield->device_type;
        index++;
        mb_put_binary_float(false, bitefield->minimum, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bitefield->maximum, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bitefield->value, &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d %d index:%d\n",
      __FILE__, __LINE__, header->Size, *size, index);
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
int mbr_reson7k3_wr_SonarSourceVersion(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SonarSourceVersion *SonarSourceVersion = &(store->SonarSourceVersion);
  s7k3_header *header = &(SonarSourceVersion->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SonarSourceVersion(verbose, SonarSourceVersion, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SonarSourceVersion;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      buffer[index] = SonarSourceVersion->version[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_WetEndVersion8k(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_WetEndVersion8k *WetEndVersion8k = &(store->WetEndVersion8k);
  s7k3_header *header = &(WetEndVersion8k->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_WetEndVersion8k(verbose, WetEndVersion8k, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_WetEndVersion8k;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 32; i++) {
      buffer[index] = WetEndVersion8k->version[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RawDetection(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_rawdetectiondata *rawdetectiondata;
  s7k3_bathydata *bathydata;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RawDetection *RawDetection = &(store->RawDetection);
  s7k3_header *header = &(RawDetection->header);

  /* use data_field_size no larger than 34 */
  if (RawDetection->data_field_size > 34) {
    RawDetection->data_field_size = 34;
  }

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RawDetection;
  *size += RawDetection->number_beams * RawDetection->data_field_size;
  if (RawDetection->optionaldata == true) {
    header->OptionalDataOffset = *size - MBSYS_RESON7K_RECORDTAIL_SIZE;
    *size += R7KOPTHDRSIZE_RawDetection
              + RawDetection->number_beams * R7KOPTDATSIZE_RawDetection;
  } else {
    header->OptionalDataOffset = 0;
  }
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RawDetection(verbose, RawDetection, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, RawDetection->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, RawDetection->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, RawDetection->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, RawDetection->number_beams, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RawDetection->data_field_size, &buffer[index]);
    index += 4;
    buffer[index] = RawDetection->detection_algorithm;
    index++;
    mb_put_binary_int(false, RawDetection->flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RawDetection->sampling_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RawDetection->tx_angle, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RawDetection->applied_roll, &buffer[index]);
    index += 4;
    for (int i = 0; i < 15; i++) {
      mb_put_binary_int(false, RawDetection->reserved[i], &buffer[index]);
      index += 4;
    }

    /* insert the data */
    for (int i = 0; i < RawDetection->number_beams; i++) {
      rawdetectiondata = (s7k3_rawdetectiondata *)&RawDetection->rawdetectiondata[i];
      mb_put_binary_short(false, rawdetectiondata->beam_descriptor, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, rawdetectiondata->detection_point, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, rawdetectiondata->rx_angle, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, rawdetectiondata->flags, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, rawdetectiondata->quality, &buffer[index]);
      index += 4;
      if (RawDetection->data_field_size >= 22) {
        mb_put_binary_float(false, rawdetectiondata->uncertainty, &buffer[index]);
        index += 4;
      }
      if (RawDetection->data_field_size >= 26) {
        mb_put_binary_float(false, rawdetectiondata->signal_strength, &buffer[index]);
        index += 4;
      }
      if (RawDetection->data_field_size >= 30) {
        mb_put_binary_float(false, rawdetectiondata->min_limit, &buffer[index]);
        index += 4;
      }
      if (RawDetection->data_field_size >= 34) {
        mb_put_binary_float(false, rawdetectiondata->max_limit, &buffer[index]);
        index += 4;
      }
    }

      /* insert the optional data */
    if (RawDetection->optionaldata == true) {
      mb_put_binary_float(false, RawDetection->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(false, RawDetection->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(false, RawDetection->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(false, RawDetection->heading, &buffer[index]);
      index += 4;
      buffer[index] = RawDetection->height_source;
      index += 1;
      mb_put_binary_float(false, RawDetection->tide, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, RawDetection->roll, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, RawDetection->pitch, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, RawDetection->heave, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, RawDetection->vehicle_depth, &buffer[index]);
      index += 4;
      for (int i = 0; i < RawDetection->number_beams; i++) {
        bathydata = (s7k3_bathydata *)&RawDetection->bathydata[i];
        mb_put_binary_float(false, bathydata->depth, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->alongtrack, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->acrosstrack, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->pointing_angle, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->azimuth_angle, &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
    index += 4;

    /* check size */
    if (*size != index) {
      fprintf(stderr, "Bad size comparison: file:%s line:%d size:%d index:%d\n", __FILE__, __LINE__, *size, index);
      fprintf(stderr, "RawDetection->number_beams:%d RawDetection->optionaldata:%d\n",
              RawDetection->number_beams, RawDetection->optionaldata);
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
int mbr_reson7k3_wr_Snippet(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_snippetdata *snippetdata;
  int index;
  char *buffer;
  int nsample;
  u16 *u16_ptr;
  u32 *u32_ptr;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Snippet *Snippet = &(store->Snippet);
  s7k3_header *header = &(Snippet->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Snippet;
  for (int i = 0; i < Snippet->number_beams; i++) {
    snippetdata = &(Snippet->snippetdata[i]);
    *size += R7KRDTSIZE_snippetdata;
    nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
    if ((Snippet->flags & 0x01) != 0)
      *size += sizeof(int) * nsample;
    else
      *size += sizeof(short) * nsample;
  }
  if (Snippet->optionaldata == true) {
    header->OptionalDataOffset = *size - MBSYS_RESON7K_RECORDTAIL_SIZE;
    *size += 24 + 12 * Snippet->number_beams;
  } else {
    header->OptionalDataOffset = 0;
  }

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Snippet(verbose, Snippet, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, Snippet->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, Snippet->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, Snippet->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, Snippet->number_beams, &buffer[index]);
    index += 2;
    buffer[index] = Snippet->error_flag;
    index++;
    buffer[index] = Snippet->control_flags;
    index++;
    mb_put_binary_int(false, Snippet->flags, &buffer[index]);
    index += 4;
    for (int i = 0; i < 6; i++) {
      mb_put_binary_int(false, Snippet->reserved[i], &buffer[index]);
      index += 4;
    }

    /* insert the Snippet parameters */
    for (int i = 0; i < Snippet->number_beams; i++) {
      snippetdata = &(Snippet->snippetdata[i]);

      /* extract snippetdata data */
      mb_put_binary_short(false, snippetdata->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_int(false, snippetdata->begin_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, snippetdata->detect_sample, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, snippetdata->end_sample, &buffer[index]);
      index += 4;
    }

    /* loop over all beams to insert Snippet data */
    if ((Snippet->flags & 0x01) != 0) {
      for (int i = 0; i < Snippet->number_beams; i++) {
        snippetdata = (s7k3_snippetdata *)&(Snippet->snippetdata[i]);
        nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
        u32_ptr = (u32 *)snippetdata->amplitude;
        for (int j = 0; j < nsample; j++) {
          mb_put_binary_int(false, u32_ptr[j], &buffer[index]);
          index += 4;
        }
      }
    }
    else {
      for (int i = 0; i < Snippet->number_beams; i++) {
        snippetdata = (s7k3_snippetdata *)&(Snippet->snippetdata[i]);
        nsample = snippetdata->end_sample - snippetdata->begin_sample + 1;
        u16_ptr = (u16 *)snippetdata->amplitude;
        for (int j = 0; j < nsample; j++) {
          mb_put_binary_short(false, u16_ptr[j], &buffer[index]);
          index += 2;
        }
      }
    }

    /* insert optional data - calculated bathymetry */
    if (Snippet->optionaldata == true) {
      header->OptionalDataOffset = index;

      mb_put_binary_float(false, Snippet->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(false, Snippet->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(false, Snippet->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(false, Snippet->heading, &buffer[index]);
      index += 4;
      for (int i = 0; i < Snippet->number_beams; i++) {
        mb_put_binary_float(false, Snippet->beam_alongtrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_float(false, Snippet->beam_acrosstrack[i], &buffer[index]);
        index += 4;
        mb_put_binary_int(false, Snippet->center_sample[i], &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_VernierProcessingDataFiltered(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_VernierProcessingDataFiltered *VernierProcessingDataFiltered = &(store->VernierProcessingDataFiltered);
  s7k3_header *header = &(VernierProcessingDataFiltered->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_VernierProcessingDataFiltered(verbose, VernierProcessingDataFiltered, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_VernierProcessingDataFiltered;
  *size += VernierProcessingDataFiltered->number_soundings * R7KRDTSIZE_VernierProcessingDataFiltered;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_InstallationParameters(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_InstallationParameters *InstallationParameters = &(store->InstallationParameters);
  s7k3_header *header = &(InstallationParameters->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_InstallationParameters(verbose, InstallationParameters, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_InstallationParameters;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, InstallationParameters->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, InstallationParameters->firmware_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = InstallationParameters->firmware_version[i];
      index++;
    }
    mb_put_binary_short(false, InstallationParameters->software_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = InstallationParameters->software_version[i];
      index++;
    }
    mb_put_binary_short(false, InstallationParameters->s7k3_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = InstallationParameters->s7k3_version[i];
      index++;
    }
    mb_put_binary_short(false, InstallationParameters->protocal_version_len, &buffer[index]);
    index += 2;
    for (int i = 0; i < 128; i++) {
      buffer[index] = InstallationParameters->protocal_version[i];
      index++;
    }
    mb_put_binary_float(false, InstallationParameters->transmit_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->transmit_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->transmit_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->transmit_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->transmit_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->transmit_heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->receive_heading, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->motion_heading, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, InstallationParameters->motion_time_delay, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, InstallationParameters->position_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->position_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, InstallationParameters->position_z, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, InstallationParameters->position_time_delay, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, InstallationParameters->waterline_z, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_BITESummary(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_BITESummary *BITESummary = &(store->BITESummary);
  s7k3_header *header = &(BITESummary->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_BITESummary(verbose, BITESummary, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_BITESummary;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CompressedBeamformedMagnitude(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CompressedBeamformedMagnitude *CompressedBeamformedMagnitude = &(store->CompressedBeamformedMagnitude);
  s7k3_header *header = &(CompressedBeamformedMagnitude->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CompressedBeamformedMagnitude(verbose, CompressedBeamformedMagnitude, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CompressedBeamformedMagnitude;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CompressedWaterColumn(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_compressedwatercolumndata *compressedwatercolumndata;
  int index;
  char *buffer;
  int segmentnumbersvalid = false;
  size_t size_beamheader, size_sample, nwrite;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CompressedWaterColumn *CompressedWaterColumn = &(store->CompressedWaterColumn);
  s7k3_header *header = &(CompressedWaterColumn->header);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CompressedWaterColumn;
  size_beamheader = 6;
  if (CompressedWaterColumn->flags & 0x4000) {
    segmentnumbersvalid = true;
    size_beamheader++;
  }
  size_sample = CompressedWaterColumn->magsamplesize + CompressedWaterColumn->phasesamplesize;
  for (int i = 0;i<CompressedWaterColumn->number_beams;i++) {
    compressedwatercolumndata = &CompressedWaterColumn->compressedwatercolumndata[i];
    *size += size_beamheader + size_sample * compressedwatercolumndata->samples;
  }
  header->OptionalDataOffset = 0;
  header->Size = *size;

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CompressedWaterColumn(verbose, CompressedWaterColumn, error);

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, CompressedWaterColumn->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, CompressedWaterColumn->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, CompressedWaterColumn->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CompressedWaterColumn->number_beams, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, CompressedWaterColumn->samples, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CompressedWaterColumn->compressed_samples, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CompressedWaterColumn->flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CompressedWaterColumn->first_sample, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CompressedWaterColumn->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CompressedWaterColumn->compression_factor, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CompressedWaterColumn->reserved, &buffer[index]);
    index += 4;

    /* insert the data */
    for (int i = 0; i < CompressedWaterColumn->number_beams; i++) {
      compressedwatercolumndata = (s7k3_compressedwatercolumndata *)&(CompressedWaterColumn->compressedwatercolumndata[i]);
      mb_put_binary_short(false, compressedwatercolumndata->beam_number, &buffer[index]);
      index += 2;
      if (segmentnumbersvalid == true) {
        buffer[index] = compressedwatercolumndata->segment_number;
        index += 1;
      }
      mb_put_binary_int(false, compressedwatercolumndata->samples, &buffer[index]);
      index += 4;
      nwrite = (CompressedWaterColumn->magsamplesize + CompressedWaterColumn->phasesamplesize)
                * compressedwatercolumndata->samples;
      memcpy(&buffer[index], compressedwatercolumndata->data, nwrite);
      index += nwrite;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SegmentedRawDetection(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_segmentedrawdetectiontxdata *segmentedrawdetectiontxdata;
  s7k3_segmentedrawdetectionrxdata *segmentedrawdetectionrxdata;
  s7k3_bathydata *bathydata;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SegmentedRawDetection *SegmentedRawDetection = &(store->SegmentedRawDetection);
  s7k3_header *header = &(SegmentedRawDetection->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SegmentedRawDetection(verbose, SegmentedRawDetection, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SegmentedRawDetection;
  SegmentedRawDetection->record_header_size = 36;
  SegmentedRawDetection->segment_field_size = 68;
  SegmentedRawDetection->rx_field_size = 32;
  *size += SegmentedRawDetection->n_segments * SegmentedRawDetection->segment_field_size;
  *size += SegmentedRawDetection->n_rx * SegmentedRawDetection->rx_field_size;
  if (SegmentedRawDetection->optionaldata == true) {
    header->OptionalDataOffset = *size - MBSYS_RESON7K_RECORDTAIL_SIZE;
    *size += R7KOPTHDRSIZE_SegmentedRawDetection
              + SegmentedRawDetection->n_rx * R7KOPTDATSIZE_SegmentedRawDetection;
  } else {
    header->OptionalDataOffset = 0;
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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_short(false, SegmentedRawDetection->record_header_size, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, SegmentedRawDetection->n_segments, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SegmentedRawDetection->segment_field_size, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, SegmentedRawDetection->n_rx, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SegmentedRawDetection->rx_field_size, &buffer[index]);
    index += 2;
    mb_put_binary_long(false, SegmentedRawDetection->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, SegmentedRawDetection->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, SegmentedRawDetection->multi_ping, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, SegmentedRawDetection->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, SegmentedRawDetection->rx_delay, &buffer[index]);
    index += 4;

    /* insert the data */
    for (int i = 0; i < SegmentedRawDetection->n_segments; i++) {
      segmentedrawdetectiontxdata = (s7k3_segmentedrawdetectiontxdata *)&SegmentedRawDetection->segmentedrawdetectiontxdata[i];
      mb_put_binary_short(false, segmentedrawdetectiontxdata->segment_number, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_angle_along, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_angle_across, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_delay, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, segmentedrawdetectiontxdata->pulse_type, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->pulse_bandwidth, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_pulse_width, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_pulse_width_across, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_pulse_width_along, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, segmentedrawdetectiontxdata->tx_pulse_envelope, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_pulse_envelope_parameter, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->tx_relative_src_level, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->rx_beam_width, &buffer[index]);
      index += 4;
      buffer[index] = segmentedrawdetectiontxdata->detection_algorithm;
      index += 1;
      mb_put_binary_int(false, segmentedrawdetectiontxdata->flags, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->sampling_rate, &buffer[index]);
      index += 4;
      buffer[index] = segmentedrawdetectiontxdata->tvg;
      index += 1;
      mb_put_binary_float(false, segmentedrawdetectiontxdata->rx_bandwidth, &buffer[index]);
      index += 4;
    }

      /* insert the data */
    for (int i = 0; i < SegmentedRawDetection->n_rx; i++) {
      segmentedrawdetectionrxdata = (s7k3_segmentedrawdetectionrxdata *)&SegmentedRawDetection->segmentedrawdetectionrxdata[i];
      mb_put_binary_short(false, segmentedrawdetectionrxdata->beam_number, &buffer[index]);
      index += 2;
      mb_put_binary_short(false, segmentedrawdetectionrxdata->used_segment, &buffer[index]);
      index += 2;
      mb_put_binary_float(false, segmentedrawdetectionrxdata->detection_point, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectionrxdata->rx_angle_cross, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, segmentedrawdetectionrxdata->flags2, &buffer[index]);
      index += 4;
      mb_put_binary_int(false, segmentedrawdetectionrxdata->quality, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectionrxdata->uncertainty, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectionrxdata->signal_strength, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, segmentedrawdetectionrxdata->sn_ratio, &buffer[index]);
      index += 4;
    }

      /* insert the optional data */
    if (SegmentedRawDetection->optionaldata == true) {
      mb_put_binary_float(false, SegmentedRawDetection->frequency, &buffer[index]);
      index += 4;
      mb_put_binary_double(false, SegmentedRawDetection->latitude, &buffer[index]);
      index += 8;
      mb_put_binary_double(false, SegmentedRawDetection->longitude, &buffer[index]);
      index += 8;
      mb_put_binary_float(false, SegmentedRawDetection->heading, &buffer[index]);
      index += 4;
      buffer[index] = SegmentedRawDetection->height_source;
      index += 1;
      mb_put_binary_float(false, SegmentedRawDetection->tide, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SegmentedRawDetection->roll, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SegmentedRawDetection->pitch, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SegmentedRawDetection->heave, &buffer[index]);
      index += 4;
      mb_put_binary_float(false, SegmentedRawDetection->vehicle_depth, &buffer[index]);
      index += 4;
      for (int i = 0; i < SegmentedRawDetection->n_rx; i++) {
        bathydata = (s7k3_bathydata *)&SegmentedRawDetection->bathydata[i];
        mb_put_binary_float(false, bathydata->depth, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->alongtrack, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->acrosstrack, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->pointing_angle, &buffer[index]);
        index += 4;
        mb_put_binary_float(false, bathydata->azimuth_angle, &buffer[index]);
        index += 4;
      }
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CalibratedBeam(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibratedBeam *CalibratedBeam = &(store->CalibratedBeam);
  s7k3_header *header = &(CalibratedBeam->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CalibratedBeam(verbose, CalibratedBeam, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CalibratedBeam;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SystemEvents(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemEvents *SystemEvents = &(store->SystemEvents);
  s7k3_header *header = &(SystemEvents->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SystemEvents(verbose, SystemEvents, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SystemEvents;
      // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SystemEventMessage(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemEventMessage *SystemEventMessage = &(store->SystemEventMessage);
  s7k3_header *header = &(SystemEventMessage->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SystemEventMessage(verbose, SystemEventMessage, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SystemEventMessage;
  *size += SystemEventMessage->message_length;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, SystemEventMessage->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_short(false, SystemEventMessage->event_id, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, SystemEventMessage->message_length, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, SystemEventMessage->event_identifier, &buffer[index]);
    index += 2;

    /* insert the data */
    for (int i = 0; i < SystemEventMessage->message_length; i++) {
      buffer[index] = SystemEventMessage->message[i];
      index++;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RDRRecordingStatus(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RDRRecordingStatus *RDRRecordingStatus = &(store->RDRRecordingStatus);
  s7k3_header *header = &(RDRRecordingStatus->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RDRRecordingStatus(verbose, RDRRecordingStatus, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RDRRecordingStatus;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_Subscriptions(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_Subscriptions *Subscriptions = &(store->Subscriptions);
  s7k3_header *header = &(Subscriptions->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_Subscriptions(verbose, Subscriptions, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_Subscriptions;
  *size += Subscriptions->n_subscriptions * R7KRDTSIZE_Subscriptions;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RDRStorageRecording(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RDRStorageRecording *RDRStorageRecording = &(store->RDRStorageRecording);
  s7k3_header *header = &(RDRStorageRecording->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RDRStorageRecording(verbose, RDRStorageRecording, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RDRStorageRecording;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CalibrationStatus(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibrationStatus *CalibrationStatus = &(store->CalibrationStatus);
  s7k3_header *header = &(CalibrationStatus->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CalibrationStatus(verbose, CalibrationStatus, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CalibrationStatus;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CalibratedSideScan(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CalibratedSideScan *CalibratedSideScan = &(store->CalibratedSideScan);
  s7k3_header *header = &(CalibratedSideScan->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CalibratedSideScan(verbose, CalibratedSideScan, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CalibratedSideScan;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SnippetBackscatteringStrength(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SnippetBackscatteringStrength *SnippetBackscatteringStrength = &(store->SnippetBackscatteringStrength);
  s7k3_header *header = &(SnippetBackscatteringStrength->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SnippetBackscatteringStrength(verbose, SnippetBackscatteringStrength, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SnippetBackscatteringStrength;
    // Notdone

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_MB2Status(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_MB2Status *MB2Status = &(store->MB2Status);
  s7k3_header *header = &(MB2Status->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_MB2Status(verbose, MB2Status, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_MB2Status;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_FileHeader(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  s7k3_subsystem *subsystem;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_FileHeader *FileHeader = &(store->FileHeader);
  s7k3_header *header = &(FileHeader->header);

  // Make sure optional data offset is set so that there is space to overwrite
  // the file catalog size and location when the file is closed
  FileHeader->optionaldata = true;
  FileHeader->file_catalog_size = 0;
  FileHeader->file_catalog_offset = 0;

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_FileHeader(verbose, FileHeader, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_FileHeader;
  for (int i = 0; i < FileHeader->number_devices; i++)
    *size += R7KRDTSIZE_FileHeader;
  header->OptionalDataOffset = *size - MBSYS_RESON7K_RECORDTAIL_SIZE;
  header->OptionalDataIdentifier = 7300;
  *size += 12;
  header->Size = *size;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    for (int i = 0; i < 2; i++) {
      mb_put_binary_long(false, FileHeader->file_identifier[i], &buffer[index]);
      index += 8;
    }
    mb_put_binary_short(false, FileHeader->version, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, FileHeader->reserved, &buffer[index]);
    index += 2;
    for (int i = 0; i < 2; i++) {
      mb_put_binary_long(false, FileHeader->session_identifier[i], &buffer[index]);
      index += 8;
    }
    mb_put_binary_int(false, FileHeader->record_data_size, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, FileHeader->number_devices, &buffer[index]);
    index += 4;
    for (int i = 0; i < 64; i++) {
      buffer[index] = FileHeader->recording_name[i];
      index++;
    }
    for (int i = 0; i < 16; i++) {
      buffer[index] = FileHeader->recording_version[i];
      index++;
    }
    for (int i = 0; i < 64; i++) {
      buffer[index] = FileHeader->user_defined_name[i];
      index++;
    }
    for (int i = 0; i < 128; i++) {
      buffer[index] = FileHeader->notes[i];
      index++;
    }
    for (int i = 0; i < FileHeader->number_devices; i++) {
      subsystem = &(FileHeader->subsystem[i]);
      mb_put_binary_int(false, subsystem->device_identifier, &buffer[index]);
      index += 4;
      mb_put_binary_short(false, subsystem->system_enumerator, &buffer[index]);
      index += 2;
    }

      /* insert the optional data */
    if (FileHeader->optionaldata == true) {
      mb_put_binary_int(false, FileHeader->file_catalog_size, &buffer[index]);
      index += 4;
      mb_put_binary_long(false, FileHeader->file_catalog_offset, &buffer[index]);
      index += 8;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_TimeMessage(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_TimeMessage *TimeMessage = &(store->TimeMessage);
  s7k3_header *header = &(TimeMessage->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_TimeMessage(verbose, TimeMessage, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_TimeMessage;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RemoteControl(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControl *RemoteControl = &(store->RemoteControl);
  s7k3_header *header = &(RemoteControl->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RemoteControl(verbose, RemoteControl, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RemoteControl;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RemoteControlAcknowledge(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlAcknowledge *RemoteControlAcknowledge = &(store->RemoteControlAcknowledge);
  s7k3_header *header = &(RemoteControlAcknowledge->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RemoteControlAcknowledge(verbose, RemoteControlAcknowledge, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RemoteControlAcknowledge;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RemoteControlNotAcknowledge(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlNotAcknowledge *RemoteControlNotAcknowledge = &(store->RemoteControlNotAcknowledge);
  s7k3_header *header = &(RemoteControlNotAcknowledge->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RemoteControlNotAcknowledge(verbose, RemoteControlNotAcknowledge, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RemoteControlNotAcknowledge;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_RemoteControlSonarSettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size,
                                          int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_RemoteControlSonarSettings *RemoteControlSonarSettings = &(store->RemoteControlSonarSettings);
  s7k3_header *header = &(RemoteControlSonarSettings->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_RemoteControlSonarSettings(verbose, RemoteControlSonarSettings, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_RemoteControlSonarSettings;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, RemoteControlSonarSettings->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, RemoteControlSonarSettings->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->frequency, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->sample_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->receiver_bandwidth, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->tx_pulse_width, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->tx_pulse_type, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->tx_pulse_envelope, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->tx_pulse_envelope_par, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, RemoteControlSonarSettings->tx_pulse_mode, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, RemoteControlSonarSettings->tx_pulse_reserved, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, RemoteControlSonarSettings->max_ping_rate, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->ping_period, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->range_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->power_selection, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->gain_selection, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->control_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->projector_id, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->steering_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->steering_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->beamwidth_vertical, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->beamwidth_horizontal, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->focal_point, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->projector_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->projector_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->transmit_flags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->hydrophone_id, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->rx_weighting, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->rx_weighting_par, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->rx_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->range_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->range_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->depth_minimum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->depth_maximum, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->absorption, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->spreading, &buffer[index]);
    index += 4;
    RemoteControlSonarSettings->vernier_operation_mode = buffer[index];
    index ++;
    RemoteControlSonarSettings->autofilter_window = buffer[index];
    index ++;
    mb_put_binary_float(false, RemoteControlSonarSettings->tx_offset_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->tx_offset_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->tx_offset_z, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->head_tilt_x, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->head_tilt_y, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->head_tilt_z, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->ping_state, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, RemoteControlSonarSettings->beam_angle_mode, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, RemoteControlSonarSettings->s7kcenter_mode, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, RemoteControlSonarSettings->gate_depth_min, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->gate_depth_max, &buffer[index]);
    index += 4;

    mb_put_binary_double(false, RemoteControlSonarSettings->trigger_width, &buffer[index]);
    index += 8;
    mb_put_binary_double(false, RemoteControlSonarSettings->trigger_offset, &buffer[index]);
    index += 8;
    mb_put_binary_short(false, RemoteControlSonarSettings->projector_selection, &buffer[index]);
    index += 2;
    for (int i = 0;i<2;i++) {
      mb_put_binary_int(false, RemoteControlSonarSettings->reserved2[i], &buffer[index]);
      index += 4;
    }
    mb_put_binary_float(false, RemoteControlSonarSettings->alternate_gain, &buffer[index]);
    index += 4;
    buffer[index] = RemoteControlSonarSettings->vernier_filter;
    index ++;
    buffer[index] = RemoteControlSonarSettings->reserved3;
    index ++;
    mb_put_binary_short(false, RemoteControlSonarSettings->custom_beams, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, RemoteControlSonarSettings->coverage_angle, &buffer[index]);
    index += 4;
    buffer[index] = RemoteControlSonarSettings->coverage_mode;
    index ++;
    buffer[index] = RemoteControlSonarSettings->quality_filter;
    index ++;
    mb_put_binary_float(false, RemoteControlSonarSettings->received_steering, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->flexmode_coverage, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->flexmode_steering, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->constant_spacing, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, RemoteControlSonarSettings->beam_mode, &buffer[index]);
    index += 2;
    mb_put_binary_float(false, RemoteControlSonarSettings->depth_gate_tilt, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, RemoteControlSonarSettings->applied_frequency, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, RemoteControlSonarSettings->element_number, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_CommonSystemSettings(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_CommonSystemSettings *CommonSystemSettings = &(store->CommonSystemSettings);
  s7k3_header *header = &(CommonSystemSettings->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_CommonSystemSettings(verbose, CommonSystemSettings, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_CommonSystemSettings;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_long(false, CommonSystemSettings->serial_number, &buffer[index]);
    index += 8;
    mb_put_binary_int(false, CommonSystemSettings->ping_number, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CommonSystemSettings->sound_velocity, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CommonSystemSettings->absorption, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CommonSystemSettings->spreading_loss, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->sequencer_control, &buffer[index]);
    index += 4;
    buffer[index] = CommonSystemSettings->mru_format;
    index++;
    buffer[index] = CommonSystemSettings->mru_baudrate;
    index++;
    buffer[index] = CommonSystemSettings->mru_parity;
    index++;
    buffer[index] = CommonSystemSettings->mru_databits;
    index++;
    buffer[index] = CommonSystemSettings->mru_stopbits;
    index++;
    buffer[index] = CommonSystemSettings->orientation;
    index++;
    buffer[index] = CommonSystemSettings->record_version;
    index++;
    mb_put_binary_float(false, CommonSystemSettings->motion_latency, &buffer[index]);
    index += 4;
    buffer[index] = CommonSystemSettings->svp_filter;
    index++;
    buffer[index] = CommonSystemSettings->sv_override;
    index++;
    mb_put_binary_short(false, CommonSystemSettings->activeenum, &buffer[index]);
    index += 2;
    mb_put_binary_int(false, CommonSystemSettings->active_id, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->system_mode, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->masterslave_mode, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->tracker_flags, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CommonSystemSettings->tracker_swathwidth, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_enable, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_obsize, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_sensitivity, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_detections, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_reserved[0], &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->multidetect_reserved[1], &buffer[index]);
    index += 2;
    for (int i = 0;i<4;i++) {
      buffer[index] = CommonSystemSettings->slave_ip[i];
      index++;
    }
    mb_put_binary_int(false, CommonSystemSettings->snippet_controlflags, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->snippet_minwindow, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->snippet_maxwindow, &buffer[index]);
    index += 4;
    mb_put_binary_int(false, CommonSystemSettings->fullrange_dualhead, &buffer[index]);
    index += 4;
    mb_put_binary_float(false, CommonSystemSettings->delay_multiplier, &buffer[index]);
    index += 4;
    buffer[index] = CommonSystemSettings->powersaving_mode;
    index++;
    buffer[index] = CommonSystemSettings->flags;
    index++;
    mb_put_binary_short(false, CommonSystemSettings->range_blank, &buffer[index]);
    index += 2;
    buffer[index] = CommonSystemSettings->startup_normalization;
    index++;
    buffer[index] = CommonSystemSettings->restore_pingrate;
    index++;
    buffer[index] = CommonSystemSettings->restore_power;
    index++;
    buffer[index] = CommonSystemSettings->sv_interlock;
    index++;
    buffer[index] = CommonSystemSettings->ignorepps_errors;
    index++;
    for (int i = 0;i<15;i++) {
      buffer[index] = CommonSystemSettings->reserved1[i];
      index++;
    }
    mb_put_binary_int(false, CommonSystemSettings->compressed_wcflags, &buffer[index]);
    index += 4;
    buffer[index] = CommonSystemSettings->deckmode;
    index++;
    buffer[index] = CommonSystemSettings->reserved2;
    index++;
    buffer[index] = CommonSystemSettings->powermode_flags;
    index++;
    buffer[index] = CommonSystemSettings->powermode_max;
    index++;
    mb_put_binary_float(false, CommonSystemSettings->water_temperature, &buffer[index]);
    index += 4;
    buffer[index] = CommonSystemSettings->sensor_override;
    index++;
    buffer[index] = CommonSystemSettings->sensor_dataflags;
    index++;
    buffer[index] = CommonSystemSettings->sensor_active;
    index++;
    buffer[index] = CommonSystemSettings->reserved3;
    index++;
    mb_put_binary_float(false, CommonSystemSettings->tracker_maxcoverage, &buffer[index]);
    index += 4;
    mb_put_binary_short(false, CommonSystemSettings->dutycycle_mode, &buffer[index]);
    index += 2;
    mb_put_binary_short(false, CommonSystemSettings->reserved4, &buffer[index]);
    index += 2;
    for (int i = 0;i<99;i++) {
      mb_put_binary_int(false, CommonSystemSettings->reserved5[i], &buffer[index]);
      index += 4;
    }

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SVFiltering(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SVFiltering *SVFiltering = &(store->SVFiltering);
  s7k3_header *header = &(SVFiltering->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SVFiltering(verbose, SVFiltering, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SVFiltering;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SystemLockStatus(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SystemLockStatus *SystemLockStatus = &(store->SystemLockStatus);
  s7k3_header *header = &(SystemLockStatus->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SystemLockStatus(verbose, SystemLockStatus, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SystemLockStatus;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    // Notdone

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SoundVelocity(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SoundVelocity *SoundVelocity = &(store->SoundVelocity);
  s7k3_header *header = &(SoundVelocity->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SoundVelocity(verbose, SoundVelocity, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SoundVelocity;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, SoundVelocity->soundvelocity, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_AbsorptionLoss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_AbsorptionLoss *AbsorptionLoss = &(store->AbsorptionLoss);
  s7k3_header *header = &(AbsorptionLoss->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_AbsorptionLoss(verbose, AbsorptionLoss, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_AbsorptionLoss;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, AbsorptionLoss->absorptionloss, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_SpreadingLoss(int verbose, int *bufferalloc, char **bufferptr, void *store_ptr, int *size, int *error) {
  int status = MB_SUCCESS;
  int index;
  char *buffer;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%d\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  s7k3_SpreadingLoss *SpreadingLoss = &(store->SpreadingLoss);
  s7k3_header *header = &(SpreadingLoss->header);

/* print out the data to be output */
#ifdef MBR_RESON7K3_DEBUG2
  if (verbose > 0)
#else
  if (verbose >= 2)
#endif
    mbsys_reson7k3_print_SpreadingLoss(verbose, SpreadingLoss, error);

  /* figure out size of output record */
  *size = MBSYS_RESON7K_RECORDHEADER_SIZE + MBSYS_RESON7K_RECORDTAIL_SIZE;
  *size += R7KHDRSIZE_SpreadingLoss;

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
    status = mbr_reson7k3_wr_header(verbose, buffer, &index, header, error);

    /* insert the data */
    index = header->Offset + 4;
    mb_put_binary_float(false, SpreadingLoss->spreadingloss, &buffer[index]);
    index += 4;

    /* reset the header size value */
    mb_put_binary_int(false, ((unsigned int)(index + 4)), &buffer[8]);

    /* now add the checksum */
    unsigned int checksum = 0;
    for (int i = 0; i < index; i++)
      checksum += (unsigned char)buffer[i];
    mb_put_binary_int(false, checksum, &buffer[index]);
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
int mbr_reson7k3_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  int status = MB_SUCCESS;
  int size = 0;
  size_t write_len = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;
  struct mbsys_reson7k3_struct *ostore = (struct mbsys_reson7k3_struct *)mb_io_ptr->store_data;

  /* get saved values */
  char **bufferptr = (char **)&mb_io_ptr->saveptr1;
  char *buffer = (char *)*bufferptr;
  int *bufferalloc = (int *)&mb_io_ptr->save6;
  int *fileheaders = (int *)&mb_io_ptr->save12;
  int *filecatalogoffsetoffset = (int *)&mb_io_ptr->save5;

  // The FileHeader record must be at the start of the file, but in general
  // MB-System programs will pass in comments before the first data records
  // are passed in from the original data file, including the FileHeader.
  // Therefore below any comments received before the FileHeader
  // will be buffered and then written immediately after the FileHeader
  // as SystemEventMessage records. After the FileHeader record is written
  // any comments will be written when received.
  //
  // The FileCatalog output data is stored in the FileCatalog_write
  // structure as the file is written. The FileCatalog record is written
  // when the file is closed, not when the input FileCatalog data are passed
  // through. When the FileCatalog is written to the end of the file the
  // FileHeader record is also updated with the offset to and size of the
  // FileCatalog record. These calls are made from mbr_reson7k3_deall() rather
  // than mbr_reson7k3_wr_data(), as only when the former is called is it
  // clear the file is finished.
  //
  // When survey data are passed in with store->kind == MB_DATA_DATA, all of
  // the ping-related records in memory associated with this ping are written
  // in a single pass. All other types of data correspond to single data records
  // and only a single record is written.

  // write FileHeader
  if (store->type == R7KRECID_FileHeader) {
    // ensure FileHeader has
#ifdef MBR_RESON7K3_DEBUG
    fprintf(stderr, "-->R7KRECID_FileHeader:                        %4.4X | %d\n", store->type, store->type);
#endif
    status = mbr_reson7k3_wr_FileHeader(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
    buffer = (char *)*bufferptr;
    write_len = (size_t)size;
    status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->FileHeader.header, error);
    status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    (*fileheaders)++;

    // Save byte offset in record to the value that will contain the byte offset
    // in the file to the start of the FileCatalog record at the end of the file
    // This value won't be defined until the file is finished, and so will be
    // overwritten just before the file is closed.
    *filecatalogoffsetoffset = store->FileHeader.header.OptionalDataOffset;

    for (int i = 0; i < ostore->n_saved_comments; i++) {
      store->type = R7KRECID_SystemEventMessage;
      store->kind = MB_DATA_COMMENT;
      store->SystemEventMessage.header = store->FileHeader.header;
      store->SystemEventMessage.header.RecordType = R7KRECID_SystemEventMessage;
      store->SystemEventMessage.serial_number = 0;
      store->SystemEventMessage.event_id = 1;
      store->SystemEventMessage.message_length = MIN(strlen(ostore->comments[i]) + 1, MB_PATH_MAXLINE - 1);
      store->SystemEventMessage.event_identifier = 0;
      if (store->SystemEventMessage.message_alloc
          < store->SystemEventMessage.message_length) {
        if ((status = mb_reallocd(verbose, __FILE__, __LINE__, MB_PATH_MAXLINE,
                              (void **)&store->SystemEventMessage.message,
                              error)) == MB_SUCCESS) {
          store->SystemEventMessage.message_alloc = MB_PATH_MAXLINE;
        }
        else
          store->SystemEventMessage.message_alloc = 0;
      }
      if (store->SystemEventMessage.message_alloc
          >= store->SystemEventMessage.message_length) {
        strncpy(store->SystemEventMessage.message, ostore->comments[i], store->SystemEventMessage.message_alloc-1);
        status = mbr_reson7k3_wr_SystemEventMessage(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SystemEventMessage.header, error);
        status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
      }
    }
  }

  /* call appropriate writing routines for ping data */
  else if (store->kind == MB_DATA_DATA) {
    /* Write all of the records in memory */

    /* Reson 7k sonar settings (record 7000) */
    if (store->read_SonarSettings == true) {
      store->type = R7KRECID_SonarSettings;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_SonarSettings:                     %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_SonarSettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SonarSettings.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }
    /* Reson 7k match filter (record 7002) */
    if (status == MB_SUCCESS && store->read_MatchFilter == true) {
      store->type = R7KRECID_MatchFilter;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_MatchFilter:                       %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_MatchFilter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->MatchFilter.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k beam geometry (record 7004) */
    if (status == MB_SUCCESS && store->read_BeamGeometry == true) {
      store->type = R7KRECID_BeamGeometry;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_BeamGeometry:                    --%4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_BeamGeometry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->BeamGeometry.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k bathymetry (record 7006) */
    if (status == MB_SUCCESS && store->read_Bathymetry == true) {
      store->type = R7KRECID_Bathymetry;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_Bathymetry:                          %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_Bathymetry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Bathymetry.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k SideScan imagery data (record 7007) */
    if (status == MB_SUCCESS && store->read_SideScan == true) {
      store->type = R7KRECID_SideScan;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_SideScan:                          %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_SideScan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SideScan.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k WaterColumn data (record 7008) */
    if (status == MB_SUCCESS && store->read_WaterColumn == true) {
      store->type = R7KRECID_WaterColumn;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_WaterColumn:                       %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_WaterColumn(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->WaterColumn.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k VerticalDepth data (record 7009) */
    if (status == MB_SUCCESS && store->read_VerticalDepth == true) {
      store->type = R7KRECID_VerticalDepth;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_VerticalDepth:                     %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_VerticalDepth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->VerticalDepth.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k TVG data (record 7010) */
    if (status == MB_SUCCESS && store->read_TVG == true) {
      store->type = R7KRECID_TVG;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_TVG:                               %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_TVG(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->TVG.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k Image data (record 7011) */
    if (status == MB_SUCCESS && store->read_Image == true) {
      store->type = R7KRECID_Image;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_Image:                             %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_Image(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Image.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k PingMotion (record 7012) */
    if (status == MB_SUCCESS && store->read_PingMotion == true) {
      store->type = R7KRECID_PingMotion;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_PingMotion:                        %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_PingMotion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->PingMotion.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k DetectionDataSetup (record 7017) */
    if (status == MB_SUCCESS && store->read_DetectionDataSetup == true) {
      store->type = R7KRECID_DetectionDataSetup;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_DetectionDataSetup:                %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_DetectionDataSetup(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->DetectionDataSetup.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k Beamformed magnitude and phase data (record 7018) */
    if (status == MB_SUCCESS && store->read_Beamformed == true) {
      store->type = R7KRECID_Beamformed;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_Beamformed:                        %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_Beamformed(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Beamformed.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k RawDetection (record 7027) */
    if (status == MB_SUCCESS && store->read_RawDetection == true) {
      store->type = R7KRECID_RawDetection;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_RawDetection:                      %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_RawDetection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RawDetection.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k Snippet (record 7028) */
    if (status == MB_SUCCESS && store->read_Snippet == true) {
      store->type = R7KRECID_Snippet;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_Snippet:                           %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_Snippet(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Snippet.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k CompressedBeamformedMagnitude Data (Record 7041) */
    if (status == MB_SUCCESS && store->read_CompressedBeamformedMagnitude == true) {
      store->type = R7KRECID_CompressedBeamformedMagnitude;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_CompressedBeamformedMagnitude:     %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_CompressedBeamformedMagnitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CompressedBeamformedMagnitude.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k CompressedWaterColumn Data (Record 7042) */
    if (status == MB_SUCCESS && store->read_CompressedWaterColumn == true) {
      store->type = R7KRECID_CompressedWaterColumn;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_CompressedWaterColumn:             %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_CompressedWaterColumn(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CompressedWaterColumn.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k SegmentedRawDetection Data (part of Record 7047) */
    if (status == MB_SUCCESS && store->read_SegmentedRawDetection == true) {
      store->type = R7KRECID_SegmentedRawDetection;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_SegmentedRawDetection:             %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_SegmentedRawDetection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SegmentedRawDetection.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k CalibratedBeam Data (Record 7048) */
    if (status == MB_SUCCESS && store->read_CalibratedBeam == true) {
      store->type = R7KRECID_CalibratedBeam;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_CalibratedBeam:                    %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_CalibratedBeam(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibratedBeam.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k CalibratedSideScan Data (part of record 7057) */
    if (status == MB_SUCCESS && store->read_CalibratedSideScan == true) {
      store->type = R7KRECID_CalibratedSideScan;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_CalibratedSideScan:                %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_CalibratedSideScan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibratedSideScan.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k SnippetBackscatteringStrength (Record 7058) */
    if (status == MB_SUCCESS && store->read_SnippetBackscatteringStrength == true) {
      store->type = R7KRECID_SnippetBackscatteringStrength;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_SnippetBackscatteringStrength:     %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_SnippetBackscatteringStrength(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SnippetBackscatteringStrength.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Reson 7k RemoteControlSonarSettings settings (record 7503) */
    if (status == MB_SUCCESS && store->read_RemoteControlSonarSettings == true) {
      store->type = R7KRECID_RemoteControlSonarSettings;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_RemoteControlSonarSettings:      %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_RemoteControlSonarSettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RemoteControlSonarSettings.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }

    /* Processed sidescan - MB-System extension to s7k3 format (record 3199) */
    if (status == MB_SUCCESS && store->read_ProcessedSideScan == true) {
      store->type = R7KRECID_ProcessedSideScan;
#ifdef MBR_RESON7K3_DEBUG
      fprintf(stderr, "**>R7KRECID_ProcessedSideScan:                 %4.4X | %d\n", store->type, store->type);
#endif
      status = mbr_reson7k3_wr_ProcessedSideScan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->ProcessedSideScan.header, error);
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);
    }
  }

  // if comment and no FileHeader already written then store the comment in the store structure
  // associated with the output mb_io_ptr
  else if (store->kind == MB_DATA_COMMENT && *fileheaders == 0) {
    if (ostore->n_saved_comments < MBSYS_RESON7K_MAX_BUFFERED_COMMENTS) {
      strncpy(ostore->comments[ostore->n_saved_comments], ostore->SystemEventMessage.message, MB_PATH_MAXLINE);
      (ostore->n_saved_comments)++;
    }
  }

  /* call appropriate writing routine for other records */
  else {
#ifdef MBR_RESON7K3_DEBUG
    if (store->type ==R7KRECID_None)
      fprintf(stderr, "-->R7KRECID_None:                              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_ReferencePoint)
      fprintf(stderr, "-->R7KRECID_ReferencePoint:                    %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_UncalibratedSensorOffset)
      fprintf(stderr, "-->R7KRECID_UncalibratedSensorOffset:          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CalibratedSensorOffset)
      fprintf(stderr, "-->R7KRECID_CalibratedSensorOffset:            %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Position)
      fprintf(stderr, "-->R7KRECID_Position:                          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CustomAttitude)
      fprintf(stderr, "-->R7KRECID_CustomAttitude:                    %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Tide)
      fprintf(stderr, "-->R7KRECID_Tide:                              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Altitude)
      fprintf(stderr, "-->R7KRECID_Altitude:                          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_MotionOverGround)
      fprintf(stderr, "-->R7KRECID_MotionOverGround:                  %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Depth)
      fprintf(stderr, "-->R7KRECID_Depth:                             %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SoundVelocityProfile)
      fprintf(stderr, "-->R7KRECID_SoundVelocityProfile:              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CTD)
      fprintf(stderr, "-->R7KRECID_CTD:                               %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Geodesy)
      fprintf(stderr, "-->R7KRECID_Geodesy:                           %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RollPitchHeave)
      fprintf(stderr, "-->R7KRECID_RollPitchHeave:                    %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Heading)
      fprintf(stderr, "-->R7KRECID_Heading:                           %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SurveyLine)
      fprintf(stderr, "-->R7KRECID_SurveyLine:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Navigation)
      fprintf(stderr, "-->R7KRECID_Navigation:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Attitude)
      fprintf(stderr, "-->R7KRECID_Attitude:                          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_PanTilt)
      fprintf(stderr, "-->R7KRECID_PanTilt:                           %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SonarInstallationIDs)
      fprintf(stderr, "-->R7KRECID_SonarInstallationIDs:              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SonarPipeEnvironment)
      fprintf(stderr, "-->R7KRECID_SonarPipeEnvironment:              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_ContactOutput)
      fprintf(stderr, "-->R7KRECID_ContactOutput:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_ProcessedSideScan)
      fprintf(stderr, "-->R7KRECID_ProcessedSideScan:                 %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SonarSettings)
      fprintf(stderr, "-->R7KRECID_SonarSettings:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Configuration)
      fprintf(stderr, "-->R7KRECID_Configuration:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_MatchFilter)
      fprintf(stderr, "-->R7KRECID_MatchFilter:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_FirmwareHardwareConfiguration)
      fprintf(stderr, "-->R7KRECID_FirmwareHardwareConfiguration:     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_BeamGeometry)
      fprintf(stderr, "-->R7KRECID_BeamGeometry:                      %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Bathymetry)
      fprintf(stderr, "-->R7KRECID_Bathymetry:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SideScan)
      fprintf(stderr, "-->R7KRECID_SideScan:                          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_WaterColumn)
      fprintf(stderr, "-->R7KRECID_WaterColumn:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_VerticalDepth)
      fprintf(stderr, "-->R7KRECID_VerticalDepth:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_TVG)
      fprintf(stderr, "-->R7KRECID_TVG:                               %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Image)
      fprintf(stderr, "-->R7KRECID_Image:                             %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_PingMotion)
      fprintf(stderr, "-->R7KRECID_PingMotion:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_AdaptiveGate)
      fprintf(stderr, "-->R7KRECID_AdaptiveGate:                      %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_DetectionDataSetup)
      fprintf(stderr, "-->R7KRECID_DetectionDataSetup:                %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Beamformed)
      fprintf(stderr, "-->R7KRECID_Beamformed:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_VernierProcessingDataRaw)
      fprintf(stderr, "-->R7KRECID_VernierProcessingDataRaw:          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_BITE)
      fprintf(stderr, "-->R7KRECID_BITE:                              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SonarSourceVersion)
      fprintf(stderr, "-->R7KRECID_SonarSourceVersion:                %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_WetEndVersion8k)
      fprintf(stderr, "-->R7KRECID_WetEndVersion8k:                   %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RawDetection)
      fprintf(stderr, "-->R7KRECID_RawDetection:                      %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Snippet)
      fprintf(stderr, "-->R7KRECID_Snippet:                           %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_VernierProcessingDataFiltered)
      fprintf(stderr, "-->R7KRECID_VernierProcessingDataFiltered:     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_InstallationParameters)
      fprintf(stderr, "-->R7KRECID_InstallationParameters:            %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_BITESummary)
      fprintf(stderr, "-->R7KRECID_BITESummary:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CompressedBeamformedMagnitude)
      fprintf(stderr, "-->R7KRECID_CompressedBeamformedMagnitude:     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CompressedWaterColumn)
      fprintf(stderr, "-->R7KRECID_CompressedWaterColumn:             %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SegmentedRawDetection)
      fprintf(stderr, "-->R7KRECID_SegmentedRawDetection:             %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CalibratedBeam)
      fprintf(stderr, "-->R7KRECID_CalibratedBeam:                    %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SystemEvents)
      fprintf(stderr, "-->R7KRECID_SystemEvents:                      %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SystemEventMessage)
      fprintf(stderr, "-->R7KRECID_SystemEventMessage:                %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RDRRecordingStatus)
      fprintf(stderr, "-->R7KRECID_RDRRecordingStatus:                %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_Subscriptions)
      fprintf(stderr, "-->R7KRECID_Subscriptions:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RDRStorageRecording)
      fprintf(stderr, "-->R7KRECID_RDRStorageRecording:               %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CalibrationStatus)
      fprintf(stderr, "-->R7KRECID_CalibrationStatus:                 %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CalibratedSideScan)
      fprintf(stderr, "-->R7KRECID_CalibratedSideScan:                %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SnippetBackscatteringStrength)
      fprintf(stderr, "-->R7KRECID_SnippetBackscatteringStrength:     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_MB2Status)
      fprintf(stderr, "-->R7KRECID_MB2Status:                         %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_FileHeader)
      fprintf(stderr, "-->R7KRECID_FileHeader:                        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_FileCatalog)
      fprintf(stderr, "-->R7KRECID_FileCatalog:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_TimeMessage)
      fprintf(stderr, "-->R7KRECID_TimeMessage:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RemoteControl)
      fprintf(stderr, "-->R7KRECID_RemoteControl:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RemoteControlAcknowledge)
      fprintf(stderr, "-->R7KRECID_RemoteControlAcknowledge:          %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RemoteControlNotAcknowledge)
      fprintf(stderr, "-->R7KRECID_RemoteControlNotAcknowledge:       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_RemoteControlSonarSettings)
      fprintf(stderr, "-->R7KRECID_RemoteControlSonarSettings:        %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_CommonSystemSettings)
      fprintf(stderr, "-->R7KRECID_CommonSystemSettings:              %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SVFiltering)
      fprintf(stderr, "-->R7KRECID_SVFiltering:                       %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SystemLockStatus)
      fprintf(stderr, "-->R7KRECID_SystemLockStatus:                  %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SoundVelocity)
      fprintf(stderr, "-->R7KRECID_SoundVelocity:                     %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_AbsorptionLoss)
      fprintf(stderr, "-->R7KRECID_AbsorptionLoss:                    %4.4X | %d\n", store->type, store->type);
    if (store->type ==R7KRECID_SpreadingLoss)
      fprintf(stderr, "-->R7KRECID_SpreadingLoss:                     %4.4X | %d\n", store->type, store->type);
#endif

    switch (store->type) {
      case R7KRECID_ReferencePoint:
        status = mbr_reson7k3_wr_ReferencePoint(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->ReferencePoint.header, error);
        break;
      case R7KRECID_UncalibratedSensorOffset:
        status = mbr_reson7k3_wr_UncalibratedSensorOffset(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->UncalibratedSensorOffset.header, error);
        break;
      case R7KRECID_CalibratedSensorOffset:
        status = mbr_reson7k3_wr_CalibratedSensorOffset(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibratedSensorOffset.header, error);
        break;
      case R7KRECID_Position:
        status = mbr_reson7k3_wr_Position(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Position.header, error);
        break;
      case R7KRECID_CustomAttitude:
        status = mbr_reson7k3_wr_CustomAttitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CustomAttitude.header, error);
        break;
      case R7KRECID_Tide:
        status = mbr_reson7k3_wr_Tide(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Tide.header, error);
        break;
      case R7KRECID_Altitude:
        status = mbr_reson7k3_wr_Altitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Altitude.header, error);
        break;
      case R7KRECID_MotionOverGround:
        status = mbr_reson7k3_wr_MotionOverGround(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->MotionOverGround.header, error);
        break;
      case R7KRECID_Depth:
        status = mbr_reson7k3_wr_Depth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Depth.header, error);
        break;
      case R7KRECID_SoundVelocityProfile:
        status = mbr_reson7k3_wr_SoundVelocityProfile(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SoundVelocityProfile.header, error);
        break;
      case R7KRECID_CTD:
        status = mbr_reson7k3_wr_CTD(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CTD.header, error);
        break;
      case R7KRECID_Geodesy:
        status = mbr_reson7k3_wr_Geodesy(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Geodesy.header, error);
        break;
      case R7KRECID_RollPitchHeave:
        status = mbr_reson7k3_wr_RollPitchHeave(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RollPitchHeave.header, error);
        break;
      case R7KRECID_Heading:
        status = mbr_reson7k3_wr_Heading(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Heading.header, error);
        break;
      case R7KRECID_SurveyLine:
        status = mbr_reson7k3_wr_SurveyLine(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SurveyLine.header, error);
        break;
      case R7KRECID_Navigation:
        status = mbr_reson7k3_wr_Navigation(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Navigation.header, error);
        break;
      case R7KRECID_Attitude:
        status = mbr_reson7k3_wr_Attitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Attitude.header, error);
        break;
      case R7KRECID_PanTilt:
        status = mbr_reson7k3_wr_PanTilt(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->PanTilt.header, error);
        break;
      case R7KRECID_SonarInstallationIDs:
        status = mbr_reson7k3_wr_SonarInstallationIDs(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SonarInstallationIDs.header, error);
        break;
      case R7KRECID_SonarPipeEnvironment:
        status = mbr_reson7k3_wr_SonarPipeEnvironment(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SonarPipeEnvironment.header, error);
        break;
      case R7KRECID_ContactOutput:
        status = mbr_reson7k3_wr_ContactOutput(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->ContactOutput.header, error);
        break;
      //case R7KRECID_ProcessedSidescan:
        //status = mbr_reson7k3_wr_ProcessedSidescan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->ProcessedSidescan.header, error);
        //break;
      //case R7KRECID_SonarSettings:
        //status = mbr_reson7k3_wr_SonarSettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SonarSettings.header, error);
        //break;
      case R7KRECID_Configuration:
        status = mbr_reson7k3_wr_Configuration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Configuration.header, error);
        break;
      //case R7KRECID_MatchFilter:
        //status = mbr_reson7k3_wr_MatchFilter(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->MatchFilter.header, error);
        //break;
      case R7KRECID_FirmwareHardwareConfiguration:
        status = mbr_reson7k3_wr_FirmwareHardwareConfiguration(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->FirmwareHardwareConfiguration.header, error);
        break;
      //case R7KRECID_BeamGeometry:
        //status = mbr_reson7k3_wr_BeamGeometry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->BeamGeometry.header, error);
        //break;
      //case R7KRECID_Bathymetry:
        //status = mbr_reson7k3_wr_Bathymetry(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Bathymetry.header, error);
        //break;
      //case R7KRECID_SideScan:
        //status = mbr_reson7k3_wr_SideScan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SideScan.header, error);
        //break;
      //case R7KRECID_WaterColumn:
        //status = mbr_reson7k3_wr_WaterColumn(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->WaterColumn.header, error);
        //break;
      //case R7KRECID_VerticalDepth:
        //status = mbr_reson7k3_wr_VerticalDepth(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->VerticalDepth.header, error);
        //break;
      //case R7KRECID_TVG:
        //status = mbr_reson7k3_wr_TVG(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->TVG.header, error);
        //break;
      //case R7KRECID_Image:
        //status = mbr_reson7k3_wr_Image(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Image.header, error);
        //break;
      //case R7KRECID_PingMotion:
        //status = mbr_reson7k3_wr_PingMotion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->PingMotion.header, error);
        //break;
      //case R7KRECID_AdaptiveGate:
        //status = mbr_reson7k3_wr_AdaptiveGate(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->AdaptiveGate.header, error);
        //break;
      //case R7KRECID_DetectionDataSetup:
        //status = mbr_reson7k3_wr_DetectionDataSetup(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->DetectionDataSetup.header, error);
        //break;
      //case R7KRECID_Beamformed:
        //status = mbr_reson7k3_wr_Beamformed(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Beamformed.header, error);
        //break;
      //case R7KRECID_VernierProcessingDataRaw:
        //status = mbr_reson7k3_wr_VernierProcessingDataRaw(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->VernierProcessingDataRaw.header, error);
        //break;
      case R7KRECID_BITE:
        status = mbr_reson7k3_wr_BITE(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->BITE.header, error);
        break;
      case R7KRECID_SonarSourceVersion:
        status = mbr_reson7k3_wr_SonarSourceVersion(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SonarSourceVersion.header, error);
        break;
      case R7KRECID_WetEndVersion8k:
        status = mbr_reson7k3_wr_WetEndVersion8k(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->WetEndVersion8k.header, error);
        break;
      //case R7KRECID_RawDetection:
        //status = mbr_reson7k3_wr_RawDetection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RawDetection.header, error);
        //break;
      //case R7KRECID_Snippet:
        //status = mbr_reson7k3_wr_Snippet(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Snippet.header, error);
        //break;
      //case R7KRECID_VernierProcessingDataFiltered:
        //status = mbr_reson7k3_wr_VernierProcessingDataFiltered(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->VernierProcessingDataFiltered.header, error);
        //break;
      case R7KRECID_InstallationParameters:
        status = mbr_reson7k3_wr_InstallationParameters(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->InstallationParameters.header, error);
        break;
      case R7KRECID_BITESummary:
        status = mbr_reson7k3_wr_BITESummary(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->BITESummary.header, error);
        break;
      //case R7KRECID_CompressedBeamformedMagnitude:
        //status = mbr_reson7k3_wr_CompressedBeamformedMagnitude(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CompressedBeamformedMagnitude.header, error);
        //break;
      //case R7KRECID_CompressedWaterColumn:
        //status = mbr_reson7k3_wr_CompressedWaterColumn(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CompressedWaterColumn.header, error);
        //break;
      //case R7KRECID_SegmentedRawDetection:
        //status = mbr_reson7k3_wr_SegmentedRawDetection(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SegmentedRawDetection.header, error);
        //break;
      //case R7KRECID_CalibratedBeam:
        //status = mbr_reson7k3_wr_CalibratedBeam(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibratedBeam.header, error);
        //break;
      case R7KRECID_SystemEvents:
        status = mbr_reson7k3_wr_SystemEvents(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SystemEvents.header, error);
        break;
      case R7KRECID_SystemEventMessage:
        status = mbr_reson7k3_wr_SystemEventMessage(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SystemEventMessage.header, error);
        break;
      case R7KRECID_RDRRecordingStatus:
        status = mbr_reson7k3_wr_RDRRecordingStatus(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RDRRecordingStatus.header, error);
        break;
      case R7KRECID_Subscriptions:
        status = mbr_reson7k3_wr_Subscriptions(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->Subscriptions.header, error);
        break;
      case R7KRECID_RDRStorageRecording:
        status = mbr_reson7k3_wr_RDRStorageRecording(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RDRStorageRecording.header, error);
        break;
      case R7KRECID_CalibrationStatus:
        status = mbr_reson7k3_wr_CalibrationStatus(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibrationStatus.header, error);
        break;
      //case R7KRECID_CalibratedSideScan:
        //status = mbr_reson7k3_wr_CalibratedSideScan(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CalibratedSideScan.header, error);
        //break;
      //case R7KRECID_SnippetBackscatteringStrength:
        //status = mbr_reson7k3_wr_SnippetBackscatteringStrength(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SnippetBackscatteringStrength.header, error);
        //break;
      case R7KRECID_MB2Status:
        status = mbr_reson7k3_wr_MB2Status(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->MB2Status.header, error);
        break;
      //case R7KRECID_FileHeader:
        //status = mbr_reson7k3_wr_FileHeader(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->FileHeader.header, error);
        //(*fileheaders)++;
        //break;
      case R7KRECID_FileCatalog:
        // write catalog when file is closed rather than when old catalog is read
        // - not all input files will have a catalog
        //status = mbr_reson7k3_wr_FileCatalog(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        status = MB_SUCCESS;
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->FileCatalog.header, error);
        break;
      case R7KRECID_TimeMessage:
        status = mbr_reson7k3_wr_TimeMessage(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->TimeMessage.header, error);
        break;
      case R7KRECID_RemoteControl:
        status = mbr_reson7k3_wr_RemoteControl(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RemoteControl.header, error);
        break;
      case R7KRECID_RemoteControlAcknowledge:
        status = mbr_reson7k3_wr_RemoteControlAcknowledge(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RemoteControlAcknowledge.header, error);
        break;
      case R7KRECID_RemoteControlNotAcknowledge:
        status = mbr_reson7k3_wr_RemoteControlNotAcknowledge(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RemoteControlNotAcknowledge.header, error);
        break;
      //case R7KRECID_RemoteControlSonarSettings:
        //status = mbr_reson7k3_wr_RemoteControlSonarSettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        //buffer = (char *)*bufferptr;
        //write_len = (size_t)size;
        //status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->RemoteControlSonarSettings.header, error);
        //break;
      case R7KRECID_CommonSystemSettings:
        status = mbr_reson7k3_wr_CommonSystemSettings(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->CommonSystemSettings.header, error);
        break;
      case R7KRECID_SVFiltering:
        status = mbr_reson7k3_wr_SVFiltering(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SVFiltering.header, error);
        break;
      case R7KRECID_SystemLockStatus:
        status = mbr_reson7k3_wr_SystemLockStatus(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SystemLockStatus.header, error);
        break;
      case R7KRECID_SoundVelocity:
        status = mbr_reson7k3_wr_SoundVelocity(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SoundVelocity.header, error);
        break;
      case R7KRECID_AbsorptionLoss:
        status = mbr_reson7k3_wr_AbsorptionLoss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->AbsorptionLoss.header, error);
        break;
      case R7KRECID_SpreadingLoss:
        status = mbr_reson7k3_wr_SpreadingLoss(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
        buffer = (char *)*bufferptr;
        write_len = (size_t)size;
        status = mbr_reson7k3_FileCatalog_update(verbose, mbio_ptr, store_ptr, write_len, &store->SpreadingLoss.header, error);
        break;

      default:
        fprintf(stderr, "call nothing bad kind: %d type %x\n", store->kind, store->type);
        status = MB_FAILURE;
        *error = MB_ERROR_BAD_KIND;
    }

    // finally write the record to the output file
    if (status == MB_SUCCESS) {

      // write the record
      buffer = (char *)*bufferptr;
      write_len = (size_t)size;
      status = mb_fileio_put(verbose, mbio_ptr, buffer, &write_len, error);

    }
  }

#ifdef MBR_RESON7K3_DEBUG2
  fprintf(stderr, "RESON7K3 DATA WRITTEN: type:%d status:%d error:%d\n\n", store->kind, status, *error);
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
int mbr_wt_reson7k3(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
  struct mbsys_reson7k3_struct *store = (struct mbsys_reson7k3_struct *)store_ptr;

  /* write next data to file */
  const int status = mbr_reson7k3_wr_data(verbose, mb_io_ptr, store, error);

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

int mbr_reson7k3_FileCatalog_compare2(const void *a, const void *b) {
  const s7k3_filecatalogdata *aa = (s7k3_filecatalogdata *) a;
  const s7k3_filecatalogdata *bb = (s7k3_filecatalogdata *) b;

  // just do time comparison
  if (aa->time_d < bb->time_d) return -1;
  if (aa->time_d > bb->time_d) return 1;
  return 0;
}

/*--------------------------------------------------------------------*/
int mbr_register_reson7k3(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_reson7k3(
      verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
      mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
      &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
      &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
      &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_reson7k3;
  mb_io_ptr->mb_io_format_free = &mbr_dem_reson7k3;
  mb_io_ptr->mb_io_store_alloc = &mbsys_reson7k3_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_reson7k3_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_reson7k3;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_reson7k3;
  mb_io_ptr->mb_io_dimensions = &mbsys_reson7k3_dimensions;
  mb_io_ptr->mb_io_pingnumber = &mbsys_reson7k3_pingnumber;
  mb_io_ptr->mb_io_sonartype = &mbsys_reson7k3_sonartype;
  mb_io_ptr->mb_io_sidescantype = &mbsys_reson7k3_sidescantype;
  mb_io_ptr->mb_io_preprocess = &mbsys_reson7k3_preprocess;
  mb_io_ptr->mb_io_extract_platform = &mbsys_reson7k3_extract_platform;
  mb_io_ptr->mb_io_extract = &mbsys_reson7k3_extract;
  mb_io_ptr->mb_io_insert = &mbsys_reson7k3_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_reson7k3_extract_nav;
  mb_io_ptr->mb_io_extract_nnav = &mbsys_reson7k3_extract_nnav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_reson7k3_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_reson7k3_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_reson7k3_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_reson7k3_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_reson7k3_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_reson7k3_detects;
  mb_io_ptr->mb_io_gains = &mbsys_reson7k3_gains;
  mb_io_ptr->mb_io_copyrecord = &mbsys_reson7k3_copy;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;
  mb_io_ptr->mb_io_extract_segytraceheader = NULL;
  mb_io_ptr->mb_io_extract_segy = NULL;
  mb_io_ptr->mb_io_insert_segy = NULL;
  mb_io_ptr->mb_io_ctd = &mbsys_reson7k3_ctd;
  mb_io_ptr->mb_io_ancilliarysensor = &mbsys_reson7k3_ancilliarysensor;

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
