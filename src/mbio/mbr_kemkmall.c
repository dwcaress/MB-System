/*--------------------------------------------------------------------
 *    The MB-system:  mbr_kemkmall.c  5/25/2018
 *
 *    Copyright (c) 2018-2023 by
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
 * mbr_kemkmall.c contains the functions for reading and writing
 * multibeam data in the KEMKMALL format.
 * These functions include:
 *   mbr_alm_kemkmall  - allocate read/write memory
 *   mbr_dem_kemkmall   - deallocate read/write memory
 *   mbr_rt_kemkmall     - read and translate data
 *   mbr_wt_kemkmall     - translate and write data
 *
 * Author:  B. Y. Raanan
 * Date:  May 25, 2018
 *
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mb_define.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_status.h"
#include "mbsys_kmbes.h"

/* turn on debug statements here */
// #define MBR_KEMKMALL_DEBUG 1

/*--------------------------------------------------------------------*/
int mbr_info_kemkmall(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
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
  *system = MB_SYS_KMBES;
  *beams_bath_max = MBSYS_KMBES_MAX_NUM_BEAMS;
  *beams_amp_max = MBSYS_KMBES_MAX_NUM_BEAMS;
  *pixels_ss_max = MBSYS_KMBES_MAX_PIXELS;
  strncpy(format_name, "KEMKMALL", MB_NAME_LENGTH);
  strncpy(system_name, "KMBES", MB_NAME_LENGTH);
  strncpy(format_description, "Format name:          MBF_KEMKMALL\n"
          "Informal Description: Kongsberg multibeam echosounder system kmall datagram format\n"
          "Attributes:           Kongsberg fourth generation multibeam sonars (EM2040, EM712, \n"
          "                      EM304, EM124), bathymetry, amplitude, backscatter, variable beams, \n"
          "                      binary datagrams, Kongsberg.\n", MB_DESCRIPTION_LENGTH);
  *numfile = 1;
  *filetype = MB_FILETYPE_SINGLE;
  *variable_beams = true;
  *traveltime = true;
  *beam_flagging = true;
  *platform_source = MB_DATA_NONE;
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

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_alm_kemkmall(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
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
  mb_io_ptr->structure_size = 0;
  mb_io_ptr->data_structure_size = 0;
  int status = mbsys_kmbes_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  /* allocate starting memory for data record buffer */
  char **bufferptr = (char **)&mb_io_ptr->raw_data;
  size_t *bufferalloc = (size_t *)&mb_io_ptr->structure_size;

  *bufferptr = NULL;
  *bufferalloc = 0;
  if (status == MB_SUCCESS) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, MBSYS_KMBES_START_BUFFER_SIZE, (void **)bufferptr, error);
    if (status == MB_SUCCESS)
      *bufferalloc = MBSYS_KMBES_START_BUFFER_SIZE;
  }

  /* prep memory for data datagram index table */
  mb_io_ptr->saveptr1 = NULL;
  mb_io_ptr->save1 = 0;

  /* set store variables for asynchronous data sources */
  int *nav_saved = (int *)&mb_io_ptr->save3;
  int *heading_saved = (int *)&mb_io_ptr->save4;
  int *attitude_saved = (int *)&mb_io_ptr->save5;
  int *sensordepth_saved = (int *)&mb_io_ptr->save6;
  int *kluge_set = (int *)&mb_io_ptr->save10;
  *nav_saved = MB_DATA_NONE;
  *heading_saved = MB_DATA_NONE;
  *attitude_saved = MB_DATA_NONE;
  *sensordepth_saved = MB_DATA_NONE;
  *kluge_set = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_kemkmall(int verbose, void *mbio_ptr, int *error) {
  struct mbsys_kmbes_index_table *dgm_index_table = NULL;
  int *dgm_count = NULL;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  int status = MB_SUCCESS;

  /* deallocate reading/writing buffer */
  if (mb_io_ptr->raw_data != NULL && mb_io_ptr->structure_size > 0) {
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
    mb_io_ptr->raw_data = NULL;
    mb_io_ptr->data_structure_size = 0;
  }

  /* deallocate file indexing array */
  if (mb_io_ptr->saveptr1 != NULL) {

    /* get pointers to datagram index table */
    dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;
    dgm_count = (int *)&mb_io_ptr->save1;

    if (dgm_index_table->num_alloc > 0) {
      status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&dgm_index_table->indextable), error);
    }
    status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&dgm_index_table), error);
    dgm_index_table = NULL;
    *dgm_count = 0;
  }

  /* deallocate memory for data descriptor */
  status = mbsys_kmbes_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/

int mbr_kemkmall_create_dgm_index_table(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  struct mbsys_kmbes_index_table *dgm_index_table = NULL;
  size_t size_bytes = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* we will store the datagram index table in mbio descriptor field saveptr1 */
  dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;

  /* get pointer to raw data structure */
  // struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* allocate the datagram index table struct (vector struct) */
  int status = mb_mallocd(verbose, __FILE__, __LINE__,
             sizeof(struct mbsys_kmbes_index_table), (void **)(&dgm_index_table), error);

  if (status == MB_SUCCESS) {
    dgm_index_table->dgm_count = 0;

    /* allocate the datagram index table array */
    size_bytes = MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE * sizeof(struct mbsys_kmbes_index);
    status = mb_mallocd(verbose, __FILE__, __LINE__, size_bytes,
               (void **)(&dgm_index_table->indextable), error);
    if (status == MB_SUCCESS) {
      dgm_index_table->num_alloc = MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE;
    } else {
      dgm_index_table->num_alloc = 0;

    }
  }

  /* init internal data structure variables */
  mb_io_ptr->saveptr1 = (void *)dgm_index_table;
  mb_io_ptr->save1 = 0;     // most recently read entry in index table, after indexing
  mb_io_ptr->save2 = false; // file has been indexed

    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:      %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  /* return status */
  return(status);
};
/*--------------------------------------------------------------------*/

int mbr_kemkmall_add_dgm_to_dgm_index_table(int verbose, void *index_table_ptr,
                                            void *new_index_ptr, int *error) {
  struct mbsys_kmbes_index_table *dgm_index_table = NULL;
  struct mbsys_kmbes_index *new_dgm_index = NULL;
  size_t dgm_count = 0;
  size_t new_num_alloc = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:         %d\n", verbose);
    fprintf(stderr, "dbg2       index_table_ptr: %p\n", (void *)index_table_ptr);
    fprintf(stderr, "dbg2       new_index_ptr:   %p\n", (void *)new_index_ptr);
  }

  /* get pointer to datagram index table */
  dgm_index_table = (struct mbsys_kmbes_index_table *)index_table_ptr;

  /* get pointer to the new datagram index structure */
  new_dgm_index = (struct mbsys_kmbes_index *)new_index_ptr;

  int status = MB_SUCCESS;

  /* reallocate the datagram index table array if needed */
  dgm_count = dgm_index_table->dgm_count;
  if (dgm_count >= (dgm_index_table->num_alloc-1)) {
    new_num_alloc = dgm_index_table->num_alloc + MBSYS_KMBES_INDEX_TABLE_BLOCK_SIZE;

    /* reallocate the datagram index table array */
    status = mb_reallocd(verbose, __FILE__, __LINE__,
               sizeof(struct mbsys_kmbes_index) * new_num_alloc,
               (void **)(&dgm_index_table->indextable), error);

    if (status == MB_SUCCESS) {
      dgm_index_table->num_alloc = new_num_alloc;
    } else {
      dgm_index_table->num_alloc = 0;
      dgm_index_table->dgm_count = 0;
    }
  }

  if (status == MB_SUCCESS) {
    dgm_index_table->indextable[dgm_count] = *new_dgm_index;
    dgm_index_table->indextable[dgm_count].index_org = dgm_count;
    dgm_index_table->dgm_count++;
  }

    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:      %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  /* return status */
  return(status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_indextable_compare(const void *a, const void *b) {
  struct mbsys_kmbes_index *aa = NULL;
  struct mbsys_kmbes_index *bb = NULL;
  int result = 0;

  aa = (struct mbsys_kmbes_index*) a;
  bb = (struct mbsys_kmbes_index*) b;

  /* compare so that index table of datagrams is ordered correctly
      - Any comment datagrams should be at the beginning in time order
      - The first datagram after any comments should be the IIP (installation parameters)
      - The first datagram after the IIP should be the first IOP (runtime parameters)
      - The first datagram after the first IOP should be the first SVP (sound velocity profile)
      - The first datagram after the first SVP should be the first FCF (backscatter calibration file)
      - All other datagrams should be in time order, excepting that all datagrams
        associated with a ping should be grouped together (MRZ, XMS, MWC)
      - Within a ping datagram group, the MRZ datagrams should be first, then
        the XMS datagram, then the MWC datagrams.
      - The MRZ datagrams in a ping should be grouped by receiver index
      - The MWC datagrams in a ping should be grouped by receiver index
      - If MRZ or MWC datagrams are partitioned (too large for a single UDP
        packet and so broken up into multiple packets), then these datagrams
        should be in the partition order.
      */

  /* deal with comment datagrams */
  if (aa->emdgm_type == XMC || bb->emdgm_type == XMC) {
    if (aa->emdgm_type == bb->emdgm_type) {
      if (aa->time_d < bb->time_d) {
        result = -1;
      }
      else if (aa->time_d > bb->time_d) {
        result = 1;
      }
      else {
        result = 0;
      }
    }
    else if (aa->emdgm_type == XMC) {
      result = -1;
    }
    else { // if (bb->emdgm_type == XMC)
      result = 1;
    }
  }

  /* deal with IIP datagram */
  else if (aa->emdgm_type == IIP) {
    result = -1;
  }
  else if (bb->emdgm_type == IIP) {
    result = 1;
  }

  /* deal with IOP datagram */
  else if (aa->emdgm_type == IOP) {
    result = -1;
  }
  else if (bb->emdgm_type == IOP) {
  result = 1;
  }

  /* deal with SVP datagram */
  else if (aa->emdgm_type == SVP) {
    result = -1;
  }
  else if (bb->emdgm_type == SVP) {
  result = 1;
  }

  /* deal with FCF datagram */
  else if (aa->emdgm_type == FCF) {
    result = -1;
  }
  else if (bb->emdgm_type == FCF) {
  result = 1;
  }

  /* deal with XMB datagram */
  else if (aa->emdgm_type == XMB) {
    result = -1;
  }
  else if (bb->emdgm_type == XMB) {
  result = 1;
  }

  /* deal with both datagrams being ping datagrams (MRZ, XMS, MWC) */
  else if ((aa->emdgm_type == MRZ || aa->emdgm_type == XMT || aa->emdgm_type == XMS || aa->emdgm_type == MWC)
      && (bb->emdgm_type == MRZ || bb->emdgm_type == XMT|| bb->emdgm_type == XMS || bb->emdgm_type == MWC)) {

    /* if ping numbers are different order by time stamp */
    if (aa->ping_num != bb->ping_num) {
      if (aa->time_d < bb->time_d) {
        result = -1;
      }
      else if (aa->time_d > bb->time_d) {
        result = 1;
      }
      else {
        result = 0;
      }
    }

    /* if ping numbers match */
    else { /* if (aa->ping_num == bb->ping_num) */

      /* if ping numbers match and datagram types are different order by MRZ < XMT < XMS < MWC */
      if ((aa->emdgm_type == MRZ && bb->emdgm_type == XMT)
          || (aa->emdgm_type == MRZ && bb->emdgm_type == XMS)
          || (aa->emdgm_type == MRZ && bb->emdgm_type == MWC)
          || (aa->emdgm_type == XMT && bb->emdgm_type == XMS)
          || (aa->emdgm_type == XMT && bb->emdgm_type == MWC)
          || (aa->emdgm_type == XMS && bb->emdgm_type == MWC)) {
        result = -1;
      }
      else if ((bb->emdgm_type == MRZ && aa->emdgm_type == XMT)
          || (bb->emdgm_type == MRZ && aa->emdgm_type == XMS)
          || (bb->emdgm_type == MRZ && aa->emdgm_type == MWC)
          || (bb->emdgm_type == XMT && aa->emdgm_type == XMS)
          || (bb->emdgm_type == XMT && aa->emdgm_type == MWC)
          || (bb->emdgm_type == XMS && aa->emdgm_type == MWC)) {
        result = 1;
      }

      /* if ping numbers match and datagram types match order by receiver index
          - set ping timestamp for later datagrams within the ping if possible */
      else { //if (aa->emdgm_type == bb->emdgm_type)
        if (aa->rx_index < bb->rx_index) {
          result = -1;
        }
        else if (aa->rx_index > bb->rx_index) {
          result = 1;
        }
        else { // if (aa->rx_index == bb->rx_index) ==> this shouldn't happen
          result = 0;
        }
      }
    }
  }

  /* deal with all other pairs of datagrams - order by timestamp */
  else if (aa->time_d < bb->time_d) {
    result = -1;
  }
  else if (aa->time_d > bb->time_d) {
    result = 1;
  }
  else {
    result = 0;
  }

  return(result);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_hdr(int verbose, char *buffer, void *header_ptr, void *emdgm_type_ptr, int *error) {
  struct mbsys_kmbes_header *header = NULL;
  mbsys_kmbes_emdgm_type *emdgm_type = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:        %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:         %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       header_ptr:     %p\n", (void *)header_ptr);
    fprintf(stderr, "dbg2       emdgm_type_ptr: %p\n", (void *)emdgm_type_ptr);
  }

  /* get pointer to header structure */
  header = (struct mbsys_kmbes_header *)header_ptr;
  emdgm_type = (mbsys_kmbes_emdgm_type *)emdgm_type_ptr;

  /* extract the data */
  index = 0;
  mb_get_binary_int(true, &buffer[index], &(header->numBytesDgm));
  index += 4;
  memcpy(&(header->dgmType), &buffer[index], sizeof(header->dgmType));
  index += 4;
  header->dgmVersion = buffer[index];
  index++;
  header->systemID = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(header->echoSounderID));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(header->time_sec));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(header->time_nanosec));

  /* identify the datagram type */
  if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_INSTALLATION_PARAM, 4) == 0 ) {
    *emdgm_type = IIP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_OP_RUNTIME, 4) == 0) {
    *emdgm_type = IOP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_BE_BIST, 4) == 0) {
    *emdgm_type = IBE;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_BR_BIST, 4) == 0) {
    *emdgm_type = IBR;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_I_BS_BIST, 4) == 0) {
    *emdgm_type = IBS;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_POSITION, 4) == 0) {
    *emdgm_type = SPO;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_KM_BINARY, 4) == 0) {
    *emdgm_type = SKM;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_SOUND_VELOCITY_PROFILE, 4) == 0) {
    *emdgm_type = SVP;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_SOUND_VELOCITY_TRANSDUCER, 4) == 0) {
    *emdgm_type = SVT;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_CLOCK, 4) == 0) {
    *emdgm_type = SCL;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_DEPTH, 4) == 0) {
    *emdgm_type = SDE;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_HEIGHT, 4) == 0) {
    *emdgm_type = SHI;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_S_HEADING, 4) == 0) {
    *emdgm_type = SHA;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_M_RANGE_AND_DEPTH, 4) == 0) {
    *emdgm_type = MRZ;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_M_WATER_COLUMN, 4) == 0) {
    *emdgm_type = MWC;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_C_POSITION, 4) == 0) {
    *emdgm_type = CPO;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_C_HEAVE, 4) == 0) {
    *emdgm_type = CHE;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_F_BSCALIBRATIONFILE, 4) == 0) {
    *emdgm_type = FCF;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_MBSYSTEM, 4) == 0) {
    *emdgm_type = XMB;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_COMMENT, 4) == 0) {
    *emdgm_type = XMC;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_EXTENSION, 4) == 0) {
    *emdgm_type = XMT;
  }
  else if (strncmp((const char *)header->dgmType, MBSYS_KMBES_X_PSEUDOSIDESCAN, 4) == 0) {
    *emdgm_type = XMS;
  }
  else {
    *emdgm_type = UNKNOWN;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", header->numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %.4s\n", header->dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", header->dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", header->systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", header->echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", header->time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", header->time_nanosec);
  }

  int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       dgmType:    %.4s\n", header->dgmType);
    fprintf(stderr, "dbg2       emdgm_type: %d\n", *emdgm_type);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_spo(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  size_t numBytesRawSensorData = 0;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_spo *spo = &(store->spo);

  /* copy the header */
  spo->header = *header;

  /* calc number of bytes for raw sensor data */
  numBytesRawSensorData = spo->header.numBytesDgm - MBSYS_KMBES_SPO_VAR_OFFSET;

  /* extract the data */
  index = MBSYS_KMBES_HEADER_SIZE;

  /* common part */
  mb_get_binary_short(true, &buffer[index], &(spo->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(spo->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(spo->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(spo->cmnPart.padding));
  index += 2;

  /* sensor data block */
  mb_get_binary_int(true, &buffer[index], &(spo->sensorData.timeFromSensor_sec));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(spo->sensorData.timeFromSensor_nanosec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(spo->sensorData.posFixQuality_m));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(spo->sensorData.correctedLat_deg));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(spo->sensorData.correctedLong_deg));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(spo->sensorData.speedOverGround_mPerSec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(spo->sensorData.courseOverGround_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(spo->sensorData.ellipsoidHeightReRefPoint_m));
  index += 4;
  memcpy(&(spo->sensorData.posDataFromSensor), &buffer[index], numBytesRawSensorData);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                 %u\n", spo->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                     %s\n", spo->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                  %u\n", spo->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                    %u\n", spo->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:               %u\n", spo->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                    %u\n", spo->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:                %u\n", spo->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:             %u\n", spo->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:                %u\n", spo->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:                %u\n", spo->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                     %u\n", spo->cmnPart.padding);

    fprintf(stderr, "dbg5       timeFromSensor_sec:          %u\n", spo->sensorData.timeFromSensor_sec);
    fprintf(stderr, "dbg5       timeFromSensor_nanosec:      %u\n", spo->sensorData.timeFromSensor_nanosec);
    fprintf(stderr, "dbg5       posFixQuality_m:             %f\n", spo->sensorData.posFixQuality_m);
    fprintf(stderr, "dbg5       correctedLat_deg:            %f\n", spo->sensorData.correctedLat_deg);
    fprintf(stderr, "dbg5       correctedLong_deg:           %f\n", spo->sensorData.correctedLong_deg);
    fprintf(stderr, "dbg5       speedOverGround_mPerSec:     %f\n", spo->sensorData.speedOverGround_mPerSec);
    fprintf(stderr, "dbg5       courseOverGround_deg:        %f\n", spo->sensorData.courseOverGround_deg);
    fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m: %f\n", spo->sensorData.ellipsoidHeightReRefPoint_m);
    fprintf(stderr, "dbg5       posDataFromSensor:           %s\n", spo->sensorData.posDataFromSensor);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType,header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/


int mbr_kemkmall_rd_skm(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_skm *skm = &(store->skm);

  /* copy the header */
  skm->header = *header;

  /* extract the data */
  index = MBSYS_KMBES_HEADER_SIZE;

  /* info part */
  mb_get_binary_short(true, &buffer[index], &(skm->infoPart.numBytesInfoPart));
  index += 2;
  skm->infoPart.sensorSystem = buffer[index];
  index++;
  skm->infoPart.sensorStatus = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(skm->infoPart.sensorInputFormat));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(skm->infoPart.numSamplesArray));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(skm->infoPart.numBytesPerSample));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(skm->infoPart.sensorDataContents));
  index += 2;

  for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {

    /* KMbinary */
    memcpy(&(skm->sample[i].KMdefault.dgmType), &buffer[index], 4);
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(skm->sample[i].KMdefault.numBytesDgm));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(skm->sample[i].KMdefault.dgmVersion));
    index += 2;
    mb_get_binary_int(true, &buffer[index], &(skm->sample[i].KMdefault.time_sec));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(skm->sample[i].KMdefault.time_nanosec));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(skm->sample[i].KMdefault.status));
    index += 4;
    mb_get_binary_double(true, &buffer[index], &(skm->sample[i].KMdefault.latitude_deg));
    index += 8;
    mb_get_binary_double(true, &buffer[index], &(skm->sample[i].KMdefault.longitude_deg));
    index += 8;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.ellipsoidHeight_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.roll_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.pitch_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.heading_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.heave_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.rollRate));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.pitchRate));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.yawRate));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.velNorth));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.velEast));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.velDown));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.latitudeError_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.longitudeError_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.ellipsoidHeightError_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.rollError_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.pitchError_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.headingError_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.heaveError_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.northAcceleration));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.eastAcceleration));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].KMdefault.downAcceleration));
    index += 4;

    /* KMdelayedHeave */
    mb_get_binary_int(true, &buffer[index], &(skm->sample[i].delayedHeave.time_sec));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(skm->sample[i].delayedHeave.time_nanosec));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(skm->sample[i].delayedHeave.delayedHeave_m));
    index += 4;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", skm->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", skm->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", skm->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", skm->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", skm->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", skm->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", skm->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesInfoPart:           %u\n", skm->infoPart.numBytesInfoPart);
    fprintf(stderr, "dbg5       sensorSystem:               %u\n", skm->infoPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:               %u\n", skm->infoPart.sensorStatus);
    fprintf(stderr, "dbg5       sensorInputFormat:          %u\n", skm->infoPart.sensorInputFormat);
    fprintf(stderr, "dbg5       numSamplesArray:            %u\n", skm->infoPart.numSamplesArray);
    fprintf(stderr, "dbg5       numBytesPerSample:          %u\n", skm->infoPart.numBytesPerSample);
    fprintf(stderr, "dbg5       sensorDataContents:         %u\n", skm->infoPart.sensorDataContents);

    for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.dgmType:                %s\n", i, skm->sample[i].KMdefault.dgmType);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.numBytesDgm:            %u\n", i, skm->sample[i].KMdefault.numBytesDgm);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.dgmVersion:             %u\n", i, skm->sample[i].KMdefault.dgmVersion);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.time_sec:               %u\n", i, skm->sample[i].KMdefault.time_sec);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.time_nanosec:           %u\n", i, skm->sample[i].KMdefault.time_nanosec);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.status:                 %u\n", i, skm->sample[i].KMdefault.status);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.latitude_deg:           %f\n", i, skm->sample[i].KMdefault.latitude_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.longitude_deg:          %f\n", i, skm->sample[i].KMdefault.longitude_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.ellipsoidHeight_m:      %f\n", i, skm->sample[i].KMdefault.ellipsoidHeight_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.roll_deg:               %f\n", i, skm->sample[i].KMdefault.roll_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitch_deg:              %f\n", i, skm->sample[i].KMdefault.pitch_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heading_deg:            %f\n", i, skm->sample[i].KMdefault.heading_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heave_m:                %f\n", i, skm->sample[i].KMdefault.heave_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.rollRate:               %f\n", i, skm->sample[i].KMdefault.rollRate);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitchRate:              %f\n", i, skm->sample[i].KMdefault.pitchRate);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.yawRate:                %f\n", i, skm->sample[i].KMdefault.yawRate);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velNorth:               %f\n", i, skm->sample[i].KMdefault.velNorth);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velEast:                %f\n", i, skm->sample[i].KMdefault.velEast);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velDown:                %f\n", i, skm->sample[i].KMdefault.velDown);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.latitudeError_m:        %f\n", i, skm->sample[i].KMdefault.latitudeError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.longitudeError_m:       %f\n", i, skm->sample[i].KMdefault.longitudeError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.ellipsoidHeightError_m: %f\n", i, skm->sample[i].KMdefault.ellipsoidHeightError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.rollError_deg:          %f\n", i, skm->sample[i].KMdefault.rollError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitchError_deg:         %f\n", i, skm->sample[i].KMdefault.pitchError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.headingError_deg:       %f\n", i, skm->sample[i].KMdefault.headingError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heaveError_m:           %f\n", i, skm->sample[i].KMdefault.heaveError_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.northAcceleration:      %f\n", i, skm->sample[i].KMdefault.northAcceleration);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.eastAcceleration:       %f\n", i, skm->sample[i].KMdefault.eastAcceleration);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.downAcceleration:       %f\n", i, skm->sample[i].KMdefault.downAcceleration);

      //
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.time_sec:            %u\n", i, skm->sample[i].delayedHeave.time_sec);
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.time_nanosec:        %u\n", i, skm->sample[i].delayedHeave.time_nanosec);
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.delayedHeave_m:      %f\n", i, skm->sample[i].delayedHeave.delayedHeave_m);
    }

  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV1;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_svp(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_svp *svp = &(store->svp);

  /* copy the header */
  svp->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  /* svp common part */
  mb_get_binary_short(true, &buffer[index], &(svp->numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svp->numSamples));
  index += 2;
  memcpy(&svp->sensorFormat, &buffer[index], 4);
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(svp->time_sec));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(svp->latitude_deg));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(svp->longitude_deg));
  index += 8;

  /* svp data block */
  for (int i = 0; i < svp->numSamples; i++ ) {
    mb_get_binary_float(true, &buffer[index], &(svp->sensorData[i].depth_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svp->sensorData[i].soundVelocity_mPerSec));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(svp->sensorData[i].padding));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svp->sensorData[i].temp_C));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svp->sensorData[i].salinity));
    index += 4;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:     %u\n", svp->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:         %.4s\n", svp->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:      %u\n", svp->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:        %u\n", svp->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:   %u\n", svp->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:        %u\n", svp->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:    %u\n", svp->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", svp->numBytesCmnPart);
    fprintf(stderr, "dbg5       numSamples:       %u\n", svp->numSamples);
    fprintf(stderr, "dbg5       sensorFormat:     %s\n", svp->sensorFormat);
    fprintf(stderr, "dbg5       time_sec:         %u\n", svp->time_sec);
    fprintf(stderr, "dbg5       latitude_deg:     %f\n", svp->latitude_deg);
    fprintf(stderr, "dbg5       longitude_deg:    %f\n", svp->longitude_deg);

    for (int i = 0; i < svp->numSamples; i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].depth_m:                %f\n", i, svp->sensorData[i].depth_m);
      fprintf(stderr, "dbg5       sensorData[%3d].soundVelocity_mPerSec:  %f\n", i, svp->sensorData[i].soundVelocity_mPerSec);
      fprintf(stderr, "dbg5       sensorData[%3d].padding:                %d\n", i, svp->sensorData[i].padding);
      fprintf(stderr, "dbg5       sensorData[%3d].temp_C:                 %f\n", i, svp->sensorData[i].temp_C);
      fprintf(stderr, "dbg5       sensorData[%3d].salinity:               %f\n", i, svp->sensorData[i].salinity);
    }
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_VELOCITY_PROFILE;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_svt(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_svt *svt = &(store->svt);

  /* copy the header */
  svt->header = *header;

  /* extract the data */
  index = MBSYS_KMBES_HEADER_SIZE;

  /* svp info */
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.numBytesInfoPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.sensorInputFormat));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.numSamplesArray));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.numBytesPerSample));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(svt->infoPart.sensorDataContents));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(svt->infoPart.filterTime_sec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(svt->infoPart.soundVelocity_mPerSec_offset));
  index += 4;

  /* svt data blocks */
  for (int i=0; i<(svt->infoPart.numSamplesArray); i++ ) {
    mb_get_binary_int(true, &buffer[index], &(svt->sensorData[i].time_sec));
    index += 4;
    mb_get_binary_int(true, &buffer[index], &(svt->sensorData[i].time_nanosec));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svt->sensorData[i].soundVelocity_mPerSec));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svt->sensorData[i].temp_C));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svt->sensorData[i].pressure_Pa));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(svt->sensorData[i].salinity));
    index += 4;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:             %u\n", svt->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                 %s\n", svt->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:              %u\n", svt->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                %u\n", svt->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:           %u\n", svt->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                %u\n", svt->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:            %u\n", svt->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesInfoPart:         %u\n", svt->infoPart.numBytesInfoPart);
    fprintf(stderr, "dbg5       sensorStatus:             %u\n", svt->infoPart.sensorStatus);
    fprintf(stderr, "dbg5       sensorInputFormat:        %u\n", svt->infoPart.sensorInputFormat);
    fprintf(stderr, "dbg5       numSamplesArray:          %u\n", svt->infoPart.numSamplesArray);
    fprintf(stderr, "dbg5       sensorDataContents:       %u\n", svt->infoPart.sensorDataContents);
    fprintf(stderr, "dbg5       filterTime_sec:           %f\n", svt->infoPart.filterTime_sec);
    fprintf(stderr, "dbg5       soundVelocity_mPerSec_offset: %f\n", svt->infoPart.soundVelocity_mPerSec_offset);

    for (int i = 0; i < (svt->infoPart.numSamplesArray); i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].time_sec:                     %u\n", i, svt->sensorData[i].time_sec);
      fprintf(stderr, "dbg5       sensorData[%3d].time_nanosec:                 %u\n", i, svt->sensorData[i].time_nanosec);
      fprintf(stderr, "dbg5       sensorData[%3d].soundVelocity_mPerSec:        %f\n", i, svt->sensorData[i].soundVelocity_mPerSec);
      fprintf(stderr, "dbg5       sensorData[%3d].temp_C:                       %f\n", i, svt->sensorData[i].temp_C);
      fprintf(stderr, "dbg5       sensorData[%3d].pressure_Pa:                  %f\n", i, svt->sensorData[i].pressure_Pa);
      fprintf(stderr, "dbg5       sensorData[%3d].salinity:                     %f\n", i, svt->sensorData[i].salinity);
    }
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SSV;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_scl(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  size_t numBytesRawSensorData = 0;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_scl *scl = &(store->scl);

  /* copy the header */
  scl->header = *header;

  /* calc number of bytes for raw sensor data */
  numBytesRawSensorData = scl->header.numBytesDgm - MBSYS_KMBES_SCL_VAR_OFFSET;

  /* extract the data */
  index = MBSYS_KMBES_HEADER_SIZE;

  // common part
  mb_get_binary_short(true, &buffer[index], &(scl->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(scl->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(scl->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(scl->cmnPart.padding));
  index += 2;

  // sensor data block
  mb_get_binary_float(true, &buffer[index], &(scl->sensorData.offset_sec));
  index += 4;
  mb_get_binary_int(true, &buffer[index], &(scl->sensorData.clockDevPU_nanosec));
  index += 4;
  memcpy(&(scl->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:         %u\n", scl->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:             %s\n", scl->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:          %u\n", scl->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:            %u\n", scl->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:       %u\n", scl->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:            %u\n", scl->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:        %u\n", scl->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:     %u\n", scl->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:        %u\n", scl->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:        %u\n", scl->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:             %u\n", scl->cmnPart.padding);

    fprintf(stderr, "dbg5       offset_sec:          %f\n", scl->sensorData.offset_sec);
    fprintf(stderr, "dbg5       clockDevPU_nanosec:  %d\n", scl->sensorData.clockDevPU_nanosec);
    fprintf(stderr, "dbg5       dataFromSensor:      %s\n", scl->sensorData.dataFromSensor);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_CLOCK;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_sde(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_sde *sde = &(store->sde);

  /* copy the header */
  sde->header = *header;

  /* calc number of bytes for raw sensor data */
  size_t numBytesRawSensorData = sde->header.numBytesDgm - MBSYS_KMBES_SDE_VAR_OFFSET;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  // common part
  mb_get_binary_short(true, &buffer[index], &(sde->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sde->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sde->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sde->cmnPart.padding));
  index += 2;

  // sensor data block
  mb_get_binary_float(true, &buffer[index], &(sde->sensorData.depthUsed_m));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sde->sensorData.offset));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sde->sensorData.scale));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sde->sensorData.latitude_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(sde->sensorData.longitude_deg));
  index += 4;
  memcpy(&(sde->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", sde->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", sde->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", sde->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", sde->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", sde->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", sde->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", sde->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", sde->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:     %u\n", sde->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:     %u\n", sde->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:          %u\n", sde->cmnPart.padding);

    fprintf(stderr, "dbg5       depthUsed_m:      %f\n", sde->sensorData.depthUsed_m);
    fprintf(stderr, "dbg5       offset:           %f\n", sde->sensorData.offset);
    fprintf(stderr, "dbg5       scale:            %f\n", sde->sensorData.scale);
    fprintf(stderr, "dbg5       latitude_deg:     %f\n", sde->sensorData.latitude_deg);
    fprintf(stderr, "dbg5       longitude_deg:    %f\n", sde->sensorData.longitude_deg);
    fprintf(stderr, "dbg5       dataFromSensor:   %s\n", sde->sensorData.dataFromSensor);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_SENSORDEPTH;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %c%c%c%c read - time: %d.%9.9d status:%d error:%d\n",
  header->dgmType[0], header->dgmType[1], header->dgmType[2], header->dgmType[3],
  header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_shi(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_shi *shi = &(store->shi);

  /* copy the header */
  shi->header = *header;

  /* calc number of bytes for raw sensor data */
  size_t numBytesRawSensorData = shi->header.numBytesDgm - MBSYS_KMBES_SHI_VAR_OFFSET;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  // common part
  mb_get_binary_short(true, &buffer[index], &(shi->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(shi->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(shi->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(shi->cmnPart.padding));
  index += 2;

  // sensor data block
  mb_get_binary_short(true, &buffer[index], &(shi->sensorData.sensorType));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(shi->sensorData.heigthUsed_m));
  index += 4;
  memcpy(&(shi->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", shi->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", shi->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", shi->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", shi->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", shi->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", shi->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", shi->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", shi->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:     %u\n", shi->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:     %u\n", shi->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:          %u\n", shi->cmnPart.padding);

    fprintf(stderr, "dbg5       sensorType:       %u\n", shi->sensorData.sensorType);
    fprintf(stderr, "dbg5       heigthUsed_m:     %f\n", shi->sensorData.heigthUsed_m);
    fprintf(stderr, "dbg5       dataFromSensor:   %s\n", shi->sensorData.dataFromSensor);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEIGHT;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_sha(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_sha *sha = &(store->sha);

  /* get header */
  sha->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  // common part
  mb_get_binary_short(true, &buffer[index], &(sha->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->cmnPart.padding));
  index += 2;

  // sensor info
  mb_get_binary_short(true, &buffer[index], &(sha->dataInfo.numBytesInfoPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->dataInfo.numSamplesArray));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->dataInfo.numBytesPerSample));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(sha->dataInfo.numBytesRawSensorData));
  index += 2;

  // sensor data blocks
  for (int i = 0; i < sha->dataInfo.numSamplesArray; i++) {
    mb_get_binary_int(true, &buffer[index], &(sha->sensorData[i].timeSinceRecStart_nanosec));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(sha->sensorData[i].headingCorrected_deg));
    index += 4;
    memcpy(&(sha->sensorData[i].dataFromSensor), &buffer[index], sha->dataInfo.numBytesRawSensorData);
    index += sha->dataInfo.numBytesRawSensorData;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:            %u\n", sha->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                %s\n", sha->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:             %u\n", sha->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:               %u\n", sha->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:          %u\n", sha->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:               %u\n", sha->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:           %u\n", sha->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:        %u\n", sha->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:           %u\n", sha->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:           %u\n", sha->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                %u\n", sha->cmnPart.padding);

    fprintf(stderr, "dbg5       numBytesInfoPart:       %u\n", sha->dataInfo.numBytesInfoPart);
    fprintf(stderr, "dbg5       numSamplesArray:        %u\n", sha->dataInfo.numSamplesArray);
    fprintf(stderr, "dbg5       numBytesPerSample:      %u\n", sha->dataInfo.numBytesPerSample);
    fprintf(stderr, "dbg5       numBytesRawSensorData:  %u\n", sha->dataInfo.numBytesRawSensorData);

    for (int i = 0; i < sha->dataInfo.numSamplesArray; i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].timeSinceRecStart_nanosec: %u\n", i, sha->sensorData[i].timeSinceRecStart_nanosec);
      fprintf(stderr, "dbg5       sensorData[%3d].headingCorrected_deg:      %f\n", i, sha->sensorData[i].headingCorrected_deg);
      fprintf(stderr, "dbg5       sensorData[%3d].dataFromSensor:            %s\n", i, sha->sensorData[i].dataFromSensor);
    }
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEADING;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_mrz(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *imrz, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *) buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *) store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  struct mbsys_kmbes_mrz *mrz = NULL;
  struct mbsys_kmbes_m_partition partition;
  struct mbsys_kmbes_m_body cmnPart;
  unsigned int index_EMdgmMbody = 0;
  unsigned int index_pingInfo = 0;
  unsigned int index_txSectorInfo = 0;
  unsigned int index_rxInfo = 0;
  unsigned int index_extraDetClassInfo = 0;
  unsigned int index_sounding, index_SIsample = 0;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", header->numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", header->dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", header->dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", header->systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", header->echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", header->time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", header->time_nanosec);
  }

  /* get the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  /* EMdgmMpartition - data partition information */
  mb_get_binary_short(true, &buffer[index], &(partition.numOfDgms));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(partition.dgmNum));
  index += 2;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numOfDgms = %d\n", partition.numOfDgms);
    fprintf(stderr, "dbg5       dgmNum    = %d\n", partition.dgmNum);
  }

  /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
  mb_get_binary_short(true, &buffer[index], &(cmnPart.numBytesCmnPart));
  index_EMdgmMbody = index;
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cmnPart.pingCnt));
  index += 2;
  cmnPart.rxFansPerPing = buffer[index];
  index++;
  cmnPart.rxFanIndex = buffer[index];
  index++;
  cmnPart.swathsPerPing = buffer[index];
  index++;
  cmnPart.swathAlongPosition = buffer[index];
  index++;
  cmnPart.txTransducerInd = buffer[index];
  index++;
  cmnPart.rxTransducerInd = buffer[index];
  index++;
  cmnPart.numRxTransducers = buffer[index];
  index++;
  cmnPart.algorithmType = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       pingCnt:             %d\n", cmnPart.pingCnt);
    fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", cmnPart.rxFansPerPing);
    fprintf(stderr, "dbg5       rxFanIndex:          %d\n", cmnPart.rxFanIndex);
    fprintf(stderr, "dbg5       swathsPerPing:       %d\n", cmnPart.swathsPerPing);
    fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", cmnPart.swathAlongPosition);
    fprintf(stderr, "dbg5       txTransducerInd:     %d\n", cmnPart.txTransducerInd);
    fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", cmnPart.rxTransducerInd);
    fprintf(stderr, "dbg5       numRxTransducers:    %d\n", cmnPart.numRxTransducers);
    fprintf(stderr, "dbg5       algorithmType:       %d\n", cmnPart.algorithmType);
  }

  /* now figure out which of the MRZ datagrams for this ping we are reading
    (cmnPart.rxFanIndex out of cmnPart.rxFansPerPing) */
  *imrz = cmnPart.rxFanIndex;
  mrz = &store->mrz[*imrz];
  mrz->header = *header;
  mrz->partition = partition;
  mrz->cmnPart = cmnPart;

  /* reset index to start of EMdgmMRZ_pingInfo using cmnPart.numBytesCmnPart
      - this avoids breaking the decoding if fields have been added to cmnPart */
  index_pingInfo = index_EMdgmMbody + cmnPart.numBytesCmnPart;
  index = index_pingInfo;

  /* EMdgmMRZ_pingInfo - ping info */
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.numBytesInfoData));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.padding0));
  index += 2;

  /* Ping info */
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.pingRate_Hz));
  index += 4;

  mrz->pingInfo.beamSpacing = buffer[index];
  index++;
  mrz->pingInfo.depthMode = buffer[index];
  index++;
  mrz->pingInfo.subDepthMode = buffer[index];
  index++;
  mrz->pingInfo.distanceBtwSwath = buffer[index];
  index++;
  mrz->pingInfo.detectionMode = buffer[index];
  index++;
  mrz->pingInfo.pulseForm = buffer[index];
  index++;

  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.padding1));
  index += 2;

  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.frequencyMode_Hz));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.freqRangeLowLim_Hz));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.freqRangeHighLim_Hz));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.maxTotalTxPulseLength_sec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.maxEffTxPulseLength_sec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.maxEffTxBandWidth_Hz));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.absCoeff_dBPerkm));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.portSectorEdge_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.starbSectorEdge_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.portMeanCov_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.starbMeanCov_deg));
  index += 4;

  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.portMeanCov_m));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.starbMeanCov_m));
  index += 2;

  mrz->pingInfo.modeAndStabilisation = buffer[index];
  index++;
  mrz->pingInfo.runtimeFilter1 = buffer[index];
  index++;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.runtimeFilter2));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(mrz->pingInfo.pipeTrackingStatus));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.transmitArraySizeUsed_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.receiveArraySizeUsed_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.transmitPower_dB));
  index += 4;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.SLrampUpTimeRemaining));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.padding2));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.yawAngle_deg));
  index += 4;

  /* Info of tx sector data block, EMdgmMRZ_txSectorInfo */
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.numTxSectors));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.numBytesPerTxSector));
  index += 2;

  /* Info at time of midpoint of first tx pulse */
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.headingVessel_deg));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.soundSpeedAtTxDepth_mPerSec));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.txTransducerDepth_m));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.z_waterLevelReRefPoint_m));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.x_kmallToall_m));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.y_kmallToall_m));
  index += 4;

  mrz->pingInfo.latLongInfo = buffer[index];
  index++;
  mrz->pingInfo.posSensorStatus = buffer[index];
  index++;
  mrz->pingInfo.attitudeSensorStatus = buffer[index];
  index++;
  mrz->pingInfo.padding2 = buffer[index];
  index++;

  mb_get_binary_double(true, &buffer[index], &(mrz->pingInfo.latitude_deg));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(mrz->pingInfo.longitude_deg));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.ellipsoidHeightReRefPoint_m));
  index += 4;

  if (mrz->header.dgmVersion >= 1) {
    mb_get_binary_float(true, &buffer[index], &(mrz->pingInfo.bsCorrectionOffset_dB));
    index += 4;
    mrz->pingInfo.lambertsLawApplied = buffer[index];
    index++;
    mrz->pingInfo.iceWindow = buffer[index];
    index++;
    mb_get_binary_short(true, &buffer[index], &(mrz->pingInfo.activeModes));
    index += 2;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesInfoData:             %d\n", mrz->pingInfo.numBytesInfoData);
    fprintf(stderr, "dbg5       padding0:                     %d\n", mrz->pingInfo.padding0);
    fprintf(stderr, "dbg5       pingRate_Hz:                  %f\n", mrz->pingInfo.pingRate_Hz);
    fprintf(stderr, "dbg5       beamSpacing:                  %d\n", mrz->pingInfo.beamSpacing);
    fprintf(stderr, "dbg5       depthMode:                    %d\n", mrz->pingInfo.depthMode);
    fprintf(stderr, "dbg5       subDepthMode:                 %d\n", mrz->pingInfo.subDepthMode);
    fprintf(stderr, "dbg5       distanceBtwSwath:             %d\n", mrz->pingInfo.distanceBtwSwath);
    fprintf(stderr, "dbg5       detectionMode:                %d\n", mrz->pingInfo.detectionMode);
    fprintf(stderr, "dbg5       pulseForm:                    %d\n", mrz->pingInfo.pulseForm);
    fprintf(stderr, "dbg5       padding1:                     %d\n", mrz->pingInfo.padding1);
    fprintf(stderr, "dbg5       frequencyMode_Hz:             %f\n", mrz->pingInfo.frequencyMode_Hz);
    fprintf(stderr, "dbg5       freqRangeLowLim_Hz:           %f\n", mrz->pingInfo.freqRangeLowLim_Hz);
    fprintf(stderr, "dbg5       freqRangeHighLim_Hz:          %f\n", mrz->pingInfo.freqRangeHighLim_Hz);
    fprintf(stderr, "dbg5       maxEffTxPulseLength_sec:      %f\n", mrz->pingInfo.maxEffTxPulseLength_sec);
    fprintf(stderr, "dbg5       maxTotalTxPulseLength_sec:    %f\n", mrz->pingInfo.maxTotalTxPulseLength_sec);
    fprintf(stderr, "dbg5       maxEffTxBandWidth_Hz:         %f\n", mrz->pingInfo.maxEffTxBandWidth_Hz);
    fprintf(stderr, "dbg5       absCoeff_dBPerkm:             %f\n", mrz->pingInfo.absCoeff_dBPerkm);
    fprintf(stderr, "dbg5       portSectorEdge_deg:           %f\n", mrz->pingInfo.portSectorEdge_deg);
    fprintf(stderr, "dbg5       starbSectorEdge_deg:          %f\n", mrz->pingInfo.starbSectorEdge_deg);
    fprintf(stderr, "dbg5       portMeanCov_m:                %d\n", mrz->pingInfo.portMeanCov_m);
    fprintf(stderr, "dbg5       starbMeanCov_m:               %d\n", mrz->pingInfo.starbMeanCov_m);
    fprintf(stderr, "dbg5       modeAndStabilisation:         %d\n", mrz->pingInfo.modeAndStabilisation);
    fprintf(stderr, "dbg5       runtimeFilter1:               %d\n", mrz->pingInfo.runtimeFilter1);
    fprintf(stderr, "dbg5       runtimeFilter2:               %d\n", mrz->pingInfo.runtimeFilter2);
    fprintf(stderr, "dbg5       pipeTrackingStatus:           %d\n", mrz->pingInfo.pipeTrackingStatus);
    fprintf(stderr, "dbg5       transmitArraySizeUsed_deg:    %f\n", mrz->pingInfo.transmitArraySizeUsed_deg);
    fprintf(stderr, "dbg5       receiveArraySizeUsed_deg:     %f\n", mrz->pingInfo.receiveArraySizeUsed_deg);
    fprintf(stderr, "dbg5       transmitPower_dB:             %f\n", mrz->pingInfo.transmitPower_dB);
    fprintf(stderr, "dbg5       SLrampUpTimeRemaining:        %d\n", mrz->pingInfo.SLrampUpTimeRemaining);
    fprintf(stderr, "dbg5       padding2:                     %d\n", mrz->pingInfo.padding2);
    fprintf(stderr, "dbg5       yawAngle_deg:                 %f\n", mrz->pingInfo.yawAngle_deg);
    fprintf(stderr, "dbg5       numTxSectors:                 %d\n", mrz->pingInfo.numTxSectors);
    fprintf(stderr, "dbg5       numBytesPerTxSector:          %d\n", mrz->pingInfo.numBytesPerTxSector);
    fprintf(stderr, "dbg5       headingVessel_deg:            %f\n", mrz->pingInfo.headingVessel_deg);
    fprintf(stderr, "dbg5       soundSpeedAtTxDepth_mPerSec:  %f\n", mrz->pingInfo.soundSpeedAtTxDepth_mPerSec);
    fprintf(stderr, "dbg5       txTransducerDepth_m:          %f\n", mrz->pingInfo.txTransducerDepth_m);
    fprintf(stderr, "dbg5       z_waterLevelReRefPoint_m:     %f\n", mrz->pingInfo.z_waterLevelReRefPoint_m);
    fprintf(stderr, "dbg5       x_kmallToall_m:               %f\n", mrz->pingInfo.x_kmallToall_m);
    fprintf(stderr, "dbg5       y_kmallToall_m:               %f\n", mrz->pingInfo.y_kmallToall_m);
    fprintf(stderr, "dbg5       latLongInfo:                  %d\n", mrz->pingInfo.latLongInfo);
    fprintf(stderr, "dbg5       posSensorStatus:              %d\n", mrz->pingInfo.posSensorStatus);
    fprintf(stderr, "dbg5       attitudeSensorStatus:         %d\n", mrz->pingInfo.attitudeSensorStatus);
    fprintf(stderr, "dbg5       padding3:                     %d\n", mrz->pingInfo.padding3);
    fprintf(stderr, "dbg5       latitude_deg:                 %f\n", mrz->pingInfo.latitude_deg);
    fprintf(stderr, "dbg5       longitude_deg:                %f\n", mrz->pingInfo.longitude_deg);
    fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m:  %f\n", mrz->pingInfo.ellipsoidHeightReRefPoint_m);
    fprintf(stderr, "dbg5       bsCorrectionOffset_dB:        %f\n", mrz->pingInfo.bsCorrectionOffset_dB);
    fprintf(stderr, "dbg5       lambertsLawApplied:           %u\n", mrz->pingInfo.lambertsLawApplied);
    fprintf(stderr, "dbg5       iceWindow:                    %u\n", mrz->pingInfo.iceWindow);
    fprintf(stderr, "dbg5       activeModes:                  %u\n", mrz->pingInfo.activeModes);
  }

  /* calculate index at start of txSectorInfo structures using pingInfo.numBytesInfoData
      - this avoids breaking the decoding if fields have been added to pingInfo */
  index_txSectorInfo = index_pingInfo + mrz->pingInfo.numBytesInfoData;

  /* EMdgmMRZ_txSectorInfo - sector information */
  for (int i = 0; i<(mrz->pingInfo.numTxSectors); i++)
  {
  /* calculate index at start of of each txSectorInfo using pingInfo.numBytesPerTxSector
      - this avoids breaking the decoding if fields have been added to txSectorInfo */
    index = index_txSectorInfo + i * mrz->pingInfo.numBytesPerTxSector;

    mrz->sectorInfo[i].txSectorNumb = buffer[index];
    index++;
    mrz->sectorInfo[i].txArrNumber = buffer[index];
    index++;
    mrz->sectorInfo[i].txSubArray = buffer[index];
    index++;
    mrz->sectorInfo[i].padding0 = buffer[index];
    index++;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].sectorTransmitDelay_sec));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].tiltAngleReTx_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].txNominalSourceLevel_dB));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].txFocusRange_m));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].centreFreq_Hz));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].signalBandWidth_Hz));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].totalSignalLength_sec));
    index += 4;
    mrz->sectorInfo[i].pulseShading = buffer[index];
    index++;
    mrz->sectorInfo[i].signalWaveForm = buffer[index];
    index++;
    mb_get_binary_short(true, &buffer[index], &(mrz->sectorInfo[i].padding1));
    index += 2;

    if (mrz->header.dgmVersion >= 1) {
      mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].highVoltageLevel_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].sectorTrackingCorr_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sectorInfo[i].effectiveSignalLength_sec));
      index += 4;
    }

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mrz->pingInfo.numTxSectors);
      fprintf(stderr, "dbg5       txSectorNumb:                %d\n", mrz->sectorInfo[i].txSectorNumb);
      fprintf(stderr, "dbg5       txArrNumber:                 %d\n", mrz->sectorInfo[i].txArrNumber);
      fprintf(stderr, "dbg5       txSubArray:                  %d\n", mrz->sectorInfo[i].txSubArray);
      fprintf(stderr, "dbg5       padding0:                    %d\n", mrz->sectorInfo[i].padding0);
      fprintf(stderr, "dbg5       sectorTransmitDelay_sec:     %f\n", mrz->sectorInfo[i].sectorTransmitDelay_sec);
      fprintf(stderr, "dbg5       tiltAngleReTx_deg:           %f\n", mrz->sectorInfo[i].tiltAngleReTx_deg);
      fprintf(stderr, "dbg5       txNominalSourceLevel_dB:     %f\n", mrz->sectorInfo[i].txNominalSourceLevel_dB);
      fprintf(stderr, "dbg5       txFocusRange_m:              %f\n", mrz->sectorInfo[i].txFocusRange_m);
      fprintf(stderr, "dbg5       centreFreq_Hz:               %f\n", mrz->sectorInfo[i].centreFreq_Hz);
      fprintf(stderr, "dbg5       signalBandWidth_Hz:          %f\n", mrz->sectorInfo[i].signalBandWidth_Hz);
      fprintf(stderr, "dbg5       totalSignalLength_sec:       %f\n", mrz->sectorInfo[i].totalSignalLength_sec);
      fprintf(stderr, "dbg5       pulseShading:                %d\n", mrz->sectorInfo[i].pulseShading);
      fprintf(stderr, "dbg5       signalWaveForm:              %d\n", mrz->sectorInfo[i].signalWaveForm);
      fprintf(stderr, "dbg5       padding1:                    %d\n", mrz->sectorInfo[i].padding1);
      fprintf(stderr, "dbg5       highVoltageLevel_dB:         %f\n", mrz->sectorInfo[i].highVoltageLevel_dB);
      fprintf(stderr, "dbg5       sectorTrackingCorr_dB:       %f\n", mrz->sectorInfo[i].sectorTrackingCorr_dB);
      fprintf(stderr, "dbg5       effectiveSignalLength_sec:   %f\n", mrz->sectorInfo[i].effectiveSignalLength_sec);
    }
  }

  /* calculate index at start of rxInfo using pingInfo.numTxSectors & mrz->pingInfo.numBytesPerTxSector
      - this avoids breaking the decoding if fields have been added to txSectorInfo */
  index_rxInfo = index_txSectorInfo + mrz->pingInfo.numTxSectors * mrz->pingInfo.numBytesPerTxSector;
  index = index_rxInfo;

  /* EMdgmMRZ_rxInfo - receiver specific info */
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numBytesRxInfo));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numSoundingsMaxMain));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numSoundingsValidMain));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numBytesPerSounding));
  index += 2;

  mb_get_binary_float(true, &buffer[index], &(mrz->rxInfo.WCSampleRate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->rxInfo.seabedImageSampleRate));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->rxInfo.BSnormal_dB));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mrz->rxInfo.BSoblique_dB));
  index += 4;

  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.extraDetectionAlarmFlag));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numExtraDetections));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numExtraDetectionClasses));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mrz->rxInfo.numBytesPerClass));
  index += 2;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesRxInfo:             %d\n", mrz->rxInfo.numBytesRxInfo);
    fprintf(stderr, "dbg5       numSoundingsMaxMain:        %d\n", mrz->rxInfo.numSoundingsMaxMain);
    fprintf(stderr, "dbg5       numSoundingsValidMain:      %d\n", mrz->rxInfo.numSoundingsValidMain);
    fprintf(stderr, "dbg5       numBytesPerSounding:        %d\n", mrz->rxInfo.numBytesPerSounding);
    fprintf(stderr, "dbg5       WCSampleRate:               %f\n", mrz->rxInfo.WCSampleRate);
    fprintf(stderr, "dbg5       seabedImageSampleRate:      %f\n", mrz->rxInfo.seabedImageSampleRate);
    fprintf(stderr, "dbg5       BSnormal_dB:                %f\n", mrz->rxInfo.BSnormal_dB);
    fprintf(stderr, "dbg5       BSoblique_dB:               %f\n", mrz->rxInfo.BSoblique_dB);
    fprintf(stderr, "dbg5       extraDetectionAlarmFlag:    %d\n", mrz->rxInfo.extraDetectionAlarmFlag);
    fprintf(stderr, "dbg5       numExtraDetections:         %d\n", mrz->rxInfo.numExtraDetections);
    fprintf(stderr, "dbg5       numExtraDetectionClasses:   %d\n", mrz->rxInfo.numExtraDetectionClasses);
    fprintf(stderr, "dbg5       numBytesPerClass:           %d\n", mrz->rxInfo.numBytesPerClass);
  }

  /* calculate index at start of rxInfo using rxInfo.numExtraDetectionClasses & rxInfo.numBytesPerClass
      - this avoids breaking the decoding if fields have been added to extraDetClassInfo */
  index_extraDetClassInfo = index_rxInfo + mrz->rxInfo.numBytesRxInfo;

  /* check against corrupted data */
  if (index_extraDetClassInfo + mrz->rxInfo.numExtraDetectionClasses * mrz->rxInfo.numBytesPerClass > header->numBytesDgm) {
    *error = MB_ERROR_BAD_DATA;
    status = MB_FAILURE;
    if (verbose > 0) {
      fprintf(stderr, "\nCorrupted MRZ datagram dropped...\n");
    }
  }

  if (status == MB_SUCCESS)
  {
    /* EMdgmMRZ_extraDetClassInfo -  Extra detection class info */
    for (int i = 0; i<(mrz->rxInfo.numExtraDetectionClasses); i++)
    {
    /* calculate index at start of of each extraDetClassInfo using pingInfo.numBytesPerTxSector
        - this avoids breaking the decoding if fields have been added to txSectorInfo */
      index = index_extraDetClassInfo + i * mrz->rxInfo.numBytesPerClass;

      mb_get_binary_short(true, &buffer[index], &(mrz->extraDetClassInfo[i].numExtraDetInClass));
      index += 2;
      mrz->extraDetClassInfo[i].padding = buffer[index];
      index++;
      mrz->extraDetClassInfo[i].alarmFlag = buffer[index];
      index++;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       numExtraDetInClass:  %d\n", mrz->extraDetClassInfo[i].numExtraDetInClass);
        fprintf(stderr, "dbg5       padding:             %d\n", mrz->extraDetClassInfo[i].padding);
        fprintf(stderr, "dbg5       alarmFlag:           %d\n", mrz->extraDetClassInfo[i].alarmFlag);
      }
    }

    /* calculate index at start of sounding using rxInfo.numExtraDetectionClasses & rxInfo.numBytesPerClass
        - this avoids breaking the decoding if fields have been added to extraDetClassInfo */
    index_sounding = index_extraDetClassInfo + mrz->rxInfo.numExtraDetectionClasses * mrz->rxInfo.numBytesPerClass;

    /* EMdgmMRZ_sounding - Data for each sounding */
    int numSidescanSamples = 0;
    int numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
    for (int i = 0; i<numSoundings; i++)
    {
    /* calculate index at start of of each sounding using pingInfo.numBytesPerTxSector
        - this avoids breaking the decoding if fields have been added to sounding */
      index = index_sounding + i * mrz->rxInfo.numBytesPerSounding;

      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].soundingIndex));
      index += 2;
      mrz->sounding[i].txSectorNumb = buffer[index];
      index++;

      /* Detection info. */
      mrz->sounding[i].detectionType = buffer[index];
      index++;
      mrz->sounding[i].detectionMethod = buffer[index];
      index++;
      mrz->sounding[i].rejectionInfo1 = buffer[index];
      index++;
      mrz->sounding[i].rejectionInfo2 = buffer[index];
      index++;
      mrz->sounding[i].postProcessingInfo = buffer[index];
      index++;
      mrz->sounding[i].detectionClass = buffer[index];
      index++;
      mrz->sounding[i].detectionConfidenceLevel = buffer[index];
      index++;
      /* These two bytes specified as padding in the Kongsberg specification but are
         here used for the MB-System beam flag - if the first mb_u_char == 1 then the
         second byte is an MB-System beamflag */
      // mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].padding));
      // index += 2;
      mrz->sounding[i].beamflag_enabled = buffer[index];
      index++;
      mrz->sounding[i].beamflag = buffer[index];
      index++;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].rangeFactor));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].qualityFactor));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].detectionUncertaintyVer_m));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].detectionUncertaintyHor_m));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].detectionWindowLength_sec));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].echoLength_sec));
      index += 4;

      /* Water column paramters. */
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].WCBeamNumb));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].WCrange_samples));
      index += 2;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].WCNomBeamAngleAcross_deg));
      index += 4;

      /* Reflectivity data (backscatter (BS) data). */
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].meanAbsCoeff_dBPerkm));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].reflectivity1_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].reflectivity2_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].receiverSensitivityApplied_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].sourceLevelApplied_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].BScalibration_dB));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].TVG_dB));
      index += 4;

      /* Range and angle data. */
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].beamAngleReRx_deg));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].beamAngleCorrection_deg));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].twoWayTravelTime_sec));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].twoWayTravelTimeCorrection_sec));
      index += 4;

      /* Georeferenced depth points. */
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].deltaLatitude_deg));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].deltaLongitude_deg));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].z_reRefPoint_m));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].y_reRefPoint_m));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].x_reRefPoint_m));
      index += 4;
      mb_get_binary_float(true, &buffer[index], &(mrz->sounding[i].beamIncAngleAdj_deg));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].realTimeCleanInfo));
      index += 2;

      /* Seabed image. */
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].SIstartRange_samples));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].SIcentreSample));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mrz->sounding[i].SInumSamples));
      index += 2;

      numSidescanSamples += mrz->sounding[i].SInumSamples;

      /* calculate beamflag */
      if (mrz->sounding[i].beamflag_enabled != 1) {
        if (mrz->sounding[i].detectionType >= 2) {
          mrz->sounding[i].beamflag = MB_FLAG_NULL;
        }
        else if (mrz->sounding[i].detectionType == 1) {
          mrz->sounding[i].beamflag = (char)(MB_FLAG_FLAG + MB_FLAG_SONAR);
        }
        else if (mrz->sounding[i].qualityFactor > MBSYS_KMBES_QUAL_FACTOR_THRESHOLD) {
            mrz->sounding[i].beamflag = (char)(MB_FLAG_FLAG + MB_FLAG_SONAR);
        }
        else {
            mrz->sounding[i].beamflag = MB_FLAG_NONE;
        }
        mrz->sounding[i].beamflag_enabled = 1;
      }

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       soundingIndex:                   %d\n", mrz->sounding[i].soundingIndex);
        fprintf(stderr, "dbg5       txSectorNumb:                    %d\n", mrz->sounding[i].txSectorNumb);
        fprintf(stderr, "dbg5       detectionType:                   %d\n", mrz->sounding[i].detectionType);
        fprintf(stderr, "dbg5       detectionMethod:                 %d\n", mrz->sounding[i].detectionMethod);
        fprintf(stderr, "dbg5       rejectionInfo1:                  %d\n", mrz->sounding[i].rejectionInfo1);
        fprintf(stderr, "dbg5       rejectionInfo2:                  %d\n", mrz->sounding[i].rejectionInfo2);
        fprintf(stderr, "dbg5       postProcessingInfo:              %d\n", mrz->sounding[i].postProcessingInfo);
        fprintf(stderr, "dbg5       detectionClass:                  %d\n", mrz->sounding[i].detectionClass);
        fprintf(stderr, "dbg5       detectionConfidenceLevel:        %d\n", mrz->sounding[i].detectionConfidenceLevel);
        // fprintf(stderr, "dbg5       padding:                        %d\n", mrz->sounding[i].padding);
        fprintf(stderr, "dbg5       beamflag_enabled:                %d\n", mrz->sounding[i].beamflag_enabled);
        fprintf(stderr, "dbg5       beamflag:                        %d\n", mrz->sounding[i].beamflag);
        fprintf(stderr, "dbg5       rangeFactor:                     %f\n", mrz->sounding[i].rangeFactor);
        fprintf(stderr, "dbg5       qualityFactor:                   %f\n", mrz->sounding[i].qualityFactor);
        fprintf(stderr, "dbg5       detectionUncertaintyVer_m:       %f\n", mrz->sounding[i].detectionUncertaintyVer_m);
        fprintf(stderr, "dbg5       detectionUncertaintyHor_m:       %f\n", mrz->sounding[i].detectionUncertaintyHor_m);
        fprintf(stderr, "dbg5       detectionWindowLength_sec:       %f\n", mrz->sounding[i].detectionWindowLength_sec);
        fprintf(stderr, "dbg5       echoLength_sec:                  %f\n", mrz->sounding[i].echoLength_sec);
        fprintf(stderr, "dbg5       WCBeamNumb:                      %d\n", mrz->sounding[i].WCBeamNumb);
        fprintf(stderr, "dbg5       WCrange_samples:                 %d\n", mrz->sounding[i].WCrange_samples);
        fprintf(stderr, "dbg5       WCNomBeamAngleAcross_deg:        %f\n", mrz->sounding[i].WCNomBeamAngleAcross_deg);
        fprintf(stderr, "dbg5       meanAbsCoeff_dBPerkm:            %f\n", mrz->sounding[i].meanAbsCoeff_dBPerkm);
        fprintf(stderr, "dbg5       reflectivity1_dB:                %f\n", mrz->sounding[i].reflectivity1_dB);
        fprintf(stderr, "dbg5       reflectivity2_dB:                %f\n", mrz->sounding[i].reflectivity2_dB);
        fprintf(stderr, "dbg5       receiverSensitivityApplied_dB:   %f\n", mrz->sounding[i].receiverSensitivityApplied_dB);
        fprintf(stderr, "dbg5       sourceLevelApplied_dB:           %f\n", mrz->sounding[i].sourceLevelApplied_dB);
        fprintf(stderr, "dbg5       BScalibration_dB:                %f\n", mrz->sounding[i].BScalibration_dB);
        fprintf(stderr, "dbg5       TVG_dB:                          %f\n", mrz->sounding[i].TVG_dB);
        fprintf(stderr, "dbg5       beamAngleReRx_deg:               %f\n", mrz->sounding[i].beamAngleReRx_deg);
        fprintf(stderr, "dbg5       beamAngleCorrection_deg:         %f\n", mrz->sounding[i].beamAngleCorrection_deg);
        fprintf(stderr, "dbg5       twoWayTravelTime_sec:            %f\n", mrz->sounding[i].twoWayTravelTime_sec);
        fprintf(stderr, "dbg5       twoWayTravelTimeCorrection_sec:  %f\n", mrz->sounding[i].twoWayTravelTimeCorrection_sec);
        fprintf(stderr, "dbg5       deltaLatitude_deg:               %f\n", mrz->sounding[i].deltaLatitude_deg);
        fprintf(stderr, "dbg5       deltaLongitude_deg:              %f\n", mrz->sounding[i].deltaLongitude_deg);
        fprintf(stderr, "dbg5       z_reRefPoint_m:                  %f\n", mrz->sounding[i].z_reRefPoint_m);
        fprintf(stderr, "dbg5       y_reRefPoint_m:                  %f\n", mrz->sounding[i].y_reRefPoint_m);
        fprintf(stderr, "dbg5       x_reRefPoint_m:                  %f\n", mrz->sounding[i].x_reRefPoint_m);
        fprintf(stderr, "dbg5       beamIncAngleAdj_deg:             %f\n", mrz->sounding[i].beamIncAngleAdj_deg);
        fprintf(stderr, "dbg5       realTimeCleanInfo:               %d\n", mrz->sounding[i].realTimeCleanInfo);
        fprintf(stderr, "dbg5       SIstartRange_samples:            %d\n", mrz->sounding[i].SIstartRange_samples);
        fprintf(stderr, "dbg5       SIcentreSample:                  %d\n", mrz->sounding[i].SIcentreSample);
        fprintf(stderr, "dbg5       SInumSamples:                    %d\n", mrz->sounding[i].SInumSamples);
      }
    }

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numSidescanSamples:  %d\n", numSidescanSamples);
    }

    /* calculate index at start of SIsample_desidB using numSoundings and mrz->rxInfo.numBytesPerSounding
        - this avoids breaking the decoding if fields have been added to sounding */
    index_SIsample = index_sounding + numSoundings * mrz->rxInfo.numBytesPerSounding;
    index = index_SIsample;

    for (int i = 0; i < numSidescanSamples; i++)
    {
      mb_get_binary_short(true, &buffer[index], &(mrz->SIsample_desidB[i]));
      index += 2;
    }
  }

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       imrz:       %d\n", *imrz);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - index:%d time: %d.%9.9d ping:%d rx:%d/%d xms expected:%d status:%d error:%d\n",
          header->dgmType, index, header->time_sec, header->time_nanosec, cmnPart.pingCnt, cmnPart.rxFanIndex, cmnPart.rxFansPerPing,
          store->xmb.mbsystem_extensions, status, *error);
#endif

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_mwc(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *imwc, int *error) {
  struct mbsys_kmbes_mwc *mwc = NULL;
  struct mbsys_kmbes_m_partition partition;
  struct mbsys_kmbes_m_body cmnPart;
  size_t alloc_size = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;

  /* get the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  /* EMdgmMpartition - data partition information */
  mb_get_binary_short(true, &buffer[index], &(partition.numOfDgms));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(partition.dgmNum));
  index += 2;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numOfDgms:  %d\n", partition.numOfDgms);
    fprintf(stderr, "dbg5       dgmNum:     %d\n", partition.dgmNum);
  }

  /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
  mb_get_binary_short(true, &buffer[index], &(cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cmnPart.pingCnt));
  index += 2;
  cmnPart.rxFansPerPing = buffer[index];
  index++;
  cmnPart.rxFanIndex = buffer[index];
  index++;
  cmnPart.swathsPerPing = buffer[index];
  index++;
  cmnPart.swathAlongPosition = buffer[index];
  index++;
  cmnPart.txTransducerInd = buffer[index];
  index++;
  cmnPart.rxTransducerInd = buffer[index];
  index++;
  cmnPart.numRxTransducers = buffer[index];
  index++;
  cmnPart.algorithmType = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       pingCnt:             %d\n", cmnPart.pingCnt);
    fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", cmnPart.rxFansPerPing);
    fprintf(stderr, "dbg5       rxFanIndex:          %d\n", cmnPart.rxFanIndex);
    fprintf(stderr, "dbg5       swathsPerPing:       %d\n", cmnPart.swathsPerPing);
    fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", cmnPart.swathAlongPosition);
    fprintf(stderr, "dbg5       txTransducerInd:     %d\n", cmnPart.txTransducerInd);
    fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", cmnPart.rxTransducerInd);
    fprintf(stderr, "dbg5       numRxTransducers:    %d\n", cmnPart.numRxTransducers);
    fprintf(stderr, "dbg5       algorithmType:       %d\n", cmnPart.algorithmType);
  }

  /* now figure out which of the MWC datagrams for this ping we are reading
    (cmnPart.rxFanIndex out of cmnPart.rxFansPerPing) */
  *imwc = cmnPart.rxFanIndex;
  mwc = &store->mwc[cmnPart.rxFanIndex];
  mwc->header = *header;
  mwc->partition = partition;
  mwc->cmnPart = cmnPart;

  /* EMdgmMWCtxInfo - transmit sectors, general info for all sectors */
  mb_get_binary_short(true, &buffer[index], &(mwc->txInfo.numBytesTxInfo));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mwc->txInfo.numTxSectors));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mwc->txInfo.numBytesPerTxSector));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mwc->txInfo.padding));
  index += 2;
  mb_get_binary_float(true, &buffer[index], &(mwc->txInfo.heave_m));
  index += 4;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesTxInfo:       %d\n", mwc->txInfo.numBytesTxInfo);
    fprintf(stderr, "dbg5       numTxSectors:         %d\n", mwc->txInfo.numTxSectors);
    fprintf(stderr, "dbg5       numBytesPerTxSector:  %d\n", mwc->txInfo.numBytesPerTxSector);
    fprintf(stderr, "dbg5       padding:              %d\n", mwc->txInfo.padding);
    fprintf(stderr, "dbg5       heave_m:              %f\n", mwc->txInfo.heave_m);
  }

  /* EMdgmMWCtxSectorData - transmit sector data, loop for all i = numTxSectors */
  for (int i=0; i<(mwc->txInfo.numTxSectors); i++) {
    mb_get_binary_float(true, &buffer[index], &(mwc->sectorData[i].tiltAngleReTx_deg));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mwc->sectorData[i].centreFreq_Hz));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(mwc->sectorData[i].txBeamWidthAlong_deg));
    index += 4;
    mb_get_binary_short(true, &buffer[index], &(mwc->sectorData[i].txSectorNum));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(mwc->sectorData[i].padding));
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mwc->txInfo.numTxSectors);
      fprintf(stderr, "dbg5       tiltAngleReTx_deg:     %f\n", mwc->sectorData[i].tiltAngleReTx_deg);
      fprintf(stderr, "dbg5       centreFreq_Hz:         %f\n", mwc->sectorData[i].centreFreq_Hz);
      fprintf(stderr, "dbg5       txBeamWidthAlong_deg:  %f\n", mwc->sectorData[i].txBeamWidthAlong_deg);
      fprintf(stderr, "dbg5       txSectorNum:           %d\n", mwc->sectorData[i].txSectorNum);
      fprintf(stderr, "dbg5       padding:               %d\n", mwc->sectorData[i].padding);
    }
  }

  /* EMdgmMWCrxInfo - receiver, general info */
  mb_get_binary_short(true, &buffer[index], &(mwc->rxInfo.numBytesRxInfo));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(mwc->rxInfo.numBeams));
  index += 2;
  mwc->rxInfo.numBytesPerBeamEntry = buffer[index];
  index ++;
  mwc->rxInfo.phaseFlag = buffer[index];
  index ++;
  mwc->rxInfo.TVGfunctionApplied = buffer[index];
  index ++;
  mwc->rxInfo.TVGoffset_dB = buffer[index];
  index ++;
  mb_get_binary_float(true, &buffer[index], &(mwc->rxInfo.sampleFreq_Hz));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(mwc->rxInfo.soundVelocity_mPerSec));
  index += 4;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesRxInfo:         %d\n", mwc->rxInfo.numBytesRxInfo);
    fprintf(stderr, "dbg5       numBeams:               %d\n", mwc->rxInfo.numBeams);
    fprintf(stderr, "dbg5       numBytesPerBeamEntry:   %d\n", mwc->rxInfo.numBytesPerBeamEntry);
    fprintf(stderr, "dbg5       phaseFlag:              %d\n", mwc->rxInfo.phaseFlag);
    fprintf(stderr, "dbg5       TVGfunctionApplied:     %d\n", mwc->rxInfo.TVGfunctionApplied);
    fprintf(stderr, "dbg5       TVGoffset_dB:           %d\n", mwc->rxInfo.TVGoffset_dB);
    fprintf(stderr, "dbg5       sampleFreq_Hz:          %f\n", mwc->rxInfo.sampleFreq_Hz);
    fprintf(stderr, "dbg5       soundVelocity_mPerSec:  %f\n", mwc->rxInfo.soundVelocity_mPerSec);
  }

  int status = MB_SUCCESS;

  /* EMdgmMWCrxBeamData - receiver, specific info for each beam */
  alloc_size = (size_t)(mwc->rxInfo.numBeams * sizeof(struct mbsys_kmbes_mwc_rx_beam_data));
  if (mwc->beamData_p_alloc_size < alloc_size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size, (void **)&(mwc->beamData_p), error);
    if (status == MB_SUCCESS) {
      mwc->beamData_p_alloc_size = alloc_size;
      memset(mwc->beamData_p, 0, alloc_size);
    } else {
      mwc->beamData_p_alloc_size = 0;
    }
  }

  if (status == MB_SUCCESS) {
    for (int i=0; i<(mwc->rxInfo.numBeams) && status == MB_SUCCESS; i++) {
      mb_get_binary_float(true, &buffer[index], &(mwc->beamData_p[i].beamPointAngReVertical_deg));
      index += 4;
      mb_get_binary_short(true, &buffer[index], &(mwc->beamData_p[i].startRangeSampleNum));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mwc->beamData_p[i].detectedRangeInSamples));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mwc->beamData_p[i].beamTxSectorNum));
      index += 2;
      mb_get_binary_short(true, &buffer[index], &(mwc->beamData_p[i].numSampleData));
      index += 2;
      if (mwc->header.dgmVersion >= 1) {
        mb_get_binary_float(true, &buffer[index], &(mwc->beamData_p[i].detectedRangeInSamplesHighResolution));
        index += 4;
      } else {
        mwc->beamData_p[i].detectedRangeInSamplesHighResolution = mwc->beamData_p[i].detectedRangeInSamples;
      }

      /* Allocate sample amplitude array. Sample amplitudes are in 0.5 dB resolution */
      alloc_size = (size_t)(mwc->beamData_p[i].numSampleData);
      if (mwc->beamData_p[i].sampleAmplitude05dB_p_alloc_size < alloc_size) {
        alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
        status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                            (void **)&(mwc->beamData_p[i].sampleAmplitude05dB_p), error);
        if (status == MB_SUCCESS) {
          mwc->beamData_p[i].sampleAmplitude05dB_p_alloc_size = alloc_size;
        } else {
          mwc->beamData_p[i].sampleAmplitude05dB_p_alloc_size = 0;
        }
      }

      /* Now get the amplitude samples */
      if (status == MB_SUCCESS) {
        memcpy(mwc->beamData_p[i].sampleAmplitude05dB_p, &buffer[index], mwc->beamData_p[i].numSampleData);
        index += mwc->beamData_p[i].numSampleData;

        /* Allocate for and get phase data, if available, according to mwc->rxInfo.phaseFlag */
        switch (mwc->rxInfo.phaseFlag) {
          /* no phase data included */
          case 0:
            break;

          /* 8-bit phase data */
          case 1:
            /* Rx beam phase in 180/128 degree resolution. */
            alloc_size = (size_t)(mwc->beamData_p[i].numSampleData);
            if (mwc->beamData_p[i].samplePhase8bit_alloc_size < alloc_size) {
              alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
              status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                                  (void **)&(mwc->beamData_p[i].samplePhase8bit), error);
              if (status == MB_SUCCESS)
                mwc->beamData_p[i].samplePhase8bit_alloc_size = alloc_size;
              else
                mwc->beamData_p[i].samplePhase8bit_alloc_size = 0;
            }
            if (status == MB_SUCCESS) {
              memcpy(mwc->beamData_p[i].samplePhase8bit, &buffer[index], mwc->beamData_p[i].numSampleData);
              index += mwc->beamData_p[i].numSampleData;
            }
            break;

          /* 16 bit phase data */
          case 2:
            /* Rx beam phase in 0.01 degree resolution */
            alloc_size = (size_t)(2 * mwc->beamData_p[i].numSampleData);
            if (mwc->beamData_p[i].samplePhase16bit_alloc_size < alloc_size) {
              alloc_size = (1 + (int)(alloc_size / 1024)) * 1024;
              status = mb_reallocd(verbose, __FILE__, __LINE__, alloc_size,
                                  (void **)&(mwc->beamData_p[i].samplePhase16bit), error);
              if (status == MB_SUCCESS)
                mwc->beamData_p[i].samplePhase16bit_alloc_size = alloc_size;
              else
                mwc->beamData_p[i].samplePhase16bit_alloc_size = 0;
            }
            if (status == MB_SUCCESS) {
              for (int k = 0; k<mwc->beamData_p[i].numSampleData; k++) {
                mb_get_binary_short(true, &buffer[index], &(mwc->beamData_p[i].samplePhase16bit[k]));
                index += 2;
              }
            }
            break;
        }

        if (status == MB_SUCCESS && verbose >= 5) {
          fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
          fprintf(stderr, "dbg5       #MWC receiver beam data          %d/%d:\n", i, mwc->rxInfo.numBeams);
          fprintf(stderr, "dbg5       beamPointAngReVertical_deg:            %f\n", mwc->beamData_p[i].beamPointAngReVertical_deg);
          fprintf(stderr, "dbg5       startRangeSampleNum:                   %d\n", mwc->beamData_p[i].startRangeSampleNum);
          fprintf(stderr, "dbg5       detectedRangeInSamples:                %d\n", mwc->beamData_p[i].detectedRangeInSamples);
          fprintf(stderr, "dbg5       beamTxSectorNum:                       %d\n", mwc->beamData_p[i].beamTxSectorNum);
          fprintf(stderr, "dbg5       numSampleData:                         %d\n", mwc->beamData_p[i].numSampleData);
          fprintf(stderr, "dbg5       detectedRangeInSamplesHighResolution:  %f\n", mwc->beamData_p[i].detectedRangeInSamplesHighResolution);
          fprintf(stderr, "dbg5       (amplitude phase):       [\n");
          for (int k = 0; k < (mwc->beamData_p[i].numSampleData); k++) {
            if (k % 10 == 0)
              fprintf(stderr, "dbg5             ");
            if (mwc->rxInfo.phaseFlag == 1)
              fprintf(stderr, " (%d %d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k], mwc->beamData_p[i].samplePhase8bit[k]);
            else if (mwc->rxInfo.phaseFlag == 2)
              fprintf(stderr, " (%d %d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k], mwc->beamData_p[i].samplePhase16bit[k]);
            else
              fprintf(stderr, " (%d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k]);
            if ((k+1) % 10 == 0)
              fprintf(stderr, "\n");
          }
        }
      }
    }
  }

  /* reset datagram version if necessary */
  if (mwc->header.dgmVersion == 0)
    mwc->header.dgmVersion = 1;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind - Not MB_DATA_WATER_COLUMN because for this format water column
       is returned as part of the survey ping */
    store->kind = MB_DATA_DATA;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       imwc:       %d\n", *imwc);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);

}

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_cpo(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_cpo *cpo = &(store->cpo);

  /* copy the header */
  cpo->header = *header;

  /* calc number of bytes for raw sensor data */
  size_t numBytesRawSensorData = cpo->header.numBytesDgm - MBSYS_KMBES_CPO_VAR_OFFSET;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  // common part
  mb_get_binary_short(true, &buffer[index], &(cpo->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cpo->cmnPart.sensorSystem));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cpo->cmnPart.sensorStatus));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cpo->cmnPart.padding));
  index += 2;

  // sensor data block
  mb_get_binary_int(true, &buffer[index], &cpo->sensorData.timeFromSensor_sec);
  index += 4;
  mb_get_binary_int(true, &buffer[index], &cpo->sensorData.timeFromSensor_nanosec);
  index += 4;
  mb_get_binary_float(true, &buffer[index], &cpo->sensorData.posFixQuality_m);
  index += 4;
  mb_get_binary_double(true, &buffer[index], &cpo->sensorData.correctedLat_deg);
  index += 8;
  mb_get_binary_double(true, &buffer[index], &cpo->sensorData.correctedLong_deg);
  index += 8;
  mb_get_binary_float(true, &buffer[index], &cpo->sensorData.speedOverGround_mPerSec);
  index += 4;
  mb_get_binary_float(true, &buffer[index], &cpo->sensorData.courseOverGround_deg);
  index += 4;
  mb_get_binary_float(true, &buffer[index], &cpo->sensorData.ellipsoidHeightReRefPoint_m);
  index += 4;
  memcpy(&cpo->sensorData.posDataFromSensor, &buffer[index], numBytesRawSensorData);
  index += numBytesRawSensorData;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                  %u\n", cpo->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                      %s\n", cpo->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                   %u\n", cpo->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                     %u\n", cpo->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:                %u\n", cpo->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                     %u\n", cpo->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:                 %u\n", cpo->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:              %u\n", cpo->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:                 %u\n", cpo->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:                 %u\n", cpo->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                      %u\n", cpo->cmnPart.padding);

    fprintf(stderr, "dbg5       timeFromSensor_sec:           %u\n", cpo->sensorData.timeFromSensor_sec);
    fprintf(stderr, "dbg5       timeFromSensor_nanosec:       %u\n", cpo->sensorData.timeFromSensor_nanosec);
    fprintf(stderr, "dbg5       posFixQuality_m:              %f\n", cpo->sensorData.posFixQuality_m);
    fprintf(stderr, "dbg5       correctedLat_deg:             %f\n", cpo->sensorData.correctedLat_deg);
    fprintf(stderr, "dbg5       correctedLong_deg:            %f\n", cpo->sensorData.correctedLong_deg);
    fprintf(stderr, "dbg5       speedOverGround_mPerSec:      %f\n", cpo->sensorData.speedOverGround_mPerSec);
    fprintf(stderr, "dbg5       courseOverGround_deg:         %f\n", cpo->sensorData.courseOverGround_deg);
    fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m:  %f\n", cpo->sensorData.ellipsoidHeightReRefPoint_m);
    fprintf(stderr, "dbg5       posDataFromSensor:            %s\n", cpo->sensorData.posDataFromSensor);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_NAV2;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_che(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_che *che = &(store->che);

  /* copy the header */
  che->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", che->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", che->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", che->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", che->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", che->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", che->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", che->header.time_nanosec);
  }

  /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
  mb_get_binary_short(true, &buffer[index], &(che->cmnPart.numBytesCmnPart));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(che->cmnPart.pingCnt));
  index += 2;
  che->cmnPart.rxFansPerPing = buffer[index];
  index++;
  che->cmnPart.rxFanIndex = buffer[index];
  index++;
  che->cmnPart.swathsPerPing = buffer[index];
  index++;
  che->cmnPart.swathAlongPosition = buffer[index];
  index++;
  che->cmnPart.txTransducerInd = buffer[index];
  index++;
  che->cmnPart.rxTransducerInd = buffer[index];
  index++;
  che->cmnPart.numRxTransducers = buffer[index];
  index++;
  che->cmnPart.algorithmType = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesCmnPart:      %d\n", che->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       pingCnt:              %d\n", che->cmnPart.pingCnt);
    fprintf(stderr, "dbg5       rxFansPerPing:        %d\n", che->cmnPart.rxFansPerPing);
    fprintf(stderr, "dbg5       rxFanIndex:           %d\n", che->cmnPart.rxFanIndex);
    fprintf(stderr, "dbg5       swathsPerPing:        %d\n", che->cmnPart.swathsPerPing);
    fprintf(stderr, "dbg5       swathAlongPosition:   %d\n", che->cmnPart.swathAlongPosition);
    fprintf(stderr, "dbg5       txTransducerInd:      %d\n", che->cmnPart.txTransducerInd);
    fprintf(stderr, "dbg5       rxTransducerInd:      %d\n", che->cmnPart.rxTransducerInd);
    fprintf(stderr, "dbg5       numRxTransducers:     %d\n", che->cmnPart.numRxTransducers);
    fprintf(stderr, "dbg5       algorithmType:        %d\n", che->cmnPart.algorithmType);
  }

  mb_get_binary_float(true, &buffer[index], &che->data.heave_m);
  index += 4;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       heave_m                         = %f\n", che->data.heave_m);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_HEAVE;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_iip(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_iip *iip = &(store->iip);

  /* copy the header */
  iip->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &iip->numBytesCmnPart);
  index += 2;
  mb_get_binary_short(true, &buffer[index], &iip->info);
  index += 2;
  mb_get_binary_short(true, &buffer[index], &iip->status);
  index += 2;
  size_t numBytesRawSensorData = iip->header.numBytesDgm - MBSYS_KMBES_IIP_VAR_OFFSET;
  memcpy(&iip->install_txt, &buffer[index], numBytesRawSensorData);
//fprintf(stderr, "\niip->install_txt:\n%s\n", iip->install_txt);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", iip->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", iip->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", iip->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", iip->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", iip->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", iip->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", iip->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", iip->numBytesCmnPart);
    fprintf(stderr, "dbg5       info:             %u\n", iip->info);
    fprintf(stderr, "dbg5       status:           %u\n", iip->status);
    fprintf(stderr, "dbg5       install_txt:      %s\n", iip->install_txt);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_INSTALLATION;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_iop(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_iop *iop = &(store->iop);

  /* copy the header */
  iop->header = *header;

  size_t numBytesRawSensorData = iop->header.numBytesDgm - MBSYS_KMBES_IOP_VAR_OFFSET;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &iop->numBytesCmnPart);
  index += 2;
  mb_get_binary_short(true, &buffer[index], &iop->info);
  index += 2;
  mb_get_binary_short(true, &buffer[index], &iop->status);
  index += 2;
  memcpy(&iop->runtime_txt, &buffer[index], numBytesRawSensorData);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", iop->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", iop->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", iop->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", iop->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", iop->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", iop->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", iop->header.time_nanosec);

    fprintf(stderr, "dbg5       iop->iop->numBytesCmnPart:  %u\n", iop->numBytesCmnPart);
    fprintf(stderr, "dbg5       iop->info:                  %u\n", iop->info);
    fprintf(stderr, "dbg5       iop->status:                %u\n", iop->status);
    fprintf(stderr, "dbg5       iop->runtime_txt:           %s\n", iop->runtime_txt);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_RUN_PARAMETER; // TODO: check data kind
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_ibe(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_ib *ibe = &(store->ibe);

  /* copy the header */
  ibe->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &ibe->numBytesCmnPart);
  index += 2;
  ibe->BISTInfo = buffer[index];
  index++;
  ibe->BISTStyle = buffer[index];
  index++;
  ibe->BISTNumber = buffer[index];
  index++;
  ibe->BISTStatus = buffer[index];
  index++;
  ibe->BISTText = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", ibe->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", ibe->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", ibe->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", ibe->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", ibe->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", ibe->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", ibe->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:            %u\n", ibe->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:                   %u\n", ibe->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:                  %u\n", ibe->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:                 %d\n", ibe->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:                 %d\n", ibe->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:                   %c\n", ibe->BISTText);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_BIST;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_ibr(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_ib *ibr = &(store->ibr);

  /* copy the header */
  ibr->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &ibr->numBytesCmnPart);
  index += 2;
  ibr->BISTInfo = buffer[index];
  index++;
  ibr->BISTStyle = buffer[index];
  index++;
  ibr->BISTNumber = buffer[index];
  index++;
  ibr->BISTStatus = buffer[index];
  index++;
  ibr->BISTText = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", ibr->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", ibr->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", ibr->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", ibr->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", ibr->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", ibr->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", ibr->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:            %u\n", ibr->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:                   %u\n", ibr->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:                  %u\n", ibr->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:                 %d\n", ibr->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:                 %d\n", ibr->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:                   %c\n", ibr->BISTText);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_BIST1;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_ibs(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_ib *ibs = &(store->ibs);

  /* copy the header */
  ibs->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &ibs->numBytesCmnPart);
  index += 2;
  ibs->BISTInfo = buffer[index];
  index++;
  ibs->BISTStyle = buffer[index];
  index++;
  ibs->BISTNumber = buffer[index];
  index++;
  ibs->BISTStatus = buffer[index];
  index++;
  ibs->BISTText = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", ibs->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", ibs->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", ibs->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", ibs->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", ibs->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", ibs->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", ibs->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:            %u\n", ibs->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:                   %u\n", ibs->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:                  %u\n", ibs->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:                 %d\n", ibs->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:                 %d\n", ibs->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:                   %c\n", ibs->BISTText);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_BIST2;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_fcf(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error){

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_fcf *fcf = &(store->fcf);
  struct mbsys_kmbes_m_partition *partition = &(fcf->partition);
  struct mbsys_kmbes_f_common *cmnPart = &(fcf->cmnPart);

  /* copy the header */
  fcf->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  /* EMdgmMpartition - data partition information */
  mb_get_binary_short(true, &buffer[index], &(partition->numOfDgms));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(partition->dgmNum));
  index += 2;

  mb_get_binary_short(true, &buffer[index], &cmnPart->numBytesCmnPart);
  index += 2;
  cmnPart->fileStatus = buffer[index];
  index++;
  cmnPart->padding1 = buffer[index];
  index++;
  mb_get_binary_int(true, &buffer[index], &cmnPart->numBytesFile);
  index += 4;
  memcpy(cmnPart->fileName, &buffer[index], MBSYS_KMBES_MAX_F_FILENAME_LENGTH);
  index += MBSYS_KMBES_MAX_F_FILENAME_LENGTH;

  memcpy(fcf->bsCalibrationFile, &buffer[index], cmnPart->numBytesFile);
  index += cmnPart->numBytesFile;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", fcf->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", fcf->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", fcf->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", fcf->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", fcf->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", fcf->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", fcf->header.time_nanosec);

    fprintf(stderr, "dbg5       numOfDgms:                  %d\n", fcf->partition.numOfDgms);
    fprintf(stderr, "dbg5       dgmNum:                     %d\n", fcf->partition.dgmNum);

    fprintf(stderr, "dbg5       numBytesCmnPart:            %u\n", fcf->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       fileStatus:                 %u\n", fcf->cmnPart.fileStatus);
    fprintf(stderr, "dbg5       fileStatus:                 %u\n", fcf->cmnPart.padding1);
    fprintf(stderr, "dbg5       numBytesFile:               %u\n", fcf->cmnPart.numBytesFile);
    fprintf(stderr, "dbg5       fcf->fileName:              %s\n", fcf->cmnPart.fileName);
    fprintf(stderr, "dbg5       fcf->bsCalibrationFile:\n");
    for (unsigned int i=0; i<cmnPart->numBytesFile; i++)
      fprintf(stderr, "%c", fcf->bsCalibrationFile[i]);
    fprintf(stderr,"\n");
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_BSCALIBRATIONFILE;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_xmb(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  size_t numBytesVersion = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_xmb *xmb = &(store->xmb);

  /* copy the header */
  xmb->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_int(true, &buffer[index], &xmb->mbsystem_extensions);
  index += 4;
  mb_get_binary_int(true, &buffer[index], &xmb->watercolumn);
  index += 4;
  for (int i = 0; i < 24; i++) {
    xmb->unused[i] = buffer[index];
    index++;
  }
  numBytesVersion = xmb->header.numBytesDgm - MBSYS_KMBES_HEADER_SIZE - 36;
  memcpy(xmb->version, &buffer[index], numBytesVersion);
  index += numBytesVersion;
  memset(&xmb->version[numBytesVersion], 0, MB_COMMENT_MAXLINE - numBytesVersion);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xmb->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", xmb->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", xmb->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", xmb->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", xmb->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", xmb->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", xmb->header.time_nanosec);

    fprintf(stderr, "dbg5       mbsystem_extensions:  %d\n", xmb->mbsystem_extensions);
    fprintf(stderr, "dbg5       watercolumn:          %d\n", xmb->watercolumn);
    for (int i = 0; i < 24; i++)
      fprintf(stderr, "dbg5       unused[%2d]:    %u\n", i, xmb->unused[i]);
    fprintf(stderr, "dbg5       version:        %s\n", xmb->version);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_MBSYSTEM;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_xmc(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_xmc *xmc = &(store->xmc);

  /* copy the header */
  xmc->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  for (int i = 0; i < 32; i++) {
    xmc->unused[i] = buffer[index];
    index++;
  }
  size_t numBytesComment = xmc->header.numBytesDgm - MBSYS_KMBES_HEADER_SIZE - 36;
  memcpy(xmc->comment, &buffer[index], numBytesComment);
  index += numBytesComment;
  memset(&xmc->comment[numBytesComment], 0, MB_COMMENT_MAXLINE - numBytesComment);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xmc->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", xmc->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", xmc->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", xmc->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", xmc->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", xmc->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", xmc->header.time_nanosec);

    for (int i = 0; i < 32; i++)
      fprintf(stderr, "dbg5       xmc->unused[%2d]:                        %u\n", i, xmc->unused[i]);
    fprintf(stderr, "dbg5       xmc->comment:                           %s\n", xmc->comment);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_COMMENT;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};
/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_xmt(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *ixmt, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *) buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *) store_ptr);
  }

  struct mbsys_kmbes_xmt *xmt = NULL;
  struct mbsys_kmbes_m_partition partition;
  struct mbsys_kmbes_m_body cmnPart;
  int index_EMdgmMbody;
  int index_pingInfo;
  // int index_txSectorInfo = 0;
  // int index_rxInfo;
  // int index_extraDetClassInfo;
  int index_sounding;
  // int index_SIsample = 0;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", header->numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", header->dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", header->dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", header->systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", header->echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", header->time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", header->time_nanosec);
  }

  /* get the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  /* EMdgmMpartition - data partition information */
  mb_get_binary_short(true, &buffer[index], &(partition.numOfDgms));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(partition.dgmNum));
  index += 2;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numOfDgms = %d\n", partition.numOfDgms);
    fprintf(stderr, "dbg5       dgmNum    = %d\n", partition.dgmNum);
  }

  /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
  mb_get_binary_short(true, &buffer[index], &(cmnPart.numBytesCmnPart));
  index_EMdgmMbody = index;
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(cmnPart.pingCnt));
  index += 2;
  cmnPart.rxFansPerPing = buffer[index];
  index++;
  cmnPart.rxFanIndex = buffer[index];
  index++;
  cmnPart.swathsPerPing = buffer[index];
  index++;
  cmnPart.swathAlongPosition = buffer[index];
  index++;
  cmnPart.txTransducerInd = buffer[index];
  index++;
  cmnPart.rxTransducerInd = buffer[index];
  index++;
  cmnPart.numRxTransducers = buffer[index];
  index++;
  cmnPart.algorithmType = buffer[index];
  index++;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       pingCnt:             %d\n", cmnPart.pingCnt);
    fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", cmnPart.rxFansPerPing);
    fprintf(stderr, "dbg5       rxFanIndex:          %d\n", cmnPart.rxFanIndex);
    fprintf(stderr, "dbg5       swathsPerPing:       %d\n", cmnPart.swathsPerPing);
    fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", cmnPart.swathAlongPosition);
    fprintf(stderr, "dbg5       txTransducerInd:     %d\n", cmnPart.txTransducerInd);
    fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", cmnPart.rxTransducerInd);
    fprintf(stderr, "dbg5       numRxTransducers:    %d\n", cmnPart.numRxTransducers);
    fprintf(stderr, "dbg5       algorithmType:       %d\n", cmnPart.algorithmType);
  }

  /* now figure out which of the XMT datagrams for this ping we are reading
    (cmnPart.rxFanIndex out of cmnPart.rxFansPerPing) */
  *ixmt = cmnPart.rxFanIndex;
  xmt = &store->xmt[*ixmt];
  xmt->header = *header;
  xmt->partition = partition;
  xmt->cmnPart = cmnPart;

  /* reset index to start of mbsys_kmbes_xmt_ping_info using cmnPart.numBytesCmnPart
      - this avoids breaking the decoding if fields have been added to cmnPart */
  index_pingInfo = index_EMdgmMbody + cmnPart.numBytesCmnPart;
  index = index_pingInfo;

  /* mbsys_kmbes_xmt_ping_info - ping info */
  mb_get_binary_short(true, &buffer[index], &(xmt->xmtPingInfo.numBytesInfoData));
  index += 2;
  mb_get_binary_short(true, &buffer[index], &(xmt->xmtPingInfo.numBytesPerSounding));
  index += 2;
  mb_get_binary_int(true, &buffer[index], &(xmt->xmtPingInfo.padding0));
  index += 4;
  mb_get_binary_double(true, &buffer[index], &(xmt->xmtPingInfo.longitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(xmt->xmtPingInfo.latitude));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(xmt->xmtPingInfo.sensordepth));
  index += 8;
  mb_get_binary_double(true, &buffer[index], &(xmt->xmtPingInfo.heading));
  index += 8;
  mb_get_binary_float(true, &buffer[index], &(xmt->xmtPingInfo.speed));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(xmt->xmtPingInfo.roll));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(xmt->xmtPingInfo.pitch));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(xmt->xmtPingInfo.heave));
  index += 4;
  mb_get_binary_float(true, &buffer[index], &(xmt->xmtPingInfo.numSoundings));
  index += 4;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesInfoData:            %d\n", xmt->xmtPingInfo.numBytesInfoData);
    fprintf(stderr, "dbg5       numBytesPerSounding:         %d\n", xmt->xmtPingInfo.numBytesPerSounding);
    fprintf(stderr, "dbg5       padding0:                    %d\n", xmt->xmtPingInfo.padding0);
    fprintf(stderr, "dbg5       longitude:                   %f\n", xmt->xmtPingInfo.longitude);
    fprintf(stderr, "dbg5       latitude:                    %f\n", xmt->xmtPingInfo.latitude);
    fprintf(stderr, "dbg5       sensordepth:                 %f\n", xmt->xmtPingInfo.sensordepth);
    fprintf(stderr, "dbg5       heading:                     %f\n", xmt->xmtPingInfo.heading);
    fprintf(stderr, "dbg5       speed:                       %f\n", xmt->xmtPingInfo.speed);
    fprintf(stderr, "dbg5       roll:                        %f\n", xmt->xmtPingInfo.roll);
    fprintf(stderr, "dbg5       pitch:                       %f\n", xmt->xmtPingInfo.pitch);
    fprintf(stderr, "dbg5       heave:                       %f\n", xmt->xmtPingInfo.heave);
    fprintf(stderr, "dbg5       numSoundings:                %d\n", xmt->xmtPingInfo.numSoundings);
  }

  /* calculate index at start of sounding using rxInfo.numExtraDetectionClasses & rxInfo.numBytesPerClass
      - this avoids breaking the decoding if fields have been added to extraDetClassInfo */
  index_sounding = index_pingInfo + xmt->xmtPingInfo.numBytesInfoData;

  /* EMdgmMRZ_sounding - Data for each sounding */
  int numSoundings = xmt->xmtPingInfo.numSoundings;
  for (int i = 0; i<numSoundings; i++) {
    /* calculate index at start of of each sounding using pingInfo.numBytesPerTxSector
       - this avoids breaking the decoding if fields have been added to sounding */
    index = index_sounding + i * xmt->xmtPingInfo.numBytesPerSounding;

    mb_get_binary_short(true, &buffer[index], &(xmt->xmtSounding[i].soundingIndex));
    index += 2;
    mb_get_binary_short(true, &buffer[index], &(xmt->xmtSounding[i].padding0));
    index += 2;
    mb_get_binary_float(true, &buffer[index], &(xmt->xmtSounding[i].twtt));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(xmt->xmtSounding[i].angle_vertical));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(xmt->xmtSounding[i].angle_azimuthal));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(xmt->xmtSounding[i].beam_heave));
    index += 4;
    mb_get_binary_float(true, &buffer[index], &(xmt->xmtSounding[i].alongtrack_offset));
    index += 4;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       soundingIndex:                   %d\n", xmt->xmtSounding[i].soundingIndex);
      fprintf(stderr, "dbg5       padding0:                        %d\n", xmt->xmtSounding[i].padding0);
      fprintf(stderr, "dbg5       twtt:                            %f\n", xmt->xmtSounding[i].twtt);
      fprintf(stderr, "dbg5       angle_vertical:                  %f\n", xmt->xmtSounding[i].angle_vertical);
      fprintf(stderr, "dbg5       angle_azimuthal:                 %f\n", xmt->xmtSounding[i].angle_azimuthal);
      fprintf(stderr, "dbg5       beam_heave:                      %f\n", xmt->xmtSounding[i].beam_heave);
      fprintf(stderr, "dbg5       alongtrack_offset:               %f\n", xmt->xmtSounding[i].alongtrack_offset);
    }
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - index:%d time: %d.%9.9d ping:%d rx:%d/%d xms expected:%d status:%d error:%d\n",
          header->dgmType, index, header->time_sec, header->time_nanosec, cmnPart.pingCnt, cmnPart.rxFanIndex, cmnPart.rxFansPerPing,
          store->xmb.mbsystem_extensions, status, *error);
#endif

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_xms(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;
  struct mbsys_kmbes_xms *xms = &(store->xms);

  /* copy the header */
  xms->header = *header;

  /* extract the data */
  int index = MBSYS_KMBES_HEADER_SIZE;

  mb_get_binary_short(true, &buffer[index], &xms->pingCnt);
  index += 2;
  mb_get_binary_short(true, &buffer[index], &xms->spare);
  index += 2;
  mb_get_binary_float(true, &buffer[index], &xms->pixel_size);
  index += 4;
  mb_get_binary_int(true, &buffer[index], &xms->pixels_ss);
  index += 4;
  for (int i=0;i<32;i++) {
    xms->unused[i] = buffer[index];
    index++;
  }
  for (int i=0;i<xms->pixels_ss;i++) {
    mb_get_binary_float(true, &buffer[index], &xms->ss[i]);
    index += 4;
  }
  for (int i=0;i<xms->pixels_ss;i++) {
    mb_get_binary_float(true, &buffer[index], &xms->ss_alongtrack[i]);
    index += 4;
  }

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values read in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xms->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", xms->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", xms->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", xms->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", xms->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", xms->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", xms->header.time_nanosec);

    fprintf(stderr, "dbg5       pingCnt:        %u\n", xms->pingCnt);
    fprintf(stderr, "dbg5       spare:          %d\n", xms->spare);
    fprintf(stderr, "dbg5       pixel_size:     %f\n", xms->pixel_size);
    fprintf(stderr, "dbg5       pixels_ss:      %d\n", xms->pixels_ss);
    for (int i=0;i<32;i++)
      fprintf(stderr, "dbg5       unused[%2d]:    %u\n", i, xms->unused[i]);
    for (int i=0;i<xms->pixels_ss;i++)
      fprintf(stderr, "dbg5       ss[%2d]:        %f %f\n",
                      i, xms->ss[i], xms->ss_alongtrack[i]);
  }

  int status = MB_SUCCESS;

  /* set kind */
  if (status == MB_SUCCESS) {
    /* set kind */
    store->kind = MB_DATA_DATA;
  }
  else {
    store->kind = MB_DATA_NONE;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_unknown(int verbose, char *buffer, void *store_ptr, void *header_ptr, int *error) {

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       buffer:     %p\n", (void *)buffer);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  // struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  // struct mbsys_kmbes_header *header = (struct mbsys_kmbes_header *)header_ptr;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  struct mbsys_kmbes_header *header;
  header = (struct mbsys_kmbes_header *) header_ptr;
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          header->dgmType, header->time_sec, header->time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};
/*--------------------------------------------------------------------*/

int mbr_kemkmall_index_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  struct mbsys_kmbes_index_table *dgm_index_table = NULL;
  struct mbsys_kmbes_index dgm_index;
  struct mbsys_kmbes_header header;
  struct mbsys_kmbes_m_body cmnPart;
  mbsys_kmbes_emdgm_type emdgm_type;
  int *file_indexed = NULL;
  int *dgm_id = NULL;
  char buffer[256];
  size_t read_len = 0;
  size_t offset = 0;
  unsigned int index = 0;
  unsigned int skip = 0;
  unsigned int num_bytes_dgm_end = 0;
  int iip_location = -1;
  const int HEADER_SKIP = 8;
  unsigned short pingCnt = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

  /* create a datagram index table in mbio descriptor */
  mbr_kemkmall_create_dgm_index_table(verbose, mbio_ptr, store_ptr, error);

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  /* now get the index table from the mbio descriptor field saveptr1 */
  dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;
  dgm_index_table->dgm_count = 0;
  dgm_id = (int *)&mb_io_ptr->save1;
  *dgm_id = 0;
  file_indexed = (int *)&mb_io_ptr->save2;
  *file_indexed = false;

  /* set file position to the start */
  fseek(mb_io_ptr->mbfp, 0, SEEK_SET);
  mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

  /* set status */
  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;
  // bool valid_id = true;

  while (*error <= MB_ERROR_NO_ERROR) {

    /* find the next valid datagram header */
    memset(&buffer, 0, sizeof(buffer));
    read_len = (size_t)MBSYS_KMBES_HEADER_SIZE;
    skip = 0;
    status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);
    status = mbr_kemkmall_rd_hdr(verbose, buffer, (void *)&header, (void *)&emdgm_type, error);
    while (status == MB_SUCCESS && emdgm_type == UNKNOWN) {
      for (int i=0;i<MBSYS_KMBES_HEADER_SIZE-1;i++) {
        buffer[i] = buffer[i+1];
      }
      read_len = 1;
      status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[MBSYS_KMBES_HEADER_SIZE-1], &read_len, error);
      skip++;
      if (status == MB_SUCCESS) {
        status = mbr_kemkmall_rd_hdr(verbose, buffer, (void *)&header, (void *)&emdgm_type, error);
      }
    }

    /* report problem */
    if (status == MB_SUCCESS && skip > 0 && verbose >= 0) {
      fprintf(stderr, "\nThe MBF_KEMKMALL module skipped data between identified\n"
              "data records. Something is broken, most likely the data...\n"
              "However, the data may include a data record type that we\n"
              "haven't seen yet, or there could be an error in the code.\n"
              "If skipped data are reported multiple times, we recommend \n"
              "you post a problem description through the discussion list \n"
              "available at https://listserver.mbari.org/sympa/arc/mbsystem \n"
              "and make a data sample available. \n"
              "Have a nice day...\n");
      fprintf(stderr, "MBF_KEMKMALL skipped %d bytes before record %.4s at file pos %ld\n",
                      skip, header.dgmType, ftell(mb_io_ptr->mbfp));
    }

    /* now parse the header and index the datagram */
    if (status == MB_SUCCESS && emdgm_type != UNKNOWN) {

      /* verify datagram is intact - seek to end of the datagram and read last int
         - make survey mb_io_ptr->file_pos records the position of the start of the datagram */
      mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp) - MBSYS_KMBES_HEADER_SIZE;
      offset = (header.numBytesDgm - MBSYS_KMBES_HEADER_SIZE - sizeof(int));
      fseek(mb_io_ptr->mbfp, offset, SEEK_CUR);

      read_len = sizeof(int);
      status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

      if (status == MB_SUCCESS) {
        /* Confirm that that byte count in the last int matches start int */
        mb_get_binary_int(true, &buffer[0], &num_bytes_dgm_end);

        if (header.numBytesDgm != num_bytes_dgm_end) {
          /* No match. Assume packet header was really corrupt, so reset
            file pointer past corrupted packet header and set datagram type to unknown. */
#ifdef MBR_KEMKMALL_DEBUG
          fprintf(stderr, "\nError in record %.4s at file pointer position %ld.\n"
                "Expected a dgm with %u bytes, but dgm closing byte indicated %u.\n"
                "Skipping past the corrupted packet header and setting datagram type to unknown.\n",
                header.dgmType, mb_io_ptr->file_pos, header.numBytesDgm, num_bytes_dgm_end);
#endif
          mb_io_ptr->file_pos += HEADER_SKIP;
          fseek(mb_io_ptr->mbfp, mb_io_ptr->file_pos, SEEK_SET);
          emdgm_type = UNKNOWN;
          // valid_id = false;
        }
      }

      /* handle a valid datagram */
      if (status == MB_SUCCESS && emdgm_type != UNKNOWN) {

        /* populate the index entry structure. */
        memset(&dgm_index, 0, sizeof(struct mbsys_kmbes_index));
        switch (emdgm_type) {

          case MWC:
          case MRZ:
            /* Valid multibeam datagram: */
            /* parse the dgm to get additional info about ping. */
            /* this is necessary to insure multi-TX/RX and dual-swath modes are handled correctly. */

            /* if MWC set flag indicating water column records are present */
            if (emdgm_type == MWC) {
              store->xmb.watercolumn = 1;
            }

            /* get ping info */
            /* skip past the header and the 2 shorts that make up the dgm partition part */
            offset = (mb_io_ptr->file_pos + MBSYS_KMBES_HEADER_SIZE + sizeof(int));
            fseek(mb_io_ptr->mbfp, offset, SEEK_SET);

            read_len = 12;
            status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

            if (status == MB_SUCCESS) {

              /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
              index = 0;
              mb_get_binary_short(true, &buffer[index], &(cmnPart.numBytesCmnPart));
              index += 2;
              mb_get_binary_short(true, &buffer[index], &(cmnPart.pingCnt));
              index += 2;
              cmnPart.rxFansPerPing = buffer[index];
              index++;
              cmnPart.rxFanIndex = buffer[index];
              index++;
              cmnPart.swathsPerPing = buffer[index];
              index++;
              cmnPart.swathAlongPosition = buffer[index];
              index++;
              cmnPart.txTransducerInd = buffer[index];
              index++;
              cmnPart.rxTransducerInd = buffer[index];
              index++;
              cmnPart.numRxTransducers = buffer[index];
              index++;
              cmnPart.algorithmType = buffer[index];
              index++;

              /* populate the datagram index entry */
              dgm_index.time_d = ((double)header.time_sec) + MBSYS_KMBES_NANO * header.time_nanosec;
              dgm_index.emdgm_type = emdgm_type;
              memcpy(&dgm_index.header, &header, sizeof(struct mbsys_kmbes_header));
              dgm_index.file_pos = mb_io_ptr->file_pos;
              dgm_index.ping_num = cmnPart.pingCnt;
              dgm_index.rx_per_ping = cmnPart.rxFansPerPing;
              dgm_index.rx_index = cmnPart.rxFanIndex;
              dgm_index.swaths_per_ping = cmnPart.swathsPerPing;
#ifdef MBR_KEMKMALL_DEBUG
              int time_i[7];
              mb_get_date(verbose, dgm_index.time_d, time_i);
              fprintf(stderr,"%.4s:%d pos:%ld nbytes:%u Cnt:%d rxFans:%d:%d swaths:%d along:%d tx:%d rx:%d numrx:%d alg:%d  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
                      header.dgmType,emdgm_type,dgm_index.file_pos,header.numBytesDgm,cmnPart.pingCnt, cmnPart.rxFansPerPing, cmnPart.rxFanIndex,
                      cmnPart.swathsPerPing, cmnPart.swathAlongPosition, cmnPart.txTransducerInd,
                      cmnPart.rxTransducerInd, cmnPart.numRxTransducers, cmnPart.algorithmType,
                      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]);
#endif

              status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, &dgm_index, error);
            }

            if (status == MB_SUCCESS) {
              offset = (size_t) (mb_io_ptr->file_pos + header.numBytesDgm);
              fseek(mb_io_ptr->mbfp, offset, SEEK_SET);
            }
            // TODO: what happens if alloc fails - while condition?
            break;

          case XMT:
            /* Valid multibeam datagram: */
            /* parse the dgm to get additional info about ping. */
            /* this is necessary to insure multi-TX/RX and dual-swath modes are handled correctly. */

            /* get ping info */
            /* skip past the header and the 2 shorts that make up the dgm partition part */
            offset = (mb_io_ptr->file_pos + MBSYS_KMBES_HEADER_SIZE + sizeof(int));
            fseek(mb_io_ptr->mbfp, offset, SEEK_SET);

            read_len = 12;
            status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

            if (status == MB_SUCCESS) {

              /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
              index = 0;
              mb_get_binary_short(true, &buffer[index], &(cmnPart.numBytesCmnPart));
              index += 2;
              mb_get_binary_short(true, &buffer[index], &(cmnPart.pingCnt));
              index += 2;
              cmnPart.rxFansPerPing = buffer[index];
              index++;
              cmnPart.rxFanIndex = buffer[index];
              index++;
              cmnPart.swathsPerPing = buffer[index];
              index++;
              cmnPart.swathAlongPosition = buffer[index];
              index++;
              cmnPart.txTransducerInd = buffer[index];
              index++;
              cmnPart.rxTransducerInd = buffer[index];
              index++;
              cmnPart.numRxTransducers = buffer[index];
              index++;
              cmnPart.algorithmType = buffer[index];
              index++;

              /* populate the datagram index entry */
              dgm_index.time_d = ((double)header.time_sec) + MBSYS_KMBES_NANO * header.time_nanosec;
              dgm_index.emdgm_type = emdgm_type;
              memcpy(&dgm_index.header, &header, sizeof(struct mbsys_kmbes_header));
              dgm_index.file_pos = mb_io_ptr->file_pos;
              dgm_index.ping_num = cmnPart.pingCnt;
              dgm_index.rx_per_ping = cmnPart.rxFansPerPing;
              dgm_index.rx_index = cmnPart.rxFanIndex;
              dgm_index.swaths_per_ping = cmnPart.swathsPerPing;
#ifdef MBR_KEMKMALL_DEBUG
              int time_i[7];
              mb_get_date(verbose, dgm_index.time_d, time_i);
              fprintf(stderr,"%.4s:%d pos:%ld nbytes:%u Cnt:%d rxFans:%d:%d swaths:%d along:%d tx:%d rx:%d numrx:%d alg:%d  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
                      header.dgmType,emdgm_type,dgm_index.file_pos,header.numBytesDgm,cmnPart.pingCnt, cmnPart.rxFansPerPing, cmnPart.rxFanIndex,
                      cmnPart.swathsPerPing, cmnPart.swathAlongPosition, cmnPart.txTransducerInd,
                      cmnPart.rxTransducerInd, cmnPart.numRxTransducers, cmnPart.algorithmType,
                      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]);
#endif

              status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, &dgm_index, error);
            }

            if (status == MB_SUCCESS) {
              offset = (size_t) (mb_io_ptr->file_pos + header.numBytesDgm);
              fseek(mb_io_ptr->mbfp, offset, SEEK_SET);
            }
            // TODO: what happens if alloc fails - while condition?
            break;

          case XMS:
            /* Valid multibeam pseudosidescan datagram: */
            /* parse the dgm to get additional info about ping. */
            /* this is necessary to insure multi-TX/RX and dual-swath modes are handled correctly. */

            /* get ping info */
            /* skip past the header and the 2 shorts that make up the dgm partition part */
            offset = (mb_io_ptr->file_pos + MBSYS_KMBES_HEADER_SIZE + sizeof(int));
            fseek(mb_io_ptr->mbfp, offset, SEEK_SET);

            read_len = 12;
            status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);

            if (status == MB_SUCCESS) {

              /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
              index = 0;
              mb_get_binary_short(true, &buffer[index], &(pingCnt));

              /* populate the datagram index entry */
              dgm_index.time_d = ((double)header.time_sec) + MBSYS_KMBES_NANO * header.time_nanosec;
              dgm_index.emdgm_type = emdgm_type;
              memcpy(&dgm_index.header, &header, sizeof(struct mbsys_kmbes_header));
              dgm_index.file_pos = mb_io_ptr->file_pos;
              dgm_index.ping_num = cmnPart.pingCnt;
              dgm_index.rx_per_ping = 0;
              dgm_index.rx_index = 0;
              dgm_index.swaths_per_ping = 0;
#ifdef MBR_KEMKMALL_DEBUG
              int time_i[7];
              mb_get_date(verbose, dgm_index.time_d, time_i);
              fprintf(stderr,"%.4s:%d pos:%ld nbytes:%u Cnt:%d rxFans:%d:%d swaths:%d along:%d tx:%d rx:%d numrx:%d alg:%d  %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
                      header.dgmType,emdgm_type,dgm_index.file_pos,header.numBytesDgm,cmnPart.pingCnt, cmnPart.rxFansPerPing, cmnPart.rxFanIndex,
                      cmnPart.swathsPerPing, cmnPart.swathAlongPosition, cmnPart.txTransducerInd,
                      cmnPart.rxTransducerInd, cmnPart.numRxTransducers, cmnPart.algorithmType,
                      time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]);
#endif

              status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, &dgm_index, error);
            }

            if (status == MB_SUCCESS) {
              offset = (size_t) (mb_io_ptr->file_pos + header.numBytesDgm);
              fseek(mb_io_ptr->mbfp, offset, SEEK_SET);
            }
            // TODO: what happens if alloc fails - while condition?
            break;

          default:
            /* Other valid datagram: */

            /* save index of first #IIP datagram */
            if (iip_location < 0 && emdgm_type == IIP) {
              iip_location = dgm_index_table->dgm_count;
            }

            /* populate the datagram index entry */
            dgm_index.time_d = ((double)header.time_sec) + MBSYS_KMBES_NANO * header.time_nanosec;
            dgm_index.emdgm_type = emdgm_type;
            memcpy(&dgm_index.header, &header, sizeof(struct mbsys_kmbes_header));
            dgm_index.file_pos = mb_io_ptr->file_pos;
            dgm_index.ping_num = 0;
            dgm_index.rx_per_ping = 0;
            dgm_index.rx_index = 0;
            dgm_index.swaths_per_ping = 0;
#ifdef MBR_KEMKMALL_DEBUG
            int time_i[7];
            mb_get_date(verbose, dgm_index.time_d, time_i);
            fprintf(stderr,"%.4s:%d pos:%ld nbytes:%u %4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d.%6.6d\n",
                    header.dgmType,emdgm_type,dgm_index.file_pos,header.numBytesDgm,time_i[0],time_i[1],time_i[2],time_i[3],time_i[4],time_i[5],time_i[6]);
#endif

            status = mbr_kemkmall_add_dgm_to_dgm_index_table(verbose, dgm_index_table, &dgm_index, error);

            if (status == MB_SUCCESS) {
              offset = (size_t) (mb_io_ptr->file_pos + header.numBytesDgm);
              fseek(mb_io_ptr->mbfp, offset, SEEK_SET);
            }
            break;
        }

        /* update file position */
        mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);
      }
    }
  }

  /* set indexed flag */
  *file_indexed = true;
  *dgm_id = 0;
  if ((dgm_index_table->dgm_count > 0) && (*error = MB_ERROR_EOF)) {
    status = MB_SUCCESS;
    *error = MB_ERROR_NO_ERROR;
  }

  /* sort the index table into the order to be read */
  if (status == MB_SUCCESS && dgm_index_table->dgm_count > 0) {

    /* each ping can have multiple MRZ and MWC datagrams - the ping timestamp is
        the timestamp of the first MRZ datagram - add ping timestamp to index entries
        for all MRZ and MWC datagrams */
    /* start by sorting the index table on ping number alone, and then looping
          over all datagrams in ping order setting the ping timestamp */
    //qsort((void *)dgm_index_table->indextable, dgm_index_table->dgm_count,
    //    sizeof(struct mbsys_kmbes_index), (void *)mbr_kemkmall_indextable_compare_pings);


    /* sort the datagram index table, leaving the datagrams up through the
       #IIP datagram in place at the start of the file - in practice this means
       any comment datagrams stay at the start of the file */

    qsort((void *)dgm_index_table->indextable, dgm_index_table->dgm_count,
        sizeof(struct mbsys_kmbes_index), (void *)mbr_kemkmall_indextable_compare);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "\n\nIndexed %ld valid EM datagrams:\n", dgm_index_table->dgm_count);
  for (unsigned int i=0; i<dgm_index_table->dgm_count; i++)
  {
    fprintf(stderr, "ID: %4d, ", i);
    fprintf(stderr, "file_pos: %8.zu, ", dgm_index_table->indextable[i].file_pos);
    fprintf(stderr, "dgm: %.4s, ", dgm_index_table->indextable[i].header.dgmType);
    fprintf(stderr, "type: %2d, ", dgm_index_table->indextable[i].emdgm_type);
    fprintf(stderr, "size: %6u, ", dgm_index_table->indextable[i].header.numBytesDgm);
    fprintf(stderr, "time: %9.3f, ", dgm_index_table->indextable[i].time_d);
    fprintf(stderr, "ping: %5d, ", dgm_index_table->indextable[i].ping_num);
    fprintf(stderr, "rxIndex: %u/%u.\n", dgm_index_table->indextable[i].rx_index,
                                dgm_index_table->indextable[i].rx_per_ping);
  }
  fprintf(stderr, "\n");
#endif

  /* set file position back to the start */
  fseek(mb_io_ptr->mbfp, 0, SEEK_SET);

    if (verbose >= 2) {
        fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
        fprintf(stderr, "dbg2  Return values:\n");
        fprintf(stderr, "dbg2       error:      %d\n", *error);
        fprintf(stderr, "dbg2  Return status:\n");
        fprintf(stderr, "dbg2       status:  %d\n", status);
    }

  /* return status */
  return (status);

};
/*--------------------------------------------------------------------*/

int mbr_kemkmall_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }
  struct mbsys_kmbes_index_table *dgm_index_table = NULL;
  struct mbsys_kmbes_index *dgm_index = NULL;
  struct mbsys_kmbes_header header;
  size_t read_len = 0;
  char **bufferptr = NULL;
  char *buffer = NULL;
  size_t *bufferalloc = NULL;
  unsigned int *dgm_id = NULL;
  int jmrz;
  int jmwc;
  int jxmt, isounding;
  int numSoundings, numBackscatterSamples;

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get buffer and related vars from mbio saved values */
  bufferptr = (char **)&mb_io_ptr->raw_data;
  bufferalloc = (size_t *)&mb_io_ptr->structure_size;
  buffer = (char *)*bufferptr;
  mbsys_kmbes_emdgm_type emdgm_type;

  /* get the datagram index table */
  dgm_index_table = (struct mbsys_kmbes_index_table *)mb_io_ptr->saveptr1;
  dgm_id = (unsigned int *)&mb_io_ptr->save1;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  int status = MB_SUCCESS;
  *error = MB_ERROR_NO_ERROR;

  /* check index to see if more datagrams can be read */
  bool done = false;
  if (mb_io_ptr->mbfp != NULL) {
    if (*dgm_id < dgm_index_table->dgm_count) {
      done = false;
    }
    else {
      done = true;
      *error = MB_ERROR_EOF;
      status = MB_FAILURE;
    }
  }

  /* if not done loop over reading data until a record is ready for return */
  while (!done) {

    // if reading a file then use the index of datagrams
    if (mb_io_ptr->mbfp != NULL) {
      // identify the next record in the index
      dgm_index = &(dgm_index_table->indextable[*dgm_id]);
      store->time_d = dgm_index->time_d;
      mb_get_date(verbose, store->time_d, store->time_i);
      emdgm_type = dgm_index->emdgm_type;

      /* allocate memory to read the record if necessary */
      read_len = (size_t)dgm_index->header.numBytesDgm;
      if (*bufferalloc <= read_len) {
        *bufferalloc = ((read_len / MBSYS_KMBES_START_BUFFER_SIZE) + 1) * MBSYS_KMBES_START_BUFFER_SIZE;
        status = mb_reallocd(verbose, __FILE__, __LINE__, *bufferalloc, (void **)bufferptr, error);
        if (status != MB_SUCCESS) {
          *bufferalloc = 0;
          done = true;
        }
        else {
          buffer = (char *)*bufferptr;
        }
      }

      /* read the next datagram */
      if (status == MB_SUCCESS) {
        fseek(mb_io_ptr->mbfp, dgm_index->file_pos, SEEK_SET);
        status = mb_fileio_get(verbose, mbio_ptr, (void *)&buffer[0], &read_len, error);
        mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);
      }

      // check for partitioned datagrams (i.e. multiple UDP packets that have
      // not been concatenated) and ignore these
      if (status == MB_SUCCESS) {
        status = mbr_kemkmall_rd_hdr(verbose, buffer, (void *)&header, (void *)&emdgm_type, error);
        if (status == MB_SUCCESS && (emdgm_type == MRZ || emdgm_type == MWC)) {
          unsigned short numOfDgms = 0;
          unsigned short dgmNum = 0;
          mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE], &numOfDgms);
          mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE+2], &dgmNum);
          if (numOfDgms != 1) {
            *error = MB_ERROR_UNINTELLIGIBLE;
            status = MB_FAILURE;
fprintf(stderr, "Dropping partial MRZ or MWC datagram numOfDgms:%d dgmNum:%d size:%12d cnt:%d ping:%10d time_d:%.9f\n",
numOfDgms, dgmNum, header.numBytesDgm, dgm_index->index_org, dgm_index->ping_num,
((double)header.time_sec + MBSYS_KMBES_NANO * header.time_nanosec));
          }
        }
      }
    }

    // else if reading from a socket, read the entire next record at once
    else {
      // ensure buffer is large enough for the largest possible number of
      // partitioned UDP packets (65536 each)
      read_len = MB_UDP_SIZE_MAX * MBSYS_KMBES_MAX_NUM_MRZ_DGMS;
      if (*bufferalloc <= read_len) {
        status = mb_reallocd(verbose, __FILE__, __LINE__, read_len, (void **)bufferptr, error);
        if (status != MB_SUCCESS) {
          *bufferalloc = 0;
          done = true;
        }
        else {
          buffer = (char *)*bufferptr;
          *bufferalloc = read_len;
        }
      }

      /* read the next valid datagram */
      read_len = *bufferalloc;
      status = mb_fileio_get(verbose, mbio_ptr, (void *)buffer, &read_len, error);
      if (status == MB_SUCCESS) {
        mb_io_ptr->file_pos += read_len;
        status = mbr_kemkmall_rd_hdr(verbose, buffer, (void *)&header, (void *)&emdgm_type, error);
        store->time_d = ((double)header.time_sec) + MBSYS_KMBES_NANO * header.time_nanosec;
        mb_get_date(verbose, store->time_d, store->time_i);

        // check for partitioned datagrams (i.e. multiple UDP packets that have
        // not been concatenated) and ignore these
        if (status == MB_SUCCESS && (emdgm_type == MRZ || emdgm_type == MWC)) {
          unsigned short numOfDgms = -1;
          mb_get_binary_short(true, &buffer[MBSYS_KMBES_HEADER_SIZE], &numOfDgms);
          if (numOfDgms != 1) {
            *error = MB_ERROR_UNINTELLIGIBLE;
            status = MB_FAILURE;
          }
        }
      }
    }

    // the full datagram should be in the buffer - check if valid according to
    // size value from first four bytes repeated in last four bytes

    /* if valid parse the record type */
    if (status == MB_SUCCESS) {

      switch (emdgm_type) {

        case IIP:
            /* #IIP - Info Installation PU */
          status = mbr_kemkmall_rd_iip(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case IOP:
            /* #IOP -  Runtime datagram */
          status = mbr_kemkmall_rd_iop(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case IBE:
            /* #IBE -  BIST error report */
          status = mbr_kemkmall_rd_ibe(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case IBR:
            /* #IBR -  BIST reply */
          status = mbr_kemkmall_rd_ibr(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case IBS:
            /* #IBS -  BIST short reply */
          status = mbr_kemkmall_rd_ibs(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SPO:
            /* #SPO - Sensor POsition data */
          status = mbr_kemkmall_rd_spo(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SKM:
            /* #SKM - KM binary sensor data */
          status = mbr_kemkmall_rd_skm(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SVP:
            /* #SVP - Sound Velocity Profile */
          status = mbr_kemkmall_rd_svp(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SVT:
          /* #SVT - Sensor sound Velocity measured at Transducer */
          status = mbr_kemkmall_rd_svt(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SCL:
            /* #SCL - Sensor CLock datagram */
          status = mbr_kemkmall_rd_scl(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SDE:
          /* #SDE - Sensor DEpth data */
          status = mbr_kemkmall_rd_sde(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SHI:
          /* #SHI - Sensor HeIght data */
          status = mbr_kemkmall_rd_shi(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case SHA:
          /* #SHA - Sensor HeAding */
          status = mbr_kemkmall_rd_sha(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case MRZ:
          /* #MRZ - multibeam data for raw range, depth, reflectivity, seabed image(SI) etc. */
          /*        not done until all MRZ datagrams for a ping are read */
          status = mbr_kemkmall_rd_mrz(verbose, buffer, store_ptr, (void *)&header, &jmrz, error);
//fprintf(stderr, "----------->%s:%d PARSE store->mrz[%d].header.numBytesDgm:%d \n",
//__FILE__, __LINE__, jmrz, store->mrz[jmrz].header.numBytesDgm);

          /* check to see if done */
          if (status != MB_SUCCESS) {
            done = false;
          } else {
            done = true;
            store->n_mrz_read = 0;
            store->n_mrz_needed = store->mrz[jmrz].cmnPart.rxFansPerPing;
            for (int imrz = 0; imrz < store->n_mrz_needed; imrz++) {
              if (store->mrz[imrz].cmnPart.pingCnt != store->mrz[jmrz].cmnPart.pingCnt) {
                done = false;
              }
              else {
                store->n_mrz_read++;
              }
            }
            if (store->n_mrz_read != store->n_mrz_needed)
              done = false;
            store->num_soundings = 0;
            store->num_backscatter_samples = 0;
            store->num_pixels = 0;
            if (done) {
              for (int imrz=0;imrz<store->n_mrz_needed;imrz++) {
                numSoundings = store->mrz[imrz].rxInfo.numSoundingsMaxMain
                                + store->mrz[imrz].rxInfo.numExtraDetections;
                numBackscatterSamples = 0;
                for (isounding=0;isounding<numSoundings;isounding++) {
                  numBackscatterSamples += store->mrz[imrz].sounding[isounding].SInumSamples;
                }
                store->num_soundings += numSoundings;
                store->num_backscatter_samples += numBackscatterSamples;
              }
            }
          }

          /* if mwc datagrams are expected then not done yet */
          if (done && store->xmb.watercolumn) {
            if (store->n_mwc_read > 0 && store->n_mwc_read == store->n_mwc_needed
                && store->mwc[jmrz].cmnPart.pingCnt == store->mrz[jmrz].cmnPart.pingCnt) {
              done = true;
            } else {
              done = false;
            }
          }

          /* if xmt and xms datagrams are expected then not done yet (xmt and xms come after mrz) */
          if (done && store->xmb.mbsystem_extensions) {
            done = false;
          }
          break;

        case MWC:
          /* #MWC - multibeam water column datagram */
          /*        not done until all MRZ datagrams for a ping are read */
          status = mbr_kemkmall_rd_mwc(verbose, buffer, store_ptr, (void *)&header, &jmwc, error);

          /* if MWC set flag indicating water column records are present */
          if (status == MB_SUCCESS) {
            store->xmb.watercolumn = 1;
          }

          /* check to see if done - first check if mrz datagrams have all been read */
          if (store->n_mrz_read > 0 && store->n_mrz_read == store->n_mrz_needed
              && store->mrz[jmwc].cmnPart.pingCnt == store->mwc[jmwc].cmnPart.pingCnt) {
            done = true;
          } else {
            done = false;
          }

          /* check if mwc datagrams have all been read */
          store->n_mwc_read = 0;
          store->n_mwc_needed = dgm_index->rx_per_ping;
          for (int imwc=0;imwc<dgm_index->rx_per_ping;imwc++) {
            if (store->mwc[imwc].cmnPart.pingCnt == store->mwc[jmwc].cmnPart.pingCnt) {
              store->n_mwc_read++;
            } else {
              done = false;
            }
          }
          if (done && store->n_mwc_read != store->n_mwc_needed) {
            done = false;
          }

          /* if xmt and xms datagrams are expected but the xms has not been read,
              then not done yet */
          if (done && store->xmb.mbsystem_extensions) {
            done = false;
          }
          break;

        case CPO:
          /* #CPO - Compatibility position sensor data */
          status = mbr_kemkmall_rd_cpo(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case CHE:
          /* #CHE - Compatibility heave data */
          status = mbr_kemkmall_rd_che(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case FCF:
          /* #FCF - Backscatter calibration file */
          status = mbr_kemkmall_rd_fcf(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case XMB:
          /* #XMB - Indicates these data were written by MB-System (MB-System only)
              also indicates presence of water column datagrams */
          status = mbr_kemkmall_rd_xmb(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case XMC:
          /* #XMC - Comment datagram (MB-System only) */
          status = mbr_kemkmall_rd_xmc(verbose, buffer, store_ptr, (void *)&header, error);
          if (status == MB_SUCCESS)
            done = true;
          break;

        case XMT:
          /* #XMT - multibeam corrected beam angles and travel times (MB-System only) */
          /* Note: if XMT datagrams exist then these data were written by MB-System and
             the ping datagrams will be ordered all MRZ, then all MWC, then all XMT, and
             finally the single XMS - therefore the ping cannot be done with this datagram as the
             XMS datagram is still to come */
          status = mbr_kemkmall_rd_xmt(verbose, buffer, store_ptr, (void *)&header, &jxmt, error);
//fprintf(stderr, "----------->%s:%d PARSE store->xmt[%d].header.numBytesDgm:%d \n",
//__FILE__, __LINE__, jxmt, store->xmt[jxmt].header.numBytesDgm);
          done = false;
          break;

        case XMS:
          /* #XMS - MB-System multibeam pseudosidescan */
          /* Note: if XMS datagrams exist then these data were written by MB-System and
             the ping datagrams will be ordered all MRZ, then all MWC, then all XMT, and
             finally the single XMS - therefore the ping will always be completed with
             this datagram */
          status = mbr_kemkmall_rd_xms(verbose, buffer, store_ptr, (void *)&header, error);
          if (status != MB_SUCCESS) {
            done = false;
          } else {
            if (store->n_mrz_read > 0 && store->n_mrz_read == store->n_mrz_needed
                && store->mrz[0].cmnPart.pingCnt == store->xms.pingCnt) {
              done = true;
            } else {
              done = false;
            }
          }
          break;

        case UNKNOWN:
          /* Unknown datagram format */
          status = mbr_kemkmall_rd_unknown(verbose, buffer, store_ptr, (void *)&header, error); // TODO: implement!
          if (status == MB_SUCCESS)
            done = true;
          break;

        default:
          /* should never get here */
          status = MB_FAILURE;
          done = true;
          break;

      }
    }

    /* set done if read failure */
    else {
      done = true;
    }

    /* increment the index counter */
    if (mb_io_ptr->mbfp != NULL)
      (*dgm_id)++;

    /* if not done but no more data in index then done with error */
    if (!done && mb_io_ptr->mbfp != NULL && *dgm_id >= dgm_index_table->dgm_count) {
      done = true;
      *error = MB_ERROR_EOF;
      status = MB_FAILURE;
    }

  }

  /* get file position */
  if (mb_io_ptr->mbfp != NULL)
    mb_io_ptr->file_bytes = ftell(mb_io_ptr->mbfp);

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointers to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get saved values */
  int *file_indexed = (int *)&mb_io_ptr->save2;
  int *nav_saved = (int *)&mb_io_ptr->save3;
  int *heading_saved = (int *)&mb_io_ptr->save4;
  int *attitude_saved = (int *)&mb_io_ptr->save5;
  int *sensordepth_saved = (int *)&mb_io_ptr->save6;

  int status = MB_SUCCESS;

  /* if reading from a file that has not been indexed, index the file */
  if (!*file_indexed && mb_io_ptr->mbfp != NULL) {
#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "About to call mbr_kemkmall_index_data...\n");
#endif
    status = mbr_kemkmall_index_data(verbose, mbio_ptr, store_ptr, error);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "\nAbout to call mbr_kemkmall_rd_data...\n");
#endif

  /* read next data from file */
  status = mbr_kemkmall_rd_data(verbose, mbio_ptr, store_ptr, error);

  /* get pointers to data structures */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  // deal with buffering asynchronous data if status == MB_SUCCESS
  if (status == MB_SUCCESS) {
    if (store->kind == MB_DATA_NAV) {
      struct mbsys_kmbes_spo *spo = &store->spo;
      double spo_time_d;
      if (*nav_saved == MB_DATA_NONE || *nav_saved == MB_DATA_NAV || *nav_saved == MB_DATA_NAV2) {
        spo_time_d = spo->sensorData.timeFromSensor_sec + 0.000000001 * spo->sensorData.timeFromSensor_nanosec;
        if (*nav_saved != MB_DATA_NAV) {
          mb_io_ptr->nfix = 0;
          *nav_saved = MB_DATA_NAV;
        }
        mb_navint_add(verbose, mbio_ptr, spo_time_d, spo->sensorData.correctedLong_deg,
                      spo->sensorData.correctedLat_deg, error);
      }
    }
    else if (store->kind == MB_DATA_NAV1) {
      struct mbsys_kmbes_skm *skm = &store->skm;
      double skm_time_d = 0.0;
      double skm_heave = 0.0;
      if (skm->infoPart.sensorDataContents & 0x00000001) {
        for (int i=0; i<skm->infoPart.numSamplesArray; i++) {
          skm_time_d = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
          if (!(skm->sample[i].KMdefault.status & 0x00000001)) {
            if (*nav_saved != MB_DATA_NAV1) {
              mb_io_ptr->nfix = 0;
              *nav_saved = MB_DATA_NAV1;
            }
            mb_navint_add(verbose, mbio_ptr, skm_time_d, skm->sample[i].KMdefault.longitude_deg,
                        skm->sample[i].KMdefault.latitude_deg, error);
          }
        }
      }
      if (skm->infoPart.sensorDataContents & 0x00000002) {
        for (int i=0; i<skm->infoPart.numSamplesArray; i++) {
          skm_time_d = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
          if (!(skm->sample[i].KMdefault.status & 0x00000008)) {
            skm_heave = -skm->sample[i].KMdefault.heave_m;
          } else {
            skm_heave = 0.0;
          }
          if (!(skm->sample[i].KMdefault.status & 0x00000002)) {
            if (*attitude_saved != MB_DATA_NAV1) {
              mb_io_ptr->nattitude = 0;
              *attitude_saved = MB_DATA_NAV1;
            }
			      mb_attint_add(verbose, mbio_ptr, skm_time_d, skm_heave,
                        skm->sample[i].KMdefault.roll_deg,
                        skm->sample[i].KMdefault.pitch_deg, error);
          }
        }
      }
      if (skm->infoPart.sensorDataContents & 0x00000004) {
        for (int i=0; i<skm->infoPart.numSamplesArray; i++) {
          skm_time_d = skm->sample[i].KMdefault.time_sec + 0.000000001 * skm->sample[i].KMdefault.time_nanosec;
          if (!(skm->sample[i].KMdefault.status & 0x00000004)) {
            if (*heading_saved != MB_DATA_NAV1) {
              mb_io_ptr->nheading = 0;
              *heading_saved = MB_DATA_NAV1;
            }
			      mb_hedint_add(verbose, mbio_ptr, skm_time_d, skm->sample[i].KMdefault.heading_deg, error);
          }
        }
      }
      // TODO: deal with delayed heave - needs buffer functions separate from roll and pitch
      //if (skm->infoPart.sensorDataContents & 0x00000040) {
      //  for (int i=0; i<skm->infoPart.numSamplesArray; i++) {
      //    skm_time_d = skm->sample[i].delayedHeave.time_sec + 0.000000001 * skm->sample[i].delayedHeave.time_nanosec;
      //    if (skm->sample[i].delayedHeave.status & 0x00000040) {
			//      mb_heaint_add(verbose, mbio_ptr, skm_time_d, (double)skm->sample[i].delayedHeave.delayedHeave_m, error);
      //    }
      //  }
      //}
    }
    else if (store->kind == MB_DATA_NAV2) {
      struct mbsys_kmbes_cpo *cpo = &store->cpo;
      if (*nav_saved == MB_DATA_NONE) {
        *nav_saved = MB_DATA_NAV2;
      }
      if (*nav_saved == MB_DATA_NAV2) {
        double cpo_time_d = cpo->sensorData.timeFromSensor_sec + 0.000000001 * cpo->sensorData.timeFromSensor_nanosec;
        mb_navint_add(verbose, mbio_ptr, cpo_time_d, cpo->sensorData.correctedLong_deg,
                      cpo->sensorData.correctedLat_deg, error);
      }
    }
    else if (store->kind == MB_DATA_SENSORDEPTH) {
      struct mbsys_kmbes_sde *sde = &store->sde;
      if (*sensordepth_saved == MB_DATA_NONE) {
        *sensordepth_saved = MB_DATA_SENSORDEPTH;
      }
      if (*sensordepth_saved == MB_DATA_SENSORDEPTH) {
        const double sde_time_d = sde->header.time_sec + 0.000000001 * sde->header.time_nanosec;
        mb_depint_add(verbose, mbio_ptr, sde_time_d, sde->sensorData.depthUsed_m, error);
      }
    }
    else if (store->kind == MB_DATA_HEADING) {
      struct mbsys_kmbes_sha *sha  = &store->sha;
      if (*heading_saved == MB_DATA_NONE) {
        *heading_saved = MB_DATA_HEADING;
      }
      if (*heading_saved == MB_DATA_HEADING) {
        const double sha_time_d = sha->header.time_sec + 0.000000001 * sha->header.time_nanosec;
        double sha_sample_time_d;
        for (int i=0; i<sha->dataInfo.numSamplesArray; i++) {
          sha_sample_time_d = sha_time_d + 0.000000001 * sha->sensorData[i].timeSinceRecStart_nanosec;
          mb_hedint_add(verbose, mbio_ptr, sha_sample_time_d, sha->sensorData[i].headingCorrected_deg, error);

        }
      }
    }
  }

  /* if this is a ping and has been processed and written by an old version of MB-System
     then if the data are on a submerged platform (mrz->pingInfo.txTransducerDepth_m > 10.0 m)
     then copy the xducer depth to the z_waterLevelReRefPoint_m, as that is used now
     for the overall depth calculation */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA
      && store->xmb.mbsystem_extensions && store->xmb.header.dgmVersion == 0) {
    for (int imrz = 0; imrz < store->n_mrz_read; imrz++) {
      struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];
      if (mrz->pingInfo.txTransducerDepth_m > 10.0) {
        mrz->pingInfo.z_waterLevelReRefPoint_m = -mrz->pingInfo.txTransducerDepth_m;
      }
    }
  }

  /* if this is a ping and the first time read by MB-System fill in xmt structure
     and generate pseudosidescan */
  if (status == MB_SUCCESS && store->kind == MB_DATA_DATA
      && !store->xmb.mbsystem_extensions) {

    /* set preprocess parameters */
    struct mb_preprocess_struct *preprocess_pars_ptr = &mb_io_ptr->preprocess_pars;
    preprocess_pars_ptr->target_sensor = 0;
    preprocess_pars_ptr->timestamp_changed = false;
    preprocess_pars_ptr->time_d = 0.0;
    preprocess_pars_ptr->n_nav = mb_io_ptr->nfix;
    preprocess_pars_ptr->nav_time_d = mb_io_ptr->fix_time_d;
    preprocess_pars_ptr->nav_lon = mb_io_ptr->fix_lon;
    preprocess_pars_ptr->nav_lat = mb_io_ptr->fix_lat;
    preprocess_pars_ptr->nav_speed = NULL;
    preprocess_pars_ptr->n_sensordepth = mb_io_ptr->nsensordepth;
    preprocess_pars_ptr->sensordepth_time_d = mb_io_ptr->sensordepth_time_d;
    preprocess_pars_ptr->sensordepth_sensordepth = mb_io_ptr->sensordepth_sensordepth;
    preprocess_pars_ptr->n_heading = mb_io_ptr->nheading;
    preprocess_pars_ptr->heading_time_d = mb_io_ptr->heading_time_d;
    preprocess_pars_ptr->heading_heading = mb_io_ptr->heading_heading;
    preprocess_pars_ptr->n_altitude = mb_io_ptr->naltitude;
    preprocess_pars_ptr->altitude_time_d = mb_io_ptr->altitude_time_d;
    preprocess_pars_ptr->altitude_altitude = mb_io_ptr->altitude_altitude;
    preprocess_pars_ptr->n_attitude = mb_io_ptr->nattitude;
    preprocess_pars_ptr->attitude_time_d = mb_io_ptr->attitude_time_d;
    preprocess_pars_ptr->attitude_roll = mb_io_ptr->attitude_roll;
    preprocess_pars_ptr->attitude_pitch = mb_io_ptr->attitude_pitch;
    preprocess_pars_ptr->attitude_heave = mb_io_ptr->attitude_heave;
    preprocess_pars_ptr->n_soundspeed = 0;
    preprocess_pars_ptr->soundspeed_time_d = NULL;
    preprocess_pars_ptr->soundspeed_soundspeed = NULL;
    preprocess_pars_ptr->no_change_survey = false;
    preprocess_pars_ptr->multibeam_sidescan_source = MB_PR_SSSOURCE_SNIPPET;
    preprocess_pars_ptr->modify_soundspeed = false;
    preprocess_pars_ptr->recalculate_bathymetry = false;
    preprocess_pars_ptr->sounding_amplitude_filter = false;
    preprocess_pars_ptr->sounding_amplitude_threshold = 0.0;
    preprocess_pars_ptr->sounding_altitude_filter = false;
    preprocess_pars_ptr->sounding_target_altitude = 0.0;
    preprocess_pars_ptr->ignore_water_column = false;
    preprocess_pars_ptr->head1_offsets = false;
    preprocess_pars_ptr->head1_offsets_x = 0.0;
    preprocess_pars_ptr->head1_offsets_y = 0.0;
    preprocess_pars_ptr->head1_offsets_z = 0.0;
    preprocess_pars_ptr->head1_offsets_heading = 0.0;
    preprocess_pars_ptr->head1_offsets_roll = 0.0;
    preprocess_pars_ptr->head1_offsets_pitch = 0.0;
    preprocess_pars_ptr->head2_offsets = false;
    preprocess_pars_ptr->head2_offsets_x = 0.0;
    preprocess_pars_ptr->head2_offsets_y = 0.0;
    preprocess_pars_ptr->head2_offsets_z = 0.0;
    preprocess_pars_ptr->head2_offsets_heading = 0.0;
    preprocess_pars_ptr->head2_offsets_roll = 0.0;
    preprocess_pars_ptr->head2_offsets_pitch = 0.0;
    preprocess_pars_ptr->n_kluge = 0;

    // call the preprocess routine
    //  - this will fill in information for xmt record and generate pseudosidescan
    status = mbsys_kmbes_preprocess(verbose, mbio_ptr, store_ptr, NULL,
                                    preprocess_pars_ptr, error);
  }

  /* set error and kind in mb_io_ptr */
  mb_io_ptr->new_error = *error;
  mb_io_ptr->new_kind = store->kind;

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "Done with mbr_kemkmall_rd_data: status:%d error:%d kind:%d\n", status, *error, store->kind);
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}
/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_header(int verbose, char **bufferptr, void *header_ptr, int *error) {
  struct mbsys_kmbes_header *header = NULL;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       header_ptr: %p\n", (void *)header_ptr);
  }

  /* get pointer to raw data structure */
  header = (struct mbsys_kmbes_header *)header_ptr;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", header->numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", header->dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", header->dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", header->systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", header->echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", header->time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", header->time_nanosec);
  }

  int status = MB_SUCCESS;

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the data */
    index = 0;

    mb_put_binary_int(true, header->numBytesDgm, &buffer[index]);
    index += 4;
    memcpy(&buffer[index], &(header->dgmType), sizeof(header->dgmType));
    index += 4;
    buffer[index] = header->dgmVersion;
    index++;
    buffer[index] = header->systemID;
    index++;
    mb_put_binary_short(true, header->echoSounderID, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, header->time_sec, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, header->time_nanosec, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_spo(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error){
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  unsigned int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_spo *spo = &(store->spo);

  /* datagram version being written */
  spo->header.dgmVersion = MBSYS_KMBES_SPO_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                 %u\n", spo->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                     %s\n", spo->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                  %u\n", spo->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                    %u\n", spo->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:               %u\n", spo->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                    %u\n", spo->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:                %u\n", spo->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:             %u\n", spo->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:                %u\n", spo->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:                %u\n", spo->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                     %u\n", spo->cmnPart.padding);

    fprintf(stderr, "dbg5       timeFromSensor_sec:          %u\n", spo->sensorData.timeFromSensor_sec);
    fprintf(stderr, "dbg5       timeFromSensor_nanosec:      %u\n", spo->sensorData.timeFromSensor_nanosec);
    fprintf(stderr, "dbg5       posFixQuality_m:             %f\n", spo->sensorData.posFixQuality_m);
    fprintf(stderr, "dbg5       correctedLat_deg:            %f\n", spo->sensorData.correctedLat_deg);
    fprintf(stderr, "dbg5       correctedLong_deg:           %f\n", spo->sensorData.correctedLong_deg);
    fprintf(stderr, "dbg5       speedOverGround_mPerSec:     %f\n", spo->sensorData.speedOverGround_mPerSec);
    fprintf(stderr, "dbg5       courseOverGround_deg:        %f\n", spo->sensorData.courseOverGround_deg);
    fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m: %f\n", spo->sensorData.ellipsoidHeightReRefPoint_m);
    fprintf(stderr, "dbg5       posDataFromSensor:           %s\n", spo->sensorData.posDataFromSensor);
  }

  /* size of output record */
  *size = (size_t) spo->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *)*bufferptr;

    /* calc number of bytes for raw sensor data */
    numBytesRawSensorData = spo->header.numBytesDgm - MBSYS_KMBES_SPO_VAR_OFFSET;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&(spo->header), error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, spo->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, spo->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, spo->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, spo->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor data block */
    mb_put_binary_int(true, spo->sensorData.timeFromSensor_sec, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, spo->sensorData.timeFromSensor_nanosec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, spo->sensorData.posFixQuality_m, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, spo->sensorData.correctedLat_deg, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, spo->sensorData.correctedLong_deg, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, spo->sensorData.speedOverGround_mPerSec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, spo->sensorData.courseOverGround_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, spo->sensorData.ellipsoidHeightReRefPoint_m, &buffer[index]);
    index += 4;

    /* raw data msg from sensor */
    memcpy(&buffer[index], &(spo->sensorData.posDataFromSensor), numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* insert closing byte count */
    mb_put_binary_int(true, spo->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          spo->header.dgmType, spo->header.time_sec, spo->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_skm(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error){
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_skm *skm = &(store->skm);

  /* datagram version being written */
  skm->header.dgmVersion = MBSYS_KMBES_SKM_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                %u\n", skm->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                    %s\n", skm->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                 %u\n", skm->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                   %u\n", skm->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:              %u\n", skm->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                   %u\n", skm->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:               %u\n", skm->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesInfoPart:           %u\n", skm->infoPart.numBytesInfoPart);
    fprintf(stderr, "dbg5       sensorSystem:               %u\n", skm->infoPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:               %u\n", skm->infoPart.sensorStatus);
    fprintf(stderr, "dbg5       sensorInputFormat:          %u\n", skm->infoPart.sensorInputFormat);
    fprintf(stderr, "dbg5       numSamplesArray:            %u\n", skm->infoPart.numSamplesArray);
    fprintf(stderr, "dbg5       numBytesPerSample:          %u\n", skm->infoPart.numBytesPerSample);
    fprintf(stderr, "dbg5       sensorDataContents:         %u\n", skm->infoPart.sensorDataContents);

    for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.dgmType:                %s\n", i, skm->sample[i].KMdefault.dgmType);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.numBytesDgm:            %u\n", i, skm->sample[i].KMdefault.numBytesDgm);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.dgmVersion:             %u\n", i, skm->sample[i].KMdefault.dgmVersion);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.time_sec:               %u\n", i, skm->sample[i].KMdefault.time_sec);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.time_nanosec:           %u\n", i, skm->sample[i].KMdefault.time_nanosec);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.status:                 %u\n", i, skm->sample[i].KMdefault.status);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.latitude_deg:           %f\n", i, skm->sample[i].KMdefault.latitude_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.longitude_deg:          %f\n", i, skm->sample[i].KMdefault.longitude_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.ellipsoidHeight_m:      %f\n", i, skm->sample[i].KMdefault.ellipsoidHeight_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.roll_deg:               %f\n", i, skm->sample[i].KMdefault.roll_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitch_deg:              %f\n", i, skm->sample[i].KMdefault.pitch_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heading_deg:            %f\n", i, skm->sample[i].KMdefault.heading_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heave_m:                %f\n", i, skm->sample[i].KMdefault.heave_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.rollRate:               %f\n", i, skm->sample[i].KMdefault.rollRate);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitchRate:              %f\n", i, skm->sample[i].KMdefault.pitchRate);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.yawRate:                %f\n", i, skm->sample[i].KMdefault.yawRate);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velNorth:               %f\n", i, skm->sample[i].KMdefault.velNorth);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velEast:                %f\n", i, skm->sample[i].KMdefault.velEast);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.velDown:                %f\n", i, skm->sample[i].KMdefault.velDown);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.latitudeError_m:        %f\n", i, skm->sample[i].KMdefault.latitudeError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.longitudeError_m:       %f\n", i, skm->sample[i].KMdefault.longitudeError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.ellipsoidHeightError_m: %f\n", i, skm->sample[i].KMdefault.ellipsoidHeightError_m);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.rollError_deg:          %f\n", i, skm->sample[i].KMdefault.rollError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.pitchError_deg:         %f\n", i, skm->sample[i].KMdefault.pitchError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.headingError_deg:       %f\n", i, skm->sample[i].KMdefault.headingError_deg);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.heaveError_m:           %f\n", i, skm->sample[i].KMdefault.heaveError_m);

      fprintf(stderr, "dbg5       sample[%3d].KMdefault.northAcceleration:      %f\n", i, skm->sample[i].KMdefault.northAcceleration);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.eastAcceleration:       %f\n", i, skm->sample[i].KMdefault.eastAcceleration);
      fprintf(stderr, "dbg5       sample[%3d].KMdefault.downAcceleration:       %f\n", i, skm->sample[i].KMdefault.downAcceleration);

      //
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.time_sec:            %u\n", i, skm->sample[i].delayedHeave.time_sec);
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.time_nanosec:        %u\n", i, skm->sample[i].delayedHeave.time_nanosec);
      fprintf(stderr, "dbg5       sample[%3d].delayedHeave.delayedHeave_m:      %f\n", i, skm->sample[i].delayedHeave.delayedHeave_m);
    }

  }

  /* size of output record */
  *size = (size_t) skm->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&skm->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* info part */
    mb_put_binary_short(true, skm->infoPart.numBytesInfoPart, &buffer[index]);
    index += 2;
    buffer[index] = skm->infoPart.sensorSystem;
    index++;
    buffer[index] = skm->infoPart.sensorStatus;
    index++;
    mb_put_binary_short(true, skm->infoPart.sensorInputFormat, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, skm->infoPart.numSamplesArray, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, skm->infoPart.numBytesPerSample, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, skm->infoPart.sensorDataContents, &buffer[index]);
    index += 2;

    for (int i=0; i<(skm->infoPart.numSamplesArray); i++ ) {

      /* KMbinary */
      memcpy(&buffer[index], &(skm->sample[i].KMdefault.dgmType), 4);
      index += 4;
      mb_put_binary_short(true, skm->sample[i].KMdefault.numBytesDgm, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, skm->sample[i].KMdefault.dgmVersion, &buffer[index]);
      index += 2;
      mb_put_binary_int(true, skm->sample[i].KMdefault.time_sec, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, skm->sample[i].KMdefault.time_nanosec, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, skm->sample[i].KMdefault.status, &buffer[index]);
      index += 4;
      mb_put_binary_double(true, skm->sample[i].KMdefault.latitude_deg, &buffer[index]);
      index += 8;
      mb_put_binary_double(true, skm->sample[i].KMdefault.longitude_deg, &buffer[index]);
      index += 8;
      mb_put_binary_float(true, skm->sample[i].KMdefault.ellipsoidHeight_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.roll_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.pitch_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.heading_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.heave_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.rollRate, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.pitchRate, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.yawRate, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.velNorth, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.velEast, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.velDown, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.latitudeError_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.longitudeError_m, &buffer[index]);
      index += 4;
            mb_put_binary_float(true, skm->sample[i].KMdefault.ellipsoidHeightError_m, &buffer[index]);
            index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.rollError_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.pitchError_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.headingError_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.heaveError_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.northAcceleration, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.eastAcceleration, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].KMdefault.downAcceleration, &buffer[index]);
      index += 4;

      /* KMdelayedHeave */
      mb_put_binary_int(true, skm->sample[i].delayedHeave.time_sec, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, skm->sample[i].delayedHeave.time_nanosec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, skm->sample[i].delayedHeave.delayedHeave_m, &buffer[index]);
      index += 4;
    }

    /* insert closing byte count */
    mb_put_binary_int(true, skm->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          skm->header.dgmType, skm->header.time_sec, skm->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_svp(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_svp *svp = &(store->svp);

  /* datagram version being written */
  svp->header.dgmVersion = MBSYS_KMBES_SVP_VERSION;
  svp->numBytesCmnPart = 28;
  svp->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + svp->numBytesCmnPart + svp->numSamples * 20 + sizeof(int);

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:     %u\n", svp->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:         %.4s\n", svp->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:      %u\n", svp->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:        %u\n", svp->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:   %u\n", svp->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:        %u\n", svp->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:    %u\n", svp->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", svp->numBytesCmnPart);
    fprintf(stderr, "dbg5       numSamples:       %u\n", svp->numSamples);
    fprintf(stderr, "dbg5       sensorFormat:     %s\n", svp->sensorFormat);
    fprintf(stderr, "dbg5       time_sec:         %u\n", svp->time_sec);
    fprintf(stderr, "dbg5       latitude_deg:     %f\n", svp->latitude_deg);
    fprintf(stderr, "dbg5       longitude_deg:    %f\n", svp->longitude_deg);

    for (int i = 0; i < (svp->numSamples); i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].depth_m:                %f\n", i, svp->sensorData[i].depth_m);
      fprintf(stderr, "dbg5       sensorData[%3d].soundVelocity_mPerSec:  %f\n", i, svp->sensorData[i].soundVelocity_mPerSec);
      fprintf(stderr, "dbg5       sensorData[%3d].padding:                %d\n", i, svp->sensorData[i].padding);
      fprintf(stderr, "dbg5       sensorData[%3d].temp_C:                 %f\n", i, svp->sensorData[i].temp_C);
      fprintf(stderr, "dbg5       sensorData[%3d].salinity:               %f\n", i, svp->sensorData[i].salinity);
    }
  }

  /* size of output record */
  *size = (size_t) svp->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&svp->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* svp common part */
    mb_put_binary_short(true, svp->numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svp->numSamples, &buffer[index]);
    index += 2;
    memcpy(&buffer[index], &svp->sensorFormat, 4);
    index += 4;
    mb_put_binary_int(true, svp->time_sec, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, svp->latitude_deg, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, svp->longitude_deg, &buffer[index]);
    index += 8;

    /* svp data block */
    for (int i = 0; i < (svp->numSamples); i++) {
      mb_put_binary_float(true, svp->sensorData[i].depth_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svp->sensorData[i].soundVelocity_mPerSec, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, svp->sensorData[i].padding, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svp->sensorData[i].temp_C, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svp->sensorData[i].salinity, &buffer[index]);
      index += 4;
    }

    /* insert closing byte count */
    mb_put_binary_int(true, svp->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          svp->header.dgmType, svp->header.time_sec, svp->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_svt(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_svt *svt = &(store->svt);

  /* datagram version being written */
  svt->header.dgmVersion = MBSYS_KMBES_SVT_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:              %u\n", svt->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                  %s\n", svt->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:               %u\n", svt->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                 %u\n", svt->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:            %u\n", svt->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                 %u\n", svt->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:             %u\n", svt->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesInfoPart:         %u\n", svt->infoPart.numBytesInfoPart);
    fprintf(stderr, "dbg5       sensorStatus:             %u\n", svt->infoPart.sensorStatus);
    fprintf(stderr, "dbg5       sensorInputFormat:        %u\n", svt->infoPart.sensorInputFormat);
    fprintf(stderr, "dbg5       numSamplesArray:          %u\n", svt->infoPart.numSamplesArray);
    fprintf(stderr, "dbg5       sensorDataContents:       %u\n", svt->infoPart.sensorDataContents);
    fprintf(stderr, "dbg5       filterTime_sec:           %f\n", svt->infoPart.filterTime_sec);
    fprintf(stderr, "dbg5       soundVelocity_mPerSec_offset: %f\n", svt->infoPart.soundVelocity_mPerSec_offset);

    for (int i = 0; i < (svt->infoPart.numSamplesArray); i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].time_sec:               %u\n", i, svt->sensorData[i].time_sec);
      fprintf(stderr, "dbg5       sensorData[%3d].time_nanosec:           %u\n", i, svt->sensorData[i].time_nanosec);
      fprintf(stderr, "dbg5       sensorData[%3d].soundVelocity_mPerSec:  %f\n", i, svt->sensorData[i].soundVelocity_mPerSec);
      fprintf(stderr, "dbg5       sensorData[%3d].temp_C:                 %f\n", i, svt->sensorData[i].temp_C);
      fprintf(stderr, "dbg5       sensorData[%3d].pressure_Pa:            %f\n", i, svt->sensorData[i].pressure_Pa);
      fprintf(stderr, "dbg5       sensorData[%3d].salinity:               %f\n", i, svt->sensorData[i].salinity);
    }
  }

  /* size of output record */
  *size = (size_t) svt->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&svt->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* svt common part */
    mb_put_binary_short(true, svt->infoPart.numBytesInfoPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svt->infoPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svt->infoPart.sensorInputFormat, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svt->infoPart.numSamplesArray, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svt->infoPart.numBytesPerSample, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, svt->infoPart.sensorDataContents, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, svt->infoPart.filterTime_sec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, svt->infoPart.soundVelocity_mPerSec_offset, &buffer[index]);
    index += 4;

    /* svt data block */
    for (int i = 0; i<svt->infoPart.numSamplesArray; i++ )
    {
      mb_put_binary_int(true, svt->sensorData[i].time_sec, &buffer[index]);
      index += 4;
      mb_put_binary_int(true, svt->sensorData[i].time_nanosec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svt->sensorData[i].soundVelocity_mPerSec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svt->sensorData[i].temp_C, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svt->sensorData[i].pressure_Pa, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, svt->sensorData[i].salinity, &buffer[index]);
      index += 4;
    }

    /* insert closing byte count */
    mb_put_binary_int(true, svt->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          svt->header.dgmType, svt->header.time_sec, svt->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_scl(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_scl *scl = &(store->scl);

  /* datagram version being written */
  scl->header.dgmVersion = MBSYS_KMBES_SCL_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:         %u\n", scl->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:             %s\n", scl->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:          %u\n", scl->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:            %u\n", scl->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:       %u\n", scl->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:            %u\n", scl->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:        %u\n", scl->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:     %u\n", scl->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:        %u\n", scl->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:        %u\n", scl->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:             %u\n", scl->cmnPart.padding);

    fprintf(stderr, "dbg5       offset_sec:          %f\n", scl->sensorData.offset_sec);
    fprintf(stderr, "dbg5       clockDevPU_nanosec:  %d\n", scl->sensorData.clockDevPU_nanosec);
    fprintf(stderr, "dbg5       dataFromSensor:      %s\n", scl->sensorData.dataFromSensor);
  }

  /* size of output record */
  *size = (size_t) scl->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&scl->header, error);

    /* calc number of bytes for raw sensor data */
    numBytesRawSensorData = scl->header.numBytesDgm - MBSYS_KMBES_SCL_VAR_OFFSET;

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, scl->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, scl->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, scl->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, scl->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor data block */
    mb_put_binary_float(true, scl->sensorData.offset_sec, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, scl->sensorData.clockDevPU_nanosec, &buffer[index]);
    index += 4;
    memcpy(&buffer[index], &(scl->sensorData.dataFromSensor), numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* insert closing byte count */
    mb_put_binary_int(true, scl->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          scl->header.dgmType, scl->header.time_sec, scl->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_sde(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error){
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_sde *sde = &(store->sde);

  /* datagram version being written */
  sde->header.dgmVersion = MBSYS_KMBES_SDE_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", sde->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", sde->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", sde->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", sde->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", sde->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", sde->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", sde->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", sde->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:     %u\n", sde->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:     %u\n", sde->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:          %u\n", sde->cmnPart.padding);

    fprintf(stderr, "dbg5       depthUsed_m:      %f\n", sde->sensorData.depthUsed_m);
    fprintf(stderr, "dbg5       offset:           %f\n", sde->sensorData.offset);
    fprintf(stderr, "dbg5       scale:            %f\n", sde->sensorData.scale);
    fprintf(stderr, "dbg5       latitude_deg:     %f\n", sde->sensorData.latitude_deg);
    fprintf(stderr, "dbg5       longitude_deg:    %f\n", sde->sensorData.longitude_deg);
    fprintf(stderr, "dbg5       dataFromSensor:   %s\n", sde->sensorData.dataFromSensor);
  }

  /* size of output record */
  *size = (size_t) sde->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&sde->header, error);

    /* calc number of bytes for raw sensor data */
    numBytesRawSensorData = sde->header.numBytesDgm - MBSYS_KMBES_SDE_VAR_OFFSET;

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, sde->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sde->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sde->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sde->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor data block */
    mb_put_binary_float(true, sde->sensorData.depthUsed_m, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sde->sensorData.offset, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, sde->sensorData.scale, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, sde->sensorData.latitude_deg, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, sde->sensorData.longitude_deg, &buffer[index]);
    index += 8;
    memcpy(&(sde->sensorData.dataFromSensor), &buffer[index], numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* insert closing byte count */
    mb_put_binary_int(true, sde->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          sde->header.dgmType, sde->header.time_sec, sde->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_shi(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error){
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_shi *shi = &(store->shi);

  /* datagram version being written */
  shi->header.dgmVersion = MBSYS_KMBES_SHI_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", shi->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", shi->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", shi->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", shi->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", shi->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", shi->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", shi->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", shi->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:     %u\n", shi->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:     %u\n", shi->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:          %u\n", shi->cmnPart.padding);

    fprintf(stderr, "dbg5       sensorType:       %u\n", shi->sensorData.sensorType);
    fprintf(stderr, "dbg5       heigthUsed_m:     %f\n", shi->sensorData.heigthUsed_m);
    fprintf(stderr, "dbg5       dataFromSensor:   %s\n", shi->sensorData.dataFromSensor);
  }

  /* size of output record */
  *size = (size_t) shi->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&shi->header, error);

    /* calc number of bytes for raw sensor data */
    numBytesRawSensorData = shi->header.numBytesDgm - MBSYS_KMBES_SHI_VAR_OFFSET;

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, shi->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, shi->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, shi->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, shi->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor data block */
    mb_put_binary_short(true, shi->sensorData.sensorType, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, shi->sensorData.heigthUsed_m, &buffer[index]);
    index += 4;
    memcpy(&buffer[index], &(shi->sensorData.dataFromSensor), numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* insert closing byte count */
    mb_put_binary_int(true, shi->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - time: %d.%9.9d status:%d error:%d\n",
          shi->header.dgmType, shi->header.time_sec, shi->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_sha(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_sha *sha = &(store->sha);

  /* datagram version being written */
  sha->header.dgmVersion = MBSYS_KMBES_SHA_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:            %u\n", sha->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                %s\n", sha->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:             %u\n", sha->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:               %u\n", sha->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:          %u\n", sha->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:               %u\n", sha->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:           %u\n", sha->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:        %u\n", sha->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:           %u\n", sha->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:           %u\n", sha->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                %u\n", sha->cmnPart.padding);

    fprintf(stderr, "dbg5       numBytesInfoPart:       %u\n", sha->dataInfo.numBytesInfoPart);
    fprintf(stderr, "dbg5       numSamplesArray:        %u\n", sha->dataInfo.numSamplesArray);
    fprintf(stderr, "dbg5       numBytesPerSample:      %u\n", sha->dataInfo.numBytesPerSample);
    fprintf(stderr, "dbg5       numBytesRawSensorData:  %u\n", sha->dataInfo.numBytesRawSensorData);

    for (int i = 0; i < (sha->dataInfo.numSamplesArray); i++) {
      fprintf(stderr, "dbg5       sensorData[%3d].timeSinceRecStart_nanosec: %u\n", i, sha->sensorData[i].timeSinceRecStart_nanosec);
      fprintf(stderr, "dbg5       sensorData[%3d].headingCorrected_deg:      %f\n", i, sha->sensorData[i].headingCorrected_deg);
      fprintf(stderr, "dbg5       sensorData[%3d].dataFromSensor:            %s\n", i, sha->sensorData[i].dataFromSensor);
    }
  }

  /* size of output record */
  *size = (size_t) sha->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&sha->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, sha->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor info */
    mb_put_binary_short(true, sha->dataInfo.numBytesInfoPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->dataInfo.numSamplesArray, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->dataInfo.numBytesPerSample, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, sha->dataInfo.numBytesRawSensorData, &buffer[index]);
    index += 2;

    /* sensor data blocks */
    for (int i = 0; i<(sha->dataInfo.numSamplesArray); i++) {
      mb_put_binary_int(true, sha->sensorData[i].timeSinceRecStart_nanosec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, sha->sensorData[i].headingCorrected_deg, &buffer[index]);
      index += 4;
      memcpy(&buffer[index], &(sha->sensorData[i].dataFromSensor), sha->dataInfo.numBytesRawSensorData);
      index += sha->dataInfo.numBytesRawSensorData;
    }

    /* insert closing byte count */
    mb_put_binary_int(true, sha->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s read - time: %d.%9.9d status:%d error:%d\n",
          sha->header.dgmType, sha->header.time_sec, sha->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_mrz(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, int imrz, size_t *size, int *error) {
  char *buffer = NULL;
  int numSoundings = 0;
  int numSidescanSamples = 0;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       imrz:       %d\n", imrz);
  }

  /* get pointer to the data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_mrz *mrz = (struct mbsys_kmbes_mrz *)&store->mrz[imrz];

  /* datagram version being written */
  mrz->header.dgmVersion = MBSYS_KMBES_MRZ_VERSION;

  /* size of output record and components thereof - set the size according to what
      we know about, as anything added by Kongsberg that we don't know about will
      have been skipped while reading */
  mrz->cmnPart.numBytesCmnPart = 12;          // dgmVersion == 0 ==> 12
  mrz->pingInfo.numBytesInfoData = 152;       // dgmVersion == 0 ==> 144
  mrz->pingInfo.numBytesPerTxSector = 48;     // dgmVersion == 0 ==> 36
  mrz->rxInfo.numBytesRxInfo = 32;            // dgmVersion == 0 ==> 32
  mrz->rxInfo.numBytesPerClass = 4;           // dgmVersion == 0 ==> 4
  mrz->rxInfo.numBytesPerSounding = 120;      // dgmVersion == 0 ==> 120
  numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
  numSidescanSamples = 0;
  for (int i = 0; i<numSoundings; i++) {
    numSidescanSamples += mrz->sounding[i].SInumSamples;
  }
  mrz->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE
                            + mrz->cmnPart.numBytesCmnPart
                            + MBSYS_KMBES_PARITION_SIZE
                            + mrz->pingInfo.numBytesInfoData
                            + mrz->pingInfo.numTxSectors * mrz->pingInfo.numBytesPerTxSector
                            + mrz->rxInfo.numBytesRxInfo
                            + mrz->rxInfo.numExtraDetectionClasses * mrz->rxInfo.numBytesPerClass
                            + numSoundings * mrz->rxInfo.numBytesPerSounding
                            + numSidescanSamples * sizeof(short)
                            + MBSYS_KMBES_END_SIZE;

  *size = (size_t) mrz->header.numBytesDgm;
//fprintf(stderr, "************>%s:%d WRITE mbr_kemkmall_wr_mrz mrz->header.numBytesDgm:%d\n",
//__FILE__, __LINE__, mrz->header.numBytesDgm);

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesDgm:    %u\n", mrz->header.numBytesDgm);
      fprintf(stderr, "dbg5       dgmType:        %s\n", mrz->header.dgmType);
      fprintf(stderr, "dbg5       dgmVersion:     %u\n", mrz->header.dgmVersion);
      fprintf(stderr, "dbg5       systemID:       %u\n", mrz->header.systemID);
      fprintf(stderr, "dbg5       echoSounderID:  %u\n", mrz->header.echoSounderID);
      fprintf(stderr, "dbg5       time_sec:       %u\n", mrz->header.time_sec);
      fprintf(stderr, "dbg5       time_nanosec:   %u\n", mrz->header.time_nanosec);
    }

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&mrz->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* EMdgmMpartition - data partition information */
    mb_put_binary_short(true, mrz->partition.numOfDgms, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->partition.dgmNum, &buffer[index]);
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numOfDgms:  %d\n", mrz->partition.numOfDgms);
      fprintf(stderr, "dbg5       dgmNum:     %d\n", mrz->partition.dgmNum);
    }

    /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
    mb_put_binary_short(true, mrz->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->cmnPart.pingCnt, &buffer[index]);
    index += 2;
    buffer[index] = mrz->cmnPart.rxFansPerPing;
    index++;
    buffer[index] = mrz->cmnPart.rxFanIndex;
    index++;
    buffer[index] = mrz->cmnPart.swathsPerPing;
    index++;
    buffer[index] = mrz->cmnPart.swathAlongPosition;
    index++;
    buffer[index] = mrz->cmnPart.txTransducerInd;
    index++;
    buffer[index] = mrz->cmnPart.rxTransducerInd;
    index++;
    buffer[index] = mrz->cmnPart.numRxTransducers;
    index++;
    buffer[index] = mrz->cmnPart.algorithmType;
    index++;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", mrz->cmnPart.numBytesCmnPart);
      fprintf(stderr, "dbg5       pingCnt:             %d\n", mrz->cmnPart.pingCnt);
      fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", mrz->cmnPart.rxFansPerPing);
      fprintf(stderr, "dbg5       rxFanIndex:          %d\n", mrz->cmnPart.rxFanIndex);
      fprintf(stderr, "dbg5       swathsPerPing:       %d\n", mrz->cmnPart.swathsPerPing);
      fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", mrz->cmnPart.swathAlongPosition);
      fprintf(stderr, "dbg5       txTransducerInd:     %d\n", mrz->cmnPart.txTransducerInd);
      fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", mrz->cmnPart.rxTransducerInd);
      fprintf(stderr, "dbg5       numRxTransducers:    %d\n", mrz->cmnPart.numRxTransducers);
      fprintf(stderr, "dbg5       algorithmType:       %d\n", mrz->cmnPart.algorithmType);
    }

    /* EMdgmMRZ_pingInfo - ping info */
    mb_put_binary_short(true, mrz->pingInfo.numBytesInfoData, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->pingInfo.padding0, &buffer[index]);
    index += 2;

    /* ping info */
    mb_put_binary_float(true, mrz->pingInfo.pingRate_Hz, &buffer[index]);
    index += 4;

    buffer[index] = mrz->pingInfo.beamSpacing;
    index++;
    buffer[index] = mrz->pingInfo.depthMode;
    index++;
    buffer[index] = mrz->pingInfo.subDepthMode;
    index++;
    buffer[index] = mrz->pingInfo.distanceBtwSwath;
    index++;
    buffer[index] = mrz->pingInfo.detectionMode;
    index++;
    buffer[index] = mrz->pingInfo.pulseForm;
    index++;

    mb_put_binary_short(true, mrz->pingInfo.padding1, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, mrz->pingInfo.frequencyMode_Hz, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.freqRangeLowLim_Hz, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.freqRangeHighLim_Hz, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.maxTotalTxPulseLength_sec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.maxEffTxPulseLength_sec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.maxEffTxBandWidth_Hz, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.absCoeff_dBPerkm, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.portSectorEdge_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.starbSectorEdge_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.portMeanCov_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.starbMeanCov_deg, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, mrz->pingInfo.portMeanCov_m, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->pingInfo.starbMeanCov_m, &buffer[index]);
    index += 2;

    buffer[index] = mrz->pingInfo.modeAndStabilisation;
    index++;
    buffer[index] = mrz->pingInfo.runtimeFilter1;
    index++;

    mb_put_binary_short(true, mrz->pingInfo.runtimeFilter2, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, mrz->pingInfo.pipeTrackingStatus, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.transmitArraySizeUsed_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.receiveArraySizeUsed_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.transmitPower_dB, &buffer[index]);
    index += 4;
    mb_put_binary_short(true, mrz->pingInfo.SLrampUpTimeRemaining, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->pingInfo.padding2, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, mrz->pingInfo.yawAngle_deg, &buffer[index]);
    index += 4;

    // Info of tx sector data block, EMdgmMRZ_txSectorInfo
    mb_put_binary_short(true, mrz->pingInfo.numTxSectors, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->pingInfo.numBytesPerTxSector, &buffer[index]);
    index += 2;

    // Info at time of midpoint of first tx pulse
    mb_put_binary_float(true, mrz->pingInfo.headingVessel_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.soundSpeedAtTxDepth_mPerSec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.txTransducerDepth_m, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.z_waterLevelReRefPoint_m, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.x_kmallToall_m, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.y_kmallToall_m, &buffer[index]);
    index += 4;

    buffer[index] = mrz->pingInfo.latLongInfo;
    index++;
    buffer[index] = mrz->pingInfo.posSensorStatus;
    index++;
    buffer[index] = mrz->pingInfo.attitudeSensorStatus;
    index++;
    buffer[index] = mrz->pingInfo.padding3;
    index++;

    mb_put_binary_double(true, mrz->pingInfo.latitude_deg, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, mrz->pingInfo.longitude_deg, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, mrz->pingInfo.ellipsoidHeightReRefPoint_m, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->pingInfo.bsCorrectionOffset_dB, &buffer[index]);
    index += 4;
    buffer[index] = mrz->pingInfo.lambertsLawApplied;
    index++;
    buffer[index] = mrz->pingInfo.iceWindow;
    index++;
    mb_put_binary_short(true, mrz->pingInfo.activeModes, &buffer[index]);
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesInfoData:            %d\n", mrz->pingInfo.numBytesInfoData);
      fprintf(stderr, "dbg5       padding0:                    %d\n", mrz->pingInfo.padding0);
      fprintf(stderr, "dbg5       pingRate_Hz:                 %f\n", mrz->pingInfo.pingRate_Hz);
      fprintf(stderr, "dbg5       beamSpacing:                 %d\n", mrz->pingInfo.beamSpacing);
      fprintf(stderr, "dbg5       depthMode:                   %d\n", mrz->pingInfo.depthMode);
      fprintf(stderr, "dbg5       subDepthMode:                %d\n", mrz->pingInfo.subDepthMode);
      fprintf(stderr, "dbg5       distanceBtwSwath:            %d\n", mrz->pingInfo.distanceBtwSwath);
      fprintf(stderr, "dbg5       detectionMode:               %d\n", mrz->pingInfo.detectionMode);
      fprintf(stderr, "dbg5       pulseForm:                   %d\n", mrz->pingInfo.pulseForm);
      fprintf(stderr, "dbg5       padding1:                    %d\n", mrz->pingInfo.padding1);
      fprintf(stderr, "dbg5       frequencyMode_Hz:            %f\n", mrz->pingInfo.frequencyMode_Hz);
      fprintf(stderr, "dbg5       freqRangeLowLim_Hz:          %f\n", mrz->pingInfo.freqRangeLowLim_Hz);
      fprintf(stderr, "dbg5       freqRangeHighLim_Hz:         %f\n", mrz->pingInfo.freqRangeHighLim_Hz);
      fprintf(stderr, "dbg5       maxEffTxPulseLength_sec:     %f\n", mrz->pingInfo.maxEffTxPulseLength_sec);
      fprintf(stderr, "dbg5       maxTotalTxPulseLength_sec:   %f\n", mrz->pingInfo.maxTotalTxPulseLength_sec);
      fprintf(stderr, "dbg5       maxEffTxBandWidth_Hz:        %f\n", mrz->pingInfo.maxEffTxBandWidth_Hz);
      fprintf(stderr, "dbg5       absCoeff_dBPerkm:            %f\n", mrz->pingInfo.absCoeff_dBPerkm);
      fprintf(stderr, "dbg5       portSectorEdge_deg:          %f\n", mrz->pingInfo.portSectorEdge_deg);
      fprintf(stderr, "dbg5       starbSectorEdge_deg:         %f\n", mrz->pingInfo.starbSectorEdge_deg);
      fprintf(stderr, "dbg5       portMeanCov_m:               %d\n", mrz->pingInfo.portMeanCov_m);
      fprintf(stderr, "dbg5       starbMeanCov_m:              %d\n", mrz->pingInfo.starbMeanCov_m);
      fprintf(stderr, "dbg5       modeAndStabilisation:        %d\n", mrz->pingInfo.modeAndStabilisation);
      fprintf(stderr, "dbg5       runtimeFilter1:              %d\n", mrz->pingInfo.runtimeFilter1);
      fprintf(stderr, "dbg5       runtimeFilter2:              %d\n", mrz->pingInfo.runtimeFilter2);
      fprintf(stderr, "dbg5       pipeTrackingStatus:          %d\n", mrz->pingInfo.pipeTrackingStatus);
      fprintf(stderr, "dbg5       transmitArraySizeUsed_deg:   %f\n", mrz->pingInfo.transmitArraySizeUsed_deg);
      fprintf(stderr, "dbg5       receiveArraySizeUsed_deg:    %f\n", mrz->pingInfo.receiveArraySizeUsed_deg);
      fprintf(stderr, "dbg5       transmitPower_dB:            %f\n", mrz->pingInfo.transmitPower_dB);
      fprintf(stderr, "dbg5       SLrampUpTimeRemaining:       %d\n", mrz->pingInfo.SLrampUpTimeRemaining);
      fprintf(stderr, "dbg5       padding2:                    %d\n", mrz->pingInfo.padding2);
      fprintf(stderr, "dbg5       yawAngle_deg:                %f\n", mrz->pingInfo.yawAngle_deg);
      fprintf(stderr, "dbg5       numTxSectors:                %d\n", mrz->pingInfo.numTxSectors);
      fprintf(stderr, "dbg5       numBytesPerTxSector:         %d\n", mrz->pingInfo.numBytesPerTxSector);
      fprintf(stderr, "dbg5       headingVessel_deg:           %f\n", mrz->pingInfo.headingVessel_deg);
      fprintf(stderr, "dbg5       soundSpeedAtTxDepth_mPerSec: %f\n", mrz->pingInfo.soundSpeedAtTxDepth_mPerSec);
      fprintf(stderr, "dbg5       txTransducerDepth_m:         %f\n", mrz->pingInfo.txTransducerDepth_m);
      fprintf(stderr, "dbg5       z_waterLevelReRefPoint_m:    %f\n", mrz->pingInfo.z_waterLevelReRefPoint_m);
      fprintf(stderr, "dbg5       x_kmallToall_m:              %f\n", mrz->pingInfo.x_kmallToall_m);
      fprintf(stderr, "dbg5       y_kmallToall_m:              %f\n", mrz->pingInfo.y_kmallToall_m);
      fprintf(stderr, "dbg5       latLongInfo:                 %d\n", mrz->pingInfo.latLongInfo);
      fprintf(stderr, "dbg5       posSensorStatus:             %d\n", mrz->pingInfo.posSensorStatus);
      fprintf(stderr, "dbg5       attitudeSensorStatus:        %d\n", mrz->pingInfo.attitudeSensorStatus);
      fprintf(stderr, "dbg5       padding3:                    %d\n", mrz->pingInfo.padding3);
      fprintf(stderr, "dbg5       latitude_deg:                %f\n", mrz->pingInfo.latitude_deg);
      fprintf(stderr, "dbg5       longitude_deg:               %f\n", mrz->pingInfo.longitude_deg);
      fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m: %f\n", mrz->pingInfo.ellipsoidHeightReRefPoint_m);
      fprintf(stderr, "dbg5       bsCorrectionOffset_dB:       %f\n", mrz->pingInfo.bsCorrectionOffset_dB);
      fprintf(stderr, "dbg5       lambertsLawApplied:          %u\n", mrz->pingInfo.lambertsLawApplied);
      fprintf(stderr, "dbg5       iceWindow:                   %u\n", mrz->pingInfo.iceWindow);
      fprintf(stderr, "dbg5       activeModes:                 %u\n", mrz->pingInfo.activeModes);
    }

    /* EMdgmMRZ_txSectorInfo - sector information */
    for (int i = 0; i < (mrz->pingInfo.numTxSectors); i++) {
      buffer[index] = mrz->sectorInfo[i].txSectorNumb;
      index++;
      buffer[index] = mrz->sectorInfo[i].txArrNumber;
      index++;
      buffer[index] = mrz->sectorInfo[i].txSubArray;
      index++;
      buffer[index] = mrz->sectorInfo[i].padding0;
      index++;
      mb_put_binary_float(true, mrz->sectorInfo[i].sectorTransmitDelay_sec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].tiltAngleReTx_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].txNominalSourceLevel_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].txFocusRange_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].centreFreq_Hz, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].signalBandWidth_Hz, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].totalSignalLength_sec, &buffer[index]);
      index += 4;
      buffer[index] = mrz->sectorInfo[i].pulseShading;
      index++;
      buffer[index] = mrz->sectorInfo[i].signalWaveForm;
      index++;
      mb_put_binary_short(true, mrz->sectorInfo[i].padding1, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, mrz->sectorInfo[i].highVoltageLevel_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].sectorTrackingCorr_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sectorInfo[i].effectiveSignalLength_sec, &buffer[index]);
      index += 4;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mrz->pingInfo.numTxSectors);
        fprintf(stderr, "dbg5       txSectorNumb:                %d\n", mrz->sectorInfo[i].txSectorNumb);
        fprintf(stderr, "dbg5       txArrNumber:                 %d\n", mrz->sectorInfo[i].txArrNumber);
        fprintf(stderr, "dbg5       txSubArray:                  %d\n", mrz->sectorInfo[i].txSubArray);
        fprintf(stderr, "dbg5       padding0:                    %d\n", mrz->sectorInfo[i].padding0);
        fprintf(stderr, "dbg5       sectorTransmitDelay_sec:     %f\n", mrz->sectorInfo[i].sectorTransmitDelay_sec);
        fprintf(stderr, "dbg5       tiltAngleReTx_deg:           %f\n", mrz->sectorInfo[i].tiltAngleReTx_deg);
        fprintf(stderr, "dbg5       txNominalSourceLevel_dB:     %f\n", mrz->sectorInfo[i].txNominalSourceLevel_dB);
        fprintf(stderr, "dbg5       txFocusRange_m:              %f\n", mrz->sectorInfo[i].txFocusRange_m);
        fprintf(stderr, "dbg5       centreFreq_Hz:               %f\n", mrz->sectorInfo[i].centreFreq_Hz);
        fprintf(stderr, "dbg5       signalBandWidth_Hz:          %f\n", mrz->sectorInfo[i].signalBandWidth_Hz);
        fprintf(stderr, "dbg5       totalSignalLength_sec:       %f\n", mrz->sectorInfo[i].totalSignalLength_sec);
        fprintf(stderr, "dbg5       pulseShading:                %d\n", mrz->sectorInfo[i].pulseShading);
        fprintf(stderr, "dbg5       signalWaveForm:              %d\n", mrz->sectorInfo[i].signalWaveForm);
        fprintf(stderr, "dbg5       padding1:                    %d\n", mrz->sectorInfo[i].padding1);
        fprintf(stderr, "dbg5       highVoltageLevel_dB:         %f\n", mrz->sectorInfo[i].highVoltageLevel_dB);
        fprintf(stderr, "dbg5       sectorTrackingCorr_dB:       %f\n", mrz->sectorInfo[i].sectorTrackingCorr_dB);
        fprintf(stderr, "dbg5       effectiveSignalLength_sec:   %f\n", mrz->sectorInfo[i].effectiveSignalLength_sec);
      }
    }

    /* EMdgmMRZ_rxInfo - receiver specific info */
    mb_put_binary_short(true, mrz->rxInfo.numBytesRxInfo, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numSoundingsMaxMain, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numSoundingsValidMain, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numBytesPerSounding, &buffer[index]);
    index += 2;

    mb_put_binary_float(true, mrz->rxInfo.WCSampleRate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->rxInfo.seabedImageSampleRate, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->rxInfo.BSnormal_dB, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mrz->rxInfo.BSoblique_dB, &buffer[index]);
    index += 4;

    mb_put_binary_short(true, mrz->rxInfo.extraDetectionAlarmFlag, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numExtraDetections, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numExtraDetectionClasses, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mrz->rxInfo.numBytesPerClass, &buffer[index]);
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesInfoData:          %d\n", mrz->rxInfo.numBytesRxInfo);
      fprintf(stderr, "dbg5       numSoundingsMaxMain:       %d\n", mrz->rxInfo.numSoundingsMaxMain);
      fprintf(stderr, "dbg5       numSoundingsValidMain:     %d\n", mrz->rxInfo.numSoundingsValidMain);
      fprintf(stderr, "dbg5       numBytesPerSounding:       %d\n", mrz->rxInfo.numBytesPerSounding);
      fprintf(stderr, "dbg5       WCSampleRate:              %f\n", mrz->rxInfo.WCSampleRate);
      fprintf(stderr, "dbg5       seabedImageSampleRate:     %f\n", mrz->rxInfo.seabedImageSampleRate);
      fprintf(stderr, "dbg5       BSnormal_dB:               %f\n", mrz->rxInfo.BSnormal_dB);
      fprintf(stderr, "dbg5       BSoblique_dB:              %f\n", mrz->rxInfo.BSoblique_dB);
      fprintf(stderr, "dbg5       extraDetectionAlarmFlag:   %d\n", mrz->rxInfo.extraDetectionAlarmFlag);
      fprintf(stderr, "dbg5       numExtraDetections:        %d\n", mrz->rxInfo.numExtraDetections);
      fprintf(stderr, "dbg5       numExtraDetectionClasses:  %d\n", mrz->rxInfo.numExtraDetectionClasses);
      fprintf(stderr, "dbg5       numBytesPerClass:          %d\n", mrz->rxInfo.numBytesPerClass);
    }

    /* EMdgmMRZ_extraDetClassInfo -  Extra detection class info */
    for (int i = 0; i < (mrz->rxInfo.numExtraDetectionClasses); i++) {
      mb_put_binary_short(true, mrz->extraDetClassInfo[i].numExtraDetInClass, &buffer[index]);
      index += 2;
      buffer[index] = mrz->extraDetClassInfo[i].padding;
      index++;
      buffer[index] = mrz->extraDetClassInfo[i].alarmFlag;
      index++;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       numExtraDetInClass:  %d\n", mrz->extraDetClassInfo[i].numExtraDetInClass);
        fprintf(stderr, "dbg5       padding:             %d\n", mrz->extraDetClassInfo[i].padding);
        fprintf(stderr, "dbg5       alarmFlag:           %d\n", mrz->extraDetClassInfo[i].alarmFlag);
      }
    }

    /* EMdgmMRZ_sounding - Data for each sounding */
    //numSidescanSamples = 0;
    //numSoundings = mrz->rxInfo.numSoundingsMaxMain + mrz->rxInfo.numExtraDetections;
    for (int i = 0; i < numSoundings; i++) {
      mb_put_binary_short(true, mrz->sounding[i].soundingIndex, &buffer[index]);
      index += 2;
      buffer[index] = mrz->sounding[i].txSectorNumb;
      index++;

      /* Detection info */
      buffer[index] = mrz->sounding[i].detectionType;
      index++;
      buffer[index] = mrz->sounding[i].detectionMethod;
      index++;
      buffer[index] = mrz->sounding[i].rejectionInfo1;
      index++;
      buffer[index] = mrz->sounding[i].rejectionInfo2;
      index++;
      buffer[index] = mrz->sounding[i].postProcessingInfo;
      index++;
      buffer[index] = mrz->sounding[i].detectionClass;
      index++;
      buffer[index] = mrz->sounding[i].detectionConfidenceLevel;
      index++;
    /* These two bytes specified as padding in the Kongsberg specification but are
       here used for the MB-System beam flag - if the first mb_u_char == 1 then the
       second byte is an MB-System beamflag */
      // mb_put_binary_short(true, mrz->sounding[i].padding, &buffer[index]);
      // index += 2;
      buffer[index] = mrz->sounding[i].beamflag_enabled;
      index++;
      buffer[index] = mrz->sounding[i].beamflag;
      index++;
      mb_put_binary_float(true, mrz->sounding[i].rangeFactor, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].qualityFactor, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].detectionUncertaintyVer_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].detectionUncertaintyHor_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].detectionWindowLength_sec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].echoLength_sec, &buffer[index]);
      index += 4;

      /* Water column paramters */
      mb_put_binary_short(true, mrz->sounding[i].WCBeamNumb, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mrz->sounding[i].WCrange_samples, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, mrz->sounding[i].WCNomBeamAngleAcross_deg, &buffer[index]);
      index += 4;

      /* Reflectivity data (backscatter (BS) data) */
      mb_put_binary_float(true, mrz->sounding[i].meanAbsCoeff_dBPerkm, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].reflectivity1_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].reflectivity2_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].receiverSensitivityApplied_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].sourceLevelApplied_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].BScalibration_dB, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].TVG_dB, &buffer[index]);
      index += 4;

      /* Range and angle data */
      mb_put_binary_float(true, mrz->sounding[i].beamAngleReRx_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].beamAngleCorrection_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].twoWayTravelTime_sec, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].twoWayTravelTimeCorrection_sec, &buffer[index]);
      index += 4;

      /* Georeferenced depth points */
      mb_put_binary_float(true, mrz->sounding[i].deltaLatitude_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].deltaLongitude_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].z_reRefPoint_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].y_reRefPoint_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].x_reRefPoint_m, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mrz->sounding[i].beamIncAngleAdj_deg, &buffer[index]);
      index += 4;
      mb_put_binary_short(true, mrz->sounding[i].realTimeCleanInfo, &buffer[index]);
      index += 2;

      /* Seabed image */
      mb_put_binary_short(true, mrz->sounding[i].SIstartRange_samples, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mrz->sounding[i].SIcentreSample, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mrz->sounding[i].SInumSamples, &buffer[index]);
      index += 2;

      //numSidescanSamples += mrz->sounding[i].SInumSamples;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       soundingIndex:                  %d\n", mrz->sounding[i].soundingIndex);
        fprintf(stderr, "dbg5       txSectorNumb:                   %d\n", mrz->sounding[i].txSectorNumb);
        fprintf(stderr, "dbg5       detectionType:                  %d\n", mrz->sounding[i].detectionType);
        fprintf(stderr, "dbg5       detectionMethod:                %d\n", mrz->sounding[i].detectionMethod);
        fprintf(stderr, "dbg5       rejectionInfo1:                 %d\n", mrz->sounding[i].rejectionInfo1);
        fprintf(stderr, "dbg5       rejectionInfo2:                 %d\n", mrz->sounding[i].rejectionInfo2);
        fprintf(stderr, "dbg5       postProcessingInfo:             %d\n", mrz->sounding[i].postProcessingInfo);
        fprintf(stderr, "dbg5       detectionClass:                 %d\n", mrz->sounding[i].detectionClass);
        fprintf(stderr, "dbg5       detectionConfidenceLevel        %d\n", mrz->sounding[i].detectionConfidenceLevel);
        // fprintf(stderr, "dbg5       padding:                       %d\n", mrz->sounding[i].padding);
        fprintf(stderr, "dbg5       beamflag_enabled:               %d\n", mrz->sounding[i].beamflag_enabled);
        fprintf(stderr, "dbg5       beamflag:                       %d\n", mrz->sounding[i].beamflag);
        fprintf(stderr, "dbg5       rangeFactor:                    %f\n", mrz->sounding[i].rangeFactor);
        fprintf(stderr, "dbg5       qualityFactor:                  %f\n", mrz->sounding[i].qualityFactor);
        fprintf(stderr, "dbg5       detectionUncertaintyVer_m:      %f\n", mrz->sounding[i].detectionUncertaintyVer_m);
        fprintf(stderr, "dbg5       detectionUncertaintyHor_m:      %f\n", mrz->sounding[i].detectionUncertaintyHor_m);
        fprintf(stderr, "dbg5       detectionWindowLength_sec:      %f\n", mrz->sounding[i].detectionWindowLength_sec);
        fprintf(stderr, "dbg5       echoLength_sec:                 %f\n", mrz->sounding[i].echoLength_sec);
        fprintf(stderr, "dbg5       WCBeamNumb:                     %d\n", mrz->sounding[i].WCBeamNumb);
        fprintf(stderr, "dbg5       WCrange_samples:                %d\n", mrz->sounding[i].WCrange_samples);
        fprintf(stderr, "dbg5       WCNomBeamAngleAcross_deg:       %f\n", mrz->sounding[i].WCNomBeamAngleAcross_deg);
        fprintf(stderr, "dbg5       meanAbsCoeff_dBPerkm:           %f\n", mrz->sounding[i].meanAbsCoeff_dBPerkm);
        fprintf(stderr, "dbg5       reflectivity1_dB:               %f\n", mrz->sounding[i].reflectivity1_dB);
        fprintf(stderr, "dbg5       reflectivity2_dB:               %f\n", mrz->sounding[i].reflectivity2_dB);
        fprintf(stderr, "dbg5       receiverSensitivityApplied_dB:  %f\n", mrz->sounding[i].receiverSensitivityApplied_dB);
        fprintf(stderr, "dbg5       sourceLevelApplied_dB:          %f\n", mrz->sounding[i].sourceLevelApplied_dB);
        fprintf(stderr, "dbg5       BScalibration_dB:               %f\n", mrz->sounding[i].BScalibration_dB);
        fprintf(stderr, "dbg5       TVG_dB:                         %f\n", mrz->sounding[i].TVG_dB);
        fprintf(stderr, "dbg5       beamAngleReRx_deg:              %f\n", mrz->sounding[i].beamAngleReRx_deg);
        fprintf(stderr, "dbg5       beamAngleCorrection_deg:        %f\n", mrz->sounding[i].beamAngleCorrection_deg);
        fprintf(stderr, "dbg5       twoWayTravelTime_sec            %f\n", mrz->sounding[i].twoWayTravelTime_sec);
        fprintf(stderr, "dbg5       twoWayTravelTimeCorrection_sec  %f\n", mrz->sounding[i].twoWayTravelTimeCorrection_sec);
        fprintf(stderr, "dbg5       deltaLatitude_deg:              %f\n", mrz->sounding[i].deltaLatitude_deg);
        fprintf(stderr, "dbg5       deltaLongitude_deg:             %f\n", mrz->sounding[i].deltaLongitude_deg);
        fprintf(stderr, "dbg5       z_reRefPoint_m:                 %f\n", mrz->sounding[i].z_reRefPoint_m);
        fprintf(stderr, "dbg5       y_reRefPoint_m:                 %f\n", mrz->sounding[i].y_reRefPoint_m);
        fprintf(stderr, "dbg5       x_reRefPoint_m:                 %f\n", mrz->sounding[i].x_reRefPoint_m);
        fprintf(stderr, "dbg5       beamIncAngleAdj_deg:            %f\n", mrz->sounding[i].beamIncAngleAdj_deg);
        fprintf(stderr, "dbg5       realTimeCleanInfo:              %d\n", mrz->sounding[i].realTimeCleanInfo);
        fprintf(stderr, "dbg5       SIstartRange_samples:           %d\n", mrz->sounding[i].SIstartRange_samples);
        fprintf(stderr, "dbg5       SIcentreSample:                 %d\n", mrz->sounding[i].SIcentreSample);
        fprintf(stderr, "dbg5       SInumSamples:                   %d\n", mrz->sounding[i].SInumSamples);
      }
    }

    for (int i = 0; i < numSidescanSamples; i++) {
      mb_put_binary_short(true, mrz->SIsample_desidB[i], &buffer[index]);
      index += 2;
    }

    /* Insert closing byte count */
    mb_put_binary_int(true, mrz->header.numBytesDgm, &buffer[index]);
    index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          mrz->header.dgmType, *size, index, mrz->header.time_sec, mrz->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_mwc(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, int imwc, size_t *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       imwcc:      %d\n", imwc);
  }

  char *buffer = NULL;
  int index = 0;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_mwc *mwc = &(store->mwc[imwc]);

  /* datagram version being written */
  mwc->header.dgmVersion = MBSYS_KMBES_MWC_VERSION;

  /* size of output record and components thereof - set the size according to what
      we know about, as anything added by Kongsberg that we don't know about will
      have been skipped while reading */
  mwc->cmnPart.numBytesCmnPart = 12;
  mwc->txInfo.numBytesTxInfo = 12;
  mwc->txInfo.numBytesPerTxSector = 16;
  mwc->rxInfo.numBytesRxInfo = 16;
  mwc->rxInfo.numBytesPerBeamEntry = 16;
  int numBytesPerSample = 1 + mwc->rxInfo.phaseFlag;
  int numBytesWC = 0;
  for (int i=0; i<mwc->rxInfo.numBeams; i++) {
    numBytesWC = mwc->beamData_p[i].numSampleData * numBytesPerSample;
  }
  mwc->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE
                            + mwc->cmnPart.numBytesCmnPart
                            + MBSYS_KMBES_PARITION_SIZE
                            + mwc->txInfo.numBytesTxInfo
                            + mwc->txInfo.numTxSectors * mwc->txInfo.numBytesPerTxSector
                            + mwc->rxInfo.numBytesRxInfo
                            + mwc->rxInfo.numBeams * mwc->rxInfo.numBytesPerBeamEntry
                            + numBytesWC
                            + MBSYS_KMBES_END_SIZE;

  *size = (size_t) mwc->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&mwc->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* EMdgmMpartition - data partition information */
    mb_put_binary_short(true, mwc->partition.numOfDgms, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->partition.dgmNum, &buffer[index]);
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numOfDgms:  %d\n", mwc->partition.numOfDgms);
      fprintf(stderr, "dbg5       dgmNum:     %d\n", mwc->partition.dgmNum);
    }

    /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
    mb_put_binary_short(true, mwc->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->cmnPart.pingCnt, &buffer[index]);
    index += 2;
    buffer[index] = mwc->cmnPart.rxFansPerPing;
    index++;
    buffer[index] = mwc->cmnPart.rxFanIndex;
    index++;
    buffer[index] = mwc->cmnPart.swathsPerPing;
    index++;
    buffer[index] = mwc->cmnPart.swathAlongPosition;
    index++;
    buffer[index] = mwc->cmnPart.txTransducerInd;
    index++;
    buffer[index] = mwc->cmnPart.rxTransducerInd;
    index++;
    buffer[index] = mwc->cmnPart.numRxTransducers;
    index++;
    buffer[index] = mwc->cmnPart.algorithmType;
    index++;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", mwc->cmnPart.numBytesCmnPart);
      fprintf(stderr, "dbg5       pingCnt:             %d\n", mwc->cmnPart.pingCnt);
      fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", mwc->cmnPart.rxFansPerPing);
      fprintf(stderr, "dbg5       rxFanIndex:          %d\n", mwc->cmnPart.rxFanIndex);
      fprintf(stderr, "dbg5       swathsPerPing:       %d\n", mwc->cmnPart.swathsPerPing);
      fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", mwc->cmnPart.swathAlongPosition);
      fprintf(stderr, "dbg5       txTransducerInd:     %d\n", mwc->cmnPart.txTransducerInd);
      fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", mwc->cmnPart.rxTransducerInd);
      fprintf(stderr, "dbg5       numRxTransducers:    %d\n", mwc->cmnPart.numRxTransducers);
      fprintf(stderr, "dbg5       algorithmType:       %d\n", mwc->cmnPart.algorithmType);
    }

    /* EMdgmMWCtxInfo - transmit sectors, general info for all sectors */
    mb_put_binary_short(true, mwc->txInfo.numBytesTxInfo, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->txInfo.numTxSectors, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->txInfo.numBytesPerTxSector, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->txInfo.padding, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, mwc->txInfo.heave_m, &buffer[index]);
    index += 4;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesTxInfo:       %d\n", mwc->txInfo.numBytesTxInfo);
      fprintf(stderr, "dbg5       numTxSectors:         %d\n", mwc->txInfo.numTxSectors);
      fprintf(stderr, "dbg5       numBytesPerTxSector:  %d\n", mwc->txInfo.numBytesPerTxSector);
      fprintf(stderr, "dbg5       padding:              %d\n", mwc->txInfo.padding);
      fprintf(stderr, "dbg5       heave_m:              %f\n", mwc->txInfo.heave_m);
    }

    /* EMdgmMWCtxSectorData - transmit sector data, loop for all i = numTxSectors */
    for (int i=0; i<(mwc->txInfo.numTxSectors); i++) {
      mb_put_binary_float(true, mwc->sectorData[i].tiltAngleReTx_deg, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mwc->sectorData[i].centreFreq_Hz, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, mwc->sectorData[i].txBeamWidthAlong_deg, &buffer[index]);
      index += 4;
      mb_put_binary_short(true, mwc->sectorData[i].txSectorNum, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mwc->sectorData[i].padding, &buffer[index]);
      index += 2;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       #MWC transmit sector %d/%d:\n", i + 1, mwc->txInfo.numTxSectors);
        fprintf(stderr, "dbg5       tiltAngleReTx_deg:     %f\n", mwc->sectorData[i].tiltAngleReTx_deg);
        fprintf(stderr, "dbg5       centreFreq_Hz:         %f\n", mwc->sectorData[i].centreFreq_Hz);
        fprintf(stderr, "dbg5       txBeamWidthAlong_deg:  %f\n", mwc->sectorData[i].txBeamWidthAlong_deg);
        fprintf(stderr, "dbg5       txSectorNum:           %d\n", mwc->sectorData[i].txSectorNum);
        fprintf(stderr, "dbg5       padding:               %d\n", mwc->sectorData[i].padding);
      }
    }

    /* EMdgmMWCrxInfo - receiver, general info */
    mb_put_binary_short(true, mwc->rxInfo.numBytesRxInfo, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, mwc->rxInfo.numBeams, &buffer[index]);
    index += 2;
    buffer[index] = mwc->rxInfo.numBytesPerBeamEntry;
    index ++;
    buffer[index] = mwc->rxInfo.phaseFlag;
    index ++;
    buffer[index] = mwc->rxInfo.TVGfunctionApplied;
    index ++;
    buffer[index] = mwc->rxInfo.TVGoffset_dB;
    index ++;
    mb_put_binary_float(true, mwc->rxInfo.sampleFreq_Hz, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, mwc->rxInfo.soundVelocity_mPerSec, &buffer[index]);
    index += 4;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesRxInfo:         %d\n", mwc->rxInfo.numBytesRxInfo);
      fprintf(stderr, "dbg5       numBeams:               %d\n", mwc->rxInfo.numBeams);
      fprintf(stderr, "dbg5       numBytesPerBeamEntry:   %d\n", mwc->rxInfo.numBytesPerBeamEntry);
      fprintf(stderr, "dbg5       phaseFlag               %d\n", mwc->rxInfo.phaseFlag);
      fprintf(stderr, "dbg5       TVGfunctionApplied:     %d\n", mwc->rxInfo.TVGfunctionApplied);
      fprintf(stderr, "dbg5       TVGoffset_dB:           %d\n", mwc->rxInfo.TVGoffset_dB);
      fprintf(stderr, "dbg5       sampleFreq_Hz:          %f\n", mwc->rxInfo.sampleFreq_Hz);
      fprintf(stderr, "dbg5       soundVelocity_mPerSec:  %f\n", mwc->rxInfo.soundVelocity_mPerSec);
    }

    /* EMdgmMWCrxBeamData - receiver, specific info for each beam */
    for (int i=0; i<(mwc->rxInfo.numBeams); i++) {
      mb_put_binary_float(true, mwc->beamData_p[i].beamPointAngReVertical_deg, &buffer[index]);
      index += 4;
      mb_put_binary_short(true, mwc->beamData_p[i].startRangeSampleNum, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mwc->beamData_p[i].detectedRangeInSamples, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mwc->beamData_p[i].beamTxSectorNum, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, mwc->beamData_p[i].numSampleData, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, mwc->beamData_p[i].detectedRangeInSamplesHighResolution, &buffer[index]);
      index += 4;

      /* now insert the samples */
      memcpy(&buffer[index], &(mwc->beamData_p[i].sampleAmplitude05dB_p), mwc->beamData_p[i].numSampleData);
      index += mwc->beamData_p[i].numSampleData;

      switch (mwc->rxInfo.phaseFlag) {
        /* no phase data included */
        case 0:
          break;

        /* 8-bit phase data */
        case 1:
          /* Rx beam phase in 180/128 degree resolution. */
          memcpy(&buffer[index], mwc->beamData_p[i].samplePhase8bit, mwc->beamData_p[i].numSampleData);
          index += mwc->beamData_p[i].numSampleData;
          break;  // TODO(schwehr): Should this fall through?

        /* 16 bit phase data */
        case 2:
          /* Rx beam phase in 0.01 degree resolution */
          for (int k = 0; k < mwc->beamData_p[i].numSampleData; k++) {
            mb_put_binary_short(true, mwc->beamData_p[i].samplePhase16bit[k], &buffer[index]);
            index += 2;
          }
      }

      if (status == MB_SUCCESS && verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       #MWC receiver beam data %d/%d:\n", i, mwc->rxInfo.numBeams);
        fprintf(stderr, "dbg5       tiltAngleReTx_deg:                     %f\n", mwc->beamData_p[i].beamPointAngReVertical_deg);
        fprintf(stderr, "dbg5       startRangeSampleNum:                   %d\n", mwc->beamData_p[i].startRangeSampleNum);
        fprintf(stderr, "dbg5       detectedRangeInSamples:                %d\n", mwc->beamData_p[i].detectedRangeInSamples);
        fprintf(stderr, "dbg5       beamTxSectorNum:                       %d\n", mwc->beamData_p[i].beamTxSectorNum);
        fprintf(stderr, "dbg5       numSampleData:                         %d\n", mwc->beamData_p[i].numSampleData);
        fprintf(stderr, "dbg5       detectedRangeInSamplesHighResolution:  %f\n", mwc->beamData_p[i].detectedRangeInSamplesHighResolution);
        fprintf(stderr, "dbg5       (amplitude phase)                      [\n");
        for (int k = 0; k < (mwc->beamData_p[i].numSampleData); k++) {
          if (k % 10 == 0)
            fprintf(stderr, "dbg5             ");
          if (mwc->rxInfo.phaseFlag == 1)
            fprintf(stderr, " (%d %d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k], mwc->beamData_p[i].samplePhase8bit[k]);
          else if (mwc->rxInfo.phaseFlag == 2)
            fprintf(stderr, " (%d %d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k], mwc->beamData_p[i].samplePhase16bit[k]);
          else
            fprintf(stderr, " (%d),", mwc->beamData_p[i].sampleAmplitude05dB_p[k]);
          if ((k+1) % 10 == 0)
            fprintf(stderr, "\n");
        }
      }

    /* Insert closing byte count */
    mb_put_binary_int(true, mwc->header.numBytesDgm, &buffer[index]);
    // index += 4;
    }
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          mwc->header.dgmType, *size, index, mwc->header.time_sec, mwc->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_cpo(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_cpo *cpo = &(store->cpo);

  /* datagram version being written */
  cpo->header.dgmVersion = MBSYS_KMBES_CPO_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:                  %u\n", cpo->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                      %s\n", cpo->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:                   %u\n", cpo->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                     %u\n", cpo->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:                %u\n", cpo->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                     %u\n", cpo->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:                 %u\n", cpo->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:              %u\n", cpo->cmnPart.numBytesCmnPart);
    fprintf(stderr, "dbg5       sensorSystem:                 %u\n", cpo->cmnPart.sensorSystem);
    fprintf(stderr, "dbg5       sensorStatus:                 %u\n", cpo->cmnPart.sensorStatus);
    fprintf(stderr, "dbg5       padding:                      %u\n", cpo->cmnPart.padding);

    fprintf(stderr, "dbg5       timeFromSensor_sec:           %u\n", cpo->sensorData.timeFromSensor_sec);
    fprintf(stderr, "dbg5       timeFromSensor_nanosec:       %u\n", cpo->sensorData.timeFromSensor_nanosec);
    fprintf(stderr, "dbg5       posFixQuality_m:              %f\n", cpo->sensorData.posFixQuality_m);
    fprintf(stderr, "dbg5       correctedLat_deg:             %f\n", cpo->sensorData.correctedLat_deg);
    fprintf(stderr, "dbg5       correctedLong_deg:            %f\n", cpo->sensorData.correctedLong_deg);
    fprintf(stderr, "dbg5       speedOverGround_mPerSec:      %f\n", cpo->sensorData.speedOverGround_mPerSec);
    fprintf(stderr, "dbg5       courseOverGround_deg:         %f\n", cpo->sensorData.courseOverGround_deg);
    fprintf(stderr, "dbg5       ellipsoidHeightReRefPoint_m:  %f\n", cpo->sensorData.ellipsoidHeightReRefPoint_m);
    fprintf(stderr, "dbg5       posDataFromSensor:            %s\n", cpo->sensorData.posDataFromSensor);
  }

  /* size of output record */
  *size = (size_t) cpo->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&cpo->header, error);

    /* calc number of bytes for raw sensor data */
    numBytesRawSensorData = cpo->header.numBytesDgm - MBSYS_KMBES_CPO_VAR_OFFSET;

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* common part */
    mb_put_binary_short(true, cpo->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, cpo->cmnPart.sensorSystem, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, cpo->cmnPart.sensorStatus, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, cpo->cmnPart.padding, &buffer[index]);
    index += 2;

    /* sensor data block */
    mb_put_binary_int(true, cpo->sensorData.timeFromSensor_sec, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, cpo->sensorData.timeFromSensor_nanosec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, cpo->sensorData.posFixQuality_m, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, cpo->sensorData.correctedLat_deg, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, cpo->sensorData.correctedLong_deg, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, cpo->sensorData.speedOverGround_mPerSec, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, cpo->sensorData.courseOverGround_deg, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, cpo->sensorData.ellipsoidHeightReRefPoint_m, &buffer[index]);
    index += 4;

    /* raw data msg from sensor */
    memcpy(&buffer[index], &(cpo->sensorData.posDataFromSensor), numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* insert closing byte count */
    mb_put_binary_int(true, cpo->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          cpo->header.dgmType, *size, index, cpo->header.time_sec, cpo->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_che(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  // size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_che *che = &(store->che);

  /* datagram version being written */
  che->header.dgmVersion = MBSYS_KMBES_CHE_VERSION;

  /* size of output record */
  *size = (size_t) che->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&che->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesDgm:    %u\n", che->header.numBytesDgm);
      fprintf(stderr, "dbg5       dgmType:        %s\n", che->header.dgmType);
      fprintf(stderr, "dbg5       dgmVersion:     %u\n", che->header.dgmVersion);
      fprintf(stderr, "dbg5       systemID:       %u\n", che->header.systemID);
      fprintf(stderr, "dbg5       echoSounderID:  %u\n", che->header.echoSounderID);
      fprintf(stderr, "dbg5       time_sec:       %u\n", che->header.time_sec);
      fprintf(stderr, "dbg5       time_nanosec:   %u\n", che->header.time_nanosec);
    }

    /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
    mb_put_binary_short(true, che->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, che->cmnPart.pingCnt, &buffer[index]);
    index += 2;
    buffer[index] = che->cmnPart.rxFansPerPing;
    index++;
    buffer[index] = che->cmnPart.rxFanIndex;
    index++;
    buffer[index] = che->cmnPart.swathsPerPing;
    index++;
    buffer[index] = che->cmnPart.swathAlongPosition;
    index++;
    buffer[index] = che->cmnPart.txTransducerInd;
    index++;
    buffer[index] = che->cmnPart.rxTransducerInd;
    index++;
    buffer[index] = che->cmnPart.numRxTransducers;
    index++;
    buffer[index] = che->cmnPart.algorithmType;
    index++;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", che->cmnPart.numBytesCmnPart);
      fprintf(stderr, "dbg5       pingCnt:             %d\n", che->cmnPart.pingCnt);
      fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", che->cmnPart.rxFansPerPing);
      fprintf(stderr, "dbg5       rxFanIndex:          %d\n", che->cmnPart.rxFanIndex);
      fprintf(stderr, "dbg5       swathsPerPing:       %d\n", che->cmnPart.swathsPerPing);
      fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", che->cmnPart.swathAlongPosition);
      fprintf(stderr, "dbg5       txTransducerInd:     %d\n", che->cmnPart.txTransducerInd);
      fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", che->cmnPart.rxTransducerInd);
      fprintf(stderr, "dbg5       numRxTransducers:    %d\n", che->cmnPart.numRxTransducers);
      fprintf(stderr, "dbg5       algorithmType:       %d\n", che->cmnPart.algorithmType);
    }

    /* sensor data block */
    mb_put_binary_float(true, che->data.heave_m, &buffer[index]);
    index += 4;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       heave_m:             %f\n", che->data.heave_m);
    }

    /* insert closing byte count */
    mb_put_binary_int(true, che->header.numBytesDgm, &buffer[index]);
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          che->header.dgmType, *size, index, che->header.time_sec, che->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_iip(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_iip *iip = &(store->iip);

  /* datagram version being written */
  iip->header.dgmVersion = MBSYS_KMBES_IIP_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", iip->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", iip->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", iip->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", iip->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", iip->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", iip->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", iip->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", iip->numBytesCmnPart);
    fprintf(stderr, "dbg5       info:             %u\n", iip->info);
    fprintf(stderr, "dbg5       status:           %u\n", iip->status);
    fprintf(stderr, "dbg5       install_txt:      %s\n", iip->install_txt);
  }

  /* size of output record */
  *size = (size_t) iip->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&iip->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    numBytesRawSensorData = iip->header.numBytesDgm - MBSYS_KMBES_IIP_VAR_OFFSET;

    mb_put_binary_short(true, iip->numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, iip->info, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, iip->status, &buffer[index]);
    index += 2;
    memcpy(&buffer[index], iip->install_txt, numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* Insert closing byte count */
    mb_put_binary_int(true, iip->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          iip->header.dgmType, *size, index, iip->header.time_sec, iip->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_iop(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesRawSensorData = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_iop *iop = &(store->iop);

  /* datagram version being written */
  iop->header.dgmVersion = MBSYS_KMBES_IOP_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", iop->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", iop->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", iop->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", iop->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", iop->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", iop->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", iop->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:  %u\n", iop->numBytesCmnPart);
    fprintf(stderr, "dbg5       info:             %u\n", iop->info);
    fprintf(stderr, "dbg5       status:           %u\n", iop->status);
    fprintf(stderr, "dbg5       runtime_txt:      %s\n", iop->runtime_txt);
  }

  /* size of output record */
  *size = (size_t) iop->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&iop->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    numBytesRawSensorData = iop->header.numBytesDgm - MBSYS_KMBES_IOP_VAR_OFFSET;

    mb_put_binary_short(true, iop->numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, iop->info, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, iop->status, &buffer[index]);
    index += 2;
    memcpy(&buffer[index], iop->runtime_txt, numBytesRawSensorData);
    index += numBytesRawSensorData;

    /* Insert closing byte count */
    mb_put_binary_int(true, iop->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          iop->header.dgmType, *size, index, iop->header.time_sec, iop->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_ibe(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_ib *ibe = &(store->ibe);

  /* datagram version being written */
  ibe->header.dgmVersion = MBSYS_KMBES_BIST_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", ibe->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", ibe->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", ibe->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", ibe->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", ibe->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", ibe->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", ibe->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:   %u\n", ibe->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:          %u\n", ibe->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:         %u\n", ibe->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:        %d\n", ibe->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:        %d\n", ibe->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:          %c\n", ibe->BISTText);
  }

  /* size of output record */
  *size = (size_t) ibe->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&ibe->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    mb_put_binary_short(true, ibe->numBytesCmnPart, &buffer[index]);
    index += 2;
    buffer[index] = ibe->BISTInfo;
    index++;
    buffer[index] = ibe->BISTStyle;
    index++;
    buffer[index] = ibe->BISTNumber;
    index++;
    buffer[index] = ibe->BISTStatus;
    index++;
    buffer[index] = ibe->BISTText;
    index++;

    /* Insert closing byte count */
    mb_put_binary_int(true, ibe->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          ibe->header.dgmType, *size, index, ibe->header.time_sec, ibe->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_ibr(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_ib *ibr = &(store->ibr);

  /* datagram version being written */
  ibr->header.dgmVersion = MBSYS_KMBES_BIST_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", ibr->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", ibr->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", ibr->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", ibr->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", ibr->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", ibr->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", ibr->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:   %u\n", ibr->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:          %u\n", ibr->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:         %u\n", ibr->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:        %d\n", ibr->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:        %d\n", ibr->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:          %c\n", ibr->BISTText);
  }

  /* size of output record */
  *size = (size_t) ibr->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&ibr->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    mb_put_binary_short(true, ibr->numBytesCmnPart, &buffer[index]);
    index += 2;
    buffer[index] = ibr->BISTInfo;
    index++;
    buffer[index] = ibr->BISTStyle;
    index++;
    buffer[index] = ibr->BISTNumber;
    index++;
    buffer[index] = ibr->BISTStatus;
    index++;
    buffer[index] = ibr->BISTText;
    index++;

    /* Insert closing byte count */
    mb_put_binary_int(true, ibr->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          ibr->header.dgmType, *size, index, ibr->header.time_sec, ibr->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_ibs(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_ib *ibs = &(store->ibs);

  /* datagram version being written */
  ibs->header.dgmVersion = MBSYS_KMBES_BIST_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:      %u\n", ibs->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:          %s\n", ibs->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:       %u\n", ibs->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:         %u\n", ibs->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:    %u\n", ibs->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:         %u\n", ibs->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:     %u\n", ibs->header.time_nanosec);

    fprintf(stderr, "dbg5       numBytesCmnPart:   %u\n", ibs->numBytesCmnPart);
    fprintf(stderr, "dbg5       BISTInfo:          %u\n", ibs->BISTInfo);
    fprintf(stderr, "dbg5       BISTStyle:         %u\n", ibs->BISTStyle);
    fprintf(stderr, "dbg5       BISTNumber:        %d\n", ibs->BISTNumber);
    fprintf(stderr, "dbg5       BISTStatus:        %d\n", ibs->BISTStatus);
    fprintf(stderr, "dbg5       BISTText:          %c\n", ibs->BISTText);
  }

  /* size of output record */
  *size = (size_t) ibs->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&ibs->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    mb_put_binary_short(true, ibs->numBytesCmnPart, &buffer[index]);
    index += 2;
    buffer[index] = ibs->BISTInfo;
    index++;
    buffer[index] = ibs->BISTStyle;
    index++;
    buffer[index] = ibs->BISTNumber;
    index++;
    buffer[index] = ibs->BISTStatus;
    index++;
    buffer[index] = ibs->BISTText;
    index++;

    /* Insert closing byte count */
    mb_put_binary_int(true, ibs->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          ibs->header.dgmType, *size, index, ibs->header.time_sec, ibs->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_fcf(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_fcf *fcf = &(store->fcf);
  struct mbsys_kmbes_m_partition *partition = &(fcf->partition);
  struct mbsys_kmbes_f_common *cmnPart = &(fcf->cmnPart);

  /* datagram version being written */
  fcf->header.dgmVersion = MBSYS_KMBES_FCF_VERSION;

  /* size of output record and components thereof - set the size according to what
      we know about, as anything added by Kongsberg that we don't know about will
      have been skipped while reading */
  cmnPart->numBytesCmnPart = 72;
  fcf->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE
                            + cmnPart->numBytesCmnPart
                            + MBSYS_KMBES_PARITION_SIZE
                            + cmnPart->numBytesFile
                            + MBSYS_KMBES_END_SIZE;

  *size = (size_t) fcf->header.numBytesDgm;
//fprintf(stderr, "************>%s:%d WRITE mbr_kemkmall_wr_fcf fcf->header.numBytesDgm:%d\n",
//__FILE__, __LINE__, fcf->header.numBytesDgm);

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesDgm:                %u\n", fcf->header.numBytesDgm);
      fprintf(stderr, "dbg5       dgmType:                    %s\n", fcf->header.dgmType);
      fprintf(stderr, "dbg5       dgmVersion:                 %u\n", fcf->header.dgmVersion);
      fprintf(stderr, "dbg5       systemID:                   %u\n", fcf->header.systemID);
      fprintf(stderr, "dbg5       echoSounderID:              %u\n", fcf->header.echoSounderID);
      fprintf(stderr, "dbg5       time_sec:                   %u\n", fcf->header.time_sec);
      fprintf(stderr, "dbg5       time_nanosec:               %u\n", fcf->header.time_nanosec);
    }

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&fcf->header, error);

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numOfDgms:                 %d\n", partition->numOfDgms);
      fprintf(stderr, "dbg5       dgmNum:                    %d\n", partition->dgmNum);

      fprintf(stderr, "dbg5       numBytesCmnPart:            %u\n", cmnPart->numBytesCmnPart);
      fprintf(stderr, "dbg5       fileStatus:                 %u\n", cmnPart->fileStatus);
      fprintf(stderr, "dbg5       fileStatus:                 %u\n", cmnPart->padding1);
      fprintf(stderr, "dbg5       numBytesFile:               %u\n", cmnPart->numBytesFile);
      fprintf(stderr, "dbg5       fcf->fileName:              %s\n", cmnPart->fileName);
    }

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* EMdgmMpartition - data partition information */
    mb_put_binary_short(true, partition->numOfDgms, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, partition->dgmNum, &buffer[index]);
    index += 2;

    /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
    mb_put_binary_short(true, cmnPart->numBytesCmnPart, &buffer[index]);
    index += 2;
    buffer[index] = cmnPart->fileStatus;
    index++;
    buffer[index] = cmnPart->padding1;
    index++;
    mb_put_binary_int(true, cmnPart->numBytesFile, &buffer[index]);
    index += 4;
    memcpy(&buffer[index], cmnPart->fileName, MBSYS_KMBES_MAX_F_FILENAME_LENGTH);
    index += MBSYS_KMBES_MAX_F_FILENAME_LENGTH;
    memcpy(&buffer[index], fcf->bsCalibrationFile, cmnPart->numBytesFile);
    index += cmnPart->numBytesFile;

    /* Insert closing byte count */
    mb_put_binary_int(true, fcf->header.numBytesDgm, &buffer[index]);
    index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          fcf->header.dgmType, *size, index, fcf->header.time_sec, fcf->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_xmb(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesVersion = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_xmb *xmb = &(store->xmb);
  struct mbsys_kmbes_iip *iip = &(store->iip);

  /* datagram version being written */
  xmb->header.dgmVersion = MBSYS_KMBES_XMB_VERSION;

  /* have to construct this record now */
  strncpy(xmb->version, MB_VERSION, MB_COMMENT_MAXLINE-1);
  numBytesVersion = strlen(xmb->version) + (strlen(xmb->version) % 2);
  xmb->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + numBytesVersion + 36;
  strncpy((char *)xmb->header.dgmType, "#XMB", 4);
  xmb->header.dgmVersion = MBSYS_KMBES_XMB_VERSION;
  xmb->header.systemID = iip->header.systemID;
  xmb->header.echoSounderID = iip->header.echoSounderID;
  xmb->header.time_sec = iip->header.time_sec;
  xmb->header.time_nanosec = iip->header.time_nanosec;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:             %u\n", xmb->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:                 %s\n", xmb->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:              %u\n", xmb->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:                %u\n", xmb->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:           %u\n", xmb->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:                %u\n", xmb->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:            %u\n", xmb->header.time_nanosec);
    fprintf(stderr, "dbg5       mbsystem_extensions:     %d\n", xmb->mbsystem_extensions);
    fprintf(stderr, "dbg5       watercolumn:             %d\n", xmb->watercolumn);
    for (int i=0;i<24;i++)
      fprintf(stderr, "dbg5       unused[%2d]:              %u\n", i, xmb->unused[i]);
    fprintf(stderr, "dbg5       version:                   %s\n", xmb->version);
  }

  /* size of output record */
  *size = (size_t) xmb->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&xmb->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    mb_put_binary_int(true, 1, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, xmb->watercolumn, &buffer[index]);
    index += 4;
    for (int i=0;i<24;i++) {
      buffer[index] = xmb->unused[i];
      index++;
    }
    memcpy(&buffer[index], xmb->version, numBytesVersion);
    index += numBytesVersion;

    /* Insert closing byte count */
    mb_put_binary_int(true, xmb->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          xmb->header.dgmType, *size, index, xmb->header.time_sec, xmb->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_xmc(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  size_t numBytesComment = 0;
  char *buffer = NULL;
  int index = 0;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_xmc *xmc = &(store->xmc);

  /* datagram version being written */
  xmc->header.dgmVersion = MBSYS_KMBES_XMC_VERSION;

  /* size of output record */
  numBytesComment = strlen(store->xmc.comment) + (strlen(store->xmc.comment) % 2);
  store->xmc.header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE + numBytesComment + 36;
  *size = (size_t) xmc->header.numBytesDgm;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xmc->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", xmc->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", xmc->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", xmc->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", xmc->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", xmc->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", xmc->header.time_nanosec);

    for (int i=0;i<32;i++)
      fprintf(stderr, "dbg5       unused[%2d]:    %u\n", i, xmc->unused[i]);
    fprintf(stderr, "dbg5       comment:        %s\n", xmc->comment);
  }

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&xmc->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    for (int i=0;i<32;i++) {
      buffer[index] = xmc->unused[i];
      index++;
    }
    memcpy(&buffer[index], xmc->comment, numBytesComment);
    index += numBytesComment;

    /* Insert closing byte count */
    mb_put_binary_int(true, xmc->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          xmc->header.dgmType, *size, index, xmc->header.time_sec, xmc->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};
/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_xmt(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, int ixmt, size_t *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
    fprintf(stderr, "dbg2       ixmt:       %d\n", ixmt);
  }

  char *buffer = NULL;
  // int numSoundings = 0;
  // int numSidescanSamples = 0;
  int index = 0;

  /* get pointer to the data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_xmt *xmt = (struct mbsys_kmbes_xmt *)&store->xmt[ixmt];

  /* datagram version being written */
  xmt->header.dgmVersion = MBSYS_KMBES_XMT_VERSION;

  /* size of output record and components thereof - set the size according to what
      we know about, as anything added by Kongsberg that we don't know about will
      have been skipped while reading */
  xmt->cmnPart.numBytesCmnPart = 12;
  xmt->xmtPingInfo.numBytesInfoData = MBSYS_KMBES_XMT_PINGINFO_DATALENGTH;
  xmt->xmtPingInfo.numBytesPerSounding = MBSYS_KMBES_XMT_SOUNDING_DATALENGTH;
  xmt->xmtPingInfo.padding0 = 0;
  xmt->header.numBytesDgm = MBSYS_KMBES_HEADER_SIZE
                            + xmt->cmnPart.numBytesCmnPart
                            + MBSYS_KMBES_PARITION_SIZE
                            + xmt->xmtPingInfo.numBytesInfoData
                            + xmt->xmtPingInfo.numSoundings * xmt->xmtPingInfo.numBytesPerSounding
                            + MBSYS_KMBES_END_SIZE;

  *size = (size_t) xmt->header.numBytesDgm;
//fprintf(stderr, "************>%s:%d WRITE mbr_kemkmall_wr_xmt xmt->header.numBytesDgm:%d\n",
//__FILE__, __LINE__, xmt->header.numBytesDgm);

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xmt->header.numBytesDgm);
      fprintf(stderr, "dbg5       dgmType:        %s\n", xmt->header.dgmType);
      fprintf(stderr, "dbg5       dgmVersion:     %u\n", xmt->header.dgmVersion);
      fprintf(stderr, "dbg5       systemID:       %u\n", xmt->header.systemID);
      fprintf(stderr, "dbg5       echoSounderID:  %u\n", xmt->header.echoSounderID);
      fprintf(stderr, "dbg5       time_sec:       %u\n", xmt->header.time_sec);
      fprintf(stderr, "dbg5       time_nanosec:   %u\n", xmt->header.time_nanosec);
    }

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&xmt->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    /* EMdgmMpartition - data partition information */
    mb_put_binary_short(true, xmt->partition.numOfDgms, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, xmt->partition.dgmNum, &buffer[index]);
    index += 2;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numOfDgms:  %d\n", xmt->partition.numOfDgms);
      fprintf(stderr, "dbg5       dgmNum:     %d\n", xmt->partition.dgmNum);
    }

    /* EMdgmMbody - information of transmitter and receiver used to find data in datagram */
    mb_put_binary_short(true, xmt->cmnPart.numBytesCmnPart, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, xmt->cmnPart.pingCnt, &buffer[index]);
    index += 2;
    buffer[index] = xmt->cmnPart.rxFansPerPing;
    index++;
    buffer[index] = xmt->cmnPart.rxFanIndex;
    index++;
    buffer[index] = xmt->cmnPart.swathsPerPing;
    index++;
    buffer[index] = xmt->cmnPart.swathAlongPosition;
    index++;
    buffer[index] = xmt->cmnPart.txTransducerInd;
    index++;
    buffer[index] = xmt->cmnPart.rxTransducerInd;
    index++;
    buffer[index] = xmt->cmnPart.numRxTransducers;
    index++;
    buffer[index] = xmt->cmnPart.algorithmType;
    index++;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesCmnPart:     %d\n", xmt->cmnPart.numBytesCmnPart);
      fprintf(stderr, "dbg5       pingCnt:             %d\n", xmt->cmnPart.pingCnt);
      fprintf(stderr, "dbg5       rxFansPerPing:       %d\n", xmt->cmnPart.rxFansPerPing);
      fprintf(stderr, "dbg5       rxFanIndex:          %d\n", xmt->cmnPart.rxFanIndex);
      fprintf(stderr, "dbg5       swathsPerPing:       %d\n", xmt->cmnPart.swathsPerPing);
      fprintf(stderr, "dbg5       swathAlongPosition:  %d\n", xmt->cmnPart.swathAlongPosition);
      fprintf(stderr, "dbg5       txTransducerInd:     %d\n", xmt->cmnPart.txTransducerInd);
      fprintf(stderr, "dbg5       rxTransducerInd:     %d\n", xmt->cmnPart.rxTransducerInd);
      fprintf(stderr, "dbg5       numRxTransducers:    %d\n", xmt->cmnPart.numRxTransducers);
      fprintf(stderr, "dbg5       algorithmType:       %d\n", xmt->cmnPart.algorithmType);
    }

    /* mbsys_kmbes_xmt_ping_info - ping info */
    mb_put_binary_short(true, xmt->xmtPingInfo.numBytesInfoData, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, xmt->xmtPingInfo.numBytesPerSounding, &buffer[index]);
    index += 2;
    mb_put_binary_int(true, xmt->xmtPingInfo.padding0, &buffer[index]);
    index += 4;
    mb_put_binary_double(true, xmt->xmtPingInfo.longitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, xmt->xmtPingInfo.latitude, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, xmt->xmtPingInfo.sensordepth, &buffer[index]);
    index += 8;
    mb_put_binary_double(true, xmt->xmtPingInfo.heading, &buffer[index]);
    index += 8;
    mb_put_binary_float(true, xmt->xmtPingInfo.speed, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, xmt->xmtPingInfo.roll, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, xmt->xmtPingInfo.pitch, &buffer[index]);
    index += 4;
    mb_put_binary_float(true, xmt->xmtPingInfo.heave, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, xmt->xmtPingInfo.numSoundings, &buffer[index]);
    index += 4;

    if (verbose >= 5) {
      fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
      fprintf(stderr, "dbg5       numBytesInfoData:            %d\n", xmt->xmtPingInfo.numBytesInfoData);
      fprintf(stderr, "dbg5       numBytesPerSounding:         %d\n", xmt->xmtPingInfo.numBytesPerSounding);
      fprintf(stderr, "dbg5       padding0:                    %d\n", xmt->xmtPingInfo.padding0);
      fprintf(stderr, "dbg5       longitude:                   %f\n", xmt->xmtPingInfo.longitude);
      fprintf(stderr, "dbg5       latitude:                    %f\n", xmt->xmtPingInfo.latitude);
      fprintf(stderr, "dbg5       sensordepth:                 %f\n", xmt->xmtPingInfo.sensordepth);
      fprintf(stderr, "dbg5       heading:                     %f\n", xmt->xmtPingInfo.heading);
      fprintf(stderr, "dbg5       speed:                       %f\n", xmt->xmtPingInfo.speed);
      fprintf(stderr, "dbg5       roll:                        %f\n", xmt->xmtPingInfo.roll);
      fprintf(stderr, "dbg5       pitch:                       %f\n", xmt->xmtPingInfo.pitch);
      fprintf(stderr, "dbg5       heave:                       %f\n", xmt->xmtPingInfo.heave);
      fprintf(stderr, "dbg5       numSoundings:                %d\n", xmt->xmtPingInfo.numSoundings);
    }

    /* EMdgmMRZ_sounding - Data for each sounding */
    //numSidescanSamples = 0;
    //numSoundings = xmt->rxInfo.numSoundingsMaxMain + xmt->rxInfo.numExtraDetections;
    for (int i = 0; i < xmt->xmtPingInfo.numSoundings; i++) {
      mb_put_binary_short(true, xmt->xmtSounding[i].soundingIndex, &buffer[index]);
      index += 2;
      mb_put_binary_short(true, xmt->xmtSounding[i].padding0, &buffer[index]);
      index += 2;
      mb_put_binary_float(true, xmt->xmtSounding[i].twtt, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, xmt->xmtSounding[i].angle_vertical, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, xmt->xmtSounding[i].angle_azimuthal, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, xmt->xmtSounding[i].beam_heave, &buffer[index]);
      index += 4;
      mb_put_binary_float(true, xmt->xmtSounding[i].alongtrack_offset, &buffer[index]);
      index += 4;

      if (verbose >= 5) {
        fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
        fprintf(stderr, "dbg5       soundingIndex:                  %d\n", xmt->xmtSounding[i].soundingIndex);
        fprintf(stderr, "dbg5       padding0:                       %d\n", xmt->xmtSounding[i].padding0);
        fprintf(stderr, "dbg5       twtt:                           %f\n", xmt->xmtSounding[i].twtt);
        fprintf(stderr, "dbg5       angle_vertical:                 %f\n", xmt->xmtSounding[i].angle_vertical);
        fprintf(stderr, "dbg5       angle_azimuthal:                %f\n", xmt->xmtSounding[i].angle_azimuthal);
        fprintf(stderr, "dbg5       beam_heave:                     %f\n", xmt->xmtSounding[i].beam_heave);
        fprintf(stderr, "dbg5       alongtrack_offset:              %f\n", xmt->xmtSounding[i].alongtrack_offset);
      }
    }

    /* Insert closing byte count */
    mb_put_binary_int(true, xmt->header.numBytesDgm, &buffer[index]);
    index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          xmt->header.dgmType, *size, index, xmt->header.time_sec, xmt->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_xms(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  // size_t numBytesComment = 0;
  char *buffer = NULL;
  int index = 0;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  struct mbsys_kmbes_xms *xms = &(store->xms);

  /* datagram version being written */
  xms->header.dgmVersion = MBSYS_KMBES_XMS_VERSION;

  if (verbose >= 5) {
    fprintf(stderr, "\ndbg5  Values to be written in MBIO function <%s>\n", __func__);
    fprintf(stderr, "dbg5       numBytesDgm:    %u\n", xms->header.numBytesDgm);
    fprintf(stderr, "dbg5       dgmType:        %s\n", xms->header.dgmType);
    fprintf(stderr, "dbg5       dgmVersion:     %u\n", xms->header.dgmVersion);
    fprintf(stderr, "dbg5       systemID:       %u\n", xms->header.systemID);
    fprintf(stderr, "dbg5       echoSounderID:  %u\n", xms->header.echoSounderID);
    fprintf(stderr, "dbg5       time_sec:       %u\n", xms->header.time_sec);
    fprintf(stderr, "dbg5       time_nanosec:   %u\n", xms->header.time_nanosec);

    fprintf(stderr, "dbg5       pingCnt:        %u\n", xms->pingCnt);
    fprintf(stderr, "dbg5       spare:          %d\n", xms->spare);
    fprintf(stderr, "dbg5       pixel_size:     %f\n", xms->pixel_size);
    fprintf(stderr, "dbg5       pixels_ss:      %d\n", xms->pixels_ss);
    for (int i=0;i<32;i++)
      fprintf(stderr, "dbg5       unused[%2d]:    %u\n", i, xms->unused[i]);
    for (int i=0;i<xms->pixels_ss;i++)
      fprintf(stderr, "dbg5       ss[%2d]:        %f %f\n",
                      i, xms->ss[i], xms->ss_alongtrack[i]);
  }

  /* size of output record */
  *size = (size_t) xms->header.numBytesDgm;

  int status = MB_SUCCESS;

  /* allocate memory to write rest of record if necessary */
  if (*bufferalloc < *size) {
    status = mb_reallocd(verbose, __FILE__, __LINE__, *size, (void **)bufferptr, error);
    if (status != MB_SUCCESS)
      *bufferalloc = 0;
    else
      *bufferalloc = *size;
  }

  /* proceed to write if buffer allocated */
  if (status == MB_SUCCESS) {
    /* get buffer for writing */
    buffer = (char *) *bufferptr;

    /* insert the header */
    mbr_kemkmall_wr_header(verbose, bufferptr, (void *)&xms->header, error);

    /* insert the data */
    index = MBSYS_KMBES_HEADER_SIZE;

    mb_put_binary_short(true, xms->pingCnt, &buffer[index]);
    index += 2;
    mb_put_binary_short(true, xms->spare, &buffer[index]);
    index += 2;
    mb_put_binary_float(true, xms->pixel_size, &buffer[index]);
    index += 4;
    mb_put_binary_int(true, xms->pixels_ss, &buffer[index]);
    index += 4;
    for (int i=0;i<32;i++) {
      buffer[index] = xms->unused[i];
      index++;
    }
  for (int i=0;i<xms->pixels_ss;i++) {
    mb_put_binary_float(true, xms->ss[i], &buffer[index]);
    index += 4;
  }
  for (int i=0;i<xms->pixels_ss;i++) {
    mb_put_binary_float(true, xms->ss_alongtrack[i], &buffer[index]);
    index += 4;
  }

    /* Insert closing byte count */
    mb_put_binary_int(true, xms->header.numBytesDgm, &buffer[index]);
    // index += 4;
  }

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       size:       %zu\n", *size);
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type %.4s written - size: %lu %d time: %d.%9.9d status:%d error:%d\n",
          xms->header.dgmType, *size, index, xms->header.time_sec, xms->header.time_nanosec, status, *error);
#endif

  /* return status */
  return (status);
};

/*--------------------------------------------------------------------*/

int mbr_kemkmall_wr_unknown(int verbose, size_t *bufferalloc, char **bufferptr, void *store_ptr, size_t *size, int *error) {
  (void)size;  // unused

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       bufferalloc:%zu\n", *bufferalloc);
    fprintf(stderr, "dbg2       bufferptr:  %p\n", (void *)bufferptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* get pointer to raw data structure */
  // struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  const int status = MB_SUCCESS;

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "KEMKMALL datagram type unknown written - status:%d error:%d\n",
  status, *error);
#endif

  /* return status */
  return (status);

};
/*--------------------------------------------------------------------*/
int mbr_kemkmall_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  // size_t write_len = 0;
  char **bufferptr = NULL;
  size_t *bufferalloc = NULL;
  size_t size = 0;

  /* get pointer to mbio descriptor */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* get buffer and related vars from mbio saved values */
  bufferptr = (char **)&mb_io_ptr->raw_data;
  bufferalloc = (size_t *)&mb_io_ptr->structure_size;
  // char *buffer = (char *)*bufferptr;

  /* get pointer to raw data structure */
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;

  int status = MB_SUCCESS;

  /* write the current data record type */
  switch (store->kind) {

    case MB_DATA_INSTALLATION:
      /* #IIP - Info Installation PU */
      status = mbr_kemkmall_wr_iip(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      status = mbr_kemkmall_wr_xmb(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_RUN_PARAMETER:
      /* #IOP -  Runtime datagram */
      status = mbr_kemkmall_wr_iop(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_BIST:
      /* #IBE (BIST Error report) */
      status = mbr_kemkmall_wr_ibe(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_BIST1:
      /* #IBR (BIST reply) */
      status = mbr_kemkmall_wr_ibr(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_BIST2:
      /* #IBS (BIST short reply) */
      status = mbr_kemkmall_wr_ibs(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_NAV:
      /* #SPO - Sensor POsition data */
      status = mbr_kemkmall_wr_spo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_NAV1:
      /* #SKM - KM binary sensor data */
      status = mbr_kemkmall_wr_skm(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_VELOCITY_PROFILE:
      /* #SVP - Sound Velocity Profile */
      status = mbr_kemkmall_wr_svp(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_SSV:
      /* #SVT - Sensor sound Velocity measured at Transducer */
      status = mbr_kemkmall_wr_svt(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_CLOCK:
      /* #SCL - Sensor CLock datagram */
      status = mbr_kemkmall_wr_scl(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_SENSORDEPTH:
      /* #SDE - Sensor DEpth data */
      status = mbr_kemkmall_wr_sde(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_HEIGHT:
      /* #SHI - Sensor HeIght data */
      status = mbr_kemkmall_wr_shi(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_HEADING:
      /* #SHA - Sensor HeAding */
      status = mbr_kemkmall_wr_sha(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_DATA:
      /* #MRZ - multibeam data for raw range,
      depth, reflectivity, seabed image(SI) etc. */
      for (int imrz=0;imrz<store->n_mrz_read;imrz++) {
        status = mbr_kemkmall_wr_mrz(verbose, bufferalloc, bufferptr, store_ptr, imrz, &size, error);
        if (status == MB_SUCCESS)
          status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      }
      /* #XMT - multibeam corrected beam angles and travel times (MB-System only) */
      for (int ixmt=0;ixmt<store->n_mrz_read;ixmt++) {
        status = mbr_kemkmall_wr_xmt(verbose, bufferalloc, bufferptr, store_ptr, ixmt, &size, error);
        if (status == MB_SUCCESS)
          status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      }
      /* #XMS - multibeam pseudosidescan (MB-System only) */
      status = mbr_kemkmall_wr_xms(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_WATER_COLUMN:
      /* #MWC - multibeam water column datagram */
      for (int imwc=0;imwc< store->n_mwc_read;imwc++) {
        status = mbr_kemkmall_wr_mwc(verbose, bufferalloc, bufferptr, store_ptr, imwc, &size, error);
        if (status == MB_SUCCESS)
          status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      }
      break;

    case MB_DATA_NAV2:
      /* #CPO - Compatibility position sensor data */
      status = mbr_kemkmall_wr_cpo(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_HEAVE:
      /* #CHE - Compatibility heave data */
      status = mbr_kemkmall_wr_che(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_BSCALIBRATIONFILE:
      /* #FCF - Backscatter calibration file datagram */
      status = mbr_kemkmall_wr_fcf(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_MBSYSTEM:
      /* #XMB - Indicates these data were written by MB-System (MB-System only) */
      /* Always written immediately after the #IIP datagram */
      //status = mbr_kemkmall_wr_xmb(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      //if (status == MB_SUCCESS)
      //  status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case MB_DATA_COMMENT:
      /* #XMC - Comment datagram (MB-System only) */
      status = mbr_kemkmall_wr_xmc(verbose, bufferalloc, bufferptr, store_ptr, &size, error);
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    case UNKNOWN:
      /* Unknown datagram format */
      status = mbr_kemkmall_wr_unknown(verbose, bufferalloc, bufferptr, store_ptr, &size, error); // TODO: implement!
      if (status == MB_SUCCESS)
        status = mb_fileio_put(verbose, mbio_ptr, (char *)(*bufferptr), &size, error);
      break;

    default:
        /* should never get here */
        status = MB_FAILURE;
        break;
  }

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "Data record written: type:%d status:%d error:%d\n", store->kind, status, *error);
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbr_wt_kemkmall(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
    fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
    fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
  }

  /* check for non-null pointers */
  assert(mbio_ptr != NULL);
  assert(store_ptr != NULL);

#ifdef MBR_KEMKMALL_DEBUG
  struct mbsys_kmbes_struct *store = (struct mbsys_kmbes_struct *)store_ptr;
  fprintf(stderr, "\nAbout to call mbr_kemkmall_wr_data record kind:%d\n", store->kind);
#endif

  /* write next data to file */
  const int status = mbr_kemkmall_wr_data(verbose, mbio_ptr, store_ptr, error);

#ifdef MBR_KEMKMALL_DEBUG
  fprintf(stderr, "Done with mbr_kemkmall_wr_data: status:%d error:%d\n", status, *error);
#endif

  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", __func__);
    fprintf(stderr, "dbg2  Return values:\n");
    fprintf(stderr, "dbg2       error:      %d\n", *error);
    fprintf(stderr, "dbg2  Return status:\n");
    fprintf(stderr, "dbg2       status:  %d\n", status);
  }

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/
int mbr_register_kemkmall(int verbose, void *mbio_ptr, int *error) {
  if (verbose >= 2) {
    fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", __func__);
    fprintf(stderr, "dbg2  Input arguments:\n");
    fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
  }

  /* get mb_io_ptr */
  struct mb_io_struct *mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

  /* set format info parameters */
  const int status = mbr_info_kemkmall(
      verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
      mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
      &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
      &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
      &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

  /* set format and system specific function pointers */
  mb_io_ptr->mb_io_format_alloc = &mbr_alm_kemkmall;
  mb_io_ptr->mb_io_format_free = &mbr_dem_kemkmall;
  mb_io_ptr->mb_io_store_alloc = &mbsys_kmbes_alloc;
  mb_io_ptr->mb_io_store_free = &mbsys_kmbes_deall;
  mb_io_ptr->mb_io_read_ping = &mbr_rt_kemkmall;
  mb_io_ptr->mb_io_write_ping = &mbr_wt_kemkmall;
  mb_io_ptr->mb_io_dimensions = &mbsys_kmbes_dimensions;
  mb_io_ptr->mb_io_pingnumber = &mbsys_kmbes_pingnumber;
  mb_io_ptr->mb_io_sonartype = &mbsys_kmbes_sonartype;
  mb_io_ptr->mb_io_sidescantype =&mbsys_kmbes_sidescantype;
  mb_io_ptr->mb_io_preprocess = &mbsys_kmbes_preprocess;
  mb_io_ptr->mb_io_extract = &mbsys_kmbes_extract;
  mb_io_ptr->mb_io_insert = &mbsys_kmbes_insert;
  mb_io_ptr->mb_io_extract_nav = &mbsys_kmbes_extract_nav;
//  mb_io_ptr->mb_io_extract_nnav = &mbsys_kmbes_extract_nnav;
  mb_io_ptr->mb_io_insert_nav = &mbsys_kmbes_insert_nav;
  mb_io_ptr->mb_io_extract_altitude = &mbsys_kmbes_extract_altitude;
  mb_io_ptr->mb_io_insert_altitude = NULL;
  mb_io_ptr->mb_io_extract_svp = &mbsys_kmbes_extract_svp;
  mb_io_ptr->mb_io_insert_svp = &mbsys_kmbes_insert_svp;
  mb_io_ptr->mb_io_ttimes = &mbsys_kmbes_ttimes;
  mb_io_ptr->mb_io_detects = &mbsys_kmbes_detects;
  mb_io_ptr->mb_io_pulses = &mbsys_kmbes_pulses;
  mb_io_ptr->mb_io_gains = &mbsys_kmbes_gains;
  mb_io_ptr->mb_io_copyrecord = &mbsys_kmbes_copy;
  mb_io_ptr->mb_io_makess = &mbsys_kmbes_makess;
  mb_io_ptr->mb_io_extract_rawss = NULL;
  mb_io_ptr->mb_io_insert_rawss = NULL;
  mb_io_ptr->mb_io_extract_segytraceheader = NULL;
  mb_io_ptr->mb_io_extract_segy = NULL;
  mb_io_ptr->mb_io_insert_segy = NULL;
  mb_io_ptr->mb_io_ctd = NULL;
  mb_io_ptr->mb_io_ancilliarysensor = NULL;

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

  /* return status */
  return (status);
}

/*--------------------------------------------------------------------*/
