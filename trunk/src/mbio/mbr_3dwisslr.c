/*--------------------------------------------------------------------
 *    The MB-system:	mbr_3dwisslr.c	2/11/93
 *	$Id$
 *
 *    Copyright (c) 1993-2017 by
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
 * mbr_3dwisslr.c contains the functions for reading and writing
 * multibeam data in the MBSYS_3DDWISSL format.
 * These functions include:
 *   mbr_alm_3dwisslr	- allocate read/write memory
 *   mbr_dem_3dwisslr	- deallocate read/write memory
 *   mbr_rt_3dwisslr	- read and translate data
 *   mbr_wt_3dwisslr	- translate and write data
 *
 * Author:	D. W. Caress
 * Date:	December 27, 2013
 *
 * $Log: mbr_3dwisslr.c,v $
 *
 */

/* standard include files */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>

/* mbio include files */
#include "mb_status.h"
#include "mb_format.h"
#include "mb_io.h"
#include "mb_define.h"
#include "mbsys_3ddwissl.h"

/* include for byte swapping on little-endian machines */
#ifdef BYTESWAPPED
#include "mb_swap.h"
#endif

/* essential function prototypes */
int mbr_register_3dwisslr(int verbose, void *mbio_ptr, int *error);
int mbr_info_3dwisslr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error);
int mbr_alm_3dwisslr(int verbose, void *mbio_ptr, int *error);
int mbr_dem_3dwisslr(int verbose, void *mbio_ptr, int *error);
int mbr_rt_3dwisslr(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_wt_3dwisslr(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_3dwisslr_index_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_3dwisslr_indextable_compare(const void *a, const void *b);
int mbr_3dwisslr_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);
int mbr_3dwisslr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error);

static char rcs_id[] = "$Id$";


/*--------------------------------------------------------------------*/
int mbr_register_3dwisslr(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_register_3dwisslr";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* check for non-null structure pointers */
	assert(mbio_ptr != NULL);

	/* get mb_io_ptr */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set format info parameters */
	status = mbr_info_3dwisslr(
	    verbose, &mb_io_ptr->system, &mb_io_ptr->beams_bath_max, &mb_io_ptr->beams_amp_max, &mb_io_ptr->pixels_ss_max,
	    mb_io_ptr->format_name, mb_io_ptr->system_name, mb_io_ptr->format_description, &mb_io_ptr->numfile, &mb_io_ptr->filetype,
	    &mb_io_ptr->variable_beams, &mb_io_ptr->traveltime, &mb_io_ptr->beam_flagging, &mb_io_ptr->platform_source,
	    &mb_io_ptr->nav_source, &mb_io_ptr->sensordepth_source, &mb_io_ptr->heading_source, &mb_io_ptr->attitude_source,
	    &mb_io_ptr->svp_source, &mb_io_ptr->beamwidth_xtrack, &mb_io_ptr->beamwidth_ltrack, error);

	/* set format and system specific function pointers */
	mb_io_ptr->mb_io_format_alloc = &mbr_alm_3dwisslr;
	mb_io_ptr->mb_io_format_free = &mbr_dem_3dwisslr;
	mb_io_ptr->mb_io_store_alloc = &mbsys_3ddwissl_alloc;
	mb_io_ptr->mb_io_store_free = &mbsys_3ddwissl_deall;
	mb_io_ptr->mb_io_read_ping = &mbr_rt_3dwisslr;
	mb_io_ptr->mb_io_write_ping = &mbr_wt_3dwisslr;
	mb_io_ptr->mb_io_dimensions = &mbsys_3ddwissl_dimensions;
	mb_io_ptr->mb_io_preprocess = &mbsys_3ddwissl_preprocess;
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

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
		fprintf(stderr, "dbg2       preprocess:         %p\n", (void *)mb_io_ptr->mb_io_preprocess);
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

	/* return status */
	return (status);
}

/*--------------------------------------------------------------------*/
int mbr_info_3dwisslr(int verbose, int *system, int *beams_bath_max, int *beams_amp_max, int *pixels_ss_max, char *format_name,
                      char *system_name, char *format_description, int *numfile, int *filetype, int *variable_beams,
                      int *traveltime, int *beam_flagging, int *platform_source, int *nav_source, int *sensordepth_source,
                      int *heading_source, int *attitude_source, int *svp_source, double *beamwidth_xtrack,
                      double *beamwidth_ltrack, int *error) {
	char *function_name = "mbr_info_3dwisslr";
	int status = MB_SUCCESS;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
	}

	/* set format info parameters */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	*system = MB_SYS_3DDWISSL;
	*beams_bath_max = 0;
	*beams_amp_max = 0;
	*pixels_ss_max = 0;
	strncpy(format_name, "3DDWISSL", MB_NAME_LENGTH);
	strncpy(system_name, "3DDWISSL", MB_NAME_LENGTH);
	strncpy(format_description,
	        "Format name:          MBSYS_3DDWISSL\nInformal Description: 3DatDepth prototype binary swath mapping LIDAR "
	        "format\nAttributes:           3DatDepth LIDAR, variable pulses, bathymetry and amplitude, \n                      "
	        "binary, 3DatDepth.\n",
	        MB_DESCRIPTION_LENGTH);
	*numfile = 1;
	*filetype = MB_FILETYPE_NORMAL;
	*variable_beams = MB_YES;
	*traveltime = MB_NO;
	*beam_flagging = MB_YES;
	*platform_source = MB_DATA_NONE;
	*nav_source = MB_DATA_DATA;
	*sensordepth_source = MB_DATA_DATA;
	*heading_source = MB_DATA_DATA;
	*attitude_source = MB_DATA_DATA;
	*svp_source = MB_DATA_NONE;
	*beamwidth_xtrack = 0.02;
	*beamwidth_ltrack = 0.02;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
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
int mbr_alm_3dwisslr(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_alm_3dwisslr";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	int *file_header_readwritten;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set initial status */
	status = MB_SUCCESS;

	/* allocate memory for data structure */
	status = mbsys_3ddwissl_alloc(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;

	/* set file header read flag */
	file_header_readwritten = (int *)&mb_io_ptr->save1;
	*file_header_readwritten = MB_NO;

	/* set saved bytes flag */
	mb_io_ptr->save2 = MB_NO;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_dem_3dwisslr(int verbose, void *mbio_ptr, int *error) {
	char *function_name = "mbr_dem_3dwisslr";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
    
    /* deallocate reading/writing buffer */
    if (mb_io_ptr->data_structure_size > 0 && mb_io_ptr->raw_data != NULL) {
        status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->raw_data), error);
        mb_io_ptr->raw_data = NULL;
        mb_io_ptr->data_structure_size = 0;
    }
    
    /* deallocate file indexing array */
    if (mb_io_ptr->num_indextable_alloc > 0 && mb_io_ptr->indextable != NULL) {
        status = mb_freed(verbose, __FILE__, __LINE__, (void **)(&mb_io_ptr->indextable), error);
        mb_io_ptr->indextable = NULL;
        mb_io_ptr->num_indextable = 0;
        mb_io_ptr->num_indextable_alloc = 0;
    }

	/* deallocate memory  */
	status = mbsys_3ddwissl_deall(verbose, mbio_ptr, &mb_io_ptr->store_data, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_rt_3dwisslr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_rt_3dwisslr";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
    int *file_indexed;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointers to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_3ddwissl_struct *)store_ptr;
    
	/* get saved values */
	file_indexed = (int *)&mb_io_ptr->save2;

	/* if needed index the file */
    if (*file_indexed == MB_NO) {
        status = mbr_3dwisslr_index_data(verbose, mbio_ptr, store_ptr, error);
    }

	/* read next data from file */
	status = mbr_3dwisslr_rd_data(verbose, mbio_ptr, store_ptr, error);

	/* if needed calculate bathymetry */
	if (status == MB_SUCCESS && store->kind == MB_DATA_DATA && store->bathymetry_calculated == MB_NO) {
		mbsys_3ddwissl_calculatebathymetry(verbose, mbio_ptr, store_ptr, error);
	}

	/* print out status info */
	if (verbose > 1)
		mbsys_3ddwissl_print_store(verbose, store_ptr, error);

	/* set error and kind in mb_io_ptr */
	mb_io_ptr->new_error = *error;
	mb_io_ptr->new_kind = store->kind;

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_wt_3dwisslr(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_wt_3dwisslr";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointers to mbio descriptor and data structure */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* write next data to file */
	status = mbr_3dwisslr_wr_data(verbose, mbio_ptr, store_ptr, error);

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
int mbr_3dwisslr_index_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_3dwisslr_rd_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
    int *file_indexed;
	int *file_header_readwritten;
	char *buffer = NULL;
	size_t read_len;
	size_t index;
	unsigned short magic_number = 0;
	int time_i[7];
    double time_d;
	int done;
	int i;
    int count = 0;
	int skip;
	int valid_id;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get saved values */
	file_indexed = (int *)&mb_io_ptr->save2;

	/* set file position */
	mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
	done = MB_NO;

	/* read the fileheader, which is returned as a parameter record */
		/* calculate size of file header and allocate read buffer */
    read_len = (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE
                        + 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
    if (mb_io_ptr->data_structure_size < read_len) {
        status = mb_reallocd(verbose, __FILE__, __LINE__, read_len, (void **)(&mb_io_ptr->raw_data), error);
        if (status == MB_SUCCESS) {
            mb_io_ptr->data_structure_size = read_len;
            buffer = mb_io_ptr->raw_data;
        }
    }
    
    /* allocate indexing array */
    if (mb_io_ptr->num_indextable_alloc <= mb_io_ptr->num_indextable) {
        mb_io_ptr->num_indextable_alloc += MB_BUFFER_MAX;
        status = mb_reallocd(verbose, __FILE__, __LINE__,
                                mb_io_ptr->num_indextable_alloc * sizeof(struct mb_io_indextable_struct),
                                (void **)(&mb_io_ptr->indextable), error);
    }
    
    /* read file header and check the first few bytes */
    count = 0;
    if (status == MB_SUCCESS)
        status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
    if (status == MB_SUCCESS) {
        index = 0;
        mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->parameter_id)); index += 2;
        mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->magic_number)); index += 2;

        /* if ok and parameter_id is for the fileheader and the magic number is correct
         * then parse the start of the file header */
        if (status == MB_SUCCESS
            && store->parameter_id == MBSYS_3DDWISSL_RECORD_FILEHEADER
            && store->magic_number == MBF_3DWISSLR_MAGICNUMBER) {

            /* get scan information */
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->file_version)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->sub_version)); index += 2;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->cross_track_angle_start)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->cross_track_angle_end)); index += 4;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->pulses_per_scan)); index += 2;
            store->soundings_per_pulse = buffer[index]; index += 1;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->heada_scans_per_file)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->headb_scans_per_file)); index += 2;

            /* calculate size of a processed scan record and allocate read buffer and pulses array */
            store->scan_count = store->heada_scans_per_file + store->headb_scans_per_file;
            store->size_pulse_record_raw
                = MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE
                    + store->pulses_per_scan
                        * (MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE
                            + store->soundings_per_pulse * MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE);
            if (mb_io_ptr->data_structure_size < store->size_pulse_record_raw) {
                status = mb_reallocd(verbose, __FILE__, __LINE__, store->size_pulse_record_raw,
                                     (void **)(&mb_io_ptr->raw_data), error);
                if (status == MB_SUCCESS) {
                    mb_io_ptr->data_structure_size = store->size_pulse_record_raw;
                    buffer = mb_io_ptr->raw_data;
                }
            }
    
            /* allocate indexing array */
            if (mb_io_ptr->num_indextable_alloc <= store->scan_count) {
                mb_io_ptr->num_indextable_alloc = store->scan_count;
                status = mb_reallocd(verbose, __FILE__, __LINE__,
                                        mb_io_ptr->num_indextable_alloc * sizeof(struct mb_io_indextable_struct),
                                        (void **)(&mb_io_ptr->indextable), error);
            }
            
            /* augment the index table */
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].count = count;
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].time_d = (double)(mb_io_ptr->num_indextable);
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].offset = 0;
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].size = read_len;
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].kind = (mb_u_char) MB_DATA_PARAMETER;
            mb_io_ptr->indextable[mb_io_ptr->num_indextable].read = (mb_u_char) 0;
            mb_io_ptr->num_indextable++;
            count++;
        }
        
        /* else this is not a first generation WISSL file, set error */
        else {
            status = MB_FAILURE;
            *error = MB_ERROR_BAD_FORMAT;
            store->kind = MB_DATA_NONE;
            done = MB_YES;
        }
    }
    
    /* read subsequent data records */
    while (done == MB_NO) {
		/* read and check two bytes until a valid record_id is found */
		read_len = (size_t)sizeof(short);
        buffer = mb_io_ptr->raw_data;
		valid_id = MB_NO;
		skip = 0;
		status = mb_fileio_get(verbose, mbio_ptr, (void *)buffer, &read_len, error);
		do {
			if (status == MB_SUCCESS) {
                memcpy(&store->record_id, buffer, sizeof(short));
				if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA
                    || store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADB
                    || store->record_id == MBSYS_3DDWISSL_RECORD_COMMENT) {
					valid_id = MB_YES;
				}
				else {
//fprintf(stderr,"%s:%s():%d SKIP BAD RECORD ID: %x %x %x %d skip:%d valid_id:%d status:%d error:%d\n",
//__FILE__, __FUNCTION__, __LINE__, (mb_u_char)buffer[0], (mb_u_char)buffer[1], store->record_id,store->record_id,skip,valid_id,status,*error);
					skip++;
                    buffer[0] = buffer[1];
                    read_len = (size_t)sizeof(char);
                    status = mb_fileio_get(verbose, mbio_ptr, (void *)&(buffer[1]), &read_len, error);
				}
			}
            else {
                store->record_id = 0;
            }
		} while (status == MB_SUCCESS && valid_id == MB_NO);
//fprintf(stderr,"%s:%s():%d RECORD ID: %x %d skip:%d valid_id:%d status:%d error:%d\n",
//__FILE__, __FUNCTION__, __LINE__, store->record_id,store->record_id,skip,valid_id,status,*error);

		/* read MBSYS_3DDWISSL_RECORD_RAWHEADA or MBSYS_3DDWISSL_RECORD_RAWHEADB record */
        if (status == MB_SUCCESS
            && (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA
                || store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADB)) {
//if (store->record_id == MBSYS_3DDWISSL_RECORD_RAWHEADA)
//fprintf(stderr,"%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_RAWHEADA\n",
//__FILE__, __FUNCTION__, __LINE__);
//else
//fprintf(stderr,"%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_RAWHEADB\n",
//__FILE__, __FUNCTION__, __LINE__);
            read_len = (size_t)(store->size_pulse_record_raw - 2);
            buffer = mb_io_ptr->raw_data;
            status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
            if (status == MB_SUCCESS) {
                index = 0;
                mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->year)); index += 2;
                store->month = buffer[index]; index += 1;
                store->day = buffer[index]; index += 1;
                mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->jday)); index += 2;
                mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->hour)); index += 2;
                store->minutes = buffer[index]; index += 1;
                store->seconds = buffer[index]; index += 1;
                mb_get_binary_int(MB_YES, (void *)&buffer[index], &(store->nanoseconds)); index += 4;
                        
                /* get the timestamp */
                time_i[0] = store->year;
                time_i[1] = store->month;
                time_i[2] = store->day;
                time_i[3] = store->hour;
                time_i[4] = store->minutes;
                time_i[5] = store->seconds;
                time_i[6] = (int)(0.001 * store->nanoseconds);
                mb_get_time(verbose, time_i, &time_d);
            
                /* augment the index table */
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].count = count;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].time_d = time_d;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].offset = ftell(mb_io_ptr->mbfp) - store->size_pulse_record_raw;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].size = store->size_pulse_record_raw;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].kind = (mb_u_char) MB_DATA_DATA;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].read = (mb_u_char) 0;
                mb_io_ptr->num_indextable++;
                count++;
            }
            else {
                done = MB_YES;
            }
        }

		/* read comment record */
        else if (status == MB_SUCCESS
            && store->record_id == MBSYS_3DDWISSL_RECORD_COMMENT) {
//fprintf(stderr,"%s:%s():%d Reading MBSYS_3DDWISSL_RECORD_COMMENT\n",
//__FILE__, __FUNCTION__, __LINE__);
            read_len = (size_t)(2);
            buffer = mb_io_ptr->raw_data;
            status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
            if (status == MB_SUCCESS) {
                index = 0;
                mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->comment_len)); index += 2;
                read_len = (size_t)(MIN(store->comment_len, MB_COMMENT_MAXLINE-1));
                memset(store->comment, 0, MB_COMMENT_MAXLINE);
                status = mb_fileio_get(verbose, mbio_ptr, store->comment, &read_len, error);
          
                /* augment the index table */
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].count = count;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].time_d = (double) mb_io_ptr->num_indextable;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].offset = ftell(mb_io_ptr->mbfp) - (long)(read_len + 4);
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].size = read_len + 4;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].kind = (mb_u_char) MB_DATA_COMMENT;
                mb_io_ptr->indextable[mb_io_ptr->num_indextable].read = (mb_u_char) 0;
                mb_io_ptr->num_indextable++;
                count++;
            }
            done = MB_YES;
            if (status == MB_SUCCESS)
                store->kind = MB_DATA_COMMENT;
        }
        
        /* else if failure done */
        else if (status != MB_SUCCESS) {
            done = MB_YES;
        }
    }
//fprintf(stderr,"%s:%s():%d END of read loop: status:%d error:%d kind:%d\n",
//__FILE__, __FUNCTION__, __LINE__, status,*error,store->kind);
    
    /* set indexed flag */
    *file_indexed = MB_YES;
    if (mb_io_ptr->num_indextable > 0) {
        status = MB_SUCCESS;
        *error = MB_ERROR_NO_ERROR;
    }
    
    /* set file position back to the start */
    fseek(mb_io_ptr->mbfp, 0, SEEK_SET);

	/* sort the index table */
	if (status == MB_SUCCESS) {
        qsort((void *)mb_io_ptr->indextable, mb_io_ptr->num_indextable,
              sizeof(struct mb_io_indextable_struct),
              (void *)mbr_3dwisslr_indextable_compare);
//for (i=0;i<mb_io_ptr->num_indextable;i++) {
//fprintf(stderr,"%4.4d %4.4d %15.4f %3d %5ld %lu\n",
//i, mb_io_ptr->indextable[i].count, mb_io_ptr->indextable[i].time_d, mb_io_ptr->indextable[i].kind,
//mb_io_ptr->indextable[i].offset, mb_io_ptr->indextable[i].size);
//}
    }

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
int mbr_3dwisslr_indextable_compare(const void *a, const void *b) {
    struct mb_io_indextable_struct *aa;
    struct mb_io_indextable_struct *bb;
    int result = 0;
    
    aa = (struct mb_io_indextable_struct*) a;
    bb = (struct mb_io_indextable_struct*) b;
    if (aa->time_d < bb->time_d)
        result = -1;
    else if (aa->time_d > bb->time_d)
        result = 1;
    return(result);
}

/*--------------------------------------------------------------------*/
int mbr_3dwisslr_rd_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_3dwisslr_rd_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_calibration_struct *calibration;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	int *file_header_readwritten;
	char *buffer = NULL;
	size_t read_len;
	size_t index;
	int time_i[7];
	int found;
	int i, ipulse, isounding;
	int irecord;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get saved values */
	file_header_readwritten = (int *)&mb_io_ptr->save1;

	/* set file position */
	mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;
    
    /* find next unread record in the file index table */
    found = MB_NO;
    for (i=0; i<mb_io_ptr->num_indextable && found == MB_NO; i++) {
        if (mb_io_ptr->indextable[i].read == MB_NO) {
            found = MB_YES;
            irecord = i;
        }
    }
    
    /* read the next record */
    if (found == MB_YES) {
        /* set the file offset */
        fseek(mb_io_ptr->mbfp, mb_io_ptr->indextable[irecord].offset, SEEK_SET);
        
        /* read the next record into the buffer */
        buffer = mb_io_ptr->raw_data;
        read_len = mb_io_ptr->indextable[irecord].size;
        status = mb_fileio_get(verbose, mbio_ptr, buffer, &read_len, error);
        mb_io_ptr->indextable[irecord].read = MB_YES;
        
        /* parse a file header */
        if (status == MB_SUCCESS
            && mb_io_ptr->indextable[irecord].kind == MB_DATA_PARAMETER) {

			index = 0;
			mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->parameter_id)); index += 2;
			mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->magic_number)); index += 2;
            
            /* get scan information */
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->file_version)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->sub_version)); index += 2;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->cross_track_angle_start)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->cross_track_angle_end)); index += 4;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->pulses_per_scan)); index += 2;
            store->soundings_per_pulse = buffer[index]; index += 1;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->heada_scans_per_file)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->headb_scans_per_file)); index += 2;

            /* calculate size of a processed scan record and allocate read buffer and pulses array */
            store->scan_count = store->heada_scans_per_file + store->headb_scans_per_file;
            store->size_pulse_record_raw
                = MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE
                    + store->pulses_per_scan
                        * (MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE
                            + store->soundings_per_pulse * MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE);
            store->size_pulse_record_processed 
                = MBSYS_3DDWISSL_V1S1_PRO_SCAN_HEADER_SIZE
                    + store->pulses_per_scan
                        * (MBSYS_3DDWISSL_V1S1_PRO_PULSE_HEADER_SIZE
                            + store->soundings_per_pulse * MBSYS_3DDWISSL_V1S1_PRO_SOUNDING_SIZE);
            if (mb_io_ptr->data_structure_size < store->size_pulse_record_raw) {
                status = mb_reallocd(verbose, __FILE__, __LINE__, store->size_pulse_record_raw,
                                     (void **)(&mb_io_ptr->raw_data), error);
                if (status == MB_SUCCESS) {
                    mb_io_ptr->data_structure_size = store->size_pulse_record_raw;
                    buffer = mb_io_ptr->raw_data;
                }
            }
            if (store->num_pulses_alloc < store->pulses_per_scan) {
                read_len = store->pulses_per_scan * sizeof(struct mbsys_3ddwissl_pulse_struct);
                status = mb_reallocd(verbose, __FILE__, __LINE__, read_len, (void **)(&store->pulses), error);
                if (status == MB_SUCCESS) {
                    store->num_pulses_alloc = store->pulses_per_scan;
                }
            }
//fprintf(stderr,"%s:%s():%d INDEX at end of scan information: %zu  size_pulse_record_raw:%d size_pulse_record_processed:%d data_structure_size:%d\n",
//__FILE__, __FUNCTION__, __LINE__, index,store->size_pulse_record_raw,store->size_pulse_record_processed,mb_io_ptr->data_structure_size);

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
            
            /* get calibration information for head a */
            calibration = &store->calibration_a;
            memcpy(calibration->cfg_path, &buffer[index], 64); index +=64;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(calibration->laser_head_no)); index += 4;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(calibration->process_for_air)); index += 4;
            calibration->temperature_compensation = buffer[index]; index += 1;
            calibration->emergency_shutdown = buffer[index]; index += 1;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ocb_temperature_limit_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ocb_humidity_limit)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_temperature_limit_1_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_temperature_limit_2_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_humidity_limit)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->dig_temperature_limit_c)); index += 4;
            memcpy(calibration->l_d_cable_set, &buffer[index], 24); index +=24;
            memcpy(calibration->ocb_comm_port, &buffer[index], 24); index += 24;
            memcpy(calibration->ocb_comm_cfg, &buffer[index], 24); index += 24;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ao_deg_to_volt)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ai_neg_v_to_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ai_pos_v_to_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_air)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_air)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g300)); index += 4;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly2)); index += 8;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly1)); index += 8;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly)); index += 8;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->laser_start_time_sec)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->scanner_shift_cts)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_lrg_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_med_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_sml_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->el_angle_fixed_deg)); index += 4;
            memcpy(calibration->unused, &buffer[index], 116); index += 116;
//fprintf(stderr,"%s:%s():%d INDEX at end of calibration a: %zu\n",
//__FILE__, __FUNCTION__, __LINE__, index);
            
            /* get calibration information for head b */
            calibration = &store->calibration_b;
            memcpy(calibration->cfg_path, &buffer[index], 64); index +=64;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(calibration->laser_head_no)); index += 4;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(calibration->process_for_air)); index += 4;
            calibration->temperature_compensation = buffer[index]; index += 1;
            calibration->emergency_shutdown = buffer[index]; index += 1;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ocb_temperature_limit_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ocb_humidity_limit)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_temperature_limit_1_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_temperature_limit_2_c)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->pb_humidity_limit)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->dig_temperature_limit_c)); index += 4;
            memcpy(calibration->l_d_cable_set, &buffer[index], 24); index +=24;
            memcpy(calibration->ocb_comm_port, &buffer[index], 24); index += 24;
            memcpy(calibration->ocb_comm_cfg, &buffer[index], 24); index += 24;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ao_deg_to_volt)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ai_neg_v_to_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->az_ai_pos_v_to_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_air)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_air)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g4000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g3000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g2000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g1000)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g400)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->t1_water_secondary_g300)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->ff_water_secondary_g300)); index += 4;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly2)); index += 8;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly1)); index += 8;
            mb_get_binary_double(MB_YES, (void *)&buffer[index], &(calibration->temp_comp_poly)); index += 8;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->laser_start_time_sec)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->scanner_shift_cts)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_lrg_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_med_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->factory_scanner_sml_deg)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(calibration->el_angle_fixed_deg)); index += 4;
            memcpy(calibration->unused, &buffer[index], 116); index += 116;
//fprintf(stderr,"%s:%s():%d INDEX at end of calibration b: %zu\n",
//__FILE__, __FUNCTION__, __LINE__, index);

            /* if ok and parameter_id is for the fileheader and the magic number is correct
             * then set flag */
            if (store->parameter_id == MBSYS_3DDWISSL_RECORD_FILEHEADER
                && store->magic_number == MBF_3DWISSLR_MAGICNUMBER) {
                /* set read flag */
                *file_header_readwritten = MB_YES;
                
                store->kind = MB_DATA_PARAMETER;
            }
            /* else this is not a first generation WISSL file, set error */
            else {
                status = MB_FAILURE;
                *error = MB_ERROR_BAD_FORMAT;
                store->kind = MB_DATA_NONE;
            }
		}
      
        /* parse a data record */
        else if (status == MB_SUCCESS
            && mb_io_ptr->indextable[irecord].kind == MB_DATA_DATA) {
            index = 0;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->record_id)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->year)); index += 2;
            store->month = buffer[index]; index += 1;
            store->day = buffer[index]; index += 1;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->jday)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->hour)); index += 2;
            store->minutes = buffer[index]; index += 1;
            store->seconds = buffer[index]; index += 1;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(store->nanoseconds)); index += 4;
            store->time_d = 0.0;
            store->navlon = 0.0;
            store->navlat = 0.0;
            store->sensordepth = 0.0;
            store->speed = 0.0;
            store->heading = 0.0;
            store->roll = 0.0;
            store->pitch = 0.0;
            store->gain = buffer[index]; index += 1;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->digitizer_temperature)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->ctd_temperature)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->ctd_salinity)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->ctd_pressure)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->index)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->range_start)); index += 4;
            mb_get_binary_float(MB_YES, (void *)&buffer[index], &(store->range_end)); index += 4;
            mb_get_binary_int(MB_YES, (void *)&buffer[index], &(store->pulse_count)); index += 4;
            
            /* read the pulses */
            for (ipulse=0; ipulse<store->pulses_per_scan; ipulse++) {
                pulse = &store->pulses[ipulse];
                mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->angle_az)); index += 4;
                mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->angle_el)); index += 4;
                mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->offset_az)); index += 4;
                mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->offset_el)); index += 4;
                mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->time_offset)); index += 4;
                pulse->time_d = 0.0;
                pulse->acrosstrack_offset = 0.0;
                pulse->alongtrack_offset = 0.0;
                pulse->sensordepth_offset = 0.0;
                pulse->heading_offset = 0.0;
                pulse->roll_offset = 0.0;
                pulse->pitch_offset = 0.0;
                for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
                    mb_get_binary_float(MB_YES, (void *)&buffer[index], &(pulse->soundings[isounding].range)); index += 4;
                }
                for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
                    mb_get_binary_short(MB_YES, (void *)&buffer[index], &(pulse->soundings[isounding].amplitude)); index += 2;
                }
                for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
                    pulse->soundings[isounding].beamflag = MB_FLAG_NULL;
                    pulse->soundings[isounding].acrosstrack = 0.0;
                    pulse->soundings[isounding].alongtrack = 0.0;
                    pulse->soundings[isounding].depth = 0.0;
                }
            }
            
            store->bathymetry_calculated = MB_NO;
            store->kind = MB_DATA_DATA;
        }
      
        /* parse a comment */
        else if (status == MB_SUCCESS
            && mb_io_ptr->indextable[irecord].kind == MB_DATA_COMMENT) {
            index = 0;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->record_id)); index += 2;
            mb_get_binary_short(MB_YES, (void *)&buffer[index], &(store->comment_len)); index += 2;
            read_len = (size_t)(MIN(store->comment_len, MB_COMMENT_MAXLINE-1));
            memset(store->comment, 0, MB_COMMENT_MAXLINE);
            memcpy(store->comment, &buffer[4], read_len);
            store->kind = MB_DATA_COMMENT;
        }

    }
    
    /* else no more records to read */
    else {
        status = MB_FAILURE;
        *error = MB_ERROR_EOF;
    }
    
//fprintf(stderr,"%s:%s():%d END of mbr_3dwisslr_rd_data: status:%d error:%d kind:%d\n",
//__FILE__, __FUNCTION__, __LINE__, status,*error,store->kind);

	/* print out status info */
	if (verbose >= 3 && status == MB_SUCCESS)
		mbsys_3ddwissl_print_store(verbose, store_ptr, error);

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
int mbr_3dwisslr_wr_data(int verbose, void *mbio_ptr, void *store_ptr, int *error) {
	char *function_name = "mbr_3dwisslr_wr_data";
	int status = MB_SUCCESS;
	struct mb_io_struct *mb_io_ptr;
	struct mbsys_3ddwissl_struct *store;
	struct mbsys_3ddwissl_pulse_struct *pulse;
	struct mbsys_3ddwissl_calibration_struct *calibration;
	int *file_header_readwritten;
	char *buffer = NULL;
	size_t write_len;
	size_t index;
    long file_pos;
	int i, ipulse, isounding;

	/* print input debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> called\n", function_name);
		fprintf(stderr, "dbg2  Revision id: %s\n", rcs_id);
		fprintf(stderr, "dbg2  Input arguments:\n");
		fprintf(stderr, "dbg2       verbose:    %d\n", verbose);
		fprintf(stderr, "dbg2       mbio_ptr:   %p\n", (void *)mbio_ptr);
		fprintf(stderr, "dbg2       store_ptr:  %p\n", (void *)store_ptr);
	}

	/* check for non-null pointers */
	assert(mbio_ptr != NULL);
	assert(store_ptr != NULL);

	/* get pointer to mbio descriptor */
	mb_io_ptr = (struct mb_io_struct *)mbio_ptr;
	store = (struct mbsys_3ddwissl_struct *)store_ptr;

	/* get saved values */
	file_header_readwritten = (int *)&mb_io_ptr->save1;

	/* set file position */
	mb_io_ptr->file_pos = ftell(mb_io_ptr->mbfp);

	/* print output debug statements */
	if (verbose >= 4) {
		fprintf(stderr, "\ndbg4  Data record kind in MBIO function <%s>\n", function_name);
		fprintf(stderr, "dbg4       kind:       %d\n", store->kind);
	}

	/* set status */
	status = MB_SUCCESS;
	*error = MB_ERROR_NO_ERROR;

	/* if first write then write the magic number file header */
	if (store->kind == MB_DATA_PARAMETER
        || (store->kind == MB_DATA_DATA && *file_header_readwritten != MB_YES)) {
        /* if comments have been written then reset file position to start of file */
        if (mb_io_ptr->file_pos > 0) {
            fseek(mb_io_ptr->mbfp, 0, SEEK_SET);
        }
        
		/* calculate size of output lidar record and allocate write buffer to handle that */
        write_len = (size_t)MAX((MBSYS_3DDWISSL_V1S1_RAW_SCAN_HEADER_SIZE
                                + store->pulses_per_scan
                                    * (MBSYS_3DDWISSL_V1S1_RAW_PULSE_HEADER_SIZE
                                + store->soundings_per_pulse
                                    * MBSYS_3DDWISSL_V1S1_RAW_SOUNDING_SIZE)),
                               (MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE
                                + 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE));
        if (mb_io_ptr->data_structure_size < write_len) {
            status = mb_reallocd(verbose, __FILE__, __LINE__, write_len,
                                 (void **)(&mb_io_ptr->raw_data), error);
            if (status == MB_SUCCESS) {
                mb_io_ptr->data_structure_size = write_len;
            }
        }
        
		/* calculate size of parameter record to be written here */
        write_len = (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE
                            + 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
        
		/* write file header which is also the parameter record */
		if (status == MB_SUCCESS) {
			index = 0;
            buffer = mb_io_ptr->raw_data;
            
            /* start of parameter record (and file ) */
            store->parameter_id = MBSYS_3DDWISSL_RECORD_FILEHEADER;
            store->magic_number = MBF_3DWISSLR_MAGICNUMBER;
			mb_put_binary_short(MB_YES, store->parameter_id, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->magic_number, (void **)&buffer[index]); index += 2;
            
            /* get scan information */
            mb_put_binary_short(MB_YES, store->file_version, (void **)&buffer[index]); index += 2;
            mb_put_binary_short(MB_YES, store->sub_version, (void **)&buffer[index]); index += 2;
            mb_put_binary_float(MB_YES, store->cross_track_angle_start, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, store->cross_track_angle_end, (void **)&buffer[index]); index += 4;
            mb_put_binary_short(MB_YES, store->pulses_per_scan, (void **)&buffer[index]); index += 2;
            buffer[index] = store->soundings_per_pulse; index += 1;
            mb_put_binary_short(MB_YES, store->heada_scans_per_file, (void **)&buffer[index]); index += 2;
            mb_put_binary_short(MB_YES, store->headb_scans_per_file, (void **)&buffer[index]); index += 2;
//fprintf(stderr,"%s:%s():%d INDEX at end of scan information: %zu  size_pulse_record_raw:%d size_pulse_record_processed:%d data_structure_size:%d\n",
//__FILE__, __FUNCTION__, __LINE__, index,store->size_pulse_record_raw,store->size_pulse_record_processed,mb_io_ptr->data_structure_size);
//fprintf(stderr,"    file_version:%d sub_version:%d pulses_per_scan:%d soundings_per_pulse:%d\n",
//store->file_version,store->sub_version,store->pulses_per_scan,store->soundings_per_pulse);

            /* get calibration information for head a */
            calibration = &store->calibration_a;
            memcpy((void **)&buffer[index], calibration->cfg_path, 64); index +=64;
            mb_put_binary_int(MB_YES, calibration->laser_head_no, (void **)&buffer[index]); index += 4;
            mb_put_binary_int(MB_YES, calibration->process_for_air, (void **)&buffer[index]); index += 4;
            buffer[index] = calibration->temperature_compensation; index += 1;
            buffer[index] = calibration->emergency_shutdown; index += 1;
            mb_put_binary_float(MB_YES, calibration->ocb_temperature_limit_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ocb_humidity_limit, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_temperature_limit_1_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_temperature_limit_2_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_humidity_limit, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->dig_temperature_limit_c, (void **)&buffer[index]); index += 4;
            memcpy((void **)&buffer[index], calibration->l_d_cable_set, 24); index +=24;
            memcpy((void **)&buffer[index], calibration->ocb_comm_port, 24); index += 24;
            memcpy((void **)&buffer[index], calibration->ocb_comm_cfg, 24); index += 24;
            mb_put_binary_float(MB_YES, calibration->az_ao_deg_to_volt, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->az_ai_neg_v_to_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->az_ai_pos_v_to_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_air, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_air, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly2, (void **)&buffer[index]); index += 8;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly1, (void **)&buffer[index]); index += 8;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly, (void **)&buffer[index]); index += 8;
            mb_put_binary_float(MB_YES, calibration->laser_start_time_sec, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->scanner_shift_cts, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_lrg_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_med_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_sml_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->el_angle_fixed_deg, (void **)&buffer[index]); index += 4;
            memcpy((void **)&buffer[index], calibration->unused, 116); index +=116;
//fprintf(stderr,"%s:%s():%d INDEX at end of calibration a: %zu\n",
//__FILE__, __FUNCTION__, __LINE__, index);
                
            /* get calibration information for head b */
            calibration = &store->calibration_b;
            memcpy((void **)&buffer[index], calibration->cfg_path, 64); index +=64;
            mb_put_binary_int(MB_YES, calibration->laser_head_no, (void **)&buffer[index]); index += 4;
            mb_put_binary_int(MB_YES, calibration->process_for_air, (void **)&buffer[index]); index += 4;
            buffer[index] = calibration->temperature_compensation; index += 1;
            buffer[index] = calibration->emergency_shutdown; index += 1;
            mb_put_binary_float(MB_YES, calibration->ocb_temperature_limit_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ocb_humidity_limit, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_temperature_limit_1_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_temperature_limit_2_c, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->pb_humidity_limit, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->dig_temperature_limit_c, (void **)&buffer[index]); index += 4;
            memcpy((void **)&buffer[index], calibration->l_d_cable_set, 24); index +=24;
            memcpy((void **)&buffer[index], calibration->ocb_comm_port, 24); index += 24;
            memcpy((void **)&buffer[index], calibration->ocb_comm_cfg, 24); index += 24;
            mb_put_binary_float(MB_YES, calibration->az_ao_deg_to_volt, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->az_ai_neg_v_to_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->az_ai_pos_v_to_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_air, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_air, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g4000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g3000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g2000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g1000, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g400, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->t1_water_secondary_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->ff_water_secondary_g300, (void **)&buffer[index]); index += 4;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly2, (void **)&buffer[index]); index += 8;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly1, (void **)&buffer[index]); index += 8;
            mb_put_binary_double(MB_YES, calibration->temp_comp_poly, (void **)&buffer[index]); index += 8;
            mb_put_binary_float(MB_YES, calibration->laser_start_time_sec, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->scanner_shift_cts, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_lrg_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_med_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->factory_scanner_sml_deg, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, calibration->el_angle_fixed_deg, (void **)&buffer[index]); index += 4;
            memcpy((void **)&buffer[index], calibration->unused, 116); index +=116;
//fprintf(stderr,"%s:%s():%d INDEX at end of calibration b: %zu\n",
//__FILE__, __FUNCTION__, __LINE__, index);
                
			/* write file header from buffer */
			status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
//fprintf(stderr,"%s:%s():%d Wrote file header %zu bytes\n",
//__FILE__, __FUNCTION__, __LINE__, write_len);

            /* reset file position to end of file in case comments have been written */
            fseek(mb_io_ptr->mbfp, 0, SEEK_END);
               
            *file_header_readwritten = MB_YES;
		}
    }

    /* write comment record */
    if (store->kind == MB_DATA_COMMENT) {
		/* calculate size of output comment record and parameter record and
            allocate write buffer to handle the larger of the two */
        write_len = MAX( (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE
                            + 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE),
                        MB_COMMENT_MAXLINE + 4);
        if (mb_io_ptr->data_structure_size < write_len) {
            status = mb_reallocd(verbose, __FILE__, __LINE__, write_len,
                                 (void **)(&mb_io_ptr->raw_data), error);
            if (status == MB_SUCCESS) {
                mb_io_ptr->data_structure_size = write_len;
            }
        }
        
        /* write dummy file header / parameter record if one hasn't already been
         * written */
        if (*file_header_readwritten == MB_NO) {
        
            /* calculate size of parameter record to be written here */
            write_len = (size_t)(MBSYS_3DDWISSL_V1S1_PARAMETER_SIZE
                                + 2 * MBSYS_3DDWISSL_V1S1_CALIBRATION_SIZE);
        
            /* write file header which is also the parameter record */
			index = 0;
            buffer = mb_io_ptr->raw_data;
            memset(buffer, 0, write_len);
            
            /* start of parameter record (and file ) */
			mb_put_binary_short(MB_YES, store->parameter_id, (void **)&buffer[index]); index += 2;
			mb_put_binary_short(MB_YES, store->magic_number, (void **)&buffer[index]); index += 2;
                
			/* write file header from buffer */
			status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
//fprintf(stderr,"%s:%s():%d Wrote dummy file header %zu bytes\n",
//__FILE__, __FUNCTION__, __LINE__, write_len);

            /* reset file position to end of file in case comments have been written */
            fseek(mb_io_ptr->mbfp, 0, SEEK_END);
               
            *file_header_readwritten = MB_MAYBE;
        }

        /* encode the comment */
        index = 0;
        buffer = mb_io_ptr->raw_data;
        
        store->record_id = MBSYS_3DDWISSL_RECORD_COMMENT;
		store->comment_len = MIN(strlen(store->comment), MB_COMMENT_MAXLINE-1);
        mb_put_binary_short(MB_YES, store->record_id, &buffer[index]);
        index += 2;
        mb_put_binary_short(MB_YES, store->comment_len, &buffer[index]);
        index += 2;
        memcpy(&buffer[index], store->comment, store->comment_len);
        index += store->comment_len;

        /* write comment record */
        write_len = (size_t)index;
        status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
//fprintf(stderr,"%s:%s():%d Wrote comment %zu bytes\n",
//__FILE__, __FUNCTION__, __LINE__, write_len);
    }

    /* write LIDAR scan record */
    else if (store->kind == MB_DATA_DATA) {
        /* encode the data */
        index = 0;
        buffer = mb_io_ptr->raw_data;
        
        if (store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADA) {
            store->record_id = MBSYS_3DDWISSL_RECORD_RAWHEADA;
        }
        if (store->record_id == MBSYS_3DDWISSL_RECORD_PROHEADB) {
            store->record_id = MBSYS_3DDWISSL_RECORD_RAWHEADB;
        }

        mb_put_binary_short(MB_YES, store->record_id, &buffer[index]); index += 2;
        mb_put_binary_short(MB_YES, store->year, &buffer[index]); index += 2;
        buffer[index] = (mb_u_char)store->month; index += 1;
        buffer[index] = (mb_u_char)store->day; index += 1;
        mb_put_binary_short(MB_YES, store->jday, &buffer[index]); index += 2;
        mb_put_binary_short(MB_YES, store->hour, &buffer[index]); index += 2;
        buffer[index] = (mb_u_char)store->minutes; index++;
        buffer[index] = (mb_u_char)store->seconds; index++;
        mb_put_binary_int(MB_YES, store->nanoseconds, &buffer[index]); index += 4;
        
        buffer[index] = store->gain; index += 1;
        mb_put_binary_float(MB_YES, store->digitizer_temperature, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->ctd_temperature, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->ctd_salinity, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->ctd_pressure, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->index, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->range_start, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->range_end, (void **)&buffer[index]); index += 4;
        mb_put_binary_float(MB_YES, store->pulse_count, (void **)&buffer[index]); index += 4;

        /* write scan pulses */
        for (ipulse=0; ipulse<store->pulses_per_scan; ipulse++) {
            pulse = &store->pulses[ipulse];
            mb_put_binary_float(MB_YES, pulse->angle_az, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, pulse->angle_el, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, pulse->offset_az, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, pulse->offset_el, (void **)&buffer[index]); index += 4;
            mb_put_binary_float(MB_YES, pulse->time_offset, (void **)&buffer[index]); index += 4;
            for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
                mb_put_binary_float(MB_YES, pulse->soundings[isounding].range, (void **)&buffer[index]); index += 4;
            }
            for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
                mb_put_binary_short(MB_YES, pulse->soundings[isounding].amplitude, (void **)&buffer[index]); index += 2;
            }
//for (isounding=0; isounding<store->soundings_per_pulse; isounding++) {
//fprintf(stderr,"%s:%s():%d Writing sounding ipulse:%d isounding:%d beamflag:%d\n",
//__FILE__, __FUNCTION__, __LINE__, ipulse, isounding, pulse->soundings[isounding].beamflag);
//}
        }

        /* write LIDAR scan record */
        write_len = (size_t)index;
        status = mb_fileio_put(verbose, mbio_ptr, (void *)buffer, &write_len, error);
//fprintf(stderr,"%s:%s():%d Wrote lidar record %zu bytes  pulse_count:%d time_d:%f\n",
//__FILE__, __FUNCTION__, __LINE__, write_len, store->pulse_count, store->time_d);
	}

	/* print output debug statements */
	if (verbose >= 2) {
		fprintf(stderr, "\ndbg2  MBIO function <%s> completed\n", function_name);
		fprintf(stderr, "dbg2  Return values:\n");
		fprintf(stderr, "dbg2       error:      %d\n", *error);
		fprintf(stderr, "dbg2  Return status:\n");
		fprintf(stderr, "dbg2       status:  %d\n", status);
	}

	/* return status */
	return (status);
}
/*--------------------------------------------------------------------*/
